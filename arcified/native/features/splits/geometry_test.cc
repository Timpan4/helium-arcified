#include "geometry.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace arcified::splits {
namespace {

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

using lifecycle::PageId;
using lifecycle::Status;
const PageId kA{"a"};
const PageId kB{"b"};
const PageId kC{"c"};
const lifecycle::SplitId kSplit{"split"};
const lifecycle::Attachment kAttachment{lifecycle::WindowId{"window"},
                                        lifecycle::SpaceId{"space"}};

lifecycle::Model Workspace() {
  lifecycle::Model model;
  REQUIRE(model.AddProfile(lifecycle::ProfileId{"profile"}) == Status::kOk);
  REQUIRE(model.AddSpace(kAttachment.space, lifecycle::ProfileId{"profile"}) ==
          Status::kOk);
  REQUIRE(model.AddWindow(kAttachment.window) == Status::kOk);
  REQUIRE(model.Attach(kAttachment) == Status::kOk);
  for (const auto& page : {kA, kB, kC}) {
    REQUIRE(model.OpenTemporary(
                page, lifecycle::TemporaryTabId{page.value}, kAttachment,
                "https://example.test/" + page.value) == Status::kOk);
  }
  REQUIRE(model.SetSplit(kSplit, kAttachment, {kA, kB, kC}) == Status::kOk);
  return model;
}

Geometry Project(const lifecycle::Model& model,
                 const LayoutMetadata& metadata,
                 const GeometryInput& input) {
  const auto result = ProjectGeometry(model, kSplit, metadata, input);
  REQUIRE(std::holds_alternative<Geometry>(result));
  return std::get<Geometry>(result);
}

void AxesAndTargets() {
  const auto model = Workspace();
  LayoutMetadata metadata{Orientation::kHorizontal,
                          {{kA, 1}, {kB, 2}, {kC, 1}}};
  const GeometryInput input{{810, 500}, {100, 80}, 5, kB};
  const auto horizontal = Project(model, metadata, input);
  REQUIRE(horizontal.mode == GeometryMode::kAllPanes);
  REQUIRE(horizontal.panes.size() == 3);
  REQUIRE(horizontal.panes[0].page == kA);
  REQUIRE(horizontal.panes[0].bounds.width == 200);
  REQUIRE(horizontal.panes[1].bounds.x == 205);
  REQUIRE(horizontal.panes[1].bounds.width == 400);
  REQUIRE(horizontal.panes[2].bounds.x == 610);
  REQUIRE(horizontal.panes[2].bounds.width == 200);
  REQUIRE(horizontal.dividers[0].before == kA);
  REQUIRE(horizontal.dividers[0].after == kB);
  REQUIRE(horizontal.dividers[0].bounds.width == 5);
  REQUIRE(PaneTargetAt(horizontal, {205, 0})->page == kB);
  REQUIRE(PaneTargetAt(horizontal, {809, 499})->page == kC);
  REQUIRE(!PaneTargetAt(horizontal, {200, 10}));
  REQUIRE(!PaneTargetAt(horizontal, {810, 10}));
  REQUIRE(!PaneTargetAt(horizontal, {-1, 0}));
  REQUIRE(!PaneTargetAt(horizontal, {0, 500}));
  REQUIRE(
      !PaneTargetAt(horizontal, {std::numeric_limits<double>::infinity(), 0}));

  metadata.orientation = Orientation::kVertical;
  const auto vertical =
      Project(model, metadata, {{500, 810}, {80, 100}, 5, kB});
  REQUIRE(vertical.panes[1].page == kB);
  REQUIRE(vertical.panes[1].bounds.y == 205);
  REQUIRE(vertical.panes[1].bounds.height == 400);
  REQUIRE(vertical.panes[1].bounds.width == 500);
  REQUIRE(vertical.dividers[1].bounds.y == 605);
  REQUIRE(vertical.dividers[1].bounds.height == 5);
  REQUIRE(PaneTargetAt(vertical, {10, 205})->page == kB);
  REQUIRE(!PaneTargetAt(vertical, {10, 605}));
  REQUIRE(model.pages().at(kB).current_navigation == "https://example.test/b");
  REQUIRE(model.pages().at(kB).activity == lifecycle::Activity::kUnknown);
  REQUIRE(model.pages().at(kB).attachment() == kAttachment);
  REQUIRE(model.splits().at(kSplit).pages == std::vector<PageId>({kA, kB, kC}));
}

void MinimaAndExtremeWeights() {
  const auto model = Workspace();
  const double maximum = std::numeric_limits<double>::max();
  const double tiny = std::numeric_limits<double>::denorm_min();
  LayoutMetadata metadata{Orientation::kHorizontal,
                          {{kA, tiny}, {kB, maximum}, {kC, tiny}}};
  const GeometryInput input{{810, 500}, {100, 80}, 5, kB};
  const auto extreme = Project(model, metadata, input);
  REQUIRE(extreme.mode == GeometryMode::kAllPanes);
  REQUIRE(extreme.panes[0].bounds.width == 100);
  REQUIRE(extreme.panes[1].bounds.width == 600);
  REQUIRE(extreme.panes[2].bounds.width == 100);
  metadata.weights = {{kA, maximum}, {kB, maximum}, {kC, maximum}};
  const auto equal = Project(model, metadata, {{610, 500}, {100, 80}, 5, kB});
  REQUIRE(equal.mode == GeometryMode::kAllPanes);
  for (const auto& pane : equal.panes)
    REQUIRE(pane.bounds.width == 200);
  const auto exact = Project(model, metadata, {{310, 80}, {100, 80}, 5, kB});
  REQUIRE(exact.mode == GeometryMode::kAllPanes);
  for (const auto& pane : exact.panes)
    REQUIRE(pane.bounds.width == 100);
  const auto cross_small =
      Project(model, metadata, {{810, 79}, {100, 80}, 5, kB});
  REQUIRE(cross_small.mode == GeometryMode::kActiveOnly);
  const auto overflow_dividers =
      Project(model, metadata, {{810, 500}, {100, 80}, maximum, kB});
  REQUIRE(overflow_dividers.mode == GeometryMode::kActiveOnly);
  const auto unrepresentable = Project(
      model,
      {Orientation::kHorizontal, {{kA, maximum}, {kB, tiny}, {kC, tiny}}},
      {{maximum, 500}, {100, 80}, 0, kB});
  REQUIRE(unrepresentable.mode == GeometryMode::kActiveOnly);
}

void FallbackAndKeyboard() {
  const auto model = Workspace();
  const LayoutMetadata metadata{Orientation::kVertical, {}};
  const GeometryInput input{{500, 249}, {80, 80}, 5, kB};
  const auto narrow = Project(model, metadata, input);
  REQUIRE(narrow.mode == GeometryMode::kActiveOnly);
  REQUIRE(narrow.panes.size() == 1);
  REQUIRE(narrow.panes[0].page == kB);
  REQUIRE(narrow.panes[0].bounds.height == 249);
  REQUIRE(narrow.dividers.empty());
  REQUIRE(std::get<GeometryError>(
              ProjectResize(model, kSplit, metadata, input, kA, 1)) ==
          GeometryError::kResizeUnavailable);
  const auto key =
      KeyboardTarget(model, kSplit, metadata, kB, Direction::kDown);
  const auto target = std::get<std::optional<PageId>>(key);
  REQUIRE(target == kC);
  auto selected = input;
  selected.active_page = *target;
  const auto next = Project(model, metadata, selected);
  REQUIRE(next.mode == GeometryMode::kActiveOnly);
  REQUIRE(next.panes[0].page == kC);
  REQUIRE(!std::get<std::optional<PageId>>(
      KeyboardTarget(model, kSplit, metadata, kC, Direction::kDown)));
  REQUIRE(!std::get<std::optional<PageId>>(
      KeyboardTarget(model, kSplit, metadata, kB, Direction::kLeft)));
  REQUIRE(std::get<std::optional<PageId>>(KeyboardTarget(
              model, kSplit, metadata, kB, Direction::kUp)) == kA);
  const auto zero = Project(model, metadata, {{0, 0}, {80, 80}, 5, kB});
  REQUIRE(zero.mode == GeometryMode::kActiveOnly);
  REQUIRE(zero.panes[0].bounds.width == 0);
  REQUIRE(zero.panes[0].bounds.height == 0);
  REQUIRE(!PaneTargetAt(zero, {0, 0}));
  const auto expanded = Project(model, metadata, {{500, 250}, {80, 80}, 5, kB});
  REQUIRE(expanded.mode == GeometryMode::kAllPanes);
  REQUIRE(expanded.panes.size() == 3);
  REQUIRE(metadata.weights.empty());
  REQUIRE(model.splits().at(kSplit).pages.size() == 3);
  REQUIRE(!model.windows().at(kAttachment.window).selected_space);
}

void ResizeAndReorder() {
  auto model = Workspace();
  const LayoutMetadata metadata{Orientation::kVertical,
                                {{kA, 1}, {kB, 2}, {kC, 1}}};
  const GeometryInput input{{500, 810}, {80, 100}, 5, kB};
  auto result = ProjectResize(model, kSplit, metadata, input, kA, 100);
  REQUIRE((std::holds_alternative<std::map<PageId, double>>(result)));
  LayoutMetadata resized{metadata.orientation,
                         std::get<std::map<PageId, double>>(result)};
  const auto changed = Project(model, resized, input);
  REQUIRE(changed.panes[0].bounds.height == 300);
  REQUIRE(changed.panes[1].bounds.height == 300);
  REQUIRE(changed.panes[2].bounds.height == 200);
  result = ProjectResize(model, kSplit, metadata, input, kA,
                         std::numeric_limits<double>::max());
  resized.weights = std::get<std::map<PageId, double>>(result);
  const auto clamped = Project(model, resized, input);
  REQUIRE(clamped.panes[0].bounds.height == 500);
  REQUIRE(clamped.panes[1].bounds.height == 100);
  REQUIRE(clamped.panes[2].bounds.height == 200);
  result = ProjectResize(model, kSplit, metadata, input, kA,
                         -std::numeric_limits<double>::max());
  resized.weights = std::get<std::map<PageId, double>>(result);
  REQUIRE(Project(model, resized, input).panes[0].bounds.height == 100);
  REQUIRE(metadata.weights.at(kA) == 1);
  REQUIRE(model.SetSplit(kSplit, kAttachment, {kC, kA, kB}) == Status::kOk);
  const auto reordered = Project(model, metadata, input);
  REQUIRE(reordered.panes[0].page == kC);
  REQUIRE(reordered.panes[2].page == kB);
  REQUIRE(reordered.panes[2].bounds.height == 400);
  REQUIRE(std::get<std::optional<PageId>>(KeyboardTarget(
              model, kSplit, metadata, kA, Direction::kUp)) == kC);
  REQUIRE(std::get<GeometryError>(
              ProjectResize(model, kSplit, metadata, input, kB, 10)) ==
          GeometryError::kResizeUnavailable);
}

void ResizeBoundaryRounding() {
  const auto model = Workspace();
  for (auto orientation : {Orientation::kHorizontal, Orientation::kVertical}) {
    const bool horizontal = orientation == Orientation::kHorizontal;
    const LayoutMetadata metadata{orientation, {{kA, 1}, {kB, 2}, {kC, 1}}};
    const GeometryInput input{horizontal ? Size{400, 500} : Size{500, 400},
                              horizontal ? Size{100, 80} : Size{80, 100}, 5,
                              kB};
    for (double delta : {0.0, std::numeric_limits<double>::max(),
                         -std::numeric_limits<double>::max()}) {
      const auto result =
          ProjectResize(model, kSplit, metadata, input, kA, delta);
      REQUIRE((std::holds_alternative<std::map<PageId, double>>(result)));
      const auto geometry = Project(
          model, {orientation, std::get<std::map<PageId, double>>(result)},
          input);
      REQUIRE(geometry.mode == GeometryMode::kAllPanes);
      REQUIRE(geometry.panes.size() == 3);
      double edge = 0;
      for (std::size_t i = 0; i < geometry.panes.size(); ++i) {
        const auto& bounds = geometry.panes[i].bounds;
        const double origin = horizontal ? bounds.x : bounds.y;
        const double length = horizontal ? bounds.width : bounds.height;
        REQUIRE(origin == edge);
        REQUIRE(length == (i == (delta > 0 ? 0u : 1u) ? 190 : 100));
        edge = origin + length;
        if (i < geometry.dividers.size()) {
          const auto& divider = geometry.dividers[i].bounds;
          REQUIRE((horizontal ? divider.x : divider.y) == edge);
          REQUIRE((horizontal ? divider.width : divider.height) == 5);
          edge += 5;
        }
      }
      REQUIRE(edge == 400);
    }
    // Equivalent relative weights must also leave room for the last minimum,
    // even when proportional boundaries round toward that pane.
    const auto normalized = Project(
        model, {orientation, {{kA, 100.0 / 190}, {kB, 1}, {kC, 100.0 / 190}}},
        input);
    REQUIRE(normalized.mode == GeometryMode::kAllPanes);
    REQUIRE((horizontal ? normalized.panes[0].bounds.width
                        : normalized.panes[0].bounds.height) >= 100);
    REQUIRE((horizontal ? normalized.panes[1].bounds.width
                        : normalized.panes[1].bounds.height) >= 100);
    REQUIRE((horizontal ? normalized.panes[2].bounds.width
                        : normalized.panes[2].bounds.height) == 100);
    const auto& last = normalized.panes[2].bounds;
    REQUIRE((horizontal ? last.x + last.width : last.y + last.height) == 400);
  }
}

void SignificantProductsAndOrigins() {
  auto model = Workspace();
  REQUIRE(model.SetSplit(kSplit, kAttachment, {kA, kB}) == Status::kOk);
  for (auto orientation : {Orientation::kHorizontal, Orientation::kVertical}) {
    const bool horizontal = orientation == Orientation::kHorizontal;
    const GeometryInput input{horizontal ? Size{1e300, 500} : Size{500, 1e300},
                              horizontal ? Size{1e-200, 80} : Size{80, 1e-200},
                              0, kA};
    // The first ratio underflows to zero; the second loses precision as a
    // subnormal. Both products are representable at the first pane's origin.
    for (double weight : {1e-100, 1e-20}) {
      const LayoutMetadata metadata{orientation, {{kA, weight}, {kB, 1e300}}};
      const auto geometry = Project(model, metadata, input);
      REQUIRE(geometry.mode == GeometryMode::kAllPanes);
      const auto& first = geometry.panes[0].bounds;
      const auto& second = geometry.panes[1].bounds;
      REQUIRE((horizontal ? first.x : first.y) == 0);
      REQUIRE((horizontal ? first.width : first.height) == weight);
      REQUIRE((horizontal ? second.x : second.y) == weight);
      REQUIRE((horizontal ? second.width : second.height) == 1e300);
    }

    // Small extents also survive at origin zero in an ordinary viewport.
    const LayoutMetadata metadata{orientation, {{kA, 1e-20}, {kB, 1}}};
    auto ordinary = input;
    ordinary.viewport = horizontal ? Size{1, 500} : Size{500, 1};
    const auto leading = Project(model, metadata, ordinary);
    REQUIRE(leading.mode == GeometryMode::kAllPanes);
    REQUIRE((horizontal ? leading.panes[0].bounds.width
                        : leading.panes[0].bounds.height) == 1e-20);
    // At the far edge, that extent cannot advance a double coordinate. Do
    // not turn it into a zero-width pane or an unrelated, much larger share.
    REQUIRE(model.SetSplit(kSplit, kAttachment, {kB, kA}) == Status::kOk);
    REQUIRE(Project(model, metadata, ordinary).mode ==
            GeometryMode::kActiveOnly);
    REQUIRE(model.SetSplit(kSplit, kAttachment, {kA, kB, kC}) == Status::kOk);
    // An overflowing weight sum still requires normalization. Its tiny term
    // must survive in the product even though it cannot affect that sum.
    const double maximum = std::numeric_limits<double>::max();
    auto overflow = input;
    overflow.viewport = horizontal ? Size{maximum, 500} : Size{500, maximum};
    const auto scaled = Project(
        model, {orientation, {{kA, 1e-100}, {kB, maximum}, {kC, maximum}}},
        overflow);
    REQUIRE(scaled.mode == GeometryMode::kAllPanes);
    REQUIRE((horizontal ? scaled.panes[0].bounds.width
                        : scaled.panes[0].bounds.height) == 1e-100 / 2);
    REQUIRE(model.SetSplit(kSplit, kAttachment, {kA, kB}) == Status::kOk);
  }
}

void InvalidInputs() {
  auto model = Workspace();
  const LayoutMetadata metadata;
  const GeometryInput valid{{810, 500}, {100, 80}, 5, kB};
  for (double bad : {-1.0, std::numeric_limits<double>::infinity(),
                     std::numeric_limits<double>::quiet_NaN()}) {
    for (int field = 0; field < 5; ++field) {
      auto input = valid;
      double* dimensions[] = {&input.viewport.width, &input.viewport.height,
                              &input.minimum_pane.width,
                              &input.minimum_pane.height,
                              &input.divider_extent};
      *dimensions[field] = bad;
      REQUIRE(std::get<GeometryError>(
                  ProjectGeometry(model, kSplit, metadata, input)) ==
              GeometryError::kInvalidDimensions);
    }
  }
  auto zero_minimum = valid;
  zero_minimum.minimum_pane.width = 0;
  REQUIRE(std::get<GeometryError>(
              ProjectGeometry(model, kSplit, metadata, zero_minimum)) ==
          GeometryError::kInvalidDimensions);
  REQUIRE(std::get<LayoutError>(ProjectGeometry(
              model, lifecycle::SplitId{"missing"}, metadata, valid)) ==
          LayoutError::kSplitNotFound);
  auto missing_active = valid;
  missing_active.active_page = PageId{"missing"};
  REQUIRE(std::get<LayoutError>(
              ProjectGeometry(model, kSplit, metadata, missing_active)) ==
          LayoutError::kPageNotInSplit);
  REQUIRE(std::get<LayoutError>(ProjectGeometry(
              model, kSplit, {Orientation::kHorizontal, {{kA, 0}}}, valid)) ==
          LayoutError::kInvalidWeight);
  REQUIRE(std::get<GeometryError>(
              ProjectResize(model, kSplit, metadata, valid, kA,
                            std::numeric_limits<double>::infinity())) ==
          GeometryError::kInvalidResize);
  REQUIRE(std::get<GeometryError>(KeyboardTarget(model, kSplit, metadata, kA,
                                                 static_cast<Direction>(-1))) ==
          GeometryError::kInvalidDirection);
  REQUIRE(model.SetSplit(kSplit, kAttachment, {kB}) == Status::kOk);
  const auto single = Project(model, metadata, valid);
  REQUIRE(single.panes.size() == 1);
  REQUIRE(single.panes[0].bounds.width == 810);
  REQUIRE(single.dividers.empty());
  REQUIRE(!std::get<std::optional<PageId>>(
      KeyboardTarget(model, kSplit, metadata, kB, Direction::kRight)));
}

}  // namespace
}  // namespace arcified::splits

int main() {
  using namespace arcified::splits;
  AxesAndTargets();
  MinimaAndExtremeWeights();
  FallbackAndKeyboard();
  ResizeAndReorder();
  ResizeBoundaryRounding();
  SignificantProductsAndOrigins();
  InvalidInputs();
  std::cout << "split geometry fixtures passed\n";
}
