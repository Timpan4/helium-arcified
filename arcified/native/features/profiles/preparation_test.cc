#include "preparation.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace arcified::profiles {
namespace {

using lifecycle::Attachment;
using lifecycle::Model;
using lifecycle::PageId;
using lifecycle::ProfileId;
using lifecycle::SpaceId;
using lifecycle::Status;
using lifecycle::WindowId;
using private_browsing::ContextClass;
using private_browsing::NativeContextFacts;
using private_browsing::NativeContextId;
using private_browsing::NativeContextKind;

static_assert(!std::is_convertible_v<ProfileId, NativeContextId>);

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const ProfileId kWork{"work"};
const ProfileId kHome{"home"};
const ProfileId kPrivate{"private-live-profile"};
const NativeContextId kWorkNative{"native-work"};
const NativeContextId kHomeNative{"native-home"};
const NativeContextId kOtrNative{"native-otr"};
const SpaceId kDesk{"desk"};
const SpaceId kResearch{"research"};
const SpaceId kHomeSpace{"home-space"};
const SpaceId kPrivateSpace{"private-transient-space"};
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const Attachment kDeskFirst{kFirst, kDesk};
const Attachment kDeskSecond{kSecond, kDesk};
const Attachment kResearchFirst{kFirst, kResearch};
const Attachment kHomeFirst{kFirst, kHomeSpace};
const Attachment kPrivateFirst{kFirst, kPrivateSpace};
const ProfileBinding kWorkBinding{kWork, kWorkNative};
const ProfileBinding kHomeBinding{kHome, kHomeNative};
const ProfileBinding kOtrBinding{kPrivate, kOtrNative};
const NativeContextFacts kWorkFacts{NativeContextKind::kRegular, kWorkNative,
                                    kWork};
const NativeContextFacts kHomeFacts{NativeContextKind::kRegular, kHomeNative,
                                    kHome};
const NativeContextFacts kOtrFacts{NativeContextKind::kOffTheRecord, kOtrNative,
                                   kWork};

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kHome) == Status::kOk);
  REQUIRE(model.AddProfile(kPrivate) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kResearch, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kHomeSpace, kHome) == Status::kOk);
  // Synthetic transient graph only. This fixture neither serializes private
  // state nor grants permission to put it in an ordinary workspace store.
  REQUIRE(model.AddSpace(kPrivateSpace, kPrivate) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  for (const auto& attachment :
       {kDeskFirst, kDeskSecond, kResearchFirst, kHomeFirst, kPrivateFirst})
    REQUIRE(model.Attach(attachment) == Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kHomeSpace) == Status::kOk);
  return model;
}

template <typename T>
void ExpectRefusal(const T& result, PreparationRefusal expected) {
  REQUIRE(std::holds_alternative<PreparationRefusal>(result));
  REQUIRE(std::get<PreparationRefusal>(result) == expected);
}

void SharedProfileScopesAndExplicitTargets() {
  const auto model = Workspace();
  const auto desk = std::get<SpacePreparation>(
      PrepareSpace(model, kDeskFirst, kWorkBinding, kWorkFacts));
  const auto research = std::get<SpacePreparation>(
      PrepareSpace(model, kResearchFirst, kWorkBinding, kWorkFacts));
  const auto home = std::get<SpacePreparation>(
      PrepareSpace(model, kHomeFirst, kHomeBinding, kHomeFacts));
  REQUIRE(desk.target.profile == research.target.profile);
  REQUIRE(desk.target.context == research.target.context);
  REQUIRE(desk.attachment.space != research.attachment.space);
  REQUIRE(desk.attachment.window == home.attachment.window);
  REQUIRE(desk.target.context != home.target.context);
  REQUIRE(desk.target.context_class == ContextClass::kRegular);

  const auto services =
      std::get<RegularProfileServiceTarget>(PrepareRegularProfileServices(
          model, kDeskFirst, kWorkBinding, kWorkFacts));
  REQUIRE(services.profile == kWork);
  REQUIRE(services.context == kWorkNative);
  ExpectRefusal(PrepareSpace(model, kDeskFirst, kHomeBinding, kHomeFacts),
                PreparationRefusal::kBindingProfileMismatch);
  ExpectRefusal(PrepareSpace(model, kDeskFirst, kWorkBinding, kHomeFacts),
                PreparationRefusal::kNativeContextMismatch);
  auto wrong_mapping = kWorkFacts;
  wrong_mapping.original_profile = kHome;
  ExpectRefusal(PrepareSpace(model, kDeskFirst, kWorkBinding, wrong_mapping),
                PreparationRefusal::kOriginalProfileMismatch);
  REQUIRE(model.windows().at(kFirst).selected_space == kHomeSpace);
  REQUIRE(model.pages().empty());
  REQUIRE(model.items().empty());
}

void UnreferencedProfilesAndUnattachedSpaces() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  const auto unreferenced = std::get<RegularProfileServiceTarget>(
      PrepareRegularProfile(model, kWork, kWorkBinding, kWorkFacts));
  REQUIRE(unreferenced.profile == kWork);
  REQUIRE(unreferenced.context == kWorkNative);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  const auto unattached = std::get<RegularProfileServiceTarget>(
      PrepareRegularProfile(model, kWork, kWorkBinding, kWorkFacts));
  REQUIRE(unattached.context == unreferenced.context);
  REQUIRE(model.windows().empty());
  REQUIRE(model.attachments().empty());
  ExpectRefusal(PrepareRegularProfile(model, kHome, kHomeBinding, kHomeFacts),
                PreparationRefusal::kProfileNotFound);
  ExpectRefusal(PrepareRegularProfile(model, kWork, kHomeBinding, kHomeFacts),
                PreparationRefusal::kBindingProfileMismatch);
  ExpectRefusal(PrepareRegularProfile(model, kWork, kWorkBinding, kHomeFacts),
                PreparationRefusal::kNativeContextMismatch);
  REQUIRE(model.AddProfile(kPrivate) == Status::kOk);
  ExpectRefusal(PrepareRegularProfile(model, kPrivate, kOtrBinding, kOtrFacts),
                PreparationRefusal::kRegularProfileRequired);
}

void ExactOtrIdentityDoesNotGrantRegularServices() {
  const auto model = Workspace();
  const auto prepared = std::get<SpacePreparation>(
      PrepareSpace(model, kPrivateFirst, kOtrBinding, kOtrFacts));
  REQUIRE(prepared.target.profile == kPrivate);
  REQUIRE(prepared.target.context == kOtrNative);
  REQUIRE(prepared.target.context_class == ContextClass::kOffTheRecord);
  ExpectRefusal(PrepareRegularProfileServices(model, kPrivateFirst, kOtrBinding,
                                              kOtrFacts),
                PreparationRefusal::kRegularProfileRequired);

  // Sibling OTRs share original-Profile metadata but remain different objects.
  auto sibling = kOtrFacts;
  sibling.context = NativeContextId{"native-sibling-otr"};
  ExpectRefusal(PrepareSpace(model, kPrivateFirst, kOtrBinding, sibling),
                PreparationRefusal::kNativeContextMismatch);
  ExpectRefusal(PrepareSpace(model, kDeskFirst, kWorkBinding, kOtrFacts),
                PreparationRefusal::kNativeContextMismatch);
  auto no_original_mapping = kOtrFacts;
  no_original_mapping.original_profile.reset();
  REQUIRE(std::holds_alternative<SpacePreparation>(
      PrepareSpace(model, kPrivateFirst, kOtrBinding, no_original_mapping)));

  for (const auto kind :
       {NativeContextKind::kGuest, NativeContextKind::kGuestOffTheRecord,
        NativeContextKind::kSystem, NativeContextKind::kSystemOffTheRecord,
        NativeContextKind::kOther}) {
    auto unsupported = kOtrFacts;
    unsupported.kind = kind;
    ExpectRefusal(PrepareSpace(model, kPrivateFirst, kOtrBinding, unsupported),
                  PreparationRefusal::kUnsupportedNativeContext);
    ExpectRefusal(PrepareRegularProfileServices(model, kPrivateFirst,
                                                kOtrBinding, unsupported),
                  PreparationRefusal::kUnsupportedNativeContext);
  }
}

void MissingFactsAndAttachmentsFailClosed() {
  auto model = Workspace();
  ExpectRefusal(PrepareSpace(model, {WindowId{"missing"}, kDesk}, kWorkBinding,
                             kWorkFacts),
                PreparationRefusal::kWindowNotFound);
  ExpectRefusal(PrepareSpace(model, {kFirst, SpaceId{"missing"}}, kWorkBinding,
                             kWorkFacts),
                PreparationRefusal::kSpaceNotFound);
  ExpectRefusal(
      PrepareSpace(model, {kSecond, kResearch}, kWorkBinding, kWorkFacts),
      PreparationRefusal::kNotAttached);
  ExpectRefusal(PreparePage(model, PageId{"missing"}, kWorkBinding, kWorkFacts),
                PreparationRefusal::kPageNotFound);
  ExpectRefusal(PrepareSpace(model, kDeskFirst, kWorkBinding, {}),
                PreparationRefusal::kUnknownNativeContext);
  auto facts = kWorkFacts;
  facts.context.reset();
  ExpectRefusal(PrepareSpace(model, kDeskFirst, kWorkBinding, facts),
                PreparationRefusal::kUnknownNativeContext);
  facts.context = NativeContextId{""};
  ExpectRefusal(PrepareSpace(model, kDeskFirst, kWorkBinding, facts),
                PreparationRefusal::kUnknownNativeContext);
  facts = kWorkFacts;
  facts.original_profile.reset();
  ExpectRefusal(
      PrepareRegularProfileServices(model, kDeskFirst, kWorkBinding, facts),
      PreparationRefusal::kUnknownNativeContext);
  facts.original_profile = ProfileId{""};
  ExpectRefusal(PrepareSpace(model, kDeskFirst, kWorkBinding, facts),
                PreparationRefusal::kUnknownNativeContext);
  facts = kWorkFacts;
  facts.kind = NativeContextKind::kUnknown;
  ExpectRefusal(PrepareSpace(model, kDeskFirst, kWorkBinding, facts),
                PreparationRefusal::kUnknownNativeContext);
  REQUIRE(model.windows().at(kFirst).selected_space == kHomeSpace);
}

void PagePreparationUsesCurrentOwnerWithoutActuation() {
  auto model = Workspace();
  const lifecycle::ItemId item{"saved"};
  const PageId page{"live-saved"};
  const std::string saved = "https://saved.example.test/";
  const std::string current = "https://current.example.test/draft";
  REQUIRE(model.AddItem(item, kDesk, saved) == Status::kOk);
  REQUIRE(std::holds_alternative<lifecycle::OpenedPage>(
      model.OpenItem(item, kDeskFirst, page)));
  REQUIRE(model.RecordNavigation(page, current) == Status::kOk);
  REQUIRE(model.ReportActivity(page, lifecycle::Activity::kProtected) ==
          Status::kOk);
  const auto prepared = std::get<PagePreparation>(
      PreparePage(model, page, kWorkBinding, kWorkFacts));
  REQUIRE(prepared.page == page);
  REQUIRE(prepared.space.attachment == kDeskFirst);
  REQUIRE(prepared.space.target.profile == kWork);
  ExpectRefusal(PreparePage(model, page, kHomeBinding, kHomeFacts),
                PreparationRefusal::kBindingProfileMismatch);
  REQUIRE(model.pages().size() == 1);
  REQUIRE(model.pages().at(page).current_navigation == current);
  REQUIRE(model.pages().at(page).activity == lifecycle::Activity::kProtected);
  REQUIRE(model.items().at(item).saved_destination == saved);
  REQUIRE(model.windows().at(kFirst).selected_space == kHomeSpace);

  // Simulate an independently completed, approved semantic saved-page handoff.
  // The earlier preparation remains an owned stale snapshot, not a reservation.
  REQUIRE(model.TransferSavedPage(page, kDeskSecond) == Status::kOk);
  const auto after = std::get<PagePreparation>(
      PreparePage(model, page, kWorkBinding, kWorkFacts));
  REQUIRE(after.space.attachment == kDeskSecond);
  REQUIRE(prepared.space.attachment == kDeskFirst);
  REQUIRE(after.space.target.context == prepared.space.target.context);
  REQUIRE(model.pages().at(page).current_navigation == current);
  REQUIRE(model.pages().at(page).activity == lifecycle::Activity::kProtected);

  const PageId temporary{"local-temporary"};
  REQUIRE(model.OpenTemporary(temporary, lifecycle::TemporaryTabId{"local"},
                              kResearchFirst, current) == Status::kOk);
  const auto local = std::get<PagePreparation>(
      PreparePage(model, temporary, kWorkBinding, kWorkFacts));
  REQUIRE(local.space.attachment == kResearchFirst);
  REQUIRE(model.pages().at(temporary).attachment() == kResearchFirst);
}

}  // namespace
}  // namespace arcified::profiles

int main() {
  using namespace arcified::profiles;
  SharedProfileScopesAndExplicitTargets();
  UnreferencedProfilesAndUnattachedSpaces();
  ExactOtrIdentityDoesNotGrantRegularServices();
  MissingFactsAndAttachmentsFailClosed();
  PagePreparationUsesCurrentOwnerWithoutActuation();
  std::cout << "profiles preparation fixtures passed\n";
}
