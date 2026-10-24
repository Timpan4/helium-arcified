# Organization multi-select

Status: standalone C++ preparation for [issue 68](research/arc-dia/publication/multi-select.md), based on checkpoint `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. Native acceptance remains open on Linux, Windows and macOS. No issue closure, browser integration, persistence, undo or batch actuation is claimed.

[multi_select.h](../native/features/organization/multi_select.h) and [multi_select.cc](../native/features/organization/multi_select.cc) implement transient selection and non-mutating move previews. They reuse the canonical folder IDs and validated read API in [folders.h](../native/features/organization/folders.h). The [lifecycle contract](lifecycle-contract.md) remains authoritative for ownership and live pages. No lifecycle code changes are needed.

## Selection behavior

The caller provides unique, nonempty row identities in its authoritative current view order. Row identities are a variant of `organization::FolderId`, `lifecycle::ItemId` and `lifecycle::TemporaryTabId`. Equal text in different identity types does not make rows equal. Selection stores only a selected identity set and an optional transient range anchor. It stores no order, active page, focus, navigation or persisted state.

| Gesture | Result |
| --- | --- |
| Replace | Select only the target and establish the range anchor. |
| Toggle | Add or remove the target, retain other selections and establish the range anchor, including when the target is deselected. |
| Range | Replace selection with the inclusive range between anchor and target in the supplied order; retain the anchor. |
| Additive range | Union the inclusive range with the prior selection; retain the anchor. |
| Range without an anchor | Use the target as the anchor and select it. Additive range retains the prior selected identities. |

These are semantic gestures. A future native adapter maps the platform's keyboard modifiers and commands, including Command on macOS and Control on Linux/Windows, without storing a second focus state. Keyboard, menu and drag entry points can use the same selection and planning API. Native bindings and platform conventions have not been exercised.

`ApplySelection` returns an owned selection or an explicit error, leaving inputs unchanged. It resolves range endpoints by identity after reordering, never by cached index. Missing selected identities or an absent anchor identity return `kStaleSelection`. The caller explicitly uses `ReconcileSelection` after a view change to retain only represented selections and clear an absent anchor. Reconciliation never chooses another active page or transfers focus. A collapsed or filtered view can therefore drop hidden selections explicitly, rather than silently applying them to different rows.

`ProjectSelectedRows` returns every current row in supplied order with its selected boolean, including false after deselection. This is an accessibility projection, not a native accessibility implementation. Roles, focus, multiselectable container properties, events and announcements remain the native adapter's responsibility.

## Stable move preview

`PlanSelectionMove` reads `Folders::Snapshot(model, source_space)` for each request and propagates its lifecycle-reference validation errors. It uses the hierarchy's placement and ancestry queries rather than maintaining another tree. Represented rows must follow canonical hierarchy preorder. Rows omitted by a collapsed or filtered view are allowed. Unplaced saved items follow represented rows, in the caller's canonical order from the same lifecycle view. No lifecycle map ordering is interpreted as user order. Temporaries remain caller-ordered for selection and are never inserted into the folder hierarchy.

The planner returns source root references in canonical relative order, a stable destination `Placement`, and the proposed direct destination children. It suppresses a selected descendant when an ancestor folder is selected. The ancestor moves as a reference with its subtree; the preview neither copies descendants nor duplicates live pages. Unplaced saved items can be previewed into a folder or the Space root. Existing destination siblings retain their relative order.

The destination uses a folder or Space root and an optional existing direct sibling ID to insert before. If that sibling is itself a moving root, the planner advances to the next unaffected sibling in the original destination order, or appends when none remains. The returned placement shows the resolved anchor. Dropping a selection into itself or any selected folder's descendant is refused. A before anchor from another container or a missing folder/Space is refused, rather than interpreted as an index or fallback destination.

The preview covers same-Space organization metadata only. A saved item can change folder membership/order while its existing live page remains protected and attached exactly where it was. This is not an item ownership move or the approved saved-page handoff. It changes no lifecycle owner, attachment, current navigation, saved destination, activity, Profile or selection.

| Request | Outcome |
| --- | --- |
| Same-Space saved items/folders | Preview their stable organization placement, including live saved items. |
| Temporary-only or mixed saved/temporary selection | Refuse the entire plan with `kTemporaryMoveUnsupported`. Never keep, transfer, skip or clone temporary tabs implicitly. |
| Different-Profile destination | Refuse with `kCrossProfileMigrationUnsupported`; no reopen/copy/source-disposition policy is inferred. |
| Different Space in the same Profile, containing a live saved item | Refuse with `kLiveItemOwnershipMoveUnsupported`, including live descendants of selected folders. |
| Other cross-Space request | Refuse with `kCrossSpaceUnsupported`; no coordinated lifecycle/hierarchy commit is implemented. |
| Stale identity, hierarchy order contradiction, invalid destination or cyclic placement | Refuse the entire plan without changing source or destination. |

## Cancellation and stale snapshots

A plan owns only snapshot references and destination order. It is neither a reservation nor a commit token, and it carries no actuation guarantee. Cancellation is discarding the preview. Planning and every refusal leave the source unchanged because both lifecycle and folder inputs are const and no mutations or native commands are issued.

Before any future commit, the caller must re-read lifecycle identity/ownership, canonical source order, hierarchy, selected ancestors and the destination's direct sibling anchor. Replanning after a sibling moves or an item changes owner can return a different plan or an error. An old plan remains an old snapshot. There is deliberately no executor, mutation transaction, or loop over single-entry mutations. Native batching must establish a coordinated failure/cancellation boundary before actuation; this component cannot promise rollback or success for a partially applied batch.

## Focused synthetic verification

[multi_select_test.cc](../native/features/organization/multi_select_test.cc) uses typed synthetic IDs and reserved `.test` destinations. It covers:

1. Inclusive forward/reverse ranges, range shrinking, additive ranges, toggling, replace and absent-anchor behavior across folder, saved and temporary row identities.
2. Identity-preserving reorder, explicit reconciliation on hidden/deleted rows, selected accessibility projection and false state after deselection.
3. Duplicate/empty/missing row refusals and distinct identity types with equal text.
4. Stable destination previews, selected folder/descendant deduplication, same-Space live-item metadata placement, self-anchor resolution and source immutability when a preview is discarded.
5. Unplaced-item caller order, canonical represented-order validation and valid filtered/collapsed source views.
6. Whole-selection refusal for temporary/mixed selection, cross-Space/cross-Profile moves, live ownership moves, missing destinations and cycles.
7. Stale destination/row-order rejection after a folder mutation and lifecycle-owner invalidation surfaced through a fresh snapshot. Retained plans remain unchanged snapshots.

Run from the repository root using the existing C++17 compiler and Chromium formatter style:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/organization/multi_select.h \
  arcified/native/features/organization/multi_select.cc \
  arcified/native/features/organization/multi_select_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/organization/folders.cc \
  arcified/native/features/organization/multi_select.cc \
  arcified/native/features/organization/multi_select_test.cc \
  -o /tmp/arcified-organization-multi-select-test
/tmp/arcified-organization-multi-select-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/organization/folders.cc \
  arcified/native/features/organization/multi_select.cc \
  arcified/native/features/organization/multi_select_test.cc \
  -o /tmp/arcified-organization-multi-select-sanitized
/tmp/arcified-organization-multi-select-sanitized
```

These checks cover standalone C++ semantics, not an installed browser or native acceptance. Native pointer/keyboard/menu/drag interactions, visual destination preview, focus routing, accessibility events, cancellation during actual actuation, persistence/failure recovery and Linux/Windows/macOS runtime evidence remain open. The [native integration design](native-space-integration.md) and [source references](native-space-source-references.md) describe the unproven host/Profile and live-page handoff boundaries. This work adds no patch, build target, dependency or second native tab-selection system. Activity, cleanup, destructive actions and populated cross-Profile migration remain owner decisions.
