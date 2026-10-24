# Little Arc entry and promotion planning

Status: reusable standalone C++ planning component for [issue 111](https://github.com/Timpan4/helium-arcified/issues/111), 2026-10-23. No native window, browser integration or platform acceptance is implemented. Work started at `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`.

[planning.h](../../native/features/little_arc/planning.h) and [planning.cc](../../native/features/little_arc/planning.cc) consume the authoritative [lifecycle model](../../native/model/lifecycle.h), semantic contract version 3. They allocate no identities and mutate no model state. Live navigation, activity, page ownership, Profile identity and persistence stay with their existing owners. The component stores no focus state, route rules, page registry, history or cleanup timer.

## Entry boundary

The coordinator approved this local boundary before implementation. `ResolvedTarget` carries an explicit `ProfileId` and `Attachment`. An integrated caller owns adaptation from Air Traffic's resolved output. Little Arc imports no Air Traffic module and neither matches URLs nor chooses a Profile, Space or window. The explicit Profile identity is checked against the current Space mapping so stale or inconsistent routing cannot silently choose a different Profile.

`PlanEntry(model, target, intent, presentation)` requires an existing window/Space attachment. Provisioning a logical attachment does not prove that its native host or Profile context exists. The result is either an `EntryPlan` or a typed refusal for a missing target, unattached target, mismatched Profile or empty external destination.

The caller supplies these choices without defaults:

| Input | Local behavior | Outside this component |
| --- | --- | --- |
| `ExternalLink{destination}` | Retains the initial requested destination in the plan. Empty input is refused. | Native default-browser handoff, URL parsing/security validation, protocol handling, redirects, opener/referrer policy and actual navigation. A nonempty string is not proof of browser validation. |
| `QuickLookup{}` | Returns a lookup intent with no URL and creates no page. | Shortcut registration, lookup chrome, focus, submitted search policy and native page creation. No blank-page or search-provider URL is invented. |
| `Presentation::kLightweightWindow` | Retains the caller's requested lightweight presentation. | Creating, activating, sizing or placing a window. |
| `Presentation::kMainWindow` | Retains the caller's requested main presentation. | Preference storage and the always-open-in-main-browser product policy. |

Repeated equal destinations remain requests. Planning does not merge pages by URL, reuse an existing page, allocate another page or decide whether multiple lightweight windows are allowed. Each invocation validates its supplied attachment independently. The target's selected Space is never changed as a side effect. A target may use a different Profile from its logical window's current selection; this is model validation only, not proof of secure native multi-Profile presentation.

## Promotion boundary

`PlanPromotion(model, page_id, target)` reads the current live page and returns one of these outcomes:

| Result | Preconditions and meaning |
| --- | --- |
| `PresentInMainWindow` | The page and target have the same Profile and attachment. The caller may prepare expansion of the existing lightweight host into main presentation, retaining the same `PageId`, Space and actual native OS window. This is a presentation intent, not an implementation or authorization to reparent the native page. Temporary ownership and saved state remain unchanged. |
| `TransferSavedPage` | The destination attachment is different, the page is already saved and both use the same Profile. A Space-kept item's destination must still be its owning Space. A Profile-owned saved item may use another attached Space in that Profile, as the existing lifecycle transition permits. The plan references the original page, item and source/destination attachments. |
| `kTemporaryTransferUnavailable` | A temporary page would need another attachment, including another Space in the same logical window. The lifecycle contract supplies no temporary transfer operation. No Keep, reopen, navigation copy or substitute page is performed. |
| Other typed refusal | Missing page/target, missing attachment, target mapping mismatch, page/Profile mismatch or an incompatible saved Space. Refusal leaves every original intact. |

No result changes ownership or calls `Keep`, `TransferSavedPage`, `SelectSpace`, close or navigation APIs. Unknown/protected activity does not block nondestructive planning and is never cleared. An explicitly separate `Keep` can make a page eligible for the existing saved-transfer path, but this module does not suggest, require or perform Keep to bypass a temporary-transfer refusal. Promotion alone does not mean saving a page.

The caller must already know which page the action targets. After root integration, explicit page commands can use the coordinator's shared `PageCommandTarget{PageId, ProfileId, Attachment}`, `Resolve(model, explicit_focused_page, expected_attachment)` and `Revalidate(model, target)` seam. This local component remains uncoupled and owns no additional focus-state store. Native code must separately verify the actual TabModel/WebContents binding and operation generation, including replacement and ABA cases. Lifecycle IDs alone cannot establish those facts.

## Native completion and revalidation

Plans own snapshots, not reservations or commit tokens. They contain no copy of current navigation. A snapshot can become stale after navigation, Keep, attachment transfer, close, Profile shutdown or target removal. Replanning reads current lifecycle state; it cannot detect a native object replacement or a move away and back by itself. No asynchronous protocol is implemented here.

The eventual native caller must:

1. Validate external input through browser URL/security policy and obtain the caller-resolved target. Provision and verify the actual target host and immutable Profile context. Coordinate cancellation/failure of provisional host creation without closing unrelated pages or leaving visible success behind.
2. Serialize conflicting commands and revalidate the target mapping, page identity/ownership/attachment, actual native bindings and operation generation immediately before actuation. Preserve native security, before-unload, modal, focus and accessibility behavior. A plan does not authorize dropping these checks.
3. For external entry, perform one intended native creation/navigation under the resolved Profile. Report successful creation/commits through the existing lifecycle APIs with fresh, non-reused identities and actual committed navigation. Quick lookup alone reports no page. Failed/cancelled native work must not create a synthetic success record. Duplicate OS deliveries and reuse remain an explicit caller policy gap.
4. For in-place promotion, preserve the actual host and page while preparing main chrome/commands. Publish presentation/focus only after native success. Do not invent a saved item or transfer to another window. For saved-page transfer, preserve the existing TabModel, WebContents, navigation controller and Profile through the source-backed transfer route, then report the successful lifecycle transition and coordinate selection separately. No renderer/history clone or URL reopen qualifies.
5. Retain the exact original on cancellation or failure. Reconcile callbacks and any native success that races with failed model reporting. Native prepare/commit, retained ownership and rollback must be designed before wiring these plans to production. Calling the synchronous model first cannot stand in for native completion.

## Source constraints

This slice relies on the supplied [native Space investigation](../native-space-integration.md) and its [pinned source references](../native-space-source-references.md). It does not claim a fresh Chromium checkout, source preparation or runtime observation. That investigation predates contract version 3; its old statements about unapproved Profile/Space mapping and settings scope are superseded by the current [lifecycle contract](../lifecycle-contract.md). Its native ownership constraints remain relevant.

| Existing evidence | Consequence for Little Arc |
| --- | --- |
| [C1: desktop window/Profile ownership](../native-space-source-references.md#c1-desktop-window-and-profile-ownership) and [C2: widget/command coupling](../native-space-source-references.md#c2-view-widget-and-command-coupling) | Chromium's desktop window and chrome capture one immutable Profile. Same-window multi-Profile switching requires ownership extraction, not a BrowserView/WebView adapter, a changing Profile getter or hidden backing OS windows. A lightweight window type does not resolve that constraint. |
| [C3: live identity and handoff](../native-space-source-references.md#c3-live-identity-and-handoff) | TabModel-preserving detach/insert is the candidate saved-page route. Extracting WebContents destroys its wrapper, and source empty-strip behavior can close a window. Last-page transfer and retained ownership/failure handling remain native proof gates. |
| [C4: navigation/session boundaries](../native-space-source-references.md#c4-navigation-and-session-boundaries) | Native navigation and Profile-local sessions remain authoritative. An entry plan supplies intent, not a parallel navigation/session controller. Cross-Profile reopen cannot preserve the original page's live state or change its Profile. |

The [publication brief](../research/arc-dia/publication/little-arc.md) and matching `little-arc` [backlog entry](../research/arc-dia/backlog.json) define the requested user outcome. Those documentary references establish intent, not Windows/Linux parity or native behavior. The window-ownership prerequisite remains open.

## Focused checks

[planning_test.cc](../../native/features/little_arc/planning_test.cc) uses synthetic IDs and reserved `.test` URLs. Its fixtures cover explicit external/main/lightweight/lookup intent, repeated requests with distinct caller-selected targets, target validation, unchanged same-host page references, refused temporary/cross-Profile/wrong-Space promotion, and saved-transfer plans. A separate synchronous lifecycle report checks retained page/Profile/navigation/activity/item identity and split-reference cleanup. It does not simulate native success, cancellation or rollback. A retained old snapshot and fresh replan demonstrate why plans are not live references. Explicit Keep and Profile-shortcut fixtures exercise existing lifecycle semantics without adding transitions.

Run from the repository root with the existing toolchain. `--driver-mode=g++` follows the standalone lifecycle check's host linker requirement. No dependencies or global packages are added.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/little_arc/planning.h \
  arcified/native/features/little_arc/planning.cc \
  arcified/native/features/little_arc/planning_test.cc
clang-tidy --checks='clang-analyzer-*,bugprone-*' --warnings-as-errors='*' \
  arcified/native/features/little_arc/planning.cc \
  arcified/native/features/little_arc/planning_test.cc \
  -- -std=c++17 -Wall -Wextra -Werror -pedantic
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/little_arc/planning.cc \
  arcified/native/features/little_arc/planning_test.cc \
  -o /tmp/arcified-little-arc-test
/tmp/arcified-little-arc-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/little_arc/planning.cc \
  arcified/native/features/little_arc/planning_test.cc \
  -o /tmp/arcified-little-arc-sanitized
/tmp/arcified-little-arc-sanitized
```

The explicit clang-tidy families are needed because the host's default invocation enables no checks. No Chromium GN target, shared tooling, patch or execution-catalog entry is added by this slice.

Local results on 2026-10-23: all six fixtures passed in both ordinary and address/undefined-sanitized builds. The formatter check and explicit clang-tidy invocation exited successfully; clang-tidy suppressed diagnostics in non-user headers. All relative file links resolved through filesystem metadata. The tracked diff and each new file's `git diff --no-index --check /dev/null <path>` emitted no whitespace diagnostics. The latter commands return 1 for new-file differences. These results cover this local component, not native acceptance.

## Open acceptance and policy

All installed native acceptance from issue 111 remains open on Linux, Windows and macOS. Standalone fixtures prove plan results and synthetic lifecycle behavior only. No native UI, visual, focus, accessibility, storage isolation, history preservation, default-browser registration, restart or failure-recovery check ran. Full Chromium preparation/build, heavy downloads, remote CI and benchmarks remain deferred.

The remaining implementation includes OS external handoff, one focused lightweight window, visible promotion and keyboard lookup actions, main-window preference UI/storage, native completion/reconciliation and actual temporary promotion. Same-window Profile presentation and saved transfer need the ownership/native gates above. Fullscreen and multiple-display placement/activation require installed-build interaction evidence.

Invocation/reuse, duplicate-link and multiple-lightweight-window behavior, promotion history, Archive provenance and idle cleanup require explicit policy and native coverage. Closing/resetting pages must retain existing safety checks while activity/manual-destructive policy remains open. No timeout, retention period, cleanup schedule or automatic source disposition is inferred. Populated cross-Profile migration remains open and cannot be implemented by relabeling a live page. Account, sync and AI integrations are outside this slice.
