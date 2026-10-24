<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Focus routing contract independent review, round 1

Reviewed 2026-10-24 for [issue 44](https://github.com/Timpan4/helium-arcified/issues/44), at coordinator baseline `a4b6b480d3de07857ab6e939149254a8359e5449`.

Disposition: Clean bounded documentary review. No actionable findings. The frozen packet is suitable for research aggregation and owner consideration. This does not close issue 44, approve its proposed defaults, or establish native routing, focus, security, accessibility or platform acceptance.

## Scope and identity

I reviewed all eleven files in `arcified/native/production/focus-routing-contract/`, the complete authoritative assigned issue object, the relevant current root workstream controls and execution state, lifecycle v3, the native ownership design, the existing command-target header/implementation/fixtures, and the Peek, focus-mode and layout-composition foundations. I inspected the cited prepared-source command/focus seams read-only. I also compared compatibility Q2/D1 with their exact sibling inputs and read the clean bounded [compatibility review](compatibility-review-round-1.md).

All eleven packet SHA-256 identities matched the review request:

| File | SHA-256 |
| --- | --- |
| `ownership.json` | `33d98514a95d6011006a2df17e67978e46817f11a030461b42a3cb8ed5d59d31` |
| `authority.json` | `8a5658555706f093541f3947fb586baf9b7de607fbb6e893e4deaa46162e3886` |
| `sources.json` | `47afd19bc97e309fec72f07dae577cf30cb09f050419f0b2557fb67fa7ae7ab2` |
| `identities.sha256` | `0f319de075c4f9914443ae4720f6f26ea9a4a973ebe21f93cdebb1b442bf4df3` |
| `routing-fixtures.json` | `5226cc6e0aa012a14041ef2dd4d4b23a6cc4f8cdff1d7a0c9e6ac1127f5970f1` |
| `decision.md` | `ce3ce49000bf1cc031b02a66fe71a80b7169aefc10f490ba0fd1a85e633946a8` |
| `verify-packet.mjs` | `c8e9cd067ce42019c06f9f5cc796320aa1514adcff12da492f346ae251ac7e42` |
| `result.json` | `14280bc80fd0e762a0a49fe3558207745b3434a4d9dd17fa14f32da24bd0dbec` |
| `checks.md` | `4ae4ab1510d657d0eb2a6417d7af4660da7be44d5f64e8fa9ba54f7597b9e006` |
| `compatibility-consolidation.json` | `3d7d6706a9c21c6cdb400f6cc1d095eae858778053b7703b1c1562cab2f3ea3a` |
| `compatibility-inputs.sha256` | `8190e07975d015cad3f5b9ebbf1b62df561a8ede05cbf3c039a76af7a9a63047` |

The issue object equals the `focus-routing-contract` object in `/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/arcified/docs/production/research/assignment.json`, including every acceptance criterion, dependency, reference limitation and command allocation.

The twenty-input identity check returned exit 1 because two live root-control files changed. Eighteen matched, including every prepared-source/version file and every listed foundation:

| Live root file | Packet snapshot | Observed during review |
| --- | --- | --- |
| `non-ai-production-workstreams.json` | `daf98f1cf993c1a21686289a88489bf001266d7fff4d85089d30055584c8a7b0` | `50745712bcffcfc79f526270512b41e23a6aef1cfe76547058d63b33faa1d9c1` |
| `execution-state.json` | `dae2aed7885b1f03f5f96a1285265a1189b8776a6472880c0f12c94de7f7ad02` | `4a91383f8ed5ed3ae8477f21a1900fd28c9ede91f4591d7fbe9f23d30bf4cbe9` |

I reconciled their current relevant controls. They still assign this research decision to the coordinator, reserve 40744 with Library at 40701, retain root ownership of heavy/native work, make the prepared tree read-only for other threads, and prohibit competing models, registries, codecs and classifiers. Current owner decisions preserve shared saved items, local temporaries, exact live-page handoff, same-native-window cross-Profile selection, Profile settings scope and outbound-link branching. Activity, cleanup, destructive action and populated migration decisions remain open. No relevant conflict with this packet was found. Earlier whole-file bytes were not reconstructed. These are historical provenance limits, not evidence of failed focus behavior or a reason to rewrite shared controls.

## Acceptance assessment

The active page, selected workspace object and actual keyboard focus are separate throughout the proposed contract. Address, ordinary Reload, Find, Zoom, site controls and page credential actions use the host-authorized active page. Move, Keep and Archive use the explicit workspace selection and retain lifecycle/kind eligibility. Context menus and explicit controls retain their named target rather than silently substituting the global target. Profile-level and host-level commands remain separately scoped. The split, Peek, folder, multiselection and cross-Profile examples preserve this distinction.

The contract records entry and exit for split, Peek, chrome reveal, modal, fullscreen and close. It covers pointer and keyboard activation, hover, failed chrome focus return, modal refusal, denied fullscreen reveal, close cancellation, inactive explicit close, and empty-host return. Active-page cues and workspace selection cues must differ by more than color; accessible state must describe actual active identity and focus. Pending or failed focus does not become a successful focus claim. Native modal/fullscreen security and Escape ownership remain authoritative. Exact platform presentation remains delegated to platform/accessibility owners without changing target semantics.

All 31 focus scenarios have explicit before/after focus, active page, workspace selection, target pair, eligibility/cancellation state, visible cue, accessible cue, owner-question references and three OPEN platform results. The eight external-route rows separately cover known-Profile and unattributed OS links, missing notification page/host, missing PiP initiator, stale native generation, extension Save As without WebContents, and exact live origin without native validation. All external targets remain unresolved/unavailable with OPEN policy and platform states. The tables describe proposed constraints and illustrative states. They are not an executable router or evidence that any of these commands worked.

The stale/native boundary is explicit. The existing `PageCommandTarget` resolves a caller-supplied page and preserves Profile/attachment. It does not infer selected Space or detect native replacement and away-and-back ABA. Its existing fixtures demonstrate that limitation. The packet therefore requires actual TabModel/WebContents, exact immutable Profile/BrowserContext, attachment, relevant document/frame, generation, input eligibility and serialized UI-sequence revalidation before effects. Queued work cancels rather than replaying against a newly active page. Actual host activation and live-page transfer must succeed before semantic publication. No new registry, Model, metadata codec, classifier, fake native patch or model-success substitute is introduced.

## Source assessment

The inspected prepared-source seams support N1 through N8 within their stated bounds:

1. `browser_window_interface.h:83` fixes the desktop window's real Profile and homogeneous tab ownership. The packet correctly rejects a changing Profile getter as a routing solution and retains the root ownership extraction gate.
2. `browser_commands.cc:742` confirms the important Reload distinction. A split alone follows the active-tab path, while explicit multiple UI-tab selection can fan out through `ReloadInternal`. The packet preserves that source finding, requires ordinary Reload to remain active-page-only, and requests integration work rather than claiming the current generic call already satisfies issue 44. Q4 concerns only a separately labeled bulk action.
3. The cited password, Find, Zoom and address entry points use active WebContents or their Browser-owned controllers. `FindBarController::HandleActiveTabChanged` changes observed contents and saved-focus handling. BrowserView derives active contents from its tab strip and checks location-bar activity/visibility before Zen reveal/focus. These are concrete callers, not a complete credential/security audit.
4. `MultiContentsView` changes primary contents, borders/overlay and traversal order, and restricts inactive-pane focus notifications to a visible widget. The native focus controller captures a Profile, focuses active contents and has a macOS-specific accessible bubble activation path. The app-info command supplies one inspected PageInfo entry point. None proves a coherent Arcified activation binding or installed accessibility behavior.

I also retrieved the cited public documentary sources without browser UI interaction. The [Arc multitasking guide](https://start.arc.net/master-multitasking) supports split creation inside one window. [YAB Split View](https://yetanotherbrowser.com/manual/split-view) supports last-clicked-pane command targeting and documents saved-page copying that the packet expressly declines to adopt. [YAB Peek](https://yetanotherbrowser.com/manual/peek) supports the overlay association, dismissal/return and promotion descriptions. Its [0.36 changelog](https://yetanotherbrowser.com/changelog/0.36) supports the narrow Find-in-Peek claim. Direct opening of that changelog initially failed; following the official changelog index link succeeded. [Arc macOS notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes) at January 11, 2024 and [Arc Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes) at February 22, 2024 support the stated historical Peek controls. These documents do not approve downstream lifetime defaults, establish Linux behavior, prove runtime parity, or turn vendor claims into measured results.

## Compatibility and owner boundaries

The consolidated compatibility Q2 is an exact copy of the sibling question. D1 is also preserved as the attributed candidate, including its Linux/Windows scope, reproduction prerequisites, evidence request and `SOURCE_RISK_NOT_RUNTIME_REPRODUCED` classification. Both sibling input hashes matched. The clean compatibility review supports the documentary source-risk characterization; this review did not rerun or independently inspect the underlying download seam. There is no runtime defect record or claim of a credential/data leak. Dialog-parent identity remains separate from the DownloadItem BrowserContext and download/private classification.

Q5 preserves an exact known originating/registered Profile while leaving host/Space choice unsettled. Q6 separately handles absent Profile attribution. Neither adopts the last-active/default Profile, selected workspace object, URL/title or an unrelated native parent as authority. Q3's local return recommendation does not authorize external-event fallback, page reopening, cloning or host creation. The eight external rows preserve these boundaries even where an initiating host survives.

All six owner questions remain `UNRESOLVED`. Q1 does not automatically approve selection coupling, Q2 does not approve close/dismiss lifetime behavior, Q3 does not approve a return default, Q4 does not reopen the required ordinary active-page Reload semantics, and Q5/Q6 do not implement a destination chooser or fallback. Recommendations include tradeoffs and require coordinator/owner settlement. Existing approved lifecycle behavior is retained; root-owned cleanup, private/guest persistence, non-link navigation, destructive handling and source disposition remain open.

## Independent light verification and remaining gates

The following checks ran from the coordinator worktree and prove only the stated local scope:

1. `git rev-parse HEAD` matched the assigned baseline.
2. `sha256sum` over all eleven packet files matched the frozen review inputs.
3. `node --check arcified/native/production/focus-routing-contract/verify-packet.mjs` exited 0 without diagnostics.
4. `node arcified/native/production/focus-routing-contract/verify-packet.mjs` exited 0 and reported 31 synthetic focus rows, eight external-route rows, nine declared command families and OPEN platform results. This audits artifact formatting, coverage and internal consistency; it does not execute those routes.
5. `sha256sum --check arcified/native/production/focus-routing-contract/identities.sha256` matched eighteen inputs and exposed the two live-control differences reconciled above. All nine prepared-source/version identities matched.
6. `sha256sum --check arcified/native/production/focus-routing-contract/compatibility-inputs.sha256` exited 0 for both inputs.
7. Hosted JavaScript comparison confirmed full issue-object equality, exact compatibility Q2 copying, all 39 scenario records' platform states, and all six unresolved owner-question states. Manual review compared the complete scenario inventory and source findings with all three acceptance criteria.

Owner settlement and real native evidence remain required. Commands/window ownership must supply the actual coherent activation binding and reconcile multiselect Reload. Split, Peek, chrome, fullscreen and accessibility owners must implement approved transitions and cues. The existing native security, credential, URL, before-unload, private and download boundaries must remain intact. Root must prove same-actual-window multi-Profile isolation and exact live saved-page handoff, then obtain native command, stale-document/generation, modal/fullscreen/close, visual and accessibility evidence on Linux, Windows and macOS. Compatibility owners still need actual missing-initiator and D1 reproduction evidence. Missing proof that is honestly OPEN is a verification limit of this decision packet, not an actionable defect.

The coordinator supplied root's Linux baseline status as BUILT at Helium 0.19.2.1 / Chromium 155.0.8059.79 with matching ChromeDriver. I did not independently verify that build or run it. It supplies no focus test result. Older root execution prose about compilation still running is historical; it is not treated as a current build failure. Feature compilation, runtime, UX and native acceptance remain OPEN.

No native setup, build, test, benchmark, remote CI, browser/visual interaction, remote write, staging, commit, PR linking/watching, owner contact or nested delegation occurred. Prepared source, packet and shared controls were not modified. This report is the only authored file.
