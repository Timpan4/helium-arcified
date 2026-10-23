# Motion presentation component

Status: standalone C++ component for [issue 83](https://github.com/Timpan4/helium-arcified/issues/83), based on checkpoint `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. Native acceptance remains open. This code interpolates presentation values and rejects obsolete runs. It does not animate native UI, implement a browser animation driver, or establish same-window multi-Profile hosting.

## Scope and interface

[`PresentationTransition`](../../native/features/motion/presentation_transition.h) maintains one normalized presentation value and an optional interpolation run. It has no dependency on the lifecycle model or layout module. [Lifecycle contract v3](../lifecycle-contract.md) remains authoritative for ownership and navigation. Space themes remain Space-scoped. No lifecycle, persistence, focus, selection, activity, cleanup, destructive-action or migration policy is added.

Layout owns visibility, rectangles, anchoring, hit testing and focus. Its immediate authoritative projection may be hidden, docked or overlay, including focus-mode overrides. Motion can decorate that projection; it cannot delay or commit it. The scalar has no built-in meaning such as "sidebar visible" or "selected Space". No completion gates input or semantic selection.

| API | Observable behavior |
| --- | --- |
| `Progress::Parse(value)` | Accepts finite values in the closed interval `[0, 1]`. Rejects NaN, infinities and out-of-domain values rather than silently clamping external input. This interval defines normalized interpolation, not a product limit. |
| `Retarget(target, mode)` | Invalidates the previous run and starts from the last accepted value. Returns a fresh opaque token. Reduced motion or zero distance sets the target immediately and returns no run. Mode is supplied for every request, never persisted. |
| `Sample(token, phase)` | Accepts only the current run. Interpolates between that run's start and target using the externally evaluated phase. Phase may rise or fall; the component chooses no easing. Floating-point rounding is bounded by the supplied endpoints. Phase 1 samples the exact target but does not finish the run. |
| `Finish(token)` | Accepts only the current run, sets its exact target and invalidates the token. A duplicate finish is rejected. |
| `Cancel(token)` | Invalidates only the matching run. Leaves the last value unchanged and returns a success flag, not a target or geometry. Old cancellation cannot cancel a replacement run. |
| `Reset(authoritative_value)` | Invalidates any run and adopts only the caller-supplied, freshly queried authoritative presentation. The caller owns the snap and any mapping from native geometry to the scalar. |
| `ReduceMotion()` | Settles the latest active target and invalidates its token. Returns false without changing the value when there is no run. Cancellation followed by this call cannot replay a cancelled target. Disabling reduced motion restarts nothing; a later explicit request supplies `kAnimate`. |

Tokens retain an allocated identity through shared ownership. While a stale callback retains a token, that identity remains alive and cannot be allocated to another run. Tokens from another component, a destroyed component, an earlier run, cancellation, reset, immediate settlement or completion cannot match a current run. There is no generation counter, expiry, timeout or reuse budget. Token retention does not keep a native view or the component alive. The native caller still needs its normal weak-lifetime protection.

## Native caller contract

The coordinator accepted this presentation-only seam with the layout owner before edits. No native adapter or shared header is introduced here.

1. On the UI sequence, query layout's current authoritative projection and the current native motion preference. Construct or reset the interpolation with the corresponding presentation value. The value is an accepted calculation, not proof that a compositor displayed it.
2. Supply each target and its current motion mode. Capture the returned token by value in that run's callbacks. Invalidate the old run through `Retarget`, `Cancel`, `Reset` or `ReduceMotion` before stopping its native animation, because stopping may synchronously invoke callbacks. Never substitute a mutable "current token" when an old callback fires.
3. Use externally validated timing and native animation semantics. Feed evaluated normalized phase to `Sample`, then apply `value()` only if it accepted the token. Keep sampling and application serialized, with native lifetime checks. Stop the native driver after completion or cancellation; rejected callbacks do not stop it themselves. An invalid phase is an adapter error, not permission to advance or finish the run.
4. On layout cancellation, call `Cancel` and query the authoritative projection again. The caller may apply that projection immediately and pass its scalar to `Reset`. Cancel never restores earlier geometry, selects a Space, or infers a target from past layout. If native application fails or another owner changes presentation, reset from the newly observed authoritative value before continuing.
5. When reduced motion becomes enabled during a run, call `ReduceMotion` before stopping the native driver. If it reports an active run, synchronously apply `value()` through the layout-owned renderer. A new request must query the setting again. If there is no active run, leave the projection alone. Platform preference observation and native rendering remain adapter work.

Do not feed an absolute native shown/hidden ratio into this API as though it were a new run's phase, or apply easing twice. Where an existing native controller already owns the property and supports reversal, retain that controller's semantics instead of introducing a second property owner. This component is an available interpolation algorithm for an eventual caller, not a replacement animation scheduler.

## Inspected native evidence

Chromium pin `155.0.8059.79` resolves to `a49e2e3d4addfd84e765b52afe230bedb81096db` in the existing [source identity record](../native-space-source-references.md#source-identity). The following pinned files and local patches were read directly for this slice. Gitiles TEXT was fetched with read-only HTTP after the web reader failed. No source preparation, archive download or patch application ran.

| Source | Observation and consequence |
| --- | --- |
| [`ui/gfx/animation/slide_animation.cc`](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/gfx/animation/slide_animation.cc) | `BeginAnimating` starts from the current value on reversal. Zero duration synchronously reports progress and end. Preserve current-value continuity and invalidate old callbacks before native stop/start operations. This component does not choose SlideAnimation's duration or dampening. |
| [`ui/gfx/animation/animation.h`](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/gfx/animation/animation.h) | `PrefersReducedMotion` belongs on the browser UI thread. `RichAnimationDuration` guidance can change dynamically and should be refreshed before each run. The native caller must obtain and observe these settings; this module does not emulate platform preferences. |
| [`browser_animation_controller.cc`](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/animation/browser_animation_controller.cc) | Existing browser animation machinery owns providers, motion specifications, native animation delegation and compositor observations. Reuse native machinery rather than adding clocks or frame logging here. |
| [`layout/zen-mode.patch`](../../../patches/helium/ui/layout/zen-mode.patch), lines 59 to 66, 431 to 476 and 767 to 790 | Helium uses `gfx::SlideAnimation` for top/side chrome and native layout invalidation in progress callbacks. The patch sets rich-animation duration during construction. Refreshing that wiring dynamically requires later native integration; it is not fixed by this standalone component. |
| [`layout/vertical.patch`](../../../patches/helium/ui/layout/vertical.patch), lines 93 to 105 and 752 onward | Layout consumes native animation values and presets controller state. Geometry remains with layout. |
| [`disable-side-panel-flyover.patch`](../../../patches/helium/core/disable-side-panel-flyover.patch) | Helium disables side-panel flyover by default. This slice does not enable it or adopt its timing. |

## Focused verification

On Linux, Clang, clang-format and clang-tidy `23.1.3` checked this standalone code. All seven synthetic fixtures passed with ASan/UBSan. They cover external numeric validation; exact sampling/completion and falling external phases; repeated reversal and obsolete callbacks; cancellation followed by an authoritative snap; reduced motion during reversal and subsequent mode changes; immediate/same-target requests; token copies, different owners and destroyed owners. No browser, real browsing data or service is involved.

Commands from the repository root:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/motion/presentation_transition.h \
  arcified/native/features/motion/presentation_transition.cc \
  arcified/native/features/motion/presentation_transition_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/features/motion/presentation_transition.cc \
  arcified/native/features/motion/presentation_transition_test.cc \
  -o /tmp/arcified-motion-83-test
/tmp/arcified-motion-83-test
clang-tidy --checks='clang-analyzer-*,bugprone-*' --warnings-as-errors='*' \
  arcified/native/features/motion/presentation_transition.cc \
  arcified/native/features/motion/presentation_transition_test.cc \
  -- -std=c++17 -Wall -Wextra -Werror -pedantic
git diff --check
git diff --no-index --check -- /dev/null arcified/native/features/motion/presentation_transition.h
git diff --no-index --check -- /dev/null arcified/native/features/motion/presentation_transition.cc
git diff --no-index --check -- /dev/null arcified/native/features/motion/presentation_transition_test.cc
git diff --no-index --check -- /dev/null arcified/docs/features/motion.md
```

Formatting, compilation, execution and explicit clang-tidy checks passed. The initial clang-tidy invocation without explicit checks reported "no checks enabled". The explicit analyzer/bugprone invocation above resolved that configuration gap. Its final run reported no diagnostics for these files and suppressed diagnostics from non-user headers. No Markdown formatter/linter is configured for this documentation path. The separate no-index whitespace checks include these untracked files; exit 1 with no diagnostics denotes their difference from the empty file.

## Native acceptance still open

| Issue requirement | Remaining proof |
| --- | --- |
| Sidebar hide/reveal and Space switches preserve content anchoring and accept input | Native layout and rendering integration, keyboard/pointer input and anchoring checks. The scalar engine cannot prove rectangles, focus or visible continuity. |
| Reduced motion and interrupted/reversed transitions avoid stale final state | Algorithmic behavior is covered locally. Real OS preference changes, native cancellation callbacks, destroyed views and rendered results remain unverified. Peek and split own their separate integration checks. |
| Native visual checks and frame traces on Linux, Windows and macOS; validated durations | No installed native build or visual/frame evidence was produced on any platform. No timing or easing was selected. Windows/macOS compilation is also unverified. Full Chromium preparation/build and benchmarks were deferred by the task. |
| Disable app-generated sounds/haptics while retaining understandable loading/privacy feedback | Preference UI, persistence, sound/haptic integration and non-auditory native feedback are not implemented or verified by this slice. |
| Same actual native window across Profiles | Production host extraction remains unproven under the [native integration design](../native-space-integration.md). No model or interpolation result is native host evidence. |

The publication brief's acceptance boxes remain open. Central independent review and eventual native integration are separate from these local algorithm checks. No shared tooling, lifecycle files, patch series, workflows or execution catalog were changed.
