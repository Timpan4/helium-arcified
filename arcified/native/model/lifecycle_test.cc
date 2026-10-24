#include "lifecycle.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace arcified::lifecycle {
namespace {

// Synthetic, typed transition fixtures for contract v3. Never real browsing
// data.
constexpr std::uint32_t kFixtureContractVersion = 3;
static_assert(kFixtureContractVersion == kContractVersion);
static_assert(!std::is_convertible_v<ProfileId, SpaceId>);
static_assert(!std::is_convertible_v<PageId, ItemId>);

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
const Attachment kDeskFirst{kFirst, kDesk};
const Attachment kDeskSecond{kSecond, kDesk};
const Attachment kResearchFirst{kFirst, kResearch};
const Attachment kHomeSecond{kSecond, kHome};
const ItemId kMail{"mail"};
const PageId kMailPage{"mail-page"};
const PageId kDraft{"draft"};
const PageId kOther{"other"};
const TemporaryTabId kDraftTab{"draft-tab"};
const ShortcutId kMailShortcut{"mail-shortcut"};
const SplitId kSplit{"layout"};
const std::string kSaved = "https://mail.example.test/inbox";
const std::string kSameSite = "https://mail.example.test/compose";
const std::string kOtherSite = "https://docs.example.test/draft";

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kPersonal) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kResearch, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kHome, kPersonal) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  REQUIRE(model.Attach(kDeskFirst) == Status::kOk);
  REQUIRE(model.Attach(kDeskSecond) == Status::kOk);
  REQUIRE(model.Attach(kResearchFirst) == Status::kOk);
  REQUIRE(model.Attach(kHomeSecond) == Status::kOk);
  return model;
}

void ExpectOpen(const OpenResult& result,
                const PageId& page,
                OpenDisposition disposition) {
  REQUIRE(std::holds_alternative<OpenedPage>(result));
  REQUIRE(std::get<OpenedPage>(result).page == page);
  REQUIRE(std::get<OpenedPage>(result).disposition == disposition);
}

void ExpectError(const OpenResult& result, Status status) {
  REQUIRE(std::holds_alternative<Status>(result));
  REQUIRE(std::get<Status>(result) == status);
}

void OpenMail(Model& model) {
  REQUIRE(model.AddItem(kMail, kDesk, kSaved) == Status::kOk);
  ExpectOpen(model.OpenItem(kMail, kDeskFirst, kMailPage), kMailPage,
             OpenDisposition::kCreated);
}

void OpenDraft(Model& model) {
  REQUIRE(model.OpenTemporary(kDraft, kDraftTab, kDeskFirst, kOtherSite) ==
          Status::kOk);
}

void VersionAndBoundaryFailures() {
  REQUIRE(CheckContractVersion(kFixtureContractVersion) == Status::kOk);
  REQUIRE(CheckContractVersion(0) == Status::kUnsupportedVersion);
  REQUIRE(CheckContractVersion(1) == Status::kUnsupportedVersion);
  REQUIRE(CheckContractVersion(2) == Status::kUnsupportedVersion);
  REQUIRE(CheckContractVersion(kContractVersion + 1) ==
          Status::kUnsupportedVersion);
  auto model = Workspace();
  REQUIRE(model.AddProfile(ProfileId{""}) == Status::kInvalidInput);
  REQUIRE(model.AddProfile(kWork) == Status::kDuplicateId);
  REQUIRE(model.AddSpace(SpaceId{"missing-owner"}, ProfileId{"absent"}) ==
          Status::kNotFound);
  REQUIRE(model.AddItem(kMail, kDesk, "") == Status::kInvalidInput);
  REQUIRE(model.Attach({WindowId{"absent"}, kDesk}) == Status::kNotFound);
  REQUIRE(!model.VisibleItems({WindowId{"absent"}, kDesk}));
  REQUIRE(model.OpenTemporary(kDraft, kDraftTab, {WindowId{"absent"}, kDesk},
                              kSaved) == Status::kNotAttached);
  REQUIRE(model.items().empty());
  REQUIRE(model.pages().empty());
  OpenMail(model);
  OpenDraft(model);
  REQUIRE(model.Keep(kDraft, kMail) == Status::kDuplicateId);
  REQUIRE(model.TemporaryPages(kDeskFirst) == std::vector<PageId>{kDraft});
  REQUIRE(model.items().at(kMail).saved_destination == kSaved);
  REQUIRE(model.pages().at(kDraft).current_navigation == kOtherSite);
}

void SharedSavedAndLocalTemporary() {
  auto model = Workspace();
  OpenMail(model);
  OpenDraft(model);
  REQUIRE(model.VisibleItems(kDeskFirst) == model.VisibleItems(kDeskSecond));
  REQUIRE(*model.VisibleItems(kDeskSecond) == std::vector<ItemId>{kMail});
  REQUIRE(model.VisibleItems(kResearchFirst)->empty());
  REQUIRE(model.VisibleItems(kHomeSecond)->empty());
  REQUIRE(model.TemporaryPages(kDeskFirst) == std::vector<PageId>{kDraft});
  REQUIRE(model.TemporaryPages(kDeskSecond).empty());
  ExpectOpen(model.OpenItem(kMail, kDeskSecond, PageId{"unused"}), kMailPage,
             OpenDisposition::kExisting);
  REQUIRE(model.pages().size() == 2);
  REQUIRE(model.pages().at(kMailPage).attachment() == kDeskSecond);
  REQUIRE(model.Detach(kDeskFirst) == Status::kReferenced);
  REQUIRE(model.Detach(kDeskSecond) == Status::kReferenced);
  ExpectOpen(model.OpenItem(kMail, kDeskFirst, PageId{"unused-again"}),
             kMailPage, OpenDisposition::kExisting);
  REQUIRE(model.Detach(kDeskSecond) == Status::kOk);
  REQUIRE(model.items().count(kMail) == 1);
  REQUIRE(model.pages().at(kDraft).attachment() == kDeskFirst);
  REQUIRE(model.Attach(kDeskSecond) == Status::kOk);
  REQUIRE(*model.VisibleItems(kDeskSecond) == std::vector<ItemId>{kMail});
}

void KeepAndNavigationRemainDistinct() {
  auto model = Workspace();
  OpenDraft(model);
  REQUIRE(model.ReportActivity(kDraft, Activity::kProtected) == Status::kOk);
  REQUIRE(model.Keep(kDraft, kMail) == Status::kOk);
  REQUIRE(model.TemporaryPages(kDeskFirst).empty());
  REQUIRE(model.pages().size() == 1);
  REQUIRE(model.pages().at(kDraft).profile == kWork);
  REQUIRE(model.pages().at(kDraft).activity == Activity::kProtected);
  REQUIRE(std::get<SavedPage>(model.pages().at(kDraft).owner).item == kMail);
  REQUIRE(model.items().at(kMail).saved_destination == kOtherSite);
  REQUIRE(*model.VisibleItems(kDeskSecond) == std::vector<ItemId>{kMail});
  REQUIRE(model.RecordNavigation(kDraft, kSameSite) == Status::kOk);
  REQUIRE(model.pages().at(kDraft).current_navigation == kSameSite);
  REQUIRE(model.items().at(kMail).saved_destination == kOtherSite);
  REQUIRE(model.RecordNavigation(kDraft, kSaved) == Status::kOk);
  REQUIRE(model.pages().at(kDraft).current_navigation == kSaved);
  REQUIRE(model.items().at(kMail).saved_destination == kOtherSite);
  REQUIRE(model.Reset(kDraft) == Status::kWorkNotQuiescent);
  REQUIRE(model.ReportActivity(kDraft, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.Reset(kDraft) == Status::kOk);
  REQUIRE(model.pages().at(kDraft).current_navigation == kOtherSite);
  REQUIRE(model.pages().at(kDraft).activity == Activity::kUnknown);
  REQUIRE(model.ClosePage(kDraft) == Status::kWorkNotQuiescent);
  REQUIRE(model.ReportActivity(kDraft, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kDraft) == Status::kOk);
  REQUIRE(model.items().count(kMail) == 1);
  ExpectOpen(model.OpenItem(kMail, kDeskSecond, kMailPage), kMailPage,
             OpenDisposition::kCreated);
  REQUIRE(model.pages().at(kMailPage).current_navigation == kOtherSite);
  REQUIRE(model.pages().at(kMailPage).attachment() == kDeskSecond);
}

void ProfileShortcutsAreReferences() {
  auto model = Workspace();
  OpenMail(model);
  const ShortcutId replacement{"replacement-shortcut"};
  REQUIRE(model.AddShortcut(replacement, kWork, kMail) == Status::kOk);
  REQUIRE(*model.VisibleShortcuts(kResearchFirst) ==
          std::vector<ShortcutId>{replacement});
  REQUIRE(model.VisibleShortcuts(kDeskFirst) ==
          model.VisibleShortcuts(kDeskSecond));
  REQUIRE(model.VisibleShortcuts(kHomeSecond)->empty());
  REQUIRE(!model.VisibleShortcuts({WindowId{"absent"}, kDesk}));
  REQUIRE(model.AddShortcut(ShortcutId{"leak"}, kPersonal, kMail) ==
          Status::kProfileMismatch);
  REQUIRE(model.RecordNavigation(kMailPage, kSameSite) == Status::kOk);
  ExpectOpen(model.OpenShortcut(replacement, kResearchFirst, kOther), kMailPage,
             OpenDisposition::kExisting);
  REQUIRE(model.pages().size() == 1);
  REQUIRE(model.pages().at(kMailPage).current_navigation == kSameSite);
  REQUIRE(model.pages().at(kMailPage).attachment() == kDeskFirst);
  REQUIRE(model.RemoveShortcut(replacement) == Status::kOk);
  REQUIRE(model.items().count(kMail) == 1);
  REQUIRE(model.pages().count(kMailPage) == 1);
  REQUIRE(model.AddShortcut(kMailShortcut, kWork, kMail) == Status::kOk);
  REQUIRE(model.ReportActivity(kMailPage, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kMailPage) == Status::kOk);
  REQUIRE(model.RemoveItem(kMail) == Status::kReferenced);
  REQUIRE(model.Detach(kDeskFirst) == Status::kOk);
  ExpectError(model.OpenShortcut(kMailShortcut, kResearchFirst, kOther),
              Status::kNotAttached);
  REQUIRE(model.pages().empty());
  REQUIRE(model.Attach(kDeskFirst) == Status::kOk);
  ExpectOpen(model.OpenShortcut(kMailShortcut, kResearchFirst, kOther), kOther,
             OpenDisposition::kCreated);
  REQUIRE(model.pages().at(kOther).attachment() == kDeskFirst);
  REQUIRE(model.pages().at(kOther).current_navigation == kSaved);

  const ItemId profile_item{"profile-entry"};
  REQUIRE(model.AddItem(profile_item, kWork, kOtherSite) == Status::kOk);
  REQUIRE(*model.VisibleItems(kResearchFirst) ==
          std::vector<ItemId>{profile_item});
  REQUIRE(model.VisibleItems(kHomeSecond)->empty());
  ExpectError(model.OpenItem(profile_item, kHomeSecond, PageId{"wrong"}),
              Status::kProfileMismatch);
  ExpectOpen(
      model.OpenItem(profile_item, kResearchFirst, PageId{"profile-page"}),
      PageId{"profile-page"}, OpenDisposition::kCreated);
}

void SplitReferencesAndCloseProtection() {
  auto model = Workspace();
  OpenMail(model);
  OpenDraft(model);
  REQUIRE(model.SetSplit(kSplit, kDeskFirst, {kMailPage, kDraft}) ==
          Status::kOk);
  REQUIRE(model.SetSplit(kSplit, kDeskFirst, {kMailPage, kMailPage}) ==
          Status::kInvalidInput);
  REQUIRE(model.SetSplit(kSplit, kDeskSecond, {kMailPage, kDraft}) ==
          Status::kInvalidInput);
  REQUIRE(model.SetSplit(kSplit, kDeskFirst, {PageId{"missing"}}) ==
          Status::kNotFound);
  REQUIRE(model.RecordNavigation(kMailPage, kSameSite) == Status::kOk);
  REQUIRE(model.splits().at(kSplit).pages ==
          (std::vector<PageId>{kMailPage, kDraft}));
  REQUIRE(model.pages().size() == 2);
  const SplitId other_layout{"other-layout"};
  REQUIRE(model.SetSplit(other_layout, kDeskFirst, {kDraft, kMailPage}) ==
          Status::kOk);
  REQUIRE(model.RemoveSplit(other_layout) == Status::kOk);
  REQUIRE(model.pages().size() == 2);
  REQUIRE(model.pages().at(kMailPage).current_navigation == kSameSite);
  const SplitId shared_layout{"shared-layout"};
  REQUIRE(model.SetSplit(shared_layout, kDeskFirst, {kDraft, kMailPage}) ==
          Status::kOk);
  REQUIRE(model.ClosePage(kMailPage) == Status::kWorkNotQuiescent);
  REQUIRE(model.ReportActivity(kMailPage, Activity::kProtected) == Status::kOk);
  REQUIRE(model.Reset(kMailPage) == Status::kWorkNotQuiescent);
  REQUIRE(model.pages().at(kMailPage).current_navigation == kSameSite);
  REQUIRE(model.ReportActivity(kMailPage, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kMailPage) == Status::kOk);
  REQUIRE(model.splits().at(kSplit).pages == std::vector<PageId>{kDraft});
  REQUIRE(model.splits().at(shared_layout).pages ==
          std::vector<PageId>{kDraft});
  REQUIRE(model.items().count(kMail) == 1);
  REQUIRE(model.ReportActivity(kDraft, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kDraft) == Status::kOk);
  REQUIRE(model.splits().empty());
  REQUIRE(model.items().count(kMail) == 1);
}

void WindowCloseIsAtomicAndLocal() {
  auto model = Workspace();
  OpenMail(model);
  OpenDraft(model);
  REQUIRE(model.OpenTemporary(kOther, TemporaryTabId{"second-tab"}, kDeskSecond,
                              kOtherSite) == Status::kOk);
  REQUIRE(model.SetSplit(kSplit, kDeskFirst, {kMailPage, kDraft}) ==
          Status::kOk);
  REQUIRE(model.ReportActivity(kMailPage, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.CloseWindow(kFirst) == Status::kWorkNotQuiescent);
  REQUIRE(model.windows().count(kFirst) == 1);
  REQUIRE(model.pages().size() == 3);
  REQUIRE(model.attachments().count(kDeskFirst) == 1);
  REQUIRE(model.splits().at(kSplit).pages ==
          (std::vector<PageId>{kMailPage, kDraft}));
  REQUIRE(model.ReportActivity(kDraft, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.CloseWindow(kFirst) == Status::kOk);
  REQUIRE(model.windows().count(kFirst) == 0);
  REQUIRE(model.pages().size() == 1);
  REQUIRE(model.pages().at(kOther).attachment() == kDeskSecond);
  REQUIRE(model.splits().empty());
  REQUIRE(model.attachments().count(kDeskFirst) == 0);
  REQUIRE(model.attachments().count(kResearchFirst) == 0);
  REQUIRE(model.attachments().count(kDeskSecond) == 1);
  REQUIRE(*model.VisibleItems(kDeskSecond) == std::vector<ItemId>{kMail});
  ExpectOpen(model.OpenItem(kMail, kDeskSecond, PageId{"reopened"}),
             PageId{"reopened"}, OpenDisposition::kCreated);
  REQUIRE(model.pages().at(PageId{"reopened"}).current_navigation == kSaved);
}

void CrossProfileReopenNeverRelabels() {
  auto model = Workspace();
  OpenMail(model);
  REQUIRE(model.RecordNavigation(kMailPage, kSameSite) == Status::kOk);
  REQUIRE(model.MoveItem(kMail, kHome) == Status::kReopenRequired);
  REQUIRE(model.MoveItem(kMail, kResearch) == Status::kReferenced);
  ExpectError(model.OpenItem(kMail, kHomeSecond, kOther),
              Status::kProfileMismatch);
  REQUIRE(model.ReassignEmptySpace(kDesk, kPersonal) == Status::kReferenced);
  const ItemId copy{"personal-copy"};
  REQUIRE(model.CopySavedDestination(kMail, copy, kHome) == Status::kOk);
  REQUIRE(model.items().at(copy).saved_destination == kSaved);
  ExpectOpen(model.OpenItem(copy, kHomeSecond, kOther), kOther,
             OpenDisposition::kCreated);
  REQUIRE(model.pages().at(kOther).profile == kPersonal);
  REQUIRE(model.pages().at(kOther).current_navigation == kSaved);
  REQUIRE(model.pages().at(kMailPage).profile == kWork);
  REQUIRE(model.pages().at(kMailPage).current_navigation == kSameSite);
  REQUIRE(std::get<SpaceId>(model.items().at(kMail).owner) == kDesk);
  REQUIRE(model.RemoveProfile(kWork) == Status::kReferenced);
  REQUIRE(model.ReportActivity(kMailPage, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kMailPage) == Status::kOk);
  REQUIRE(model.MoveItem(kMail, kHome) == Status::kReopenRequired);
  REQUIRE(model.MoveItem(kMail, kResearch) == Status::kOk);
  REQUIRE(std::get<SpaceId>(model.items().at(kMail).owner) == kResearch);
  REQUIRE(model.items().at(kMail).saved_destination == kSaved);
  ExpectOpen(model.OpenItem(kMail, kResearchFirst, PageId{"moved"}),
             PageId{"moved"}, OpenDisposition::kCreated);
  REQUIRE(model.pages().at(PageId{"moved"}).profile == kWork);
}

void ReferenceSafeDeletionAndReassignment() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kMail, kDesk, kSaved) == Status::kOk);
  REQUIRE(model.AddShortcut(kMailShortcut, kWork, kMail) == Status::kOk);
  REQUIRE(model.RemoveItem(kMail) == Status::kReferenced);
  REQUIRE(model.RemoveSpace(kDesk) == Status::kReferenced);
  REQUIRE(model.RemoveProfile(kWork) == Status::kReferenced);
  REQUIRE(model.RemoveShortcut(kMailShortcut) == Status::kOk);
  REQUIRE(model.RemoveItem(kMail) == Status::kOk);
  REQUIRE(model.ReassignEmptySpace(kDesk, kPersonal) == Status::kReferenced);
  REQUIRE(model.Detach(kDeskFirst) == Status::kOk);
  REQUIRE(model.Detach(kDeskSecond) == Status::kOk);
  REQUIRE(model.ReassignEmptySpace(kDesk, kPersonal) == Status::kOk);
  REQUIRE(model.spaces().at(kDesk).profile == kPersonal);
  REQUIRE(model.RemoveSpace(kDesk) == Status::kOk);
  REQUIRE(model.Detach(kResearchFirst) == Status::kOk);
  REQUIRE(model.RemoveSpace(kResearch) == Status::kOk);
  const ItemId profile_item{"standalone"};
  REQUIRE(model.AddItem(profile_item, kWork, kSaved) == Status::kOk);
  REQUIRE(model.RemoveProfile(kWork) == Status::kReferenced);
  REQUIRE(model.RemoveItem(profile_item) == Status::kOk);
  REQUIRE(model.RemoveProfile(kWork) == Status::kOk);
  REQUIRE(model.profiles().count(kPersonal) == 1);
  REQUIRE(model.spaces().at(kHome).profile == kPersonal);
}

void RequireValidSplitReferences(const Model& model) {
  for (const auto& [id, split] : model.splits()) {
    REQUIRE(!split.pages.empty());
    for (const auto& page : split.pages) {
      REQUIRE(model.pages().count(page) == 1);
      REQUIRE(model.pages().at(page).attachment() == split.attachment);
    }
  }
}

void BorrowedPageCloseOwnsCommandId() {
  auto model = Workspace();
  OpenMail(model);
  OpenDraft(model);
  const SplitId single{"single-reference"};
  REQUIRE(model.SetSplit(kSplit, kDeskFirst, {kMailPage, kDraft}) ==
          Status::kOk);
  REQUIRE(model.SetSplit(single, kDeskFirst, {kMailPage}) == Status::kOk);
  REQUIRE(model.ReportActivity(kMailPage, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ReportActivity(kDraft, Activity::kProtected) == Status::kOk);
  REQUIRE(model.ClosePage(model.splits().at(kSplit).pages.front()) ==
          Status::kOk);
  REQUIRE(model.pages().count(kMailPage) == 0);
  REQUIRE(model.pages().count(kDraft) == 1);
  REQUIRE(model.pages().at(kDraft).activity == Activity::kProtected);
  REQUIRE(model.pages().at(kDraft).current_navigation == kOtherSite);
  REQUIRE(model.splits().count(single) == 0);
  REQUIRE(model.splits().at(kSplit).pages == std::vector<PageId>{kDraft});
  RequireValidSplitReferences(model);
  REQUIRE(model.ClosePage(model.splits().at(kSplit).pages.front()) ==
          Status::kWorkNotQuiescent);
  REQUIRE(model.ReportActivity(kDraft, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(model.splits().at(kSplit).pages.front()) ==
          Status::kOk);
  REQUIRE(model.pages().empty());
  REQUIRE(model.splits().empty());
  REQUIRE(model.items().count(kMail) == 1);
}

void BorrowedWindowCloseOwnsCommandId() {
  auto model = Workspace();
  OpenMail(model);
  OpenDraft(model);
  REQUIRE(model.OpenTemporary(kOther, TemporaryTabId{"second-tab"}, kDeskSecond,
                              kOtherSite) == Status::kOk);
  REQUIRE(model.SetSplit(kSplit, kDeskFirst, {kMailPage, kDraft}) ==
          Status::kOk);
  REQUIRE(model.ReportActivity(kMailPage, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.CloseWindow(model.pages().at(kMailPage).attachment().window) ==
          Status::kWorkNotQuiescent);
  REQUIRE(model.pages().size() == 3);
  REQUIRE(model.windows().count(kFirst) == 1);
  RequireValidSplitReferences(model);
  REQUIRE(model.ReportActivity(kDraft, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.CloseWindow(model.pages().at(kMailPage).attachment().window) ==
          Status::kOk);
  REQUIRE(model.windows().count(kFirst) == 0);
  REQUIRE(model.pages().size() == 1);
  REQUIRE(model.pages().at(kOther).attachment() == kDeskSecond);
  REQUIRE(model.splits().empty());
  REQUIRE(model.attachments().count(kDeskFirst) == 0);
  REQUIRE(model.attachments().count(kResearchFirst) == 0);
  REQUIRE(model.attachments().count(kDeskSecond) == 1);
  REQUIRE(model.items().count(kMail) == 1);
  REQUIRE(model.ReportActivity(kOther, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.CloseWindow(model.attachments().find(kDeskSecond)->window) ==
          Status::kOk);
  REQUIRE(model.windows().empty());
  REQUIRE(model.attachments().empty());
  REQUIRE(model.pages().empty());
}

void BorrowedMutationInputs() {
  auto model = Workspace();
  OpenMail(model);
  REQUIRE(model.SetSplit(kSplit, kDeskFirst, {kMailPage}) == Status::kOk);
  REQUIRE(model.SetSplit(model.splits().begin()->first,
                         model.splits().at(kSplit).attachment,
                         model.splits().at(kSplit).pages) == Status::kOk);
  RequireValidSplitReferences(model);
  REQUIRE(model.RemoveSplit(model.splits().begin()->first) == Status::kOk);
  REQUIRE(model.AddShortcut(kMailShortcut, kWork, kMail) == Status::kOk);
  REQUIRE(model.RemoveShortcut(model.shortcuts().begin()->first) ==
          Status::kOk);
  REQUIRE(model.ReportActivity(model.pages().begin()->first,
                               Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(model.pages().begin()->first) == Status::kOk);
  REQUIRE(model.MoveItem(model.items().begin()->first, kResearch) ==
          Status::kOk);
  REQUIRE(model.RemoveItem(model.items().begin()->first) == Status::kOk);
  REQUIRE(model.Detach(*model.attachments().find(kDeskFirst)) == Status::kOk);
  REQUIRE(model.Detach(*model.attachments().find(kDeskSecond)) == Status::kOk);
  REQUIRE(model.ReassignEmptySpace(model.spaces().find(kDesk)->first,
                                   model.spaces().at(kHome).profile) ==
          Status::kOk);
  REQUIRE(model.RemoveSpace(model.spaces().find(kDesk)->first) == Status::kOk);
  REQUIRE(model.Detach(*model.attachments().find(kResearchFirst)) ==
          Status::kOk);
  REQUIRE(model.RemoveSpace(model.spaces().find(kResearch)->first) ==
          Status::kOk);
  REQUIRE(model.RemoveProfile(model.profiles().find(kWork)->first) ==
          Status::kOk);
  REQUIRE(model.profiles().count(kPersonal) == 1);
}

void SavedSelectionTransfersLiveState() {
  auto model = Workspace();
  OpenMail(model);
  OpenDraft(model);
  REQUIRE(model.RecordNavigation(kMailPage, kSameSite) == Status::kOk);
  REQUIRE(model.ReportActivity(kMailPage, Activity::kProtected) == Status::kOk);
  REQUIRE(model.ReportActivity(kDraft, Activity::kProtected) == Status::kOk);
  const SplitId single{"single-reference"};
  REQUIRE(model.SetSplit(kSplit, kDeskFirst, {kMailPage, kDraft}) ==
          Status::kOk);
  REQUIRE(model.SetSplit(single, kDeskFirst, {kMailPage}) == Status::kOk);
  // Same-attachment selection preserves layouts and ignores the unused ID.
  ExpectOpen(
      model.OpenItem(kMail, model.pages().at(kMailPage).attachment(), kDraft),
      kMailPage, OpenDisposition::kExisting);
  REQUIRE(model.splits().at(kSplit).pages.size() == 2);
  REQUIRE(model.splits().count(single) == 1);
  // Rejected transfers leave the complete source reference graph intact.
  REQUIRE(model.TransferSavedPage(kMailPage, kHomeSecond) ==
          Status::kProfileMismatch);
  REQUIRE(model.TransferSavedPage(kMailPage, kResearchFirst) ==
          Status::kInvalidInput);
  REQUIRE(model.TransferSavedPage(kMailPage, {WindowId{"absent"}, kDesk}) ==
          Status::kNotAttached);
  REQUIRE(model.TransferSavedPage(kDraft, kDeskSecond) ==
          Status::kInvalidInput);
  REQUIRE(model.splits().at(kSplit).pages ==
          (std::vector<PageId>{kMailPage, kDraft}));
  REQUIRE(model.splits().count(single) == 1);
  RequireValidSplitReferences(model);
  ExpectOpen(model.OpenItem(kMail, kDeskSecond, PageId{"unused"}), kMailPage,
             OpenDisposition::kExisting);
  REQUIRE(model.pages().size() == 2);
  REQUIRE(model.pages().at(kMailPage).attachment() == kDeskSecond);
  REQUIRE(model.pages().at(kMailPage).profile == kWork);
  REQUIRE(model.pages().at(kMailPage).current_navigation == kSameSite);
  REQUIRE(model.pages().at(kMailPage).activity == Activity::kProtected);
  REQUIRE(model.items().at(kMail).saved_destination == kSaved);
  REQUIRE(model.splits().at(kSplit).pages == std::vector<PageId>{kDraft});
  REQUIRE(model.splits().count(single) == 0);
  REQUIRE(model.pages().at(kDraft).attachment() == kDeskFirst);
  REQUIRE(model.TemporaryPages(kDeskSecond).empty());
  RequireValidSplitReferences(model);
  // A direct transfer also snapshots IDs borrowed from a compacted layout.
  REQUIRE(model.SetSplit(single, kDeskSecond, {kMailPage}) == Status::kOk);
  REQUIRE(model.TransferSavedPage(model.splits().at(single).pages.front(),
                                  kDeskFirst) == Status::kOk);
  REQUIRE(model.splits().count(single) == 0);
  REQUIRE(model.pages().at(kMailPage).attachment() == kDeskFirst);
  REQUIRE(model.pages().at(kMailPage).current_navigation == kSameSite);
  REQUIRE(model.pages().at(kMailPage).activity == Activity::kProtected);
  RequireValidSplitReferences(model);
  REQUIRE(model.AddShortcut(kMailShortcut, kWork, kMail) == Status::kOk);
  const Attachment research_second{kSecond, kResearch};
  REQUIRE(model.Attach(research_second) == Status::kOk);
  REQUIRE(model.Detach(kDeskSecond) == Status::kOk);
  ExpectError(model.OpenShortcut(kMailShortcut, research_second, kOther),
              Status::kNotAttached);
  REQUIRE(model.pages().at(kMailPage).attachment() == kDeskFirst);
  REQUIRE(model.Attach(kDeskSecond) == Status::kOk);
  ExpectOpen(model.OpenShortcut(kMailShortcut, research_second, kOther),
             kMailPage, OpenDisposition::kExisting);
  REQUIRE(model.pages().at(kMailPage).attachment() == kDeskSecond);
  REQUIRE(model.pages().at(kMailPage).current_navigation == kSameSite);
  REQUIRE(model.pages().at(kMailPage).activity == Activity::kProtected);

  const ItemId profile_item{"profile-entry"};
  REQUIRE(model.AddItem(profile_item, kWork, kOtherSite) == Status::kOk);
  ExpectOpen(model.OpenItem(profile_item, kResearchFirst, kOther), kOther,
             OpenDisposition::kCreated);
  REQUIRE(model.SetSplit(single, kResearchFirst, {kOther}) == Status::kOk);
  ExpectOpen(model.OpenItem(profile_item, kDeskSecond, PageId{"unused"}),
             kOther, OpenDisposition::kExisting);
  REQUIRE(model.pages().at(kOther).attachment() == kDeskSecond);
  REQUIRE(model.pages().at(kOther).profile == kWork);
  REQUIRE(model.pages().at(kOther).current_navigation == kOtherSite);
  REQUIRE(model.pages().at(kOther).activity == Activity::kUnknown);
  REQUIRE(model.splits().count(single) == 0);
  RequireValidSplitReferences(model);
}

void SpaceSelectionKeepsWindowAndProfiles() {
  auto model = Workspace();
  OpenMail(model);
  OpenDraft(model);
  const Attachment home_first{kFirst, kHome};
  REQUIRE(model.Attach(home_first) == Status::kOk);
  REQUIRE(model.OpenTemporary(kOther, TemporaryTabId{"personal-tab"},
                              home_first, kSaved) == Status::kOk);
  REQUIRE(!model.windows().at(kFirst).selected_space);
  REQUIRE(model.SelectSpace(kFirst, kDesk) == Status::kOk);
  REQUIRE(model.SelectSpace(model.pages().at(kMailPage).attachment().window,
                            model.pages().at(kOther).attachment().space) ==
          Status::kOk);
  REQUIRE(model.windows().size() == 2);
  REQUIRE(model.windows().at(kFirst).selected_space == kHome);
  REQUIRE(!model.windows().at(kSecond).selected_space);
  REQUIRE(model.pages().at(kOther).profile == kPersonal);
  REQUIRE(model.pages().at(kMailPage).profile == kWork);
  REQUIRE(model.pages().at(kMailPage).attachment() == kDeskFirst);
  REQUIRE(model.pages().at(kDraft).attachment() == kDeskFirst);
  REQUIRE(model.TemporaryPages(home_first) == std::vector<PageId>{kOther});
  REQUIRE(model.TemporaryPages(kDeskFirst) == std::vector<PageId>{kDraft});
  REQUIRE(model.VisibleItems(home_first)->empty());
  ExpectError(model.OpenItem(kMail, home_first, PageId{"wrong-profile"}),
              Status::kProfileMismatch);
  REQUIRE(model.windows().at(kFirst).selected_space == kHome);
  REQUIRE(model.SelectSpace(kFirst, kResearch) == Status::kOk);
  REQUIRE(model.windows().at(kFirst).selected_space == kResearch);
  REQUIRE(model.SelectSpace(kSecond, kResearch) == Status::kNotAttached);
  REQUIRE(!model.windows().at(kSecond).selected_space);
  REQUIRE(model.SelectSpace(WindowId{"absent"}, kDesk) == Status::kNotFound);
  REQUIRE(model.SelectSpace(kFirst, SpaceId{"absent"}) == Status::kNotFound);
  // Detach may borrow the selected Space ID through its attachment input.
  REQUIRE(model.Detach({kFirst, *model.windows().at(kFirst).selected_space}) ==
          Status::kOk);
  REQUIRE(!model.windows().at(kFirst).selected_space);
  REQUIRE(model.pages().at(kOther).profile == kPersonal);
  REQUIRE(model.pages().at(kMailPage).profile == kWork);
}

void OutboundLinkBranchesWithoutTouchingSource() {
  for (const auto activity : {Activity::kUnknown, Activity::kProtected}) {
    auto model = Workspace();
    OpenMail(model);
    OpenDraft(model);
    REQUIRE(model.RecordNavigation(kMailPage, kSameSite) == Status::kOk);
    REQUIRE(model.ReportActivity(kMailPage, activity) == Status::kOk);
    REQUIRE(model.SetSplit(kSplit, kDeskFirst, {kMailPage, kDraft}) ==
            Status::kOk);
    const auto result = model.PlanSavedPageLink(
        model.pages().find(kMailPage)->first, NavigationKind::kLink,
        SiteRelation::kOutbound, model.pages().at(kDraft).current_navigation);
    REQUIRE(std::holds_alternative<TemporaryBranchPlan>(result));
    const auto& plan = std::get<TemporaryBranchPlan>(result);
    REQUIRE(plan.source == kMailPage);
    REQUIRE(plan.profile == kWork);
    REQUIRE(plan.attachment == kDeskFirst);
    REQUIRE(plan.destination == kOtherSite);
    // Planning, including a cancelled/failed native attempt, records nothing.
    REQUIRE(model.pages().size() == 2);
    REQUIRE(model.TemporaryPages(kDeskFirst) == std::vector<PageId>{kDraft});
    // Simulated native success is reported separately with fresh identities.
    const TemporaryTabId branch_tab{"outbound-tab"};
    REQUIRE(model.OpenTemporary(kOther, branch_tab, plan.attachment,
                                plan.destination) == Status::kOk);
    REQUIRE(model.pages().at(kOther).profile == kWork);
    REQUIRE(model.pages().at(kOther).attachment() == kDeskFirst);
    REQUIRE(model.pages().at(kOther).current_navigation == kOtherSite);
    REQUIRE(model.pages().at(kOther).activity == Activity::kUnknown);
    REQUIRE(std::get<TemporaryTab>(model.pages().at(kOther).owner).id ==
            branch_tab);
    REQUIRE(model.TemporaryPages(kDeskSecond).empty());
    REQUIRE(model.pages().at(kMailPage).current_navigation == kSameSite);
    REQUIRE(model.pages().at(kMailPage).profile == kWork);
    REQUIRE(model.pages().at(kMailPage).activity == activity);
    REQUIRE(model.pages().at(kMailPage).attachment() == kDeskFirst);
    REQUIRE(std::get<SavedPage>(model.pages().at(kMailPage).owner).item ==
            kMail);
    REQUIRE(model.items().at(kMail).saved_destination == kSaved);
    REQUIRE(model.items().size() == 1);
    REQUIRE(model.splits().at(kSplit).pages ==
            (std::vector<PageId>{kMailPage, kDraft}));
    REQUIRE(model.ClosePage(kMailPage) == Status::kWorkNotQuiescent);
    REQUIRE(model.Reset(kMailPage) == Status::kWorkNotQuiescent);
    RequireValidSplitReferences(model);
  }
}

void LinkRoutingRefusesUnapprovedOrUnknownInputs() {
  auto model = Workspace();
  OpenMail(model);
  OpenDraft(model);
  REQUIRE(model.ReportActivity(kMailPage, Activity::kProtected) == Status::kOk);
  REQUIRE(model.SetSplit(kSplit, kDeskFirst, {kMailPage, kDraft}) ==
          Status::kOk);
  const auto expect_refusal = [](const LinkRoutingResult& result,
                                 LinkRoutingRefusal expected) {
    REQUIRE(std::holds_alternative<LinkRoutingRefusal>(result));
    REQUIRE(std::get<LinkRoutingRefusal>(result) == expected);
  };
  for (const auto kind : {NavigationKind::kRedirect, NavigationKind::kScript,
                          NavigationKind::kTyped, NavigationKind::kOther}) {
    expect_refusal(model.PlanSavedPageLink(kMailPage, kind,
                                           SiteRelation::kOutbound, kOtherSite),
                   LinkRoutingRefusal::kOutsideApprovedScope);
  }
  // Identical destinations yield different decisions from caller
  // classification.
  expect_refusal(model.PlanSavedPageLink(kMailPage, NavigationKind::kLink,
                                         SiteRelation::kUnknown, kOtherSite),
                 LinkRoutingRefusal::kClassificationRequired);
  expect_refusal(model.PlanSavedPageLink(kMailPage, NavigationKind::kLink,
                                         SiteRelation::kSameSite, kOtherSite),
                 LinkRoutingRefusal::kSameSitePolicyOpen);
  expect_refusal(model.PlanSavedPageLink(kDraft, NavigationKind::kLink,
                                         SiteRelation::kOutbound, kOtherSite),
                 LinkRoutingRefusal::kSourceNotSaved);
  expect_refusal(
      model.PlanSavedPageLink(PageId{"absent"}, NavigationKind::kLink,
                              SiteRelation::kOutbound, kOtherSite),
      LinkRoutingRefusal::kSourceNotFound);
  expect_refusal(model.PlanSavedPageLink(kMailPage, NavigationKind::kLink,
                                         SiteRelation::kOutbound, ""),
                 LinkRoutingRefusal::kInvalidDestination);
  REQUIRE(model.pages().size() == 2);
  REQUIRE(model.items().size() == 1);
  REQUIRE(model.items().at(kMail).saved_destination == kSaved);
  REQUIRE(model.pages().at(kMailPage).current_navigation == kSaved);
  REQUIRE(model.pages().at(kMailPage).activity == Activity::kProtected);
  REQUIRE(model.pages().at(kMailPage).profile == kWork);
  REQUIRE(model.pages().at(kMailPage).attachment() == kDeskFirst);
  REQUIRE(model.TemporaryPages(kDeskFirst) == std::vector<PageId>{kDraft});
  REQUIRE(model.splits().at(kSplit).pages ==
          (std::vector<PageId>{kMailPage, kDraft}));
  RequireValidSplitReferences(model);
}

void LinkPlanSnapshotsProfileShortcutAndCurrentAttachment() {
  auto model = Workspace();
  const ItemId item{"profile-favorite"};
  REQUIRE(model.AddItem(item, kWork, kSaved) == Status::kOk);
  ExpectOpen(model.OpenItem(item, kResearchFirst, kMailPage), kMailPage,
             OpenDisposition::kCreated);
  const auto initial = model.PlanSavedPageLink(
      kMailPage, NavigationKind::kLink, SiteRelation::kOutbound, kOtherSite);
  REQUIRE(std::holds_alternative<TemporaryBranchPlan>(initial));
  REQUIRE(std::get<TemporaryBranchPlan>(initial).attachment == kResearchFirst);
  REQUIRE(model.TransferSavedPage(kMailPage, kDeskSecond) == Status::kOk);
  // A snapshot is not a live reference or authorization to commit stale work.
  REQUIRE(std::get<TemporaryBranchPlan>(initial).attachment == kResearchFirst);
  const auto refreshed = model.PlanSavedPageLink(
      model.pages().begin()->first, NavigationKind::kLink,
      SiteRelation::kOutbound, model.items().at(item).saved_destination);
  REQUIRE(std::holds_alternative<TemporaryBranchPlan>(refreshed));
  const auto& plan = std::get<TemporaryBranchPlan>(refreshed);
  REQUIRE(plan.source == kMailPage);
  REQUIRE(plan.profile == kWork);
  REQUIRE(plan.attachment == kDeskSecond);
  REQUIRE(plan.destination == kSaved);
  REQUIRE(model.pages().size() == 1);
  REQUIRE(model.pages().at(kMailPage).activity == Activity::kUnknown);
  REQUIRE(model.items().at(item).owner == ItemOwner{kWork});
  REQUIRE(model.TemporaryPages(kDeskSecond).empty());
}

}  // namespace
}  // namespace arcified::lifecycle

int main() {
  using namespace arcified::lifecycle;
  const std::pair<const char*, void (*)()> fixtures[] = {
      {"v3/version-and-boundaries", VersionAndBoundaryFailures},
      {"v3/shared-saved-local-temporary", SharedSavedAndLocalTemporary},
      {"v3/keep-navigation-reset-reopen", KeepAndNavigationRemainDistinct},
      {"v3/profile-shortcuts", ProfileShortcutsAreReferences},
      {"v3/split-references", SplitReferencesAndCloseProtection},
      {"v3/window-close", WindowCloseIsAtomicAndLocal},
      {"v3/cross-profile-reopen", CrossProfileReopenNeverRelabels},
      {"v3/deletion-reassignment", ReferenceSafeDeletionAndReassignment},
      {"v3/borrowed-page-close", BorrowedPageCloseOwnsCommandId},
      {"v3/borrowed-window-close", BorrowedWindowCloseOwnsCommandId},
      {"v3/borrowed-mutation-inputs", BorrowedMutationInputs},
      {"v3/saved-selection-transfer", SavedSelectionTransfersLiveState},
      {"v3/space-selection-profile-isolation",
       SpaceSelectionKeepsWindowAndProfiles},
      {"v3/outbound-link-branch", OutboundLinkBranchesWithoutTouchingSource},
      {"v3/link-routing-refusals", LinkRoutingRefusesUnapprovedOrUnknownInputs},
      {"v3/link-plan-profile-and-attachment",
       LinkPlanSnapshotsProfileShortcutAndCurrentAttachment},
  };
  for (const auto& [name, run] : fixtures) {
    run();
    std::cout << "PASS " << name << '\n';
  }
}
