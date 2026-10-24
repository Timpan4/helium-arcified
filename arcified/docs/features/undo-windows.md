# Closed-window undo

[Issue 70](https://github.com/Timpan4/helium-arcified/issues/70) now has a reusable C++ close-undo protocol in [window_restore.h](../../native/features/window_restore/window_restore.h) and [window_restore.cc](../../native/features/window_restore/window_restore.cc). It stages historical identity references, confirms completed deliberate closes, prepares ordered reopen plans and checks restoration reports before consuming history. It does not restore a native window. Native acceptance, including Windows behavior, remains blocked.

The scope follows the [issue brief](../research/arc-dia/publication/undo-windows.md), the `undo-windows` entry in [backlog.json](../research/arc-dia/backlog.json), [lifecycle contract v3](../lifecycle-contract.md), and the [native Space integration design](../native-space-integration.md). The GitHub issue body was read with `gh issue view 70 --repo Timpan4/helium-arcified --json number,title,body`; its acceptance criteria match the local brief. Implementation started at checkpoint `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`.

## Implemented ownership boundary

`Journal` is single-writer and in memory. It reads the existing `lifecycle::Model`, never mutates it, never opens a URL, and never changes activity or authorizes a close. The embedding coordinator serializes model commands, native events and journal operations. It cancels and recaptures when closing membership, ownership, selection or native restore associations change before completion.

`ClosedWindow` stores the old `WindowId`, ordered `SpaceId`/`ProfileId` references, ordered `PageId`/`PageOwner` references, selected Space and an optional selected-page reference. It contains no saved-item payload, destination, current navigation, history, credentials, activity report or renderer memory. Saved items and Profile shortcuts remain in the authoritative model. Temporary identities describe only the closed window's local pages. No other window's temporary pages enter the record.

Presentation order comes from the browser's presentation registry. Capture validates exact membership against the model; it never treats map order as user order. The selected-page reference must name a captured page in the model's selected Space. This is a single selected-page reference, not a definition of per-Space focus policy, split geometry or selected-pane restoration.

`NativeRestoreId` is an opaque, Profile-scoped runtime lookup key owned by the native bridge. Every captured page requires one key, and keys must be distinct within a Profile. Equal key text in different Profiles does not equate those contexts. The bridge must resolve the key against the exact native restore entry and session generation and invalidate it when unavailable. These keys are not raw Chromium SessionIDs, portable identifiers or durable data. **Do not serialize them.** This slice has no serializer or durable journal format. It neither copies nor extends the [session snapshot format](../../native/features/durability/session_snapshot.h).

## Close and reopen protocol

1. Root integration supplies the shared native privacy classifier through `CaptureEligibility`. `StageClose` requires verified regular eligibility for the exact window, including an empty window, and every actual `(WindowId, PageId, ProfileId)` in it. It checks actual model pages before considering the caller's presentation list, so omitting an unverified page cannot bypass exclusion. Unknown, OTR, guest and unsupported contexts must return false. There is no per-call "regular" flag and no local classifier. The root adapter must resolve live native contexts, validate identity/liveness and use the shared `private_browsing::ClassifyContext` contract. A Profile ID or synthetic boolean cannot prove regular native eligibility.
2. `StageClose` accepts only `kDeliberate`. `kCrash` and `kSessionShutdown` are excluded. It checks complete ordered membership and native joins, then retains a pending record. Pending records do not appear in `history()`. A model split causes `kSplitIntegrationPending` without a record. Capture itself does not change the model's unknown/protected activity refusal.
3. The existing native close path retains before-unload, cancellation and activity handling. Only after observed native completion and the corresponding model report may the coordinator call `ConfirmClose`. It refuses while the old window or any captured old page remains in the model. `CancelClose` removes a pending record without creating undo history. The in-memory checks cannot prove that a native close happened.
4. `BeginReopen` requires a confirmed entry, fresh target `WindowId`, fresh `PageId` for every captured page, and fresh `TemporaryTabId` for each restored temporary page. The caller owns the non-reusing allocator. Historical window/page/tab IDs are not reused. Space/item IDs remain unchanged. Input identity-map order does not change recorded presentation order. The result remaps page owners to the new window while retaining immutable Profiles and native lookup references.
5. Planning and `RevalidateReopen` resolve Spaces and saved items against the current model. Missing or incompatible owners fail. An item already live elsewhere returns `kSavedPageAlreadyLive`; the journal never calls `OpenItem` to take that page. The coordinator must revalidate immediately before native actuation, verify native key availability and exact regular contexts, and serialize conflicting operations. A plan is not a reservation, native transaction or navigation permission.
6. Native restore supplies actual pages and navigation commits through the existing authority. After native success and model reports, `ConfirmReopen` checks exact window membership, immutable Profiles, page owners, attachment membership, ordered presentation and selection. Partial, extra, transferred or reordered pages do not consume history. A split appearing during restoration remains pending. Attempt IDs reject mismatched or stale completion. A successful confirmation consumes only that entry.
7. `CancelReopen` returns the entry to closed history. It neither closes partially restored pages nor claims rollback. Root integration must reconcile partial native success before retrying, allocate new identities for a new native attempt and retain accurate failure UI. `history()` reports close-confirmation order, oldest first; it includes entries with an in-flight reopen. No MRU action, automatic retry, retention cap, expiry, eviction, last-window policy or cleanup policy is selected here.

The API has no native implementation of `CaptureEligibility`. Its synthetic test allowlist proves that the protocol refuses unverified identities; it does not prove exclusion of actual private or guest browser data. The accepted classifier lives in the separate Profiles workstream, not in this slice. Root integration must bind it without deriving native eligibility from lifecycle IDs or collapsing OTR objects to their original Profile.

## Source-backed integration points

Read-only source inspection used Chromium commit `a49e2e3d4addfd84e765b52afe230bedb81096db`, the engine identity recorded in [native Space source references](../native-space-source-references.md). Individual Gitiles files were read through `curl` and base64 decoding. No source preparation, download archive, patch application or build ran.

| Inspected source | Consequence for integration |
| --- | --- |
| [TabRestoreService](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/sessions/core/tab_restore_service.h) | This is the existing recently closed tab/window service. It exposes `BrowserClosing`, `BrowserClosed`, entry lookup and restore operations. Reuse native restore records for navigation. Its API does not establish an atomic workspace/native transaction or approved close policy. |
| [tab_restore_types.h](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/sessions/core/tab_restore_types.h) | Native window entries include ordered tabs, selection and group/split metadata. Native tab entries contain serialized navigation. Entry IDs are session-unique, so a bare SessionID cannot serve as a durable workspace join. This slice does not copy those payloads. |
| [TabRestoreServiceFactory](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/sessions/tab_restore_service_factory.cc) | Construction rejects OTR Profiles with a debug assertion, and Profile selection distinguishes original contexts. This is adjacent service evidence, not proof of this feature's private/guest exclusion. The exact native classifier and registry binding remain required. |

The existing [Helium lazy-session-restore patch](../../../patches/helium/core/lazy-session-restore.patch), already inspected in the source-reference ledger, is a reuse point for selected-first loading, skipped navigation entries and native split reconstruction. This work does not apply or modify it. Deliberate-close undo must not replay crash/session recovery or replace native navigation restoration with saved destinations. Restoring a saved live page's session entry and reopening an item at its saved destination are different operations.

## Acceptance status and remaining interfaces

| Issue criterion | Evidence here | Still required |
| --- | --- | --- |
| Separate deliberate-close undo from crash/session restore | `CloseReason` gating and journal state transitions have focused synthetic coverage. | Bind actual native close reason, native completion and cancellation. Decide last-window behavior without treating shutdown as an ordinary close. |
| Preserve supported ownership/order without duplicated shared items | Reference-only capture and remapped reopen plans are exercised against the real lifecycle model. Synthetic restore retains original saved items and the other window's temporary page. | Native host/context construction, order/focus registry, native session joins and failure reconciliation. Groups have no lifecycle representation or accepted metadata join here. Native grouped-window restoration remains open; root must not advertise unsupported group recovery. |
| Exclude private windows from durable history/recovery | Exact window/page eligibility is required before retaining capture data. Synthetic allowlist refusal includes missing and mismatched Profile mappings and empty windows. | Shared classifier binding to live native regular contexts, OTR/guest exclusion and installed privacy-path evidence. There is no durable journal or journal recovery path yet. |
| Add closed-window split/pane restoration when splits exist | Capture and confirmation explicitly refuse split-bearing windows. Tests prove the refusal retains state/history. | Accepted split metadata reconciliation and closed-window split order, geometry and selected-pane tests. No split integration is claimed. |

The [#50 snapshot/recovery slice](session-durability.md) and #99 migration slice do not make runtime lookup keys durable. A later reviewed protocol must join committed workspace metadata to native Profile-local records, coordinate interrupted close/reopen writes, define native key lifetime and report unavailable native records without creating duplicate saved pages. The journal must not become a second saved-item store or navigation controller.

The same actual OS window across differently profiled Spaces remains a prerequisite in the native host design. The mixed-Profile synthetic fixture proves logical ownership only. An unmodified Profile-bound Browser/BrowserView or a separate top-level window per Profile cannot pass that gate. Activity detection, destructive actions, populated cross-Profile migration, retention and last-window policy remain open. This slice does not settle them.

Windows installed-build behavior, source/build identity, visual evidence, keyboard/accessibility interaction, groups, native splits/panes, private/guest storage exclusion, before-unload cancellation, native entry invalidation, skipped restore records, partial restore and interrupted journal writes all remain open. Linux/macOS native compilation and runtime behavior are also unverified. Issue 70 cannot close from these checks.

## Focused checks

Run from the repository root with the existing C++17 toolchain:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/window_restore/window_restore.h \
  arcified/native/features/window_restore/window_restore.cc \
  arcified/native/features/window_restore/window_restore_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/window_restore/window_restore.cc \
  arcified/native/features/window_restore/window_restore_test.cc \
  -o /tmp/arcified-window-restore-test
/tmp/arcified-window-restore-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/window_restore/window_restore.cc \
  arcified/native/features/window_restore/window_restore_test.cc \
  -o /tmp/arcified-window-restore-sanitized
/tmp/arcified-window-restore-sanitized
```

The [synthetic test executable](../../native/features/window_restore/window_restore_test.cc) drives public APIs for confirmed close/reopen, cancellation, eligibility refusal, complete membership and native joins, split gates, fresh identities, stale attempts, saved-page conflicts, owner changes, partial/extra restore, history order and targeted consumption. Native restore is explicitly simulated with `.test` destinations. No browser, personal browsing data or provider is accessed.

The strict C++17 build and all seven fixture groups passed on the local Linux host. The AddressSanitizer/UndefinedBehaviorSanitizer build passed the same fixtures with no diagnostics. Chromium-style `clang-format --dry-run --Werror` passed for all three C++ files. Separate whitespace checks cover each new file; `git diff --check` alone does not inspect untracked files. These results prove the synthetic protocol only. No Chromium build, CI dispatch, heavy download, benchmark, global install, commit, push or remote write is part of this slice.
