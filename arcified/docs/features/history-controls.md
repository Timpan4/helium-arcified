# History search and controls

Status: local read-only component and an unwired Chromium query slice for [issue 102](https://github.com/Timpan4/helium-arcified/issues/102). The standalone component passes synthetic checks. The Chromium entry point has not been compiled or run. This work does not close the issue or enable deletion, clearing, or automatic cleanup. Destructive action policy remains OPEN.

## Implemented scope

[history_search.h](../../native/features/history_controls/history_search.h) and [history_search.cc](../../native/features/history_controls/history_search.cc) provide pure search preparation and completion checks over [lifecycle contract v3](../lifecycle-contract.md). They read the existing model's selected Space, attachment and Profile. They own no selection, page, history record, database, settings, navigation or persistence state.

- `PlanHistorySearch` resolves the requesting window's currently selected Space to its Profile. Missing windows/selection, missing attachments, mismatched native-context Profile reports, off-the-record contexts and unsupported context kinds are refused. Query IDs must be nonempty.
- History scope is the entire Profile. Two Spaces sharing a Profile and two windows using that Profile target the same native service. The attachment in the plan identifies the requesting view, not a Space-specific history partition. No cross-Profile aggregation is provided.
- `ParseHistorySearchText` translates only the leading, case-sensitive `host:` prefix used by the pinned native history handler. It preserves the remaining UTF-16 text, including whitespace, case, embedded NUL and non-ASCII text. Matching, URL validation, visibility, ordering and deduplication remain native responsibilities. An empty query is permitted by the native API.
- `CheckHistorySearchCompletion` rejects superseded/cancelled queries and changed selection, Profile or context reports. It reads current model state rather than retaining another selection owner. Plans contain owned snapshots, so later caller mutations cannot alter their target or search text.

[native_history_search.cc](../../native/features/history_controls/native_history_search.cc) calls the real `HistoryServiceFactory::GetForProfile` and `history::HistoryService::QueryHistory`. It does not supply a fake provider or synthesize history results. It derives context kind from the actual `Profile`, rejecting `IsOffTheRecord()` and then nonregular Profiles before service lookup. It rechecks the plan against current scope before dispatch.

The query uses `EXPLICIT_ACCESS`, matching deliberate history inspection in Chromium. Disabling history recording therefore does not by itself hide previously recorded history. Options use the handler's per-local-day URL deduplication and explicit 404 exclusion. The parsed prefix sets native `host_only`. Remaining options retain native defaults, including recent-first order and ordinary user visits. There is no Arcified matching/indexing engine.

The consuming native surface must pass its own positive page size. No numerical page size or cap is chosen here. Pinned `QueryOptions::max_count == 0` means unbounded, so dispatch refuses zero and negative sizes. This slice implements an initial page only, without date controls or continuation. Native `QueryResults` pass unchanged to the supplied callback. The selected service reads the Profile's local database without constructing `BrowsingHistoryService`, `WebHistoryService`, Sync services, or an AI provider. Local database contents are not claimed to contain only visits generated on this device.

## Caller and integration obligations

No shared interfaces were edited. `NativeHistoryProfile` is a borrowed argument pair in this feature folder, not a registry implementation. The future native host must resolve its exact `ProfileId` to the live Chromium `Profile`. Supplying a display-name match, a path match, a pointer recovered from a stale view, or an original-Profile substitution violates the boundary. A logical ID check cannot prove that mapping correct.

1. Resolve the actual presented browsing context from the native host, derive a fresh context report, and prepare a plan using a fresh query ID. Do not obtain a Profile by changing the legacy immutable `Browser::GetProfile()` contract. The [ownership extraction](../native-space-integration.md) is still a prerequisite for the approved same-OS-window cross-Profile UX.
2. Keep the active query ID in the consuming view's existing request state. Invalidate it on every selection/context change, cancellation, teardown and Profile shutdown. IDs must not be reused, including after switching away and back to the same Space. A snapshot cannot detect a switch-away-and-return history if the caller leaves the old ID active.
3. Dispatch on the browser UI sequence while the exact Profile is live. Bind the callback to the view's native weak lifetime. Use the existing native `CancelableTaskTracker` to cancel previous work. This component creates no tracker, timer, provider, selection state or ID allocator.
4. Immediately before publishing on that sequence, resolve the current native context again and run `CheckHistorySearchCompletion`. If permitted, consume the active ID and display the native result. The guard does not consume caller state itself. Cancellation does not retract a callback already running, which is why both checks are required. Never retarget old results to a newly selected Profile.
5. Handle refusals distinctly from an empty result. Dispatch refusal drops its callback without running it. A returned native task ID means scheduling only. `kBadTaskId` becomes `kTaskNotScheduled`; a null factory result becomes `kServiceUnavailable`. Native `QueryResults` exposes records and `reached_beginning`, not a general database-success status. This slice adds no stronger failure, completeness, restart or undo guarantee.

The library should invoke this existing native provider path and retain only transient view/request state. It must not import these plans as library items or persist another copy of history. No result-opening command, focus command or page-transfer command is introduced here. Opening a record later must use the common native navigation path under its actual Profile and preserve native URL/security policy.

## Pinned source findings

Inspected on 2026-10-23. Workspace baseline is `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. The local engine pin is `155.0.8059.79`, resolving to Chromium `a49e2e3d4addfd84e765b52afe230bedb81096db` in the [existing source reference ledger](../native-space-source-references.md#source-identity). All source links below use that commit. Files were inspected by direct read-only HTTP. Gitiles web-reader access failed; some parallel Gitiles HTTP reads returned 429, so the remaining files were read from Chromium's GitHub mirror at the identical commit. No archive, prepared tree, full checkout or build was downloaded.

| Source inspected | Relevant observation |
| --- | --- |
| [history_service_factory.cc, lines 21 to 60 and 89 to 101](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/history/history_service_factory.cc#21) | History service storage is initialized from the BrowserContext path. Implicit access returns null when history recording is disabled; explicit access remains possible. Regular and Guest selection redirect to the original context. A successful factory lookup alone does not enforce private-mode exclusion. |
| [profile.h, lines 219 to 222 and 394 to 428](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/profiles/profile.h#394) | Off-the-record includes more than Incognito, including Guest and other OTR contexts. Profile kinds must come from native methods, not presentation labels. This slice excludes every OTR context and every nonregular Profile. |
| [history_tab_helper.cc, lines 645 to 653](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/history/history_tab_helper.cc#645) | Native navigation history recording already derives the Profile from WebContents, refuses OTR and uses implicit access. Arcified must not add another recorder or infer a visit from a saved destination. |
| [browsing_history_handler.cc, lines 508 to 574](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/webui/history/browsing_history_handler.cc#508) | The history UI uses explicit service access, per-day deduplication, explicit 404 exclusion and leading `host:` translation. Its combined browsing service also accepts Sync; this local query slice uses the underlying HistoryService directly. |
| [history_service.h, lines 332 to 344](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/history/core/browser/history_service.h#332), [history_service.cc, lines 1379 to 1390](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/history/core/browser/history_service.cc#1379) | The real query takes UTF-16 text, QueryOptions, a once-callback and a native cancelable task tracker. It posts backend work and replies through that tracker. |
| [history_types.h, lines 261 to 457](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/history/core/browser/history_types.h#261) | Results are moveable native URL results. Options own matching, deduplication, visit order and time semantics. Search need not return every visit. Per-day grouping uses local timezone, and text queries have different time-range behavior from empty queries. |
| [cancelable_task_tracker.h, lines 62 to 125](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/base/task/cancelable_task_tracker.h#62) | Destruction cancels tracked work; cancelling an already-running reply is a no-op. Native cancellation and view/scope guards serve different purposes. |

The inspected bytes have these SHA-256 identities. These identify source reads, not a prepared or running browser:

| Chromium file | SHA-256 |
| --- | --- |
| `chrome/browser/history/history_service_factory.cc` | `613fbdd7e281047b9a3ccfd6590b89969f5cc1d63790e6276217dd02a70704ba` |
| `chrome/browser/profiles/profile.h` | `dc1ff74874ce73db6eeb579ed9b5a85a3226387ce3a4172eb814336562634485` |
| `chrome/browser/history/history_tab_helper.cc` | `2c00763ff6ed3cad22c68e96e9f03ef373f9c1e4f32724865f00c19e37d73e40` |
| `chrome/browser/ui/webui/history/browsing_history_handler.cc` | `c7f8008d89207d8ead16bbab1b157c357ef224177beff29bf38cb8a87467be04` |
| `components/history/core/browser/history_service.h` | `6c4cf3f63f50e3d9e4d1bc8fdafb075eae5a294ac91e622f08991a06407eae52` |
| `components/history/core/browser/history_service.cc` | `4ad3439001284f60981277956ce4fb14ff0367448b96f8df9d7012de5f6c9d7b` |
| `components/history/core/browser/history_types.h` | `e064581964b55f43ea2663c28fbfd0d6d75cc78dfbb644e4f27590a06b4eb731` |
| `base/task/cancelable_task_tracker.h` | `b92323db010cd3a4b21edab8a84168e12496c2353be2a3014efac1f9d0da77d1` |
| `components/history/core/browser/browsing_history_service.cc` | `6dbf447c0ddafc5d4c8d1fddd6c9539d2babeda2dcfd3022250bdfa75d7607fb` |
| `chrome/browser/browsing_data/chrome_browsing_data_remover_delegate.cc` | `1e1bc1c7b2b0b47841006f28e6b88887aa9b22a56c9cf0f76b963080082515b8` |
| `content/browser/browsing_data/browsing_data_remover_impl.cc` | `35b615c92779133a2810da06720e3be926263f55d7f53db6f621d9a32281a976` |

The local lifecycle header, implementation and v3 contract have Git blobs `d3a7aed6f1caddd40f5422bc6e5c1f5019636338`, `7ffe87eb49d0cf4a92f83058b7c348ed3882012f` and `d6110f065107622b70a413ac9816d612f257fbca`. The older native design records v2 review evidence and explicitly leaves native ownership extraction unproven; this component uses the current v3 inputs.

## Destructive consequences remain unresolved

No delete, clear, cleanup, undo, confirmation dialog or destructive callback exists in this component. Do not enable an action by treating these source observations as an approved confirmation policy.

| Existing native path | Consequence to disclose before any future integration |
| --- | --- |
| [BrowsingHistoryService::RemoveVisits, lines 396 to 498](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/history/core/browser/browsing_history_service.cc#396) | Row removal checks deletion permission, constructs URL/day expiry ranges from grouped timestamps and can issue sync deletion directives and web-history expiry. It is not necessarily deletion of one visit timestamp. Policy, grouping, local-only intent and remote effects need explicit decisions. |
| [HistoryService deletion declarations, lines 511 to 560](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/history/core/browser/history_service.h#511) | URL deletion, time-range expiry and local/remote deletion are distinct operations. A search result is not a complete deletion manifest. Native declarations do not promise restoration of removed history. |
| [ChromeBrowsingDataRemoverDelegate, lines 414 to 602](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/browsing_data/chrome_browsing_data_remover_delegate.cc#414) | The history mask also clears related information, including host cache, speculative state, some logs, bookmark last-used timestamps and some site metadata. Some related stores do not honor the requested time range. This is broader than deleting visible rows. Exact behavior varies with compiled features and fork patches. |
| [Separate bookmark branch, lines 810 to 824](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/browsing_data/chrome_browsing_data_remover_delegate.cc#810), [separate download branch, lines 396 to 406](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/browser/browsing_data/browsing_data_remover_impl.cc#396) | Bookmark deletion and download-history removal use separate masks/paths. This distinction does not prove an installed build preserves downloaded files, kept items, boards or Archive. Those downstream stores and interaction consequences need explicit acceptance evidence. No file deletion API is called here. |

Relevant fork patches were inspected without applying the series. [Google-backed sync integration changes](../../../patches/helium/core/sync/disable-google-backed-sync-integrations.patch), Git blob `61eaa00de8c40aec05ebb89a28068131536948b3`, alter web-history availability. [History expiration](../../../patches/helium/core/sync/history-expiration-threshold.patch), blob `817283fc92d71ebc79bf243f05c49d97f356c69f`, centralizes the existing retention threshold and private-Sync cleanup. [Clear-data-on-exit](../../../patches/ungoogled-chromium/add-flag-to-clear-data-on-exit.patch), blob `6f97efee5442cb1daad14e1f0cf8a1fee4192679`, adds a disabled-by-default broad clearing flag. This feature neither enables that flag nor changes retention. Upstream source alone cannot establish final patched behavior, remote deletion effects or persistence guarantees.

## Focused verification

Run from the repository root using the installed toolchain:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/history_controls/history_search.h \
  arcified/native/features/history_controls/history_search.cc \
  arcified/native/features/history_controls/history_search_test.cc \
  arcified/native/features/history_controls/native_history_search.h \
  arcified/native/features/history_controls/native_history_search.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/history_controls/history_search.cc \
  arcified/native/features/history_controls/history_search_test.cc \
  -o /tmp/arcified-history-search-test
/tmp/arcified-history-search-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/history_controls/history_search.cc \
  arcified/native/features/history_controls/history_search_test.cc \
  -o /tmp/arcified-history-search-sanitized
/tmp/arcified-history-search-sanitized
clang-tidy \
  arcified/native/features/history_controls/history_search.cc \
  arcified/native/features/history_controls/history_search_test.cc \
  --checks='-*,clang-analyzer-*' --warnings-as-errors='*' -- -std=c++17
```

All listed checks passed on this Linux host. The fixtures use only synthetic identities, text and reserved `.test` destinations. They cover prefix/text preservation; Profile-wide scope across same-Profile Spaces/windows; cross-Profile selection; owned snapshots; missing-window/selection and private/unsupported/mismatched-context refusals; superseded, cancelled and duplicate completions; selection/detachment/window-close races; switch-away-and-back invalidation; Profile changes after empty-Space reassignment; and preservation of existing saved/temporary pages, navigation, protected activity and split references. These are public planner/guard behaviors, not native history-service tests.

The host's initial `clang-tidy` invocation enabled diagnostics only and returned `Error: no checks enabled.` The explicit analyzer selection above ran successfully without changing configuration. Native files remain outside that analysis because it requires Chromium headers. No Markdown formatter/linter is declared for this document. Local links and source-hash lengths were checked in memory; `git diff --no-index --check /dev/null <path>` checked whitespace for each new owned file.

## Open native and platform acceptance

`native_history_search.h/.cc` are source-backed, unwired implementation files. Formatting checks cover them; standalone compilation and sanitizers exclude them because no pinned prepared Chromium tree is available or authorized. No fake Chromium headers or provider substitutes are used. GN dependencies, exact native compilation, Profile-lifetime binding, weak callback integration, library UI, accessible search/results and native error presentation remain open. The parent coordinator owns any required shared host/registry changes.

The [publication acceptance](../research/arc-dia/publication/history-controls.md) remains open. Required installed-build evidence on Linux, Windows and macOS includes actual native search and open, Profile isolation across switches, private/Guest exclusion, recording-disabled behavior, query cancellation and Profile shutdown, restart behavior and keyboard/screen-reader interaction. Deletion, clear-data consequences, selected-record grouping and undo/error behavior also await owner policy and native proof. No real browsing data, accounts, AI access, full Chromium preparation/build, remote CI, push, PR or issue update was used for this slice.
