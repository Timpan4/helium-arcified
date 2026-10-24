#ifndef ARCIFIED_NATIVE_FEATURES_ORDERED_NAVIGATION_ORDERED_NAVIGATION_H_
#define ARCIFIED_NATIVE_FEATURES_ORDERED_NAVIGATION_ORDERED_NAVIGATION_H_

#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::ordered_navigation {

using TabReference =
    std::variant<lifecycle::ItemId, lifecycle::ShortcutId, lifecycle::PageId>;
using FolderReference = lifecycle::Id<struct FolderReferenceTag>;
using SnapshotRevision = lifecycle::Id<struct SnapshotRevisionTag>;

// A flattened presentation projection, not a stored tree. Include descendants
// of collapsed folders and each presented split pane in sidebar order. Folder
// references are opaque to this component and belong to the organization owner.
struct Row {
  TabReference target;
  std::vector<FolderReference> collapsed_ancestor_ids;  // Outer to inner.
  std::optional<lifecycle::SplitId> split;
};

// Caller publishes these inputs coherently with the read-only model. Revision
// is nonempty, never reused, and changes on any relevant order, lifecycle,
// selection, organization, or layout change. No clock or counter lives here.
struct SidebarSnapshot {
  SnapshotRevision revision;
  lifecycle::Attachment scope;
  std::vector<Row> tabs;
  std::vector<lifecycle::SpaceId> space_order;
};

enum class Direction { kNext, kPrevious };
enum class EdgeBehavior { kStop, kWrap };
struct RelativeTab {
  TabReference anchor;  // Current row from caller-authoritative selection.
  Direction direction;
  EdgeBehavior edge;
};
struct DirectTab {
  TabReference target;
};
struct DirectIndex {
  std::size_t index;  // Zero based. No fixed direct-selection capacity.
};
using TabRequest = std::variant<RelativeTab, DirectTab, DirectIndex>;

struct RelativeSpace {
  Direction direction;
  EdgeBehavior edge;
};
struct DirectSpace {
  lifecycle::SpaceId space;
};
using SpaceRequest = std::variant<RelativeSpace, DirectSpace, DirectIndex>;

enum class Refusal {
  kInvalidScope,
  kInvalidProjection,
  kNotVisible,
  kEmpty,
  kAnchorMissing,
  kAtBoundary,
  kIndexOutOfBounds,
  kStaleSnapshot,
};

// Describes a destination only. Optional live_page identifies the existing
// page, including one in another window. It never requests a cloned page.
// reveal_row describes the originating sidebar row; destination may differ
// for an alias to a kept item in another Space. Reveal is not focus completion.
struct TabDestination {
  Row reveal_row;
  lifecycle::Attachment destination;
  lifecycle::ProfileId profile;
  std::optional<lifecycle::ItemId> item;
  std::optional<lifecycle::PageId> live_page;
  std::optional<lifecycle::Attachment> live_attachment;
};
struct SpaceDestination {
  lifecycle::Attachment destination;
  lifecycle::ProfileId profile;
};
struct DestinationPlan {
  SnapshotRevision revision;
  lifecycle::Attachment scope;
  std::variant<TabDestination, SpaceDestination> destination;
};
using SelectionResult = std::variant<Refusal, DestinationPlan>;

// Pure queries. Neither query reads MRU, current navigation, labels or URLs,
// allocates identities, mutates the model, nor executes a native command.
SelectionResult SelectTab(const lifecycle::Model& model,
                          const SidebarSnapshot& snapshot,
                          const TabRequest& request);
SelectionResult SelectSpace(const lifecycle::Model& model,
                            const SidebarSnapshot& snapshot,
                            const SpaceRequest& request);

// Reject a changed revision; also re-resolve identity/visibility/layout against
// the current model. This is not a native reservation or focus authorization.
// The external commands adapter and native executor must still revalidate.
SelectionResult Revalidate(const lifecycle::Model& model,
                           const SidebarSnapshot& current,
                           const DestinationPlan& plan);

}  // namespace arcified::ordered_navigation

#endif  // ARCIFIED_NATIVE_FEATURES_ORDERED_NAVIGATION_ORDERED_NAVIGATION_H_
