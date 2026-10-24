# Temporary tab command identity

Issue: [49](https://github.com/Timpan4/helium-arcified/issues/49). Contract: [lifecycle v3](../lifecycle-contract.md). Native acceptance remains open.

## Ownership and integration

The lifecycle model owns `PageId`, `TemporaryTabId`, immutable `ProfileId` and the original `(WindowId, SpaceId)` attachment. A temporary page has no saved destination. `Model::TemporaryPages` supplies membership, not presentation order. Keeping a temporary page replaces its owner with `SavedPage` while retaining `PageId`, navigation and activity. Temporary pages cannot use `TransferSavedPage`.

The native adapter must resolve commands from explicit page identity and attachment. It must not resolve by URL, title, tab index, last active browser or the currently selected Profile. A pointer action supplies its row's identity. A keyboard action receives the authoritative native active `PageId` from its caller. Both use the same command dispatch path. The host supplies native strip order and active page; this feature adds no order, focus or selected-page store. Lifecycle `selected_space` remains authoritative for logical Space selection.

## Reusable identity component

[`temporary_tabs.h`](../../native/features/temporary_tabs/temporary_tabs.h) and its implementation provide pure reads over `const lifecycle::Model`:

- `ResolveTemporaryIdentity(model, page)` returns its `TemporaryTabId` or an explicit missing/non-temporary refusal. It does not resolve attachment or Profile context.
- `ValidateTemporaryIdentity(model, page, expected_tab)` checks a delayed temporary-specific intent. Keep and close invalidate that intent. A navigation or activity change leaves temporary identity intact. Success is not a native command authorization or generation check.
- `ProjectTemporaryIdentities(model, attachment, native_order)` returns owned `{page, tab}` rows in exactly the supplied order. The caller must first filter the native strip to temporary pages in that attachment. Saved pages are rejected, rather than silently filtered inside this API. Every supplied page must exist, be temporary, belong to the exact attachment and appear once. The projection also checks complete coverage against model membership, rejecting omitted local temporary pages. An empty list succeeds only for an attached scope with no temporaries. A refusal returns no partial rows, changes no state and supplies no fallback order.

Snapshots can become stale immediately after return. Callers serialize model mutations/native reports on the UI sequence and rebuild a rejected projection from a reconciled native/model snapshot. Replacing a rejected list with map order would lose the native order guarantee. The component does not prove that caller input actually came from a native strip.

At root integration, compose temporary intent validation with the navigation workstream's shared `arcified/native/features/commands/command_target.h` API: `arcified::commands::PageCommandTarget{page, profile, attachment}`, `ResolvePageCommandTarget(model, focused_page, expected_attachment)` and `RevalidatePageCommandTarget(model, target)`. That common component owns general command target validation. Profiles owns validation against the real native context. This worktree has no dependency on a missing shared header and provides no replacement command facade. Integration of those separately owned sources is still required. A page kept since a temporary row was captured must fail `ValidateTemporaryIdentity` even if its shared page command target is still valid as a saved page.

## Native actuation boundary

The [native integration design](../native-space-integration.md#commands-and-native-completion) requires native success/cancellation to be reconciled before reporting semantic transitions. The [source references](../native-space-source-references.md#c2-view-widget-and-command-coupling) establish that stock Back and New Tab dispatch through a particular browser, whose controllers and chrome capture a Profile. The [native page identity evidence](../native-space-source-references.md#c3-live-identity-and-handoff) places navigation in WebContents and its controller. A resolved lifecycle identity alone does not prove that a legacy Browser command addresses the intended page.

1. Resolve the logical page and attachment, then resolve its live native registry entry and exact Profile context on the browser UI sequence. Revalidate immediately before native dispatch. A stale row must fail visibly rather than target another tab.
2. Dispatch Back, Forward, Reload and Stop through the resolved native page/controller. Native availability, navigation history, loading state, URL policy, modifier dispositions, opener semantics and before-unload remain native responsibilities. Record actual committed navigation with `Model::RecordNavigation` on its originating `PageId`, including when its Space is no longer selected.
3. For creation, validate native URL policy and allocate fresh, never-reused IDs through the workspace allocator. Create under the resolved attachment's actual Profile. Only confirmed creation/commit can be reported using `Model::OpenTemporary`. Failure or cancellation records no model page. Coordinate native success if model reporting fails; do not drop the native page or pretend rollback succeeded.
4. Duplication must preserve the native duplication semantics and create a distinct local identity in the same attachment and Profile. A URL-only copy cannot prove history duplication. Reopen needs the approved native session/undo policy and fresh identity reconciliation. Neither operation may recreate another window's temporary page by URL matching.
5. Close must retain native protection and cancellation handling. The model's current conservative subset refuses unknown/protected activity. Do not mark activity quiescent merely to get past that refusal. Manual destructive behavior, recovery and native/model reconciliation remain open decisions and implementation gates.

Middle-click and modifier-click require browser-provided input disposition. This feature must not invent platform key mappings or override approved saved-page outbound-link routing. Closed-tab records, restart restore and automatic cleanup are not provided here.

## Acceptance coverage

| Issue requirement | Local slice | Remaining evidence |
| --- | --- | --- |
| Open, duplicate, close, reopen preserve order and sensible focus | Complete temporary identity projection preserves supplied native order and refuses stale/incomplete input; existing lifecycle transitions remain authoritative. | Native order/active-page adapter, focus integration, native duplicate/close cancellation, reopen and restore policy. |
| Back/forward, reload, stop, middle/modifier clicks target the correct live tab | Temporary intent identity validation; shared-command composition and native dispatch boundary documented. | Root integration, native registry and command wiring, browser availability, input dispositions, actual history/loading behavior. |
| Window-local tabs retain intended Space/Profile and protected work | Projection rejects foreign attachments. Synthetic lifecycle fixtures retain local identities and immutable Profile through logical Space selection; unknown/protected work blocks close. | Installed native switching, forms/media/download protection, same-window Profile isolation and failure reconciliation. |

## Focused checks

Run from the repository root using the existing C++17 toolchain:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/temporary_tabs/temporary_tabs.h \
  arcified/native/features/temporary_tabs/temporary_tabs.cc \
  arcified/native/features/temporary_tabs/temporary_tabs_test.cc
check_output_dir=$(mktemp -d)
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/temporary_tabs/temporary_tabs.cc \
  arcified/native/features/temporary_tabs/temporary_tabs_test.cc \
  -o "$check_output_dir/temporary-tabs-test"
"$check_output_dir/temporary-tabs-test"
```

Five standalone fixtures cover complete supplied order, empty scope, duplicate/missing/stale/saved/foreign input, temporary identity expiration after Keep/close, navigation/activity identity retention, equal-URL window isolation, logical cross-Profile Space selection, rejected temporary transfer and protected/unknown close refusals. They create only synthetic IDs and `.test` destinations. These are model/component checks, not native duplication, focus, browser command or data-protection evidence.

Local result on 2026-10-23: Clang 23.1.3 compiled and ran all five fixtures with the strict flags above. The same fixtures passed a separate build with `-g -fsanitize=address,undefined -fno-omit-frame-pointer`, without sanitizer diagnostics. `clang-format` 23.1.3 accepted all three C++ files using the command above. Per-file `git diff --no-index --check /dev/null` checks emitted no whitespace diagnostics for those files and this document; status 1 denotes each new-file difference. The existing model files were linked, not changed. No Markdown formatter/linter is declared for this component.

Installed native checks remain open on Linux, Windows and macOS. No Chromium preparation, build, browser interaction, persistence, accessibility or visual verification has run for this slice. Passing standalone tests cannot close issue 49.
