#ifndef ARCIFIED_NATIVE_FEATURES_LAYOUT_FOCUS_MODE_FOCUS_MODE_H_
#define ARCIFIED_NATIVE_FEATURES_LAYOUT_FOCUS_MODE_FOCUS_MODE_H_

#include "../sidebar/sidebar.h"

namespace arcified::focus_mode {

// Current host facts, never retained by Presentation. Chrome includes toolbar,
// sidebar and their menus; the host resolves actual controls and live pages.
enum class FocusLocation { kContent, kChrome, kOutsideHost };
// The host decides whether its fullscreen implementation permits an overlay.
// Both fullscreen cases keep Escape in the native fullscreen command path.
enum class Fullscreen { kNone, kOverlayAllowed, kExclusive };
struct HostFacts {
  FocusLocation focus = FocusLocation::kContent;
  Fullscreen fullscreen = Fullscreen::kNone;
  bool dialog_open = false;
};

enum class Action {
  kEnter,
  kExit,
  kToggle,
  // Host-classified intentional activation, never ordinary pointer movement.
  kRevealPointer,
  kPointerLeave,
  kRevealKeyboard,
  kDismissReveal,
  kEscape,
  // Report current facts after native focus, fullscreen or dialog changes.
  kHostChanged,
};

enum class FocusIntent {
  kPreserve,
  kActiveContent,
  kControls,
  // Host/accessibility resolves its existing return target, validates liveness
  // and visibility, then falls back to current active content/empty host UI.
  // No target identity or focus history is stored here.
  kReturnOrFallback,
};
enum class Disposition { kHandled, kIgnored, kDeferToHost };

struct Projection {
  bool active;
  // Apply to sidebar and analogous toolbar/tabstrip presentation. Overlay
  // reveal must use the same content bounds as suppression, never dock chrome.
  // Dialog/fullscreen ownership never releases this overlay to normal docked
  // preferences. Exclusive fullscreen still owns its native controls and may
  // restrict browser overlays; this does not command their native visibility.
  sidebar::PresentationOverride chrome;
};
struct Result {
  Projection projection;
  FocusIntent focus = FocusIntent::kPreserve;
  Disposition disposition = Disposition::kIgnored;
};

// Per-host, UI-sequence-only presentation reducer. Owns only mode and transient
// reveal. No page/layout/window identities, preferences, geometry, focus
// target, saved fullscreen/dialog state, timers, persistence or native effects.
//
// The host must keep an accessible Exit command reachable whenever active,
// including when chrome is suppressed or exclusively managed by fullscreen.
// Keyboard/menu/exit-affordance commands share kEnter/kExit/kToggle. Native
// actuation and subsequent fact reports remain separate from these intents.
class Presentation {
 public:
  Result Dispatch(Action action, const HostFacts& facts);
  Projection Project(const HostFacts& facts) const;

 private:
  enum class State { kNormal, kSuppressed, kPointerReveal, kKeyboardReveal };
  bool active() const { return state_ != State::kNormal; }
  State state_ = State::kNormal;
};

}  // namespace arcified::focus_mode

#endif  // ARCIFIED_NATIVE_FEATURES_LAYOUT_FOCUS_MODE_FOCUS_MODE_H_
