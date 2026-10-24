<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Automation contract decision packet

Issue: [#25](https://github.com/Timpan4/helium-arcified/issues/25).
Baseline: `a4b6b480d3de07857ab6e939149254a8359e5449`.
Recorded: 2026-10-24. Native acceptance: OPEN.

Recommend an opt-in, browser-initiated native-messaging pilot whose only command is `tabs.list`, returning opaque tab handles in an explicitly chosen normal window and Profile. Keep every adapter unavailable until the owner records approved scope in issue 25 and the applicable native evidence passes. This packet proposes a contract; it grants no permission and enables no endpoint.

## Authority and existing evidence

The complete assigned issue object is retained in [sources.json](sources.json). Root authority was read from the coordinator's `arcified/docs/production/research/assignment.json`, `non-ai-production-workstreams.json`, `lifecycle-contract.md`, `native-space-integration.md`, and `execution-state.json` under `/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/`.

Lifecycle contract v3 remains authoritative. Saved items are shared, temporary tabs are local, live pages retain immutable Profile identity, selecting an already-live saved item transfers that same page, and switching Profiles must preserve the same native window. A successful synthetic model transition proves none of these native paths. Existing metadata, codec, command-target, private and download authorities remain unchanged. Cleanup, destructive actions and source disposition remain unsettled.

The existing [artifact dossier](../../../docs/research/arc-dia/artifacts.md) records Arc 1.170.0 build 89123 and Dia 1.54.2 build 89247 hashes. It observed an Arc scripting dictionary without running either app. This packet reuses that evidence and does not claim to have read the proprietary dictionary or verified its commands. Dia's updater-resource findings establish no automation route. The assigned [Arc Developer Mode URL](https://resources.arc.net/hc/en-us/articles/20468488031511-Developer-Mode-Instant-Dev-Tools) returned 404 on retrieval. The [macOS release notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes) returned no match for AppleScript. Neither establishes Windows or Linux parity.

## Supported mechanisms and product status

"Source present" means inspected code exists. "Unavailable" means this packet supplies no enabled, accepted Arcified adapter. It does not claim that an OS lacks other automation technologies.

| OS | Supported mechanism evidence | Transport, identity and permission requirements | Arcified disposition |
| --- | --- | --- | --- |
| Linux | Prepared native-messaging POSIX launcher and extension Tabs API are present. | Browser creates a helper process with stdin/stdout pipes. Manifest resolves an executable and an allowed extension origin. Extension needs nativeMessaging permission. Package must resolve its actual user-data NativeMessagingHosts location and host executable, including Chrome fallback paths. OS account access alone supplies no per-Profile grant. | Candidate pilot only. No package/helper registration, X11/Wayland observation or end-to-end request. Unavailable. |
| Windows | Prepared native-messaging Windows launcher reads Chromium and fallback Chrome registry keys and supports host process pipes. | Installer must register the correct host manifest under the selected branding and user/machine registry scope. Browser supplies extension origin and a parent-window hint. Neither is an authorization grant for another Profile. Helper execution, packaging and policy permissions must be verified on Windows. | Candidate pilot only. Prepared Linux source is not a Windows build. No COM/UIA, AppleScript-equivalent or native adapter claim. Unavailable. |
| macOS | Prepared native-messaging POSIX launcher and Cocoa AppleScript classes/dictionary are present. | Pilot uses explicit extension/helper installation and a Profile/window grant. Apple events are a separate mechanism whose hardened sender entitlement and OS Automation consent need verification; browser JavaScript permission is separate again. No Accessibility or Full Disk Access requirement is inferred for native messaging. | Prefer the same narrow pilot. AppleScript compatibility is deferred as a separate security audit. No installed app, signing, entitlement or consent result. Unavailable. |

Chromium documents JSON messages over browser-created stdio, allowed extension origins, and nativeMessaging permission. A connected port permits two-way messages; the browser initiates it. This does not provide an arbitrary external CLI listener or authenticate a separate controller app. No such listener is proposed. [Chrome native messaging](https://developer.chrome.com/docs/extensions/develop/concepts/native-messaging).

The Tabs API separates URL, pending URL, title and favicon access from ordinary tab fields. Lack of the tabs permission does not prohibit tab mutations, so an explicit command allowlist is still necessary. Tab IDs are session identities, not durable model identities. [Chrome Tabs API](https://developer.chrome.com/docs/extensions/reference/api/tabs).

## Exact prepared-source findings

All paths below are relative to the read-only prepared source root in [sources.json](sources.json). Base and result hashes are equal because this leaf changes no prepared file. Version file says Chromium 155.0.8059.79. The prepared tree has no Git metadata, so an upstream commit identity was not fabricated.

1. `chrome/browser/extensions/api/messaging/launch_context.cc:143` loads the resolved host manifest, checks its name and allowed security origin, then resolves the executable. Lines 203 onward pass the extension origin; Windows gets a parent-window hint. An origin argument is forgeable when someone launches the helper themselves. It is not cryptographic browser authentication or a caller executable identity.
2. `launch_context_posix.cc:39` searches the fork user location, Chrome user location, fork system location, then Chrome system location. `chrome/common/chrome_paths.cc:454` defines branding-dependent system paths and user-data-relative NativeMessagingHosts paths. A unique host name and inspected package resolution are required; do not copy Google Chrome paths into an Arcified installer by assumption.
3. `launch_context_win.cc:59` checks Chromium branding and then Chrome fallback keys. Its process-launch code can use an executable directly or a legacy shell route. This packet authorizes neither shell dispatch nor a helper that accepts executable names or command text.
4. `native_message_process_host.cc:43` uses Chromium's existing host-to-browser message-size bound. Any eventual framing must use the platform protocol and existing bounds. This decision sets no new quota, timeout, retry or retention limit.
5. `chrome/browser/extensions/api/tabs/tabs_api.cc:1827` implements query. `MatchesWindow` at 1928 filters using the window's Profile and incognito allowance; it then enumerates its TabListInterface. This assumes a Profile-bound browser window. It is not proof that a future same-window multi-Profile host is correctly represented.
6. `chrome/browser/ui/cocoa/applescript/browsercrapplication+applescript.mm:29` enumerates native windows; it does not implement this proposal's per-client grants. `window_applescript.mm` provides tab wrappers. `scripting.sdef:334` declares JavaScript execution. `tab_applescript.mm:368` checks JavaScript and DevTools permission before isolated-world execution. Isolated-world JavaScript is still page access and can mutate state. It must not be treated as a read-only tab-list grant.
7. `native_messaging_apitest_desktop.cc:67` contains existing send/connect native-messaging browser tests. Those tests were inspected as future harness leads, not executed or accepted as Arcified privacy/ownership tests.

## Proposed narrow command contract

The first operation is an inventory of live tabs. It must not open a tab, focus a window, select a Space, transfer a page, refresh content or wake a discarded page.

1. The user chooses a reviewed helper and explicitly enables a connection from trusted browser UI for an actual Profile and actual normal window. The browser binds that connection to those existing authorities. A request cannot supply a Profile name, filesystem path or a different window to broaden access. No foreground-window fallback or global enumeration occurs.
2. Only the exact request object `{"command":"tabs.list"}` is accepted. Missing/unknown fields, arbitrary API method names, script text, shell text and numeric native command dispatch are rejected before enumeration. This is a proposed shape, not an implemented protocol version.
3. The response contains only tab handles for eligible live pages in the granted Profile/window, plus request success or a structured error. Handles refer to existing native page/tab identities within that connection and browser session. Titles, URLs, origins, favicons, DOM, selection, screenshots, cookies, history, saved-item destinations, downloads and filesystem paths are absent. Tab count and organization still reveal browsing activity and require the grant.
4. Do not derive ownership from URL, title, tab position or the OS window's cached Profile. Resolve real page BrowserContext/Profile through the authoritative native owner. Unknown classification, private/incognito, guest and unsupported contexts remain unavailable in this proposed first scope. This exclusion is an automation scope proposal, not a new persistence or private-mode policy.
5. Check grant and scope at execution and response delivery. Revocation, disconnection, window/Profile destruction, handoff out of scope, stale handles or unknown ownership must not produce a successful stale or broader result. Serialize with the native owner's lifecycle operations or return an explicit unavailable/stale result. Do not create a second page registry, snapshot store, Model or codec to simulate this.
6. Expose structured refusals for unavailable adapter, permission denial, revoked connection, missing scope, stale target and unsupported command. Refusals disclose no target existence or metadata outside the grant. No silent fallback to another Profile, another window, AppleScript, debugging protocol or page script.
7. Keep the connection visible and revocable in trusted browser UI. Ending it invalidates its handles and authorization. Grant persistence, audit retention and any external-controller attachment are unresolved, not invented defaults.

The helper is a trusted local program with OS-user privileges, outside the renderer sandbox. Native messaging is not a sandbox for it. A fixed host manifest and allowed origin restrict browser connection routing; they do not attest the helper binary or distinguish same-user attackers. The owner must accept a concrete helper installation/trust boundary before implementation. Pairing, signing and executable identity remain implementation work, and this packet supplies no credential storage policy.

## Separate authorization boundaries

| Capability | First scope | Requirement for later work |
| --- | --- | --- |
| Opaque live tab inventory | Proposed only | Explicit helper/Profile/window grant and native privacy/ownership verification. |
| Titles, URLs, origins and saved metadata | Excluded | Separate named data-scope approval. No implication from tabs.list. |
| DOM, text, selection, screenshots or other page content | Excluded | Explicit page/content scope and ordinary browser security. AI access would additionally need the existing separate user approval. |
| Page JavaScript or content-script execution | Excluded | Separate execution authorization. No generic evaluate endpoint, AppleScript execute passthrough or DevTools proxy. |
| Private/incognito or guest contexts | Unavailable | Separate owner decision and private classifier integration before any adapter route. |
| Focus, navigation, close, move, download or any mutation | Excluded | A named command with its own authorization and native target contract. Cleanup/destructive policies remain open. |
| Remote control or external controller IPC | Unavailable | Separate transport and authenticated-caller design. No TCP, HTTP, WebSocket, D-Bus or generic pipe server is enabled here. |

Remote debugging is unsuitable as the proposed production permission boundary. Chrome documents cookie theft through remote debugging and changed default-data-directory behavior starting in Chrome 136. This is a risk rationale, not a claim that the fork has the same switch behavior. [Chrome debugging security change](https://developer.chrome.com/blog/remote-debugging-port).

Apple's indexed entitlement documentation says the Apple Events entitlement permits prompting to control other apps and describes same-team exceptions. Direct retrieval required JavaScript; runtime TCC and sandbox behavior are unverified. System consent alone would not implement Arcified's per-command/Profile grant. [Apple Events entitlement](https://developer.apple.com/documentation/bundleresources/entitlements/com.apple.security.automation.apple-events).

## Owner decisions for coordinator consolidation

No automation-specific approval was found in the read root owner-decision ledger. Do not interpret general non-AI implementation authorization as this issue's missing security decision.

1. "Approve the proposed opt-in tabs.list pilot, returning only opaque live tab handles in one explicitly granted normal window/Profile over a browser-initiated native-messaging port, with titles/URLs, private/guest access, page content, JavaScript, mutations and external listeners excluded, or defer automation entirely?" Recommendation: approve the narrow design scope only. It provides a bounded integration target but is less useful than URL/title automation and remains disabled until native gates pass. Deferral avoids introducing a helper trust boundary.
2. "May the pilot trust an explicitly selected, reviewed local native helper running as the OS user, or must we defer until a stronger caller/binary-attestation design is selected?" Recommendation: require a concrete reviewed helper identity and package-resolution plan before enabling it. Same-user tampering and distribution/signing remain risks; allowed_origins alone does not resolve them.
3. "Approve connection-lifetime grants only for the first pilot, with no remembered grant or external-controller reconnect, or require a separately designed persistent grant?" Recommendation: connection lifetime only. Repeated setup adds friction but avoids inventing credential storage, grant lifetime or background access policy.

Coordinator must record the owner's exact choice or explicit deferral in issue 25. These are questions for consolidation, not questions issued to the owner by this leaf.

## Verification and integration handoff

No native patch or new test registration is appropriate for an unapproved decision leaf. No command registration, ActionItem mapping, message string or production endpoint was added. The reserved 40710 remains unused here; Library 40701 and automation implementation's own allocation remain intact. Any later strings owned by this leaf use IDS_ARCIFIED_AUTOMATION_CONTRACT. New code, if authorized, must use GPL-3.0-only while retaining upstream licenses.

Issue 130 should consume the recorded owner decision. Its adapter must be tested against actual native ownership and the existing private classifier. The focus-routing contract owns future focus targets. Root owns lifecycle, ownership refactor, native test registration, builds and release acceptance. This packet requests integration review of Tabs API and Cocoa consumers when that refactor lands; it does not change those authorities.

[verification.md](verification.md) records the checks actually run and the full pending scenario matrix. Linux, Windows and macOS implementation, native execution and release states all remain OPEN. Same-window multi-Profile and live saved-page handoff remain production gates.
