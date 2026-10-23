# Layout, split and motion conformance

The [scenarios](../../native/integration/layout/layout_split_composition_test.cc)
link the actual lifecycle, focus, tab presentation, sidebar, split layout,
split geometry and motion implementations. The component directories match
the reviewed inputs `127bf189` (layout), `eba06b39` (splits, including the
cleared numeric fixes) and `e2057bd1` (motion). No shared feature API changed.
This new composition still needs the parent's fresh independent review.

The test wiring uses the accepted public pipeline:

```text
focus.Project(facts).chrome
  -> tabs.sidebar_override(chrome)
  -> sidebar.Context.presentation_override
  -> sidebar.Configure(context, current_sidebar_focus)
  -> sidebar.layout().content.width/height
  -> splits::GeometryInput.viewport
  -> splits::ProjectGeometry(model, split, metadata, input)
```

The fixture reports focus outside the sidebar. It never treats a focus intent
as native completion. Bounds changes reproject tabs with fresh fit metrics,
configure the sidebar and recompute split geometry. Nonzero content origins
remain outside split geometry. The test explicitly subtracts that origin from
a fixture host point before content-local pane hit testing. A native host must
translate pane rectangles into its own coordinates when placing views.

## Coverage

| Scenario | Observable checks |
| --- | --- |
| `HorizontalRevealCannotRestoreVerticalSidebar` | Both sidebar sides, requested horizontal tabs and fallback from vertical to horizontal. Pointer and keyboard focus reveals show an overlay for vertical tabs but keep the vertical sidebar and its hit targets suppressed for horizontal tabs. Sidebar reveal commands cannot bypass suppression. Switching back to vertical restores docked preferences. Borrowed row and selection references remain intact. |
| `DialogAndFullscreenUseCurrentBounds` | Dialog suppression uses freshly supplied bounds. Actual chrome focus keeps an overlay without docking. Returning focus to content suppresses it. Overlay-allowed fullscreen permits reveal; exclusive fullscreen invalidates the old reveal and defers new reveal. Dialog/fullscreen Escape defers to the host. Returning to normal restores docking using current bounds. |
| `SplitBoundsAndReferencesSurviveSuppressionAndResize` | Both sidebar sides and split axes. Exact pane/divider bounds use docked content dimensions and start at content-local zero. Pane hit testing excludes divider gaps and outer edges. Split resize uses the current content extent. Focus suppression and overlay reveal expand content without discarding resize weights. Shrinking bounds produces active-only geometry and refuses resize, while hidden panes remain keyboard targets. Restoring bounds and exiting focus restores all panes and retained weights. |
| `MotionCannotReplayObsoleteSemanticBounds` | Semantic suppression, reveal and docking take effect before motion completes. Reversal rejects the obsolete run's samples, finish and cancel. After cancellation and a bounds change, a fresh layout query supplies the decorative reset. Canceled callbacks and reduced motion cannot replay that run. Reduced motion settles only the current run. Geometry, tab selection, Space selection and reported focus remain under their existing owners. |

Page IDs, split membership/order/attachment, temporary-tab IDs, Profile,
navigation, protected activity, tab order and selection references are checked
after presentation changes. Metadata checks retain orientation and explicitly
accepted resize weights. Synthetic dimensions, minima, fit metrics, divider
extent and interpolation phase are fixture values, not product policy. The
motion fixture maps sidebar presence to a decorative normalized scalar. That
mapping supplies no semantic rectangle or commit path.

## Exact reproduction

From the repository root, using the already installed Clang tools:

```sh
bash -n arcified/native/integration/layout/run_checks.sh
bash arcified/native/integration/layout/run_checks.sh
```

The [runner](../../native/integration/layout/run_checks.sh) compiles only the
linked scenario and its real component implementations. It runs strict C++17
with `-Wall -Wextra -Werror -pedantic`, then the same scenarios with
`-fsanitize=address,undefined -fno-sanitize-recover=all`, frame pointers and
`ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1`. Temporary binaries
are removed on exit. It checks the new test with Chromium `clang-format` and
`clang-tidy --checks='-*,clang-analyzer-*,bugprone-*' --warnings-as-errors='*'`.
The underlying reviewed components are compiled unchanged; this lint run is
scoped to the new translation unit.

On Linux x86_64 on 2026-10-23, Clang, clang-format and clang-tidy 23.1.3 passed
these checks. Both executables printed
`layout/split composition: all scenarios passed`. No sanitizer diagnostics or
test-file lint diagnostics remained. Clang-tidy reported suppressed warnings
from non-user code. Bash syntax passed. No Markdown linter is configured for
this path.

## Gaps

These are conformance scenarios with real components and synthetic caller
inputs. There is no production host or native UI. They prove the exercised
composition, not adoption by an eventual adapter or enforcement against a
future adapter that incorrectly writes semantic state from a callback. Native
view placement, actual focus/capture, fullscreen/dialog behavior, animation
callbacks/lifetime, rendering, accessibility and OS reduced-motion observation
remain unverified. No visual interaction or Linux/Windows/macOS browser build
ran. Full Chromium preparation/build, remote writes and commits were excluded.
The parent owns the independent review of this new join.
