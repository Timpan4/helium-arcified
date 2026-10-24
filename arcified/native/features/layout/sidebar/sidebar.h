#ifndef ARCIFIED_NATIVE_FEATURES_LAYOUT_SIDEBAR_SIDEBAR_H_
#define ARCIFIED_NATIVE_FEATURES_LAYOUT_SIDEBAR_SIDEBAR_H_

#include <optional>

namespace arcified::sidebar {

enum class Side { kLeft, kRight };
// Caller composition, including Focus mode. Never changes Preferences.
enum class PresentationOverride { kNone, kSuppress, kRevealOverlay };
enum class Mode { kHidden, kDocked, kOverlay };
enum class WidthFit { kFits, kInsufficientSpace };
enum class FocusLocation { kOutsideSidebar, kInsideSidebar };
enum class FocusRequest { kNone, kSidebar, kPageContent };
enum class CaptureRequest { kNone, kAcquire, kRelease };
enum class Status {
  kOk,
  kInvalidInput,
  kUnavailable,
  kResizeActive,
  kNoResize
};

struct Rect {
  double x;
  double y;
  double width;
  double height;
  bool Contains(double point_x, double point_y) const;
  friend bool operator==(const Rect& a, const Rect& b);
};

struct Preferences {
  bool persistent_visible;
  double preferred_width;
};

// All distances and pointer coordinates use the caller's same logical units.
// No product size, scale factor, fullscreen or native focus is inferred here.
struct Context {
  Rect available;
  Side side;
  double minimum_width;
  double maximum_width;
  double minimum_content_width;
  double reveal_hit_width;
  double resize_hit_width;
  PresentationOverride presentation_override;
  friend bool operator==(const Context& a, const Context& b);
};

struct Layout {
  Mode mode;
  // Insufficient caller minima/height -> hidden, no sidebar or hit targets.
  WidthFit width_fit;
  Rect content;
  std::optional<Rect> sidebar;
  std::optional<Rect> reveal_target;
  std::optional<Rect> resize_target;
  bool resizing;
};

struct Effect {
  Status status = Status::kOk;
  FocusRequest focus = FocusRequest::kNone;
  CaptureRequest capture = CaptureRequest::kNone;
  // Host resolves focus/capture safely before hiding native controls. Effects
  // do not acknowledge native success; see docs/features/sidebar-layout.md.
  // Caller may persist the new preferences. This does not claim disk success.
  bool preferences_changed = false;
};

enum class Action {
  kTogglePersistent,
  kPointerEnter,
  kPointerLeave,
  kKeyboardReveal,
  kLeave,
  kFocusLeft,
  // Escape cancels a resize first; otherwise leaves temporary reveal.
  kEscape,
  kCaptureLost,
};

// Single-sequence presentation component. No pages, focus handles, registry,
// navigation, selection, animation clock or persistence engine are owned here.
class Presentation {
 public:
  // Nonfinite or invalid size inputs are refused before construction.
  static std::optional<Presentation> Create(Preferences preferences,
                                            Context context);
  Effect Configure(Context context, FocusLocation focus);
  Effect Dispatch(Action action, FocusLocation focus);
  // Begin must hit the current resize target. Host owns pointer/touch capture
  // and routes only the captured gesture's updates. Failed capture -> cancel.
  Effect BeginResize(double pointer_x, double pointer_y);
  Effect UpdateResize(double pointer_x);
  Effect CommitResize();
  Effect CancelResize();

  // Authoritative immediately after every operation. Motion only consumes it.
  Layout layout() const;
  Preferences preferences() const { return preferences_; }

 private:
  enum class Reveal { kNone, kPointer, kEngaged };
  struct Resize {
    double start_pointer_x;
    double start_width;
    double draft_width;
  };
  Presentation(Preferences preferences, Context context);
  double ConstrainWidth(double width) const;
  bool HasRoom() const;
  bool CanPresent() const;
  Effect FinishChange(bool was_visible,
                      FocusLocation focus,
                      bool release_capture,
                      bool preferences_changed);

  Preferences preferences_;
  Context context_;
  Reveal reveal_ = Reveal::kNone;
  std::optional<Resize> resize_;
};

}  // namespace arcified::sidebar

#endif  // ARCIFIED_NATIVE_FEATURES_LAYOUT_SIDEBAR_SIDEBAR_H_
