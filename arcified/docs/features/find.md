# Local Find command binding

Issue [33](https://github.com/Timpan4/helium-arcified/issues/33). This slice provides a standalone C++17 Find command planner and effect-time binding check in [local_find](../../native/features/local_find/find_command.h). It does not install browser chrome, execute native effects or close the issue. It uses lifecycle contract **v3** identity types without mutating the model.

## Implemented behavior

`PlanFindCommand(validated_page, validated_profile, command, eligibility, controller_snapshot)` returns a concrete operation or a refusal. It compares the caller's validated focused PageId and immutable Profile with the observed native Find controller binding. Missing controller/helper availability, mismatched page/Profile, missing generation, or deferred input eligibility produces no operation. Closing an inactive native Find UI also produces no operation, avoiding a late close that would restore focus unnecessarily.

| Command | Operation payload | Native call after revalidation |
| --- | --- | --- |
| Open | `OpenFindBar` | `FindBarController::Show(false, true)` |
| Next | `AdvanceMatch{Direction::kForward}` | `FindBarController::Show(true, true)` |
| Previous | `AdvanceMatch{Direction::kBackward}` | `FindBarController::Show(true, false)` |
| Close | `EndSessionKeepingSelectionAndResults` | `FindBarController::EndFindSession(SelectionAction::kKeep, ResultAction::kKeep)` |

`RevalidateFindPlan` checks fresh input eligibility, controller availability, controller-bound page/Profile, current caller-validated focus identity, generation and active-session state for close. It rejects a changed generation even when focus returns to the same PageId. Plans own their captured identity/token values and retain no borrowed model references. They contain no attachment, query, match count, text selection, current navigation, saved destination or focus state.

The generation is a caller-owned, opaque, nonempty token. The caller must invalidate it on focus discontinuity, controller/WebContents replacement or rebinding, document change, native session invalidation, page transfer and close, including an away-and-back transition. This component neither allocates generations nor observes native events. Token equality cannot establish freshness if the caller fails to invalidate it. No URL/pointer-derived token, delay, retry limit or timeout is introduced.

`ControllerSnapshot` is a transient native observation. Publish one only after checking that the actual controller, its WebContents and their FindTabHelper are usable. Read `find_ui_active` from that helper. Absence means unavailable, including destruction or replacement gaps. A snapshot does not own a controller, helper, page or session and must not be retained as a native registry.

## Shared adapter seam

The accepted commands contract is `arcified::commands::PageCommandTarget{PageId page, ProfileId profile, Attachment attachment}` in `features/commands/command_target.h`. Its `ResolvePageCommandTarget(Model, focused_page, expected_attachment)` and `RevalidatePageCommandTarget(Model, target)` remain the only lifecycle target validators. The peer header was inspected read-only in the commands worktree. This separate build does not copy its type or depend on a header that root has not integrated yet.

The eventual adapter must perform these steps on the serialized native UI sequence.

1. Obtain the focused PageId from the caller's authoritative focus owner and resolve the common target against the current read-only lifecycle model. If resolution fails, stop. Pass `target.page` and `target.profile` to `PlanFindCommand`; retain the original common target, including its attachment, externally.
2. Obtain `CommandEligibility` from native/input-owner arbitration after native IME processing. Map only a positive eligibility result to `kEligible`. Unknown, active or stale input ownership maps to `kDeferToNativeInput`. Local Find does not inspect key events, composition/candidate APIs, modifiers or repeats. In particular, a false candidate-popup report on Linux/macOS is insufficient evidence of eligibility. Explicit close controls use the caller's eligibility decision too.
3. Sample the controller's actual bound WebContents and resolve its page/Profile through the native registry. Do not bind another page opportunistically or change focus to make a command pass. If the binding disagrees, report the refusal and let the authoritative presenter reconcile it.
4. Immediately before effect, call `RevalidatePageCommandTarget`, re-read authoritative focus and input eligibility, and verify exact TabModel/WebContents/controller identity, immutable real Profile/BrowserContext, attachment and operation generation. Call `RevalidateFindPlan` with the fresh focused target fields and controller observation. Do not yield between these checks and dispatch. Any refusal cancels the old command; do not silently retarget or replay it on a new page.
5. Execute the table's native call on the checked controller. Native callbacks can run synchronously; native ownership and lifetime safety still belong to the adapter. The standalone planner provides no atomic transaction, weak-pointer implementation, native success report or guarantee against reentrant destruction.

The common resolver checks lifecycle identity/attachment consistency, not focus continuity or native ABA. A saved-page transfer keeps its PageId and Profile but changes its attachment and native binding context. The common attachment check and external generation invalidation both remain required. No other saved page or window-local temporary page is copied, transferred, activated, closed or recreated by Find.

## Pinned source evidence

Read on 2026-10-23 against Chromium `155.0.8059.79`, commit `a49e2e3d4addfd84e765b52afe230bedb81096db`, from the repository's engine pin and [source identity record](../native-space-source-references.md#source-identity). Files were retrieved individually through Gitiles `?format=TEXT` and decoded in memory. No Chromium tree, archive or build was prepared. The web reader could not open Gitiles; direct read-only HTTP succeeded. Guessed old `chrome/browser/find_bar/find_tab_helper` paths returned 404; the authoritative helper is under `components/find_in_page`.

| Source | Observed contract used here |
| --- | --- |
| [browser_commands.cc, Find/FindNext/FindPrevious/FindInPage at 2503 and CloseFind at 2612](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/browser_commands.cc#2503) | Existing browser dispatch maps to the same controller Show arguments and keep/keep close. |
| [find_bar_controller.cc, Show at 46 and EndFindSession at 95](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/find_bar/find_bar_controller.cc#46) | Open delegates selection prefill and highlights/counts without jumping to a match. Show takes native Find focus. End hides, stops through the helper, and restores saved native focus when contents remain available. |
| [find_bar_controller.cc, ChangeWebContents at 121 and HandleActiveTabChanged at 208](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/find_bar/find_bar_controller.cc#121) | Controller observation, per-tab field selection and native visibility/focus reconciliation already exist. This planner does not call ChangeWebContents or create another binding owner. |
| [find_bar_controller.cc, NavigationEntryCommitted at 250](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/find_bar/find_bar_controller.cc#250) | Native navigation handling closes or stops search according to when Find opened. The planner does not rerun an old query on another document. |
| [find_bar_view.cc, UpdateForResult at 394 and OnAfterUserAction at 542](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/find_bar_view.cc#394) | The native textfield protects composition during result updates and delays searches until composition commits. This is text-edit ownership, not evidence that the whole platform keyboard arbitration path is correct. |
| [find_bar_view.cc, Find at 596, FindNext at 621 and EndFindSession at 636](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/find_bar_view.cc#596) | Native edits notify the controller and start helper searches; text edits can race with destruction. Buttons retain native query and close semantics. Ordinary edits stay in this path, including clipboard/data-protection hooks. |
| [find_tab_helper.cc, StartFinding at 45, StopFinding at 95 and HandleFindReply at 159](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/find_in_page/find_tab_helper.cc#45) | Chromium owns query normalization, request/session IDs, asynchronous search, empty-query clearing and stale reply rejection. No search state is mirrored here. |
| [find_types.h, SelectionAction at 12 and ResultAction at 20](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/find_in_page/find_types.h#12) | Keep converts the match selection to an ordinary selection; keep results retains native Find presentation. Close does not activate a matched link. |
| [find_bar_host.cc, RestoreSavedFocus at 402 and AcceleratorPressed at 451](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/find_bar_host.cc#402) | Native focus tracking and Escape keep/keep close already exist. Ctrl+Enter uses activate-selection semantics and stays outside this component. |

Decoded source SHA-256 values, in the table's file order with repeated paths omitted:

```text
browser_commands.cc      ed7796d9654aac0bb444f9aeb5a37c5f60f41e920ce5a3ef0b9e168b42d6441e
find_bar_controller.cc   0fd95923fe0f6f11a09fd7a78c89bb2b24c8f5576a6c22024aa48be054a5c6eb
find_bar_view.cc         53c0d41bbcbf7c98485e3e1bedef4bc8ed6acb2f1dadbd1d0e6adf07ec3a43fd
find_tab_helper.cc       08a8a602f4a571104d68b94994693b7852d9b1a825ab3ff3001c79271e670acf
find_types.h            ad8d8c8e4d4d46a1f1e12676e77f15ba0000271790b802ac961fe72c74500f4a
find_bar_host.cc         2a51d550ae0e1127fc3ff919d39fee89693d23c4bf5e15377a02ce4a746a0aa6
```

The local [Helium Find patch](../../../patches/helium/ui/find-bar.patch) was read in full. It changes geometry/styling and removes the page-action update. Its displayed hunks retain the inspected search, close and textfield logic. The series was not applied, so prepared-tree compatibility remains unproven.

## Focused checks and remaining acceptance

The standalone tests exercise concrete native operation payloads, inactive close, missing/mismatched binding, missing generation, caller input suppression for every command, eligibility changes between planning and effect, focus/Profile changes, disappearance and changed generations with the same PageId. They do not run Chromium or search a document. Tests use synthetic identities only.

Run from the repository root with existing tools:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/local_find/find_command.h \
  arcified/native/features/local_find/find_command.cc \
  arcified/native/features/local_find/find_command_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/local_find/find_command.cc \
  arcified/native/features/local_find/find_command_test.cc \
  -o /tmp/arcified-local-find-test
/tmp/arcified-local-find-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/features/local_find/find_command.cc \
  arcified/native/features/local_find/find_command_test.cc \
  -o /tmp/arcified-local-find-sanitized
/tmp/arcified-local-find-sanitized
```

All checks above passed on the Linux host. Clang-format is the focused formatting check; compiler warnings are errors. No separate C++ linter target is declared in this standalone slice. No runtime limits, dependencies, global installs or real browsing data were added.

Linux native acceptance is **open**. Windows native build and acceptance are **open**. macOS native build and acceptance are **open**. No native browser build or visual interaction was available/run for this slice. Still required are installed-build evidence for selection/scroll and focus, keyboard/pointer and split-page routing, IME composition and candidate ownership, long/empty/no-match queries, dynamic pages and late results, navigation/replacement/close/transfer races, accessibility announcements and macOS pasteboard behavior. This planner imposes no query length cap and proves none of those renderer/UI behaviors.

Compact chrome wiring and the native registry/generation producer remain external integration work. Same-window multi-Profile feasibility remains unproven and requires the ownership extraction in the [native design](../native-space-integration.md), whose v2 policy statements predate the authoritative [v3 lifecycle contract](../lifecycle-contract.md). Cleanup, activity, destructive and cross-Profile migration policy remain open. AI Ask, accounts and sync are outside this implementation.
