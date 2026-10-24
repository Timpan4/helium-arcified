#include "handoff.h"

#include <cstdlib>
#include <iostream>
#include <utility>

namespace arcified::window_ownership {
namespace {

using namespace lifecycle;
static_assert(kContractVersion == 3);

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const ProfileId kWork{"work"};
const ProfileId kPersonal{"personal"};
const SpaceId kDesk{"desk"};
const SpaceId kResearch{"research"};
const SpaceId kHome{"home"};
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const Attachment kSource{kFirst, kDesk};
const Attachment kDestination{kSecond, kDesk};
const Attachment kRequest{kSecond, kResearch};
const Attachment kForeign{kSecond, kHome};
const ItemId kItem{"saved"};
const PageId kPage{"saved-page"};
const PageId kTemporary{"local-temporary"};
const PageId kOther{"other-window-temporary"};
const SplitId kPair{"pair"};
const SplitId kSingle{"single"};
const std::string kSaved = "https://synthetic.example.test/saved";
const std::string kCurrent = "https://synthetic.example.test/unsaved-form";

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kPersonal) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kResearch, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kHome, kPersonal) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  for (const auto& attachment : {kSource, kDestination, kRequest, kForeign})
    REQUIRE(model.Attach(attachment) == Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kDesk) == Status::kOk);
  REQUIRE(model.SelectSpace(kSecond, kHome) == Status::kOk);
  REQUIRE(model.AddItem(kItem, kDesk, kSaved) == Status::kOk);
  const auto opened = model.OpenItem(kItem, kSource, kPage);
  REQUIRE(std::get<OpenedPage>(opened).disposition ==
          OpenDisposition::kCreated);
  REQUIRE(model.RecordNavigation(kPage, kCurrent) == Status::kOk);
  REQUIRE(model.ReportActivity(kPage, Activity::kProtected) == Status::kOk);
  REQUIRE(model.OpenTemporary(kTemporary, TemporaryTabId{"local-tab"}, kSource,
                              kSaved) == Status::kOk);
  REQUIRE(model.OpenTemporary(kOther, TemporaryTabId{"other-tab"}, kForeign,
                              kSaved) == Status::kOk);
  REQUIRE(model.SetSplit(kPair, kSource, {kPage, kTemporary}) == Status::kOk);
  REQUIRE(model.SetSplit(kSingle, kSource, {kPage}) == Status::kOk);
  return model;
}

HandoffPlan Plan(HandoffResult result) {
  REQUIRE(std::holds_alternative<HandoffPlan>(result));
  return std::get<HandoffPlan>(std::move(result));
}

void Refused(const HandoffResult& result, HandoffRefusal expected) {
  REQUIRE(std::holds_alternative<HandoffRefusal>(result));
  REQUIRE(std::get<HandoffRefusal>(result) == expected);
}

void RequireOriginal(const Model& model) {
  REQUIRE(model.profiles().size() == 2);
  REQUIRE(model.spaces().size() == 3);
  REQUIRE(model.windows().size() == 2);
  REQUIRE(model.attachments().size() == 4);
  REQUIRE(model.items().size() == 1);
  REQUIRE(model.pages().size() == 3);
  const auto& page = model.pages().at(kPage);
  REQUIRE(page.profile == kWork);
  REQUIRE(page.current_navigation == kCurrent);
  REQUIRE(page.activity == Activity::kProtected);
  REQUIRE(page.attachment() == kSource);
  REQUIRE(std::get<SavedPage>(page.owner).item == kItem);
  REQUIRE(model.items().at(kItem).saved_destination == kSaved);
  REQUIRE(std::get<SpaceId>(model.items().at(kItem).owner) == kDesk);
  REQUIRE(model.TemporaryPages(kSource) == std::vector<PageId>{kTemporary});
  REQUIRE(model.TemporaryPages(kDestination).empty());
  REQUIRE(model.TemporaryPages(kForeign) == std::vector<PageId>{kOther});
  REQUIRE(model.windows().at(kFirst).selected_space == kDesk);
  REQUIRE(model.windows().at(kSecond).selected_space == kHome);
  REQUIRE(model.splits().size() == 2);
  REQUIRE(model.splits().at(kPair).pages ==
          (std::vector<PageId>{kPage, kTemporary}));
  REQUIRE(model.splits().at(kSingle).pages == std::vector<PageId>{kPage});
}

void PreparationAndAbandonmentRetainSource() {
  const auto model = Workspace();
  {
    const auto plan =
        Plan(PrepareSavedPageHandoff(model, kPage, kDestination, kPage));
    REQUIRE(plan.page == kPage);
    REQUIRE(plan.item == kItem);
    REQUIRE(plan.profile == kWork);
    REQUIRE(plan.source == kSource);
    REQUIRE(plan.destination == kDestination);
    REQUIRE(plan.focus.source_active == kPage);
    REQUIRE(plan.focus.destination_page == kPage);
    REQUIRE(plan.destination_selected_space == kHome);
    REQUIRE(RevalidateHandoff(model, plan, kPage) == Revalidation::kCurrent);
    // The caller cancels or fails before native actuation. No report occurs.
  }
  RequireOriginal(model);
  Refused(PrepareSavedPageHandoff(model, kTemporary, kDestination, kTemporary),
          HandoffRefusal::kPageNotSaved);
  Refused(PrepareSavedPageHandoff(model, kPage, kForeign, kPage),
          HandoffRefusal::kProfileMismatch);
  Refused(PrepareSavedPageHandoff(model, kPage, kRequest, kPage),
          HandoffRefusal::kWrongOwningSpace);
  Refused(PrepareSavedPageHandoff(model, PageId{"missing"}, kDestination, {}),
          HandoffRefusal::kPageNotFound);
  Refused(
      PrepareSavedPageHandoff(model, kPage, {WindowId{"absent"}, kDesk}, {}),
      HandoffRefusal::kNotAttached);
  RequireOriginal(model);
}

void SplitPreviewMatchesSeparateModelReport() {
  auto model = Workspace();
  const auto plan =
      Plan(PrepareSavedPageHandoff(model, kPage, kDestination, kPage));
  REQUIRE(plan.split_effects.size() == 2);
  // This is a separate synthetic success report, not a simulated native host.
  REQUIRE(model.TransferSavedPage(plan.page, plan.destination) == Status::kOk);
  for (const auto& effect : plan.split_effects) {
    if (effect.remaining_pages.empty()) {
      REQUIRE(model.splits().count(effect.split) == 0);
    } else {
      REQUIRE(model.splits().at(effect.split).pages == effect.remaining_pages);
      REQUIRE(model.splits().at(effect.split).attachment == effect.attachment);
    }
  }
  REQUIRE(model.splits().at(kPair).pages == std::vector<PageId>{kTemporary});
  REQUIRE(model.splits().count(kSingle) == 0);
  REQUIRE(model.pages().at(kPage).attachment() == kDestination);
  REQUIRE(model.pages().at(kPage).current_navigation == kCurrent);
  REQUIRE(model.pages().at(kPage).activity == Activity::kProtected);
  REQUIRE(model.pages().at(kPage).profile == kWork);
  REQUIRE(model.items().at(kItem).saved_destination == kSaved);
  REQUIRE(model.TemporaryPages(kSource) == std::vector<PageId>{kTemporary});
  REQUIRE(model.windows().at(kSecond).selected_space == kHome);
  REQUIRE(RevalidateHandoff(model, plan, kPage) ==
          Revalidation::kSourceChanged);
}

void OpenItemResolvesOwningSpaceWithoutSelecting() {
  auto model = Workspace();
  const auto plan = Plan(PrepareOpenItemHandoff(model, kItem, kRequest, kPage));
  REQUIRE(plan.command == HandoffCommand::kOpenItem);
  REQUIRE(plan.requested_destination == kRequest);
  REQUIRE(plan.destination == kDestination);
  REQUIRE(RevalidateHandoff(model, plan, kPage) == Revalidation::kCurrent);
  RequireOriginal(model);
  const PageId unused{"unused-candidate"};
  const auto opened =
      model.OpenItem(plan.item, plan.requested_destination, unused);
  REQUIRE(std::get<OpenedPage>(opened).page == plan.page);
  REQUIRE(std::get<OpenedPage>(opened).disposition ==
          OpenDisposition::kExisting);
  REQUIRE(model.pages().count(unused) == 0);
  REQUIRE(model.pages().at(kPage).attachment() == plan.destination);
  REQUIRE(model.windows().at(kSecond).selected_space == kHome);

  auto missing_owner_attachment = Workspace();
  REQUIRE(missing_owner_attachment.Detach(kDestination) == Status::kOk);
  Refused(PrepareOpenItemHandoff(missing_owner_attachment, kItem, kRequest, {}),
          HandoffRefusal::kNotAttached);
  REQUIRE(std::get<Status>(missing_owner_attachment.OpenItem(
              kItem, kRequest, unused)) == Status::kNotAttached);
  Refused(PrepareOpenItemHandoff(model, ItemId{"missing"}, kRequest, {}),
          HandoffRefusal::kItemNotFound);
  const ItemId closed{"closed"};
  REQUIRE(model.AddItem(closed, kWork, kSaved) == Status::kOk);
  Refused(PrepareOpenItemHandoff(model, closed, kRequest, {}),
          HandoffRefusal::kItemNotLive);
  REQUIRE(model.pages().size() == 3);
}

void ProfileShortcutsUseRequestingSpace() {
  auto model = Workspace();
  const ItemId shortcut{"profile-shortcut"};
  const PageId page{"shortcut-page"};
  REQUIRE(model.AddItem(shortcut, kWork, kSaved) == Status::kOk);
  REQUIRE(std::get<OpenedPage>(model.OpenItem(shortcut, kSource, page)).page ==
          page);
  const auto plan =
      Plan(PrepareOpenItemHandoff(model, shortcut, kRequest, kTemporary));
  REQUIRE(plan.destination == kRequest);
  REQUIRE(plan.profile == kWork);
  REQUIRE(plan.focus.source_active == kTemporary);
  REQUIRE(model.pages().at(page).activity == Activity::kUnknown);
  REQUIRE(RevalidateHandoff(model, plan, kTemporary) == Revalidation::kCurrent);
  const auto opened = model.OpenItem(shortcut, kRequest, PageId{"unused"});
  REQUIRE(std::get<OpenedPage>(opened).page == page);
  REQUIRE(model.pages().at(page).attachment() == plan.destination);
  REQUIRE(model.pages().at(page).activity == Activity::kUnknown);
  REQUIRE(model.pages().at(page).profile == kWork);
}

void SameAttachmentKeepsSplits() {
  auto model = Workspace();
  const auto plan = Plan(PrepareSavedPageHandoff(model, kPage, kSource, {}));
  REQUIRE(plan.split_effects.empty());
  REQUIRE(model.SetSplit(kPair, kSource, {kTemporary, kPage}) == Status::kOk);
  REQUIRE(RevalidateHandoff(model, plan, {}) == Revalidation::kCurrent);
  REQUIRE(model.TransferSavedPage(plan.page, plan.destination) == Status::kOk);
  REQUIRE(model.splits().at(kPair).pages ==
          (std::vector<PageId>{kTemporary, kPage}));
  REQUIRE(model.splits().at(kSingle).pages == std::vector<PageId>{kPage});
}

void StaleSourceAndReplacementAreRejected() {
  auto moved = Workspace();
  const auto plan = Plan(PrepareOpenItemHandoff(moved, kItem, kRequest, kPage));
  REQUIRE(moved.TransferSavedPage(kPage, kDestination) == Status::kOk);
  REQUIRE(RevalidateHandoff(moved, plan, kPage) ==
          Revalidation::kSourceChanged);

  auto closed = Workspace();
  REQUIRE(closed.ReportActivity(kPage, Activity::kQuiescent) == Status::kOk);
  REQUIRE(closed.ClosePage(kPage) == Status::kOk);
  REQUIRE(RevalidateHandoff(closed, plan, {}) == Revalidation::kSourceChanged);
  const PageId replacement{"new-live-instance"};
  REQUIRE(
      std::get<OpenedPage>(closed.OpenItem(kItem, kSource, replacement)).page ==
      replacement);
  REQUIRE(RevalidateHandoff(closed, plan, replacement) ==
          Revalidation::kSourceChanged);
  REQUIRE(closed.pages().at(replacement).attachment() == kSource);
}

void StaleDestinationAttachmentWindowAndProfileAreRejected() {
  auto detached = Workspace();
  const auto plan =
      Plan(PrepareOpenItemHandoff(detached, kItem, kRequest, kPage));
  REQUIRE(detached.Detach(kDestination) == Status::kOk);
  REQUIRE(RevalidateHandoff(detached, plan, kPage) ==
          Revalidation::kDestinationChanged);
  auto request_detached = Workspace();
  REQUIRE(request_detached.Detach(kRequest) == Status::kOk);
  REQUIRE(RevalidateHandoff(request_detached, plan, kPage) ==
          Revalidation::kDestinationChanged);

  auto reprofiled = Workspace();
  REQUIRE(reprofiled.Detach(kRequest) == Status::kOk);
  REQUIRE(reprofiled.ReassignEmptySpace(kResearch, kPersonal) == Status::kOk);
  REQUIRE(reprofiled.Attach(kRequest) == Status::kOk);
  REQUIRE(RevalidateHandoff(reprofiled, plan, kPage) ==
          Revalidation::kDestinationChanged);
  REQUIRE(reprofiled.pages().at(kPage).profile == kWork);
  REQUIRE(reprofiled.pages().at(kPage).attachment() == kSource);

  auto closed = Workspace();
  REQUIRE(closed.ReportActivity(kOther, Activity::kQuiescent) == Status::kOk);
  REQUIRE(closed.CloseWindow(kSecond) == Status::kOk);
  REQUIRE(RevalidateHandoff(closed, plan, kPage) ==
          Revalidation::kDestinationChanged);
  REQUIRE(closed.pages().at(kPage).current_navigation == kCurrent);
}

void SelectionAndNativeFocusAreRevalidatedSeparately() {
  auto model = Workspace();
  const auto plan =
      Plan(PrepareSavedPageHandoff(model, kPage, kDestination, kPage));
  REQUIRE(RevalidateHandoff(model, plan, kTemporary) ==
          Revalidation::kFocusChanged);
  Refused(PrepareSavedPageHandoff(model, kPage, kDestination, kOther),
          HandoffRefusal::kInvalidActivePage);
  Refused(
      PrepareSavedPageHandoff(model, kPage, kDestination, PageId{"not-live"}),
      HandoffRefusal::kInvalidActivePage);
  RequireOriginal(model);
  REQUIRE(model.SelectSpace(kSecond, kResearch) == Status::kOk);
  REQUIRE(RevalidateHandoff(model, plan, kPage) ==
          Revalidation::kSelectionChanged);
  REQUIRE(model.SelectSpace(kSecond, kHome) == Status::kOk);
  REQUIRE(model.Attach({kFirst, kHome}) == Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kHome) == Status::kOk);
  REQUIRE(RevalidateHandoff(model, plan, kPage) ==
          Revalidation::kSelectionChanged);

  auto active_moved = Workspace();
  const auto active_plan = Plan(
      PrepareSavedPageHandoff(active_moved, kPage, kDestination, kTemporary));
  REQUIRE(active_moved.Keep(kTemporary, ItemId{"kept-active"}) == Status::kOk);
  REQUIRE(active_moved.TransferSavedPage(kTemporary, kDestination) ==
          Status::kOk);
  REQUIRE(RevalidateHandoff(active_moved, active_plan, kTemporary) ==
          Revalidation::kFocusChanged);
}

void SplitEditsInvalidateOnlyAffectedPreview() {
  auto model = Workspace();
  const auto plan =
      Plan(PrepareSavedPageHandoff(model, kPage, kDestination, kPage));
  REQUIRE(model.SetSplit(kPair, kSource, {kTemporary, kPage}) == Status::kOk);
  REQUIRE(RevalidateHandoff(model, plan, kPage) ==
          Revalidation::kSplitEffectsChanged);
  REQUIRE(model.SetSplit(kPair, kSource, {kPage, kTemporary}) == Status::kOk);
  REQUIRE(model.SetSplit(SplitId{"unrelated"}, kForeign, {kOther}) ==
          Status::kOk);
  REQUIRE(RevalidateHandoff(model, plan, kPage) == Revalidation::kCurrent);
  REQUIRE(model.SetSplit(SplitId{"new-reference"}, kSource, {kPage}) ==
          Status::kOk);
  REQUIRE(RevalidateHandoff(model, plan, kPage) ==
          Revalidation::kSplitEffectsChanged);
  REQUIRE(model.RemoveSplit(SplitId{"new-reference"}) == Status::kOk);
  REQUIRE(model.RemoveSplit(kSingle) == Status::kOk);
  REQUIRE(RevalidateHandoff(model, plan, kPage) ==
          Revalidation::kSplitEffectsChanged);
}

void NavigationAndActivityRemainModelOwned() {
  auto model = Workspace();
  const auto plan =
      Plan(PrepareSavedPageHandoff(model, kPage, kDestination, {}));
  const std::string later = "https://synthetic.example.test/later-document";
  REQUIRE(model.RecordNavigation(kPage, later) == Status::kOk);
  REQUIRE(RevalidateHandoff(model, plan, {}) == Revalidation::kCurrent);
  REQUIRE(model.ReportActivity(kPage, Activity::kProtected) == Status::kOk);
  REQUIRE(RevalidateHandoff(model, plan, {}) == Revalidation::kCurrent);
  REQUIRE(model.TransferSavedPage(kPage, plan.destination) == Status::kOk);
  REQUIRE(model.pages().at(kPage).current_navigation == later);
  REQUIRE(model.pages().at(kPage).activity == Activity::kProtected);
  REQUIRE(model.items().at(kItem).saved_destination == kSaved);
}

void TransferAndOpenPreconditionsAgreeWithModel() {
  const auto original = Workspace();
  for (const auto& destination : {kSource, kDestination, kRequest, kForeign,
                                  Attachment{WindowId{"absent"}, kDesk}}) {
    auto transferred = original;
    const auto prepared =
        PrepareSavedPageHandoff(original, kPage, destination, {});
    const auto status = transferred.TransferSavedPage(kPage, destination);
    REQUIRE(std::holds_alternative<HandoffPlan>(prepared) ==
            (status == Status::kOk));
    if (const auto* plan = std::get_if<HandoffPlan>(&prepared))
      REQUIRE(transferred.pages().at(kPage).attachment() == plan->destination);

    auto opened = original;
    const auto prepared_open =
        PrepareOpenItemHandoff(original, kItem, destination, {});
    const auto result = opened.OpenItem(kItem, destination, PageId{"unused"});
    REQUIRE(std::holds_alternative<HandoffPlan>(prepared_open) ==
            std::holds_alternative<OpenedPage>(result));
    if (const auto* plan = std::get_if<HandoffPlan>(&prepared_open)) {
      REQUIRE(std::get<OpenedPage>(result).page == plan->page);
      REQUIRE(opened.pages().at(kPage).attachment() == plan->destination);
    }
  }
  RequireOriginal(original);
}

}  // namespace
}  // namespace arcified::window_ownership

int main() {
  using namespace arcified::window_ownership;
  PreparationAndAbandonmentRetainSource();
  SplitPreviewMatchesSeparateModelReport();
  OpenItemResolvesOwningSpaceWithoutSelecting();
  ProfileShortcutsUseRequestingSpace();
  SameAttachmentKeepsSplits();
  StaleSourceAndReplacementAreRejected();
  StaleDestinationAttachmentWindowAndProfileAreRejected();
  SelectionAndNativeFocusAreRevalidatedSeparately();
  SplitEditsInvalidateOnlyAffectedPreview();
  NavigationAndActivityRemainModelOwned();
  TransferAndOpenPreconditionsAgreeWithModel();
  std::cout << "window ownership synthetic checks passed\n";
}
