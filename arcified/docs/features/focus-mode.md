# Focus mode presentation

Status: reusable standalone C++ presentation component for [issue 82](https://github.com/Timpan4/helium-arcified/issues/82). Installed native acceptance remains **OPEN** on Linux, Windows and macOS. The [publication brief](../research/arc-dia/publication/focus-mode.md) and `focus-mode` record in [backlog.json](../research/arc-dia/backlog.json) define the requested outcome. This slice implements reversible chrome suppression and intentional reveal semantics. It does not install a native command, menu, accelerator or accessible exit control.

[focus_mode.h](../../native/features/layout/focus_mode/focus_mode.h) and [focus_mode.cc](../../native/features/layout/focus_mode/focus_mode.cc) implement `arcified::focus_mode::Presentation`. The component stores only normal/suppressed/pointer-reveal/keyboard-reveal state per host. It has no page, attachment, window, split, focus-target or fullscreen identity. It retains no preferences, geometry, focus history, dialog facts or fullscreen facts. It creates no navigation, selection, ownership, persistence or focus registry.

## API and focus semantics

The host serializes `Dispatch(Action, HostFacts)` on its UI sequence. `HostFacts` supplies the **current** focus location, dialog presence, and fullscreen overlay eligibility. The host classifies intentional pointer activation using its actual window-local hit regions and input policy. No pointer distance, dwell time, screen edge, display size or shortcut binding is invented here. `Project(current_facts)` reads presentation without emitting focus effects. Fact changes must also be reported through `kHostChanged` to cancel stale modal/fullscreen reveal gestures.

| Action | Presentation and intent |
| --- | --- |
| Enter, including normalized Toggle | Activate suppression. Preserve content focus. If chrome still has actual focus, keep it overlaid and emit `kActiveContent`. Repeated Enter does nothing. |
| Intentional pointer reveal | Reveal overlaid chrome, preserve focus. Pointer leave suppresses again unless chrome still contains focus or an explicit keyboard reveal is latched. Ordinary pointer movement is not a reveal command. |
| Keyboard reveal | Reveal overlaid controls and emit `kControls` when focus is inside this host but outside chrome. Pointer events cannot cancel keyboard reveal. |
| Dismiss reveal | Clear transient reveal and emit `kReturnOrFallback` if chrome contains focus. Keep chrome overlaid until the host reports focus outside it. A failed, delayed or cancelled native focus request never counts as successful focus movement. |
| Exit, including normalized Toggle | Release only this component's override. If appropriate, emit `kReturnOrFallback`; never set fullscreen, bounds, selection, page state or preferences. Repeated Exit does nothing. |
| Escape | Defer to any open dialog or native fullscreen, leaving mode active. Otherwise exit focus mode. The host decides accelerator precedence before dispatch, including any higher-priority menu or page interaction. |
| Host fact change | Recompute from current facts. A dialog or exclusive fullscreen cancels transient reveal without taking native focus. Chrome with actual focus stays overlaid. |

`Result` contains the current projection, one semantic `FocusIntent`, and handled/ignored/defer-to-host disposition. A result is an intent, not a native completion acknowledgement. The authoritative host/accessibility path resolves existing return targets, checks liveness and eligibility for the resulting presentation, and falls back to the current active page or accessible empty-host surface. A return target that would immediately be hidden is ineligible. There is no saved target in this component, and callers must not add a competing feature-local snapshot or resolver. Focus outside this host never receives a focus intent.

## Layout composition

`Projection::chrome` uses the sidebar-owned `arcified::sidebar::PresentationOverride` directly:

| Override | Meaning |
| --- | --- |
| `kNone` | Focus mode is inactive. Recompute normal presentation from current preferences and host measurements. |
| `kSuppress` | Hide focus-mode-controlled browser chrome while keeping authoritative content bounds. |
| `kRevealOverlay` | Intentionally reveal that chrome over content, with the same content bounds as suppression. Never turn reveal into docked layout. |

For the sidebar, compose `focus.Project(host_facts).chrome` through `tab_presentation::Presentation::sidebar_override(chrome)` before setting `sidebar::Context::presentation_override`. Effective horizontal tabs continue suppressing the vertical sidebar; vertical tabs forward the focus-mode override. Apply the chrome override separately to the effective tabstrip and toolbar. The tab presentation component owns that composition, not another focus-mode orientation preference.

Pass the resulting current sidebar content bounds to split geometry. Focus mode stores no split metadata or page references. Entry/exit may change the content area as chrome is suppressed or normal docking resumes; reveal/dismiss and modal transitions must not move the page. Sidebar width/visibility edits remain sidebar preferences, even while suppressed. Bounds, display scale, side, fullscreen and available space remain current host inputs. Releasing the override never restores old values. Motion may consume the projection for rendering, but semantic presentation commits immediately and motion never owns visibility or layout state.

Dialogs retain native focus and Escape priority without releasing suppression to `kNone`. This prevents a normally visible sidebar from docking and moving content when a dialog opens. Exclusive fullscreen likewise never restores docked preferences. It suppresses transient browser reveal while the host retains authority over fullscreen-native controls and may restrict browser overlays. The projection does not command OS fullscreen controls, hide a dialog, or assert that a browser overlay can be drawn in exclusive fullscreen. Explicit Exit always releases focus mode independently of fullscreen. The native adapter must supply an accessible Exit command outside suppressed chrome, and keep fullscreen/dialog escape routes usable.

The [lifecycle v3 contract](../lifecycle-contract.md) remains the ownership/navigation authority. Same saved-page transfer, local temporary pages, Profile isolation and same-OS-window cross-Profile switching receive no new implementation here. [Native Space integration](../native-space-integration.md) and its [pinned references](../native-space-source-references.md) identify the existing BrowserView/Profile/widget coupling and required extraction. Eventual menu/accelerator wiring must use the existing native command infrastructure described there, after the host construction/lifetime path is proven. Current native page command resolution remains the shared commands component, not a focus-mode resolver.

## Focused checks

[focus_mode_test.cc](../../native/features/layout/focus_mode/focus_mode_test.cc) drives public reducer commands and composes the real [sidebar component](../../native/features/layout/sidebar/sidebar.h). Synthetic measurements are fixture inputs, not platform defaults or policy limits. The checks cover command normalization, pointer focus preservation, keyboard reveal, failed/delayed return, modal/fullscreen Escape priority, independent hosts, content bounds through reveal/dialog/fullscreen, and exit using changed bounds, side and preferences. No browsing data, page fetches or providers are used.

Run from the repository root with the existing C++17 compiler:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/layout/focus_mode/focus_mode.h \
  arcified/native/features/layout/focus_mode/focus_mode.cc \
  arcified/native/features/layout/focus_mode/focus_mode_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/layout/sidebar/sidebar.cc \
  arcified/native/features/layout/focus_mode/focus_mode.cc \
  arcified/native/features/layout/focus_mode/focus_mode_test.cc \
  -o /tmp/arcified-focus-mode-test
/tmp/arcified-focus-mode-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/features/layout/sidebar/sidebar.cc \
  arcified/native/features/layout/focus_mode/focus_mode.cc \
  arcified/native/features/layout/focus_mode/focus_mode_test.cc \
  -o /tmp/arcified-focus-mode-sanitized
/tmp/arcified-focus-mode-sanitized
```

Local Linux result: Chromium formatting check passed. All eleven fixtures passed with strict C++17 warnings and again with AddressSanitizer/UndefinedBehaviorSanitizer, with no sanitizer diagnostics. This is standalone component evidence, not a Chromium build or installed-browser result.

## Native gaps and acceptance

All installed-native criteria remain open. These standalone checks prove presentation and composition only. They do not prove visible controls, real focus movement, native accessibility tree/announcements, menu and accelerator reachability, dialog/fullscreen integration, split interaction, monitor/DPI behavior or failure recovery in an installed browser. No suitable installed native preview is part of this slice, so visual interaction was not checked. Windows/macOS compilation and runtime checks remain open as well.

The adapter must revalidate authoritative focus/page identity at actuation, preserve native dialog/fullscreen priority, apply focus/capture effects safely, and keep an accessible exit reachable while chrome is hidden. Test actual keyboard/menu entry and exit, removed/stale return targets, empty hosts, focused controls on dismissal, split-pane focus, content and browser fullscreen, native dialogs, monitor/scale changes, and current-preference restoration on each platform. Prepared Chromium sources, builds, downloads, benchmarks and CI remain deferred. No patch series, shared lifecycle/tooling, execution catalog or workflow changes are included.
