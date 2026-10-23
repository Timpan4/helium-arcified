#ifndef ARCIFIED_NATIVE_FEATURES_ORGANIZATION_MULTI_SELECT_H_
#define ARCIFIED_NATIVE_FEATURES_ORGANIZATION_MULTI_SELECT_H_

#include <optional>
#include <set>
#include <variant>
#include <vector>

#include "folders.h"

namespace arcified::organization {

using SelectionRowId =
    std::variant<FolderId, lifecycle::ItemId, lifecycle::TemporaryTabId>;

// Transient selection only. The anchor is for range gestures, never focus or
// the active page. The caller owns row order, visibility and native focus.
struct Selection {
  std::set<SelectionRowId> selected;
  std::optional<SelectionRowId> anchor;
};

enum class SelectionGesture { kReplace, kToggle, kRange, kAddRange };
enum class SelectionError {
  kInvalidRows,
  kTargetNotFound,
  kStaleSelection,
  kEmptySelection,
  kSourceMismatch,
  kOrderMismatch,
  kTemporaryMoveUnsupported,
  kCrossSpaceUnsupported,
  kCrossProfileMigrationUnsupported,
  kLiveItemOwnershipMoveUnsupported,
  kInvalidDestination,
  kDestinationInsideSelection,
};

using SelectionResult = std::variant<SelectionError, Selection>;

// Rows are unique, nonempty identities in authoritative current view order.
// A stale selected identity or anchor refuses without changing the input.
// Replace/toggle establish an anchor, range replaces the selected set, and
// additive range unions it. Ranges include both endpoints and keep the anchor;
// without an anchor they select the target. The native adapter maps platform
// modifiers to gestures, including Command on macOS and Control elsewhere.
SelectionResult ApplySelection(const std::vector<SelectionRowId>& rows,
                               const Selection& selection,
                               SelectionRowId target,
                               SelectionGesture gesture);

// Explicit view-change reconciliation intersects selection with current rows
// and clears an absent anchor. Never moves focus or selects a fallback page.
SelectionResult ReconcileSelection(const std::vector<SelectionRowId>& rows,
                                   const Selection& selection);

struct SelectedRow {
  SelectionRowId id;
  bool selected;
};
using SelectedRowsResult =
    std::variant<SelectionError, std::vector<SelectedRow>>;

// Native accessibility adapters publish selected state for every row,
// including false after deselection. Roles, focus and announcements stay
// native.
SelectedRowsResult ProjectSelectedRows(const std::vector<SelectionRowId>& rows,
                                       const Selection& selection);

struct SelectionMovePlan {
  lifecycle::SpaceId source_space;
  // Canonical source order; descendants of selected folders appear only via
  // their selected ancestor. These are references, never copied page state.
  std::vector<Entry> entries;
  Placement destination;
  // Direct destination children after the proposed move. Relative order of
  // unaffected siblings is retained. A before anchor within the moving roots
  // resolves to the next unaffected sibling, or append if none remains.
  std::vector<Entry> destination_children;
};

using SelectionMoveResult =
    std::variant<SelectionError, FolderStatus, SelectionMovePlan>;

// Pure same-Space organization preview. Reads a freshly validated Snapshot;
// represented rows must follow hierarchy preorder, followed by unplaced saved
// items in caller-supplied canonical order. Temporaries retain caller order for
// selection but cannot move, including in a mixed batch. Live saved items may
// change folder membership/order within their owning Space, never ownership.
// Selected unplaced items can be placed; omitted rows are not deleted.
//
// This owned snapshot is neither a reservation nor an actuation guarantee.
// Cancellation means discarding it. It mutates nothing and provides NO batch
// executor: do not loop through single-item mutations and claim atomic success.
// Before any future commit, re-read lifecycle, hierarchy, source order and
// destination, and establish a coordinated failure/cancellation boundary.
SelectionMoveResult PlanSelectionMove(const lifecycle::Model& model,
                                      const Folders& folders,
                                      lifecycle::SpaceId source_space,
                                      const std::vector<SelectionRowId>& rows,
                                      const Selection& selection,
                                      Placement destination);

}  // namespace arcified::organization

#endif  // ARCIFIED_NATIVE_FEATURES_ORGANIZATION_MULTI_SELECT_H_
