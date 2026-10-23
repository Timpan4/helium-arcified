# Horizontal split presentation

Local component for [issue 81](https://github.com/Timpan4/helium-arcified/issues/81),
`split-horizontal`. Native acceptance remains blocked. No browser adapter or
installed-build behavior is implemented here.

## Authority and API

The [issue artifact](../../../docs/research/arc-dia/publication/split-horizontal.md)
and `split-horizontal` record in
[backlog.json](../../../docs/research/arc-dia/backlog.json) define the requested
outcome. [Lifecycle contract version 3](../../../docs/lifecycle-contract.md) and
[Model](../../model/lifecycle.h) own live pages, Profile identity, attachments,
split membership and split order. This component reads those const views.

The coordinator accepted [layout.h](layout.h) before implementation. Geometry
and metadata reconciliation consumers share these types.

- `LayoutMetadata` holds orientation and `PageId` keyed relative weights only.
  Horizontal places panes left to right; vertical places them top to bottom.
  No URLs, navigation, selected-page registry or independent page list is stored.
- `ReconcileLayout` projects metadata onto current `Model::splits()` membership.
  It prunes stale keys, even stale invalid values, and rejects invalid orientation
  or nonfinite/nonpositive weights for current members. A missing member weight
  uses the dimensionless relative identity `1.0`. Defaulted panes have equal
  weights to each other, which need not equal explicitly weighted survivors.
  Weights need no particular sum and impose no pane count or pixel minimum.
- `ResolveLayout` returns an owned, short-lived view in canonical lifecycle order.
  An optional caller focus target must be a current member. An omitted target
  proposes no focus change. Stale targets fail rather than choosing a replacement.
- `PlanReorder` accepts an exact permutation of the current split. Its output is
  a proposed `SetSplit` argument snapshot, with attachment and an optional external
  focus target. It never changes the model or performs native work. Weights follow
  page identity through reorder instead of following pane indices.
- `PlanUnsplit` validates an explicit member to present after unsplitting. The
  proposal identifies only the layout to remove and the caller's focus target.
  `Model::RemoveSplit` leaves all pages alive. Closing a page remains a separate
  lifecycle operation under the existing conservative activity checks.

Callers serialize model commands, discard projections after model mutations,
and revalidate proposals against the current model and actual native liveness
before acting. These snapshots are neither reservations nor native completion
records. Commit lifecycle changes only after the real native operation succeeds.
Reconcile metadata after actual lifecycle changes, including close or saved-page
transfer. If the split disappears, `kSplitNotFound` lets the metadata owner discard
its entry. No store, observer, native focus actuator or persistence writer exists
in this component.

## Local evidence

`layout_test.cc` uses synthetic `.example.test` destinations and exercises the
public API with the existing lifecycle implementation. It checks canonical
three-page ordering without adopting Helium's native pair constraint as a
product limit; default and explicit weights; extreme finite positive weights;
invalid orientation/weights; stale-key pruning; external focus validation; exact
reorder permutations; and preserved page identity, Profile, navigation and
protected activity. It also checks refused unknown-activity close, successful
explicitly quiescent close, saved-page transfer, one-member projection, empty
split removal, and unsplit retaining every page.

The following focused check passed with Clang 23.1.3 on the Linux development
host. It proves standalone C++17 component behavior, not native browser behavior.
Run from the repository root:

```sh
test_binary=$(mktemp /tmp/arcified-splits-layout.XXXXXX)
clang++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/splits/layout.cc \
  arcified/native/features/splits/layout_test.cc -o "$test_binary" && "$test_binary"
```

Expected output is `split layout checks passed`. Chromium formatting validation:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/splits/layout.h \
  arcified/native/features/splits/layout.cc \
  arcified/native/features/splits/layout_test.cc
```

## Remaining native acceptance

| Issue criterion | Local evidence | Still open |
| --- | --- | --- |
| Independent navigation, focus and permissions in the intended Space/Profile | Metadata references existing pages; synthetic transitions retain Profile and navigation. Focus arrives from the caller. | Real native page ownership, command routing, permission targeting, and same-window cross-Profile ownership proof. |
| Resize, swap, focus, close and unsplit preserve identity/content | Pure reorder and unsplit proposals; metadata reconciliation after lifecycle close and transfer; retained page fields checked. | Native create/resize/swap/focus/close/unsplit, cancellation and stale native events. Geometry is a separate consumer. |
| Keyboard and assistive technology operate boundaries and expose active pane | Explicit focus target input/output, with stale/nonmember rejection. | Native keyboard dispatch, boundary controls, accessibility tree, focus publication, and visual interaction evidence. |
| Origin identity, permissions and browser-owned dialogs stay on the intended page | No native security surface is modified. | Installed-build origin, permission, modal and dialog tests through site-controls/security-surfaces. |

Every criterion remains open for installed native Linux, Windows and macOS
evidence. [Native Space integration](../../../docs/native-space-integration.md)
and its [pinned source references](../../../docs/native-space-source-references.md)
require an ownership refactor before same-window multi-Profile acceptance can be
claimed. Existing Helium pair containers and restore handling are source
constraints, not an approved product pane cap. No preparation/build/download,
benchmark, real browsing data, provider access or native UI check was performed.
Temporary-tab native integration and the focus-routing contract remain issue
prerequisites. Activity policy, cleanup, destructive actions and populated
migration remain owner decisions; this component does not settle them.
