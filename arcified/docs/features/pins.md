# Saved-item presentation and label editing

Issue: [67, pins](https://github.com/Timpan4/helium-arcified/issues/67).
Starting checkpoint: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`.
Native acceptance remains open on Linux, Windows and macOS.

## Scope and authority

The [published brief](../research/arc-dia/publication/pins.md) and the `pins`
entry in [the backlog](../research/arc-dia/backlog.json) require durable saved
items, independent saved destinations and current navigation, label/URL edits,
reset, unpin, order, undo and native interactions. This slice targets reusable
saved-item presentation and label-edit values. It does not complete that issue.

The [lifecycle contract](../lifecycle-contract.md) version 3 and
[`lifecycle::Model`](../../native/model/lifecycle.h) remain authoritative for
item identity, ownership, saved destinations, page identity, current navigation,
attachments and Profile boundaries. Presentation metadata must reference the
canonical `ItemId`, including when a Favorite is a `ShortcutId` alias. Different
items with the same URL remain different items.

## Reusable component

[`saved_item.h`](../../native/features/pins/saved_item.h) exposes the common
`arcified::pins` interface for Pins and Favorites. It uses only C++17 and the
existing Model. The coordinator approved this interface before implementation.
Organization owns Space hierarchy and order and consumes display labels keyed
by `ItemId`; this component adds no competing order or label store.

`SavedItemMetadata` contains only an optional `custom_label`. The caller owns
the values, indexed by canonical `ItemId`. There is no internal registry,
allocator, file format, I/O or second saved destination. No shared icon token
exists yet, so custom icon editing remains open.

`DescribeSavedItem(model, item, metadata, default_label)` checks item existence
and returns an owned label, canonical `ItemId`, whether the label is custom,
and an optional live `PageId`. It reads page ownership from Model on every call.
The result contains no URL, owner, Profile, attachment, focus or selection.
It is a disposable presentation snapshot, not a live-page cache. The caller's
nonempty default label is used when the custom label is absent. That default
is neither stored nor inferred from current navigation here.

`PlanLabelEdit(model, item, current, desired_label)` returns before/after
metadata and the canonical item ID. `nullopt` explicitly restores the default;
an empty string is refused. `ApplyLabelEdit(model, edit, current, direction)`
returns a replacement value, checking item existence and the expected metadata
side before applying forward or reverse. It refuses stale values instead of
overwriting a different label. No-op edits are allowed; a future journal decides
whether to record them. These pure values do not implement an undo stack or
approve journal retention, live-state recovery or destructive action policy.

Integration sequence:

1. Validate the displayed reference against Model and the caller's authorized
   Space/Profile presentation. Favorites validates `VisibleShortcuts` before
   resolving `Shortcut.target`, or uses a visible Profile-owned item. The common
   lookup is deliberately context-free. An alias can be visible while its
   target's owning Space is unattached to the current window; presentation must
   not hide or rehome that item. Opening is a separate lifecycle/native command.
2. Read the same metadata value for that canonical `ItemId` from every Space,
   window and Favorite alias. Supply the authoritative default display label
   and recompute the presentation after native title, model or metadata changes.
   A returned live page ID is a reference, not permission to focus or navigate.
3. Serialize edit planning, current-value validation and replacement through the
   existing coordinator. Apply a returned replacement to the same canonical
   item entry only. `ApplyLabelEdit` is not an atomic compare-and-swap, operation
   generation or native transaction. Value comparison cannot detect a change
   away from and back to an identical value. A future journal must settle
   transaction ordering and recovery; do not treat this result as a disk commit.

Saved URL editing requires an approved Model mutation API and its native failure
contract. This library does not mutate `Model::items()` or implement a competing
destination field. Removal, reset, order, activity and navigation commands stay
outside the label-edit interface.

## Acceptance coverage and open work

| Issue requirement | Boundary for this slice |
| --- | --- |
| Pin/unpin and reorder by keyboard, menu and drag | Existing `Model::Keep` is the semantic keep transition. Native input, unpin policy and UI remain open. Space folder order belongs to the organization component, not another pins order store. |
| Close retains the saved item; reset uses its saved destination | Existing model transitions demonstrate this only for its conservative quiescent subset. No native close/reset command or approved destructive policy is introduced. |
| Rename, saved URL edit and restore default survive restart and are undoable | Label editing can produce before/after values for a future journal. Durable storage, recovery, journal policy and saved URL mutation remain open. The Model has no saved URL editing command. |
| Same-site/outbound routing and reset preserving the prior page | Version 3 approves browser-classified outbound links branching through `PlanSavedPageLink`. Same-site routing and reset preservation remain open. No URL/site classifier is added here. |

## Native integration evidence

The [native integration design](../native-space-integration.md) and
[pinned source references](../native-space-source-references.md) describe the
required ownership extraction. Chromium's desktop window/Profile invariant and
Profile-local sessions prevent claiming native integration from a sidebar
projection. Helium's vertical sidebar is an eventual view reuse point; its
native pinned containers do not establish Arcified saved-item persistence.

Native navigation stays with the existing TabModel/WebContents/controller.
Presentation must resolve a `PageId` through the eventual native page registry
when handling commands. A label edit must not navigate, reset, close or transfer
that page. Native completion/cancellation must precede reporting an actual
lifecycle transition; an in-memory result is not native completion evidence.

Installed native checks must cover Linux, Windows and macOS with build identity,
keyboard/menu/drag operation, focus and accessibility, restart/crash recovery,
stale edits across windows, failure/cancellation and protected work. No prepared
Chromium tree, browser build, browsing data, provider access or native UI was used
for this slice. Those acceptance checks remain open.

## Focused verification

[`saved_item_test.cc`](../../native/features/pins/saved_item_test.cc) runs six
synthetic fixtures through public APIs:

| Fixture | Proven local behavior |
| --- | --- |
| `pins/canonical-presentation-and-distinct-items` | A temporary page with the same URL is not a saved-item match; equal-URL saved items have independent labels/live references; prior presentations remain owned snapshots. |
| `pins/alias-canonical-metadata` | A same-Profile alias uses the kept item's metadata even when its owning Space is unattached; the Model still refuses open until attachment. Standalone Profile-owned items also use this component. |
| `pins/reversible-label-values-and-conflicts` | Forward/reverse rename and restore values work; stale forward/reverse edits fail; restoring uses the latest caller default; no-op values are supported. |
| `pins/keep-transfer-close-reopen` | Kept identity retains its label across model transfer, conservative close and reopen; editing does not change saved destination, current navigation, Profile, activity, attachment or split references. |
| `pins/invalid-and-deleted-inputs` | Empty labels and missing/deleted targets refuse presentation/edit/reversal without recreating items. |
| `pins/owned-values` | Plans and rows own their values even when inputs change or IDs originate in Model getters. |

Run from the repository root with the existing toolchain:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/pins/saved_item.h \
  arcified/native/features/pins/saved_item.cc \
  arcified/native/features/pins/saved_item_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/pins/saved_item.cc \
  arcified/native/features/pins/saved_item_test.cc \
  -o /tmp/arcified-pins-saved-item-test
/tmp/arcified-pins-saved-item-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/pins/saved_item.cc \
  arcified/native/features/pins/saved_item_test.cc \
  -o /tmp/arcified-pins-saved-item-sanitized
/tmp/arcified-pins-saved-item-sanitized
```

Clang and clang-format 23.1.3 on Linux passed the strict compilation, six
fixtures and Chromium style check. The same six fixtures passed ASan/UBSan with
no sanitizer diagnostics. The unchanged lifecycle baseline also passed
its 16 fixtures before component implementation. These are standalone semantic
checks, not a Chromium build, native UI test or persistence test. Windows/macOS
compilation and installed native behavior remain unverified.
