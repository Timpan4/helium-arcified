#include "../../features/layout/focus_mode/focus_mode.h"
#include "../../features/layout/tab_presentation/tab_presentation.h"
#include "../../features/motion/presentation_transition.h"
#include "../../features/splits/geometry.h"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <optional>
#include <variant>
#include <vector>

namespace arcified::layout_conformance {
namespace {

namespace fm = focus_mode;
namespace lc = lifecycle;
namespace sb = sidebar;
namespace sp = splits;
namespace tp = tab_presentation;

[[noreturn]] void Fail(const char* expression, int line) {
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression)        \
  do {                             \
    if (!(expression))             \
      Fail(#expression, __LINE__); \
  } while (false)

template <typename T, typename... Alternatives>
T Value(const std::variant<Alternatives...>& result) {
  REQUIRE(std::holds_alternative<T>(result));
  return std::get<T>(result);
}

struct Fixture {
  const lc::ProfileId kProfile{"profile"};
  const lc::Attachment kAttachment{lc::WindowId{"window"},
                                   lc::SpaceId{"space"}};
  const lc::PageId kA{"a"};
  const lc::PageId kB{"b"};
  const lc::SplitId kSplit{"split"};

  lc::Model model;
  std::vector<tp::TabIdentity> order{kA, kB};
  std::optional<tp::TabIdentity> selected{kB};

  Fixture() {
    REQUIRE(model.AddProfile(kProfile) == lc::Status::kOk);
    REQUIRE(model.AddSpace(kAttachment.space, kProfile) == lc::Status::kOk);
    REQUIRE(model.AddWindow(kAttachment.window) == lc::Status::kOk);
    REQUIRE(model.Attach(kAttachment) == lc::Status::kOk);
    REQUIRE(model.SelectSpace(kAttachment.window, kAttachment.space) ==
            lc::Status::kOk);
    for (const auto& page : {kA, kB}) {
      REQUIRE(model.OpenTemporary(
                  page, lc::TemporaryTabId{page.value}, kAttachment,
                  "https://example.test/" + page.value) == lc::Status::kOk);
    }
    REQUIRE(model.SetSplit(kSplit, kAttachment, {kA, kB}) == lc::Status::kOk);
    REQUIRE(model.ReportActivity(kB, lc::Activity::kProtected) ==
            lc::Status::kOk);
  }

  tp::AuthoritativeTabs tabs() const {
    return {std::cref(kAttachment), std::cref(kProfile), std::cref(order),
            std::cref(selected)};
  }

  void CheckReferences(const tp::Presentation& tabs) const {
    REQUIRE(&tabs.tabs().order.get() == &order);
    REQUIRE(&tabs.tabs().selected.get() == &selected);
    REQUIRE(selected == std::optional<tp::TabIdentity>{kB});
    REQUIRE(order == std::vector<tp::TabIdentity>({kA, kB}));
    for (std::size_t i = 0; i < order.size(); ++i) {
      const auto row = tabs.RowAt(i);
      REQUIRE(row.has_value());
      REQUIRE(&row->activation_target.get() == &order[i]);
      REQUIRE(row->selected == (i == 1));
    }
    REQUIRE(model.windows().at(kAttachment.window).selected_space ==
            kAttachment.space);
    REQUIRE(model.splits().at(kSplit).attachment == kAttachment);
    REQUIRE(model.splits().at(kSplit).pages ==
            std::vector<lc::PageId>({kA, kB}));
    REQUIRE(model.pages().size() == 2);
    for (const auto& page : {kA, kB}) {
      REQUIRE(model.pages().at(page).attachment() == kAttachment);
      REQUIRE(model.pages().at(page).profile == kProfile);
      REQUIRE(model.pages().at(page).current_navigation ==
              "https://example.test/" + page.value);
      REQUIRE(std::get<lc::TemporaryTab>(model.pages().at(page).owner).id ==
              lc::TemporaryTabId{page.value});
    }
    REQUIRE(model.pages().at(kB).activity == lc::Activity::kProtected);
  }
};

// All dimensions below are synthetic fixture measurements, not UI policy.
sb::Context Context(sb::Side side) {
  return {{37, 29, 1008, 608},
          side,
          100,
          300,
          180,
          8,
          8,
          sb::PresentationOverride::kNone};
}

tp::Presentation Tabs(const Fixture& fixture,
                      const sb::Context& context,
                      tp::Orientation requested = tp::Orientation::kVertical,
                      bool force_horizontal_fallback = false) {
  return Value<tp::Presentation>(tp::Project(
      fixture.tabs(), {requested, tp::Density::kComfortable}, context.side,
      {{context.available.width, context.available.height},
       {force_horizontal_fallback ? 1100.0 : 280.0, 80},
       {300, 100}}));
}

// Test wiring only. It holds no state and uses the reviewed public seam.
sb::Layout Configure(const fm::Presentation& focus,
                     const fm::HostFacts& facts,
                     const tp::Presentation& tabs,
                     sb::Context context,
                     sb::Presentation& sidebar) {
  context.presentation_override =
      tabs.sidebar_override(focus.Project(facts).chrome);
  const auto effect =
      sidebar.Configure(context, sb::FocusLocation::kOutsideSidebar);
  REQUIRE(effect.status == sb::Status::kOk);
  REQUIRE(!effect.preferences_changed);
  return sidebar.layout();
}

sp::GeometryInput Input(const sb::Layout& layout,
                        const lc::PageId& active_page) {
  return {{layout.content.width, layout.content.height},
          {180, 120},
          8,
          active_page};
}

sp::Geometry Geometry(const Fixture& fixture,
                      const sp::LayoutMetadata& metadata,
                      const sb::Layout& layout) {
  return Value<sp::Geometry>(sp::ProjectGeometry(
      fixture.model, fixture.kSplit, metadata, Input(layout, fixture.kB)));
}

void RectIs(const sp::Rect& rect, const sp::Rect& expected) {
  REQUIRE(rect.x == expected.x);
  REQUIRE(rect.y == expected.y);
  REQUIRE(rect.width == expected.width);
  REQUIRE(rect.height == expected.height);
}

void Hidden(const sb::Layout& layout, const sb::Rect& available) {
  REQUIRE(layout.mode == sb::Mode::kHidden);
  REQUIRE(layout.content == available);
  REQUIRE(!layout.sidebar);
  REQUIRE(!layout.reveal_target);
  REQUIRE(!layout.resize_target);
}

void HorizontalRevealCannotRestoreVerticalSidebar() {
  for (const auto side : {sb::Side::kLeft, sb::Side::kRight}) {
    for (const bool fallback : {false, true}) {
      const Fixture fixture;
      const auto context = Context(side);
      const auto vertical = Tabs(fixture, context);
      const auto horizontal = Tabs(
          fixture, context,
          fallback ? tp::Orientation::kVertical : tp::Orientation::kHorizontal,
          fallback);
      REQUIRE(horizontal.effective_orientation() ==
              tp::Orientation::kHorizontal);
      REQUIRE(horizontal.used_fallback() == fallback);
      REQUIRE(horizontal.sidebar_side() == side);
      auto sidebar = sb::Presentation::Create({true, 200}, context);
      REQUIRE(sidebar.has_value());
      fm::Presentation focus;
      fm::HostFacts facts;
      REQUIRE(focus.Dispatch(fm::Action::kEnter, facts).projection.active);
      for (const auto action :
           {fm::Action::kRevealPointer, fm::Action::kRevealKeyboard}) {
        const auto result = focus.Dispatch(action, facts);
        REQUIRE(result.projection.chrome ==
                sb::PresentationOverride::kRevealOverlay);
        // Positive control: the same focus reveal does present vertical tabs.
        const auto shown = Configure(focus, facts, vertical, context, *sidebar);
        REQUIRE(shown.mode == sb::Mode::kOverlay);
        REQUIRE(shown.sidebar.has_value());
        REQUIRE(shown.content == context.available);
        Hidden(Configure(focus, facts, horizontal, context, *sidebar),
               context.available);
        REQUIRE(sidebar
                    ->Dispatch(sb::Action::kKeyboardReveal,
                               sb::FocusLocation::kOutsideSidebar)
                    .status == sb::Status::kUnavailable);
        REQUIRE(sidebar
                    ->Dispatch(sb::Action::kPointerEnter,
                               sb::FocusLocation::kOutsideSidebar)
                    .status == sb::Status::kOk);
        Hidden(sidebar->layout(), context.available);
        fixture.CheckReferences(horizontal);
      }
      focus.Dispatch(fm::Action::kExit, facts);
      Hidden(Configure(focus, facts, horizontal, context, *sidebar),
             context.available);
      REQUIRE(Configure(focus, facts, vertical, context, *sidebar).mode ==
              sb::Mode::kDocked);
      REQUIRE(sidebar->preferences().persistent_visible);
      REQUIRE(sidebar->preferences().preferred_width == 200);
    }
  }
}

void DialogAndFullscreenUseCurrentBounds() {
  const Fixture fixture;
  auto context = Context(sb::Side::kLeft);
  auto tabs = Tabs(fixture, context);
  auto sidebar = sb::Presentation::Create({true, 200}, context);
  REQUIRE(sidebar.has_value());
  fm::Presentation focus;
  fm::HostFacts facts;
  focus.Dispatch(fm::Action::kEnter, facts);
  focus.Dispatch(fm::Action::kRevealPointer, facts);
  REQUIRE(Configure(focus, facts, tabs, context, *sidebar).mode ==
          sb::Mode::kOverlay);

  facts.dialog_open = true;
  REQUIRE(focus.Dispatch(fm::Action::kHostChanged, facts).focus ==
          fm::FocusIntent::kPreserve);
  context.available = {83, 47, 808, 408};
  tabs = Tabs(fixture, context);
  Hidden(Configure(focus, facts, tabs, context, *sidebar), context.available);
  REQUIRE(focus.Dispatch(fm::Action::kRevealKeyboard, facts).disposition ==
          fm::Disposition::kDeferToHost);
  REQUIRE(focus.Dispatch(fm::Action::kEscape, facts).disposition ==
          fm::Disposition::kDeferToHost);
  Hidden(Configure(focus, facts, tabs, context, *sidebar), context.available);

  // A dialog/failed return request cannot hide controls retaining actual focus.
  facts.focus = fm::FocusLocation::kChrome;
  auto layout = Configure(focus, facts, tabs, context, *sidebar);
  REQUIRE(layout.mode == sb::Mode::kOverlay);
  REQUIRE(layout.content == context.available);
  facts.focus = fm::FocusLocation::kContent;
  facts.dialog_open = false;
  focus.Dispatch(fm::Action::kHostChanged, facts);
  Hidden(Configure(focus, facts, tabs, context, *sidebar), context.available);

  for (const auto fullscreen :
       {fm::Fullscreen::kOverlayAllowed, fm::Fullscreen::kExclusive}) {
    facts.fullscreen = fullscreen;
    focus.Dispatch(fm::Action::kHostChanged, facts);
    const auto reveal = focus.Dispatch(fm::Action::kRevealPointer, facts);
    layout = Configure(focus, facts, tabs, context, *sidebar);
    if (fullscreen == fm::Fullscreen::kOverlayAllowed) {
      REQUIRE(reveal.disposition == fm::Disposition::kHandled);
      REQUIRE(layout.mode == sb::Mode::kOverlay);
      REQUIRE(layout.content == context.available);
    } else {
      REQUIRE(reveal.disposition == fm::Disposition::kDeferToHost);
      Hidden(layout, context.available);
    }
    REQUIRE(focus.Dispatch(fm::Action::kEscape, facts).disposition ==
            fm::Disposition::kDeferToHost);
    REQUIRE(focus.Project(facts).active);
  }
  facts.fullscreen = fm::Fullscreen::kNone;
  focus.Dispatch(fm::Action::kHostChanged, facts);
  Hidden(Configure(focus, facts, tabs, context, *sidebar), context.available);
  focus.Dispatch(fm::Action::kExit, facts);
  layout = Configure(focus, facts, tabs, context, *sidebar);
  REQUIRE(layout.mode == sb::Mode::kDocked);
  REQUIRE(layout.content == (sb::Rect{283, 47, 608, 408}));
  REQUIRE(sidebar->preferences().preferred_width == 200);
  fixture.CheckReferences(tabs);
}

void SplitBoundsAndReferencesSurviveSuppressionAndResize() {
  for (const auto side : {sb::Side::kLeft, sb::Side::kRight}) {
    for (const auto orientation :
         {sp::Orientation::kHorizontal, sp::Orientation::kVertical}) {
      const Fixture fixture;
      const auto context = Context(side);
      auto tabs = Tabs(fixture, context);
      auto sidebar = sb::Presentation::Create({true, 200}, context);
      REQUIRE(sidebar.has_value());
      fm::Presentation focus;
      fm::HostFacts facts;
      sp::LayoutMetadata metadata{orientation,
                                  {{fixture.kA, 1}, {fixture.kB, 3}}};
      auto layout = Configure(focus, facts, tabs, context, *sidebar);
      REQUIRE(layout.content ==
              (sb::Rect{side == sb::Side::kLeft ? 237.0 : 37.0, 29, 808, 608}));
      auto geometry = Geometry(fixture, metadata, layout);
      REQUIRE(geometry.mode == sp::GeometryMode::kAllPanes);
      REQUIRE(geometry.panes.size() == 2);
      REQUIRE(geometry.dividers.size() == 1);
      REQUIRE(geometry.panes[0].page == fixture.kA);
      REQUIRE(geometry.panes[1].page == fixture.kB);
      REQUIRE(geometry.dividers[0].before == fixture.kA);
      REQUIRE(geometry.dividers[0].after == fixture.kB);
      const bool horizontal = orientation == sp::Orientation::kHorizontal;
      if (horizontal) {
        RectIs(geometry.panes[0].bounds, {0, 0, 200, 608});
        RectIs(geometry.panes[1].bounds, {208, 0, 600, 608});
        RectIs(geometry.dividers[0].bounds, {200, 0, 8, 608});
      } else {
        RectIs(geometry.panes[0].bounds, {0, 0, 808, 150});
        RectIs(geometry.panes[1].bounds, {0, 158, 808, 450});
        RectIs(geometry.dividers[0].bounds, {0, 150, 808, 8});
      }
      // Host coordinates require a separate translation. The hit-test API
      // consumes the translated-back content-local point, never window space.
      const sp::Point host_point{layout.content.x + (horizontal ? 208.0 : 0.0),
                                 layout.content.y + (horizontal ? 0.0 : 158.0)};
      const auto target = sp::PaneTargetAt(
          geometry,
          {host_point.x - layout.content.x, host_point.y - layout.content.y});
      REQUIRE(target.has_value());
      REQUIRE(target->page == fixture.kB);
      REQUIRE(!sp::PaneTargetAt(
          geometry, horizontal ? sp::Point{200, 0} : sp::Point{0, 150}));
      REQUIRE(!sp::PaneTargetAt(geometry, {808, 608}));

      // Resize consumes the current docked content extent, not outer bounds.
      metadata.weights = Value<std::map<lc::PageId, double>>(sp::ProjectResize(
          fixture.model, fixture.kSplit, metadata, Input(layout, fixture.kB),
          fixture.kA, horizontal ? 200.0 : 150.0));
      const auto resized_weights = metadata.weights;
      geometry = Geometry(fixture, metadata, layout);
      RectIs(geometry.panes[0].bounds,
             {0, 0, horizontal ? 400.0 : 808.0, horizontal ? 608.0 : 300.0});

      focus.Dispatch(fm::Action::kEnter, facts);
      layout = Configure(focus, facts, tabs, context, *sidebar);
      Hidden(layout, context.available);
      geometry = Geometry(fixture, metadata, layout);
      RectIs(geometry.panes[0].bounds,
             {0, 0, horizontal ? 500.0 : 1008.0, horizontal ? 608.0 : 300.0});
      focus.Dispatch(fm::Action::kRevealPointer, facts);
      layout = Configure(focus, facts, tabs, context, *sidebar);
      REQUIRE(layout.mode == sb::Mode::kOverlay);
      REQUIRE(layout.content == context.available);
      geometry = Geometry(fixture, metadata, layout);
      RectIs(geometry.panes[1].bounds,
             {horizontal ? 508.0 : 0.0, horizontal ? 0.0 : 308.0,
              horizontal ? 500.0 : 1008.0, horizontal ? 608.0 : 300.0});

      auto small = context;
      small.available = {83, 47, 308, 108};
      tabs = Tabs(fixture, small);
      layout = Configure(focus, facts, tabs, small, *sidebar);
      REQUIRE(layout.content == small.available);
      geometry = Geometry(fixture, metadata, layout);
      REQUIRE(geometry.mode == sp::GeometryMode::kActiveOnly);
      REQUIRE(geometry.panes.size() == 1);
      REQUIRE(geometry.panes[0].page == fixture.kB);
      RectIs(geometry.panes[0].bounds, {0, 0, 308, 108});
      REQUIRE(geometry.dividers.empty());
      REQUIRE(Value<sp::GeometryError>(sp::ProjectResize(
                  fixture.model, fixture.kSplit, metadata,
                  Input(layout, fixture.kB), fixture.kA, 1)) ==
              sp::GeometryError::kResizeUnavailable);
      REQUIRE(Value<std::optional<lc::PageId>>(sp::KeyboardTarget(
                  fixture.model, fixture.kSplit, metadata, fixture.kB,
                  horizontal ? sp::Direction::kLeft : sp::Direction::kUp)) ==
              fixture.kA);
      fixture.CheckReferences(tabs);
      REQUIRE(metadata.weights == resized_weights);

      focus.Dispatch(fm::Action::kExit, facts);
      tabs = Tabs(fixture, context);
      layout = Configure(focus, facts, tabs, context, *sidebar);
      geometry = Geometry(fixture, metadata, layout);
      REQUIRE(layout.mode == sb::Mode::kDocked);
      REQUIRE(geometry.mode == sp::GeometryMode::kAllPanes);
      REQUIRE(geometry.panes[0].page == fixture.kA);
      REQUIRE(geometry.panes[1].page == fixture.kB);
      RectIs(geometry.panes[0].bounds,
             {0, 0, horizontal ? 400.0 : 808.0, horizontal ? 608.0 : 300.0});
      REQUIRE(metadata.orientation == orientation);
      REQUIRE(metadata.weights == resized_weights);
      fixture.CheckReferences(tabs);
    }
  }
}

// This fixture maps sidebar presence to a decorative scalar only. Neither the
// scalar nor a callback supplies semantic rectangles, selection or focus.
motion::Progress Decoration(const sb::Layout& layout) {
  return layout.sidebar ? motion::Progress::End() : motion::Progress::Start();
}

void MotionCannotReplayObsoleteSemanticBounds() {
  const Fixture fixture;
  auto context = Context(sb::Side::kLeft);
  auto tabs = Tabs(fixture, context);
  auto sidebar = sb::Presentation::Create({true, 200}, context);
  REQUIRE(sidebar.has_value());
  fm::Presentation focus;
  fm::HostFacts facts;
  const sp::LayoutMetadata metadata{sp::Orientation::kHorizontal,
                                    {{fixture.kA, 1}, {fixture.kB, 3}}};
  motion::PresentationTransition transition(Decoration(sidebar->layout()));
  focus.Dispatch(fm::Action::kEnter, facts);
  auto layout = Configure(focus, facts, tabs, context, *sidebar);
  const auto hide =
      transition.Retarget(Decoration(layout), motion::MotionMode::kAnimate);
  REQUIRE(hide.has_value());
  const auto midpoint = motion::Progress::Parse(0.5);
  REQUIRE(midpoint.has_value());
  REQUIRE(transition.Sample(*hide, *midpoint));
  REQUIRE(transition.value().value() == 0.5);
  Hidden(sidebar->layout(), context.available);
  RectIs(Geometry(fixture, metadata, sidebar->layout()).panes[0].bounds,
         {0, 0, 250, 608});

  const auto reveal = focus.Dispatch(fm::Action::kRevealKeyboard, facts);
  REQUIRE(reveal.focus == fm::FocusIntent::kControls);
  // An intent is not proof of native focus movement. Keep current facts.
  REQUIRE(facts.focus == fm::FocusLocation::kContent);
  layout = Configure(focus, facts, tabs, context, *sidebar);
  REQUIRE(layout.mode == sb::Mode::kOverlay);
  const auto show =
      transition.Retarget(Decoration(layout), motion::MotionMode::kAnimate);
  REQUIRE(show.has_value());
  REQUIRE(!transition.Sample(*hide, motion::Progress::End()));
  REQUIRE(!transition.Finish(*hide));
  REQUIRE(!transition.Cancel(*hide));
  REQUIRE(transition.Sample(*show, motion::Progress::End()));
  REQUIRE(sidebar->layout().mode == sb::Mode::kOverlay);
  RectIs(Geometry(fixture, metadata, sidebar->layout()).panes[1].bounds,
         {258, 0, 750, 608});

  REQUIRE(transition.Cancel(*show));
  context.available = {83, 47, 808, 408};
  tabs = Tabs(fixture, context);
  focus.Dispatch(fm::Action::kDismissReveal, facts);
  layout = Configure(focus, facts, tabs, context, *sidebar);
  Hidden(layout, context.available);
  // Cancellation requires a fresh semantic query before the decorative snap.
  transition.Reset(Decoration(sidebar->layout()));
  REQUIRE(!transition.Sample(*show, motion::Progress::End()));
  REQUIRE(!transition.Finish(*show));
  REQUIRE(!transition.ReduceMotion());
  REQUIRE(transition.value().value() == 0);
  RectIs(Geometry(fixture, metadata, sidebar->layout()).panes[1].bounds,
         {208, 0, 600, 408});

  focus.Dispatch(fm::Action::kExit, facts);
  layout = Configure(focus, facts, tabs, context, *sidebar);
  REQUIRE(layout.content == (sb::Rect{283, 47, 608, 408}));
  const auto restore =
      transition.Retarget(Decoration(layout), motion::MotionMode::kAnimate);
  REQUIRE(restore.has_value());
  // Semantic docking and split reprojection precede motion completion.
  RectIs(Geometry(fixture, metadata, sidebar->layout()).panes[0].bounds,
         {0, 0, 180, 408});
  REQUIRE(transition.ReduceMotion());
  REQUIRE(transition.value().value() == Decoration(sidebar->layout()).value());
  REQUIRE(!transition.Finish(*restore));
  REQUIRE(transition.value().value() == Decoration(sidebar->layout()).value());
  REQUIRE(!transition.Finish(*show));
  REQUIRE(transition.value().value() == Decoration(sidebar->layout()).value());
  REQUIRE(sidebar->layout().content == (sb::Rect{283, 47, 608, 408}));
  RectIs(Geometry(fixture, metadata, sidebar->layout()).panes[1].bounds,
         {188, 0, 420, 408});
  REQUIRE(facts.focus == fm::FocusLocation::kContent);
  REQUIRE(!focus.Project(facts).active);
  fixture.CheckReferences(tabs);
}

}  // namespace
}  // namespace arcified::layout_conformance

int main() try {
  using namespace arcified::layout_conformance;
  HorizontalRevealCannotRestoreVerticalSidebar();
  DialogAndFullscreenUseCurrentBounds();
  SplitBoundsAndReferencesSurviveSuppressionAndResize();
  MotionCannotReplayObsoleteSemanticBounds();
  std::cout << "layout/split composition: all scenarios passed\n";
} catch (const std::exception& error) {
  std::cerr << "unexpected exception: " << error.what() << '\n';
  return EXIT_FAILURE;
} catch (...) {
  std::cerr << "unexpected exception\n";
  return EXIT_FAILURE;
}
