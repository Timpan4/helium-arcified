#include "command_input.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

namespace {

using namespace arcified::input;
using namespace std::literals;

void Check(bool condition, const char* requirement) {
  if (!condition) {
    std::cerr << "FAIL: " << requirement << '\n';
    std::exit(EXIT_FAILURE);
  }
}

NativeInput IdleNativeInput() {
  NativeInput native;
  native.dispatch = Dispatch::kPostImeUnhandled;
  native.focus = Focus::kSameFocusedClient;
  native.before = {Activity::kInactive, Activity::kInactive,
                   Activity::kInactive};
  native.after = native.before;
  return native;
}

KeyEvent Press(Key key) {
  KeyEvent event;
  event.kind = EventKind::kPress;
  event.key = key;
  return event;
}

void CheckWithheld(const Decision& decision, WithheldReason reason) {
  const auto* withheld = std::get_if<Withheld>(&decision);
  Check(withheld && withheld->reason == reason,
        "withhold the command with the specified reason");
}

void CheckAccepts(std::u16string_view text) {
  const auto decision = ArbitrateInput(Gesture::kAccept, Press(Key::kEnter),
                                       IdleNativeInput(), text);
  const auto* accepted = std::get_if<AcceptedText>(&decision);
  Check(accepted && accepted->text == text,
        "accept exact logical UTF-16 text without normalization or clipping");
  Check(accepted->text.data() == text.data(), "borrow the native text buffer");
}

void GestureOptIn() {
  const auto native = IdleNativeInput();
  const struct {
    Gesture gesture;
    Key key;
    ControlCommand command;
  } controls[] = {
      {Gesture::kDismiss, Key::kEscape, ControlCommand::kDismiss},
      {Gesture::kPreviousResult, Key::kUp, ControlCommand::kPreviousResult},
      {Gesture::kNextResult, Key::kDown, ControlCommand::kNextResult},
  };
  for (const auto& control : controls) {
    const auto decision =
        ArbitrateInput(control.gesture, Press(control.key), native, u"label");
    const auto* command = std::get_if<ControlCommand>(&decision);
    Check(command && *command == control.command,
          "allow an opted-in unmodified control gesture");
  }
  CheckAccepts(u"");  // Empty-input semantics belong to the receiving control.
  for (Key key : {Key::kEnter, Key::kEscape, Key::kUp, Key::kDown}) {
    CheckWithheld(ArbitrateInput(Gesture::kNone, Press(key), native, u""),
                  WithheldReason::kNoGesture);
  }
  CheckWithheld(ArbitrateInput(Gesture::kAccept, Press(Key::kUp), native, u""),
                WithheldReason::kGestureMismatch);
  CheckWithheld(
      ArbitrateInput(Gesture::kDismiss, Press(Key::kEnter), native, u""),
      WithheldReason::kGestureMismatch);
  CheckWithheld(
      ArbitrateInput(Gesture::kPreviousResult, Press(Key::kDown), native, u""),
      WithheldReason::kGestureMismatch);
  CheckWithheld(
      ArbitrateInput(Gesture::kNextResult, Press(Key::kUp), native, u""),
      WithheldReason::kGestureMismatch);
}

void NativeActivityWins() {
  // Exercise each kind of activity before and after this event. In particular,
  // active-before/idle-after cannot turn composition-committing Enter into
  // command acceptance, nor candidate-closing Escape into control dismissal.
  for (auto snapshot : {&NativeInput::before, &NativeInput::after}) {
    for (auto field :
         {&TextActivity::composition, &TextActivity::candidate_popup,
          &TextActivity::pending_text_input}) {
      for (Activity state : {Activity::kActive, Activity::kUnknown}) {
        auto native = IdleNativeInput();
        (native.*snapshot).*field = state;
        for (const auto& gesture :
             {std::pair{Gesture::kAccept, Key::kEnter},
              std::pair{Gesture::kDismiss, Key::kEscape},
              std::pair{Gesture::kPreviousResult, Key::kUp},
              std::pair{Gesture::kNextResult, Key::kDown}}) {
          CheckWithheld(
              ArbitrateInput(gesture.first, Press(gesture.second), native, u""),
              state == Activity::kActive
                  ? WithheldReason::kNativeInputBusy
                  : WithheldReason::kNativeInputUnverified);
        }
      }
    }
  }
  CheckWithheld(ArbitrateInput(Gesture::kAccept, Press(Key::kEnter), {}, u""),
                WithheldReason::kNativeInputUnverified);
  for (Dispatch dispatch :
       {Dispatch::kUnverified, Dispatch::kBeforeIme, Dispatch::kHandled}) {
    auto native = IdleNativeInput();
    native.dispatch = dispatch;
    CheckWithheld(
        ArbitrateInput(Gesture::kAccept, Press(Key::kEnter), native, u""),
        dispatch == Dispatch::kHandled
            ? WithheldReason::kNativeInputBusy
            : WithheldReason::kNativeInputUnverified);
  }
  // A later, separately verified event can submit the committed text. There is
  // no sticky composition latch or retained copy of the preedit string.
  CheckAccepts(u"\u65e5\u672c\u8a9e");
}

void FocusMustRemainAuthoritative() {
  for (const auto& focus :
       {std::pair{Focus::kUnverified, WithheldReason::kNativeInputUnverified},
        std::pair{Focus::kNoFocusedClient, WithheldReason::kNoFocus},
        std::pair{Focus::kChangedDuringDispatch,
                  WithheldReason::kStaleFocus}}) {
    auto native = IdleNativeInput();
    native.focus = focus.first;
    CheckWithheld(
        ArbitrateInput(Gesture::kAccept, Press(Key::kEnter), native, u""),
        focus.second);
  }
}

void PreserveNativeEditing() {
  const auto native = IdleNativeInput();
  for (EventKind kind : {EventKind::kOther, EventKind::kRelease,
                         EventKind::kCharacter, EventKind::kTextEdit}) {
    auto event = Press(Key::kEnter);
    event.kind = kind;
    CheckWithheld(ArbitrateInput(Gesture::kAccept, event, native, u"clipboard"),
                  WithheldReason::kNotCommandKey);
  }
  for (Key key : {Key::kOther, Key::kTextSystem}) {
    CheckWithheld(ArbitrateInput(Gesture::kAccept, Press(key), native, u""),
                  WithheldReason::kNotCommandKey);
  }
  for (auto modifier : {&Modifiers::shift, &Modifiers::control, &Modifiers::alt,
                        &Modifiers::meta, &Modifiers::alt_graph}) {
    auto event = Press(Key::kEnter);
    event.modifiers.*modifier = true;
    CheckWithheld(ArbitrateInput(Gesture::kAccept, event, native, u""),
                  WithheldReason::kModifiedOrRepeatedKey);
  }
  auto event = Press(Key::kEnter);
  event.repeat = true;
  CheckWithheld(ArbitrateInput(Gesture::kAccept, event, native, u""),
                WithheldReason::kModifiedOrRepeatedKey);
}

void UnicodeRemainsLogicalAndUnchanged() {
  for (auto text :
       {u" \tname\n "sv, u"\u00e9"sv, u"e\u0301"sv, u"\u65e5\u672c\u8a9e"sv,
        u"\u05e9\u05dc\u05d5\u05dd English \u0645\u0631\u062d\u0628\u0627"sv,
        u"\u2067RTL\u2069 \u202eabc\u202c"sv,
        u"\U0001f469\u200d\U0001f4bb \U0001f1f8\U0001f1ea"sv, u"a\0b"sv,
        u"\uffff"sv}) {
    CheckAccepts(text);
  }
  // Synthetic long input, not a product capacity or truncation threshold.
  CheckAccepts(std::u16string(16384, u'\u05d0'));
  CheckAccepts(std::u16string{0xD800, 0xDC00, 0xDBFF, 0xDFFF});
  for (const auto& malformed :
       {std::u16string{0xD800}, std::u16string{0xDC00},
        std::u16string{0xDC00, 0xD800}, std::u16string{0xD800, u'x'},
        std::u16string{0xD800, 0xD800, 0xDC00}, std::u16string{u'x', 0xDBFF}}) {
    CheckWithheld(ArbitrateInput(Gesture::kAccept, Press(Key::kEnter),
                                 IdleNativeInput(), malformed),
                  WithheldReason::kInvalidUtf16);
    const auto dismiss = ArbitrateInput(Gesture::kDismiss, Press(Key::kEscape),
                                        IdleNativeInput(), malformed);
    Check(std::get_if<ControlCommand>(&dismiss) &&
              std::get<ControlCommand>(dismiss) == ControlCommand::kDismiss,
          "malformed text cannot prevent a separately eligible dismissal");
  }
}

void InvalidNativeFactsCannotAuthorizeCommands() {
  const auto event = Press(Key::kEnter);
  const auto native = IdleNativeInput();
  CheckWithheld(ArbitrateInput(static_cast<Gesture>(-1), event, native, u""),
                WithheldReason::kInvalidFacts);
  auto invalid_event = event;
  invalid_event.kind = static_cast<EventKind>(-1);
  CheckWithheld(ArbitrateInput(Gesture::kAccept, invalid_event, native, u""),
                WithheldReason::kInvalidFacts);
  invalid_event = event;
  invalid_event.key = static_cast<Key>(-1);
  CheckWithheld(ArbitrateInput(Gesture::kAccept, invalid_event, native, u""),
                WithheldReason::kInvalidFacts);
  auto invalid_native = native;
  invalid_native.dispatch = static_cast<Dispatch>(-1);
  CheckWithheld(ArbitrateInput(Gesture::kAccept, event, invalid_native, u""),
                WithheldReason::kInvalidFacts);
  invalid_native = native;
  invalid_native.focus = static_cast<Focus>(-1);
  CheckWithheld(ArbitrateInput(Gesture::kAccept, event, invalid_native, u""),
                WithheldReason::kInvalidFacts);
  for (auto snapshot : {&NativeInput::before, &NativeInput::after}) {
    for (auto field :
         {&TextActivity::composition, &TextActivity::candidate_popup,
          &TextActivity::pending_text_input}) {
      invalid_native = native;
      (invalid_native.*snapshot).*field = static_cast<Activity>(-1);
      CheckWithheld(
          ArbitrateInput(Gesture::kAccept, event, invalid_native, u""),
          WithheldReason::kInvalidFacts);
    }
  }
}

}  // namespace

int main() {
  GestureOptIn();
  NativeActivityWins();
  FocusMustRemainAuthoritative();
  PreserveNativeEditing();
  UnicodeRemainsLogicalAndUnchanged();
  InvalidNativeFactsCannotAuthorizeCommands();
  std::cout << "PASS: command input arbitration and borrowed Unicode text\n";
}
