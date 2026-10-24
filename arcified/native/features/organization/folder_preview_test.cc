#include "folder_preview.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace arcified::organization {
namespace {

namespace life = lifecycle;
static_assert(!std::is_convertible_v<FolderId, life::ItemId>);

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const life::ProfileId kProfile{"profile"};
const life::SpaceId kSpace{"space"};
const life::SpaceId kOtherSpace{"other-space"};
const life::WindowId kWindow{"window"};
const life::WindowId kOtherWindow{"other-window"};
const life::Attachment kContext{kWindow, kSpace};
const life::Attachment kOtherContext{kOtherWindow, kSpace};
const FolderId kRoot{"root"};
const FolderId kNested{"nested"};
const life::ItemId kFirst{"z-first"};
const life::ItemId kSecond{"a-second"};
const life::ItemId kThird{"m-third"};
const life::PageId kFirstPage{"first-page"};
const life::PageId kSecondPage{"second-page"};
const life::PageId kTemporary{"temporary"};

struct Fixture {
  life::Model model;
  Folders folders;
  std::map<life::ItemId, std::string> labels{{kFirst, "Caf\xc3\xa9 notes"},
                                             {kSecond, "Notes"},
                                             {kThird, "Notes"}};

  Fixture() {
    REQUIRE(model.AddProfile(kProfile) == life::Status::kOk);
    REQUIRE(model.AddSpace(kSpace, kProfile) == life::Status::kOk);
    REQUIRE(model.AddSpace(kOtherSpace, kProfile) == life::Status::kOk);
    REQUIRE(model.AddWindow(kWindow) == life::Status::kOk);
    REQUIRE(model.AddWindow(kOtherWindow) == life::Status::kOk);
    REQUIRE(model.Attach(kContext) == life::Status::kOk);
    REQUIRE(model.Attach(kOtherContext) == life::Status::kOk);
    REQUIRE(model.SelectSpace(kWindow, kSpace) == life::Status::kOk);
    for (const auto& item : {kFirst, kSecond, kThird}) {
      REQUIRE(model.AddItem(item, kSpace, "https://example.test/saved") ==
              life::Status::kOk);
    }
    REQUIRE(folders.CreateFolder(model, kRoot, "Research",
                                 {{kSpace, std::nullopt}, std::nullopt}) ==
            FolderStatus::kOk);
    REQUIRE(folders.CreateFolder(model, kNested, "Articles",
                                 {{kSpace, kRoot}, std::nullopt}) ==
            FolderStatus::kOk);
    REQUIRE(
        folders.PlaceItem(model, kFirst, {{kSpace, kNested}, std::nullopt}) ==
        FolderStatus::kOk);
    REQUIRE(
        folders.PlaceItem(model, kSecond, {{kSpace, kRoot}, Entry{kNested}}) ==
        FolderStatus::kOk);
    REQUIRE(folders.PlaceItem(model, kThird, {{kSpace, kRoot}, std::nullopt}) ==
            FolderStatus::kOk);
    REQUIRE(folders.SetExpanded(model, kRoot, false) == FolderStatus::kOk);
    REQUIRE(folders.SetExpanded(model, kNested, false) == FolderStatus::kOk);
  }

  PreviewResult Query(std::string_view query = {},
                      std::optional<life::PageId> active = std::nullopt,
                      FolderId folder = kRoot,
                      life::Attachment context = kContext) const {
    return ProjectFolderPreview(model, folders, folder, context, labels, query,
                                active);
  }

  void Open(life::ItemId item,
            life::PageId page,
            life::Attachment context = kContext) {
    const auto opened = model.OpenItem(item, context, page);
    REQUIRE(std::holds_alternative<life::OpenedPage>(opened));
    REQUIRE(std::get<life::OpenedPage>(opened).page == page);
  }
};

FolderPreview Value(const PreviewResult& result) {
  REQUIRE(std::holds_alternative<FolderPreview>(result));
  return std::get<FolderPreview>(result);
}

void NestedOrderSearchAndHierarchy() {
  Fixture f;
  const auto all = Value(f.Query());
  REQUIRE(all.matches.size() == 3);
  REQUIRE(all.matches[0].item == kSecond);
  REQUIRE(all.matches[1].item == kFirst);
  REQUIRE(all.matches[2].item == kThird);
  const auto& nested = all.matches[1];
  REQUIRE(nested.ancestry.size() == 2);
  REQUIRE(nested.ancestry[0].folder == kRoot);
  REQUIRE(nested.ancestry[0].label == "Research");
  REQUIRE(!nested.ancestry[0].expanded);
  REQUIRE(nested.ancestry[1].folder == kNested);
  REQUIRE(nested.ancestry[1].label == "Articles");
  REQUIRE(nested.location.parent == kNested);
  REQUIRE(nested.location.index == 0);
  REQUIRE(!nested.live_page);
  REQUIRE(!all.active_descendant);
  const auto filtered = Value(f.Query("\xc3\xa9"));
  REQUIRE(filtered.matches.size() == 1);
  REQUIRE(filtered.matches[0].item == kFirst);
  REQUIRE(Value(f.Query("NOTES")).matches.empty());
  REQUIRE(Value(f.Query("Research")).matches.empty());
  const auto duplicate_labels = Value(f.Query("Notes"));
  REQUIRE(duplicate_labels.matches.size() == 2);
  REQUIRE(duplicate_labels.matches[0].item == kSecond);
  REQUIRE(duplicate_labels.matches[1].item == kThird);
  const auto subtree = Value(f.Query({}, std::nullopt, kNested));
  REQUIRE(subtree.matches.size() == 1);
  REQUIRE(subtree.matches[0].ancestry.size() == 2);
  REQUIRE(subtree.matches[0].ancestry[0].folder == kRoot);
}

void ActiveProjectionAndBackgroundOpen() {
  Fixture f;
  f.Open(kFirst, kFirstPage);
  REQUIRE(f.model.RecordNavigation(kFirstPage,
                                   "https://example.test/unsaved-draft") ==
          life::Status::kOk);
  REQUIRE(f.model.ReportActivity(kFirstPage, life::Activity::kProtected) ==
          life::Status::kOk);
  f.Open(kSecond, kSecondPage);  // Observed background open is not selection.
  const auto result = Value(f.Query("absent", kFirstPage));
  REQUIRE(result.matches.empty());
  REQUIRE(result.active_descendant.has_value());
  REQUIRE(result.active_descendant->item == kFirst);
  REQUIRE(result.active_descendant->live_page == kFirstPage);
  REQUIRE(result.active_descendant->active);
  const auto all = Value(f.Query({}, kFirstPage));
  REQUIRE(!all.matches[0].active);
  REQUIRE(all.matches[1].active);
  REQUIRE(f.model.pages().size() == 2);
  REQUIRE(f.model.pages().at(kFirstPage).current_navigation ==
          "https://example.test/unsaved-draft");
  REQUIRE(f.model.pages().at(kFirstPage).activity ==
          life::Activity::kProtected);
  REQUIRE(f.model.pages().at(kFirstPage).attachment() == kContext);
  REQUIRE(f.model.windows().at(kWindow).selected_space == kSpace);
  const auto snapshot =
      std::get<Hierarchy>(f.folders.Snapshot(f.model, kSpace));
  REQUIRE(!snapshot.folders.at(kRoot).expanded);
  REQUIRE(!snapshot.folders.at(kNested).expanded);
  REQUIRE(f.folders.SetExpanded(f.model, kRoot, true) == FolderStatus::kOk);
  REQUIRE(!Value(f.Query({}, kFirstPage)).active_descendant);
  REQUIRE(
      Value(f.Query({}, kFirstPage, kNested)).active_descendant.has_value());
}

void RenameDragOutAndOwnedObservations() {
  Fixture f;
  f.Open(kFirst, kFirstPage);
  const auto before = Value(f.Query({}, kFirstPage));
  f.labels[kFirst] = "Renamed";
  REQUIRE(f.folders.Rename(f.model, kNested, "Renamed folder") ==
          FolderStatus::kOk);
  const auto renamed = Value(f.Query("Renamed", kFirstPage));
  REQUIRE(renamed.matches.size() == 1);
  REQUIRE(renamed.matches[0].item == kFirst);
  REQUIRE(renamed.matches[0].live_page == kFirstPage);
  REQUIRE(renamed.matches[0].ancestry[1].label == "Renamed folder");
  REQUIRE(before.matches[1].label == "Caf\xc3\xa9 notes");
  REQUIRE(before.matches[1].ancestry[1].label == "Articles");
  REQUIRE(
      f.folders.PlaceItem(f.model, kFirst, {{kSpace, kRoot}, Entry{kSecond}}) ==
      FolderStatus::kOk);
  const auto moved = Value(f.Query({}, kFirstPage));
  REQUIRE(moved.matches[0].item == kFirst);
  REQUIRE(moved.matches[0].live_page == kFirstPage);
  REQUIRE(moved.matches[0].location.parent == kRoot);
  REQUIRE(moved.matches[0].location.index == 0);
  REQUIRE(moved.matches[0].ancestry.size() == 1);
  REQUIRE(Value(f.Query({}, kFirstPage, kNested)).matches.empty());
  REQUIRE(!Value(f.Query({}, kFirstPage, kNested)).active_descendant);
  REQUIRE(f.folders.PlaceItem(f.model, kFirst,
                              {{kSpace, std::nullopt}, Entry{kRoot}}) ==
          FolderStatus::kOk);
  REQUIRE(Value(f.Query("Renamed", kFirstPage)).matches.empty());
  REQUIRE(!Value(f.Query({}, kFirstPage)).active_descendant);
  REQUIRE(f.model.pages().at(kFirstPage).attachment() == kContext);
}

void LiveIdentityAcrossWindowsAndTemporarySelection() {
  Fixture f;
  f.Open(kFirst, kFirstPage, kOtherContext);
  const auto remote = Value(f.Query());
  REQUIRE(remote.matches[1].live_page == kFirstPage);
  REQUIRE(!remote.matches[1].active);
  REQUIRE(f.model.pages().at(kFirstPage).attachment() == kOtherContext);
  REQUIRE(std::get<PreviewError>(f.Query({}, kFirstPage)) ==
          PreviewError::kActivePageOutsideAttachment);
  REQUIRE(Value(f.Query({}, kFirstPage, kRoot, kOtherContext))
              .active_descendant->item == kFirst);
  REQUIRE(f.model.OpenTemporary(kTemporary, life::TemporaryTabId{"temp-tab"},
                                kContext, "https://example.test/temporary") ==
          life::Status::kOk);
  const auto temporary = Value(f.Query({}, kTemporary));
  REQUIRE(temporary.matches.size() == 3);
  REQUIRE(!temporary.active_descendant);
  for (const auto& row : temporary.matches)
    REQUIRE(!row.active);
}

void StaleReferencesAndContextRefuse() {
  Fixture f;
  REQUIRE(
      std::get<PreviewError>(f.Query({}, std::nullopt, FolderId{"missing"})) ==
      PreviewError::kFolderNotFound);
  REQUIRE(std::get<PreviewError>(f.Query({}, life::PageId{"missing"})) ==
          PreviewError::kActivePageNotFound);
  REQUIRE(std::get<PreviewError>(
              f.Query({}, std::nullopt, kRoot, {kWindow, kOtherSpace})) ==
          PreviewError::kNotAttached);
  f.labels.erase(kFirst);
  REQUIRE(std::get<PreviewError>(f.Query("no matches")) ==
          PreviewError::kMissingItemLabel);
  f.labels[kFirst] = "Restored";
  REQUIRE(f.model.RemoveItem(kFirst) == life::Status::kOk);
  REQUIRE(std::get<FolderStatus>(f.Query()) == FolderStatus::kItemNotFound);

  Fixture moved;
  REQUIRE(moved.model.MoveItem(kFirst, kOtherSpace) == life::Status::kOk);
  REQUIRE(std::get<FolderStatus>(moved.Query()) ==
          FolderStatus::kItemOwnerMismatch);
}

void ClosedPagesAndUnrepresentedItems() {
  Fixture f;
  const life::ItemId unplaced{"unplaced"};
  REQUIRE(f.model.AddItem(unplaced, kSpace, "https://example.test/unplaced") ==
          life::Status::kOk);
  // A folder query requires labels only for its descendants. It is not the
  // complete saved-item listing and must not claim omitted items were deleted.
  REQUIRE(Value(f.Query()).matches.size() == 3);
  REQUIRE(f.model.items().count(unplaced) == 1);
  f.Open(kFirst, kFirstPage);
  REQUIRE(f.model.ReportActivity(kFirstPage, life::Activity::kQuiescent) ==
          life::Status::kOk);
  REQUIRE(f.model.ClosePage(kFirstPage) == life::Status::kOk);
  REQUIRE(std::get<PreviewError>(f.Query({}, kFirstPage)) ==
          PreviewError::kActivePageNotFound);
  const auto closed = Value(f.Query());
  REQUIRE(closed.matches[1].item == kFirst);
  REQUIRE(!closed.matches[1].live_page);
  REQUIRE(!closed.active_descendant);
  REQUIRE(f.model.Attach({kWindow, kOtherSpace}) == life::Status::kOk);
  REQUIRE(std::get<PreviewError>(
              f.Query({}, std::nullopt, kRoot, {kWindow, kOtherSpace})) ==
          PreviewError::kFolderNotFound);
}

}  // namespace
}  // namespace arcified::organization

int main() {
  using namespace arcified::organization;
  NestedOrderSearchAndHierarchy();
  ActiveProjectionAndBackgroundOpen();
  RenameDragOutAndOwnedObservations();
  LiveIdentityAcrossWindowsAndTemporarySelection();
  StaleReferencesAndContextRefuse();
  ClosedPagesAndUnrepresentedItems();
  std::cout << "folder preview synthetic checks passed\n";
}
