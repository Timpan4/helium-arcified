# Window chrome hit regions

Preparatory native component for [issue 35](https://github.com/Timpan4/helium-arcified/issues/35). `HitTestChrome` implements a reusable, stateless caption-region decision with standalone C++17 tests. It is not wired into Chromium or an installed browser. All installed-build acceptance remains open on Linux, Windows and macOS.

## Interface and authority

`hit_regions.h` accepts a nonempty query rectangle, an explicit native-frame policy and `ChromeHitRegions`. The geometry borrows the caller's vectors for this call only. It owns no window, page, Profile, lifecycle, navigation, persistence, appearance preference or focus state. Lifecycle contract v3 remains authoritative. The older native integration design does not approve another ownership model.

The native host must first establish that custom hit testing is appropriate. `kRetainNativeResult` always returns `kDeferToNative`, including over an interactive custom control. Resize borders, OS caption buttons and their snap behavior, frame overlays, fullscreen/immersive routing and teardown keep their existing native handling. `kDeferToNative` means preserve/continue that path, never synthesize a client or caption result.

When the host supplies `kAllowCustomChrome`, precedence is:

1. Invalid geometry, empty queries and queries not fully inside current custom chrome bounds defer to native handling.
2. Any intersection with a native exclusion defers to native handling.
3. Any intersection with an interactive exclusion returns `kClient`.
4. Full containment in one caption candidate returns `kCaption`.
5. Everything else defers to native handling.

An adapter maps the last two concrete results to its existing native client/caption result. The component neither moves nor resizes a window and does not replace platform hit-test codes. Caption dragging, double-click, system menus and snap remain platform behavior. Any overlapping native exclusion wins over interactive and caption regions. Interactive exclusions win over caption candidates. Adjacent or overlapping caption candidates are not unioned to authorize a query spanning them. This conservative ambiguity rule can leave draggable gaps for the host to resolve.

## Layout and appearance boundary

The layout/appearance caller supplies current, post-layout rectangles in one window-local integer DIP coordinate space. Origins may be negative. Extents are nonnegative, edges are half-open, and arithmetic widens before adding extents. No scale, margin, minimum size or capacity is invented here. The fixtures' geometry values are synthetic examples, not product defaults.

Convert the query and regions through the native view transforms, including RTL mirroring and clipping. Native scale conversion and enclosing rounding belong to the adapter. A point query may use the native one-DIP rectangle convention; larger contact queries must exclude any intersected control, rather than testing only their origin. Use actual hit-test regions, not painting bounds. Nonrectangular hit shapes need conservative enclosing exclusions or native hit testing. Conservatively excluding a rectangle can reduce draggable space.

Supply only controls that participate in the current hit-test layout. Visibility, event handling, animation and accessibility semantics remain native facts. Retain reserved native-caption geometry during a fade when the native layout still reserves it. Opacity does not authorize this component to remove that exclusion or enable an invisible control. An empty exclusion is ignored; a negative extent anywhere invalidates the full call, even outside the query. Regions may extend beyond current bounds after clipping or a narrow layout, but a query outside those bounds cannot become a custom target.

The component cannot detect stale snapshots, different coordinate spaces, a missed nonrectangular hit region or a dangling borrowed vector. The host must call synchronously with a coherent live layout on its UI sequence and prevent mutation during the call. After resize, display-scale change, display removal, mirroring, reparenting or presentation changes, supply the resulting native geometry again. When freshness, routing or ownership is uncertain, retain native handling. There is no generation counter or parallel layout cache here.

Native focus registry/routing, focus application, accessibility announcements, modal dialogs and popup ownership remain host-owned. This component supplies no focus suggestions or accessibility metadata.

## Source evidence

The local source patches were read at task checkpoint `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. These are source observations, not proof that the patch series applies or the browser behaves correctly.

| Source | Inspected behavior and use here | Git blob |
| --- | --- | --- |
| [`toolbar-window-frame-hit-test.patch`](../../../../patches/helium/ui/toolbar-window-frame-hit-test.patch), lines 5 to 14, 20 to 31, 81 to 109 and 119 to 123 | Teardown guard; converted toolbar coordinates; client/caption distinction; enclosing transformed rectangles and child `HitTestRect`; point represented by a one-DIP rectangle. Motivates current caller geometry, control-intersection exclusions and native fallback. | `c2142c395272145bb725e5f22a2189607fc00a6c` |
| [`zen-caption-buttons.patch`](../../../../patches/helium/ui/layout/zen-caption-buttons.patch), lines 7 to 10, 49 to 53 and 97 to 116 | Caption-button layout exclusions survive fading, mirrored native button bounds and event eligibility remain native; Windows keeps `HTMAXBUTTON` handling. Motivates reserved native exclusions and leaving platform caption results authoritative. | `b9edc5fc65a95ca4259963f358f9bbfb7e84e022` |

The [lifecycle v3 contract](../../../docs/lifecycle-contract.md) and [native integration boundaries](../../../docs/native-space-integration.md#proposed-ownership-seam) leave actual OS lifetime, frame and focus with the native host. [Source references C1/C2](../../../docs/native-space-source-references.md#c1-desktop-window-and-profile-ownership) establish that stock Browser/BrowserView ownership cannot be bypassed by presentation helpers. This directory adds no native ownership or cross-Profile mechanism.

## Focused verification

Run from the repository root using the existing compiler and formatter. The explicit Clang driver mode follows the existing lifecycle check on this host.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/window_chrome/hit_regions.h \
  arcified/native/features/window_chrome/hit_regions.cc \
  arcified/native/features/window_chrome/hit_regions_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/window_chrome/hit_regions.cc \
  arcified/native/features/window_chrome/hit_regions_test.cc \
  -o /tmp/arcified-window-chrome-test
/tmp/arcified-window-chrome-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/features/window_chrome/hit_regions.cc \
  arcified/native/features/window_chrome/hit_regions_test.cc \
  -o /tmp/arcified-window-chrome-sanitized
/tmp/arcified-window-chrome-sanitized
git diff --check -- arcified/native/features/window_chrome/
# Include newly added, untracked files. Git no-index uses status 1 for a diff.
for file in arcified/native/features/window_chrome/README.md \
  arcified/native/features/window_chrome/hit_regions.h \
  arcified/native/features/window_chrome/hit_regions.cc \
  arcified/native/features/window_chrome/hit_regions_test.cc; do
  check_status=0
  git diff --no-index --check -- /dev/null "$file" || check_status=$?
  if [ "$check_status" -gt 1 ]; then exit "$check_status"; fi
done
```

Fixtures cover native precedence, toolbar controls and gaps, contact rectangles, half-open edges, uncovered and ambiguous queries, borrowed layout updates, mirroring, retained caption reservations, narrow windows, empty/invalid input and extreme signed coordinates without overflow. They exercise public inputs and results only and use synthetic geometry. No browsing data, providers or network requests are involved.

## Native gaps

No production adapter, GN target, quilt patch, source preparation, native build or UI execution is included. A prepared-tree adapter must still demonstrate native hit-test precedence and native transform/rounding semantics. Visual verification cannot run against this standalone geometry component. Installed Linux, Windows and macOS tests still need move, resize, maximize, minimize, fullscreen, snap/tiling, mixed display scales, display removal, narrow-window control reachability, restored geometry, dialogs, popups, focus and accessibility evidence. Geometry restoration and persistence remain with the existing native implementation. This slice does not close issue 35 or its native-build prerequisites.
