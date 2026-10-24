# Focus routing contract decision packet

Issue: [44](https://github.com/Timpan4/helium-arcified/issues/44). Baseline: `a4b6b480d3de07857ab6e939149254a8359e5449`.
Recorded at 2026-10-23 22:32 UTC, 2026-10-24 in Europe/Stockholm.
Disposition: decision evidence ready for coordinator and owner review. Owner approval, independent review and native acceptance remain OPEN.

This packet specifies target semantics and fixtures proposed focus transitions. It adds no production focus owner, browser adapter, patch, native test or command registration. All authored evidence stays in this packet. [authority.json](authority.json) preserves the full assigned issue object. [sources.json](sources.json) and [identities.sha256](identities.sha256) identify exact authority, foundation and prepared-source bytes.

## Authority and inherited boundaries

The root [lifecycle contract](/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/arcified/docs/lifecycle-contract.md) version 3 owns identities, navigation and semantic transitions. Its approved shared saved items, local temporaries, same-window cross-Profile selection and live saved-page handoff remain requirements. Its open cleanup, destructive action, private/guest persistence and non-link navigation choices stay open.

The root [native Space integration design](/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/arcified/docs/native-space-integration.md) requires one coherent activation binding for visible child chrome, native command targets, Profile services, focus and accessibility. Queued work retains its originating context/page/generation. A pending modal, permission, fullscreen or capture owner stays bound to its source. A switch that cannot safely retain/reanchor that owner must defer/fail. Those are source-design obligations, not a working host API. Root execution state records native ownership and native acceptance as open.

The existing `features/commands/command_target.h/.cc` already supplies `PageCommandTarget{page, profile, attachment}`, `ResolvePageCommandTarget` and `RevalidatePageCommandTarget`. It accepts an explicit page, never infers selected Space, and cannot detect native replacement or away-and-back ABA. Its tests already cover explicit focus independent of Space selection, attachment/Profile rejection, transfer invalidation, close invalidation and the ABA limitation. They were read, not rerun. No duplicate resolver or registry is justified here.

The existing Peek component owns parent/preview references and bounded eligibility, not native focus or dismissal. Focus mode emits intentions and consumes actual host facts, not focus completion. Layout composition checks synthetic bounds/references, not native focus, accessibility or installed UI. Their existing pure fixtures are foundation evidence only. This leaf neither reimplements nor reruns them.

## Supported source findings

1. Prepared Chromium/Helium `chrome/VERSION` is 155.0.8059.79. The desktop `BrowserWindowInterface::GetProfile` contract fixes one Profile for the window lifetime and its tabs. That blocks treating a changing Profile getter as focus routing. See source N1.
2. `ReloadInternal` uses the native active tab, distinguishes split membership from multiple UI tab selection, and can reload several explicitly selected tabs. A split alone follows the active tab path. Therefore Arcified's required active-page-only ordinary Reload needs an explicit integration decision for the multi-selection path. A generic call to current Reload is insufficient evidence. See N2.
3. Page password UI and zoom use active WebContents; Find uses the Browser-owned FindBarController; address focus uses BrowserWindow. `FindBarController::HandleActiveTabChanged` changes observed WebContents and saved focus handling. These services must agree with the visible active surface. See N3, N7.
4. BrowserView obtains active WebContents from its tab strip. Location-bar focus checks platform activity/visibility and reveals Helium Zen chrome before focusing the field. MultiContentsView separately maintains active contents, visibility, border/overlay and traversal order. Focus notifications from an inactive split reach the delegate only for a visible widget. Reuse these native mechanisms; publishing a model selection alone does not prove their agreement. See N4, N5, and `multi_contents_view.cc:503-512`.
5. The native focus controller captures Profile and focuses the active contents pane. Accessible bubble focus includes platform-specific macOS widget activation. The inspected web-app app-info command obtains active WebContents for PageInfo. These are concrete integration callers, not an exhaustive credentials, security, accelerator or platform audit. See N6, N8.

The [Arc multitasking guide](https://start.arc.net/master-multitasking) documents split creation inside one window. [YAB Split View](https://yetanotherbrowser.com/manual/split-view) explicitly associates address, reload, find and zoom with the last-clicked pane. YAB also describes copying some saved pages into splits; that conflicts with this fork's reference-only lifecycle approach and is not adopted.

[YAB Peek](https://yetanotherbrowser.com/manual/peek) documents a tab-associated overlay, dismissal, underlying-page return and promotion. Its [0.36 changelog](https://yetanotherbrowser.com/changelog/0.36) describes find commands acting on Peek. [Arc macOS notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes), January 11, 2024, and [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes), February 22, 2024, document Peek promotion/dismissal controls. These documentary facts support investigating the semantics. They do not settle this fork's lifetime policy, native safety, accessible presentation or Linux behavior. Sources were retrieved at the timestamp above. No reference app ran.

## Proposed target contract

An active page surface is the live page currently authorized by the host's activation binding to receive page commands. It can be a split pane or Peek. A selected workspace object is the explicit organizational selection, such as an item, temporary object, folder or approved selection set. Neither is inferred from URL/title, the other selection, or a background context's internally active tab.

Actual keyboard focus is separate again. It can be in page content, its omnibox/find controls, revealed chrome, an owned modal, or another host. Moving into controls does not create a new page target. Modal/fullscreen/input eligibility can block dispatch while the underlying active page identity remains known. Workspace selection does not provide a security context.

| Command | Required target | Additional boundary |
| --- | --- | --- |
| Address | Active page's bound chrome and current navigation | Display actual current origin/URL, not saved destination. Editing/typed navigation remains governed by root navigation policy. |
| Reload | Active page | No implicit fan-out to selected workspace objects or both split panes. Preserve native controller/DevTools semantics. Q4 covers a separate bulk action. |
| Find and next/previous find | Active page and its native find controller | Find UI can own keyboard focus while retaining that page binding. Do not reuse another pane's controller. |
| Zoom | Active page's native zoom controller | Preserve Chromium's actual zoom scope. Shared-origin zoom effects do not mean command target fan-out. |
| Security/site controls | Active page's real Profile, WebContents and current document | Show actual origin/security state. Browser-originated prompts retain their originating owner; do not retarget them to current workspace selection. |
| Credential action | Active page's real Profile and permitted current frame/document | Native autofill/passkey/credential authorization remains authoritative. Target choice never grants permission or exposes credentials. |
| Move | Selected workspace object | Use its stable identity, kind and owning Profile/Space. Live or cross-Profile refusal is not permission to copy or remove a source. |
| Keep | Selected workspace object | A selected temporary must pass lifecycle eligibility and resolve its own page. Do not keep a different focused Peek merely because it is active. Unsupported kinds stay unavailable. |
| Archive | Selected workspace object | Target selection does not approve Archive, cleanup, deletion or undo. Root destructive/lifetime policy remains open. |

Context menus and explicit surface controls carry their named object/page target. They must label that scope and must not silently substitute either global default. Host commands use host identity; Profile-wide views use an explicit Profile. This table does not turn every browser command into a page command.

Before an effect, the executor revalidates the existing semantic target plus native TabModel/WebContents, immutable real Profile/BrowserContext, attachment, document/frame where relevant, binding generation and input eligibility on the serialized UI sequence. A stale operation cancels visibly where appropriate; it never replays on the current page. Native navigation, before-unload, permission, modal, private and download authorities stay intact. No fake service/store, alternate classifier or `Model::OpenItem` success can satisfy this boundary.

## Focus entry, exit and cues

Split entry by actual pointer activation or keyboard traversal publishes one active page and the same command semantics. Hover alone does not activate. Split exit retains existing pages and references under lifecycle; Q1 and Q3 settle workspace selection coupling and return behavior. Hidden geometry must be revealed through a supported native presentation before it can claim visible active state.

Peek entry binds active page chrome, origin/security, find and credentials to preview, while the parent remains a distinct live page. After a separately permitted dismissal, return uses a still-live, eligible target and actual native focus success. Promotion preserves the preview's PageId/Profile/native state. No hide, close, retention period, nested Peek route or navigation fallback is authorized here.

Intentional pointer chrome reveal preserves content focus. Keyboard reveal focuses controls while retaining their page binding. Dismissal requests return; failed or delayed return keeps actually focused chrome visible. Follow the existing focus-mode component's intentions and current host facts. Modal entry takes native input precedence; modal exit revalidates its source before return. Fullscreen retains its source owner and native Escape/security controls; denied overlays do not redirect to hidden pages.

Close is an intent until the native owner accepts it. Cancellation preserves current content and focus. After success, lifecycle prunes references and the host publishes an eligible replacement or accessible empty surface. Q2 and Q3 settle surface choice and fallback, without granting root-owned destructive policy.

Visible cues must distinguish the active page surface from selected workspace objects using more than color alone. Bind the active pane/Peek indicator and relevant address/security controls to the same identity. Accessible names/states must expose the actual active surface, selected organizational object and focus destination through native semantics. Announce meaningful active-target changes without reading private content or manufacturing successful focus. Keep browser-originated modal ownership and fullscreen origin cues. Exact shape, placement, wording and OS accessibility mapping belong to the platform/accessibility owners. No message is added; any future text in this leaf must use `IDS_ARCIFIED_FOCUS_ROUTING_CONTRACT`.

## Synthetic scenario table

[routing-fixtures.json](routing-fixtures.json) is the review source. Each row applies all six page command families and all three workspace families through the explicit targets shown below. `eligible` means the target can be considered after feature/native checks, not that move/keep/archive or navigation is implemented. `blocked` retains target identity but withholds dispatch. `cancel-stale` cancels an already captured command instead of using the newly active target. Every Linux, Windows and macOS result is OPEN.

| Fixture | Focus transition | Page target after transition | Workspace target | Dispatch and unresolved choices |
| --- | --- | --- | --- | --- |
| `single-page` | page-a to page-a | page-a | item-a | eligible |
| `split-pointer` | page-left to page-right | page-right | item-left | eligible; Q1 |
| `split-keyboard` | page-right to page-left | page-left | item-right | eligible; Q1 |
| `split-hover` | page-left to page-left | page-left | item-left | eligible |
| `split-address` | page-right to omnibox-for-page-right | page-right | item-left | eligible |
| `split-exit` | page-right to page-right | page-right | item-left | eligible; Q3 |
| `peek-enter` | page-parent to page-preview | page-preview | item-parent | eligible |
| `peek-exit` | page-preview to page-parent | page-parent | item-parent | eligible; Q3 |
| `peek-parent-gone` | page-preview to empty-host | none | item-parent | unavailable; Q3 |
| `peek-promotion` | page-preview to page-preview | page-preview | item-parent | eligible |
| `chrome-pointer` | page-right to page-right | page-right | item-left | eligible |
| `chrome-keyboard` | page-right to chrome-controls | page-right | item-left | eligible |
| `chrome-dismiss` | chrome-controls to page-right | page-right | item-left | eligible |
| `chrome-return-failed` | chrome-controls to chrome-controls | page-right | item-left | eligible |
| `modal-enter` | page-right to modal-for-page-right | page-right | item-left | blocked |
| `modal-exit` | modal-for-page-right to page-right | page-right | item-left | eligible |
| `modal-switch-refused` | modal-for-page-right to modal-for-page-right | page-right | item-left | blocked |
| `fullscreen-enter` | page-right to page-right | page-right | item-left | eligible |
| `fullscreen-reveal-denied` | page-right to page-right | page-right | item-left | blocked |
| `fullscreen-exit` | page-right to page-right | page-right | item-left | eligible |
| `close-cancelled` | page-right to page-right | page-right | item-left | eligible; Q2 |
| `close-active` | page-right to page-left | page-left | item-right | eligible; Q2, Q3 |
| `close-explicit-inactive` | page-right to page-right | page-right | item-left | eligible; Q2 |
| `close-last-page` | page-a to empty-host | none | item-a | unavailable; Q2, Q3 |
| `queued-transfer` | page-a to page-b | page-b | item-a | cancel-stale |
| `queued-aba` | page-a to page-a | page-a | item-a | cancel-stale |
| `cross-profile` | page-work to page-home | page-home | item-work | eligible |
| `host-deactivated` | page-right to outside-host | page-right | item-left | blocked |
| `folder-selected` | page-right to workspace-folder | page-right | folder-a | eligible; Q1 |
| `reload-multiselect` | page-right to workspace-multiselection | page-right | selection-a-b | eligible; Q4 |
| `unsupported-route` | empty-host to empty-host | none | none | unavailable |

Synthetic IDs and event ordering are illustrative fixture inputs, not a new Model schema, runtime protocol or approved default. The fixture audit checks coverage and internal consistency. It does not execute a router, browser, OS accessibility tree or owner decision.

## External activation and missing initiators, compatibility Q2

The compatibility #59 packet is in independent review. Its exact Q2 and D1 records are copied with attribution into [compatibility-consolidation.json](compatibility-consolidation.json); [compatibility-inputs.sha256](compatibility-inputs.sha256) pins the two read-only sibling inputs. This consolidation changes no compatibility file.

Proposed targeting contract:

1. Preserve an exact known originating or registered Profile independently of page, native dialog parent, selected workspace object and last-active host. A surviving host context and Space must use that same actual Profile before they can be candidates. A Profile identity alone does not choose the host/Space.
2. When the originating page/context/host/attachment is still live and eligible, target that exact binding only after native identity, generation, document/action scope and activation validation. Failure never silently retargets an operation. No native success is claimed here.
3. With no originating page, or a notification/PiP initiator that no longer exists, leave the route OPEN/unavailable pending Q5. Recommend an explicit destination choice restricted to the exact known Profile if the owner elects to support fallback. Do not auto-select a surviving or last-active host/Space, reopen a URL, clone a saved page or create a host.
4. With no exact originating/registered Profile, leave the route OPEN/unavailable pending Q6. An explicit Profile-and-Space chooser is an owner option, not an approved workflow. Never guess a last-active/default Profile or derive it from workspace selection, URL/title or a native window used merely as a dialog parent.
5. Q3's eligible parent/sibling/empty-host sequence covers local focus return after a permitted surface transition only. It grants no authority for incoming OS links, notification clicks, PiP return or native dialog parenting. Preserve existing notification registrations, browser URL/security checks and download/private classifiers.

Compatibility D1 is **SOURCE_RISK_NOT_RUNTIME_REPRODUCED**. The sibling packet reports that extension Save As may have no associated WebContents; absent an owning native window, the inspected path can choose a last-active browser window of any Profile, while DownloadPrefs still uses DownloadItem's BrowserContext. This is a candidate native parent/context mismatch to investigate, not a reproduced UI-routing defect, data-access leak or proof of changed download security context. Downloads/window-chrome owners need actual dialog-parent handle, DownloadItem BrowserContext, completion/cancellation and focus-return evidence with Profile A owning the request and Profile B last active. Do not change download classification or adopt that fallback as policy.

The added external-route fixtures preserve known exact Profile separately from an unresolved target. Every availability is unavailable and every native result remains OPEN; these are contract/evidence states, not reproduced platform failures.

| Fixture | Known exact Profile | Origin condition | Contract availability and questions |
| --- | --- | --- | --- |
| `os-link-known-profile` | profile-work | no-originating-page | unavailable, OPEN; Q5 |
| `os-link-unattributed` | unresolved | no-originating-page | unavailable, OPEN; Q6 |
| `notification-page-gone` | profile-work | initiator-missing-host-survives | unavailable, OPEN; Q5 |
| `notification-host-gone` | profile-work | initiator-and-host-missing | unavailable, OPEN; Q5 |
| `pip-initiator-gone` | profile-work | initiator-missing | unavailable, OPEN; Q5 |
| `origin-generation-stale` | profile-work | initiator-native-binding-replaced | unavailable, OPEN; Q5 |
| `extension-saveas-no-contents` | profile-work | associated-webcontents-absent-unrelated-profile-last-active | unavailable, OPEN; Q5 |
| `exact-live-origin` | profile-work | initiator-live-native-validation-not-run | unavailable, OPEN |

## Questions for coordinator consolidation

Q1. When pointer or keyboard focus enters a split pane, should workspace selection stay on the explicitly selected object until an explicit workspace selection, or also select that pane's workspace object?

Recommended: Keep workspace selection independent and visibly label both active page and selected object. Tradeoff: Stable organization targets avoid silently moving/archiving another object, but users need clear distinct cues.

Q2. For target choice only, should generic Close request dismissal of the active Peek, otherwise close the active pane/page, or require explicit surface close controls?

Recommended: Route generic Close to the active surface's close/dismiss intent, keeping Archive a separate selected-object command. Tradeoff: Consistent surface targeting is convenient; explicit controls reduce ambiguity. Any actual hide/close, protected-work handling and undo still require root lifecycle policy and native completion.

Q3. After a permitted dismissal or close invalidates focus, should return prefer an eligible live Peek parent, then a surviving visible sibling, then an accessible empty host, or wait in the empty host for explicit selection?

Recommended: Use eligible parent, then visible sibling, then empty host. Do not reopen, clone, or activate a hidden/background page as fallback. Tradeoff: Automatic return preserves continuity but requires visible, live, permitted targets and native focus acknowledgement. If several siblings qualify, ordering remains with the split/tab-order owner.

Q4. Should the first production scope expose a separately labeled Reload selected action for explicit multi-selection, or leave bulk reload unavailable while ordinary Reload remains active-page-only?

Recommended: Leave bulk reload unavailable in this scope and retain active-page-only default. A future explicit bulk command needs its own review. Tradeoff: This matches issue 44 but differs from Chromium's current multi-selection ReloadInternal behavior. Preserving bulk convenience requires a distinct action rather than implicit default fan-out.

Q5. When an incoming OS link has no originating page, or a notification/PiP initiator no longer exists, which surviving host/Space should receive it?

Recommended: Keep these routes unavailable until owner settlement. For a known exact originating/registered Profile, recommend an explicit destination choice restricted to a host context and Space with that same real Profile, after native liveness/attachment validation. Do not automatically select a last-active host/Space or create one. Tradeoff: Avoids silently targeting an unrelated context. An explicit same-Profile destination choice adds interaction and still needs root's native ownership proof; no destination policy or approval exists yet.

Q6. If an incoming OS link has no resolvable exact originating or registered Profile, should an explicit Profile-and-Space chooser be supported, or should the route remain unavailable?

Recommended: Keep the route unavailable pending an owner-approved attribution/selection workflow. Never infer a Profile from last-active state, workspace selection, URL/title or an unrelated native dialog parent. Tradeoff: Avoids invented attribution and credential-context changes; unattributed OS-link UX remains incomplete until the owner chooses scope and native validation is implemented.

Root already owns typed/same-site/non-link navigation, cleanup, private persistence, destructive confirmation and source disposition. Those remain dependencies rather than duplicate owner questions. No owner was contacted by this leaf.

## Native integration requests and acceptance

1. Commands/window ownership owners must expose the actual authoritative activation binding and reuse the existing PageCommandTarget seam. Bind retained Profile chrome, find/zoom/site/credential services, accelerators and macOS menus coherently. Reject stale callback generations and inactive hosts.
2. Split, Peek, chrome, fullscreen and accessibility owners must implement and verify the approved transitions/cues after coordinator records Q1-Q6. Root must settle the permitted dismissal/lifetime policy first. Preserve current classifier authorities and existing lifecycle/metadata codecs.
3. Coordinator/root must reconcile ordinary Reload with N2. Do not claim active-only behavior by calling an existing multi-selection path unchanged.
4. After real production binding exists, add meaningful registered native browser/interactive tests for these routes. Exercise distinct split origins, Peek-origin credential/security UI, modal cancellation, fullscreen Escape, empty hosts, transfer, ABA/replacement, native menus and accessibility focus. Verify same-window multi-Profile isolation and live saved-page state handoff directly. These tests need the real host and exact production patch, not a synthetic replacement.
5. Run authorized installed native scenarios on Linux, Windows and macOS and record exact build/source/patch identities, visuals and accessibility observations. Browser/visual execution, builds and native tests were not authorized for this leaf.

| Platform | Evidence available in this packet | Native state |
| --- | --- | --- |
| Linux | Exact prepared 155.0.8059.79 source inspection and hashes; foundation source | OPEN. No build, launch, interaction, visual or AX execution. |
| Windows | Documentary Arc Windows references and inspected shared source | OPEN. No Windows native build or interaction. |
| macOS | Documentary YAB/Arc references and inspected shared source with macOS branch | OPEN. No reference execution or macOS native build/interaction. |

A native patch, changed result-source files, GN registration and native test changes are inapplicable to this decision leaf before owner settlement and the root host/lifetime proof. All prepared sources remain unchanged; sources.json records base identities and the absence of a changed result variant. Command 40744 is reserved only. No legacy command is installed, so no ActionItem mapping is added; an eventual legacy registration must include the actual mapping and preserve Library 40701. Prefer ActionItems for any other new action or request a coordinated ID. Newly authored executable packet code uses GPL-3.0-only. Existing upstream licenses remain untouched.

Research, source identity and fixture coverage checks are recorded in [checks.md](checks.md). Owner approval, native implementation/test registration, same-window multi-Profile isolation, saved-page live handoff, platform UX/accessibility and release evidence remain open. Independent review belongs to the coordinator.
