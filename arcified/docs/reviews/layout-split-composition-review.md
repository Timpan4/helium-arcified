# Layout, split and motion composition review

No blocking correctness or ownership finding in this join. One nonblocking
coverage finding remains. The scenarios exercise real component calls and
use independent expected rectangles. They do not establish native adoption.

Reviewed on 2026-10-23. Scope includes every file currently present under
`arcified/native/integration/layout/`, namely
`layout_split_composition_test.cc` and `run_checks.sh`, plus
`arcified/docs/features/layout-split-composition.md`. All three are untracked
inputs to this review. Only this review file was added.

## Finding

1. **P3. Assert the decorative endpoint after reducing motion.**
   At [test line 468](../../native/integration/layout/layout_split_composition_test.cc#L468),
   the test checks that `ReduceMotion()` succeeds and invalidates tokens, but
   never checks `transition.value()` afterward. The current value is zero
   before the restore run, whose current target is one. An implementation
   that clears the run and returns true without settling its value would
   satisfy all remaining assertions. Consequently, the feature document's
   statement that reduced motion settles the current run is stronger than
   this scenario's assertions. Add an assertion that the value equals
   `Decoration(sidebar->layout()).value()` immediately after reduction and
   remains there after the stale completion attempts. This is a coverage
   gap, not an observed defect in the independently reviewed motion component.

## Coverage and ownership

- `HorizontalRevealCannotRestoreVerticalSidebar` covers both sidebar sides
  and both requested-horizontal and vertical-to-horizontal fallback. A
  vertical positive control proves that pointer and keyboard focus reveals
  can produce an overlay. The same reveal routed through horizontal tabs
  suppresses the sidebar and all its reveal/resize targets. Direct sidebar
  reveal actions cannot bypass it. Exiting focus still suppresses horizontal
  presentation; switching to vertical restores docked preferences. This
  scenario does not project split geometry.
- `DialogAndFullscreenUseCurrentBounds` replaces the available rectangle,
  reprojects tabs, then configures the sidebar. It checks dialog suppression,
  preservation of controls with reported chrome focus, content-focus return,
  overlay-allowed fullscreen, exclusive fullscreen invalidation, deferred
  Escape and docking at the current rectangle. The fullscreen cases reuse
  the refreshed rectangle. They do not each introduce another bounds change,
  and this scenario does not project split geometry.
- `SplitBoundsAndReferencesSurviveSuppressionAndResize` covers both sides
  and both split axes. Exact docked pane/divider rectangles distinguish
  content dimensions from outer dimensions. Nonzero content origins stay
  outside `GeometryInput`; the fixture subtracts the origin before hit
  testing. The test checks the second pane's leading edge, a divider start
  and the outer bottom-right corner. It does not exhaust every hit-test edge.
  Resize uses the current docked content extent and explicitly accepts the
  returned weights. Suppression and overlay then reproject expanded content.
  Small bounds produce active-only geometry and refuse resize without losing
  membership or the keyboard target for the hidden pane. Restoring bounds
  restores both panes, orientation and accepted weights.
- `MotionCannotReplayObsoleteSemanticBounds` proves semantic suppression,
  overlay and docking through fresh sidebar queries and split projections
  before animation completion. It rejects obsolete hide samples, finish and
  cancel after reversal. After canceling show, changing bounds and dismissing
  reveal, it resets decoration from a fresh layout query and rejects the
  canceled run's sample and finish. Reducing motion without a run returns
  false. The final reduction invalidates the current restore token and the
  canceled show token, subject to the endpoint assertion gap above.

`Configure` carries the actual `focus.Project(facts).chrome` through
`tabs.sidebar_override` into `sidebar.Configure`. `Input` passes only the
resulting content width and height to the real split geometry implementation.
Neither helper stores state or supplies an alternative reducer. The fixture
consistently reports focus outside the sidebar. The separate chrome-focus
case can represent toolbar focus; it is not evidence of focus inside sidebar
controls or successful native focus movement.

The borrowed attachment, Profile, order and selection objects belong to a
fixture that outlives its tab projections. `RowAt` targets are checked against
the original vector elements, which are never reordered or reallocated.
Returned layouts, geometry, hit targets and page IDs are owned snapshots,
not retained references into expired temporaries. The accepted split weights
are the only deliberate metadata update after setup.

The lifecycle model is const after fixture construction. Reference checks
also verify page count, navigation, attachment, Profile, temporary-tab owner
IDs, protected activity, split membership/order and selected Space. Row order
and selected tab remain in their original owners. Motion owns only its
decorative scalar and run tokens. It receives no semantic-state reference or
callback capable of committing layout, focus or selection. The assertions
and API shape establish that separation for this fixture, not enforcement
against a future host that introduces such a callback.

All dimensions, fit requirements, pane minima, divider extents, resize deltas
and interpolation phases are synthetic test inputs. They introduce no product
size policy. The feature document identifies these limits accurately, apart
from the specific reduced-motion assertion gap.

## Runner review and checks

The runner resolves its repository root from its own path and quotes paths
and array arguments. `mktemp -d` creates a private directory. The deferred,
quoted EXIT trap removes only that directory. Compiler outputs stay there;
formatting uses dry-run and lint does not request fixes. `set -euo pipefail`
propagates failures from the sequential compile, execution and tooling steps.
There is no output pipeline that can swallow a failure.

Executed on Linux x86_64 with Clang, clang-format and clang-tidy 23.1.3:

1. `bash -n arcified/native/integration/layout/run_checks.sh` passed.
2. `bash arcified/native/integration/layout/run_checks.sh` exited zero.
   Both strict C++17 and fail-fast ASan/UBSan executables printed
   `layout/split composition: all scenarios passed`. No sanitizer diagnostics
   appeared. Chromium formatting and scoped analyzer/bugprone lint passed.
   Clang-tidy reported 22 warnings suppressed from non-user code.
3. With an exported shell function standing in for the first `clang++` call,
   returning status 1 caused the runner to exit 1 and remove its temporary
   directory. A separate injected `kill -s TERM "$BASHPID"` at that call caused
   exit 143 and also removed its directory. The injected function printed
   `output_dir` before exiting; both exact paths were checked afterward and
   were absent. These checks cover early failure and TERM cleanup, not
   cancellation of a running external compiler or every process-group signal
   route. No claim is made that the runner forwards signals to descendants.
4. `git diff --exit-code` found no changes between the layout directory and
   `127bf189`, splits and `eba06b39`, or motion and `e2057bd1`. This confirms
   the reviewed component inputs; their standalone suites were not rerun.
5. `git diff --no-index --check /dev/null` on each new input produced no
   whitespace diagnostics. The expected no-index difference status was
   handled separately from errors. No Markdown linter was run.

SHA-256 values before and after verification were identical:

```text
5a9ab84966e2f2283eb5ce782e95bbb174c0b501c6b6467854aa275dac5c38ef  layout_split_composition_test.cc
84532165dabf28296c9542de531b3f64b8dd06e6bede6cb3d35d78cc413abc47  run_checks.sh
ac9c2b6d90ccb385f80abeb4bd8caa59088b317b922f263ddc3a6726680d86a4  layout-split-composition.md
```

## Remaining native gaps

There is no production host, native facade or second semantic state owner in
this change. No native view placement, actual focus/capture, dialog/fullscreen
transition, animation callback lifetime, rendering, accessibility integration
or OS reduced-motion observation was exercised. Borrow safety here does not
validate asynchronous host lifetimes. A native adapter still needs to report
current facts and measurements, translate coordinates, preserve semantic
owners and safely cancel its own animation callbacks.

The document does not claim visual verification or a Linux, Windows or macOS
browser build. Those paths remain unverified. No Chromium preparation/build,
heavy download, commit, remote write, PR link or PR watch was performed.
