#include "planning.h"

#include <cstdlib>
#include <iostream>

namespace arcified::little_arc {
namespace {

using namespace lifecycle;

static_assert(kContractVersion == 3, "Review plans when lifecycle changes");

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

struct Fixtures {
  const ProfileId kWork{"work"};
  const ProfileId kPersonal{"personal"};
  const SpaceId kDesk{"desk"};
  const SpaceId kResearch{"research"};
  const SpaceId kHome{"home"};
  const WindowId kSmall{"small"};
  const WindowId kMain{"main"};
  const WindowId kOther{"other"};
  const Attachment kSource{kSmall, kDesk};
  const Attachment kDestination{kMain, kDesk};
  const Attachment kOtherDestination{kOther, kDesk};
  const Attachment kResearchMain{kMain, kResearch};
  const Attachment kHomeMain{kMain, kHome};
  const PageId kTemporary{"temporary-page"};
  const TemporaryTabId kTab{"temporary-tab"};
  const PageId kSavedPage{"saved-page"};
  const ItemId kItem{"saved-item"};
  const SplitId kSplit{"source-split"};
  const std::string kLink = "https://entry.example.test/link";
  const std::string kDraft = "https://entry.example.test/draft";

  Model Workspace() {
    Model model;
    REQUIRE(model.AddProfile(kWork) == Status::kOk);
    REQUIRE(model.AddProfile(kPersonal) == Status::kOk);
    REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
    REQUIRE(model.AddSpace(kResearch, kWork) == Status::kOk);
    REQUIRE(model.AddSpace(kHome, kPersonal) == Status::kOk);
    for (const auto& window : {kSmall, kMain, kOther})
      REQUIRE(model.AddWindow(window) == Status::kOk);
    for (const auto& attachment :
         {kSource, kDestination, kOtherDestination, kResearchMain, kHomeMain}) {
      REQUIRE(model.Attach(attachment) == Status::kOk);
    }
    REQUIRE(model.SelectSpace(kMain, kHome) == Status::kOk);
    REQUIRE(model.OpenTemporary(kTemporary, kTab, kSource, kDraft) ==
            Status::kOk);
    REQUIRE(model.ReportActivity(kTemporary, Activity::kProtected) ==
            Status::kOk);
    REQUIRE(model.AddItem(kItem, kDesk, kLink) == Status::kOk);
    REQUIRE(std::holds_alternative<OpenedPage>(
        model.OpenItem(kItem, kSource, kSavedPage)));
    REQUIRE(model.RecordNavigation(kSavedPage, kDraft) == Status::kOk);
    REQUIRE(model.SetSplit(kSplit, kSource, {kTemporary, kSavedPage}) ==
            Status::kOk);
    return model;
  }

  void ExpectOriginalGraph(const Model& model) {
    REQUIRE(model.pages().size() == 2);
    REQUIRE(model.items().size() == 1);
    REQUIRE(model.items().at(kItem).saved_destination == kLink);
    REQUIRE(model.items().at(kItem).owner == ItemOwner{kDesk});
    for (const auto& page : {kTemporary, kSavedPage}) {
      REQUIRE(model.pages().at(page).profile == kWork);
      REQUIRE(model.pages().at(page).attachment() == kSource);
      REQUIRE(model.pages().at(page).current_navigation == kDraft);
    }
    REQUIRE(std::get<TemporaryTab>(model.pages().at(kTemporary).owner).id ==
            kTab);
    REQUIRE(model.pages().at(kTemporary).activity == Activity::kProtected);
    REQUIRE(model.pages().at(kSavedPage).activity == Activity::kUnknown);
    REQUIRE(std::get<SavedPage>(model.pages().at(kSavedPage).owner).item ==
            kItem);
    REQUIRE(model.TemporaryPages(kDestination).empty());
    REQUIRE(model.splits().at(kSplit).pages ==
            (std::vector<PageId>{kTemporary, kSavedPage}));
    REQUIRE(model.windows().at(kMain).selected_space == kHome);
  }

  template <typename Result>
  void ExpectRefusal(const Result& result, Refusal refusal) {
    REQUIRE(std::holds_alternative<Refusal>(result));
    REQUIRE(std::get<Refusal>(result) == refusal);
  }

  void EntryPreservesExplicitIntent() {
    const auto model = Workspace();
    for (auto presentation :
         {Presentation::kMainWindow, Presentation::kLightweightWindow}) {
      // Repeated links yield independent requests, never inferred
      // reuse/identity.
      for (const auto& attachment : {kDestination, kOtherDestination}) {
        const auto result = PlanEntry(model, {kWork, attachment},
                                      ExternalLink{kLink}, presentation);
        REQUIRE(std::holds_alternative<EntryPlan>(result));
        const auto& plan = std::get<EntryPlan>(result);
        REQUIRE(plan.target.profile == kWork);
        REQUIRE(plan.target.attachment == attachment);
        REQUIRE(plan.presentation == presentation);
        REQUIRE(std::get<ExternalLink>(plan.intent).destination == kLink);
      }
    }
    const auto lookup = PlanEntry(model, {kPersonal, kHomeMain}, QuickLookup{},
                                  Presentation::kLightweightWindow);
    REQUIRE(std::holds_alternative<EntryPlan>(lookup));
    REQUIRE(std::holds_alternative<QuickLookup>(
        std::get<EntryPlan>(lookup).intent));
    REQUIRE(std::get<EntryPlan>(lookup).target.profile == kPersonal);
    ExpectOriginalGraph(model);
  }

  void RejectInvalidResolvedTargets() {
    const auto model = Workspace();
    const std::pair<ResolvedTarget, Refusal> cases[] = {
        {{ProfileId{"missing"}, kDestination}, Refusal::kTargetNotFound},
        {{kWork, {WindowId{"missing"}, kDesk}}, Refusal::kTargetNotFound},
        {{kWork, {kMain, SpaceId{"missing"}}}, Refusal::kTargetNotFound},
        {{kPersonal, kDestination}, Refusal::kTargetProfileMismatch},
        {{kWork, {kSmall, kResearch}}, Refusal::kTargetNotAttached},
    };
    for (const auto& [target, refusal] : cases) {
      ExpectRefusal(PlanEntry(model, target, ExternalLink{kLink},
                              Presentation::kLightweightWindow),
                    refusal);
      ExpectRefusal(PlanPromotion(model, kTemporary, target), refusal);
    }
    ExpectRefusal(PlanEntry(model, {kWork, kDestination}, ExternalLink{""},
                            Presentation::kLightweightWindow),
                  Refusal::kEmptyDestination);
    ExpectOriginalGraph(model);
  }

  void PresentSamePageInSameHost() {
    const auto model = Workspace();
    for (const auto& page : {kTemporary, kSavedPage}) {
      // Borrowed lifecycle identities are accepted; the result owns its
      // snapshot.
      const auto result =
          PlanPromotion(model, model.pages().find(page)->first,
                        {kWork, model.pages().at(page).attachment()});
      REQUIRE(std::holds_alternative<PresentInMainWindow>(result));
      const auto& source = std::get<PresentInMainWindow>(result).source;
      REQUIRE(source.page == page);
      REQUIRE(source.profile == kWork);
      REQUIRE(source.attachment == kSource);
    }
    ExpectOriginalGraph(model);
  }

  void PromotionRefusesOwnershipChanges() {
    const auto model = Workspace();
    for (const auto& target : {kDestination, kResearchMain}) {
      ExpectRefusal(PlanPromotion(model, kTemporary, {kWork, target}),
                    Refusal::kTemporaryTransferUnavailable);
    }
    for (const auto& page : {kTemporary, kSavedPage}) {
      ExpectRefusal(PlanPromotion(model, page, {kPersonal, kHomeMain}),
                    Refusal::kPageProfileMismatch);
    }
    ExpectRefusal(PlanPromotion(model, kSavedPage, {kWork, kResearchMain}),
                  Refusal::kSavedSpaceMismatch);
    ExpectRefusal(
        PlanPromotion(model, PageId{"missing"}, {kWork, kDestination}),
        Refusal::kPageNotFound);
    ExpectOriginalGraph(model);
  }

  void SavedTransferPlanAndSeparateSyntheticReport() {
    auto model = Workspace();
    const auto result = PlanPromotion(model, kSavedPage, {kWork, kDestination});
    REQUIRE(std::holds_alternative<TransferSavedPage>(result));
    const auto& plan = std::get<TransferSavedPage>(result);
    REQUIRE(plan.source.page == kSavedPage);
    REQUIRE(plan.source.profile == kWork);
    REQUIRE(plan.source.attachment == kSource);
    REQUIRE(plan.item == kItem);
    REQUIRE(plan.destination == kDestination);
    ExpectOriginalGraph(model);

    // This models a separate success report. It does not simulate native
    // transfer.
    REQUIRE(model.TransferSavedPage(plan.source.page, plan.destination) ==
            Status::kOk);
    REQUIRE(model.pages().size() == 2);
    REQUIRE(model.pages().at(kSavedPage).attachment() == kDestination);
    REQUIRE(model.pages().at(kSavedPage).profile == kWork);
    REQUIRE(model.pages().at(kSavedPage).current_navigation == kDraft);
    REQUIRE(model.pages().at(kSavedPage).activity == Activity::kUnknown);
    REQUIRE(std::get<SavedPage>(model.pages().at(kSavedPage).owner).item ==
            kItem);
    REQUIRE(model.items().at(kItem).saved_destination == kLink);
    REQUIRE(model.splits().at(kSplit).pages == std::vector<PageId>{kTemporary});
    REQUIRE(model.pages().at(kTemporary).attachment() == kSource);
    REQUIRE(model.pages().at(kTemporary).activity == Activity::kProtected);
    REQUIRE(model.windows().at(kMain).selected_space == kHome);

    // Old snapshots stay old. A fresh decision reads the new authoritative
    // owner.
    REQUIRE(plan.source.attachment == kSource);
    const auto refreshed =
        PlanPromotion(model, kSavedPage, {kWork, kOtherDestination});
    REQUIRE(std::holds_alternative<TransferSavedPage>(refreshed));
    REQUIRE(std::get<TransferSavedPage>(refreshed).source.attachment ==
            kDestination);
  }

  void ExplicitKeepAndProfileShortcutUseExistingSemantics() {
    auto model = Workspace();
    // Only the caller's separate, explicit Keep changes the temporary owner.
    const ItemId kept{"explicitly-kept"};
    REQUIRE(model.Keep(kTemporary, kept) == Status::kOk);
    const auto result = PlanPromotion(model, kTemporary, {kWork, kDestination});
    REQUIRE(std::holds_alternative<TransferSavedPage>(result));
    REQUIRE(std::get<TransferSavedPage>(result).item == kept);
    REQUIRE(model.pages().at(kTemporary).activity == Activity::kProtected);
    REQUIRE(model.pages().at(kTemporary).attachment() == kSource);

    const ItemId favorite{"profile-favorite"};
    const PageId favorite_page{"favorite-page"};
    REQUIRE(model.AddItem(favorite, kWork, kLink) == Status::kOk);
    REQUIRE(std::holds_alternative<OpenedPage>(
        model.OpenItem(favorite, kSource, favorite_page)));
    const auto shortcut =
        PlanPromotion(model, favorite_page, {kWork, kResearchMain});
    REQUIRE(std::holds_alternative<TransferSavedPage>(shortcut));
    const auto& plan = std::get<TransferSavedPage>(shortcut);
    REQUIRE(plan.item == favorite);
    REQUIRE(plan.destination == kResearchMain);
    REQUIRE(model.TransferSavedPage(plan.source.page, plan.destination) ==
            Status::kOk);
    REQUIRE(model.items().at(favorite).owner == ItemOwner{kWork});
    REQUIRE(model.pages().at(favorite_page).profile == kWork);
  }
};

}  // namespace
}  // namespace arcified::little_arc

int main() {
  using namespace arcified::little_arc;
  Fixtures context;
  const std::pair<const char*, void (Fixtures::*)()> fixtures[] = {
      {"entry-explicit-intent", &Fixtures::EntryPreservesExplicitIntent},
      {"resolved-target-refusals", &Fixtures::RejectInvalidResolvedTargets},
      {"same-host-presentation", &Fixtures::PresentSamePageInSameHost},
      {"promotion-ownership-refusals",
       &Fixtures::PromotionRefusesOwnershipChanges},
      {"saved-plan-and-synthetic-report",
       &Fixtures::SavedTransferPlanAndSeparateSyntheticReport},
      {"explicit-keep-and-profile-shortcut",
       &Fixtures::ExplicitKeepAndProfileShortcutUseExistingSemantics},
  };
  for (const auto& [name, run] : fixtures) {
    (context.*run)();
    std::cout << "PASS " << name << '\n';
  }
}
