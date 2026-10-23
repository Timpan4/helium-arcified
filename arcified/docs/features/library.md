# Library saved-source component

Local implementation slice for [issue 19](https://github.com/Timpan4/helium-arcified/issues/19), based on the [publication brief](../research/arc-dia/publication/library.md) and its `library` entry in [backlog.json](../research/arc-dia/backlog.json). This is a reusable C++17 saved-source projection and guarded semantic selection component. It implements no native Library surface, sidebar action, command binding, provider launch or focus behavior. Issue acceptance remains open.

## Source and authority

The implementation baseline is `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. [Lifecycle contract version 3](../lifecycle-contract.md) and its existing [model](../../native/model/lifecycle.h) own saved items, Profile/Space scope, shortcut aliases and live-page transitions. The component consumes `VisibleItems` and `VisibleShortcuts`, then delegates selection to `Model::OpenItem`. It creates no second ownership graph, page registry, persistence store, navigation state or selected-page record. No shared API change was needed.

The [native integration design](../native-space-integration.md) and [source references](../native-space-source-references.md) identify the production ownership gate. That design records a version 2 review; this component explicitly targets the current version 3 contract. Chromium `155.0.8059.79`, commit `a49e2e3d4addfd84e765b52afe230bedb81096db`, remains the inspected native source identity. Source inspection does not establish prepared-tree compatibility or runtime behavior.

## Search and selection

[saved_source.h](../../native/features/library/saved_source.h) exposes `SearchSaved` and `OpenSavedResult`. Both resolve the supplied attachment against the current model on the caller's serialized sequence.

1. `SearchSaved` projects the current Space's kept items, the current Profile's standalone shortcuts, and targets exposed by that Profile's shortcut aliases. It does not enumerate other Spaces' unaliased items. Each `ItemId` produces one row. A directly visible item takes precedence over aliases; multiple aliases to the same target collapse. Different items with identical destinations remain separate.
2. Rows contain the stable item identity, a source type and a copied saved destination for display/search. They contain no current navigation, temporary tabs, history, downloads, capture records or content. Treat the projection as disposable. Do not persist it or infer user order from its item-identity ordering.
3. An empty query lists the visible saved entries. Other queries use literal substring matching against saved destinations with locale-independent ASCII case folding. Non-ASCII UTF-8 bytes compare exactly. Titles, localized Unicode matching, ranking and native URL display formatting are not implemented. There is no result cap or retention policy.
4. A valid empty result is an empty vector. An unavailable attachment returns `Status::kNotAttached`. Source type remains explicit as `kKeptInSpace`, `kProfileShortcut` or `kShortcutAlias`.
5. `OpenSavedResult` accepts an item identity, attachment and caller-allocated candidate page ID. It rechecks current visibility. Deleted, moved, newly hidden or cross-Profile results return `kNotFound`. It never accepts the row's destination as an action target. It then calls `Model::OpenItem`, retaining that method's failures and existing-page behavior. An alias to an item in another Space still requires that owning Space to be attached; otherwise selection returns `kNotAttached` without attaching or switching anything.
6. Successful semantic selection creates a page from the authoritative saved destination or transfers the existing saved page with the same `PageId`, immutable Profile, current navigation and activity. Selection does not change the window's selected Space. The caller still owns non-reused ID allocation. No Library focus or navigation authority is introduced.

`OpenSavedResult` mutates the standalone model synchronously. It is not a native prepare/commit protocol and must not be called speculatively to claim native success. The eventual host coordinator must revalidate the source/attachment and real Profile, preserve native page identity, coordinate cancellation/failure, perform native presentation and focus, and only then record the corresponding semantic transition. Changes to that shared transaction interface belong to the coordinator.

The lifecycle model has opaque Profile identities, not an Incognito/guest policy or security-service implementation. Its visibility checks prevent mixing modeled Profile IDs. A production host must bind exact native Profile/OTR identities, provide only the authorized attachment/model, and never collapse a private Profile into its original Profile. Private persistence and native isolation acceptance remain open.

## Provider disposition

These are integration dispositions, not implemented provider adapters or synthetic provider records. The peer contract accepted in this workstream keeps capture/media/created-item providers unavailable. Capture and recent-file components consume authoritative caller data only. Portability exports saved metadata and produces an inert import plan with explicit Profile mappings; it allocates no Library records. Site diagnostics has no native provider. Library introduces no common provider contract.

| Source | Current Library disposition | Authority and remaining integration |
| --- | --- | --- |
| Saved workspace material | Local searchable projection implemented | Lifecycle v3 saved items and aliases. Native storage, presentation and command wiring remain open. |
| Spaces organizer | Native destination unavailable | No organizer provider or new Space state is created here. The shared host coordinator must supply the eventual entry point. |
| History | Existing native destination identified; Library launch unavailable | Retain Chromium's History UI and Profile policy. Do not import history rows into this component. |
| Downloads | Existing native destination identified; Library launch unavailable | Retain Chromium's Downloads UI, manager, safety and recovery. No download records/actions are implemented here. |
| Archive | Unavailable | Lifecycle has no Archive transition or restore journal. History is not labeled Archive. Cleanup/destructive/restore policy stays open. |
| Media and captures | Unavailable | No native provider, capture index, file scan or media-state import. |
| Created items | Unavailable | No native provider or fabricated creation store. |

The pinned [command controller](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/browser_command_controller.cc#1258) routes `IDC_SHOW_HISTORY` and `IDC_SHOW_DOWNLOADS` through `webui::GetBrowserForOpeningWebUi` to `ShowHistory` and `ShowDownloads`. The inspected [History and Downloads helpers](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/chrome_pages.cc#337) use the existing singleton-tab paths. History explicitly shows an Incognito disclaimer instead of the History UI; Downloads hides its existing bubble before opening its page. The [WebUI browser resolver](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/webui/util/webui_util_desktop.cc#76) returns the supplied ordinary browser and resolves picture-in-picture to its opener's browser. These symbol regions were read via Gitiles HTTP on 2026-10-23. They are reuse points, not proof of a Library launch or same-window multi-Profile host compatibility. A guessed `ui/webui/webui_util.cc` path returned 404 and supplies no evidence.

## Focused checks

[saved_source_test.cc](../../native/features/library/saved_source_test.cc) uses synthetic IDs and reserved `.test` destinations. Its fixtures cover unavailable versus empty sources, Profile/Space visibility, temporary/live-navigation exclusion, source types, alias deduplication, distinct same-URL identities, ASCII/UTF-8 matching, closed-item opening, protected saved-page selection from another window, owning-attachment refusal, removed aliases/items, changed ownership, detached attachments and a changed attachment Profile. The test pins lifecycle contract version 3 at compile time.

Run from the repository root with the existing tools:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/library/saved_source.h \
  arcified/native/features/library/saved_source.cc \
  arcified/native/features/library/saved_source_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/library/saved_source.cc \
  arcified/native/features/library/saved_source_test.cc \
  -o /tmp/arcified-library-saved-source-test
/tmp/arcified-library-saved-source-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/library/saved_source.cc \
  arcified/native/features/library/saved_source_test.cc \
  -o /tmp/arcified-library-saved-source-sanitized
/tmp/arcified-library-saved-source-sanitized
```

The focused standalone compile/run and AddressSanitizer/UndefinedBehaviorSanitizer compile/run passed locally on Linux. They prove modeled search/selection behavior only. The Library-owned C++ files pass the Chromium-style formatter check; whitespace checks cover all four new files. No shared tooling or build target was changed.

Coordinator static analysis passed with `clang-tidy --checks='clang-analyzer-*,bugprone-*' --warnings-as-errors='*' arcified/native/features/library/saved_source.cc arcified/native/features/library/saved_source_test.cc -- -std=c++17 -fno-exceptions`. The installed tool has no default checks, so those families were selected explicitly. An initial exception-enabled analysis flagged throwing static test initialization and possible variant exceptions. The no-exception mode matches the non-Windows `no_exceptions` configuration inspected in the pinned [compiler configuration](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/build/config/compiler/BUILD.gn#2399). No user-code diagnostics remain in that analysis mode; this is not a full Chromium build configuration or native compatibility check.

## Native acceptance still blocked

The native Library entry point, sidebar/command invocation, real saved-source persistence, provider launching, actual focus return, URL presentation/accessibility and browser cancellation/failure handling remain unimplemented. Same actual OS window switching across Profiles and live saved-page handoff still need the production ownership extraction and native proof described in the integration design. Linux, Windows and macOS installed-build interaction, privacy isolation, persistence/restart and visual/accessibility acceptance have not run. No native behavior follows from these standalone tests, and this slice does not close issue 19.
