# Shortcut settings conflict previews

Status: reusable native conflict collector and an **unapplied** service patch for
[issue 34](https://github.com/Timpan4/helium-arcified/issues/34). Native integration
and acceptance remain open on Linux, Windows and macOS. This slice adds no UI,
binding store, command catalog, persistence or focus routing.

## Existing behavior and source identity

The [publication brief](../research/arc-dia/publication/shortcut-settings.md) asks
for action search, recording/reset, collision explanations, platform labels and
restart/context behavior. Helium already implements much of the source surface:

| Source inspected | Existing behavior |
| --- | --- |
| [Core service patch](../../../patches/helium/core/custom-keyboard-shortcuts.patch) | `BrowserShortcutService` reads native defaults, parses through `AcceleratorFromString`, compares `ui::Accelerator`, persists Profile-pref deltas, rebuilds command metadata and notifies observers. Assignment removes other commands' modifiable matches. Reset restores the target's defaults and removes other commands' modifiable matches. |
| [Settings patch](../../../patches/helium/settings/custom-keyboard-shortcuts-page.patch) | `/system/shortcuts` searches localized names and shortcut labels, records DOM key codes, explains visible browser collisions before Save, removes individual bindings and resets commands/all commands. Capture conflict matching uses serialized-string equality. Native assignment remains authoritative. |
| [BrowserView wiring](../../../patches/helium/ui/custom-keyboard-shortcuts-wiring.patch) | Caches the Profile service, reloads native accelerator registrations on changes and updates active macOS menu accelerators. |
| [Layout shortcuts](../../../patches/helium/ui/layout/shortcuts.patch) and [keyboard shortcuts](../../../patches/helium/core/keyboard-shortcuts.patch) | Existing native command IDs, command-controller dispatch, platform menu/accelerator integration and reserved-command handling. These remain unchanged. |

The checkpoint is `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. The engine pin is
`155.0.8059.79`, Chromium commit `a49e2e3d4addfd84e765b52afe230bedb81096db`.
The core patch Git blob is `65b9d7b7f94858720b2311958fcdbd3ab086deeb`.
The validator refuses a different core patch or engine version. Searching all
local patch headers found no later patch to either service file. This does not
prove that the complete patch series applies to Chromium.

Pinned [accelerator.h](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/base/accelerators/accelerator.h)
was read directly. Native equality includes key code, key state, masked modifiers
and interruption state. Serialization spellings are therefore unsuitable as the
authority for conflicts. The existing native parser and display formatter remain
the source of truth. Pinned [focus_manager.h](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/views/focus/focus_manager.h)
was also read. It documents focused-view handling and native accelerator
priorities. This slice changes neither.

## Component and service interface

[conflict_preview.h](../../native/features/shortcut_settings/conflict_preview.h)
borrows the service's current command map and proposed `AcceleratorEntry` range.
Its template avoids a Chromium dependency for the portable algorithm fixtures;
the native patch instantiates it with the existing `ui::Accelerator` entries.
It delegates equality to that type. It returns command IDs sorted and deduplicated,
excludes the target itself, and retains a locked indication when any matching
binding for a command is unmodifiable. Reset considers only modifiable matches,
exactly as `RemoveConflictingAccelerators` does. Proposed-entry mutability does
not decide which existing binding can be removed.

The [patch artifact](../../native/features/shortcut_settings/native-conflict-preview.patch)
installs that header and adds these const methods to `BrowserShortcutService`:

```cpp
AcceleratorChangePreview PreviewAcceleratorAssignment(
    int command_id, const std::string& serialized) const;
AcceleratorChangePreview PreviewAcceleratorReset(int command_id) const;
```

`AcceleratorChangePreview` contains the existing `AssignmentResult` and
`std::vector<ShortcutConflict>`. Each conflict contains `command_id` and `locked`.

1. Assignment checks unknown command before invalid accelerator, using the same
   `EntryFromString` as mutation. It collects all native-equal matches. On macOS
   it also includes locked platform defaults absent from visible command metadata.
   Existing `IsAcceleratorLocked` determines refusal. A locked result means no
   command would change, including modifiable matches listed alongside the lock.
2. Successful assignment reports the other commands whose matching modifiable
   bindings would be removed. It does not mean those commands lose every binding.
   Self-assignment produces no conflict, including a successful no-op.
3. Reset checks the actual `defaults_` map and reports the other commands whose
   modifiable bindings would be removed. Reset does not use assignment's lock
   refusal. It also replaces the target's own custom bindings with its defaults;
   those target changes are not cross-command conflicts. A known command with no
   defaults resets to no bindings. An unknown command reports `kUnknownCommand`.
4. Previews read the actual service state and change no prefs, cache or observers.
   They are snapshots, not reservations. Refresh and review conflicts after any
   intervening service change, then call the existing mutation on the same Profile
   service. Mutation still validates its own input. This patch adds no atomic
   confirmation token or stale-preview rejection protocol.

The existing assignment, reset, persistence and notification methods remain
byte-identical. No second lifecycle, navigation or selected-context state is
introduced. Contract v3's Profile/Space mapping and saved-page behavior remain
unchanged. Shortcuts retain the existing Profile-service scope. They do not become
Space-scoped themes. Same-window presentation across Profiles still depends on
the unproven production host extraction in the [native integration design](../native-space-integration.md).

## Collision and accessibility limits

The service can explain browser bindings it owns, plus its known macOS platform
reservations. A platform reservation is a browser-enforced lock, not proof of a
conflict with the user's OS configuration. No API here enumerates arbitrary OS,
window-manager, accessibility-tool, extension or webpage bindings. A successful
preview must not be labeled "no system conflicts" or "works on every page".
Webpages and focused text fields may consume keys. OS interception can prevent an
event from reaching the browser at all. Native runtime checks are required.

Helium's existing formatter supplies localized/platform modifier text. This
artifact does not infer keyboard layout or promise physical-key fidelity beyond
the native parser/accelerator dispatch contract. It adds no recorder interception,
accelerator priority, focus traversal or split-pane routing. Accessibility owns
focus/routing integration and accessible non-shortcut routes to core actions.
The frontend and Mojo handler do not call these previews yet. Search, capture,
reset confirmation and announcement improvements require a subsequent UI slice.

## Checks and remaining acceptance

Run from the repository root using the installed tools:

```sh
node --check arcified/native/features/shortcut_settings/validate_patch.mjs
node arcified/native/features/shortcut_settings/validate_patch.mjs
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/shortcut_settings/conflict_preview.h \
  arcified/native/features/shortcut_settings/conflict_preview_test.cc \
  arcified/native/features/shortcut_settings/browser_shortcut_service_preview_unittest.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/shortcut_settings/conflict_preview_test.cc \
  -o /tmp/arcified-shortcut-conflict-test
/tmp/arcified-shortcut-conflict-test
git diff --check -- arcified/native/features/shortcut_settings \
  arcified/docs/features/shortcut-settings.md
```

The portable fixtures passed on this Linux host. They exercise the actual
collector with synthetic equality tokens: multiple matching defaults, duplicate
commands, mixed locked/modifiable matches, self exclusion, empty proposals,
reset filtering and merging platform reservations. They do not execute Chromium's
parser, service or accelerator dispatch. The validator passed forward/reverse
application against only the two complete service files reconstructed from the
pinned Helium patch. It also checks the embedded header/gtest copies against the
owned files and verifies that original service mutations remain unchanged.
Formatting and Node syntax checks passed. No Markdown or standalone JavaScript
formatter/linter is declared for these files; Node syntax checking is not linting.

[Native gtests](../../native/features/shortcut_settings/browser_shortcut_service_preview_unittest.cc)
are authored for real `TestingProfile`/service instances: normalized native
equality, validation order, self-assignment, read-only prefs/bindings/observer
behavior, reset displacement, resetting a command without defaults, stale
previews and macOS locked reservations. They have **not been compiled or run**.
The reset displacement fixture chooses a command with nonempty defaults that
the actual service reports as modifiable. It does not assume New Tab can be
reassigned: without a live macOS main menu, the pinned platform fallback locks
Cocoa defaults. This dependency was found in independent source review and
removed from both the owned test and its embedded patch copy.
The patch installs their source but does not register them in GN. A later approved
integration must register the test source in the native unit-test target and the
helper header in the UI source list, apply the artifact after Helium's core
shortcut patch, and compile/run `BrowserShortcutPreviewTest.*` on each platform.
No series, build, workflow or shared tooling file is changed here.

All issue acceptance criteria remain open. Installed native evidence is still
needed for UI search/record/reset, browser/OS/webpage collision explanations,
platform modifiers, accessible action routes, restart persistence, dialogs,
split panes and text-field precedence on Linux, Windows and macOS. No native
application was launched, so no affected interaction or visual check is claimed.
No full Chromium preparation/build, browsing data, provider access or remote
write was used.
