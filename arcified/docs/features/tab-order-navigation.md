# Sidebar-order navigation

Status: reusable standalone C++ selection component for [issue 100](../research/arc-dia/publication/tab-order-navigation.md). Native browser integration and Linux, Windows and macOS acceptance remain open. This component implements destination selection, not issue closure.

The implementation is [ordered_navigation.h](../../native/features/ordered_navigation/ordered_navigation.h), [ordered_navigation.cc](../../native/features/ordered_navigation/ordered_navigation.cc), and [focused fixtures](../../native/features/ordered_navigation/ordered_navigation_test.cc). It consumes the read-only [lifecycle model](../../native/model/lifecycle.h) under [contract v3](../lifecycle-contract.md). It does not own a tree, persistence, current navigation, selection, focus, native handles or an executor.

## Input and selection contract

The authoritative organization and presentation owners flatten the sidebar into `SidebarSnapshot`. `scope` identifies the currently selected attachment. `tabs` contains `Row` references in presentation order, including descendants of collapsed folders. `space_order` supplies the visible attached Spaces in their presentation order. Neither order comes from the model's map keys, MRU, URL, title or activity. The component accepts a caller-selected subset; it validates the supplied references but cannot prove the caller enumerated every organization row.

| Input | Meaning |
| --- | --- |
| `SnapshotRevision revision` | Nonempty opaque caller revision. Never reuse it. Change it when relevant lifecycle, order, selected/focused context, organization, expansion or layout changes. Publish the snapshot coherently with the model on the serialized owner sequence. This component does not allocate or store revisions. |
| `Attachment scope` | Current window and selected Space, checked against the model. Queries refuse an unattached scope or a different selected Space. Initial selection with no selected Space requires the external coordinator. |
| `Row::target` | `ItemId`, `ShortcutId` or `PageId`. An item must be visible in this Space/Profile. An alias must be visible to this Profile, and its target's owning Space must be attached to the requesting window. A page reference must be local to this exact attachment. A saved `PageId` row additionally requires a valid split reference; ordinary saved rows use item/alias identities. |
| `collapsed_ancestor_ids` | Opaque organization references, outer to inner. They request destination-row reveal, without copying folder membership or expansion state. Empty and repeated references are rejected. The organization owner supplies actual ancestry and existence. |
| Optional `SplitId split` | Identity-only hint identifying the target's live page in a layout in this attachment. Missing layouts, absent membership and nonlocal pages are rejected. It contains no geometry or selected pane state. |
| `space_order` | Unique Space references attached to this window. Different Profiles and multiple Spaces sharing one Profile are permitted. Hidden/unlisted Spaces cannot be selected through this projection. |

`SelectTab` accepts `RelativeTab`, `DirectTab`, or zero-based `DirectIndex`. Relative selection uses the caller's current row reference as its anchor and moves exactly one row in the supplied order. It requires explicit `kWrap` or `kStop` edge behavior. Collapsed folder descendants remain stops. A split group is projected as its presented pane references, each a stop at the position supplied by the caller. A group container is not an additional stop. The component does not choose pane capacity or split geometry.

Duplicate target references are rejected. An item and an explicit shortcut alias remain distinct presentation references even when they resolve to the same saved page. The organization/presentation owner decides which references to expose. There is no MRU list, recent-tab capacity, direct-selection capacity, automatic cleanup or closing fallback.

`SelectSpace` accepts `RelativeSpace`, `DirectSpace`, or `DirectIndex`. Relative Space selection anchors at the model's selected Space. Direct selection uses the supplied visible order. A missing anchor is refused rather than guessed. Empty orders, out-of-bounds indices and stopped edges have distinct refusals. The same flat snapshot is validated before either kind of selection; stale tab rows require republishing even for a Space request.

## Destination and freshness

`DestinationPlan` owns its revision, input scope and destination metadata. A `TabDestination` carries the original row for reveal, resolved activation attachment/Profile, optional saved item, optional existing live page, and that page's observed source attachment. A `SpaceDestination` carries only destination attachment/Profile. No payload copies a URL or current navigation.

A kept item's destination remains its owning Space. An alias can therefore reveal a row in the originating sidebar while activation requires a different attached Space in the same Profile. Standalone Profile items resolve into the requesting attachment. A saved page already live in another window retains the exact `PageId` and source attachment in the plan. Planning never transfers it, opens a second instance, resets it, changes activity or touches its navigation. Temporary pages never enter a cross-window destination.

`Revalidate` requires the same caller revision and scope, then resolves the target again and compares destination, Profile, live identity/source attachment and reveal metadata. Changed revisions always refuse the old plan, even when its target remains present. A closed temporary page, removed split, changed source attachment or changed reveal metadata is also refused if a caller accidentally retained a revision. After reorder/close, republish and rerun selection with the current anchor. A removed anchor yields `kAnchorMissing`; it does not silently select the row now occupying the old index.

Revision correctness remains the caller's obligation. Revalidation cannot detect native replacement, focus changes omitted from the revision, reorder omitted from the revision, or semantic movement away and back to the same attachment. It is not a reservation, native operation generation, successful focus report or security authorization.

## Integration seams

1. Organization supplies authoritative Space-kept hierarchy, item order and ancestry. Layout supplies the flattened presentation and split references. Adapt those inputs to `SidebarSnapshot`; do not persist this projection as a second tree. Tabs supplies labels. No labels or folder mutation are needed here.
2. The existing command system translates native platform accelerators and menu actions to these semantic requests. Preserve the separate recent-switcher path. The component assigns no modifiers, native command IDs or accessible strings, and does not mutate shortcut preferences.
3. Before acting, the coordinator obtains a fresh coherent projection and calls `Revalidate`. For an existing live page it consumes the commands peer's `arcified::commands::ResolvePageCommandTarget(model, page, expected_attachment)` and `RevalidatePageCommandTarget(model, target)`, using the observed source attachment until an actual transfer succeeds. The commands peer owns `PageCommandTarget`; this component neither copies that type nor supplies a competing public command-target resolver. Root integration adds the actual header/link dependency after combining branches.
4. The native executor validates real TabModel/WebContents binding, immutable Profile, attachment and operation generation immediately before effects on the serialized UI sequence. For an already-live saved item it must transfer the existing page under v3, preserving state, rather than navigate or clone. For a closed item it must coordinate native open success before reporting the corresponding lifecycle transition. The current model does not supply a native transaction. Native failure/cancellation must not publish a selection success.
5. After native activation succeeds, layout reveals the intended destination, using organization ancestry and refreshed destination-Space metadata when an alias changes Spaces. The focus owner publishes actual page/focus state. Accessibility publishes the actual visible destination and completed outcome from current labels, never an optimistic announcement from the plan. No reveal metadata constitutes page selection or focus completion.

The accepted commands header was inspected read-only in the sibling commands worktree. This feature's separate build intentionally depends only on `lifecycle.cc`. The organization, layout, tabs, commands and accessibility adapters remain integration work for the coordinator.

## Source evidence and constraints

The issue brief and matching `tab-order-navigation` entry in [backlog.json](../research/arc-dia/backlog.json) require sidebar-order traversal distinct from recent switching, collapsed-folder/split behavior, direct Space/tab selection and coherent visible/focused destinations after reorder/close. This standalone slice supplies the pure selection portion.

Local [tab-cycling-mru.patch](../../../patches/helium/core/tab-cycling-mru.patch) was read. It routes `SelectNextTab`/`SelectPreviousTab` through `SelectRelativeTabCycle`, which reads the Profile preference and chooses MRU or relative order. Wiring these new commands blindly through that preference-sensitive path would erase the required distinction. Reuse the native command infrastructure while preserving separate semantic intents.

Local [custom-keyboard-shortcuts-wiring.patch](../../../patches/helium/ui/custom-keyboard-shortcuts-wiring.patch) was read. BrowserView caches a Profile shortcut service, registers accelerators with its focus manager, and updates macOS menu accelerators on activation. The eventual adapter must preserve platform modifiers and revalidate Profile-specific routing across presentation changes. No new global shortcut map is introduced here.

The read [native integration design](../native-space-integration.md) and [source references](../native-space-source-references.md) predate v3. Their pinned Chromium commit `a49e2e3d4addfd84e765b52afe230bedb81096db` documents immutable desktop Browser/Profile ownership, native page handoff constraints and Profile-bound command/chrome coupling. Those documents are evidence of integration gates, not an available same-window multi-Profile API. This slice follows v3 where the documents still describe v2 or previously open mapping policy. Same-window multi-Profile ownership extraction remains unproven. No Chromium checkout, preparation, build, new upstream fetch or native interaction was performed for this component.

Activity, cleanup, destructive actions and populated cross-Profile migration policies remain open under v3. The selector neither decides nor invokes them. It adds no AI, account, sync, real browsing data or external service access.

## Focused checks

Run from the repository root with the existing toolchain:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/ordered_navigation/ordered_navigation.h \
  arcified/native/features/ordered_navigation/ordered_navigation.cc \
  arcified/native/features/ordered_navigation/ordered_navigation_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/ordered_navigation/ordered_navigation.cc \
  arcified/native/features/ordered_navigation/ordered_navigation_test.cc \
  -o /tmp/arcified-ordered-navigation-test
/tmp/arcified-ordered-navigation-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/ordered_navigation/ordered_navigation.cc \
  arcified/native/features/ordered_navigation/ordered_navigation_test.cc \
  -o /tmp/arcified-ordered-navigation-sanitized
/tmp/arcified-ordered-navigation-sanitized
```

Both warning-clean C++17 builds and both fixture executions passed on the Linux host. The sanitizer run emitted no diagnostics. The Chromium-style format dry-run passed. Fixtures use synthetic IDs and reserved `.test` destinations only.

| Fixture | Behavior covered |
| --- | --- |
| `SidebarOrderAndReveal` | Caller order differs from ID order; next/previous/direct selection; nested collapsed ancestor metadata; no page creation or Space selection. |
| `BoundariesAndEmptyOrder` | Explicit stop/wrap, singleton wrap, empty order, missing direct target and out-of-bounds index. |
| `SavedIdentityAndAliases` | Same existing cross-window saved identity/source; protected activity/current navigation unchanged; alias owning-Space resolution and missing destination attachment refusal. |
| `VisibilityAndProjectionValidation` | Profile/local-Space visibility, local temporaries, invalid references, duplicate rows/ancestors, empty revision, shared Profile item and stale selected Space. |
| `ReorderAndCloseFreshness` | Old plan refused after reorder; fresh next uses new order; closed temporary invalidates plan; removed anchor refused and fresh direct index resolves remaining order. |
| `SplitIdentityAndStaleMetadata` | Saved/temporary split-pane traversal; removed split refusal; saved page reference without split refused; transfer/source attachment and reveal changes invalidate a plan. |
| `SpaceOrderAndIsolation` | Caller Space order, direct/relative selection, cross-Profile destination without model selection or page reprofile, duplicate/hidden/empty Space order and stale Space plan. |

These checks exercise the feature component against the actual in-memory v3 model. They are not native browser, GN build, persistence, keyboard/focus or accessibility acceptance tests.

| Target | Native acceptance still open |
| --- | --- |
| Linux | Installed native sidebar interaction, platform accelerators, destination reveal, focus/active-page/accessibility agreement, reorder/close races and state-preserving saved-page transfer. |
| Windows | Native compilation and the same installed interaction cases, including platform modifiers and Profile isolation. |
| macOS | Native compilation and the same installed interaction cases, including menu accelerators and Profile isolation. |

The native ownership route, common command adapter and final focus/accessibility publication must exist before these platform scenarios can be proven. No issue closure, commit, push or PR is included in this work.
