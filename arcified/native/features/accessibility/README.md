# Sidebar accessibility semantics

This directory implements a standalone C++17 semantic planner for [issue 149](https://github.com/Timpan4/helium-arcified/issues/149). It derives sidebar hierarchy metadata, tree keyboard intents, advertised command requests, focus suggestions and semantic changes from caller-supplied facts. It does not implement a Chromium sidebar or complete the issue's installed-build acceptance.

The coordinator accepted this stateless boundary during the accessibility workstream. The organizer/native host supplies the complete labeled visible preorder directly. Layout supplies visibility and geometry only. No layout row model, lifecycle resolver, native focus registry, focus application or announcement service is added here. Native page validation remains the shared commands area's `PageCommandTarget` resolver.

## Borrowed projection

`Projection::Borrow(rows)` validates the boundary and returns `nullopt` for malformed input. An empty projection is valid. The host owns all rows, names, parent references, selection, expansion, command labels and enabled facts. The helper retains a pointer to that input and derived lookup/position metadata only. It owns no folder, ordering, selection, navigation, persistence or lifecycle state.

1. Supply every visible logical row in preorder, including offscreen rows when the native list is virtualized. Omit descendants of collapsed nodes. This is not a viewport-only or partially loaded projection. Root rows have no parent. Each other row's parent must be an expanded ancestor in the current preorder. A subtree cannot resume after a sibling. Parent references are accessibility hierarchy, not lifecycle ownership.
2. Give rows unique, nonempty presentation IDs and nonempty localized UTF-16 names. Duplicate display names are valid. The host binds these IDs to authoritative entities in the correct native context. Names, URLs, indices and geometry cannot supply that binding. These presentation IDs are not `PageId`, `ItemId`, `SpaceId` or a new persistent identity allocator.
3. Supply optional `selected` independently of focus. Absence means nonselectable. Supply optional `expanded` only for expandable rows. Absence means a leaf, including an empty folder without expandable children. Tabs must be leaves. Multiple selected facts are accepted because the helper cannot determine the host's selection model; the host must publish matching tree selection semantics.
4. Supply only currently advertised commands, with nonempty IDs and localized labels. Command IDs must be unique within a row; the same command can appear on different rows. Disabled commands remain available for menu description. Reorder, move and keep/pin command meaning, destination choice and permission remain authoritative caller concerns.
5. Keep the rows, command vectors and backing name strings alive and unchanged for the entire borrow. Discard the projection before mutating any of them, then borrow again. The rvalue vector overloads are deleted. Returned `RowSemantics` points into this same borrow. When comparing changes, both old and new caller-owned presentations and their label storage must remain immutable through the call. This module does not capture or retain snapshots.

Validation cannot establish completeness, native identity validity, the selected context, or whether facts are still current. A missing parent, collapsed parent with visible children, cycle, duplicate ID, tab with expansion state, missing name or malformed command fails the whole borrow. It does not silently filter, rename, reorder or repair input. The `nullopt` result deliberately supplies no partially trusted projection; callers should surface their own diagnostic and keep their last successfully applied native presentation rather than publish an empty successful tree.

## Semantics and host integration

Every described row is a tree item under a host-labeled vertical tree. `RowSemantics` exposes the borrowed row and one-based `level`, `position` and `set_size`. Sibling counts use the complete visible logical projection, not the rendered viewport. The host maps these facts, localized name, optional selected/expanded state and content kind to native AX nodes. Do not expose expanded/collapsed state on leaves. Command descriptors become menu items in a host-labeled menu. Native focus, disabled state, containment, group nodes, action handlers and any native integer-width conversion belong to that adapter.

`OnKey` plans Up, Down, Home, End, Left and Right. It returns a focus suggestion, expansion request or no action. It never selects or changes supplied facts. Left/Right use native text direction, following Chromium's RTL behavior, rather than the sidebar's screen edge. Expansion requires the host to apply its organization change and supply a fresh projection after success. Unknown focus and edge keys produce no action. Activation, typeahead, modifiers, IME handling and application shortcuts remain host/input responsibilities.

`RequestCommand` is the shared gate for a menu, keyboard or assistive command request against this presentation. It returns an owned row/command ID pair only when that row advertises the enabled command. It does not resolve a page, execute a command or authorize a lifecycle transition. Before native dispatch, revalidate the active native context, row binding and capability, then use `PageCommandTarget` where the command targets a page. Use the same authoritative native command path for pointer, keyboard and accessibility activation. A command disappearing or becoming disabled refuses a fresh request; earlier requests grant no reservation.

`SuggestFocusAfterChange` takes the actual old/new presentations and the host's current focus identity. It retains a surviving focused row. If that row disappeared, it suggests a surviving next old sibling, previous old sibling or parent, repeating up the old ancestry for a removed subtree. The host must validate the candidate in its current focus registry and may reject it when native focus/context changed. Focus outside the tree stays outside. Unknown old focus with no current match, an empty tree or no surviving candidate returns no suggestion. The host chooses its page/sidebar-toggle/return fallback. This is a local semantic suggestion order, not destructive-action policy, native focus routing or a promise that a row's page survived. A saved row can remain after its live page closes.

Undo and passive reveal do not take focus from a surviving row or from the page. The host owns any explicit user request to focus a revealed/restored target. Before/after comparisons are ambiguous if sourced from different windows/Spaces, if IDs were reused, or if the underlying presentation changed during the call. The helper cannot detect those cases. Scope and serialize the call at the native host, keep IDs stable, and revalidate suggestions before applying them. No asynchronous callback protocol is invented here.

`ChangesSince` identifies added/removed rows and changes to name, kind, hierarchy, selected/expanded facts or command descriptors. Reorder updates sibling metadata; changing the number of siblings updates surviving siblings' metadata. Added rows require the host to publish their complete semantics. These results are facts, not ordered native events, spoken text, success announcements or focus events. The host publishes/coalesces platform events only after actual native success. Failed or cancelled actions supply no successful change.

## Source evidence

The publication brief and `backlog.json` entry require accessible names/states/hierarchy, non-drag operations and coherent focus. [Lifecycle contract v3](../../../docs/lifecycle-contract.md) remains authoritative. In particular, folder membership is presentation, saved rows need not disappear on page close, and destructive/undo policy remains open. The [native integration plan](../../../docs/native-space-integration.md) and [source references](../../../docs/native-space-source-references.md) establish ownership constraints only. Their reviewed v2 design predates the v3 approvals and is not adopted as current policy.

Pinned Chromium source was read at `a49e2e3d4addfd84e765b52afe230bedb81096db`, the repository's `155.0.8059.79` engine pin, via Gitiles file HTTP without a source checkout or build:

- [TreeView, lines 419 to 424](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/views/controls/tree/tree_view.cc#419) sets tree/vertical semantics. [Lines 959 to 995](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/views/controls/tree/tree_view.cc#959) derive hierarchy, position and sibling count from the native model. [Lines 1042 to 1067](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/views/controls/tree/tree_view.cc#1042) establish tree-item, naming and selected-state primitives.
- [TreeView, lines 765 to 785](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/views/controls/tree/tree_view.cc#765) reverses Left/Right expansion behavior in RTL. This helper borrows that direction convention, while retaining independent caller-supplied selection.
- [Helium toolbar-actions.patch, lines 497 to 503](../../../../patches/helium/ui/layout/toolbar-actions.patch#L497) updates the sidebar collapse button's localized accessible name with its expand/collapse state. Preserve this native control as a host focus fallback rather than adding a competing focus owner.
- The [WAI tree pattern](https://www.w3.org/WAI/ARIA/apg/patterns/treeview/) supplies reference keyboard and tree semantics. It is web guidance, not proof of Chromium native platform behavior. This helper's key and focus/selection separation follows that pattern; its concrete native mapping still requires the platform adapter. Removal suggestion order is a documented local proposal, not a claim that Arc, WAI or Chromium prescribe that exact policy.

## Focused checks

Run from the repository root with the existing tools. The explicit Clang driver mode follows the lifecycle check commands for this T3 host. No new dependency, GN target, patch or build-system integration is introduced.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/accessibility/sidebar_accessibility.h \
  arcified/native/features/accessibility/sidebar_accessibility.cc \
  arcified/native/features/accessibility/sidebar_accessibility_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/accessibility/sidebar_accessibility.cc \
  arcified/native/features/accessibility/sidebar_accessibility_test.cc \
  -o /tmp/arcified-sidebar-accessibility-test
/tmp/arcified-sidebar-accessibility-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/features/accessibility/sidebar_accessibility.cc \
  arcified/native/features/accessibility/sidebar_accessibility_test.cc \
  -o /tmp/arcified-sidebar-accessibility-sanitized
/tmp/arcified-sidebar-accessibility-sanitized
git diff --check -- arcified/native/features/accessibility/
```

The files are new and untracked before coordinator review. Each also passed `git diff --no-index --check /dev/null <path>` using its exact path in this directory. That check returns 1 for a new-file difference; no whitespace diagnostic was emitted. The tracked-only `git diff --check` cannot check these untracked additions by itself.

The executable checks synthetic hierarchy and command fixtures only. It covers nested hierarchy/sibling metadata, duplicate names, malformed projections, LTR/RTL keyboard intents, collapsed branches, selection independence, enabled-command gating and stale command rejection, owned request IDs, sibling/ancestor focus suggestions, empty/stale/outside focus, undo/reveal retention and change metadata. Both compiler execution and AddressSanitizer/UndefinedBehaviorSanitizer passed on the local Linux host. These are not native browser tests.

## Native gaps

Issue 149 stays open. No installed native build, UI interaction, visual check, AT-SPI/Orca, UIA/Narrator, macOS AX/VoiceOver, forced contrast or reduced-motion check ran. Linux, Windows and macOS installed-build acceptance all remain open, as do platform compiler checks outside this Linux host. The helper does not draw or animate anything, so it cannot prove contrast, focus-ring visibility or motion preferences.

The organizer/folder projection, real command menus and keyboard bindings, AX node/event adapter, host focus registry/application, explicit undo/reveal focus requests, selection model, cross-context stale-event reconciliation and native failure/cancellation integration remain host/adjacent-area work. Non-drag move/reorder/pin execution has not run in a browser. No new lifecycle, destructive-action, migration, persistence, page-transfer or same-window multi-Profile implementation is claimed. Those native dependencies remain unproven.
