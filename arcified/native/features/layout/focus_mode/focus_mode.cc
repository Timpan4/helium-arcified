#include "focus_mode.h"

namespace arcified::focus_mode {
namespace {

bool HostOwnsChrome(const HostFacts& facts) {
  return facts.dialog_open || facts.fullscreen == Fullscreen::kExclusive;
}

bool CanMoveFocus(const HostFacts& facts) {
  return !HostOwnsChrome(facts) && facts.focus != FocusLocation::kOutsideHost;
}

}  // namespace

Projection Presentation::Project(const HostFacts& facts) const {
  using Override = sidebar::PresentationOverride;
  if (!active())
    return {false, Override::kNone};
  // Never hide chrome that still contains real focus, including a failed or
  // cancelled host return-focus request. Reproject after the host reports it.
  if (facts.focus == FocusLocation::kChrome ||
      (!HostOwnsChrome(facts) &&
       (state_ == State::kPointerReveal || state_ == State::kKeyboardReveal))) {
    return {true, Override::kRevealOverlay};
  }
  return {true, Override::kSuppress};
}

Result Presentation::Dispatch(Action action, const HostFacts& facts) {
  FocusIntent focus = FocusIntent::kPreserve;
  Disposition disposition = Disposition::kHandled;
  // A modal/native fullscreen transition invalidates old reveal gestures.
  // Returning to the page later starts from current facts, not a saved reveal.
  if (active() && HostOwnsChrome(facts))
    state_ = State::kSuppressed;

  if (action == Action::kToggle)
    action = active() ? Action::kExit : Action::kEnter;

  switch (action) {
    case Action::kEnter:
      if (active()) {
        disposition = Disposition::kIgnored;
        break;
      }
      state_ = State::kSuppressed;
      if (CanMoveFocus(facts) && facts.focus == FocusLocation::kChrome)
        focus = FocusIntent::kActiveContent;
      break;
    case Action::kExit:
      if (!active()) {
        disposition = Disposition::kIgnored;
        break;
      }
      state_ = State::kNormal;
      if (CanMoveFocus(facts) && facts.focus == FocusLocation::kChrome)
        focus = FocusIntent::kReturnOrFallback;
      break;
    case Action::kRevealPointer:
    case Action::kRevealKeyboard:
      if (!active()) {
        disposition = Disposition::kIgnored;
        break;
      }
      if (HostOwnsChrome(facts)) {
        disposition = Disposition::kDeferToHost;
        break;
      }
      if (action == Action::kRevealKeyboard) {
        state_ = State::kKeyboardReveal;
        if (CanMoveFocus(facts) && facts.focus != FocusLocation::kChrome)
          focus = FocusIntent::kControls;
      } else if (state_ != State::kKeyboardReveal) {
        state_ = State::kPointerReveal;
      }
      break;
    case Action::kPointerLeave:
      if (state_ == State::kPointerReveal)
        state_ = State::kSuppressed;
      else
        disposition = Disposition::kIgnored;
      break;
    case Action::kDismissReveal:
      if (!active()) {
        disposition = Disposition::kIgnored;
        break;
      }
      if (HostOwnsChrome(facts)) {
        disposition = Disposition::kDeferToHost;
        break;
      }
      state_ = State::kSuppressed;
      if (CanMoveFocus(facts) && facts.focus == FocusLocation::kChrome)
        focus = FocusIntent::kReturnOrFallback;
      break;
    case Action::kEscape:
      // Do not consume native dialog/fullscreen Escape, even with focus mode
      // active. Explicit Exit remains available and does not exit fullscreen.
      if (facts.dialog_open || facts.fullscreen != Fullscreen::kNone) {
        disposition = Disposition::kDeferToHost;
      } else if (active()) {
        state_ = State::kNormal;
        if (CanMoveFocus(facts) && facts.focus == FocusLocation::kChrome)
          focus = FocusIntent::kReturnOrFallback;
      } else {
        disposition = Disposition::kIgnored;
      }
      break;
    case Action::kHostChanged:
    case Action::kToggle:
      // Normalized above, so the transition always uses the same command path.
      break;
  }
  return {Project(facts), focus, disposition};
}

}  // namespace arcified::focus_mode
