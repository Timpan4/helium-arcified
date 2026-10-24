# Verification and integration requests

Issue 26 native acceptance is OPEN on Linux, Windows and macOS.
No native build, native test, browser/visual check, reference app, authenticated provider request, benchmark or remote CI ran. The root's prepared Chromium source remained read-only.

## Acceptance coverage

| Assigned criterion | Packet evidence | Remaining criterion |
| --- | --- | --- |
| Providers/domains, scopes, account/Profile ownership, storage, revocation and refresh | decision.md names candidates, exclusions, recommended data handling and unresolved choices; sources.json identifies primary sources and local byte identities. | Owner must approve or defer the named scope. No provider is supported in an installed native build by this packet. |
| Stale/offline states and native notification privacy without invented intervals/quotas | decision.md defines event-driven failure states, last-check disclosure, manual refresh and a conservative notification proposal. | Owner must accept choices; native adapters and per-OS privacy/activation proof remain open. |
| Owner approval/deferral in issue before real connections | authority.json records no approval in the supplied authority; decision.md contains questions for coordinator consolidation. | Coordinator obtains actual owner answer and records it in issue 26. No issue mutation authorized here. |

## Required future scenarios

Each row is OPEN on Linux, Windows and macOS. These are requirements for later approved implementation, not registered or executed tests.

| Scenario | Evidence needed |
| --- | --- |
| Consent, cancellation and denied access | Correct provider registration, permission screen, originating Profile, callback validation and cancellation without publication. |
| Limited scope | Selected GitHub repositories only; primary Google query; no writes or broadened grants. Demonstrate inaccessible/unsupported routes remain unavailable. |
| Shared Profile and distinct Profiles | Spaces sharing a Profile use the intended connection; distinct Profiles remain isolated even with the same remote account and while switching Spaces in one actual native window. |
| Pending callback after account/Space/Profile changes | No result attaches to current selection or a replacement connection. Private/Guest/unsupported routes reuse authoritative refusal. |
| Token lifecycle | Provider expiry, denied refresh, replacement/rotation, lost credential protection, disconnected session and restart behave as approved. No raw token/payload in logs, export or metadata. |
| Offline and incomplete data | No-data, stale, partial pagination, throttled and failed-fetch states remain distinguishable from successful empty results. Cancelled work and shutdown do not publish late data. |
| Google event semantics | Calendar leaf proves recurrence, declined/cancelled/all-day events, time zones, sleep/resume and meeting-link validation before Join claims. No fixed event horizon inferred here. |
| Optional native notification | Opt-out, generic payload, OS permission denial, lock-screen/history exposure, stale click, disconnect and withdrawal on actual OS backend. No notification content crosses Profile. |
| Activation and live pages | Focus-routing/native ownership leaves prove correct host and exact Profile, same-window multi-Profile behavior and live saved-page handoff without reload, clone or substitute window. |
| Disconnect and revocation | Stop local requests immediately; distinguish remote revoke success from offline/unconfirmed status; preserve bookmarks, saved items, downloads, user files and other connections as the approved decision requires. |
| Unsupported providers | Enterprise/custom domains, Outlook, shared calendars, iCloud and arbitrary feeds show unavailable until separately authorized. No scraping or account auto-discovery fallback. |

## Integration requests

1. Research coordinator consolidates the decision questions, obtains independent review and submits the actual owner approval/deferral through root. Do not treat this recommendation as issue closure.
2. Live Folder/calendar coordinators consume the provider/permission decision before implementing service access. Account-wide/team discovery and shared calendars require a new scope decision.
3. Native ownership and focus-routing coordinators retain the actual Profile/host gates. Async integration and notification targets must follow their approved lifetime and command contracts.
4. Root/private-browsing authority supplies exact regular/OTR/Guest context classification. Credential persistence must prove actual platform key protection before it is offered.
5. If implementation becomes approved, coordinator owns production patch assembly, prepared base/result identities, GN registration and native test execution. This decision packet requests no current changes to those shared sources.

## Focused light checks

Checks are recorded in result.json after execution. JSON uses two-space formatting with a final newline. Markdown is authored to the repository's existing simple format. No configured Markdown/JSON formatter or linter was found in the scoped Arcified configuration search; no package was installed.

The focused packet check reads via fast_filesystem and evaluates JSON parsing/canonical formatting, Markdown whitespace/final newlines, local Markdown link targets, required result fields, assigned issue/baseline/command identity, coverage of every assigned criterion and OPEN native status. It does not execute any app logic.

The source identity check uses `sha256sum` on the exact local paths in sources.json. A second check compares the prepared-header identities. Root controls are active files; their hashes are a sampled authority identity and are not required to remain frozen as other tasks proceed.

No code was authored. C++ syntax/clang-format, sparse patch apply and GN/test-registration checks are inapplicable. Passing pure lifecycle/private-boundary foundation tests is not reused as native evidence, and those tests were not rerun.
