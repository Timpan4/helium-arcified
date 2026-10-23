#include "space_management.h"

#include <cstdlib>
#include <iostream>
#include <utility>

namespace arcified::space_management {
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
const SpaceId kGhost{"ghost"};
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const Attachment kDeskFirst{kFirst, kDesk};
const Attachment kDeskSecond{kSecond, kDesk};
const Attachment kResearchFirst{kFirst, kResearch};
const Attachment kHomeFirst{kFirst, kHome};
const Attachment kHomeSecond{kSecond, kHome};
const ItemId kKept{"kept"};
const ItemId kClosed{"closed"};
const ItemId kOtherKept{"other-kept"};
const ItemId kProfileItem{"profile-item"};
const PageId kSavedPage{"saved-page"};
const PageId kDraft{"draft"};
const PageId kOtherDraft{"other-draft"};
const PageId kProfilePage{"profile-page"};
const ShortcutId kAlias{"alias"};
const ShortcutId kOtherAlias{"other-alias"};
const SplitId kSplit{"split"};
const std::string kDestination = "https://saved.example.test/start";
const std::string kNavigation = "https://saved.example.test/unsaved";

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kPersonal) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kResearch, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kHome, kPersonal) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  for (const auto& attachment :
       {kDeskFirst, kDeskSecond, kResearchFirst, kHomeFirst, kHomeSecond})
    REQUIRE(model.Attach(attachment) == Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kDesk) == Status::kOk);
  REQUIRE(model.SelectSpace(kSecond, kHome) == Status::kOk);
  return model;
}

void RequireOpen(const OpenResult& result, const PageId& page) {
  REQUIRE(std::holds_alternative<OpenedPage>(result));
  REQUIRE(std::get<OpenedPage>(result).page == page);
}

Model PopulatedWorkspace() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kKept, kDesk, kDestination) == Status::kOk);
  REQUIRE(model.AddItem(kClosed, kDesk, kDestination) == Status::kOk);
  REQUIRE(model.AddItem(kOtherKept, kResearch, kDestination) == Status::kOk);
  REQUIRE(model.AddItem(kProfileItem, kWork, kDestination) == Status::kOk);
  REQUIRE(model.AddShortcut(kAlias, kWork, kKept) == Status::kOk);
  REQUIRE(model.AddShortcut(kOtherAlias, kWork, kOtherKept) == Status::kOk);
  RequireOpen(model.OpenItem(kKept, kDeskFirst, kSavedPage), kSavedPage);
  RequireOpen(model.OpenItem(kProfileItem, kDeskFirst, kProfilePage),
              kProfilePage);
  REQUIRE(model.RecordNavigation(kSavedPage, kNavigation) == Status::kOk);
  REQUIRE(model.ReportActivity(kSavedPage, Activity::kProtected) ==
          Status::kOk);
  REQUIRE(model.OpenTemporary(kDraft, TemporaryTabId{"draft-tab"}, kDeskFirst,
                              kNavigation) == Status::kOk);
  REQUIRE(model.OpenTemporary(kOtherDraft, TemporaryTabId{"other-draft-tab"},
                              kDeskSecond, kNavigation) == Status::kOk);
  REQUIRE(model.SetSplit(kSplit, kDeskFirst, {kSavedPage, kDraft}) ==
          Status::kOk);
  return model;
}

Metadata Labels() {
  return {{kResearch, {"Research", "books", "blue"}},
          {kDesk, {"Desk", "desk", "green"}},
          {kHome, {"Home", std::nullopt, "warm"}}};
}

template <typename Result>
void RequireError(const Result& result,
                  MetadataErrorCode code,
                  const SpaceId& space) {
  REQUIRE(std::holds_alternative<MetadataError>(result));
  REQUIRE(std::get<MetadataError>(result).code == code);
  REQUIRE(std::get<MetadataError>(result).space == space);
}

Metadata TakeMetadata(MetadataResult result) {
  REQUIRE(std::holds_alternative<Metadata>(result));
  return std::get<Metadata>(std::move(result));
}

Reconciliation TakeReconciliation(ReconciliationResult result) {
  REQUIRE(std::holds_alternative<Reconciliation>(result));
  return std::get<Reconciliation>(std::move(result));
}

void ExpectPopulatedGraph(const Model& model) {
  REQUIRE(model.spaces().size() == 3);
  REQUIRE(model.spaces().at(kDesk).profile == kWork);
  REQUIRE(model.spaces().at(kResearch).profile == kWork);
  REQUIRE(model.spaces().at(kHome).profile == kPersonal);
  REQUIRE(model.windows().at(kFirst).selected_space == kDesk);
  REQUIRE(model.windows().at(kSecond).selected_space == kHome);
  REQUIRE(model.attachments().size() == 5);
  REQUIRE(model.items().size() == 4);
  REQUIRE(std::get<SpaceId>(model.items().at(kKept).owner) == kDesk);
  REQUIRE(std::get<ProfileId>(model.items().at(kProfileItem).owner) == kWork);
  REQUIRE(model.items().at(kKept).saved_destination == kDestination);
  REQUIRE(model.pages().size() == 4);
  const auto& page = model.pages().at(kSavedPage);
  REQUIRE(page.profile == kWork);
  REQUIRE(page.current_navigation == kNavigation);
  REQUIRE(page.activity == Activity::kProtected);
  REQUIRE(page.attachment() == kDeskFirst);
  REQUIRE(std::get<SavedPage>(page.owner).item == kKept);
  REQUIRE(model.pages().at(kProfilePage).attachment() == kDeskFirst);
  REQUIRE(model.TemporaryPages(kDeskFirst) == std::vector<PageId>{kDraft});
  REQUIRE(model.TemporaryPages(kDeskSecond) ==
          std::vector<PageId>{kOtherDraft});
  REQUIRE(model.shortcuts().size() == 2);
  REQUIRE(model.shortcuts().at(kAlias).target == kKept);
  REQUIRE(model.splits().at(kSplit).pages ==
          (std::vector<PageId>{kSavedPage, kDraft}));
}

void IdentityAndOrderPreserveGraph() {
  auto model = PopulatedWorkspace();
  const auto* original_page = &model.pages().at(kSavedPage);
  const auto* original_item = &model.items().at(kKept);
  const auto original = Labels();
  const auto retained = original;
  // Borrow both target and display payload from caller metadata. Equal names
  // across distinct Spaces do not merge either Space or its pages/items.
  auto renamed = TakeMetadata(ReplaceIdentity(
      model, original, original.at(1).space, original.at(0).identity));
  REQUIRE(original == retained);
  REQUIRE(renamed.at(1).space == kDesk);
  REQUIRE(renamed.at(1).identity == original.at(0).identity);
  REQUIRE(renamed.at(0) == original.at(0));
  REQUIRE(renamed.at(2) == original.at(2));
  const auto reordered =
      TakeMetadata(ReorderSpaces(model, renamed, {kHome, kDesk, kResearch}));
  REQUIRE(reordered == (Metadata{renamed.at(2), renamed.at(1), renamed.at(0)}));
  REQUIRE(!ValidateMetadata(model, reordered));
  REQUIRE(TakeMetadata(ReorderSpaces(model, reordered,
                                     {kHome, kDesk, kResearch})) == reordered);
  REQUIRE(&model.pages().at(kSavedPage) == original_page);
  REQUIRE(&model.items().at(kKept) == original_item);
  ExpectPopulatedGraph(model);
}

void ReconcileWithoutLosingUnknownPayload() {
  auto model = Workspace();
  const auto labels = Labels();
  const SpaceMetadata unknown{kGhost, {"Recover me", "old-icon", "old-theme"}};
  const Metadata input{labels.at(0), unknown, labels.at(1)};
  const auto retained = input;
  const auto result = TakeReconciliation(ReconcileMetadata(model, input));
  REQUIRE(input == retained);
  REQUIRE(result.metadata ==
          (Metadata{labels.at(0), labels.at(1), {kHome, {}}}));
  REQUIRE(result.removed == Metadata{unknown});
  REQUIRE(result.added == std::vector<SpaceId>{kHome});
  REQUIRE(!ValidateMetadata(model, result.metadata));
  const auto repeated =
      TakeReconciliation(ReconcileMetadata(model, result.metadata));
  REQUIRE(repeated.metadata == result.metadata);
  REQUIRE(repeated.removed.empty());
  REQUIRE(repeated.added.empty());

  // Model creation/deletion is performed by the fixture, never reconciliation.
  const SpaceId fresh{"fresh"};
  REQUIRE(model.AddSpace(fresh, kWork) == Status::kOk);
  RequireError(ReplaceIdentity(model, result.metadata, kDesk, {}),
               MetadataErrorCode::kMissingSpace, fresh);
  auto extended = TakeReconciliation(ReconcileMetadata(model, result.metadata));
  REQUIRE(extended.added == std::vector<SpaceId>{fresh});
  REQUIRE(extended.metadata.back().space == fresh);
  REQUIRE(model.RemoveSpace(fresh) == Status::kOk);
  const auto pruned =
      TakeReconciliation(ReconcileMetadata(model, extended.metadata));
  REQUIRE(pruned.metadata == result.metadata);
  REQUIRE(pruned.removed == (Metadata{{fresh, {}}}));
}

void RefuseAmbiguousOrStaleInputs() {
  const auto model = PopulatedWorkspace();
  const auto input = Labels();
  const auto retained = input;
  RequireError(ReorderSpaces(model, input, {kDesk, kDesk, kHome}),
               MetadataErrorCode::kDuplicateSpace, kDesk);
  RequireError(ReorderSpaces(model, input, {kDesk, kHome}),
               MetadataErrorCode::kMissingSpace, kResearch);
  RequireError(ReorderSpaces(model, input, {kDesk, kHome, kGhost}),
               MetadataErrorCode::kUnknownSpace, kGhost);
  RequireError(ReplaceIdentity(model, input, kGhost, {}),
               MetadataErrorCode::kUnknownSpace, kGhost);
  Metadata incomplete{input.at(1), input.at(0)};
  RequireError(ReplaceIdentity(model, incomplete, kDesk, {}),
               MetadataErrorCode::kMissingSpace, kHome);
  RequireError(ReorderSpaces(model, incomplete, {kDesk, kHome, kResearch}),
               MetadataErrorCode::kMissingSpace, kHome);
  auto duplicate = input;
  duplicate.push_back(input.at(0));
  RequireError(ReconcileMetadata(model, duplicate),
               MetadataErrorCode::kDuplicateSpace, kResearch);
  RequireError(ReplaceIdentity(model, duplicate, kDesk, {}),
               MetadataErrorCode::kDuplicateSpace, kResearch);
  const Metadata duplicate_unknown{{kGhost, {"First", {}, {}}},
                                   {kGhost, {"Second", {}, {}}}};
  RequireError(ReconcileMetadata(model, duplicate_unknown),
               MetadataErrorCode::kDuplicateSpace, kGhost);
  const Metadata stale{{kGhost, {}}};
  RequireError(ReplaceIdentity(model, stale, kDesk, {}),
               MetadataErrorCode::kUnknownSpace, kGhost);
  REQUIRE(input == retained);
  ExpectPopulatedGraph(model);
}

void DisplayValuesAndEmptyModels() {
  const Model empty;
  REQUIRE(!ValidateMetadata(empty, {}));
  REQUIRE(TakeMetadata(ReorderSpaces(empty, {}, {})).empty());
  const auto empty_reconciliation =
      TakeReconciliation(ReconcileMetadata(empty, {}));
  REQUIRE(empty_reconciliation.metadata.empty());
  REQUIRE(empty_reconciliation.removed.empty());
  REQUIRE(empty_reconciliation.added.empty());
  RequireError(ReplaceIdentity(empty, {}, kGhost, {}),
               MetadataErrorCode::kUnknownSpace, kGhost);
  REQUIRE(!InspectReferences(empty, kGhost));

  const auto model = Workspace();
  const auto defaults = TakeReconciliation(ReconcileMetadata(model, {}));
  REQUIRE(defaults.added == (std::vector<SpaceId>{kDesk, kHome, kResearch}));
  REQUIRE(defaults.metadata.front().identity == DisplayIdentity{});
  const DisplayIdentity display{"Arbete \xF0\x9F\x8C\x8D", "", "theme/custom"};
  const auto replaced =
      TakeMetadata(ReplaceIdentity(model, Labels(), kDesk, display));
  REQUIRE(replaced.at(1).identity == display);
  REQUIRE(replaced.at(1).identity.icon_key.has_value());
  REQUIRE(replaced.at(1).identity.icon_key->empty());
  const auto cleared =
      TakeMetadata(ReplaceIdentity(model, replaced, kDesk, {}));
  REQUIRE(!cleared.at(1).identity.name);
  REQUIRE(!cleared.at(1).identity.icon_key);
  REQUIRE(!cleared.at(1).identity.theme_key);
}

void InspectOnlyExistingLifecycleReferences() {
  auto model = PopulatedWorkspace();
  const auto report = InspectReferences(model, kDesk);
  REQUIRE(report.has_value());
  REQUIRE(report->space == kDesk);
  REQUIRE(report->kept_items == (std::vector<ItemId>{kClosed, kKept}));
  REQUIRE(report->attached_pages ==
          (std::vector<PageId>{kDraft, kOtherDraft, kProfilePage, kSavedPage}));
  REQUIRE(report->aliases_of_kept_items == std::vector<ShortcutId>{kAlias});
  REQUIRE(report->attachments ==
          (std::vector<Attachment>{kDeskFirst, kDeskSecond}));
  REQUIRE(report->splits == std::vector<SplitId>{kSplit});
  REQUIRE(report->selected_by == std::vector<WindowId>{kFirst});
  const auto other = InspectReferences(model, kResearch);
  REQUIRE(other->kept_items == std::vector<ItemId>{kOtherKept});
  REQUIRE(other->attached_pages.empty());
  REQUIRE(other->aliases_of_kept_items == std::vector<ShortcutId>{kOtherAlias});
  REQUIRE(!InspectReferences(model, kGhost));
  ExpectPopulatedGraph(model);

  // The advisory snapshot retains owned IDs when Model subsequently changes.
  REQUIRE(model.TransferSavedPage(kSavedPage, kDeskSecond) == Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kHome) == Status::kOk);
  REQUIRE(model.RemoveShortcut(kAlias) == Status::kOk);
  REQUIRE(model.RemoveItem(kClosed) == Status::kOk);
  REQUIRE(report->aliases_of_kept_items == std::vector<ShortcutId>{kAlias});
  REQUIRE(report->kept_items == (std::vector<ItemId>{kClosed, kKept}));
  REQUIRE(report->selected_by == std::vector<WindowId>{kFirst});
  const auto fresh = InspectReferences(model, kDesk);
  REQUIRE(fresh->aliases_of_kept_items.empty());
  REQUIRE(fresh->kept_items == std::vector<ItemId>{kKept});
  REQUIRE(fresh->selected_by.empty());
  REQUIRE(model.pages().at(kSavedPage).profile == kWork);
  REQUIRE(model.pages().at(kSavedPage).current_navigation == kNavigation);
  REQUIRE(model.pages().at(kSavedPage).activity == Activity::kProtected);
}

}  // namespace
}  // namespace arcified::space_management

int main() {
  using namespace arcified::space_management;
  IdentityAndOrderPreserveGraph();
  ReconcileWithoutLosingUnknownPayload();
  RefuseAmbiguousOrStaleInputs();
  DisplayValuesAndEmptyModels();
  InspectOnlyExistingLifecycleReferences();
  std::cout << "Space management synthetic checks passed\n";
}
