#include "focus_mode.h"

#include <cstdlib>
#include <iostream>
#include <utility>

namespace arcified::focus_mode {
namespace {

using Override = sidebar::PresentationOverride;

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

void CommandsShareReversibleMode() {
  Presentation mode;
  HostFacts host;
  REQUIRE(!mode.Project(host).active);
  REQUIRE(mode.Project(host).chrome == Override::kNone);
  REQUIRE(mode.Dispatch(Action::kExit, host).disposition ==
          Disposition::kIgnored);
  auto result = mode.Dispatch(Action::kEnter, host);
  REQUIRE(result.projection.active);
  REQUIRE(result.projection.chrome == Override::kSuppress);
  REQUIRE(result.focus == FocusIntent::kPreserve);
  REQUIRE(mode.Dispatch(Action::kEnter, host).disposition ==
          Disposition::kIgnored);
  REQUIRE(!mode.Dispatch(Action::kToggle, host).projection.active);
  REQUIRE(mode.Dispatch(Action::kToggle, host).projection.active);
  result = mode.Dispatch(Action::kEscape, host);
  REQUIRE(result.disposition == Disposition::kHandled);
  REQUIRE(!result.projection.active);
  REQUIRE(result.projection.chrome == Override::kNone);
  REQUIRE(mode.Dispatch(Action::kEscape, host).disposition ==
          Disposition::kIgnored);
}

void EntryWaitsForAuthoritativeFocus() {
  Presentation mode;
  HostFacts host{FocusLocation::kChrome};
  auto result = mode.Dispatch(Action::kEnter, host);
  REQUIRE(result.focus == FocusIntent::kActiveContent);
  REQUIRE(result.projection.chrome == Override::kRevealOverlay);
  // A failed native focus request must not hide the still-focused control.
  REQUIRE(mode.Dispatch(Action::kHostChanged, host).projection.chrome ==
          Override::kRevealOverlay);
  host.focus = FocusLocation::kContent;
  REQUIRE(mode.Dispatch(Action::kHostChanged, host).projection.chrome ==
          Override::kSuppress);
}

void PointerRevealDoesNotMoveFocus() {
  Presentation mode;
  HostFacts host;
  REQUIRE(mode.Dispatch(Action::kRevealPointer, host).disposition ==
          Disposition::kIgnored);
  mode.Dispatch(Action::kEnter, host);
  auto result = mode.Dispatch(Action::kRevealPointer, host);
  REQUIRE(result.projection.chrome == Override::kRevealOverlay);
  REQUIRE(result.focus == FocusIntent::kPreserve);
  result = mode.Dispatch(Action::kPointerLeave, host);
  REQUIRE(result.projection.chrome == Override::kSuppress);
  REQUIRE(result.focus == FocusIntent::kPreserve);
  mode.Dispatch(Action::kRevealPointer, host);
  host.focus = FocusLocation::kChrome;
  REQUIRE(mode.Dispatch(Action::kPointerLeave, host).projection.chrome ==
          Override::kRevealOverlay);
  host.focus = FocusLocation::kContent;
  REQUIRE(mode.Dispatch(Action::kHostChanged, host).projection.chrome ==
          Override::kSuppress);
}

void KeyboardDismissWaitsForNativeReturn() {
  Presentation mode;
  HostFacts host;
  mode.Dispatch(Action::kEnter, host);
  auto result = mode.Dispatch(Action::kRevealKeyboard, host);
  REQUIRE(result.focus == FocusIntent::kControls);
  REQUIRE(result.projection.chrome == Override::kRevealOverlay);
  mode.Dispatch(Action::kRevealPointer, host);
  // Pointer exit cannot cancel an explicit keyboard reveal.
  REQUIRE(mode.Dispatch(Action::kPointerLeave, host).projection.chrome ==
          Override::kRevealOverlay);
  host.focus = FocusLocation::kChrome;
  result = mode.Dispatch(Action::kDismissReveal, host);
  REQUIRE(result.focus == FocusIntent::kReturnOrFallback);
  REQUIRE(result.projection.chrome == Override::kRevealOverlay);
  // Delayed, cancelled or failed actuation reports the same actual focus.
  result = mode.Dispatch(Action::kHostChanged, host);
  REQUIRE(result.focus == FocusIntent::kPreserve);
  REQUIRE(result.projection.chrome == Override::kRevealOverlay);
  host.focus = FocusLocation::kContent;
  result = mode.Dispatch(Action::kHostChanged, host);
  REQUIRE(result.projection.active);
  REQUIRE(result.projection.chrome == Override::kSuppress);
}

void ExitReleasesOnlyFocusMode() {
  Presentation mode;
  HostFacts host{FocusLocation::kChrome};
  mode.Dispatch(Action::kEnter, host);
  const auto result = mode.Dispatch(Action::kExit, host);
  REQUIRE(!result.projection.active);
  REQUIRE(result.projection.chrome == Override::kNone);
  REQUIRE(result.focus == FocusIntent::kReturnOrFallback);
  // The component did not pretend that native focus moved on dispatch.
  REQUIRE(host.focus == FocusLocation::kChrome);
}

void DialogAndFullscreenKeepHostCommandPriority() {
  for (const auto fullscreen : {Fullscreen::kNone, Fullscreen::kOverlayAllowed,
                                Fullscreen::kExclusive}) {
    for (const bool dialog : {false, true}) {
      if (fullscreen == Fullscreen::kNone && !dialog)
        continue;
      Presentation mode;
      HostFacts host{FocusLocation::kContent, fullscreen, dialog};
      REQUIRE(mode.Dispatch(Action::kEnter, host).projection.active);
      auto result = mode.Dispatch(Action::kEscape, host);
      REQUIRE(result.disposition == Disposition::kDeferToHost);
      REQUIRE(result.projection.active);
      REQUIRE(result.focus == FocusIntent::kPreserve);
      REQUIRE(result.projection.chrome == Override::kSuppress);
      result = mode.Dispatch(Action::kRevealKeyboard, host);
      if (dialog || fullscreen == Fullscreen::kExclusive) {
        REQUIRE(result.disposition == Disposition::kDeferToHost);
        REQUIRE(result.focus == FocusIntent::kPreserve);
      } else {
        REQUIRE(result.focus == FocusIntent::kControls);
        REQUIRE(result.projection.chrome == Override::kRevealOverlay);
      }
      // Explicit menu/accessible Exit releases mode, never native fullscreen.
      result = mode.Dispatch(Action::kExit, host);
      REQUIRE(!result.projection.active);
      REQUIRE(host.fullscreen == fullscreen);
      REQUIRE(host.dialog_open == dialog);
    }
  }
}

void ModalTransitionCancelsStaleRevealWithoutFocusTheft() {
  Presentation mode;
  HostFacts host;
  mode.Dispatch(Action::kEnter, host);
  mode.Dispatch(Action::kRevealKeyboard, host);
  host.dialog_open = true;
  auto result = mode.Dispatch(Action::kHostChanged, host);
  REQUIRE(result.projection.chrome == Override::kSuppress);
  REQUIRE(result.focus == FocusIntent::kPreserve);
  REQUIRE(mode.Dispatch(Action::kDismissReveal, host).disposition ==
          Disposition::kDeferToHost);
  host.dialog_open = false;
  REQUIRE(mode.Dispatch(Action::kHostChanged, host).projection.chrome ==
          Override::kSuppress);
  mode.Dispatch(Action::kRevealPointer, host);
  host.fullscreen = Fullscreen::kExclusive;
  REQUIRE(mode.Dispatch(Action::kHostChanged, host).projection.chrome ==
          Override::kSuppress);
  host.fullscreen = Fullscreen::kNone;
  REQUIRE(mode.Dispatch(Action::kHostChanged, host).projection.chrome ==
          Override::kSuppress);
}

void OutsideHostNeverReceivesFocusIntent() {
  Presentation first;
  Presentation second;
  HostFacts outside{FocusLocation::kOutsideHost};
  REQUIRE(first.Dispatch(Action::kEnter, outside).focus ==
          FocusIntent::kPreserve);
  REQUIRE(first.Dispatch(Action::kRevealKeyboard, outside).focus ==
          FocusIntent::kPreserve);
  REQUIRE(first.Dispatch(Action::kDismissReveal, outside).focus ==
          FocusIntent::kPreserve);
  REQUIRE(first.Dispatch(Action::kExit, outside).focus ==
          FocusIntent::kPreserve);
  REQUIRE(!second.Project(HostFacts{}).active);
  REQUIRE(second.Project(HostFacts{}).chrome == Override::kNone);
}

// Geometry and size values below are synthetic caller fixtures, not defaults or
// product limits. Exercise the actual #51 projection, not a sidebar mock.
sidebar::Context Context(sidebar::Side side) {
  return {{-1200, 40, 1200, 800}, side, 100, 500, 400, 8, 12, Override::kNone};
}

void SidebarCompositionKeepsContentBoundsThroughRevealAndDialog() {
  for (const auto side : {sidebar::Side::kLeft, sidebar::Side::kRight}) {
    for (const bool visible : {false, true}) {
      auto context = Context(side);
      auto sidebar = sidebar::Presentation::Create({visible, 240}, context);
      REQUIRE(sidebar.has_value());
      Presentation mode;
      HostFacts host;
      const auto apply = [&](const Result& result) {
        context.presentation_override = result.projection.chrome;
        const auto effect = sidebar->Configure(
            context, sidebar::FocusLocation::kOutsideSidebar);
        REQUIRE(effect.status == sidebar::Status::kOk);
        REQUIRE(!effect.preferences_changed);
        REQUIRE(sidebar->layout().content == context.available);
      };
      apply(mode.Dispatch(Action::kEnter, host));
      REQUIRE(sidebar->layout().mode == sidebar::Mode::kHidden);
      apply(mode.Dispatch(Action::kRevealPointer, host));
      REQUIRE(sidebar->layout().mode == sidebar::Mode::kOverlay);
      apply(mode.Dispatch(Action::kPointerLeave, host));
      host.dialog_open = true;
      apply(mode.Dispatch(Action::kHostChanged, host));
      REQUIRE(sidebar->layout().mode == sidebar::Mode::kHidden);
      apply(mode.Dispatch(Action::kEscape, host));
      host.dialog_open = false;
      apply(mode.Dispatch(Action::kHostChanged, host));
      REQUIRE(sidebar->layout().mode == sidebar::Mode::kHidden);
      host.fullscreen = Fullscreen::kExclusive;
      apply(mode.Dispatch(Action::kHostChanged, host));
      apply(mode.Dispatch(Action::kEscape, host));
      host.fullscreen = Fullscreen::kNone;
      apply(mode.Dispatch(Action::kHostChanged, host));
      REQUIRE(sidebar->layout().mode == sidebar::Mode::kHidden);
      REQUIRE(sidebar->preferences().persistent_visible == visible);
      REQUIRE(sidebar->preferences().preferred_width == 240);
    }
  }
}

void SidebarDismissKeepsFocusedControlsUntilHostReturn() {
  auto context = Context(sidebar::Side::kRight);
  auto sidebar = sidebar::Presentation::Create({false, 240}, context);
  REQUIRE(sidebar.has_value());
  Presentation mode;
  HostFacts host;
  const auto apply = [&](Action action) {
    const auto result = mode.Dispatch(action, host);
    context.presentation_override = result.projection.chrome;
    const auto effect = sidebar->Configure(
        context, host.focus == FocusLocation::kChrome
                     ? sidebar::FocusLocation::kInsideSidebar
                     : sidebar::FocusLocation::kOutsideSidebar);
    REQUIRE(effect.status == sidebar::Status::kOk);
    REQUIRE(effect.focus == sidebar::FocusRequest::kNone);
    REQUIRE(sidebar->layout().content == context.available);
    return result;
  };
  apply(Action::kEnter);
  REQUIRE(apply(Action::kRevealKeyboard).focus == FocusIntent::kControls);
  host.focus = FocusLocation::kChrome;
  REQUIRE(apply(Action::kDismissReveal).focus ==
          FocusIntent::kReturnOrFallback);
  REQUIRE(sidebar->layout().mode == sidebar::Mode::kOverlay);
  apply(Action::kHostChanged);
  REQUIRE(sidebar->layout().mode == sidebar::Mode::kOverlay);
  host.focus = FocusLocation::kContent;
  apply(Action::kHostChanged);
  REQUIRE(sidebar->layout().mode == sidebar::Mode::kHidden);
}

void ExitProjectsCurrentBoundsSideAndPreferences() {
  auto context = Context(sidebar::Side::kLeft);
  auto sidebar = sidebar::Presentation::Create({false, 240}, context);
  REQUIRE(sidebar.has_value());
  Presentation mode;
  HostFacts host;
  const auto apply = [&](Action action) {
    context.presentation_override =
        mode.Dispatch(action, host).projection.chrome;
    REQUIRE(sidebar->Configure(context, sidebar::FocusLocation::kOutsideSidebar)
                .status == sidebar::Status::kOk);
  };
  apply(Action::kEnter);
  // Normal preference changes during suppression remain owned by #51.
  REQUIRE(sidebar
              ->Dispatch(sidebar::Action::kTogglePersistent,
                         sidebar::FocusLocation::kOutsideSidebar)
              .preferences_changed);
  REQUIRE(sidebar->layout().mode == sidebar::Mode::kHidden);
  context.available = {1600, -900, 1400, 900};
  context.side = sidebar::Side::kRight;
  apply(Action::kHostChanged);
  REQUIRE(sidebar->layout().content == context.available);
  apply(Action::kRevealKeyboard);
  REQUIRE(sidebar->layout().mode == sidebar::Mode::kOverlay);
  REQUIRE(sidebar->layout().content == context.available);
  REQUIRE(sidebar->layout().sidebar == (sidebar::Rect{2760, -900, 240, 900}));
  // A deliberate resize while overlaid updates current preference only.
  REQUIRE(sidebar->BeginResize(2760, -800).status == sidebar::Status::kOk);
  REQUIRE(sidebar->UpdateResize(2700).status == sidebar::Status::kOk);
  REQUIRE(sidebar->CommitResize().preferences_changed);
  REQUIRE(sidebar->preferences().preferred_width == 300);
  context.available = {-1400, 60, 1400, 1000};
  context.side = sidebar::Side::kLeft;
  apply(Action::kHostChanged);
  REQUIRE(sidebar->layout().content == context.available);
  apply(Action::kExit);
  REQUIRE(sidebar->layout().mode == sidebar::Mode::kDocked);
  REQUIRE(sidebar->layout().sidebar == (sidebar::Rect{-1400, 60, 300, 1000}));
  REQUIRE(sidebar->layout().content == (sidebar::Rect{-1100, 60, 1100, 1000}));
  REQUIRE(sidebar->preferences().persistent_visible);
}

}  // namespace
}  // namespace arcified::focus_mode

int main() {
  using namespace arcified::focus_mode;
  const std::pair<const char*, void (*)()> fixtures[] = {
      {"commands-and-reversible-mode", CommandsShareReversibleMode},
      {"entry-authoritative-focus", EntryWaitsForAuthoritativeFocus},
      {"pointer-reveal", PointerRevealDoesNotMoveFocus},
      {"keyboard-dismiss-native-return", KeyboardDismissWaitsForNativeReturn},
      {"exit-releases-only-mode", ExitReleasesOnlyFocusMode},
      {"dialog-fullscreen-priority",
       DialogAndFullscreenKeepHostCommandPriority},
      {"modal-cancels-transient-reveal",
       ModalTransitionCancelsStaleRevealWithoutFocusTheft},
      {"outside-host-and-independent-hosts",
       OutsideHostNeverReceivesFocusIntent},
      {"sidebar-reveal-dialog-content-bounds",
       SidebarCompositionKeepsContentBoundsThroughRevealAndDialog},
      {"sidebar-dismiss-authoritative-focus",
       SidebarDismissKeepsFocusedControlsUntilHostReturn},
      {"exit-current-bounds-side-preferences",
       ExitProjectsCurrentBoundsSideAndPreferences},
  };
  for (const auto& [name, run] : fixtures) {
    run();
    std::cout << "PASS " << name << '\n';
  }
}
