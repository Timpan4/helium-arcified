# Sidebar layout

Status: reusable standalone C++ presentation component for [issue 51](https://github.com/Timpan4/helium-arcified/issues/51). Native installed acceptance remains **OPEN** on Linux, Windows and macOS. No Chromium adapter, prepared tree, browser build, download, benchmark or CI run is included.

The [publication brief](../research/arc-dia/publication/sidebar-layout.md) and `sidebar-layout` record in [backlog.json](../research/arc-dia/backlog.json) require persistent visibility, accessible temporary reveal and resizing while retaining page state. [sidebar.h](../../native/features/layout/sidebar/sidebar.h), [sidebar.cc](../../native/features/layout/sidebar/sidebar.cc) and [the executable fixtures](../../native/features/layout/sidebar/sidebar_test.cc) implement presentation and interaction semantics. They do not prove native page-state preservation.

## Interface and ownership

`arcified::sidebar::Presentation::Create(Preferences, Context)` returns an optional component. It refuses nonfinite geometry, negative bounds, nonpositive sidebar minima/maxima or hit widths, reversed width limits and nonpositive preferred widths. Empty available bounds are valid but cannot present a sidebar. Public commands take owned inputs and return `Effect`; `layout()` and `preferences()` return values. The caller serializes operations on its UI sequence.

`Preferences` contains only `persistent_visible` and `preferred_width`. A persistent toggle edits visibility explicitly; resize commit edits width only when the visible draft changed. `preferences_changed` means the caller may persist these values, not that a write succeeded. Temporary reveal, constraints, cancellation and mode overrides never overwrite the durable width or visibility. This is no new persistence engine.

The sidebar header owns `Side::{kLeft,kRight}` and `PresentationOverride::{kNone,kSuppress,kRevealOverlay}` for layout composition. Tab-layout choice consumes `Side`; it does not define a second side type. Focus mode owns its override. Horizontal tab-layout suppression takes precedence over generic focus reveal in the caller's composition; vertical layout passes the focus override through. The sidebar never changes that override or owns fullscreen state.

`Context` supplies the available rectangle, side, sidebar minimum/maximum, minimum content width, reveal/resize hit widths and presentation override. All distances and pointer coordinates use the same caller-supplied logical units. Host adapters derive them from actual native bounds and platform/input metrics, including display scaling, fullscreen, dialogs and caption exclusions. No product size or scale factor is supplied here. Fixture numbers are synthetic inputs, not defaults or platform recommendations.

The component has no lifecycle-model reference, page ID, selected-tab state, navigation, native focus handle, focus registry or saved focus snapshot. It cannot detach, recreate, select, reload or migrate a page. Lifecycle version 3 remains authoritative: saved Space pages are shared, temporary pages are local and selecting a live saved page transfers the same identity. Activity, destructive actions and migration choices remain open. Split geometry remains the split component's responsibility using caller-supplied content bounds.

## Layout and narrow-space behavior

`layout()` immediately returns authoritative geometry and a mode. `kDocked` reserves sidebar width in content bounds. `kOverlay` and `kHidden` retain the complete supplied content rectangle. Animation consumes these results; it never feeds interpolated geometry back into the component or delays a semantic transition.

With `kNone`, durable visibility produces docked presentation; otherwise a temporary reveal overlays. `kSuppress` hides the sidebar and provides no reveal/resize target. `kRevealOverlay` overlays regardless of durable visibility and retains exactly the same content bounds as `kSuppress`. Leaving a caller-forced overlay emits a semantic focus request when appropriate but cannot clear the caller-owned override. Focus mode must compose `kSuppress` when its own leave action should hide the overlay.

The maximum usable width is the smaller of `Context.maximum_width` and available width minus the caller's minimum content width. When that cannot meet the caller's sidebar minimum, or available height is zero, `Layout.width_fit` is `kInsufficientSpace`. The result is hidden, content keeps the full available rectangle, and sidebar/reveal/resize rectangles are absent. Keyboard reveal and resize begin return `kUnavailable`. There is no below-minimum sidebar or undersized pointer target. The caller can choose another presentation using its own fallback policy. Preferences survive, so restoring sufficient bounds recovers the original preferred width and persistent visibility.

When the minima fit, width clamps to the usable range without editing preference. Each pointer target retains its complete caller-supplied width and available height. The component shifts a target inward at an edge rather than narrowing it. If a supplied hit width exceeds the available width, that optional target is absent; no smaller target is invented. Keyboard reveal can still work when sidebar minima fit but a pointer metric cannot fit. The native adapter must provide an accessible resize alternative if pointer resizing is unavailable. Target rectangles use half-open hit testing. The adapter owns painted dividers, pointer/touch modality, accessible names, active-row scrolling and actual control sizes.

## Interaction and native effects

1. Route a persistent command through `Dispatch(kTogglePersistent, current_focus)`. It clears temporary reveal and cancels any resize. Hiding a focused sidebar requests `kPageContent`; ordinary pointer reveal requests no focus movement.
2. Route pointer entry across the combined reveal/sidebar interaction region through `kPointerEnter`. Pointer leave closes pointer-only reveal unless a resize is active. If native focus is inside, reveal stays engaged until `kFocusLeft` or explicit leave. Keyboard reveal requests `kSidebar` and stays open across pointer leave. `kFocusLeft` requires a current outside-focus report; no focus state is stored.
3. Route keyboard leave through `kLeave` or `kEscape`. Both can request `kPageContent` without changing page selection. Escape first cancels an active resize, retaining temporary keyboard reveal; a subsequent Escape leaves it. Explicit leave also cancels resizing. A persistent sidebar stays docked when leaving its controls.
4. Start a pointer or touch gesture with `BeginResize(x, y)` against the current target. On `kAcquire`, the host acquires native capture and routes only that captured gesture's events. If capture fails, call `CancelResize()`. `UpdateResize(x)` uses the original pointer/width pair, grows inward from either side, clamps to supplied bounds and changes only the draft. Updates never request focus movement. `CommitResize()` publishes the draft preference and releases capture. A click with no visible width change preserves a preferred width constrained by a small window.
5. On Escape, pointer/touch cancellation or capture failure, cancel the draft. `CancelResize()` restores the prior width and retains reveal. `kCaptureLost` also drops temporary reveal because pointer location may have changed. The host re-evaluates pointer leave after a successful release so capture does not leave a stale hover overlay. `kFocusLeft`, explicit leave and persistent toggle cancel an active resize. A changed valid `Context`, including side, bounds, metrics or override, cancels the resize and releases capture; an identical context preserves it. Override changes clear stale temporary reveal. Invalid updates/configuration leave the entire prior component state intact.

`FocusLocation` is a current semantic observation supplied per operation; it is never retained. `FocusRequest::{kSidebar,kPageContent}` names a destination class only. The host resolves the actual live, allowed control or current page, including split and modal restrictions. These effects do not acknowledge native focus movement. Before hiding or removing a focused native control, the host must safely apply the emitted return-focus request and release any capture it still owns. A failed focus operation must retain a safe visible native presentation rather than hide the focused control. Native adapters can compute a candidate using a copy of this value component, perform the required native effects, then adopt its result; on failure retain the prior component/presentation. Publish durable preference changes only after that native interaction succeeds. There is no asynchronous native transaction implemented here.

## Source integration boundary

The [pinned integration design](../native-space-integration.md) and [source references](../native-space-source-references.md) describe Chromium `155.0.8059.79`, commit `a49e2e3d4addfd84e765b52afe230bedb81096db`, and the real host/Profile ownership extraction still required. Their reviewed version-2 design does not supersede the [version-3 lifecycle contract](../lifecycle-contract.md). This presentation component neither implements nor bypasses that extraction.

The local [vertical-layout patch](../../../patches/helium/ui/layout/vertical.patch) already places left/right vertical strips and separates a same-mode alignment relayout from strip reset/reinitialization. Reuse that distinction when binding authoritative geometry rather than recreating tab views for visibility or side changes. The [split patch](../../../patches/helium/ui/split-view.patch) ties resize UI and menu cancellation to the current native tab model. Native observation and stale-control cancellation belong in the host adapter, not a parallel sidebar page registry. Only the documented adjacent source slices were inspected; no patch application or native integration was performed.

## Focused verification

Run from the repository root using the existing C++17 toolchain. The driver mode follows the existing lifecycle check documented for this host.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/layout/sidebar/sidebar.h \
  arcified/native/features/layout/sidebar/sidebar.cc \
  arcified/native/features/layout/sidebar/sidebar_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/layout/sidebar/sidebar.cc \
  arcified/native/features/layout/sidebar/sidebar_test.cc \
  -o /tmp/arcified-sidebar-layout-test
/tmp/arcified-sidebar-layout-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/features/layout/sidebar/sidebar.cc \
  arcified/native/features/layout/sidebar/sidebar_test.cc \
  -o /tmp/arcified-sidebar-layout-sanitized
/tmp/arcified-sidebar-layout-sanitized
clang-tidy arcified/native/features/layout/sidebar/sidebar.cc \
  --checks='-*,clang-analyzer-*,bugprone-*' --warnings-as-errors='*' \
  -- -std=c++17
```

Fixtures cover persistent toggling; hover versus engaged keyboard reveal; leave and focus-left; overlay/suppression composition with either preference; left/right resizing and clamping; capture acquisition/release; commit/cancel; Escape ordering; changed/unchanged context; unavailable minima and later preference recovery; complete or unavailable caller-sized hit targets; scaled synthetic geometry; and atomic invalid-input refusal. They assert public geometry, preferences and semantic effects. They do not simulate a native renderer or claim focus actually moved.

Local check results on 2026-10-23: Chromium formatting passed; strict C++17 compilation and all fixture groups passed; AddressSanitizer/UndefinedBehaviorSanitizer compilation and execution passed; production `clang-tidy` analyzer/bugprone checks passed with no user-code diagnostics. System-header warnings were suppressed by the analyzer's default filtering. New-file whitespace checks emitted no diagnostics. No native browser interaction or visual check ran.

Native acceptance stays open for every brief criterion on Linux, Windows and macOS: preserve selected page/document, scroll, form/caret and split identity across hide/reveal, resizing and fullscreen; keyboard entry/leave with actual focus and accessibility announcements; native pointer/touch capture and cancellation; active-tab location, caption/dialog exclusions and minimum hit metrics on small/mixed-scale displays. Record source/build identity and visual evidence in a later authorized installed-native run. No real browsing data or AI provider is used by these fixtures.
