#include "sidebar.h"

#include <cstdlib>
#include <iostream>
#include <limits>

namespace arcified::sidebar {
namespace {

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

constexpr auto kOutside = FocusLocation::kOutsideSidebar;
constexpr auto kInside = FocusLocation::kInsideSidebar;

// Synthetic caller metrics, not product defaults or platform recommendations.
Context Bounds() {
  return {{10, 20, 800, 600},         Side::kLeft, 120, 360, 200, 16, 24,
          PresentationOverride::kNone};
}

Presentation Sidebar(bool visible = true, Context context = Bounds()) {
  auto result = Presentation::Create({visible, 240}, context);
  REQUIRE(result.has_value());
  return *result;
}

void PersistentAndTemporary() {
  auto sidebar = Sidebar();
  REQUIRE(sidebar.layout().mode == Mode::kDocked);
  REQUIRE((sidebar.layout().content == Rect{250, 20, 560, 600}));
  auto effect = sidebar.Dispatch(Action::kTogglePersistent, kOutside);
  REQUIRE(effect.preferences_changed);
  REQUIRE(effect.focus == FocusRequest::kNone);
  REQUIRE(!sidebar.preferences().persistent_visible);
  REQUIRE(sidebar.layout().mode == Mode::kHidden);
  REQUIRE(sidebar.layout().content == Bounds().available);
  REQUIRE((sidebar.layout().reveal_target == Rect{10, 20, 16, 600}));

  effect = sidebar.Dispatch(Action::kPointerEnter, kOutside);
  REQUIRE(effect.focus == FocusRequest::kNone);
  REQUIRE(!effect.preferences_changed);
  REQUIRE(sidebar.layout().mode == Mode::kOverlay);
  REQUIRE(sidebar.layout().content == Bounds().available);
  REQUIRE(sidebar.layout().sidebar->width == 240);
  sidebar.Dispatch(Action::kPointerLeave, kOutside);
  REQUIRE(sidebar.layout().mode == Mode::kHidden);

  effect = sidebar.Dispatch(Action::kKeyboardReveal, kOutside);
  REQUIRE(effect.focus == FocusRequest::kSidebar);
  sidebar.Dispatch(Action::kPointerLeave, kInside);
  REQUIRE(sidebar.layout().mode == Mode::kOverlay);
  effect = sidebar.Dispatch(Action::kLeave, kInside);
  REQUIRE(effect.focus == FocusRequest::kPageContent);
  REQUIRE(!effect.preferences_changed);
  REQUIRE(sidebar.layout().mode == Mode::kHidden);
  REQUIRE(!sidebar.preferences().persistent_visible);
  REQUIRE(sidebar.preferences().preferred_width == 240);

  sidebar.Dispatch(Action::kPointerEnter, kOutside);
  sidebar.Dispatch(Action::kPointerLeave, kInside);
  REQUIRE(sidebar.layout().mode == Mode::kOverlay);
  REQUIRE(sidebar.Dispatch(Action::kFocusLeft, kInside).status ==
          Status::kInvalidInput);
  effect = sidebar.Dispatch(Action::kFocusLeft, kOutside);
  REQUIRE(effect.focus == FocusRequest::kNone);
  REQUIRE(sidebar.layout().mode == Mode::kHidden);

  sidebar.Dispatch(Action::kTogglePersistent, kOutside);
  effect = sidebar.Dispatch(Action::kLeave, kInside);
  REQUIRE(effect.focus == FocusRequest::kPageContent);
  REQUIRE(sidebar.layout().mode == Mode::kDocked);
  effect = sidebar.Dispatch(Action::kTogglePersistent, kInside);
  REQUIRE(effect.focus == FocusRequest::kPageContent);
}

void OverridesComposeWithoutPreferenceMutation() {
  for (const bool persistent : {false, true}) {
    auto sidebar = Sidebar(persistent);
    auto context = Bounds();
    context.presentation_override = PresentationOverride::kSuppress;
    auto effect = sidebar.Configure(context, kInside);
    REQUIRE(effect.focus ==
            (persistent ? FocusRequest::kPageContent : FocusRequest::kNone));
    const auto suppressed = sidebar.layout();
    REQUIRE(suppressed.mode == Mode::kHidden);
    REQUIRE(!suppressed.sidebar && !suppressed.reveal_target &&
            !suppressed.resize_target);
    REQUIRE(sidebar.Dispatch(Action::kKeyboardReveal, kOutside).status ==
            Status::kUnavailable);
    REQUIRE(sidebar.Dispatch(Action::kPointerEnter, kOutside).focus ==
            FocusRequest::kNone);

    context.presentation_override = PresentationOverride::kRevealOverlay;
    effect = sidebar.Configure(context, kOutside);
    REQUIRE(!effect.preferences_changed);
    REQUIRE(sidebar.layout().mode == Mode::kOverlay);
    REQUIRE(sidebar.layout().content == suppressed.content);
    REQUIRE(sidebar.preferences().persistent_visible == persistent);
    effect = sidebar.Dispatch(Action::kLeave, kInside);
    REQUIRE(effect.focus == FocusRequest::kPageContent);
    REQUIRE(sidebar.layout().mode == Mode::kOverlay);
    // Only caller composition changes the forced reveal override.
    context.presentation_override = PresentationOverride::kSuppress;
    sidebar.Configure(context, kInside);
    context.presentation_override = PresentationOverride::kNone;
    sidebar.Configure(context, kOutside);
    REQUIRE(sidebar.layout().mode ==
            (persistent ? Mode::kDocked : Mode::kHidden));
    REQUIRE(sidebar.preferences().persistent_visible == persistent);
    REQUIRE(sidebar.preferences().preferred_width == 240);
  }
}

void ResizeCommitAndCancel() {
  auto sidebar = Sidebar();
  REQUIRE(sidebar.BeginResize(100, 20).status == Status::kUnavailable);
  REQUIRE(sidebar.BeginResize(250, 620).status == Status::kUnavailable);
  REQUIRE(sidebar.UpdateResize(300).status == Status::kNoResize);
  auto effect = sidebar.BeginResize(250, 20);
  REQUIRE(effect.capture == CaptureRequest::kAcquire);
  REQUIRE(effect.focus == FocusRequest::kNone);
  REQUIRE(sidebar.BeginResize(250, 20).status == Status::kResizeActive);
  sidebar.UpdateResize(310);
  REQUIRE(sidebar.layout().resizing);
  REQUIRE(sidebar.layout().sidebar->width == 300);
  REQUIRE((sidebar.layout().content == Rect{310, 20, 500, 600}));
  REQUIRE(sidebar.preferences().preferred_width == 240);
  effect = sidebar.CommitResize();
  REQUIRE(effect.capture == CaptureRequest::kRelease);
  REQUIRE(effect.preferences_changed);
  REQUIRE(sidebar.preferences().preferred_width == 300);
  REQUIRE(!sidebar.layout().resizing);

  sidebar.BeginResize(310, 20);
  sidebar.UpdateResize(500);
  REQUIRE(sidebar.layout().sidebar->width == 360);
  sidebar.UpdateResize(-500);
  REQUIRE(sidebar.layout().sidebar->width == 120);
  effect = sidebar.CancelResize();
  REQUIRE(effect.capture == CaptureRequest::kRelease);
  REQUIRE(!effect.preferences_changed);
  REQUIRE(sidebar.layout().sidebar->width == 300);
  REQUIRE(sidebar.preferences().preferred_width == 300);
  REQUIRE(sidebar.CancelResize().status == Status::kNoResize);
  REQUIRE(sidebar.CommitResize().status == Status::kNoResize);

  auto context = Bounds();
  context.side = Side::kRight;
  auto right = Sidebar(true, context);
  REQUIRE((right.layout().sidebar == Rect{570, 20, 240, 600}));
  right.BeginResize(570, 20);
  right.UpdateResize(510);
  REQUIRE(right.layout().sidebar->width == 300);
  REQUIRE((right.layout().content == Rect{10, 20, 500, 600}));
  right.CommitResize();
  REQUIRE(right.preferences().preferred_width == 300);
}

void InterruptedResizeAndKeyboardLeave() {
  auto sidebar = Sidebar(false);
  sidebar.Dispatch(Action::kKeyboardReveal, kOutside);
  sidebar.BeginResize(250, 20);
  sidebar.UpdateResize(310);
  auto effect = sidebar.Dispatch(Action::kEscape, kInside);
  REQUIRE(effect.capture == CaptureRequest::kRelease);
  REQUIRE(effect.focus == FocusRequest::kNone);
  REQUIRE(sidebar.layout().mode == Mode::kOverlay);
  REQUIRE(sidebar.layout().sidebar->width == 240);
  effect = sidebar.Dispatch(Action::kEscape, kInside);
  REQUIRE(effect.focus == FocusRequest::kPageContent);
  REQUIRE(sidebar.layout().mode == Mode::kHidden);

  sidebar.Dispatch(Action::kPointerEnter, kOutside);
  sidebar.BeginResize(250, 20);
  sidebar.UpdateResize(310);
  sidebar.Dispatch(Action::kPointerLeave, kOutside);
  REQUIRE(sidebar.layout().mode == Mode::kOverlay);
  effect = sidebar.Dispatch(Action::kCaptureLost, kInside);
  REQUIRE(effect.capture == CaptureRequest::kRelease);
  REQUIRE(effect.focus == FocusRequest::kPageContent);
  REQUIRE(sidebar.layout().mode == Mode::kHidden);
  REQUIRE(sidebar.preferences().preferred_width == 240);

  sidebar.Dispatch(Action::kKeyboardReveal, kOutside);
  sidebar.BeginResize(250, 20);
  sidebar.UpdateResize(310);
  REQUIRE(sidebar.Configure(Bounds(), kInside).capture ==
          CaptureRequest::kNone);
  REQUIRE(sidebar.layout().resizing);
  auto context = Bounds();
  context.presentation_override = PresentationOverride::kSuppress;
  effect = sidebar.Configure(context, kInside);
  REQUIRE(effect.capture == CaptureRequest::kRelease);
  REQUIRE(effect.focus == FocusRequest::kPageContent);
  REQUIRE(sidebar.preferences().preferred_width == 240);
  sidebar.Configure(Bounds(), kOutside);
  REQUIRE(sidebar.layout().mode == Mode::kHidden);

  sidebar.Dispatch(Action::kTogglePersistent, kOutside);
  sidebar.BeginResize(250, 20);
  sidebar.UpdateResize(310);
  context = Bounds();
  context.available.x = 30;
  effect = sidebar.Configure(context, kInside);
  REQUIRE(effect.capture == CaptureRequest::kRelease);
  REQUIRE(effect.focus == FocusRequest::kNone);
  REQUIRE(sidebar.layout().sidebar->width == 240);
}

void NarrowFallbackAndHitMetrics() {
  auto sidebar = Sidebar();
  auto context = Bounds();
  context.available.width = 300;
  auto effect = sidebar.Configure(context, kInside);
  REQUIRE(effect.focus == FocusRequest::kPageContent);
  REQUIRE(sidebar.layout().width_fit == WidthFit::kInsufficientSpace);
  REQUIRE(sidebar.layout().mode == Mode::kHidden);
  REQUIRE(sidebar.layout().content == context.available);
  REQUIRE(!sidebar.layout().sidebar && !sidebar.layout().reveal_target &&
          !sidebar.layout().resize_target);
  REQUIRE(sidebar.Dispatch(Action::kKeyboardReveal, kOutside).status ==
          Status::kUnavailable);
  REQUIRE(sidebar.BeginResize(250, 20).status == Status::kUnavailable);
  REQUIRE(sidebar.preferences().persistent_visible);
  REQUIRE(sidebar.preferences().preferred_width == 240);
  for (auto mode :
       {PresentationOverride::kNone, PresentationOverride::kSuppress,
        PresentationOverride::kRevealOverlay}) {
    context.presentation_override = mode;
    sidebar.Configure(context, kOutside);
    REQUIRE(sidebar.layout().width_fit == WidthFit::kInsufficientSpace);
    REQUIRE(sidebar.layout().mode == Mode::kHidden);
    REQUIRE(!sidebar.layout().sidebar && !sidebar.layout().reveal_target &&
            !sidebar.layout().resize_target);
  }
  context.presentation_override = PresentationOverride::kNone;
  sidebar.Configure(Bounds(), kOutside);
  REQUIRE(sidebar.layout().sidebar->width == 240);
  REQUIRE(sidebar.layout().mode == Mode::kDocked);

  // Exact caller minimum fits. Constraining the window is not a durable edit.
  context.available.width = 320;
  context.resize_hit_width = 320;
  context.reveal_hit_width = 320;
  sidebar.Configure(context, kOutside);
  REQUIRE(sidebar.layout().width_fit == WidthFit::kFits);
  REQUIRE(sidebar.layout().sidebar->width == 120);
  REQUIRE(sidebar.layout().resize_target == context.available);
  sidebar.BeginResize(130, 20);
  REQUIRE(!sidebar.CommitResize().preferences_changed);
  REQUIRE(sidebar.preferences().preferred_width == 240);
  sidebar.Dispatch(Action::kTogglePersistent, kOutside);
  REQUIRE(sidebar.layout().reveal_target == context.available);
  sidebar.Dispatch(Action::kKeyboardReveal, kOutside);
  REQUIRE(sidebar.layout().resize_target == context.available);
  context.resize_hit_width = 321;
  context.reveal_hit_width = 321;
  sidebar.Configure(context, kOutside);
  REQUIRE(!sidebar.layout().resize_target);
  REQUIRE(sidebar.BeginResize(130, 20).status == Status::kUnavailable);
  sidebar.Dispatch(Action::kLeave, kInside);
  REQUIRE(!sidebar.layout().reveal_target);
  REQUIRE(sidebar.Dispatch(Action::kKeyboardReveal, kOutside).focus ==
          FocusRequest::kSidebar);

  // Scaled caller units and right-side input retain complete target metrics.
  context = Bounds();
  context.side = Side::kRight;
  context.available = {15, 30, 1200, 900};
  context.minimum_width = 180;
  context.maximum_width = 540;
  context.minimum_content_width = 300;
  context.resize_hit_width = 36;
  context.reveal_hit_width = 24;
  auto scaled = Sidebar(false, context);
  REQUIRE((scaled.layout().reveal_target == Rect{1191, 30, 24, 900}));
  scaled.Dispatch(Action::kPointerEnter, kOutside);
  REQUIRE(scaled.layout().resize_target->width == 36);
  REQUIRE(scaled.BeginResize(975, 30).capture == CaptureRequest::kAcquire);
  scaled.UpdateResize(915);
  REQUIRE(scaled.layout().sidebar->width == 300);

  context.available.height = 0;
  scaled.Configure(context, kOutside);
  REQUIRE(scaled.layout().mode == Mode::kHidden);
  REQUIRE(!scaled.layout().resize_target && !scaled.layout().reveal_target);
}

void InvalidInputsAreAtomic() {
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double infinity = std::numeric_limits<double>::infinity();
  REQUIRE(!Presentation::Create({true, nan}, Bounds()));
  REQUIRE(!Presentation::Create({true, 0}, Bounds()));
  auto invalid = Bounds();
  invalid.minimum_width = invalid.maximum_width + 1;
  REQUIRE(!Presentation::Create({true, 240}, invalid));
  invalid = Bounds();
  invalid.resize_hit_width = 0;
  REQUIRE(!Presentation::Create({true, 240}, invalid));
  auto sidebar = Sidebar();
  sidebar.BeginResize(250, 20);
  sidebar.UpdateResize(310);
  const auto before = sidebar.layout();
  invalid = Bounds();
  invalid.available.width = infinity;
  REQUIRE(sidebar.Configure(invalid, kInside).status == Status::kInvalidInput);
  REQUIRE(sidebar.layout().sidebar == before.sidebar);
  REQUIRE(sidebar.layout().resizing);
  REQUIRE(sidebar.UpdateResize(nan).status == Status::kInvalidInput);
  REQUIRE(sidebar.layout().sidebar == before.sidebar);
  REQUIRE(sidebar.BeginResize(infinity, 20).status == Status::kInvalidInput);
  sidebar.CancelResize();
  // Finite extreme pointer coordinates clamp; no coordinate quota is imposed.
  sidebar.BeginResize(250, 20);
  sidebar.UpdateResize(std::numeric_limits<double>::max());
  REQUIRE(sidebar.layout().sidebar->width == 360);
  sidebar.UpdateResize(-std::numeric_limits<double>::max());
  REQUIRE(sidebar.layout().sidebar->width == 120);
}

}  // namespace
}  // namespace arcified::sidebar

int main() {
  using namespace arcified::sidebar;
  PersistentAndTemporary();
  OverridesComposeWithoutPreferenceMutation();
  ResizeCommitAndCancel();
  InterruptedResizeAndKeyboardLeave();
  NarrowFallbackAndHitMetrics();
  InvalidInputsAreAtomic();
  std::cout << "sidebar presentation fixtures passed\n";
}
