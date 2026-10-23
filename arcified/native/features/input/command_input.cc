#include "command_input.h"

#include <cstddef>

namespace arcified::input {
namespace {

bool IsKnown(Gesture value) {
  switch (value) {
    case Gesture::kNone:
    case Gesture::kAccept:
    case Gesture::kDismiss:
    case Gesture::kPreviousResult:
    case Gesture::kNextResult:
      return true;
  }
  return false;
}

bool IsKnown(Key value) {
  switch (value) {
    case Key::kOther:
    case Key::kEnter:
    case Key::kEscape:
    case Key::kUp:
    case Key::kDown:
    case Key::kTextSystem:
      return true;
  }
  return false;
}

bool IsKnown(EventKind value) {
  switch (value) {
    case EventKind::kOther:
    case EventKind::kPress:
    case EventKind::kRelease:
    case EventKind::kCharacter:
    case EventKind::kTextEdit:
      return true;
  }
  return false;
}

bool IsKnown(Activity value) {
  switch (value) {
    case Activity::kUnknown:
    case Activity::kInactive:
    case Activity::kActive:
      return true;
  }
  return false;
}

bool IsKnown(Dispatch value) {
  switch (value) {
    case Dispatch::kUnverified:
    case Dispatch::kBeforeIme:
    case Dispatch::kHandled:
    case Dispatch::kPostImeUnhandled:
      return true;
  }
  return false;
}

bool IsKnown(Focus value) {
  switch (value) {
    case Focus::kUnverified:
    case Focus::kSameFocusedClient:
    case Focus::kNoFocusedClient:
    case Focus::kChangedDuringDispatch:
      return true;
  }
  return false;
}

bool IsKnown(const TextActivity& activity) {
  return IsKnown(activity.composition) && IsKnown(activity.candidate_popup) &&
         IsKnown(activity.pending_text_input);
}

bool Contains(const TextActivity& activity, Activity value) {
  return activity.composition == value || activity.candidate_popup == value ||
         activity.pending_text_input == value;
}

bool IsWellFormedUtf16(std::u16string_view text) {
  for (std::size_t i = 0; i < text.size(); ++i) {
    const char16_t unit = text[i];
    if (unit >= 0xD800 && unit <= 0xDBFF) {
      if (++i == text.size() || text[i] < 0xDC00 || text[i] > 0xDFFF) {
        return false;
      }
    } else if (unit >= 0xDC00 && unit <= 0xDFFF) {
      return false;
    }
  }
  return true;
}

}  // namespace

Decision ArbitrateInput(Gesture requested,
                        const KeyEvent& event,
                        const NativeInput& native,
                        std::u16string_view text) {
  if (!IsKnown(requested) || !IsKnown(event.kind) || !IsKnown(event.key) ||
      !IsKnown(native.dispatch) || !IsKnown(native.focus) ||
      !IsKnown(native.before) || !IsKnown(native.after)) {
    return Withheld{WithheldReason::kInvalidFacts};
  }
  if (requested == Gesture::kNone) {
    return Withheld{WithheldReason::kNoGesture};
  }
  if (native.dispatch == Dispatch::kHandled ||
      Contains(native.before, Activity::kActive) ||
      Contains(native.after, Activity::kActive)) {
    return Withheld{WithheldReason::kNativeInputBusy};
  }
  if (native.focus == Focus::kChangedDuringDispatch) {
    return Withheld{WithheldReason::kStaleFocus};
  }
  if (native.focus == Focus::kNoFocusedClient) {
    return Withheld{WithheldReason::kNoFocus};
  }
  if (native.dispatch != Dispatch::kPostImeUnhandled ||
      native.focus != Focus::kSameFocusedClient ||
      Contains(native.before, Activity::kUnknown) ||
      Contains(native.after, Activity::kUnknown)) {
    return Withheld{WithheldReason::kNativeInputUnverified};
  }
  if (event.kind != EventKind::kPress || event.key == Key::kOther ||
      event.key == Key::kTextSystem) {
    return Withheld{WithheldReason::kNotCommandKey};
  }
  const auto& modifiers = event.modifiers;
  if (event.repeat || modifiers.shift || modifiers.control || modifiers.alt ||
      modifiers.meta || modifiers.alt_graph) {
    return Withheld{WithheldReason::kModifiedOrRepeatedKey};
  }
  switch (requested) {
    case Gesture::kAccept:
      if (event.key == Key::kEnter) {
        if (!IsWellFormedUtf16(text)) {
          return Withheld{WithheldReason::kInvalidUtf16};
        }
        return AcceptedText{text};
      }
      break;
    case Gesture::kDismiss:
      if (event.key == Key::kEscape) {
        return ControlCommand::kDismiss;
      }
      break;
    case Gesture::kPreviousResult:
      if (event.key == Key::kUp) {
        return ControlCommand::kPreviousResult;
      }
      break;
    case Gesture::kNextResult:
      if (event.key == Key::kDown) {
        return ControlCommand::kNextResult;
      }
      break;
    case Gesture::kNone:
      break;
  }
  return Withheld{WithheldReason::kGestureMismatch};
}

}  // namespace arcified::input
