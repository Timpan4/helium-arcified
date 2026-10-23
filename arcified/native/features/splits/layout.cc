#include "layout.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <utility>

namespace arcified::splits {
namespace {

bool Contains(const lifecycle::Split& split, const lifecycle::PageId& page) {
  return std::find(split.pages.begin(), split.pages.end(), page) !=
         split.pages.end();
}

}  // namespace

MetadataResult ReconcileLayout(const lifecycle::Model& model,
                               const lifecycle::SplitId& split,
                               const LayoutMetadata& metadata) {
  const auto found = model.splits().find(split);
  if (found == model.splits().end())
    return LayoutError::kSplitNotFound;

  switch (metadata.orientation) {
    case Orientation::kHorizontal:
    case Orientation::kVertical:
      break;
    default:
      return LayoutError::kInvalidOrientation;
  }

  LayoutMetadata reconciled;
  reconciled.orientation = metadata.orientation;
  for (const auto& page : found->second.pages) {
    const auto entry = metadata.weights.find(page);
    const double weight = entry == metadata.weights.end() ? 1.0 : entry->second;
    if (!std::isfinite(weight) || weight <= 0.0)
      return LayoutError::kInvalidWeight;
    reconciled.weights.emplace(page, weight);
  }
  return reconciled;
}

LayoutResult ResolveLayout(const lifecycle::Model& model,
                           const lifecycle::SplitId& split,
                           const LayoutMetadata& metadata,
                           std::optional<lifecycle::PageId> focus_target) {
  auto result = ReconcileLayout(model, split, metadata);
  if (const auto* error = std::get_if<LayoutError>(&result))
    return *error;

  const auto& canonical = model.splits().at(split);
  if (focus_target && !Contains(canonical, *focus_target))
    return LayoutError::kPageNotInSplit;

  const auto& reconciled = std::get<LayoutMetadata>(result);
  LayoutView view{split,
                  canonical.attachment,
                  reconciled.orientation,
                  {},
                  std::move(focus_target)};
  for (const auto& page : canonical.pages)
    view.panes.push_back(Pane{page, reconciled.weights.at(page)});
  return view;
}

ReorderResult PlanReorder(const lifecycle::Model& model,
                          const lifecycle::SplitId& split,
                          std::vector<lifecycle::PageId> order,
                          std::optional<lifecycle::PageId> focus_target) {
  const auto found = model.splits().find(split);
  if (found == model.splits().end())
    return LayoutError::kSplitNotFound;
  const auto& canonical = found->second;
  if (order.size() != canonical.pages.size())
    return LayoutError::kInvalidOrder;

  std::set<lifecycle::PageId> remaining(canonical.pages.begin(),
                                        canonical.pages.end());
  for (const auto& page : order) {
    if (remaining.erase(page) != 1)
      return LayoutError::kInvalidOrder;
  }
  if (focus_target && !Contains(canonical, *focus_target))
    return LayoutError::kPageNotInSplit;
  return SplitReorder{split, canonical.attachment, std::move(order),
                      std::move(focus_target)};
}

UnsplitResult PlanUnsplit(const lifecycle::Model& model,
                          const lifecycle::SplitId& split,
                          lifecycle::PageId focus_target) {
  const auto found = model.splits().find(split);
  if (found == model.splits().end())
    return LayoutError::kSplitNotFound;
  if (!Contains(found->second, focus_target))
    return LayoutError::kPageNotInSplit;
  return UnsplitPlan{split, std::move(focus_target)};
}

}  // namespace arcified::splits
