#include "multi_select.h"

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

const lifecycle::ProfileId kProfile{"profile"};
const lifecycle::ProfileId kOtherProfile{"other-profile"};
const lifecycle::SpaceId kSpace{"space"};
const lifecycle::SpaceId kOtherSpace{"other-space"};
const lifecycle::SpaceId kForeignSpace{"foreign-space"};
const lifecycle::WindowId kWindow{"window"};
const FolderId kFolder{"folder"};
const FolderId kNested{"nested"};
const FolderId kDestination{"destination"};
const lifecycle::ItemId kA{"a"};
const lifecycle::ItemId kB{"b"};
const lifecycle::ItemId kC{"c"};
const lifecycle::ItemId kD{"d"};
const lifecycle::ItemId kE{"e"};
const lifecycle::ItemId kU{"unplaced-u"};
const lifecycle::ItemId kV{"unplaced-v"};
const lifecycle::TemporaryTabId kTemporary{"temporary"};
const lifecycle::PageId kLive{"live"};
const lifecycle::PageId kTemporaryPage{"temporary-page"};

static_assert(!std::is_convertible_v<FolderId, lifecycle::ItemId>);
static_assert(!std::is_convertible_v<lifecycle::PageId, SelectionRowId>);

Selection Selected(std::initializer_list<SelectionRowId> ids) {
  return {std::set<SelectionRowId>{ids}, std::nullopt};
}

Selection Apply(const std::vector<SelectionRowId>& rows,
                const Selection& selection,
                SelectionRowId target,
                SelectionGesture gesture) {
  const auto result = ApplySelection(rows, selection, target, gesture);
  REQUIRE(std::holds_alternative<Selection>(result));
  return std::get<Selection>(result);
}

void RangeAndAdditiveSelection() {
  // Mixed row kinds, deliberately unrelated to lexical ID order.
  const std::vector<SelectionRowId> rows{kD, kFolder, kA, kTemporary, kB};
  auto selection = Apply(rows, {}, kA, SelectionGesture::kReplace);
  REQUIRE(selection.selected == Selected({kA}).selected);
  REQUIRE(selection.anchor == SelectionRowId{kA});
  selection = Apply(rows, selection, kD, SelectionGesture::kRange);
  REQUIRE(selection.selected == Selected({kD, kFolder, kA}).selected);
  REQUIRE(selection.anchor == SelectionRowId{kA});
  selection = Apply(rows, selection, kTemporary, SelectionGesture::kRange);
  REQUIRE(selection.selected == Selected({kA, kTemporary}).selected);
  REQUIRE(selection.anchor == SelectionRowId{kA});
  selection = Apply(rows, selection, kD, SelectionGesture::kToggle);
  REQUIRE(selection.selected == Selected({kD, kA, kTemporary}).selected);
  REQUIRE(selection.anchor == SelectionRowId{kD});
  selection = Apply(rows, selection, kFolder, SelectionGesture::kAddRange);
  REQUIRE(selection.selected ==
          Selected({kD, kFolder, kA, kTemporary}).selected);
  selection = Apply(rows, selection, kA, SelectionGesture::kToggle);
  REQUIRE(selection.selected == Selected({kD, kFolder, kTemporary}).selected);
  REQUIRE(selection.anchor == SelectionRowId{kA});
  selection = Apply(rows, selection, kA, SelectionGesture::kReplace);
  REQUIRE(selection.selected == Selected({kA}).selected);

  const auto no_anchor = Apply(rows, {}, kB, SelectionGesture::kRange);
  REQUIRE(no_anchor.selected == Selected({kB}).selected);
  REQUIRE(no_anchor.anchor == SelectionRowId{kB});
  const auto additive_no_anchor =
      Apply(rows, Selected({kD}), kB, SelectionGesture::kAddRange);
  REQUIRE(additive_no_anchor.selected == Selected({kD, kB}).selected);
}

void ReorderReconcileAndAccessibility() {
  std::vector<SelectionRowId> rows{kA, kB, kC, kD};
  auto selection = Apply(rows, {}, kB, SelectionGesture::kReplace);
  rows = {kD, kC, kB, kA};
  selection = Apply(rows, selection, kD, SelectionGesture::kRange);
  REQUIRE(selection.selected == Selected({kD, kC, kB}).selected);
  REQUIRE(selection.anchor == SelectionRowId{kB});
  auto projection = ProjectSelectedRows(rows, selection);
  REQUIRE(std::holds_alternative<std::vector<SelectedRow>>(projection));
  const auto& projected = std::get<std::vector<SelectedRow>>(projection);
  REQUIRE(projected.size() == rows.size());
  REQUIRE(projected[0].id == SelectionRowId{kD} && projected[0].selected);
  REQUIRE(projected[1].id == SelectionRowId{kC} && projected[1].selected);
  REQUIRE(projected[2].id == SelectionRowId{kB} && projected[2].selected);
  REQUIRE(projected[3].id == SelectionRowId{kA} && !projected[3].selected);
  selection = Apply(rows, selection, kC, SelectionGesture::kToggle);
  projection = ProjectSelectedRows(rows, selection);
  REQUIRE(!std::get<std::vector<SelectedRow>>(projection)[1].selected);

  const std::vector<SelectionRowId> reduced{kD, kA};
  REQUIRE(std::get<SelectionError>(ApplySelection(
              reduced, selection, kA, SelectionGesture::kReplace)) ==
          SelectionError::kStaleSelection);
  REQUIRE(std::get<SelectionError>(ProjectSelectedRows(reduced, selection)) ==
          SelectionError::kStaleSelection);
  const auto reconciled = ReconcileSelection(reduced, selection);
  REQUIRE(std::holds_alternative<Selection>(reconciled));
  const auto& current = std::get<Selection>(reconciled);
  REQUIRE(current.selected == Selected({kD}).selected);
  REQUIRE(!current.anchor);
  const auto empty = ReconcileSelection({}, selection);
  REQUIRE(std::get<Selection>(empty).selected.empty());
  REQUIRE(!std::get<Selection>(empty).anchor);
  REQUIRE(selection.selected == Selected({kD, kB}).selected);
  REQUIRE(selection.anchor == SelectionRowId{kC});
}

void SelectionBoundaryFailures() {
  const std::vector<SelectionRowId> rows{kA, kB};
  const auto prior = Selected({kA});
  REQUIRE(std::get<SelectionError>(
              ApplySelection(rows, prior, kC, SelectionGesture::kToggle)) ==
          SelectionError::kTargetNotFound);
  REQUIRE(std::get<SelectionError>(
              ApplySelection({kA, kA}, prior, kA, SelectionGesture::kRange)) ==
          SelectionError::kInvalidRows);
  REQUIRE(std::get<SelectionError>(ReconcileSelection(
              {lifecycle::ItemId{""}}, prior)) == SelectionError::kInvalidRows);
  REQUIRE(std::get<SelectionError>(ProjectSelectedRows({kA, kA}, prior)) ==
          SelectionError::kInvalidRows);
  // Same text across canonical identity types does not collapse identities.
  const std::vector<SelectionRowId> typed{lifecycle::ItemId{"same"},
                                          FolderId{"same"},
                                          lifecycle::TemporaryTabId{"same"}};
  const auto distinct = Apply(typed, {}, typed[0], SelectionGesture::kReplace);
  REQUIRE(distinct.selected.size() == 1);
  const auto projection = ProjectSelectedRows(typed, distinct);
  REQUIRE(!std::get<std::vector<SelectedRow>>(projection)[1].selected);
  REQUIRE(!std::get<std::vector<SelectedRow>>(projection)[2].selected);
  REQUIRE(prior.selected == Selected({kA}).selected);
}

struct Fixture {
  lifecycle::Model model;
  Folders folders;
  const std::vector<SelectionRowId> rows{
      kFolder, kA, kNested, kB, kDestination, kE, kC, kD, kU, kV, kTemporary};

  Fixture() {
    using lifecycle::Status;
    REQUIRE(model.AddProfile(kProfile) == Status::kOk);
    REQUIRE(model.AddProfile(kOtherProfile) == Status::kOk);
    REQUIRE(model.AddSpace(kSpace, kProfile) == Status::kOk);
    REQUIRE(model.AddSpace(kOtherSpace, kProfile) == Status::kOk);
    REQUIRE(model.AddSpace(kForeignSpace, kOtherProfile) == Status::kOk);
    REQUIRE(model.AddWindow(kWindow) == Status::kOk);
    REQUIRE(model.Attach({kWindow, kSpace}) == Status::kOk);
    for (const auto& item : {kA, kB, kC, kD, kE, kU, kV}) {
      REQUIRE(
          model.AddItem(item, kSpace, "https://selection.example.test/saved") ==
          Status::kOk);
    }
    REQUIRE(folders.CreateFolder(model, kFolder, "Folder",
                                 {{kSpace, std::nullopt}, std::nullopt}) ==
            FolderStatus::kOk);
    REQUIRE(folders.PlaceItem(model, kA, {{kSpace, kFolder}, std::nullopt}) ==
            FolderStatus::kOk);
    REQUIRE(folders.CreateFolder(model, kNested, "Nested",
                                 {{kSpace, kFolder}, std::nullopt}) ==
            FolderStatus::kOk);
    REQUIRE(folders.PlaceItem(model, kB, {{kSpace, kNested}, std::nullopt}) ==
            FolderStatus::kOk);
    REQUIRE(folders.CreateFolder(model, kDestination, "Destination",
                                 {{kSpace, std::nullopt}, std::nullopt}) ==
            FolderStatus::kOk);
    REQUIRE(
        folders.PlaceItem(model, kE, {{kSpace, kDestination}, std::nullopt}) ==
        FolderStatus::kOk);
    for (const auto& item : {kC, kD}) {
      REQUIRE(folders.PlaceItem(model, item,
                                {{kSpace, std::nullopt}, std::nullopt}) ==
              FolderStatus::kOk);
    }
    REQUIRE(std::holds_alternative<lifecycle::OpenedPage>(
        model.OpenItem(kA, {kWindow, kSpace}, kLive)));
    REQUIRE(model.RecordNavigation(
                kLive, "https://selection.example.test/form") == Status::kOk);
    REQUIRE(model.ReportActivity(kLive, lifecycle::Activity::kProtected) ==
            Status::kOk);
    REQUIRE(model.OpenTemporary(kTemporaryPage, kTemporary, {kWindow, kSpace},
                                "https://selection.example.test/temporary") ==
            Status::kOk);
  }

  SelectionMoveResult Plan(Selection selection, Placement destination) const {
    return PlanSelectionMove(model, folders, kSpace, rows, selection,
                             std::move(destination));
  }

  void ExpectSourceIntact() const {
    const auto snapshot = folders.Snapshot(model, kSpace);
    REQUIRE(std::holds_alternative<Hierarchy>(snapshot));
    const auto& tree = std::get<Hierarchy>(snapshot);
    REQUIRE(tree.roots == std::vector<Entry>({kFolder, kDestination, kC, kD}));
    REQUIRE(tree.folders.at(kFolder).children ==
            std::vector<Entry>({kA, kNested}));
    REQUIRE(tree.folders.at(kNested).children == std::vector<Entry>({kB}));
    REQUIRE(tree.folders.at(kDestination).children == std::vector<Entry>({kE}));
    REQUIRE(!tree.Locate(kU));
    REQUIRE(!tree.Locate(kV));
    REQUIRE(model.items().size() == 7);
    REQUIRE(model.pages().size() == 2);
    REQUIRE(model.items().at(kA).owner == lifecycle::ItemOwner{kSpace});
    REQUIRE(model.items().at(kA).saved_destination ==
            "https://selection.example.test/saved");
    REQUIRE(model.pages().at(kLive).profile == kProfile);
    REQUIRE(model.pages().at(kLive).current_navigation ==
            "https://selection.example.test/form");
    REQUIRE(model.pages().at(kLive).activity ==
            lifecycle::Activity::kProtected);
    REQUIRE(model.pages().at(kLive).attachment() ==
            (lifecycle::Attachment{kWindow, kSpace}));
    REQUIRE(
        std::get<lifecycle::SavedPage>(model.pages().at(kLive).owner).item ==
        kA);
    REQUIRE(model.pages().at(kTemporaryPage).attachment() ==
            (lifecycle::Attachment{kWindow, kSpace}));
    REQUIRE(std::get<lifecycle::TemporaryTab>(
                model.pages().at(kTemporaryPage).owner)
                .id == kTemporary);
  }
};

const SelectionMovePlan& PlanValue(const SelectionMoveResult& result) {
  REQUIRE(std::holds_alternative<SelectionMovePlan>(result));
  return std::get<SelectionMovePlan>(result);
}

void StableDestinationPreviewAndCancellation() {
  Fixture f;
  auto result = f.Plan(Selected({kC, kB}), {{kSpace, kDestination}, Entry{kE}});
  const auto& plan = PlanValue(result);
  REQUIRE(plan.entries == std::vector<Entry>({kB, kC}));
  REQUIRE(plan.destination_children == std::vector<Entry>({kB, kC, kE}));
  REQUIRE(plan.destination.container.folder == kDestination);
  REQUIRE(plan.destination.before == Entry{kE});
  f.ExpectSourceIntact();  // Cancel is simply discarding the preview.

  result = f.Plan(Selected({kC, kA, kFolder, kB}),
                  {{kSpace, kDestination}, std::nullopt});
  REQUIRE(PlanValue(result).entries == std::vector<Entry>({kFolder, kC}));
  REQUIRE(PlanValue(result).destination_children ==
          std::vector<Entry>({kE, kFolder, kC}));
  f.ExpectSourceIntact();

  // Live saved items can change organization without changing page/owner state.
  result = f.Plan(Selected({kA}), {{kSpace, kDestination}, std::nullopt});
  REQUIRE(PlanValue(result).destination_children ==
          std::vector<Entry>({kE, kA}));
  f.ExpectSourceIntact();

  result =
      f.Plan(Selected({kFolder, kC}), {{kSpace, std::nullopt}, Entry{kFolder}});
  REQUIRE(PlanValue(result).destination.before == Entry{kDestination});
  REQUIRE(PlanValue(result).destination_children ==
          std::vector<Entry>({kFolder, kC, kDestination, kD}));
  result = f.Plan(Selected({kC, kD}), {{kSpace, std::nullopt}, Entry{kC}});
  REQUIRE(!PlanValue(result).destination.before);
  REQUIRE(PlanValue(result).destination_children ==
          std::vector<Entry>({kFolder, kDestination, kC, kD}));
  f.ExpectSourceIntact();
}

void UnplacedItemsAndCanonicalOrder() {
  Fixture f;
  auto result =
      f.Plan(Selected({kU, kV}), {{kSpace, kDestination}, std::nullopt});
  REQUIRE(PlanValue(result).entries == std::vector<Entry>({kU, kV}));
  REQUIRE(PlanValue(result).destination_children ==
          std::vector<Entry>({kE, kU, kV}));
  auto rows = f.rows;
  std::swap(rows[8], rows[9]);  // Caller owns order for unplaced items.
  result =
      PlanSelectionMove(f.model, f.folders, kSpace, rows, Selected({kU, kV}),
                        {{kSpace, kDestination}, std::nullopt});
  REQUIRE(PlanValue(result).entries == std::vector<Entry>({kV, kU}));
  std::swap(rows[1], rows[3]);  // Represented hierarchy order is authoritative.
  result = PlanSelectionMove(f.model, f.folders, kSpace, rows, Selected({kA}),
                             {{kSpace, kDestination}, std::nullopt});
  REQUIRE(std::get<SelectionError>(result) == SelectionError::kOrderMismatch);
  rows = {kU, kFolder};
  result = PlanSelectionMove(f.model, f.folders, kSpace, rows, Selected({kU}),
                             {{kSpace, kDestination}, std::nullopt});
  REQUIRE(std::get<SelectionError>(result) == SelectionError::kOrderMismatch);
  // A collapsed or filtered caller view may omit descendants, preserving order.
  rows = {kFolder, kDestination, kC, kD};
  result =
      PlanSelectionMove(f.model, f.folders, kSpace, rows, Selected({kFolder}),
                        {{kSpace, kDestination}, std::nullopt});
  REQUIRE(PlanValue(result).entries == std::vector<Entry>({kFolder}));
  f.ExpectSourceIntact();
}

void WholeSelectionRefusals() {
  Fixture f;
  auto expect = [&](Selection selection, Placement destination,
                    SelectionError error) {
    REQUIRE(std::get<SelectionError>(f.Plan(selection, destination)) == error);
    f.ExpectSourceIntact();
  };
  const Placement destination{{kSpace, kDestination}, std::nullopt};
  expect({}, destination, SelectionError::kEmptySelection);
  expect(Selected({kTemporary}), destination,
         SelectionError::kTemporaryMoveUnsupported);
  expect(Selected({kB, kTemporary}), destination,
         SelectionError::kTemporaryMoveUnsupported);
  expect(Selected({kC, lifecycle::ItemId{"missing"}}), destination,
         SelectionError::kStaleSelection);
  expect(Selected({kFolder}), {{kSpace, kNested}, std::nullopt},
         SelectionError::kDestinationInsideSelection);
  expect(Selected({kFolder}), {{kSpace, kFolder}, std::nullopt},
         SelectionError::kDestinationInsideSelection);
  expect(Selected({kC}), {{kSpace, FolderId{"missing"}}, std::nullopt},
         SelectionError::kInvalidDestination);
  expect(Selected({kC}), {{kSpace, kDestination}, Entry{kA}},
         SelectionError::kInvalidDestination);
  expect(Selected({kC}),
         {{lifecycle::SpaceId{"missing"}, std::nullopt}, std::nullopt},
         SelectionError::kInvalidDestination);
  expect(Selected({kC}), {{kForeignSpace, std::nullopt}, std::nullopt},
         SelectionError::kCrossProfileMigrationUnsupported);
  expect(Selected({kC}), {{kOtherSpace, std::nullopt}, std::nullopt},
         SelectionError::kCrossSpaceUnsupported);
  expect(Selected({kA, kC}), {{kOtherSpace, std::nullopt}, std::nullopt},
         SelectionError::kLiveItemOwnershipMoveUnsupported);
  expect(Selected({kFolder}), {{kOtherSpace, std::nullopt}, std::nullopt},
         SelectionError::kLiveItemOwnershipMoveUnsupported);

  auto result = PlanSelectionMove(f.model, f.folders, kSpace, {kC, kC},
                                  Selected({kC}), destination);
  REQUIRE(std::get<SelectionError>(result) == SelectionError::kInvalidRows);
  result = PlanSelectionMove(
      f.model, f.folders, kSpace, {lifecycle::ItemId{"missing"}},
      Selected({lifecycle::ItemId{"missing"}}), destination);
  REQUIRE(std::get<SelectionError>(result) == SelectionError::kSourceMismatch);
  f.ExpectSourceIntact();
}

void StalePlanRequiresFreshValidation() {
  Fixture f;
  auto result = f.Plan(Selected({kC}), {{kSpace, kDestination}, Entry{kE}});
  const auto old_plan = PlanValue(result);
  REQUIRE(f.folders.PlaceItem(f.model, kE, {{kSpace, kNested}, std::nullopt}) ==
          FolderStatus::kOk);
  result = f.Plan(Selected({kC}), old_plan.destination);
  // Caller rows are stale first, before the destination can be reused.
  REQUIRE(std::get<SelectionError>(result) == SelectionError::kOrderMismatch);
  const std::vector<SelectionRowId> fresh_rows{
      kFolder, kA, kNested, kB, kE, kDestination, kC, kD, kU, kV};
  result = PlanSelectionMove(f.model, f.folders, kSpace, fresh_rows,
                             Selected({kC}), old_plan.destination);
  REQUIRE(std::get<SelectionError>(result) ==
          SelectionError::kInvalidDestination);
  REQUIRE(old_plan.destination_children == std::vector<Entry>({kC, kE}));
  REQUIRE(f.model.items().at(kC).owner == lifecycle::ItemOwner{kSpace});

  Fixture stale;
  REQUIRE(stale.model.MoveItem(kC, kOtherSpace) == lifecycle::Status::kOk);
  result = stale.Plan(Selected({kB}), {{kSpace, kDestination}, std::nullopt});
  REQUIRE(std::get<FolderStatus>(result) == FolderStatus::kItemOwnerMismatch);
  REQUIRE(stale.model.items().at(kC).owner ==
          lifecycle::ItemOwner{kOtherSpace});
  REQUIRE(stale.model.pages().at(kLive).activity ==
          lifecycle::Activity::kProtected);
}

}  // namespace
}  // namespace arcified::organization

int main() {
  using namespace arcified::organization;
  RangeAndAdditiveSelection();
  ReorderReconcileAndAccessibility();
  SelectionBoundaryFailures();
  StableDestinationPreviewAndCancellation();
  UnplacedItemsAndCanonicalOrder();
  WholeSelectionRefusals();
  StalePlanRequiresFreshValidation();
  std::cout << "multi-select synthetic checks passed\n";
}
