#include "region_capture.h"

#include <cfenv>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <type_traits>
#include <utility>

namespace arcified::region_capture {
namespace {

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

static_assert(!std::is_copy_constructible_v<Picker>);
static_assert(!std::is_copy_assignable_v<Picker>);
static_assert(lifecycle::kContractVersion == 3);

PickerRequest Request() {
  return {{lifecycle::PageId{"selected-page"},
           lifecycle::ProfileId{"selected-profile"},
           {lifecycle::WindowId{"window"}, lifecycle::SpaceId{"space"}},
           lifecycle::SplitId{"split"},
           "document-a",
           "visual-a"},
          {{100, 50, 300, 150}, {100, 50, 300, 150}, {300, 200}},
          Permission::kAllowed};
}

Picker Start(const PickerRequest& request) {
  auto result = Picker::Start(request);
  REQUIRE(std::holds_alternative<Picker>(result));
  return std::move(std::get<Picker>(result));
}

RegionGeometry Region(const RasterMapping& mapping,
                      DipPoint first,
                      DipPoint second) {
  const auto result = SelectRegion(mapping, first, second);
  REQUIRE(std::holds_alternative<RegionGeometry>(result));
  return std::get<RegionGeometry>(result);
}

void CropIs(const RegionGeometry& region, PixelRect expected) {
  REQUIRE(region.crop.left == expected.left);
  REQUIRE(region.crop.top == expected.top);
  REQUIRE(region.crop.right == expected.right);
  REQUIRE(region.crop.bottom == expected.bottom);
}

void GeometryAndScaling() {
  const auto mapping = Request().mapping;
  const auto region = Region(mapping, {110.1, 55.1}, {150.9, 75.9});
  CropIs(region, {16, 11, 76, 51});
  REQUIRE(region.displayed_boundary.left == 100 + 16 / 1.5);
  REQUIRE(region.displayed_boundary.top == 55.5);
  REQUIRE(region.displayed_boundary.right == 100 + 76 / 1.5);
  REQUIRE(region.displayed_boundary.bottom == 75.5);
  CropIs(Region(mapping, {150.9, 75.9}, {110.1, 55.1}), {16, 11, 76, 51});
  CropIs(Region(mapping, {-1000, -1000}, {1000, 1000}), {0, 0, 300, 200});

  // Negative native overlay origin, fractional device mapping and a visible
  // clip. Adjacent pane content cannot enter this crop.
  const RasterMapping clipped{
      {-100, -50, 100, 50}, {-80, -40, 80, 40}, {500, 250}};
  const auto clip = Region(clipped, {-200, -100}, {200, 100});
  CropIs(clip, {50, 25, 450, 225});
  REQUIRE(clip.displayed_boundary.left == -80);
  REQUIRE(clip.displayed_boundary.right == 80);
  REQUIRE(clip.displayed_boundary.top == -40);
  REQUIRE(clip.displayed_boundary.bottom == 40);

  const RasterMapping one_pixel{{0, 0, 1, 1}, {0, 0, 1, 1}, {1, 1}};
  CropIs(Region(one_pixel, {0, 0}, {1, 1}), {0, 0, 1, 1});
  REQUIRE(std::get<Status>(SelectRegion(one_pixel, {0.1, 0}, {1, 1})) ==
          Status::kEmptySelection);
  REQUIRE(std::get<Status>(SelectRegion(mapping, {110, 60}, {110, 80})) ==
          Status::kEmptySelection);
  REQUIRE(std::get<Status>(SelectRegion(mapping, {-20, -20}, {20, 20})) ==
          Status::kEmptySelection);
  REQUIRE(std::get<Status>(SelectRegion(mapping, {110, 60}, {110.01, 60.01})) ==
          Status::kEmptySelection);
}

void InvalidInputsAndNumericBoundaries() {
  const auto request = Request();
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double infinity = std::numeric_limits<double>::infinity();
  const double largest = std::numeric_limits<double>::max();
  for (DipPoint point : {DipPoint{nan, 50}, DipPoint{100, infinity}}) {
    REQUIRE(
        std::get<Status>(SelectRegion(request.mapping, point, {200, 100})) ==
        Status::kInvalidGeometry);
  }
  for (auto mapping : {
           RasterMapping{{0, 0, 0, 10}, {0, 0, 1, 1}, {10, 10}},
           RasterMapping{{0, 0, 10, 10}, {-1, 0, 10, 10}, {10, 10}},
           RasterMapping{{0, 0, 10, 10}, {0, 0, 10, 10}, {0, 10}},
           RasterMapping{{0, 0, 10, 10}, {0, 0, 10, 10}, {10, -1}},
           RasterMapping{{0, 0, nan, 10}, {0, 0, 10, 10}, {10, 10}},
           RasterMapping{{0, 0, 10, 10}, {0, 0, infinity, 10}, {10, 10}},
           RasterMapping{{-largest, 0, largest, 10}, {0, 0, 1, 1}, {10, 10}},
       }) {
    REQUIRE(std::get<Status>(SelectRegion(mapping, {0, 0}, {1, 1})) ==
            Status::kInvalidGeometry);
    auto invalid = request;
    invalid.mapping = mapping;
    REQUIRE(std::get<Status>(Picker::Start(invalid)) ==
            Status::kInvalidGeometry);
  }

  // The integer bound is the PixelSize representation, not a product quota.
  const int largest_size = std::numeric_limits<int>::max();
  const RasterMapping large{{0, 0, largest, largest},
                            {0, 0, largest, largest},
                            {largest_size, largest_size}};
  CropIs(Region(large, {0, 0}, {largest, largest}),
         {0, 0, largest_size, largest_size});
  const double smallest = std::numeric_limits<double>::denorm_min();
  const RasterMapping tiny{
      {0, 0, smallest, smallest}, {0, 0, smallest, smallest}, {1, 1}};
  CropIs(Region(tiny, {0, 0}, {smallest, smallest}), {0, 0, 1, 1});
}

void PixelBoundaryContainment() {
  // Even extended precision loses this positive offset when subtracting -1.
  // The second pixel starts at exactly zero, outside both the visible clip and
  // the selected rectangle. No complete pixel fits on either axis.
  for (double offset : {1e-20, std::numeric_limits<double>::denorm_min(),
                        std::numeric_limits<double>::epsilon() / 2}) {
    const RasterMapping clipped{{-1, 0, 1, 1}, {offset, 0, 1, 1}, {2, 1}};
    REQUIRE(std::get<Status>(SelectRegion(clipped, {offset, 0}, {1, 1})) ==
            Status::kEmptySelection);
    const RasterMapping selected{{-1, 0, 1, 1}, {-1, 0, 1, 1}, {2, 1}};
    REQUIRE(std::get<Status>(SelectRegion(selected, {offset, 0}, {1, 1})) ==
            Status::kEmptySelection);
    const RasterMapping vertical{{0, -1, 1, 1}, {0, offset, 1, 1}, {1, 2}};
    REQUIRE(std::get<Status>(SelectRegion(vertical, {0, offset}, {1, 1})) ==
            Status::kEmptySelection);
  }

  const RasterMapping mapping{{-1, -1, 1, 1}, {-1, -1, 1, 1}, {2, 2}};
  // Preserve exact ordinary boundaries, while refusing just-inside edges on
  // all four sides. These checks run through double, even on wider-long-double
  // platforms, and would catch a fix that checks only the displayed boundary.
  CropIs(Region(mapping, {0, 0}, {1, 1}), {1, 1, 2, 2});
  CropIs(Region(mapping, {-1, -1}, {0, 0}), {0, 0, 1, 1});
  const double inside = std::nextafter(0.0, 1.0);
  for (auto corners : {std::pair{DipPoint{inside, 0}, DipPoint{1, 1}},
                       std::pair{DipPoint{0, inside}, DipPoint{1, 1}},
                       std::pair{DipPoint{-1, -1}, DipPoint{-inside, 0}},
                       std::pair{DipPoint{-1, -1}, DipPoint{0, -inside}}}) {
    REQUIRE(std::get<Status>(
                SelectRegion(mapping, corners.first, corners.second)) ==
            Status::kEmptySelection);
  }

  // Here the exact raster edge is 1/3, strictly beyond its rounded-down DIP
  // representation. A display-only check would incorrectly accept pixel 0.
  const double third_below = 1.0 / 3.0;
  const RasterMapping thirds{{0, 0, 1, 1}, {0, 0, 1, 1}, {3, 1}};
  REQUIRE(std::get<Status>(SelectRegion(thirds, {0, 0}, {third_below, 1})) ==
          Status::kEmptySelection);
  const double third_above = std::nextafter(third_below, 1.0);
  const auto safe = Region(thirds, {0, 0}, {third_above, 1});
  CropIs(safe, {0, 0, 1, 1});
  REQUIRE(safe.displayed_boundary.right <= third_above);

  // Subnormal calculations also need a proven enclosure, including when the
  // generic FMA exactness test cannot establish a representable residual.
  const double normal_min = std::numeric_limits<double>::min();
  const RasterMapping underflow{
      {0, 0, normal_min, 1}, {0, 0, normal_min, 1}, {3, 1}};
  REQUIRE(
      std::get<Status>(SelectRegion(underflow, {0, 0}, {normal_min / 3, 1})) ==
      Status::kEmptySelection);

  // An interior boundary remains usable when width*edge would overflow.
  const double largest = std::numeric_limits<double>::max();
  const RasterMapping overflow{{0, 0, largest, 1}, {0, 0, largest, 1}, {4, 1}};
  const auto half = Region(overflow, {0, 0}, {largest / 2, 1});
  CropIs(half, {0, 0, 2, 1});
  REQUIRE(half.displayed_boundary.right == largest / 2);

  const int rounding = std::fegetround();
  REQUIRE(std::fesetround(FE_UPWARD) == 0);
  const auto unsupported_rounding = SelectRegion(mapping, {0, 0}, {1, 1});
  REQUIRE(std::fesetround(rounding) == 0);
  REQUIRE(std::get<Status>(unsupported_rounding) == Status::kInvalidGeometry);
}

void PermissionAndTargetBoundaries() {
  auto request = Request();
  for (const auto& [permission, expected] : {
           std::pair{Permission::kUnknown, Status::kPermissionRequired},
           std::pair{Permission::kDenied, Status::kPermissionDenied},
           std::pair{Permission::kSensitiveContent, Status::kSensitiveContent},
       }) {
    request.permission = permission;
    REQUIRE(std::get<Status>(Picker::Start(request)) == expected);
  }
  request = Request();
  request.target.page.value.clear();
  REQUIRE(std::get<Status>(Picker::Start(request)) == Status::kInvalidTarget);
  request = Request();
  request.target.profile.value.clear();
  REQUIRE(std::get<Status>(Picker::Start(request)) == Status::kInvalidTarget);
  request = Request();
  request.target.attachment.window.value.clear();
  REQUIRE(std::get<Status>(Picker::Start(request)) == Status::kInvalidTarget);
  request = Request();
  request.target.attachment.space.value.clear();
  REQUIRE(std::get<Status>(Picker::Start(request)) == Status::kInvalidTarget);
  request = Request();
  request.target.split->value.clear();
  REQUIRE(std::get<Status>(Picker::Start(request)) == Status::kInvalidTarget);
  request = Request();
  request.target.committed_document.clear();
  REQUIRE(std::get<Status>(Picker::Start(request)) == Status::kInvalidTarget);
  request = Request();
  request.target.visual_revision.clear();
  REQUIRE(std::get<Status>(Picker::Start(request)) == Status::kInvalidTarget);
  request = Request();
  request.target.split.reset();
  REQUIRE(Start(request).phase() == Phase::kReady);
}

void PointerSelectionAndExplicitConfirm() {
  auto request = Request();
  auto picker = Start(request);
  REQUIRE(picker.phase() == Phase::kReady);
  REQUIRE(!picker.geometry());
  REQUIRE(std::get<Status>(picker.Confirm(request)) == Status::kWrongPhase);
  REQUIRE(picker.UpdateDrag({150, 75}) == Status::kWrongPhase);
  REQUIRE(picker.BeginDrag({300, 60}) == Status::kInvalidGeometry);
  REQUIRE(picker.BeginDrag({99, 60}) == Status::kInvalidGeometry);
  REQUIRE(picker.BeginDrag({110.1, 55.1}) == Status::kOk);
  REQUIRE(picker.phase() == Phase::kDragging);
  REQUIRE(!picker.geometry());
  REQUIRE(picker.UpdateDrag({150.9, 75.9}) == Status::kOk);
  CropIs(*picker.geometry(), {16, 11, 76, 51});
  REQUIRE(std::get<Status>(picker.Confirm(request)) == Status::kWrongPhase);
  REQUIRE(picker.EndDrag({150.9, 75.9}) == Status::kOk);
  REQUIRE(picker.phase() == Phase::kSelected);
  const auto output = picker.Confirm(request);
  REQUIRE(std::holds_alternative<RegionSelection>(output));
  const auto& selection = std::get<RegionSelection>(output);
  CropIs(selection.geometry, {16, 11, 76, 51});
  REQUIRE(selection.target.page == request.target.page);
  REQUIRE(selection.target.profile == request.target.profile);
  REQUIRE(selection.target.attachment == request.target.attachment);
  REQUIRE(selection.target.split == request.target.split);
  REQUIRE(selection.target.committed_document == "document-a");
  REQUIRE(selection.target.visual_revision == "visual-a");
  REQUIRE(selection.mapping.raster_size.width == 300);
  REQUIRE(selection.mapping.raster_size.height == 200);
  REQUIRE(picker.phase() == Phase::kSubmitted);
  REQUIRE(!picker.geometry());
  REQUIRE(std::get<Status>(picker.Confirm(request)) == Status::kWrongPhase);
  REQUIRE(picker.Cancel(CancelReason::kEscape) == Status::kWrongPhase);
  REQUIRE(picker.BeginDrag({110, 60}) == Status::kWrongPhase);
  request.target.committed_document = "changed-after-output";
  REQUIRE(selection.target.committed_document == "document-a");
}

void KeyboardAndEmptyReplacement() {
  const auto request = Request();
  auto picker = Start(request);
  REQUIRE(picker.SelectCorners({150.9, 75.9}, {110.1, 55.1}) == Status::kOk);
  CropIs(*picker.geometry(), {16, 11, 76, 51});
  REQUIRE(picker.SelectCorners({110, 60}, {110, 60}) ==
          Status::kEmptySelection);
  REQUIRE(picker.phase() == Phase::kReady);
  REQUIRE(!picker.geometry());
  REQUIRE(std::get<Status>(picker.Confirm(request)) == Status::kWrongPhase);
  REQUIRE(picker.BeginDrag({110, 60}) == Status::kOk);
  REQUIRE(picker.EndDrag({110, 60}) == Status::kEmptySelection);
  REQUIRE(picker.phase() == Phase::kReady);
  REQUIRE(picker.SelectCorners({100, 50}, {300, 150}) == Status::kOk);
  REQUIRE(picker.BeginDrag({200, 100}) == Status::kOk);
  REQUIRE(picker.EndDrag({1000, 1000}) == Status::kOk);
  CropIs(*picker.geometry(), {150, 100, 300, 200});
}

void CancellationAndMoves() {
  const auto request = Request();
  for (const auto reason : {CancelReason::kEscape, CancelReason::kPointerCancel,
                            CancelReason::kHostDismissed}) {
    for (const auto phase :
         {Phase::kReady, Phase::kDragging, Phase::kSelected}) {
      auto picker = Start(request);
      if (phase == Phase::kDragging)
        REQUIRE(picker.BeginDrag({110, 60}) == Status::kOk);
      if (phase == Phase::kSelected)
        REQUIRE(picker.SelectCorners({110, 60}, {150, 80}) == Status::kOk);
      REQUIRE(picker.Cancel(reason) == Status::kOk);
      REQUIRE(picker.phase() == Phase::kCancelled);
      REQUIRE(picker.cancel_reason() == reason);
      REQUIRE(!picker.geometry());
      REQUIRE(picker.Cancel(CancelReason::kHostDismissed) ==
              Status::kCancelled);
      REQUIRE(picker.cancel_reason() == reason);
      REQUIRE(std::get<Status>(picker.Confirm(request)) == Status::kCancelled);
      REQUIRE(picker.BeginDrag({110, 60}) == Status::kWrongPhase);
    }
  }
  auto source = Start(request);
  REQUIRE(source.SelectCorners({110, 60}, {150, 80}) == Status::kOk);
  auto moved = std::move(source);
  REQUIRE(std::get<Status>(source.Confirm(request)) == Status::kWrongPhase);
  auto assigned = Start(request);
  assigned = std::move(moved);
  REQUIRE(std::get<Status>(moved.Confirm(request)) == Status::kWrongPhase);
  REQUIRE(std::holds_alternative<RegionSelection>(assigned.Confirm(request)));
}

void TargetAndPolicyChanges() {
  const auto request = Request();
  // Mutate each independent host input. Revalidation is exact, with no
  // tolerance that could retain a stale surface or select a sibling pane.
  using Mutate = void (*)(PickerRequest&);
  const Mutate changes[] = {
      [](PickerRequest& r) { r.target.page = lifecycle::PageId{"sibling"}; },
      [](PickerRequest& r) {
        r.target.profile = lifecycle::ProfileId{"other"};
      },
      [](PickerRequest& r) {
        r.target.attachment.window = lifecycle::WindowId{"other"};
      },
      [](PickerRequest& r) {
        r.target.attachment.space = lifecycle::SpaceId{"other"};
      },
      [](PickerRequest& r) { r.target.split = lifecycle::SplitId{"other"}; },
      [](PickerRequest& r) { r.target.split.reset(); },
      [](PickerRequest& r) { r.target.committed_document = "document-b"; },
      [](PickerRequest& r) { r.target.visual_revision = "visual-b"; },
      [](PickerRequest& r) { r.mapping.raster_bounds.left = 99; },
      [](PickerRequest& r) { r.mapping.raster_bounds.top = 49; },
      [](PickerRequest& r) { r.mapping.raster_bounds.right = 301; },
      [](PickerRequest& r) { r.mapping.raster_bounds.bottom = 151; },
      [](PickerRequest& r) { r.mapping.visible_bounds.left = 101; },
      [](PickerRequest& r) { r.mapping.visible_bounds.top = 51; },
      [](PickerRequest& r) { r.mapping.visible_bounds.right = 299; },
      [](PickerRequest& r) { r.mapping.visible_bounds.bottom = 149; },
      [](PickerRequest& r) { r.mapping.raster_size.width = 600; },
      [](PickerRequest& r) { r.mapping.raster_size.height = 400; },
  };
  for (auto change : changes) {
    auto picker = Start(request);
    REQUIRE(picker.SelectCorners({110, 60}, {150, 80}) == Status::kOk);
    REQUIRE(picker.Revalidate(request) == Status::kOk);
    auto current = request;
    change(current);
    REQUIRE(std::get<Status>(picker.Confirm(current)) ==
            Status::kTargetChanged);
    REQUIRE(picker.phase() == Phase::kCancelled);
    REQUIRE(picker.cancel_reason() == CancelReason::kTargetChanged);
    REQUIRE(!picker.geometry());
    REQUIRE(std::get<Status>(picker.Confirm(request)) == Status::kCancelled);
  }
  for (const auto& [permission, expected] : {
           std::pair{Permission::kUnknown, Status::kPermissionRequired},
           std::pair{Permission::kDenied, Status::kPermissionDenied},
           std::pair{Permission::kSensitiveContent, Status::kSensitiveContent},
       }) {
    auto picker = Start(request);
    REQUIRE(picker.SelectCorners({110, 60}, {150, 80}) == Status::kOk);
    auto current = request;
    current.permission = permission;
    REQUIRE(std::get<Status>(picker.Confirm(current)) == expected);
    REQUIRE(picker.cancel_reason() == CancelReason::kPolicyChanged);
    REQUIRE(std::get<Status>(picker.Confirm(request)) == Status::kCancelled);
  }
  auto dragging = Start(request);
  REQUIRE(dragging.BeginDrag({110, 60}) == Status::kOk);
  auto changed = request;
  changed.target.visual_revision = "scrolled-during-drag";
  REQUIRE(dragging.Revalidate(changed) == Status::kTargetChanged);
  REQUIRE(dragging.EndDrag({150, 80}) == Status::kWrongPhase);
}

}  // namespace
}  // namespace arcified::region_capture

int main() {
  using namespace arcified::region_capture;
  GeometryAndScaling();
  InvalidInputsAndNumericBoundaries();
  PixelBoundaryContainment();
  PermissionAndTargetBoundaries();
  PointerSelectionAndExplicitConfirm();
  KeyboardAndEmptyReplacement();
  CancellationAndMoves();
  TargetAndPolicyChanges();
  std::cout << "region-capture geometry and picker fixtures passed\n";
}
