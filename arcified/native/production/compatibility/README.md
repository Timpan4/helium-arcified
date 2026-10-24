<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Native compatibility research, issue 59

This packet defines the compatibility evidence to collect for a declared release candidate. It does not declare one. Native acceptance is **OPEN**. The matrix covers 20 workflow scenarios on Linux, Windows and macOS, with all 60 platform cells OPEN. Zero native passes or runtime failures are recorded.

The assigned outcome is aggregation and tracking. A native patch, changed native result file, GN registration, command or message is inapplicable here. No native implementation defect has been reproduced, and feature owners already own the implementations. Adding a fake adapter or a standalone success fixture would not satisfy issue 59. There is no use of command 40728, no ActionItem mapping or message resource to register. Library 40701 and patch order are untouched. A later compatibility UI would require the assigned 40728 plus the real declarative ActionItem mapping, or pure ActionItems, and the IDS_ARCIFIED_COMPATIBILITY prefix.

## Authority and available evidence

[issue-object.json](issue-object.json) preserves the complete assigned issue object, including acceptance criteria, prerequisite keys, reference limitations and source URLs. It came from root's assignment file, not a new GitHub fetch. [source-identities.json](source-identities.json) records exact SHA-256 identities for authority/foundation inputs and the prepared source files inspected on 2026-10-24.

The root authority files are in `/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/arcified/docs/`:

1. `production/research/assignment.json` assigns compatibility as a tracking leaf.
2. `non-ai-production-workstreams.json` fixes baseline `a4b6b480d3de07857ab6e939149254a8359e5449`, read-only prepared-tree access and coordinator ownership.
3. `lifecycle-contract.md`, semantic contract v3, preserves immutable page/Profile identity, shared saved ownership, local temporaries and separately approved outbound-link branching. Its fixtures prove modeled transitions only.
4. `native-space-integration.md` requires same actual OS window across differently profiled Space switches and preservation of the actual live page during handoff. Separate windows, hidden backing windows, metadata relabeling and `Model::OpenItem` success cannot satisfy these gates.
5. `execution-state.json` reports a prepared Linux source tree and root-owned baseline compilation, with native browser launch/tests and platform acceptance OPEN. This is a recorded authority snapshot, not a fresh assessment of the ongoing build.

The prepared source root is `/home/timpan4/.t3/builds/arcified-linux-155-native-v1/src`. S01, `chrome/VERSION`, records Chromium 155.0.8059.79 and Helium version fields 0.19.2.1. The source baseline is not an installed RC. The source hashes identify the prepared downstream bytes, not an unpatched Chromium tag. No prepared file was written or native build/test command run.

The foundation `arcified/docs/research/arc-dia/artifacts.md` records static inspection of Arc 1.170.0 build 89123 and Dia 1.54.2 build 89247 macOS images. Their hashes and REA limitations are retained there. No reference app was executed, and the static menu/bundle inventory proves neither compatibility nor current reachable behavior. The existing publication `compatibility.md` carries the same native criteria. These inputs were reviewed, not reacquired. No external facts were needed beyond local primary source and recorded foundation evidence; no installers were downloaded.

Root's recorded standalone lifecycle, persistence, layout and site-control checks remain reusable for their existing pure scope. They are not rerun for this tracking packet and do not count as native compatibility results.

## Source-supported findings

Source IDs below resolve to full paths and byte identities in [source-identities.json](source-identities.json). Line references describe those exact prepared files. These findings identify seams and risks, not working product behavior.

| Evidence | Supported fact | Consequence for the matrix |
| --- | --- | --- |
| S02, `web_app_launch_process.cc:56-97` | Tabbed-browser lookup uses a Profile-specific collection, activation order and optional display filtering; protocol handling translates an installed app's registered URL. | Exercise app launch from both browser and OS entry under competing Profile activation. This file alone does not prove install/uninstall or generic inbound-link selection. |
| S03, `extension_popup.cc:275-341` | Popup construction retains a browser, uses its Profile, observes its tab strip and extension registry, and focuses the extension contents. Showing is refused over visible security dialogs. | Test popup ownership and stale context, keyboard return and security-dialog protection. Do not weaken those checks for compact chrome. |
| S04, `autofill_popup_controller_impl.cc:321-350` | Root popup display checks requesting contents focus and resolves a frame token/focused frame. | Suggestion display and selection must stay bound to the initiating frame. Add context-switch and iframe observations, not just popup visibility. |
| S05, `chrome_authenticator_request_delegate.cc:871-877,1199-1203,1271` | WebAuthn gets BrowserContext/Profile from the requesting RenderFrameHost, resolves the browser containing its WebContents, and records OTR state. | Separate address/password autofill from passkey creation/use. Record actual OS/authenticator/provider capabilities; no synced credential service is implied. |
| S06, `print_preview_dialog_controller.cc:99-100,128-145,198-215` | Preview retains a weak initiating WebContents, resolves its modal host and accounts for initiator destruction during dialog creation. | Preview and the OS print dialog need separate observed results, including cancel/context-switch and initiator lifetime. |
| S07, `download_file_picker.cc:38-60,77-93` | Preferences use the DownloadItem BrowserContext; ordinary downloads require a native view; a Linux dialog factory may return null. Linux/Windows have a fallback to the last-active browser of any Profile. | Test page and extension downloads separately. Candidate D1 is isolated in [defects.json](defects.json). Unavailable Linux pickers are not passes. |
| S08, `non_persistent_notification_handler.cc:54-72,92-125` | Click dispatch has a completion callback. If the document handler is gone, fallback navigation uses the supplied Profile and opens a foreground tab. | Test both live-document handling and missing-document fallback. This source does not specify which Arcified host/Space should be chosen. |
| S09, `persistent_notification_handler.cc:174-211` | Persistent clicks consult notification permission on the supplied Profile and reject missing permission before dispatch. | Include revocation and service-worker behavior. Permission races cannot become permission bypasses. |
| S10, `external_protocol_handler.cc:215-286,518-521` | The outbound path obtains initiating WebContents, requires a modal manager before prompting and preserves the security-dialog path; Profile comes from the page context. | Test outbound prompts separately from inbound OS/PWA links. Never call the unchecked launch route to manufacture compatibility. |
| S11, `media_router_feature.cc:44-55,75-80,104-130` | Media-router availability includes feature and BrowserContext preference/policy checks. | A source feature default is not enabled discovery, receiver access or successful casting. Record actual build/runtime/policy/network/receiver states. |
| S12, `picture_in_picture_window_manager.cc:230-241,273-275` | Return-style behavior delegates close/focus to the initiating PiP controller. | Observe the actual video/document initiator after a context switch and approved handoff. Do not substitute the most recently selected workspace item. |
| S13, `widevine.gni:9-59`; S14, `third_party/widevine/LICENSE`; A08, `flags.gn` | The fork asks for Widevine support. Upstream GN separately gates library availability, component support and bundling. Its CDM license requires a separate Google license for use/distribution. | Build support is not a CDM, license, codec or service entitlement. Arcified's explicit rule forbids proprietary CDM bundling. No binaries, signatures, secrets or provisioning route were copied or enabled here. |

These are focused seams, not an exhaustive consumer inventory. Root owns the lifecycle/ownership inventory and its unresolved construction/reentry proofs. The focus-routing-contract area owns active-surface rules and fallback decisions. This packet requests evidence from those owners instead of defining a second Model, page registry, codec, private classifier or download authority.

## Matrix and release identity

[compatibility-matrix.json](compatibility-matrix.json) is the finite scenario inventory. Each row gives the exercise, observable expected result, owner, acceptance criterion, source IDs, variants and explicit per-OS result. Install, launch and uninstall are separate rows. Password autofill, address autofill and passkeys are separate. Preview and OS print dialogs, ordinary and extension downloads, document and worker notifications, inbound and outbound protocols, video and document PiP, and absent/optional Widevine configurations are also separate.

Before aggregating a pass, root must supply a declared RC ID, exact fork/integrated patch identity and installed artifact identity for each OS. The matrix lists required identity and observation fields. An effective GN configuration must be redacted, and test fixtures must use synthetic data. Record native top-level identity continuously across same-host Profile switches as required by root; a title, screenshot or logical WindowId alone is insufficient.

Apply each row's named variants. Baseline and competing-window/Profile behavior cannot stand in for same-native-host multi-Profile behavior. Where a row depends on approved saved-page handoff, keep that variant OPEN until production handoff is proven and then test the real retained document. Include pending dialog/callback context changes, cancellation and requester closure where safely supported. Private variants follow existing classifier and feature availability. An unsupported route stays unavailable with evidence and an owner disposition.

A native result needs exact requesting and actual receiving page, document, Profile/OTR state, Space and host, with selected workspace object recorded separately from active page surface. Preserve normal URL/origin checks, credential verification, before-unload, dialog, private and download classifications. No cleanup, destructive action, source-disposition, provider or retry policy is added.

| Platform | Current state | Platform facts and limitations to retain |
| --- | --- | --- |
| Linux | Every scenario OPEN. Prepared source inspection only. | Record actual distro/kernel, architecture, X11/Wayland and native picker/portal/print/notification availability. S07 explicitly permits missing file-dialog creation. Fork flags disable mDNS and service discovery, so discoverable Cast support cannot be assumed. Verify effective build/runtime state rather than enabling services here. |
| Windows | Every scenario OPEN. No installed candidate or runtime evidence supplied. | Record OS build, package architecture/signing, OS handler registration, authenticator/credential UI and print/download ownership. S07's any-Profile parent fallback applies here too. Linux compilation cannot verify Windows behavior. |
| macOS | Every scenario OPEN. No installed candidate or runtime evidence supplied. | Record OS/build, architecture/signing/notarization, native menu/handler/credential/print/notification permissions and app focus. Static Arc/Dia macOS artifacts do not prove Arcified behavior. S07's Linux/Windows fallback must not be attributed to macOS. |

S13's library-CDM availability includes Linux/macOS x64 and arm64, and Windows x86/x64/arm64, subject to the GN gates. That is source availability, not the downstream supported-release architecture list. For every declared OS artifact record absent CDM first. The optional row remains OPEN unless a lawful, owner-authorized route and media are supplied. Lack of DRM support is a recorded limitation, not a reason to bundle a CDM or bypass prompts. License exchange, codecs and service acceptance can fail independently of key-system availability.

PASS requires observed native evidence on that exact RC and completion of applicable variants. FAIL requires a separate reproducible defect. UNAVAILABLE records why execution or a feature is absent and does not mean PASS or an approved waiver. OPEN retains missing identities, targets, execution or owner decisions. This packet does not invent a release-blocking/waiver policy.

## Separate defects and integration requests

[defects.json](defects.json) contains source-risk candidate D1. It is not a runtime defect or a remote issue. The downloads owner should inspect the extension-without-WebContents path with Profile B most recently active, then record actual parent, owning DownloadItem context, completion and return focus. No credential leak or crash is claimed from the source fallback.

1. Coordinator imports this packet after independent review, supplies candidate identities and connects focused native results from site-controls #38 and window-chrome #35. Their native completion remains a dependency, regardless of existing pure fixture success.
2. Request result artifacts from web-apps #62, extension-workflow #61, credentials-flow #137, print-flow #138, downloads #36, PiP #114 and the media-controls owner. Owners discover and register meaningful native regression tests in their implementation patches; no guessed GN target or unregistered synthetic test is supplied here.
3. Route missing-initiator and unattributed incoming-link behavior to focus-routing-contract #44. Root retains same-window Profile isolation and live saved-page handoff gates. Send candidate D1 to downloads/window-chrome for ownership review and a focused native regression.
4. Preserve unsupported results and separate defects in aggregation. A later RC with changed relevant source/flags/fixtures needs new affected evidence; unrelated unchanged passing evidence can be reused with its exact identity.

## Questions for coordinator consolidation

1. Which exact RC ID and per-OS installed artifact identities should issue 59 evaluate? Recommend root's integrated native candidates, with identity fields filled before any PASS. Source research can finish now; installed compatibility remains OPEN.
2. When an incoming OS link has no originating page, or a notification/PiP initiator no longer exists, which surviving host/Space should receive it? Recommend consolidation with #44, preserving an exact originating/registered Profile where known and keeping unsettled routes unavailable. This leaves fallback UX open without inventing last-active Profile behavior.
3. Which optional Cast receiver routes and lawful Widevine test configuration, if any, belong in the RC claim? Recommend explicit unavailable combinations until authorized hardware/media and a lawful optional CDM route exist. Keep proprietary CDMs out of distribution and retain security prompts.

## Verification and handoff

[checks.json](checks.json) records exact light checks and their scope. [prepared-source.sha256](prepared-source.sha256) can be checked with `sha256sum --check arcified/native/production/compatibility/prepared-source.sha256` from the coordinator worktree. All 14 inspected prepared files matched. JSON parsing, canonical formatting, local references, criterion/scenario/platform coverage and Markdown hygiene are checked separately. No Markdown formatter/linter is declared in the inspected repository configuration; no dependency was installed.

No native setup, preparation, compilation, browser tests, reference-app execution, browser/visual execution, benchmarks, remote CI, accounts, issue mutation, staging or commits were performed. Native test registration and runtime acceptance remain OPEN with their owners. [result.json](result.json) is the coordinator-facing disposition and remaining-work record.
