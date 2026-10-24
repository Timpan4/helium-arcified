#ifndef ARCIFIED_NATIVE_FEATURES_SPLITS_LAYOUT_H_
#define ARCIFIED_NATIVE_FEATURES_SPLITS_LAYOUT_H_

#include <map>
#include <optional>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::splits {

// Horizontal places panes left to right; vertical places them top to bottom.
enum class Orientation { kHorizontal, kVertical };

// Presentation metadata only. Model::splits() owns membership and order.
// Weights are finite positive relative values, with no required sum. A missing
// current member gets the dimensionless identity 1.0, equal to other defaulted
// panes, not necessarily to explicitly weighted survivors. No pane cap applies.
struct LayoutMetadata {
  Orientation orientation = Orientation::kHorizontal;
  std::map<lifecycle::PageId, double> weights;
};

enum class LayoutError {
  kSplitNotFound,
  kInvalidOrientation,
  kInvalidWeight,
  kPageNotInSplit,
  kInvalidOrder,
};

struct Pane {
  lifecycle::PageId page;
  double weight;
};

// Owned projection for one call, never a membership or focus registry. Discard
// after a model mutation. Focus is supplied by the external focus owner.
struct LayoutView {
  lifecycle::SplitId split;
  lifecycle::Attachment attachment;
  Orientation orientation;
  std::vector<Pane> panes;
  std::optional<lifecycle::PageId> focus_target;
};

using MetadataResult = std::variant<LayoutError, LayoutMetadata>;
using LayoutResult = std::variant<LayoutError, LayoutView>;

// Prunes stale keys, including invalid stale weights, and supplies missing
// weights. Rejects invalid orientation or invalid current-member weights.
// A missing split returns kSplitNotFound; the metadata owner may drop its
// entry.
MetadataResult ReconcileLayout(const lifecycle::Model& model,
                               const lifecycle::SplitId& split,
                               const LayoutMetadata& metadata);

// Projects canonical lifecycle order with reconciled metadata. An explicit
// focus target must be a current member. No target means no focus proposal;
// this function never chooses a fallback or updates the focus owner.
LayoutResult ResolveLayout(
    const lifecycle::Model& model,
    const lifecycle::SplitId& split,
    const LayoutMetadata& metadata,
    std::optional<lifecycle::PageId> focus_target = std::nullopt);

// Transient command snapshots, not reservations or native completion records.
// Revalidate against the model and native liveness immediately before acting.
struct SplitReorder {
  lifecycle::SplitId split;
  lifecycle::Attachment attachment;
  std::vector<lifecycle::PageId> pages;
  std::optional<lifecycle::PageId> focus_target;
};
using ReorderResult = std::variant<LayoutError, SplitReorder>;

// Accepts an exact permutation of canonical membership. Relative weights stay
// keyed by PageId; reconcile metadata after actual SetSplit success. Does not
// mutate lifecycle, move pages, navigate or actuate native focus.
ReorderResult PlanReorder(
    const lifecycle::Model& model,
    const lifecycle::SplitId& split,
    std::vector<lifecycle::PageId> order,
    std::optional<lifecycle::PageId> focus_target = std::nullopt);

struct UnsplitPlan {
  lifecycle::SplitId split;
  lifecycle::PageId focus_target;
};
using UnsplitResult = std::variant<LayoutError, UnsplitPlan>;

// Caller explicitly chooses a current member to present after unsplitting.
// Removing the layout via Model::RemoveSplit leaves every page alive. This
// proposal never closes a pane/page or decides close/activity policy.
UnsplitResult PlanUnsplit(const lifecycle::Model& model,
                          const lifecycle::SplitId& split,
                          lifecycle::PageId focus_target);

}  // namespace arcified::splits

#endif  // ARCIFIED_NATIVE_FEATURES_SPLITS_LAYOUT_H_
