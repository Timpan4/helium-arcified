# Native command input boundary

Issue [113](https://github.com/Timpan4/helium-arcified/issues/113). This directory implements standalone C++17 arbitration for command-bar input, renaming and settings. It supplies executable decisions over native observations, with borrowed Unicode text. It is not connected to a browser widget. Installed-build acceptance on Linux, Windows and macOS remains open.

## Interface and ownership

`ArbitrateInput(requested, event, native, text)` returns one of:

- `Withheld`, with a typed reason. This grants no browser command and does not consume or replay an event. Native processing continues according to its existing dispatch state. An already-handled event stays handled.
- `AcceptedText`, with the exact borrowed UTF-16 input. Only an opted-in, unmodified, nonrepeated Enter can produce it.
- `ControlCommand`, for an opted-in Escape dismissal or Up/Down result movement. These results contain no text, selected result, focus target or browser command ID.

The native caller supplies the semantic key after layout resolution, modifiers, event kind, dispatch disposition, focused-client continuity and text-system activity before and after this event. All three activity fields must be explicitly inactive in both observations. Active composition, candidate popups or pending input withhold commands, including Enter that finishes composition or Escape that closes candidates. Unknown facts also withhold commands. Default-constructed inputs are unverified and cannot grant a command.

Every enum is checked at this boundary. Out-of-domain values return `kInvalidFacts`. A changed focused client returns `kStaleFocus` when no already-handled/busy fact has withheld the event first. Missing focus and unverified dispatch/focus have distinct outcomes. Reason priority follows the checks in the implementation; a reason reports a sufficient refusal, not a complete diagnosis. There are no clocks, expiry intervals or event counters here.

The caller must attest that both observations concern this exact event and the same still-focused client. A context switch, client replacement, queued callback or uncertain event association makes that attestation invalid. Supply changed/unverified facts instead of reusing an earlier observation. The component cannot detect a caller that labels stale facts current. It cannot validate the lifetime or memory safety of a `string_view`; those are caller obligations. Revalidate through the host if execution is deferred. Never cache a successful decision as permission for another event or input target.

Only the authoritative native control owns preedit, candidate handling, text storage, cursor/selection, undo, clipboard and focus. Dead/Compose/Process keys and native Unicode-entry sequences must map to `Key::kTextSystem` even if a legacy virtual key looks like Enter or an arrow. Native text editing commands and clipboard operations map to `EventKind::kTextEdit`, not a command press. Character and release events, all modifiers including AltGr, unrelated keys and repeats cannot produce commands. This deliberately narrow local gesture set does not redefine browser accelerators. A control that needs native multiline/caret behavior supplies `Gesture::kNone`; it must not opt Up/Down into result movement by default.

`AcceptedText` borrows the final native buffer after dispatch. It preserves logical UTF-16 order, whitespace, combining sequences, supplementary characters, bidi controls, embedded NULs and long input. It rejects unpaired surrogates on acceptance without repairing or deleting text. It does not normalize, trim, case-fold, reorder, split graphemes or impose a length limit. Empty text is valid here; the receiving operation decides its meaning. Malformed text does not block an otherwise eligible dismissal. An asynchronous receiver must copy accepted text before the native buffer changes or dies. URL validation, action availability, identity resolution and navigation authority remain with their existing owners.

No user-facing strings are embedded in production code. Callers may map typed reasons to localized resource strings if useful. Native text direction, shaping, grapheme navigation, caret bounds, selection and rendering remain native responsibilities. This slice preserves their input; it does not prove translated-label layout, RTL presentation, action visibility or logical focus order in a widget.

The [lifecycle v3 contract](../../../docs/lifecycle-contract.md) remains authoritative. No ownership, persistence, navigation, selected-result or focus registry is stored here. [Native Space integration](../../../docs/native-space-integration.md) and its [source references](../../../docs/native-space-source-references.md) document the host seam; that pinned design predates the v3 policy approvals. Actual focus routing/application and accessibility announcements belong to the native host. Command-bar integration belongs to the navigation workstream. No shared interface, patch series, build tooling or lifecycle file is changed by this slice.

## Source evidence

Inspected through read-only Gitiles HTTP at Chromium commit `a49e2e3d4addfd84e765b52afe230bedb81096db`, the repository's `155.0.8059.79` pin. Source inspection establishes the reuse constraints below, not native runtime behavior.

| Source | Observation and consequence |
| --- | --- |
| [Textfield](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/views/controls/textfield/textfield.cc#825), lines 825 to 869; [controller](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/views/controls/textfield/textfield_controller.h#41), lines 41 to 44 | Controller handling can precede scheduled/Linux native editing commands. Returning true stops further native processing. A new controller must withhold browser commands for text-system events and preserve the existing native path. A controller callback alone does not prove the full context required here. |
| [Linux input method](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/base/ime/linux/input_method_auralinux.cc#65), lines 65 to 103, 135 to 229, 232 to 260, 448 to 450 | Dead-key and IME processing can be asynchronous. Even a reportedly unfiltered event can update commit/preedit. Filtered Enter uses a Process-key event. The candidate-popup query returns false because popup detection is unavailable. Pending input and before/after activity cannot be inferred from empty preedit or that false return. |
| [macOS input method](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/base/ime/mac/input_method_mac.mm#16), lines 16 to 41 | Dispatch is post-IME. Candidate detection is unavailable, and AppKit can retain a conversion session after composition is cleared. Post-IME delivery and no marked text do not by themselves prove inactivity. |
| [Windows TSF input method](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/base/ime/win/input_method_win_tsf.cc#14), lines 14 to 32, 163 to 164 | Candidate-window count observations supply positive popup knowledge. This is not a universal cross-platform proof that no text service owns a key. |
| [Native key event](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/events/event.h#891), lines 891 to 935; [DomKey](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/events/keycodes/dom/dom_key.h#105), lines 105 to 113 | Character events, repeat, Unicode alt-code entry, physical code and logical key are distinct. Dead and Compose keys have explicit semantics. The adapter must retain these distinctions. |
| [Omnibox](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/omnibox/omnibox_view_views.cc#2052), lines 2052 to 2083, 1151 to 1155, 1279 to 1301 | Native omnibox handling excludes Unicode alt-code entry, recognizes AltGr and avoids replacing composing text with inline completion. It observes composition around changes. Reuse native editing rather than implementing another editor. |
| [TextInputClient](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/base/ime/text_input_client.h#130), lines 130 to 138, 233 to 248; [Textfield geometry](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/views/controls/textfield/textfield.cc#1840), lines 1840 to 1859, 1883 to 1907 | Native text operations use UTF-16 ranges. Native direction, screen-space caret bounds and grapheme-aware composition bounds support the text system. This boundary neither recomputes nor replaces them. |
| [RFC 2781 section 2.2](https://www.rfc-editor.org/rfc/rfc2781#section-2.2) | The acceptance validator uses the specified high/low surrogate pairing rules. These code-point bounds are encoding requirements, not product limits. |

## Focused checks

Run from the repository root with the existing Clang toolchain. The driver-mode option follows the [documented host linker requirement](../../../docs/lifecycle-contract.md#synthetic-transition-fixtures-and-focused-checks).

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/input/command_input.h \
  arcified/native/features/input/command_input.cc \
  arcified/native/features/input/command_input_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/input/command_input.cc \
  arcified/native/features/input/command_input_test.cc \
  -o /tmp/arcified-command-input-test
/tmp/arcified-command-input-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/features/input/command_input.cc \
  arcified/native/features/input/command_input_test.cc \
  -o /tmp/arcified-command-input-sanitized
/tmp/arcified-command-input-sanitized
```

Fixtures check opted-in gestures, before/after activity in every field, unknown/handled/pre-IME dispatch, stale/missing focus, characters/releases/edit events, native text-system keys, modifiers/AltGr, repeats, malformed enum facts and UTF-16. Unicode fixtures cover composed/decomposed accents, Japanese, Hebrew/Arabic mixed with English, bidi controls, emoji/ZWJ, embedded NULs, empty input, a synthetic long label and surrogate boundaries. The long fixture length is test data, not an accepted-input capacity. The fixtures check exact returned code units and borrowed storage. No services, profiles, real browsing data or AI providers are accessed.

Executed in the shared Linux checkout at checkpoint `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`: formatting and warning-clean compilation passed, and both ordinary and AddressSanitizer/UndefinedBehaviorSanitizer executions printed `PASS: command input arbitration and borrowed Unicode text`. `git diff --no-index --check /dev/null` against each of this directory's four new files emitted no whitespace diagnostics; status 1 represents new-file differences. Local documentation link targets were checked with filesystem metadata. No commit was made. These results cover the standalone component only.

## Native gaps and acceptance still open

The adapter must establish authoritative per-event activity and focus continuity without inventing a focus registry or assuming that an unavailable candidate query proves inactivity. Mapping unavailable candidate/pending information to `kUnknown` withholds every requested command. That is a safe refusal in this library, not a usable completed platform integration. A source-backed native route that reliably grants commands while preserving text-service ownership is still required on each OS. No guessed platform callback, timer or heuristic is supplied to fill that gap.

Native widget integration, command-bar dependency, accelerator precedence, real IME/candidate windows, asynchronous dead-key/Compose handling, clipboard and paste policy, AppKit conversion sessions, TSF, Linux IBus/Fcitx, Unicode entry and focus changes across Space/Profile presentation remain unverified. Installed-build tests must also cover localized resource strings, RTL/mixed-script rendering, long translated labels, action visibility, logical focus order and accessibility behavior. No native browser was built or launched, and no visual verification is claimed. Issue 113 must remain open.
