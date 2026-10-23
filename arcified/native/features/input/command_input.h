#ifndef ARCIFIED_NATIVE_FEATURES_INPUT_COMMAND_INPUT_H_
#define ARCIFIED_NATIVE_FEATURES_INPUT_COMMAND_INPUT_H_

#include <string_view>
#include <variant>

namespace arcified::input {

// Callers opt in only to gestures appropriate for their control. These are
// semantic keys after native layout resolution, never physical scan codes.
enum class Gesture { kNone, kAccept, kDismiss, kPreviousResult, kNextResult };
enum class Key { kOther, kEnter, kEscape, kUp, kDown, kTextSystem };
enum class EventKind { kOther, kPress, kRelease, kCharacter, kTextEdit };

struct Modifiers {
  bool shift = false;
  bool control = false;
  bool alt = false;
  bool meta = false;
  bool alt_graph = false;
};

struct KeyEvent {
  EventKind kind = EventKind::kOther;
  // Dead/Compose/Process keys and native Unicode-entry sequences map to
  // kTextSystem even when a legacy virtual key resembles a command key.
  Key key = Key::kOther;
  Modifiers modifiers;
  bool repeat = false;
};

enum class Activity { kUnknown, kInactive, kActive };

struct TextActivity {
  Activity composition = Activity::kUnknown;
  Activity candidate_popup = Activity::kUnknown;
  // Includes asynchronous IME work and a pending dead-key/Compose sequence
  // without visible preedit. Absence of composition does not prove inactivity.
  Activity pending_text_input = Activity::kUnknown;
};

enum class Dispatch { kUnverified, kBeforeIme, kHandled, kPostImeUnhandled };
enum class Focus {
  kUnverified,
  kSameFocusedClient,
  kNoFocusedClient,
  kChangedDuringDispatch
};

// A synchronous observation for this event, supplied by the native owner.
// Neither focus nor activity is inferred or retained here. A before/after pair
// must describe the same event and focused client; otherwise use kUnverified or
// kChangedDuringDispatch. Do not reuse a decision after a callback or mutation.
struct NativeInput {
  Dispatch dispatch = Dispatch::kUnverified;
  Focus focus = Focus::kUnverified;
  TextActivity before;
  TextActivity after;
};

enum class WithheldReason {
  kInvalidFacts,
  kNoGesture,
  kNativeInputUnverified,
  kNativeInputBusy,
  kNoFocus,
  kStaleFocus,
  kNotCommandKey,
  kModifiedOrRepeatedKey,
  kGestureMismatch,
  kInvalidUtf16
};

// Withholding grants no browser command. It neither consumes the event nor
// asks the caller to replay it. Already-handled native events stay handled.
struct Withheld {
  WithheldReason reason;
};

struct AcceptedText {
  // Exact native UTF-16 text, borrowed until the native buffer changes or dies.
  // Includes whitespace, combining marks, bidi controls and embedded NULs.
  // An asynchronous receiver must copy it before any native editing/callback.
  std::u16string_view text;
};

enum class ControlCommand { kDismiss, kPreviousResult, kNextResult };
using Decision = std::variant<Withheld, AcceptedText, ControlCommand>;

// Validates caller facts and gates unmodified, nonrepeated
// Enter/Escape/Up/Down. Only accept examines text, and only for well-formed
// UTF-16. No normalization, trimming, case conversion, bidi reordering,
// clipping or length cap is applied. This is permission to dispatch the
// requested local input gesture immediately, not permission to navigate or
// mutate a page/Profile/Space.
Decision ArbitrateInput(Gesture requested,
                        const KeyEvent& event,
                        const NativeInput& native,
                        std::u16string_view text);

}  // namespace arcified::input

#endif  // ARCIFIED_NATIVE_FEATURES_INPUT_COMMAND_INPUT_H_
