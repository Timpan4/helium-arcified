#ifndef ARCIFIED_NATIVE_FEATURES_REGION_CAPTURE_REGION_CAPTURE_H_
#define ARCIFIED_NATIVE_FEATURES_REGION_CAPTURE_REGION_CAPTURE_H_

#include "../../model/lifecycle.h"

#include <optional>
#include <string>
#include <variant>

namespace arcified::region_capture {

// Coordinates in the native picker overlay's DIPs, never CSS or screen pixels.
struct DipPoint {
  double x;
  double y;
};
struct DipRect {
  double left;
  double top;
  double right;
  double bottom;
};
struct PixelSize {
  int width;
  int height;
};
// Half-open edges in the exact raster described by RasterMapping.
struct PixelRect {
  int left;
  int top;
  int right;
  int bottom;
};

struct RasterMapping {
  // Axis-aligned extent of the exact source raster in native overlay DIPs.
  // Host resolves pane placement, device scale and page zoom. No CSS=DIP rule.
  DipRect raster_bounds;
  // Visible, selectable rectangle inside raster_bounds. Nonrectangular
  // occlusion requires the host to deny capture, not approximate this clip.
  DipRect visible_bounds;
  PixelSize raster_size;
};

struct Target {
  lifecycle::PageId page;
  lifecycle::ProfileId profile;
  lifecycle::Attachment attachment;
  std::optional<lifecycle::SplitId> split;
  // Host-owned, nonempty opaque tokens. Changes invalidate the selection.
  std::string committed_document;
  // Covers scroll, layout, zoom, surface and WebContents replacement.
  std::string visual_revision;
};

// Authoritative host decision for this target, never inferred from its URL.
// Sensitive/unknown content is refused until the host can explicitly allow it.
enum class Permission { kUnknown, kAllowed, kDenied, kSensitiveContent };
struct PickerRequest {
  Target target;
  RasterMapping mapping;
  Permission permission = Permission::kUnknown;
};

enum class Status {
  kOk,
  kInvalidTarget,
  kInvalidGeometry,
  kEmptySelection,
  kPermissionRequired,
  kPermissionDenied,
  kSensitiveContent,
  kWrongPhase,
  kTargetChanged,
  kCancelled,
};
struct RegionGeometry {
  PixelRect crop;
  // Draw this snapped boundary, not the raw pointer rectangle. It is the
  // floating-point representation of the crop's native overlay DIP edges.
  // Both it and the exact affine pixel extent are contained in the selection.
  DipRect displayed_boundary;
};
using GeometryResult = std::variant<Status, RegionGeometry>;

// Normalize reversed corners, clip to visible content, then round inward.
// Empty/subpixel selections never become an empty ("capture all") native rect.
// Refuse candidates whose containment cannot be proved in double arithmetic.
// Requires IEEE binary floating point, FLT_EVAL_METHOD == 0, gradual underflow
// and no fast-math; non-nearest rounding produces kInvalidGeometry.
GeometryResult SelectRegion(const RasterMapping& mapping,
                            DipPoint first,
                            DipPoint second);

struct RegionSelection {
  Target target;
  RasterMapping mapping;
  RegionGeometry geometry;
};
enum class Phase { kReady, kDragging, kSelected, kCancelled, kSubmitted };
enum class CancelReason {
  kEscape,
  kPointerCancel,
  kHostDismissed,
  kTargetChanged,
  kPolicyChanged,
};

// Single-sequence, local picker only. Owns no pixels, pages or native handles.
// Host routes Escape, pointer cancellation/lost capture, native dismissal and
// target/policy notifications here. Pointer release selects; Confirm is an
// explicit separate action. Confirm emits at most once, never saves an image.
class Picker {
 public:
  static std::variant<Status, Picker> Start(PickerRequest request);
  Picker(const Picker&) = delete;
  Picker& operator=(const Picker&) = delete;
  Picker(Picker&& other) noexcept;
  Picker& operator=(Picker&& other) noexcept;

  Phase phase() const;
  std::optional<CancelReason> cancel_reason() const;
  std::optional<RegionGeometry> geometry() const;

  Status BeginDrag(DipPoint point);
  Status UpdateDrag(DipPoint point);
  Status EndDrag(DipPoint point);
  // Accessible keyboard UI supplies its two corners through the same geometry
  // path. No invented keyboard increment, focus or native key binding policy.
  Status SelectCorners(DipPoint first, DipPoint second);
  Status Cancel(CancelReason reason);

  // Host must supply a fresh authoritative snapshot, not echo request_.
  // Call on native target/policy changes and again before submission.
  Status Revalidate(const PickerRequest& current);
  std::variant<Status, RegionSelection> Confirm(const PickerRequest& current);

 private:
  explicit Picker(PickerRequest request);
  struct Ready {};
  struct Dragging {
    DipPoint anchor;
    DipPoint current;
  };
  struct Selected {
    RegionGeometry geometry;
  };
  struct Cancelled {
    CancelReason reason;
  };
  struct Submitted {};

  PickerRequest request_;
  std::variant<Ready, Dragging, Selected, Cancelled, Submitted> state_;
};

}  // namespace arcified::region_capture

#endif  // ARCIFIED_NATIVE_FEATURES_REGION_CAPTURE_REGION_CAPTURE_H_
