#include "region_capture.h"

#include <algorithm>
#include <cfenv>
#include <cfloat>
#include <cmath>
#include <limits>
#include <utility>

namespace arcified::region_capture {
namespace {

bool Valid(DipPoint point) {
  return std::isfinite(point.x) && std::isfinite(point.y);
}

bool Valid(DipRect rect) {
  return Valid(DipPoint{rect.left, rect.top}) &&
         Valid(DipPoint{rect.right, rect.bottom}) && rect.left < rect.right &&
         rect.top < rect.bottom && std::isfinite(rect.right - rect.left) &&
         std::isfinite(rect.bottom - rect.top);
}

bool Valid(const RasterMapping& mapping) {
  const auto& raster = mapping.raster_bounds;
  const auto& visible = mapping.visible_bounds;
  return Valid(raster) && Valid(visible) && mapping.raster_size.width > 0 &&
         mapping.raster_size.height > 0 && visible.left >= raster.left &&
         visible.top >= raster.top && visible.right <= raster.right &&
         visible.bottom <= raster.bottom;
}

bool Same(DipRect a, DipRect b) {
  return a.left == b.left && a.top == b.top && a.right == b.right &&
         a.bottom == b.bottom;
}

bool Same(const RasterMapping& a, const RasterMapping& b) {
  return Same(a.raster_bounds, b.raster_bounds) &&
         Same(a.visible_bounds, b.visible_bounds) &&
         a.raster_size.width == b.raster_size.width &&
         a.raster_size.height == b.raster_size.height;
}

bool Same(const Target& a, const Target& b) {
  return a.page == b.page && a.profile == b.profile &&
         a.attachment == b.attachment && a.split == b.split &&
         a.committed_document == b.committed_document &&
         a.visual_revision == b.visual_revision;
}

bool Valid(const Target& target) {
  return !target.page.value.empty() && !target.profile.value.empty() &&
         !target.attachment.window.value.empty() &&
         !target.attachment.space.value.empty() &&
         (!target.split || !target.split->value.empty()) &&
         !target.committed_document.empty() && !target.visual_revision.empty();
}

Status CheckPermission(Permission permission) {
  switch (permission) {
    case Permission::kUnknown:
      return Status::kPermissionRequired;
    case Permission::kAllowed:
      return Status::kOk;
    case Permission::kDenied:
      return Status::kPermissionDenied;
    case Permission::kSensitiveContent:
      return Status::kSensitiveContent;
  }
  return Status::kPermissionRequired;
}

DipPoint Clamp(DipPoint point, DipRect bounds) {
  return {std::clamp(point.x, bounds.left, bounds.right),
          std::clamp(point.y, bounds.top, bounds.bottom)};
}

// This is only a candidate. Cancellation can erase an offset at a pixel edge;
// containment is proved independently in DIP space below, never by an epsilon.
double PixelEdge(double edge, double start, double end, int pixels) {
  return (edge - start) / (end - start) * pixels;
}

struct Enclosure {
  double lower;
  double upper;
};

Enclosure Rounded(double value, bool exact) {
  if (exact)
    return {value, value};
  return {std::nextafter(value, -std::numeric_limits<double>::infinity()),
          std::nextafter(value, std::numeric_limits<double>::infinity())};
}

Enclosure Sum(double a, double b) {
  const double sum = a + b;
  // TwoSum recovers the exact rounding error under round-to-nearest with
  // gradual underflow. Nonfinite intermediates cannot establish exactness.
  const double virtual_b = sum - a;
  const double error = (a - (sum - virtual_b)) + (b - virtual_b);
  return Rounded(sum, std::isfinite(virtual_b) && error == 0);
}

bool ProductResidualFits(double a, double b) {
  if (a == 0 || b == 0)
    return true;
  int a_exponent;
  int b_exponent;
  std::frexp(a, &a_exponent);
  std::frexp(b, &b_exponent);
  // Each significand has at most digits bits. The product's smallest possible
  // bit must fit in a subnormal before a zero FMA residual can prove exactness.
  return a_exponent + b_exponent >= std::numeric_limits<double>::min_exponent +
                                        std::numeric_limits<double>::digits;
}

Enclosure Product(double a, double b) {
  const double product = a * b;
  return Rounded(product, std::isfinite(product) && ProductResidualFits(a, b) &&
                              std::fma(a, b, -product) == 0);
}

Enclosure Quotient(double numerator, double denominator) {
  const double quotient = numerator / denominator;
  return Rounded(quotient,
                 std::isfinite(quotient) &&
                     ProductResidualFits(quotient, denominator) &&
                     std::fma(quotient, denominator, -numerator) == 0);
}

struct DipBoundary {
  Enclosure extent;
  double displayed;
};

DipBoundary DipEdge(int edge, double start, double end, int pixels) {
  // Preserve the exact host endpoints, including subnormal extents.
  if (edge == 0)
    return {{start, start}, start};
  if (edge == pixels)
    return {{end, end}, end};

  const auto width = Sum(end, -start);
  Enclosure offset;
  double displayed_offset;
  if (std::isfinite(width.upper * edge)) {
    // Multiply first to retain ordinary exact boundaries such as 200*50/500.
    const auto lower = Product(width.lower, edge);
    const auto upper = Product(width.upper, edge);
    offset = {Quotient(lower.lower, pixels).lower,
              Quotient(upper.upper, pixels).upper};
    displayed_offset = ((end - start) * edge) / pixels;
  } else {
    // Avoid overflowing a representable interior boundary. All factors here
    // are nonnegative, so endpoint arithmetic encloses the exact expression.
    const auto fraction = Quotient(edge, pixels);
    offset = {Product(width.lower, fraction.lower).lower,
              Product(width.upper, fraction.upper).upper};
    displayed_offset = (end - start) * (static_cast<double>(edge) / pixels);
  }
  return {{Sum(start, offset.lower).lower, Sum(start, offset.upper).upper},
          start + displayed_offset};
}

bool Contained(DipBoundary edge, double lower, double upper) {
  return edge.extent.lower >= lower && edge.extent.upper <= upper &&
         edge.displayed >= lower && edge.displayed <= upper;
}

}  // namespace

GeometryResult SelectRegion(const RasterMapping& mapping,
                            DipPoint first,
                            DipPoint second) {
  static_assert(std::numeric_limits<double>::is_iec559 &&
                std::numeric_limits<double>::radix == 2 &&
                std::numeric_limits<int>::digits <=
                    std::numeric_limits<double>::digits &&
                FLT_EVAL_METHOD == 0);
  if (std::fegetround() != FE_TONEAREST || !Valid(mapping) || !Valid(first) ||
      !Valid(second))
    return Status::kInvalidGeometry;
  first = Clamp(first, mapping.visible_bounds);
  second = Clamp(second, mapping.visible_bounds);
  const auto& raster = mapping.raster_bounds;
  const auto& size = mapping.raster_size;
  PixelRect crop{
      static_cast<int>(std::ceil(PixelEdge(
          std::min(first.x, second.x), raster.left, raster.right, size.width))),
      static_cast<int>(
          std::ceil(PixelEdge(std::min(first.y, second.y), raster.top,
                              raster.bottom, size.height))),
      static_cast<int>(std::floor(PixelEdge(
          std::max(first.x, second.x), raster.left, raster.right, size.width))),
      static_cast<int>(
          std::floor(PixelEdge(std::max(first.y, second.y), raster.top,
                               raster.bottom, size.height)))};
  if (crop.left >= crop.right || crop.top >= crop.bottom)
    return Status::kEmptySelection;
  const auto left = DipEdge(crop.left, raster.left, raster.right, size.width);
  const auto top = DipEdge(crop.top, raster.top, raster.bottom, size.height);
  const auto right = DipEdge(crop.right, raster.left, raster.right, size.width);
  const auto bottom =
      DipEdge(crop.bottom, raster.top, raster.bottom, size.height);
  // Prove both the exact affine pixel extent and the rounded drawn boundary
  // are inside the clamped selection. An uncertain candidate is refused.
  if (!Contained(left, std::min(first.x, second.x),
                 std::max(first.x, second.x)) ||
      !Contained(right, std::min(first.x, second.x),
                 std::max(first.x, second.x)) ||
      !Contained(top, std::min(first.y, second.y),
                 std::max(first.y, second.y)) ||
      !Contained(bottom, std::min(first.y, second.y),
                 std::max(first.y, second.y)))
    return Status::kEmptySelection;
  DipRect boundary{left.displayed, top.displayed, right.displayed,
                   bottom.displayed};
  if (!Valid(boundary))
    return Status::kInvalidGeometry;
  return RegionGeometry{crop, boundary};
}

Picker::Picker(PickerRequest request) : request_(std::move(request)) {}

Picker::Picker(Picker&& other) noexcept
    : request_(std::move(other.request_)), state_(std::move(other.state_)) {
  other.state_ = Submitted{};
}

Picker& Picker::operator=(Picker&& other) noexcept {
  if (this != &other) {
    request_ = std::move(other.request_);
    state_ = std::move(other.state_);
    other.state_ = Submitted{};
  }
  return *this;
}

std::variant<Status, Picker> Picker::Start(PickerRequest request) {
  if (!Valid(request.target))
    return Status::kInvalidTarget;
  if (!Valid(request.mapping))
    return Status::kInvalidGeometry;
  Status permission = CheckPermission(request.permission);
  if (permission != Status::kOk)
    return permission;
  return Picker(std::move(request));
}

Phase Picker::phase() const {
  if (std::holds_alternative<Ready>(state_))
    return Phase::kReady;
  if (std::holds_alternative<Dragging>(state_))
    return Phase::kDragging;
  if (std::holds_alternative<Selected>(state_))
    return Phase::kSelected;
  if (std::holds_alternative<Cancelled>(state_))
    return Phase::kCancelled;
  return Phase::kSubmitted;
}

std::optional<CancelReason> Picker::cancel_reason() const {
  if (const auto* cancelled = std::get_if<Cancelled>(&state_))
    return cancelled->reason;
  return std::nullopt;
}

std::optional<RegionGeometry> Picker::geometry() const {
  if (const auto* selected = std::get_if<Selected>(&state_))
    return selected->geometry;
  if (const auto* drag = std::get_if<Dragging>(&state_)) {
    const auto result =
        SelectRegion(request_.mapping, drag->anchor, drag->current);
    if (const auto* region = std::get_if<RegionGeometry>(&result))
      return *region;
  }
  return std::nullopt;
}

Status Picker::BeginDrag(DipPoint point) {
  if (phase() != Phase::kReady && phase() != Phase::kSelected)
    return Status::kWrongPhase;
  if (!Valid(point))
    return Status::kInvalidGeometry;
  // A new gesture must originate on the intended visible pane. Motion after
  // pointer capture may leave it and is clamped, never retargeted to a sibling.
  const auto& bounds = request_.mapping.visible_bounds;
  if (point.x < bounds.left || point.x >= bounds.right ||
      point.y < bounds.top || point.y >= bounds.bottom)
    return Status::kInvalidGeometry;
  state_ = Dragging{point, point};
  return Status::kOk;
}

Status Picker::UpdateDrag(DipPoint point) {
  auto* drag = std::get_if<Dragging>(&state_);
  if (!drag)
    return Status::kWrongPhase;
  if (!Valid(point))
    return Status::kInvalidGeometry;
  drag->current = point;
  return Status::kOk;
}

Status Picker::EndDrag(DipPoint point) {
  const Status update = UpdateDrag(point);
  if (update != Status::kOk)
    return update;
  const auto drag = std::get<Dragging>(state_);
  auto region = SelectRegion(request_.mapping, drag.anchor, drag.current);
  if (const auto* error = std::get_if<Status>(&region)) {
    state_ = Ready{};
    return *error;
  }
  state_ = Selected{std::get<RegionGeometry>(region)};
  return Status::kOk;
}

Status Picker::SelectCorners(DipPoint first, DipPoint second) {
  if (phase() != Phase::kReady && phase() != Phase::kSelected)
    return Status::kWrongPhase;
  auto region = SelectRegion(request_.mapping, first, second);
  if (const auto* error = std::get_if<Status>(&region)) {
    // Remove a previous selection on an empty replacement, so Enter cannot
    // accidentally submit the prior visible region after a failed edit.
    if (*error == Status::kEmptySelection)
      state_ = Ready{};
    return *error;
  }
  state_ = Selected{std::get<RegionGeometry>(region)};
  return Status::kOk;
}

Status Picker::Cancel(CancelReason reason) {
  if (phase() == Phase::kSubmitted)
    return Status::kWrongPhase;
  if (phase() == Phase::kCancelled)
    return Status::kCancelled;
  state_ = Cancelled{reason};
  return Status::kOk;
}

Status Picker::Revalidate(const PickerRequest& current) {
  if (phase() == Phase::kCancelled)
    return Status::kCancelled;
  if (phase() == Phase::kSubmitted)
    return Status::kWrongPhase;
  const Status permission = CheckPermission(current.permission);
  if (permission != Status::kOk) {
    Cancel(CancelReason::kPolicyChanged);
    return permission;
  }
  if (!Same(request_.target, current.target) ||
      !Same(request_.mapping, current.mapping)) {
    Cancel(CancelReason::kTargetChanged);
    return Status::kTargetChanged;
  }
  return Status::kOk;
}

std::variant<Status, RegionSelection> Picker::Confirm(
    const PickerRequest& current) {
  const Status status = Revalidate(current);
  if (status != Status::kOk)
    return status;
  const auto* selected = std::get_if<Selected>(&state_);
  if (!selected)
    return Status::kWrongPhase;
  RegionSelection selection{request_.target, request_.mapping,
                            selected->geometry};
  state_ = Submitted{};
  return selection;
}

}  // namespace arcified::region_capture
