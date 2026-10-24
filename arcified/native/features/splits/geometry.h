#ifndef ARCIFIED_NATIVE_FEATURES_SPLITS_GEOMETRY_H_
#define ARCIFIED_NATIVE_FEATURES_SPLITS_GEOMETRY_H_

#include "layout.h"

namespace arcified::splits {

// Caller-selected units, shared by all dimensions, deltas and coordinates.
struct Size {
  double width;
  double height;
};

struct Point {
  double x;
  double y;
};

struct Rect {
  double x;
  double y;
  double width;
  double height;
};

struct GeometryInput {
  Size viewport;
  Size minimum_pane;
  double divider_extent;
  lifecycle::PageId active_page;
};

enum class GeometryError {
  kInvalidDimensions,
  kInvalidDirection,
  kInvalidResize,
  kResizeUnavailable,
};

enum class GeometryMode { kAllPanes, kActiveOnly };

struct PaneBounds {
  lifecycle::PageId page;
  Rect bounds;
};

struct DividerBounds {
  lifecycle::PageId before;
  lifecycle::PageId after;
  Rect bounds;
};

// Disposable projection, not a focus/membership owner. Recompute after model,
// metadata, viewport or caller selection changes. Bounds are viewport-local.
struct Geometry {
  GeometryMode mode;
  std::vector<PaneBounds> panes;
  std::vector<DividerBounds> dividers;
};

using GeometryResult = std::variant<LayoutError, GeometryError, Geometry>;

// Viewport dimensions and divider extent must be finite and nonnegative;
// minimum pane dimensions must be finite and positive. Each pane receives a
// proportional share subject to the supplied minimum. If panes and dividers
// cannot fit on either axis, only the explicit active page fills the viewport.
// This fallback never changes membership, weights, selection or scrolling.
GeometryResult ProjectGeometry(const lifecycle::Model& model,
                               const lifecycle::SplitId& split,
                               const LayoutMetadata& metadata,
                               const GeometryInput& input);

using ResizeResult = std::
    variant<LayoutError, GeometryError, std::map<lifecycle::PageId, double>>;

// Delta is measured along the layout axis from the current divider position.
// Positive grows the preceding pane. Clamps to caller minima and returns new
// relative weights for all current panes, preserving nonadjacent extents.
// Does not apply metadata or mutate the model. No resize in active-only mode.
ResizeResult ProjectResize(const lifecycle::Model& model,
                           const lifecycle::SplitId& split,
                           const LayoutMetadata& metadata,
                           const GeometryInput& input,
                           const lifecycle::PageId& before,
                           double delta);

enum class Direction { kLeft, kRight, kUp, kDown };
using KeyboardTargetResult =
    std::variant<LayoutError, GeometryError, std::optional<lifecycle::PageId>>;

// Physical directions follow canonical order on the layout axis, without
// wrapping. Perpendicular directions and endpoints have no target. Hidden
// panes remain reachable during fallback; reproject with the selected target.
// The caller owns actual native/accessibility focus and key bindings.
KeyboardTargetResult KeyboardTarget(const lifecycle::Model& model,
                                    const lifecycle::SplitId& split,
                                    const LayoutMetadata& metadata,
                                    const lifecycle::PageId& active_page,
                                    Direction direction);

// Returns an existing visible pane and highlight bounds. Half-open edges keep
// divider gaps and the viewport's outer edge out of the target. A target grants
// no permission to navigate, create, move or close a page. Native callers must
// revalidate the snapshot and apply browser link/drop/security policy.
std::optional<PaneBounds> PaneTargetAt(const Geometry& geometry, Point point);

}  // namespace arcified::splits

#endif  // ARCIFIED_NATIVE_FEATURES_SPLITS_GEOMETRY_H_
