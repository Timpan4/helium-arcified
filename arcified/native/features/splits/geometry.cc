#include "geometry.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace arcified::splits {
namespace {

bool Nonnegative(double value) {
  return std::isfinite(value) && value >= 0;
}

double Along(Size size, Orientation orientation) {
  return orientation == Orientation::kHorizontal ? size.width : size.height;
}

double Across(Size size, Orientation orientation) {
  return orientation == Orientation::kHorizontal ? size.height : size.width;
}

double Extent(const Rect& bounds, Orientation orientation) {
  return orientation == Orientation::kHorizontal ? bounds.width : bounds.height;
}

Rect Bounds(double start,
            double length,
            double cross,
            Orientation orientation) {
  if (orientation == Orientation::kHorizontal)
    return {start, 0, length, cross};
  return {0, start, cross, length};
}

// Evaluate available * weight / (scale * remaining_weight) without first
// rounding weight / scale to a subnormal or zero. The frexp mantissas keep
// intermediate products finite; only the final scalbn can underflow. This
// needs no wider floating-point type (long double is double on MSVC).
double ProportionalExtent(double available,
                          double weight,
                          double scale,
                          double remaining_weight) {
  int available_exponent;
  int weight_exponent;
  int scale_exponent;
  int sum_exponent;
  const double available_mantissa = std::frexp(available, &available_exponent);
  const double weight_mantissa = std::frexp(weight, &weight_exponent);
  const double scale_mantissa = std::frexp(scale, &scale_exponent);
  const double sum_mantissa = std::frexp(remaining_weight, &sum_exponent);
  return std::scalbn(
      (available_mantissa / sum_mantissa) * (weight_mantissa / scale_mantissa),
      available_exponent + weight_exponent - scale_exponent - sum_exponent);
}

// Finite, count-bounded allocation, not iterative numerical refinement. Lock
// undersized panes in ascending weight order, then apportion the remainder.
std::vector<double> Allocate(const std::vector<Pane>& panes,
                             double available,
                             double minimum) {
  std::vector<std::size_t> order(panes.size());
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
    return panes[a].weight < panes[b].weight;
  });
  const double maximum = panes[order.back()].weight;
  double remaining_weight = 0;
  for (const auto& pane : panes)
    remaining_weight += pane.weight;
  // Keep extent weights unchanged when their finite sum is the available
  // space, as on resize reprojection. Normalize only a sum that overflows.
  const double scale = std::isfinite(remaining_weight) ? 1.0 : maximum;
  std::vector<double> weights;
  remaining_weight = 0;
  for (const auto& pane : panes) {
    weights.push_back(pane.weight / scale);
    remaining_weight += weights.back();
  }
  std::vector<double> lengths(panes.size());
  for (std::size_t i = 0; i < order.size(); ++i) {
    const auto index = order[i];
    const double proportional = ProportionalExtent(
        available, panes[index].weight, scale, remaining_weight);
    if (proportional < minimum) {
      lengths[index] = minimum;
      available -= minimum;
      remaining_weight -= weights[index];
    } else {
      for (; i < order.size(); ++i) {
        const auto remaining = order[i];
        lengths[remaining] = ProportionalExtent(
            available, panes[remaining].weight, scale, remaining_weight);
      }
      break;
    }
  }
  return lengths;
}

}  // namespace

GeometryResult ProjectGeometry(const lifecycle::Model& model,
                               const lifecycle::SplitId& split,
                               const LayoutMetadata& metadata,
                               const GeometryInput& input) {
  const auto result = ResolveLayout(model, split, metadata, input.active_page);
  if (const auto* error = std::get_if<LayoutError>(&result))
    return *error;
  if (!Nonnegative(input.viewport.width) ||
      !Nonnegative(input.viewport.height) ||
      !Nonnegative(input.divider_extent) ||
      !std::isfinite(input.minimum_pane.width) ||
      !std::isfinite(input.minimum_pane.height) ||
      input.minimum_pane.width <= 0 || input.minimum_pane.height <= 0) {
    return GeometryError::kInvalidDimensions;
  }
  const auto& view = std::get<LayoutView>(result);
  const double main = Along(input.viewport, view.orientation);
  const double cross = Across(input.viewport, view.orientation);
  const double minimum = Along(input.minimum_pane, view.orientation);
  const double available =
      main - input.divider_extent * (view.panes.size() - 1);
  const Geometry fallback{
      GeometryMode::kActiveOnly,
      {{input.active_page,
        {0, 0, input.viewport.width, input.viewport.height}}},
      {}};
  if (available / view.panes.size() < minimum ||
      cross < Across(input.minimum_pane, view.orientation)) {
    return fallback;
  }

  const auto lengths = Allocate(view.panes, available, minimum);
  // Reserve suffix minima before placing weighted boundaries. Each subtraction
  // can round toward the forbidden side; its adjacent representable value is
  // then a conservative bound. No epsilon or refinement loop is required.
  std::vector<double> latest_end(view.panes.size());
  latest_end.back() = main;
  for (std::size_t i = view.panes.size() - 1; i > 0; --i) {
    double start = latest_end[i] - minimum;
    if (latest_end[i] - start < minimum)
      start = std::nextafter(start, -std::numeric_limits<double>::infinity());
    double end = start - input.divider_extent;
    if (end + input.divider_extent > start)
      end = std::nextafter(end, -std::numeric_limits<double>::infinity());
    if (end < 0)
      return fallback;
    latest_end[i - 1] = end;
  }
  Geometry geometry{GeometryMode::kAllPanes, {}, {}};
  double position = 0;
  for (std::size_t i = 0; i < view.panes.size(); ++i) {
    // A desired extent that cannot advance its origin is unrepresentable. Do
    // not inflate it to the local coordinate spacing merely to avoid fallback.
    const double desired_end = position + lengths[i];
    if (desired_end == position)
      return fallback;
    double earliest_end = position + minimum;
    if (earliest_end - position < minimum) {
      earliest_end =
          std::nextafter(earliest_end, std::numeric_limits<double>::infinity());
    }
    if (earliest_end > latest_end[i])
      return fallback;
    // Clamp within representable bounds so the last pane consumes residue
    // without losing a minimum to the preceding pane's rounded endpoint.
    const double end =
        i + 1 == view.panes.size()
            ? main
            : std::clamp(desired_end, earliest_end, latest_end[i]);
    geometry.panes.push_back(
        {view.panes[i].page,
         Bounds(position, end - position, cross, view.orientation)});
    if (i + 1 < view.panes.size()) {
      const double divider_end = std::min(main, end + input.divider_extent);
      geometry.dividers.push_back(
          {view.panes[i].page, view.panes[i + 1].page,
           Bounds(end, divider_end - end, cross, view.orientation)});
      position = divider_end;
    }
  }
  return geometry;
}

ResizeResult ProjectResize(const lifecycle::Model& model,
                           const lifecycle::SplitId& split,
                           const LayoutMetadata& metadata,
                           const GeometryInput& input,
                           const lifecycle::PageId& before,
                           double delta) {
  const auto result = ProjectGeometry(model, split, metadata, input);
  if (const auto* error = std::get_if<LayoutError>(&result))
    return *error;
  if (const auto* error = std::get_if<GeometryError>(&result))
    return *error;
  if (!std::isfinite(delta))
    return GeometryError::kInvalidResize;
  const auto& geometry = std::get<Geometry>(result);
  if (geometry.mode == GeometryMode::kActiveOnly)
    return GeometryError::kResizeUnavailable;
  const auto divider =
      std::find_if(geometry.dividers.begin(), geometry.dividers.end(),
                   [&](const DividerBounds& candidate) {
                     return candidate.before == before;
                   });
  if (divider == geometry.dividers.end())
    return GeometryError::kResizeUnavailable;
  const auto index =
      static_cast<std::size_t>(divider - geometry.dividers.begin());
  const auto orientation = metadata.orientation;
  const double preceding = Extent(geometry.panes[index].bounds, orientation);
  const double following =
      Extent(geometry.panes[index + 1].bounds, orientation);
  const double minimum = Along(input.minimum_pane, orientation);
  // Clamp the delta before addition to avoid overflow from a finite huge drag.
  std::map<lifecycle::PageId, double> weights;
  for (const auto& pane : geometry.panes)
    weights.emplace(pane.page, Extent(pane.bounds, orientation));
  if (delta <= minimum - preceding) {
    weights.at(before) = minimum;
    weights.at(divider->after) = following + (preceding - minimum);
  } else if (delta >= following - minimum) {
    weights.at(before) = preceding + (following - minimum);
    weights.at(divider->after) = minimum;
  } else {
    weights.at(before) = preceding + delta;
    weights.at(divider->after) = following - delta;
  }
  return weights;
}

KeyboardTargetResult KeyboardTarget(const lifecycle::Model& model,
                                    const lifecycle::SplitId& split,
                                    const LayoutMetadata& metadata,
                                    const lifecycle::PageId& active_page,
                                    Direction direction) {
  const auto result = ResolveLayout(model, split, metadata, active_page);
  if (const auto* error = std::get_if<LayoutError>(&result))
    return *error;
  const auto& view = std::get<LayoutView>(result);
  bool previous;
  bool horizontal;
  switch (direction) {
    case Direction::kLeft:
      previous = true;
      horizontal = true;
      break;
    case Direction::kRight:
      previous = false;
      horizontal = true;
      break;
    case Direction::kUp:
      previous = true;
      horizontal = false;
      break;
    case Direction::kDown:
      previous = false;
      horizontal = false;
      break;
    default:
      return GeometryError::kInvalidDirection;
  }
  if (horizontal != (view.orientation == Orientation::kHorizontal))
    return std::optional<lifecycle::PageId>{};
  const auto active =
      std::find_if(view.panes.begin(), view.panes.end(),
                   [&](const Pane& pane) { return pane.page == active_page; });
  if ((previous && active == view.panes.begin()) ||
      (!previous && active + 1 == view.panes.end())) {
    return std::optional<lifecycle::PageId>{};
  }
  return std::optional<lifecycle::PageId>{
      (previous ? active - 1 : active + 1)->page};
}

std::optional<PaneBounds> PaneTargetAt(const Geometry& geometry, Point point) {
  if (!std::isfinite(point.x) || !std::isfinite(point.y))
    return std::nullopt;
  for (const auto& pane : geometry.panes) {
    const auto& bounds = pane.bounds;
    if (point.x >= bounds.x && point.y >= bounds.y &&
        point.x - bounds.x < bounds.width &&
        point.y - bounds.y < bounds.height) {
      return pane;
    }
  }
  return std::nullopt;
}

}  // namespace arcified::splits
