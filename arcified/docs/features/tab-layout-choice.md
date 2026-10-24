# Tab layout choice

Issue [104](https://github.com/Timpan4/helium-arcified/issues/104), `tab-layout-choice`. Status: reusable standalone C++ presentation component implemented; installed native acceptance remains **OPEN** on Linux, Windows and macOS. No Chromium preparation, build, download, benchmark or CI run is included.

The [publication brief](../research/arc-dia/publication/tab-layout-choice.md) and `tab-layout-choice` entry in [backlog.json](../research/arc-dia/backlog.json) require vertical and horizontal presentations of the same workspace. This slice implements measured layout choice and borrowed tab/action projection in [tab_presentation.h](../../native/features/layout/tab_presentation/tab_presentation.h) and [tab_presentation.cc](../../native/features/layout/tab_presentation/tab_presentation.cc). It adds no native facade, workspace engine, preference codec or storage writer.

## Interface and authority

`Preferences` contains only `Orientation::{kVertical,kHorizontal}` and `Density::{kCompact,kComfortable}`. The caller supplies both values. `Describe` returns semantic UI descriptors, not localized accessible names or a serialization contract. Platform materials, modifiers, icons, text and metrics belong to adapters. Sidebar side is passed separately as the existing `arcified::sidebar::Side`, owned by [the sidebar component](../../native/features/layout/sidebar/sidebar.h). There is no duplicate side enum, sidebar width, durable visibility preference or focus owner.

`AuthoritativeTabs` borrows the caller's attachment, immutable Profile identity, ordered `vector<TabIdentity>` and optional selected `TabIdentity` through const reference wrappers. `TabIdentity` references an existing saved `ItemId`, shortcut `ShortcutId` or temporary `PageId`. Aliases and items remain distinct rows even when they resolve to the same live page. Different saved identities with identical destinations also remain distinct. No title, URL, native index or map-key order supplies identity or user order.

The workspace/native adapter must supply already valid, scoped identities and a unique row order, with selection absent or referencing a row in that order. It owns saved-item visibility, local temporary filtering, Profile isolation, the selected page, and saved-item/alias-to-live-page resolution. This component deliberately does not reconstruct those authorities from the lifecycle maps. The [lifecycle v3 contract](../lifecycle-contract.md) remains authoritative. In particular, saved Space entries are shared, temporary pages are window-local, and activation of an already-live saved entry uses the existing page transfer path. Activity, destructive actions and migration decisions remain open.

`Project` returns either `Presentation` or `ProjectionError`. A successful result borrows the same objects. `RowAt(index)` returns a reference to the original identity, the caller's selected flag, and accessibility set size/one-based position. Every activation surface uses that same `activation_target`; native keyboard, pointer and accessibility adapters must route it through the host's common command resolver. The component performs no activation, navigation, selection update, page creation or focus actuation. Native roles, accessible names, accelerator registration and actual command parity remain integration work.

Borrowed objects must outlive every projection and row reference. The host serializes workspace/native changes and reprojects after order, selection, identity, Profile, attachment, density or measurement changes. Results are presentation views, not reservations, durable records, focus snapshots or a native page registry. They must not be retained across asynchronous native actuation as authorization. Exports continue to use the authoritative workspace identities and graph; no layout-specific workspace export or navigation copy is introduced here.

## Measured fallback and composition

`FitMetrics` supplies available width/height and the width/height required by each orientation at the chosen density. All measurements use the caller's same logical coordinate system. The host must account for current display scale, caption controls, keyboard focus affordances, tab overflow and minimum usable page content before calling. The component defines no pixel breakpoint, minimum size, capacity, scale factor or geometry.

1. Use the preferred orientation when its measured requirements fit, including exact equality.
2. Otherwise use the alternate orientation when it fits, without changing the preference, density, side, rows or selection.
3. If neither fits, return `kNoUsableLayout`. Negative or non-finite measurements return `kInvalidMetrics`. The host handles the failure, retains a usable surface where available and remeasures. No row truncation or substitute selection is invented.

`effective_orientation()` identifies the actual placement. `requested()` retains the original value, and `used_fallback()` distinguishes a temporary fit decision from an explicit preference change. A later projection with sufficient room returns to the original preference. Synthetic scale transformations test consistency of this calculation; they do not prove mixed-display native behavior.

The host passes Focus mode's `Projection::chrome` to `sidebar_override(chrome_override)`. **Effective horizontal layout always returns `kSuppress`; effective vertical layout forwards the supplied override.** The default argument is `kNone`. This precedence applies to measured fallback too. A Focus reveal in horizontal layout goes to the host's horizontal toolbar/tabstrip and never reveals a second vertical sidebar.

Feed the composed override to sidebar `Context::presentation_override`. Sidebar owns suppression, forced overlay, resize cancellation and content bounds. Vertical Focus suppression/reveal uses the same full content bounds and leaves durable sidebar preferences intact. Horizontal presentation remains suppressed through both states. The host passes the actual resulting sidebar content bounds to the split-geometry component; tab presentation owns no geometry. Caption/fullscreen/dialog handling and the horizontal toolbar overlay remain host work. Focus return uses the host's existing live target checks; no target or focus history is stored here.

## Source-backed integration boundary

| Inspected source or pinned design | Consequence for this component |
| --- | --- |
| [Helium vertical.patch](../../../patches/helium/ui/layout/vertical.patch), patch lines 1093 to 1126 | `BrowserView::OnVerticalTabStripModeChanged` distinguishes orientation changes, which reset/reinitialize strips, from alignment changes, which only invalidate layout. Native integration should reuse this separation while retaining tab identities and selection. This slice supplies semantic placement, not calls to a guessed native API. |
| [Native source references, H1](../native-space-source-references.md#h1-helium-patch-reuse-and-constraints) | The vertical controller and native command/MRU/shortcut infrastructure are reuse points. Shared action identity does not establish native accelerator or accessibility parity. |
| [Native integration design](../native-space-integration.md#proposed-ownership-seam), pinned Chromium `a49e2e3d4addfd84e765b52afe230bedb81096db` | Authoritative presentation order/selection belongs to the workspace coordinator; the host owns real focus, bounds, captions, fullscreen and dialogs. This component borrows those semantic references without taking their ownership. |
| [Native source references, C1/C2](../native-space-source-references.md#c1-desktop-window-and-profile-ownership) | The existing Browser/BrowserView/Profile coupling still blocks a claim of same-native-window cross-Profile extraction. Changing orientation does not solve that ownership gate. |
| [Lifecycle v3 model](../../native/model/lifecycle.h) | Reuse strongly typed IDs. Do not copy current navigation into rows, reopen a live page for a layout switch, change Profile, or introduce independent order/selection/persistence state. The older pinned design's v2 product assumptions are superseded by the v3 contract. |

The local patch body and model/header contracts were read. Chromium ownership evidence here is attributed to the existing pinned source review, not a new prepared-tree inspection or patch application.

## Focused checks

[tab_presentation_test.cc](../../native/features/layout/tab_presentation/tab_presentation_test.cc) uses synthetic identities, reserved `.test` destinations and explicit fixture measurements. It checks both orientations, both densities and both sidebar sides; original row references/order and alias selection; caller-driven reprojection; empty/no-selection projection; exact fit and fallback in either direction; consistent scaled inputs; unavailable and invalid measurements; and actual sidebar/Focus composition with durable sidebar visibility both enabled and disabled. A synthetic caller dispatches the projected alias through lifecycle v3 `OpenShortcut` and receives the same live saved page with its Profile, current navigation, protected activity and attachment intact. The test does not claim that native activation occurred.

Run from the repository root using the existing compiler and formatter. The explicit Clang driver mode follows the existing lifecycle check for this T3 host.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/layout/tab_presentation/tab_presentation.h \
  arcified/native/features/layout/tab_presentation/tab_presentation.cc \
  arcified/native/features/layout/tab_presentation/tab_presentation_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/layout/tab_presentation/tab_presentation.cc \
  arcified/native/features/layout/tab_presentation/tab_presentation_test.cc \
  arcified/native/features/layout/sidebar/sidebar.cc \
  arcified/native/features/layout/focus_mode/focus_mode.cc \
  arcified/native/model/lifecycle.cc -o /tmp/arcified-tab-presentation-test
/tmp/arcified-tab-presentation-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/features/layout/tab_presentation/tab_presentation.cc \
  arcified/native/features/layout/tab_presentation/tab_presentation_test.cc \
  arcified/native/features/layout/sidebar/sidebar.cc \
  arcified/native/features/layout/focus_mode/focus_mode.cc \
  arcified/native/model/lifecycle.cc -o /tmp/arcified-tab-presentation-sanitized
/tmp/arcified-tab-presentation-sanitized
```

The formatter, warning-clean C++17 fixture executable and AddressSanitizer/UndefinedBehaviorSanitizer fixture executable passed on the current Linux host. These standalone checks compile the production presentation components and lifecycle model together. They do not compile Chromium or prove a native interface works.

## Acceptance still open

| Issue requirement | Evidence in this slice | Remaining proof |
| --- | --- | --- |
| Switch layout, side and density without changing workspace meaning | Borrowed identity/order/selection fixtures and lifecycle alias activation preserve semantic references and existing saved-page state. | Native strip switching, same actual page/window identity, Profile isolation and command-target evidence on each platform. |
| Persist preferences; usable captions and focused controls at small/mixed-scale sizes | Explicit preference values and caller-measured fit/failure behavior. | Durability owner must define preference scope, schema, storage, recovery and restore. No codec or storage writer is added. Native captions, actual focused controls and mixed-display changes remain unverified. |
| Same keyboard/accessibility actions and predictable narrow fallback | Shared activation identity/ordinal metadata; deterministic measured fallback; real sidebar/Focus precedence fixtures. | Native roles/names, complete command parity, focus return, overflow, keyboard/pointer interaction, screen-reader announcements and horizontal chrome overlay. |
| Platform adapters and exports retain meaning | Platform-neutral values borrow authoritative graph identities; no copied navigation or layout-specific workspace state. | Native Linux/Windows/macOS adapters and durable export/import integration. No export codec exists in this slice. |

No installed browser interaction or visual verification ran. Native acceptance stays OPEN on all target platforms. No real browsing data or AI provider was accessed. Independent review and integration status are recorded in [the workstream status](layout-workstream-status.md).
