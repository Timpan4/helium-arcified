#include "inspection.h"

#include <cstdlib>
#include <iostream>

namespace arcified::workspace_organizer {
namespace {

static_assert(lc::kContractVersion == 3);

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const lc::ProfileId kWork{"work"};
const lc::ProfileId kHome{"home"};
const lc::ProfileId kUnused{"unused"};
const lc::SpaceId kDesk{"desk"};
const lc::SpaceId kResearch{"research"};
const lc::SpaceId kPersonal{"personal"};
const lc::SpaceId kOffline{"offline"};
const lc::SpaceId kEmpty{"empty"};
const lc::WindowId kFirst{"first"};
const lc::WindowId kSecond{"second"};
const lc::Attachment kDeskFirst{kFirst, kDesk};
const lc::Attachment kDeskSecond{kSecond, kDesk};
const lc::Attachment kResearchFirst{kFirst, kResearch};
const lc::Attachment kPersonalFirst{kFirst, kPersonal};
const lc::ItemId kKept{"kept"};
const lc::ItemId kStandalone{"standalone"};
const lc::ItemId kClosed{"closed"};
const lc::ShortcutId kAlias{"alias"};
const lc::PageId kSavedPage{"saved-page"};
const lc::PageId kDraftPage{"draft-page"};
const lc::TemporaryTabId kDraft{"draft"};
const lc::SplitId kSplit{"split"};

lc::Model Fixture(const std::string& destination = "https://first.test/saved") {
  lc::Model model;
  REQUIRE(model.AddProfile(kWork) == lc::Status::kOk);
  REQUIRE(model.AddProfile(kHome) == lc::Status::kOk);
  REQUIRE(model.AddProfile(kUnused) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kResearch, kWork) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kPersonal, kHome) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kOffline, kWork) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kEmpty, kWork) == lc::Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == lc::Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == lc::Status::kOk);
  for (const auto& attachment :
       {kDeskFirst, kDeskSecond, kResearchFirst, kPersonalFirst})
    REQUIRE(model.Attach(attachment) == lc::Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kDesk) == lc::Status::kOk);
  REQUIRE(model.AddItem(kKept, kDesk, destination) == lc::Status::kOk);
  REQUIRE(model.AddItem(kStandalone, kWork, destination) == lc::Status::kOk);
  REQUIRE(model.AddItem(kClosed, kOffline, destination) == lc::Status::kOk);
  REQUIRE(model.AddShortcut(kAlias, kWork, kKept) == lc::Status::kOk);
  REQUIRE(std::holds_alternative<lc::OpenedPage>(
      model.OpenItem(kKept, kDeskFirst, kSavedPage)));
  REQUIRE(model.OpenTemporary(kDraftPage, kDraft, kDeskFirst, destination) ==
          lc::Status::kOk);
  REQUIRE(model.SetSplit(kSplit, kDeskFirst, {kSavedPage, kDraftPage}) ==
          lc::Status::kOk);
  REQUIRE(model.ReportActivity(kSavedPage, lc::Activity::kProtected) ==
          lc::Status::kOk);
  return model;
}

void ExpectReference(const Inspection& inspection,
                     Identity source,
                     Relation relation,
                     Identity target) {
  for (const auto& ref : inspection.references)
    if (ref.source == source && ref.relation == relation &&
        ref.target == target)
      return;
  REQUIRE(false);
}

void ExpectSameProjection(const Inspection& a, const Inspection& b) {
  REQUIRE(a.identities == b.identities);
  REQUIRE(a.references.size() == b.references.size());
  for (std::size_t i = 0; i < a.references.size(); ++i) {
    REQUIRE(a.references[i].source == b.references[i].source);
    REQUIRE(a.references[i].relation == b.references[i].relation);
    REQUIRE(a.references[i].target == b.references[i].target);
  }
}

void CompleteIdentityProjection() {
  auto model = Fixture();
  const auto inspection = Inspect(model);
  REQUIRE(inspection.identities.size() == 22);
  REQUIRE(inspection.references.size() == 29);
  for (const auto& identity : std::vector<Identity>{
           kWork,       kHome,       kUnused,        kDesk,          kResearch,
           kPersonal,   kOffline,    kEmpty,         kFirst,         kSecond,
           kDeskFirst,  kDeskSecond, kResearchFirst, kPersonalFirst, kKept,
           kStandalone, kClosed,     kAlias,         kSavedPage,     kDraftPage,
           kDraft,      kSplit})
    REQUIRE(inspection.Contains(identity));
  for (const auto& ref : inspection.references)
    REQUIRE(inspection.Contains(ref.source));
  REQUIRE(inspection.MissingTargets().empty());
  ExpectReference(inspection, kDesk, Relation::kSpaceProfile, kWork);
  ExpectReference(inspection, kDeskFirst, Relation::kAttachmentWindow, kFirst);
  ExpectReference(inspection, kDeskFirst, Relation::kAttachmentSpace, kDesk);
  ExpectReference(inspection, kFirst, Relation::kSelectedSpace, kDesk);
  ExpectReference(inspection, kKept, Relation::kItemOwner, kDesk);
  ExpectReference(inspection, kStandalone, Relation::kItemOwner, kWork);
  ExpectReference(inspection, kAlias, Relation::kShortcutProfile, kWork);
  ExpectReference(inspection, kAlias, Relation::kShortcutTarget, kKept);
  ExpectReference(inspection, kSavedPage, Relation::kPageProfile, kWork);
  ExpectReference(inspection, kSavedPage, Relation::kPageOwner, kKept);
  ExpectReference(inspection, kSavedPage, Relation::kPageAttachment,
                  kDeskFirst);
  ExpectReference(inspection, kDraftPage, Relation::kPageOwner, kDraft);
  ExpectReference(inspection, kDraft, Relation::kTemporaryOrigin, kDeskFirst);
  ExpectReference(inspection, kSplit, Relation::kSplitAttachment, kDeskFirst);
  ExpectReference(inspection, kSplit, Relation::kSplitPage, kSavedPage);
  ExpectReference(inspection, kSplit, Relation::kSplitPage, kDraftPage);
  REQUIRE(inspection.UnattachedSpaces() ==
          std::vector<lc::SpaceId>({kEmpty, kOffline}));
  REQUIRE(inspection.UnreferencedProfiles() ==
          std::vector<lc::ProfileId>({kUnused}));
  // A closed item in an unattached Space still has an explicit owner.
  ExpectReference(inspection, kClosed, Relation::kItemOwner, kOffline);
}

void PayloadIndependenceAndAdvisoryLifetime() {
  auto model = Fixture();
  auto other = Fixture("https://different.test/path?synthetic=payload");
  REQUIRE(other.RecordNavigation(kSavedPage, "https://current.test/private") ==
          lc::Status::kOk);
  const auto before = Inspect(model);
  ExpectSameProjection(before, Inspect(other));
  // Inspect did not reset, close or mutate protected work/navigation/layouts.
  REQUIRE(model.pages().at(kSavedPage).activity == lc::Activity::kProtected);
  REQUIRE(model.pages().at(kSavedPage).current_navigation ==
          "https://first.test/saved");
  REQUIRE(model.splits().at(kSplit).pages ==
          std::vector<lc::PageId>({kSavedPage, kDraftPage}));
  REQUIRE(model.TransferSavedPage(kSavedPage, kDeskSecond) == lc::Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kPersonal) == lc::Status::kOk);
  const auto after = Inspect(model);
  ExpectReference(before, kSavedPage, Relation::kPageAttachment, kDeskFirst);
  ExpectReference(after, kSavedPage, Relation::kPageAttachment, kDeskSecond);
  ExpectReference(after, kFirst, Relation::kSelectedSpace, kPersonal);
  ExpectReference(after, kSavedPage, Relation::kPageProfile, kWork);
  ExpectReference(after, kDraft, Relation::kTemporaryOrigin, kDeskFirst);
  REQUIRE(after.MissingTargets().empty());
}

SidebarLocation Location(const SidebarResult& result) {
  REQUIRE(std::holds_alternative<SidebarLocation>(result));
  return std::get<SidebarLocation>(result);
}

void SidebarIdentityResolution() {
  auto model = Fixture();
  const auto before = Inspect(model);
  auto alias = Location(ResolveSidebar(model, kAlias, kResearchFirst));
  REQUIRE(alias.profile == kWork);
  REQUIRE(alias.item == kKept);
  REQUIRE(alias.shortcut == kAlias);
  REQUIRE(std::get<lc::SpaceId>(alias.owner) == kDesk);
  REQUIRE(alias.location == kDeskFirst);
  REQUIRE(alias.attachment == SidebarAttachment::kAttached);
  REQUIRE(alias.live->page == kSavedPage);
  REQUIRE(alias.live->attachment == kDeskFirst);
  auto remote = Location(ResolveSidebar(model, kKept, kDeskSecond));
  REQUIRE(!remote.shortcut);
  REQUIRE(remote.location == kDeskSecond);
  REQUIRE(remote.live->attachment == kDeskFirst);
  auto closed = Location(ResolveSidebar(model, kClosed, kResearchFirst));
  REQUIRE(closed.location == (lc::Attachment{kFirst, kOffline}));
  REQUIRE(closed.attachment == SidebarAttachment::kRequiresAttachment);
  REQUIRE(!closed.live);
  auto standalone =
      Location(ResolveSidebar(model, kStandalone, kResearchFirst));
  REQUIRE(std::get<lc::ProfileId>(standalone.owner) == kWork);
  REQUIRE(standalone.location == kResearchFirst);
  REQUIRE(!standalone.live);
  REQUIRE(std::get<lc::Status>(ResolveSidebar(model, kKept, kPersonalFirst)) ==
          lc::Status::kProfileMismatch);
  REQUIRE(std::get<lc::Status>(ResolveSidebar(
              model, kKept, lc::Attachment{kSecond, kResearch})) ==
          lc::Status::kNotAttached);
  REQUIRE(std::get<lc::Status>(
              ResolveSidebar(model, lc::ItemId{"missing"}, kDeskFirst)) ==
          lc::Status::kNotFound);
  REQUIRE(std::get<lc::Status>(
              ResolveSidebar(model, lc::ShortcutId{"missing"}, kDeskFirst)) ==
          lc::Status::kNotFound);
  ExpectSameProjection(before, Inspect(model));
  REQUIRE(model.pages().at(kSavedPage).activity == lc::Activity::kProtected);
  REQUIRE(model.RemoveShortcut(kAlias) == lc::Status::kOk);
  REQUIRE(std::get<lc::Status>(ResolveSidebar(model, kAlias, kDeskFirst)) ==
          lc::Status::kNotFound);
  REQUIRE(Location(ResolveSidebar(model, kKept, kDeskFirst)).live->page ==
          kSavedPage);
}

void ConservativeReferencePreviews() {
  auto model = Fixture();
  const auto before = Inspect(model);
  auto deletion = PreviewProfileDeletion(model, kWork);
  REQUIRE(deletion.state == ReferenceState::kReferenced);
  REQUIRE(deletion.external == ExternalReferences::kUninspected);
  REQUIRE(!deletion.references.empty());
  REQUIRE(model.RemoveProfile(kWork) == lc::Status::kReferenced);
  auto spare = PreviewProfileDeletion(model, kUnused);
  REQUIRE(spare.state == ReferenceState::kNoModelReferences);
  REQUIRE(spare.references.empty());
  REQUIRE(spare.external == ExternalReferences::kUninspected);
  REQUIRE(PreviewProfileDeletion(model, lc::ProfileId{"absent"}).state ==
          ReferenceState::kIdentityNotFound);
  auto populated = PreviewSpaceReassignment(model, kOffline, kHome);
  REQUIRE(populated.state == ReassignmentState::kBlockedByModelReferences);
  REQUIRE(populated.populated_migration == PopulatedMigrationPolicy::kOpen);
  REQUIRE(populated.references.size() == 1);
  REQUIRE(std::get<lc::ItemId>(populated.references.front().source) == kClosed);
  REQUIRE(model.ReassignEmptySpace(kOffline, kHome) == lc::Status::kReferenced);
  auto attached = PreviewSpaceReassignment(model, kResearch, kHome);
  REQUIRE(attached.state == ReassignmentState::kBlockedByModelReferences);
  REQUIRE(attached.populated_migration ==
          PopulatedMigrationPolicy::kNotApplicable);
  REQUIRE(model.ReassignEmptySpace(kResearch, kHome) ==
          lc::Status::kReferenced);
  REQUIRE(PreviewSpaceReassignment(model, kDesk, kWork).state ==
          ReassignmentState::kSameProfile);
  REQUIRE(model.ReassignEmptySpace(kDesk, kWork) == lc::Status::kOk);
  auto empty = PreviewSpaceReassignment(model, kEmpty, kHome);
  REQUIRE(empty.state == ReassignmentState::kEmptyUnattached);
  REQUIRE(empty.references.empty());
  REQUIRE(empty.external == ExternalReferences::kUninspected);
  REQUIRE(
      PreviewSpaceReassignment(model, kEmpty, lc::ProfileId{"absent"}).state ==
      ReassignmentState::kIdentityNotFound);
  REQUIRE(PreviewSpaceReassignment(model, lc::SpaceId{"absent"}, kHome).state ==
          ReassignmentState::kIdentityNotFound);
  // Dismissing any preview is just dropping values. No transaction was begun.
  ExpectSameProjection(before, Inspect(model));
  REQUIRE(model.pages().at(kSavedPage).activity == lc::Activity::kProtected);
  // Model guards are the authority, exercised only after advisory checks.
  REQUIRE(model.ReassignEmptySpace(kEmpty, kHome) == lc::Status::kOk);
  REQUIRE(model.RemoveProfile(kUnused) == lc::Status::kOk);
  REQUIRE(PreviewProfileDeletion(model, kUnused).state ==
          ReferenceState::kIdentityNotFound);
}

void EmptyAndMissingReferenceMeaning() {
  lc::Model model;
  REQUIRE(Inspect(model).identities.empty());
  REQUIRE(Inspect(model).UnattachedSpaces().empty());
  // Exercise the query's narrow diagnostic meaning with a synthetic projection,
  // not an invalid Model or a second import/repair path.
  Inspection projection{{kWork}, {{kDesk, Relation::kSpaceProfile, kWork}}};
  REQUIRE(projection.MissingTargets().empty());
  projection.references.push_back({kKept, Relation::kItemOwner, kOffline});
  REQUIRE(projection.MissingTargets().size() == 1);
  REQUIRE(std::get<lc::SpaceId>(projection.MissingTargets().front().target) ==
          kOffline);
}

}  // namespace
}  // namespace arcified::workspace_organizer

int main() {
  using namespace arcified::workspace_organizer;
  CompleteIdentityProjection();
  PayloadIndependenceAndAdvisoryLifetime();
  SidebarIdentityResolution();
  ConservativeReferencePreviews();
  EmptyAndMissingReferenceMeaning();
  std::cout << "workspace organizer synthetic checks passed\n";
}
