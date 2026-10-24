#include "folders.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace arcified::organization {
namespace {

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

static_assert(!std::is_convertible_v<FolderId, lifecycle::ItemId>);
static_assert(!std::is_convertible_v<FolderId, lifecycle::SpaceId>);

const lifecycle::ProfileId kProfile{"profile"};
const lifecycle::ProfileId kOtherProfile{"other-profile"};
const lifecycle::SpaceId kSpace{"space"};
const lifecycle::SpaceId kOtherSpace{"other-space"};
const lifecycle::SpaceId kForeignSpace{"foreign-space"};
const lifecycle::ItemId kOne{"one"};
const lifecycle::ItemId kTwo{"two"};
const lifecycle::ItemId kThree{"three"};
const lifecycle::ItemId kShortcut{"shortcut"};
const FolderId kFolder{"folder"};
const FolderId kNested{"nested"};
const FolderId kLeaf{"leaf"};

lifecycle::Model Workspace() {
  lifecycle::Model model;
  using S = lifecycle::Status;
  REQUIRE(model.AddProfile(kProfile) == S::kOk);
  REQUIRE(model.AddProfile(kOtherProfile) == S::kOk);
  REQUIRE(model.AddSpace(kSpace, kProfile) == S::kOk);
  REQUIRE(model.AddSpace(kOtherSpace, kProfile) == S::kOk);
  REQUIRE(model.AddSpace(kForeignSpace, kOtherProfile) == S::kOk);
  // Equal URLs must never merge distinct item identities.
  for (const auto& item : {kOne, kTwo, kThree}) {
    REQUIRE(model.AddItem(item, kSpace, "https://example.test/saved") ==
            S::kOk);
  }
  REQUIRE(model.AddItem(kShortcut, kProfile, "https://example.test/shortcut") ==
          S::kOk);
  return model;
}

Placement At(std::optional<FolderId> folder = std::nullopt,
             std::optional<Entry> before = std::nullopt,
             lifecycle::SpaceId space = kSpace) {
  return {{std::move(space), std::move(folder)}, std::move(before)};
}

Hierarchy Read(const Folders& folders,
               const lifecycle::Model& model,
               lifecycle::SpaceId space = kSpace) {
  auto result = folders.Snapshot(model, std::move(space));
  REQUIRE(std::holds_alternative<Hierarchy>(result));
  return std::get<Hierarchy>(std::move(result));
}

void ExpectSame(const Hierarchy& a, const Hierarchy& b) {
  REQUIRE(a.space == b.space);
  REQUIRE(a.roots == b.roots);
  REQUIRE(a.folders.size() == b.folders.size());
  for (const auto& [id, folder] : a.folders) {
    const auto& other = b.folders.at(id);
    REQUIRE(folder.label == other.label);
    REQUIRE(folder.expanded == other.expanded);
    REQUIRE(folder.children == other.children);
  }
}

void MixedOrderAndReparenting() {
  auto model = Workspace();
  Folders folders;
  REQUIRE(folders.PlaceItem(model, kOne, At()) == FolderStatus::kOk);
  REQUIRE(folders.CreateFolder(model, kFolder, "Work", At()) ==
          FolderStatus::kOk);
  REQUIRE(folders.PlaceItem(model, kTwo, At()) == FolderStatus::kOk);
  REQUIRE(folders.CreateFolder(model, kNested, "Nested", At(kFolder)) ==
          FolderStatus::kOk);
  REQUIRE(folders.PlaceItem(model, kThree, At(kNested)) == FolderStatus::kOk);
  auto tree = Read(folders, model);
  REQUIRE(tree.roots == std::vector<Entry>({kOne, kFolder, kTwo}));
  REQUIRE(tree.Ancestors(kThree) ==
          std::optional<std::vector<FolderId>>({{kFolder, kNested}}));
  REQUIRE(tree.Locate(kFolder)->parent == std::nullopt);
  REQUIRE(tree.Locate(kFolder)->index == 1);
  REQUIRE(folders.PlaceItem(model, kTwo, At(std::nullopt, kOne)) ==
          FolderStatus::kOk);
  REQUIRE(folders.PlaceItem(model, kOne, At(kFolder, kNested)) ==
          FolderStatus::kOk);
  REQUIRE(folders.MoveFolder(model, kNested, At(std::nullopt, kFolder)) ==
          FolderStatus::kOk);
  tree = Read(folders, model);
  REQUIRE(tree.roots == std::vector<Entry>({kTwo, kNested, kFolder}));
  REQUIRE(tree.folders.at(kFolder).children == std::vector<Entry>({kOne}));
  REQUIRE(tree.folders.at(kNested).children == std::vector<Entry>({kThree}));
  REQUIRE(tree.Ancestors(kThree) ==
          std::optional<std::vector<FolderId>>({{kNested}}));
  // Reordering a sibling around a later anchor computes position after removal.
  REQUIRE(folders.PlaceItem(model, kTwo, At(std::nullopt, kFolder)) ==
          FolderStatus::kOk);
  REQUIRE(Read(folders, model).roots ==
          std::vector<Entry>({kNested, kTwo, kFolder}));
  REQUIRE(folders.MoveFolder(model, kNested, At(std::nullopt, kNested)) ==
          FolderStatus::kOk);
  REQUIRE(Read(folders, model).roots ==
          std::vector<Entry>({kNested, kTwo, kFolder}));
}

void InvalidMovesLeaveHierarchyIntact() {
  auto model = Workspace();
  Folders folders;
  REQUIRE(folders.CreateFolder(model, kFolder, "Root", At()) ==
          FolderStatus::kOk);
  REQUIRE(folders.CreateFolder(model, kNested, "Nested", At(kFolder)) ==
          FolderStatus::kOk);
  REQUIRE(folders.CreateFolder(model, kLeaf, "Leaf", At(kNested)) ==
          FolderStatus::kOk);
  REQUIRE(folders.PlaceItem(model, kOne, At(kNested)) == FolderStatus::kOk);
  const auto original = Read(folders, model);
  REQUIRE(folders.MoveFolder(model, kFolder, At(kFolder)) ==
          FolderStatus::kCycle);
  REQUIRE(folders.MoveFolder(model, kFolder, At(kLeaf)) ==
          FolderStatus::kCycle);
  REQUIRE(folders.MoveFolder(model, kNested, At(kLeaf)) ==
          FolderStatus::kCycle);
  REQUIRE(folders.PlaceItem(model, kTwo, At(std::nullopt, kOne)) ==
          FolderStatus::kNotFound);
  REQUIRE(folders.MoveFolder(model, kNested, At(FolderId{"absent"})) ==
          FolderStatus::kNotFound);
  REQUIRE(
      folders.MoveFolder(model, kFolder, At(std::nullopt, {}, kOtherSpace)) ==
      FolderStatus::kCrossSpaceUnsupported);
  REQUIRE(
      folders.MoveFolder(model, kFolder, At(std::nullopt, {}, kForeignSpace)) ==
      FolderStatus::kCrossSpaceUnsupported);
  ExpectSame(original, Read(folders, model));
}

void ExpansionAndRenameAreMetadataOnly() {
  auto model = Workspace();
  Folders folders;
  REQUIRE(folders.CreateFolder(model, kFolder, "Root", At()) ==
          FolderStatus::kOk);
  REQUIRE(folders.CreateFolder(model, kNested, "Nested", At(kFolder)) ==
          FolderStatus::kOk);
  REQUIRE(folders.CreateFolder(model, kLeaf, "Other",
                               At({}, {}, kOtherSpace)) == FolderStatus::kOk);
  REQUIRE(folders.PlaceItem(model, kOne, At(kNested)) == FolderStatus::kOk);
  const auto before = Read(folders, model);
  REQUIRE(folders.Rename(model, kFolder, "Renamed") == FolderStatus::kOk);
  REQUIRE(folders.SetExpanded(model, kFolder, false) == FolderStatus::kOk);
  REQUIRE(folders.SetAllExpanded(model, kSpace, false) == FolderStatus::kOk);
  auto after = Read(folders, model);
  REQUIRE(after.folders.at(kFolder).label == "Renamed");
  REQUIRE(!after.folders.at(kFolder).expanded);
  REQUIRE(!after.folders.at(kNested).expanded);
  REQUIRE(Read(folders, model, kOtherSpace).folders.at(kLeaf).expanded);
  REQUIRE(after.roots == before.roots);
  REQUIRE(after.folders.at(kNested).children ==
          before.folders.at(kNested).children);
  REQUIRE(folders.SetAllExpanded(model, kSpace, true) == FolderStatus::kOk);
  REQUIRE(Read(folders, model).folders.at(kFolder).expanded);
  REQUIRE(Read(folders, model).folders.at(kNested).expanded);
  // An owned earlier snapshot is unaffected by subsequent mutations.
  REQUIRE(before.folders.at(kFolder).label == "Root");
  REQUIRE(before.folders.at(kFolder).expanded);
}

void IdentityAndPlacementValidation() {
  auto model = Workspace();
  Folders folders;
  REQUIRE(folders.CreateFolder(model, FolderId{""}, "Empty", At()) ==
          FolderStatus::kInvalidInput);
  REQUIRE(folders.CreateFolder(model, kFolder, "", At()) ==
          FolderStatus::kInvalidInput);
  REQUIRE(folders.CreateFolder(model, kFolder, "Work", At()) ==
          FolderStatus::kOk);
  REQUIRE(folders.CreateFolder(model, kFolder, "Duplicate",
                               At({}, {}, kOtherSpace)) ==
          FolderStatus::kDuplicateId);
  REQUIRE(folders.Rename(model, kFolder, "") == FolderStatus::kInvalidInput);
  REQUIRE(folders.PlaceItem(model, lifecycle::ItemId{"absent"}, At(kFolder)) ==
          FolderStatus::kItemNotFound);
  REQUIRE(folders.PlaceItem(model, kShortcut, At(kFolder)) ==
          FolderStatus::kItemOwnerMismatch);
  REQUIRE(folders.PlaceItem(model, kOne, At({}, {}, kOtherSpace)) ==
          FolderStatus::kItemOwnerMismatch);
  REQUIRE(folders.CreateFolder(model, kNested, "Wrong Space",
                               At(kFolder, {}, kOtherSpace)) ==
          FolderStatus::kCrossSpaceUnsupported);
  REQUIRE(folders.PlaceItem(model, kOne,
                            At({}, {}, lifecycle::SpaceId{"absent"})) ==
          FolderStatus::kSpaceNotFound);
  REQUIRE(folders.Rename(model, FolderId{"absent"}, "New") ==
          FolderStatus::kNotFound);
  REQUIRE(folders.SetExpanded(model, FolderId{"absent"}, false) ==
          FolderStatus::kNotFound);
  REQUIRE(std::get<FolderStatus>(folders.ReadFolder(
              model, FolderId{"absent"})) == FolderStatus::kNotFound);
  const auto tree = Read(folders, model);
  REQUIRE(tree.roots == std::vector<Entry>({kFolder}));
  REQUIRE(!tree.Locate(kOne));
  REQUIRE(!tree.Ancestors(kOne));
  REQUIRE(model.items().size() == 4);
  REQUIRE(Read(folders, model, kOtherSpace).roots.empty());
}

void StaleLifecycleReferencesAreRefusedWithoutPruning() {
  auto model = Workspace();
  Folders folders;
  REQUIRE(folders.CreateFolder(model, kFolder, "Work", At()) ==
          FolderStatus::kOk);
  REQUIRE(folders.PlaceItem(model, kOne, At(kFolder)) == FolderStatus::kOk);
  const auto original_model = model;
  const auto original = Read(folders, model);
  REQUIRE(model.MoveItem(kOne, kOtherSpace) == lifecycle::Status::kOk);
  REQUIRE(std::get<FolderStatus>(folders.Snapshot(model, kSpace)) ==
          FolderStatus::kItemOwnerMismatch);
  REQUIRE(folders.Rename(model, kFolder, "Must not apply") ==
          FolderStatus::kItemOwnerMismatch);
  REQUIRE(folders.SetExpanded(model, kFolder, false) ==
          FolderStatus::kItemOwnerMismatch);
  REQUIRE(folders.SetAllExpanded(model, kSpace, false) ==
          FolderStatus::kItemOwnerMismatch);
  REQUIRE(folders.PlaceItem(model, kOne, At({}, {}, kOtherSpace)) ==
          FolderStatus::kCrossSpaceUnsupported);
  REQUIRE(model.RemoveItem(kOne) == lifecycle::Status::kOk);
  REQUIRE(std::get<FolderStatus>(folders.ReadFolder(model, kFolder)) ==
          FolderStatus::kItemNotFound);
  REQUIRE(folders.CreateFolder(model, kNested, "Must not apply", At()) ==
          FolderStatus::kItemNotFound);
  REQUIRE(folders.MoveFolder(model, kFolder, At()) ==
          FolderStatus::kItemNotFound);
  ExpectSame(original, Read(folders, original_model));
  // A stale tree does not prevent operations in another valid Space.
  REQUIRE(folders.CreateFolder(model, kNested, "Other",
                               At({}, {}, kOtherSpace)) == FolderStatus::kOk);
  REQUIRE(model.RemoveItem(kTwo) == lifecycle::Status::kOk);
  REQUIRE(model.RemoveItem(kThree) == lifecycle::Status::kOk);
  REQUIRE(model.RemoveSpace(kSpace) == lifecycle::Status::kOk);
  REQUIRE(std::get<FolderStatus>(folders.Snapshot(model, kSpace)) ==
          FolderStatus::kSpaceNotFound);
}

void LiveIdentityAndSharedSpaceSurviveOrganization() {
  auto model = Workspace();
  const lifecycle::WindowId first{"first"};
  const lifecycle::WindowId second{"second"};
  const lifecycle::Attachment a{first, kSpace};
  const lifecycle::Attachment b{second, kSpace};
  const lifecycle::PageId page{"page"};
  const lifecycle::PageId temporary{"temporary"};
  using S = lifecycle::Status;
  REQUIRE(model.AddWindow(first) == S::kOk);
  REQUIRE(model.AddWindow(second) == S::kOk);
  REQUIRE(model.Attach(a) == S::kOk);
  REQUIRE(model.Attach(b) == S::kOk);
  REQUIRE(std::holds_alternative<lifecycle::OpenedPage>(
      model.OpenItem(kOne, a, page)));
  REQUIRE(model.RecordNavigation(page, "https://example.test/draft") == S::kOk);
  REQUIRE(model.ReportActivity(page, lifecycle::Activity::kProtected) ==
          S::kOk);
  REQUIRE(model.OpenTemporary(temporary, lifecycle::TemporaryTabId{"temp"}, a,
                              "https://example.test/temporary") == S::kOk);
  Folders folders;
  REQUIRE(folders.CreateFolder(model, kFolder, "Shared", At()) ==
          FolderStatus::kOk);
  REQUIRE(folders.PlaceItem(model, kOne, At(kFolder)) == FolderStatus::kOk);
  REQUIRE(folders.SetExpanded(model, kFolder, false) == FolderStatus::kOk);
  REQUIRE(model.pages().size() == 2);
  REQUIRE(model.pages().at(page).attachment() == a);
  REQUIRE(model.pages().at(page).current_navigation ==
          "https://example.test/draft");
  REQUIRE(model.pages().at(page).activity == lifecycle::Activity::kProtected);
  REQUIRE(model.pages().at(page).profile == kProfile);
  REQUIRE(model.items().at(kOne).saved_destination ==
          "https://example.test/saved");
  REQUIRE(model.TemporaryPages(a) ==
          std::vector<lifecycle::PageId>({temporary}));
  REQUIRE(model.TemporaryPages(b).empty());
  REQUIRE(model.VisibleItems(a) == model.VisibleItems(b));
  REQUIRE(Read(folders, model).folders.at(kFolder).children ==
          std::vector<Entry>({kOne}));
  // The same-page lifecycle transfer is independent of hierarchy and collapse.
  const auto hierarchy = Read(folders, model);
  REQUIRE(model.TransferSavedPage(page, b) == S::kOk);
  ExpectSame(hierarchy, Read(folders, model));
  REQUIRE(model.pages().at(page).current_navigation ==
          "https://example.test/draft");
  REQUIRE(model.pages().at(page).activity == lifecycle::Activity::kProtected);
}

void ArbitraryDepthUsesIterativeAncestry() {
  auto model = Workspace();
  Folders folders;
  REQUIRE(folders.CreateFolder(model, kFolder, "Root", At()) ==
          FolderStatus::kOk);
  REQUIRE(folders.CreateFolder(model, kNested, "Child", At(kFolder)) ==
          FolderStatus::kOk);
  REQUIRE(folders.CreateFolder(model, kLeaf, "Grandchild", At(kNested)) ==
          FolderStatus::kOk);
  // This is a finite synthetic hierarchy, not a product nesting limit.
  const FolderId descendant{"descendant"};
  REQUIRE(folders.CreateFolder(model, descendant, "Descendant", At(kLeaf)) ==
          FolderStatus::kOk);
  REQUIRE(folders.PlaceItem(model, kOne, At(descendant)) == FolderStatus::kOk);
  const auto tree = Read(folders, model);
  REQUIRE(tree.Ancestors(kOne) == std::optional<std::vector<FolderId>>(
                                      {{kFolder, kNested, kLeaf, descendant}}));
  REQUIRE(tree.Ancestors(kFolder)->empty());
  auto by_folder = folders.ReadFolder(model, descendant);
  REQUIRE(std::holds_alternative<Hierarchy>(by_folder));
  ExpectSame(tree, std::get<Hierarchy>(by_folder));
}

}  // namespace
}  // namespace arcified::organization

int main() {
  using namespace arcified::organization;
  MixedOrderAndReparenting();
  InvalidMovesLeaveHierarchyIntact();
  ExpansionAndRenameAreMetadataOnly();
  IdentityAndPlacementValidation();
  StaleLifecycleReferencesAreRefusedWithoutPruning();
  LiveIdentityAndSharedSpaceSurviveOrganization();
  ArbitraryDepthUsesIterativeAncestry();
  std::cout << "7 synthetic folder fixtures passed\n";
}
