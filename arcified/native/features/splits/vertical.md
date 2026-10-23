# Split geometry and interaction projections

Issue [112](https://github.com/Timpan4/helium-arcified/issues/112) remains open for native acceptance. This directory contains local, pure C++ components. They do not attach WebContents, navigate a link, move native focus, change scroll position or establish same-window multi-Profile ownership.

## Authority and interface

The [issue artifact](../../../docs/research/arc-dia/publication/split-vertical.md) and `split-vertical` entry in [backlog.json](../../../docs/research/arc-dia/backlog.json) require orientation changes, retained page identity, chosen-pane links/drop targets and keyboard/accessibility behavior under resize and narrow windows.

`Model::splits()` owns ordered membership. Pages and attachments remain in the [lifecycle core](../../model/lifecycle.h). Shared `layout.h` owns orientation and PageId-keyed relative weights. Geometry consumes its resolved view, caller-supplied content dimensions, minimum pane dimensions, divider extent and active PageId. Results are disposable presentation snapshots and proposed targets. They are never a second membership, navigation, persistence or focus owner.

Horizontal means left-to-right. Vertical means top-to-bottom. The host supplies content dimensions after any chrome or visibility overlays. Suppression retains metadata and pages, and the host recomputes projection on release. Geometry does not own an overlay or modify scrolling.

`ProjectGeometry` fixes undersized panes at the caller minimum in ascending weight order, then divides the remaining extent proportionally. It retains a finite weight sum and max-normalizes only when that sum overflows. Proportional products use `frexp` mantissas and combined exponents with `scalbn`, so a significant extent survives even when the weight ratio alone would underflow or lose precision as a subnormal. This uses `double` throughout and does not depend on `long double` having additional precision on MSVC.

Before placing bounds, a reverse pass reserves room for later pane minima and dividers. If a subtraction rounds onto the forbidden side, `nextafter` selects the adjacent conservative bound. A forward pass clamps each proportional boundary between its representable minimum and the reserved suffix. The final edge consumes the residue without losing a trailing minimum to rounding. All loops consume finite pane collections; there is no numerical refinement loop, arbitrary epsilon, iteration cap or product limit. If the viewport cannot fit all panes and dividers on either axis, or a desired extent cannot advance its resulting origin, the projection explicitly returns `kActiveOnly` for the caller's active page. It does not inflate an unrepresentable tiny extent to the coordinate spacing at a large origin. Zero-area viewports use that same mode with zero-area bounds. Minimum dimensions must be finite and positive; viewport dimensions and divider extent must be finite and nonnegative.

`ProjectResize` returns proposed PageId-keyed relative weights from the current pane extents. It changes only the adjacent extents, clamps to caller minima, and refuses resize in fallback. The owner decides whether and when to apply the metadata. `KeyboardTarget` returns an adjacent page on the requested physical axis without wrapping; hidden fallback pages remain reachable. The owner can select that target and reproject. `PaneTargetAt` returns a visible pane with highlight bounds; half-open rectangles exclude divider gaps and outer edges.

## Local verification

Ran with Clang/clang-format `23.1.3` on Linux. From the repository root:

```sh
clang++ -std=c++17 -Wall -Wextra -Werror -pedantic arcified/native/model/lifecycle.cc arcified/native/features/splits/layout.cc arcified/native/features/splits/geometry.cc arcified/native/features/splits/geometry_test.cc -o /tmp/arcified-splits-geometry-test
/tmp/arcified-splits-geometry-test
clang++ -std=c++17 -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer arcified/native/model/lifecycle.cc arcified/native/features/splits/layout.cc arcified/native/features/splits/geometry.cc arcified/native/features/splits/geometry_test.cc -o /tmp/arcified-splits-geometry-sanitized
/tmp/arcified-splits-geometry-sanitized
clang-format --style=Chromium --dry-run --Werror arcified/native/features/splits/geometry.h arcified/native/features/splits/geometry.cc arcified/native/features/splits/geometry_test.cc
```

Both executables reported `split geometry fixtures passed`. No sanitizer or formatting diagnostics appeared. The synthetic fixtures cover orientation transposition, page identity and canonical reorder, pane/divider target edges, min-constrained proportions, extreme finite weights including normalization underflow and sum-overflow avoidance, exact-minimum fit, insufficient cross-axis extent, zero viewport, divider-total overflow, unrepresentable pane origins, fallback resize refusal, directional targeting of hidden pages followed by reprojection, resize clamping and unaffected neighbors, single-pane geometry, and invalid dimensions/layout/active-page/direction inputs. Assertions also retain lifecycle navigation, activity, attachment and membership and leave caller metadata/window selection unchanged.

Round 1 regressions run in both orientations. With weights `{a:1,b:2,c:1}`, axis extent `400`, minimum `100` and divider `5`, zero-delta and negative-clamped resize reproject to exact extents `{100,190,100}`; positive-clamped resize gives `{190,100,100}`. Tests check adjoining pane/divider edges, unchanged trailing extent and final edge `400`. Equivalent normalized weights also retain every minimum and the final viewport edge under boundary rounding. With weights `{a:1e-100,b:1e300}` and axis extent `1e300`, the first extent is `1e-100`, above minimum `1e-200`. A `1e-20` first weight checks a nonzero subnormal ratio, and an overflowing sum of two maximum weights checks the normalized path. An ordinary axis extent of `1` retains a `1e-20` pane at origin zero; reversing its order returns fallback because that extent cannot advance the far-edge coordinate.

These checks prove local projection behavior on Linux. MSVC, macOS and native browser execution remain unverified. The arithmetic uses portable C++17 operations without widened floating-point assumptions, but a Linux pass is not a Windows or macOS test. These checks do not prove no-reload native orientation/reorder, browser link/drop actuation, native accessibility focus or restart persistence. Geometry has no persistence writer; omission or restoration of this metadata belongs to the single workspace/persistence owner and must be explicit.

## Pinned source observations

Read the local patches without applying them or building Chromium:

1. [split-view.patch](../../../../patches/helium/ui/split-view.patch), blob `ca7a9b897d9e60f300513a7b814d9a42eb1f6547`, retains the orientation command and binds `MultiContentsResizeArea` to a TabStripModel. Its observer cancels stale menus on active selection or split changes. Native command/focus ownership must stay with the embedding host.
2. [multi-contents-drop-target.patch](../../../../patches/helium/ui/multi-contents-drop-target.patch), blob `c3fb4694e1291b52806cd10d34e2f21ca7f399bc`, distinguishes START, END and BOTTOM target geometry, uses height for a bottom target and width for side targets, and coordinates outlines, margins and frame corners. Those presentation constants and drag delays are not product thresholds for this component.

The [source references](../../../docs/native-space-source-references.md) pin Chromium `155.0.8059.79` at `a49e2e3d4addfd84e765b52afe230bedb81096db`. The [ownership design](../../../docs/native-space-integration.md) identifies immutable Browser/Profile and BrowserView/window coupling. Local geometry cannot repair that ownership chain. The existing native pair presentation is not an approved pane limit.

## Link and drop boundary

A target identifies an existing pane and its highlight rectangle. It grants no permission to replace that page's navigation or to create, move or duplicate a page. The host must revalidate native liveness, the current document, attachment, Profile and browser URL/security policy before acting. A snapshot can become stale after reorder, transfer or closure.

Saved outbound links follow `Model::PlanSavedPageLink`: browser-authoritative classification, temporary branch, native success, then actual lifecycle reporting with fresh IDs. Chosen-pane placement must be coordinated after that creation succeeds. Geometry never bypasses that contract to navigate a saved source. Same-site and other navigation policy remains outside this component. Tab drops likewise require existing ownership/transfer policy; geometry does not authorize a cross-Profile move.

## Native acceptance still blocked

No native build, browser interaction or visual check ran. Linux, Windows and macOS acceptance remains open for retained document/history/form state through orientation and reorder; accessible pane/divider semantics and announcements; pointer/keyboard focus and commands; native drag/link creation, cancellation and stale-target rejection; resize, narrow-window presentation and release of host suppression; and restart behavior through the persistence owner. Same-window multi-Profile production ownership remains unproven. Activity, cleanup, destructive-action policy and populated migration remain open and are not implemented here.
