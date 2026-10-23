# Command-bar session core

Issue [77](https://github.com/Timpan4/helium-arcified/issues/77). This component implements presentation/session interaction in standalone C++17. It does not implement a native browser command bar or establish acceptance on Linux, Windows or macOS.

The authoritative [lifecycle v3 contract](../../../docs/lifecycle-contract.md) owns pages, saved destinations, Profile/Space mapping and attachments. The [native ownership design](../../../docs/native-space-integration.md) and [pinned source references](../../../docs/native-space-source-references.md) identify unresolved production host and dispatch prerequisites. Their earlier v2 design does not override the approved v3 semantics. This session neither reads nor mutates the lifecycle model and has no current-navigation state, persistence, native binding, focus registry or URL classifier.

## Caller contract

1. Invoke with an explicit `RequestKey`, an `Origin` and initial query text. `Origin` captures the caller's opaque initiating `SurfaceToken` and lifecycle provenance. Reinvoking while open returns `kAlreadyOpen`, preserving that origin. The caller decides whether to seed an abandoned query. The bar stores no queries after dismissal and has no cross-Profile/private-mode retention policy.
2. After each accepted text change, call `SetQuery` with a fresh opaque `QueryToken`. This clears old rows and selection before accepting new results. Empty query text is supported. Native text editing, composition and encoding belong to the input surface; this component preserves the supplied string bytes without normalization, parsing, truncation or implicit completion.
3. Publish caller-ranked `Result` records with the exact active `RequestKey`. Wrong session or query keys return `kStaleQuery` without changing rows, query or selection. Results carry stable opaque IDs, titles, explicit caller-localized action labels and typed intents. Publishing an empty list is valid. Empty required identities, labels and payloads, or duplicate row IDs, reject the entire replacement. These are structural checks, not a validation of the lifecycle graph, URL safety, permissions or native liveness.
4. The first nonempty publication for a query selects its first row. Refreshes retain selection by result identity, including after reordering. A selected row disappearing clears selection. Its return does not silently select it again. `Select` or `MoveSelection` must explicitly choose a row; movement clamps at either end and chooses first/last from no selection for Next/Previous. Changing query starts initial selection again. The caller must give a different ID to a different action or target, even if labels/URLs match. A stable result ID is a caller assertion of the same action/target, not an identity inferred from display text.
5. `Activate` alone returns an owned `ActivationIntent` for the selected row and closes the session. Without a selection it returns `kNoSelection` and stays open. Invocation, typing, publication and selection produce no activation intent. `Escape` returns an owned `CancellationIntent`, including exact origin and abandoned text, and closes. Repeated activation or Escape cannot emit that action again. Native dispatch success/failure and actual focus restoration are outside this component.

`SessionToken` and `QueryToken` share this public non-reuse contract. The caller allocates opaque nonempty identities and must never reuse them while callbacks may survive, including after dismissal, repeated identical query text and replacement of the `CommandBar` object. Query tokens must identify distinct query requests across sessions. Tokens are compared for exact equality only. No string content, numeric sequence, lexical order or query text determines freshness. The bar rejects reusing the currently active query token but has no durable token allocator, retired-token registry or way to detect historical token reuse. Callers must cancel/reconcile stale native completion separately. Request keys protect result publication only; they are not native command authorization, transaction IDs or commit tokens.

Calls are serialized by the caller. Const views may be invalidated by the next mutation. Input IDs, keys, text and vectors are taken by value before mutation, including when borrowed from a current view. Returned activation and cancellation intents own their values.

## Action and provenance seam

`Result` is `{ResultId id, std::string title, std::string action_label, Action action}`. `KindOf(action)` gives the semantic kind. Native UI uses the explicit action label and selected row to announce/show what Enter will do. Labels alone do not prove accessible native UI.

`Context` contains an explicit `ProfileId`, `Attachment` with `WindowId` and `SpaceId`, and optional `PageId` and `ItemId` provenance. Origin context and action target context are distinct snapshots. No action redirects itself to whichever page/Space happens to be selected later.

| Action | Payload | Executor responsibility |
| --- | --- | --- |
| `NavigateAction` | `Context target`, inert destination text | Native URL parsing/security, target validation, permitted navigation kind and destination policy. Typed navigation is outside lifecycle v3's approved outbound-link planner. |
| `SearchAction` | `Context target`, query, optional opaque `SearchProviderKey` | Resolve approved native provider/site-search configuration, validate private-mode/privacy policy, create/dispatch any navigation only after explicit activation. This core does not construct a search URL. |
| `SwitchToTabAction` | Target `PageId`, immutable `ProfileId`, source `Attachment`, optional `ItemId` | Revalidate source identity and context, prepare target presentation/attachment and native completion. Source may differ from invocation's Profile, Space or window. Preserve both contexts. Temporary transfer is not authorized. |
| `ExecuteCommandAction` | `Context target`, opaque `CommandKey` | Resolve a supported native command, revalidate its scope and side-effect permissions. An emitted intent is not permission to bypass activity/destructive-action policies. |
| `OpenSavedItemAction` | `SavedItemTarget target` with existing `ItemId`, `ProfileId`, `ItemOwner` | Resolve current saved ownership, live page and destination attachment; coordinate native prepare/commit without duplicating ownership via generic URL navigation. `ItemOwner` is an owning `SpaceId` or standalone shortcut `ProfileId`. |
| `RevealInWorkspaceAction` | Variant of `SavedItemTarget` or `TemporaryPageTarget` with `PageId`, `ProfileId`, `Attachment` | Reveal an existing saved item or local temporary page through native presentation. No magic command key, new item, navigation copy or lifecycle transition is implied. |

Ranking and site-search remain separate components. They can supply ordered records or adapt their outputs without this core depending on their implementations. The coordinator's separate `PageCommandTarget` resolver is the sole shared lifecycle validator for page commands, with native binding/generation checks still required. This bar provides neither a competing resolver nor an executor.

## Native input and integration prerequisites

Session methods are semantic actions called after input arbitration, not raw key handlers. `features/input/` owns pure arbitration after native IME dispatch. It requires same-client continuity, before/after composition, candidate and pending facts, and positive nonownership. Unknown, active or stale input state defers to native input. Only explicit Enter, Escape, Up and Down are eligible; modifiers, repeats, dead/compose/process keys and edit events delegate. A false `IsCandidatePopupOpen` on Linux or macOS cannot establish inactivity. The native input path preserves borrowed UTF-16 unchanged. This component does not implement another IME arbiter or intercept text events.

The real host owns accelerators, pointer/keyboard presentation, focus restoration, screen-reader announcements, provider/command permissions, PageCommandTarget/native binding validation and failure/cancellation reconciliation. It must check the original surface is still valid before restoring it; an opaque surface token here is not a native object reference. It must revalidate both invocation context and selected target at dispatch, including immutable Profile and source/destination attachments. No model transition can stand in for actual browser completion. Activity, cleanup, destructive action and populated migration policies remain open.

No clipboard, page text, browsing database, provider, native service, network, account or global configuration is accessed by this component. Destination strings and synthetic command/provider keys remain inert. There are no new result caps, query lengths, retention periods, timers, retry budgets or cleanup policy.

## Focused verification

Run from the repository root with the existing local C++17 toolchain. Explicit Clang driver mode follows the working command in the lifecycle contract. No native preparation/build or download is needed.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/commands/command_bar.h \
  arcified/native/features/commands/command_bar.cc \
  arcified/native/features/commands/command_bar_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/commands/command_bar.cc \
  arcified/native/features/commands/command_bar_test.cc \
  -o /tmp/arcified-command-bar-test
/tmp/arcified-command-bar-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/features/commands/command_bar.cc \
  arcified/native/features/commands/command_bar_test.cc \
  -o /tmp/arcified-command-bar-sanitized
/tmp/arcified-command-bar-sanitized
```

The synthetic fixtures exercise invocation, exact Escape origin/query, explicit caller retention, stale session/query rejection, repeated text with fresh tokens, atomic invalid-input refusal, stable selection under reorder/removal, keyboard-selection semantics, owned typed activation for all action kinds, both saved ownership variants, both reveal variants, different invocation/target provenance, borrowed inputs, default provider/empty Space provenance and inert unclassified text. They assert only public session state and returned intents. They do not prove browser commands, URL safety, native effects, actual query editing or native focus restoration.

On 2026-10-23, against checkpoint `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe` plus these local files, both documented compiler/run commands passed with warnings treated as errors. The sanitizer run emitted no diagnostics. The documented Chromium formatting check passed. Separate `git diff --no-index --check /dev/null` checks for each owned file emitted no whitespace diagnostics; status 1 records the new-file differences. No other feature component was linked into these checks. The coordinator owns ranking-to-bar integration coverage.

Installed native keyboard/pointer interaction, IME and accessibility/visual checks, private browsing, trust-boundary behavior, dispatch/focus failure, persistence and Linux/Windows/macOS acceptance remain open. No native browser or visual check has run for this component. The deferred native host, input and executor must provide that evidence before issue closure.
