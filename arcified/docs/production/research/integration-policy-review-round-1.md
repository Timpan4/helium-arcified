# Integration policy review, round 1

Disposition: CLEAN within the documentary decision scope. No actionable findings.
Issue: [26](https://github.com/Timpan4/helium-arcified/issues/26).
Reviewed: 2026-10-23 UTC, 2026-10-24 in Stockholm.
Baseline: `a4b6b480d3de07857ab6e939149254a8359e5449`.
Native acceptance: OPEN. Owner approval or deferral: OPEN.

## Scope and inspected identities

Independently reviewed all seven files in `arcified/native/production/integration-policy/`, including every assigned acceptance criterion, provider disposition, owner question and future verification scenario. This is the first review round. No prior findings were supplied. Only this report was written; packet files, source, root controls, Git history and remote state were not changed.

Read the full authoritative integration-policy issue object from `/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/arcified/docs/production/research/assignment.json`. Compared the complete object structurally with `authority.json`, with equality. Also compared the full integration-policy manifest record with `authority.json`, with equality. Inspected the applicable lifecycle v3, native Space integration and execution-state authority at the same root.

The seven packet hashes independently recomputed with `sha256sum` match the coordinator's supplied review inputs:

| File | SHA-256 |
| --- | --- |
| ownership.json | `7848029e1f40916f833385e2f8aff59b0efbd8479547653406ac0ae6854657ed` |
| authority.json | `a95ae95dd7bb810eb431d2a4d9275e95913b2f52349128eff2f79e21755d7ef5` |
| sources.json | `2e1feaa0453cce5cc7694f80b5d848d458b52d789ab39f7f9c6bd897c165921b` |
| decision.md | `d41d886b70581611161a1d8e503078d9c4833e2d4cc3c90c5bdf63d1758a7f9b` |
| verification.md | `df2e0a453d91d6fe1199b669c47e651aea849202428d98b155e4bf0f7dd646c3` |
| result.json | `16fce05efaa0d206b7e1e258be9384aba6476832fc2bd6d5911eb2d6f7a406d1` |
| checks.json | `de07c97e84de68dbd21298a5fabab5c116b7b2d6e213178eeaaf882e897b8544` |

The five root-authority hashes and three prepared-header hashes also matched `sources.json` when inspected. These identify inspected bytes, not a frozen current release state. Root controls may change concurrently. The prepared Chromium tree was read only.

## Acceptance and scenario coverage

| Authoritative criterion | Evidence assessed | Review conclusion and remaining gate |
| --- | --- | --- |
| Name providers/domains/scopes, account/Profile ownership, storage, revocation and refresh | `decision.md:25` through the authentication and storage sections; `sources.json`; `result.json` | Covered as proposals. GitHub.com and Google Calendar have named candidate hosts and permissions. Shared calendars, Outlook, Enterprise/custom hosts, iCloud and other routes have explicit deferred/unavailable dispositions. Profile-bound ownership, session storage, conditional persistence, manual refresh and local disconnect versus remote revocation are distinguished. Actual owner selection and native support remain OPEN. |
| Define stale/offline states and native notification privacy without invented polling or quotas | `decision.md:59` and `decision.md:67`; `verification.md:25` and `verification.md:27` | Covered. Never-fetched, complete-empty, partial, stale, offline, authorization failure, throttling, sync invalidation and shutdown states remain distinct. Last-check time supplies evidence without a fabricated freshness promise. Notifications are proposed off, with separately approved generic content and exact Profile activation. No product interval, quota, retention duration or retry count was invented. |
| Record owner approval or deferral in issue before real connections | `authority.json` ownerApproval; `decision.md:11`, `decision.md:13`, `decision.md:86`; `result.json` remainingCriteria | Accurately OPEN. The recommendation to defer is explicitly not an owner decision. Issue comments were not fetched, so absence of approval is bounded to the inspected authority. Coordinator/root must obtain the answer and record it in issue 26. No accounts or services were connected. |

Reviewed every future-scenario row in `verification.md:20` through `verification.md:30`: consent/cancellation/denial, scope restrictions, shared and distinct Profiles, stale callbacks, token lifecycle, offline/incomplete responses, Google event semantics, optional notifications, activation/live pages, disconnect/revocation and unsupported providers. Each row explicitly remains OPEN on Linux, Windows and macOS. These are planned requirements, not registered or executed tests.

The result disposition means the research packet is ready for an owner decision. It does not close the issue or claim native completion. A production patch, prepared-source diff, GN test registration, ActionItem implementation and localization changes are appropriately inapplicable to this unapproved decision leaf.

## Primary-source accuracy and provenance

Rechecked the following public primary documents on 2026-10-23 UTC. All findings here are documentary; no authenticated API calls or reference applications ran.

| Source | Independent assessment |
| --- | --- |
| [Arc macOS release notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes) | Confirmed April 11, 2024 PR/review Live Folders, June 13 calendar favorite/Join behavior, and July 18 team review requests. The packet correctly limits this to historical macOS documentation. |
| [Assigned Live Folder article](https://resources.arc.net/hc/en-us/articles/22731612065815-Automatic-GitHub-Live-Folders), [assigned calendar article](https://resources.arc.net/hc/en-us/articles/24158102740631-Live-Calendars) | Both returned 404 again. The packet preserves these retrieval failures rather than inventing historical authentication, scopes or current support. |
| [GitHub pull requests](https://docs.github.com/en/rest/pulls/pulls) | Confirmed repository PR read permission, eligible GitHub App user tokens, unauthenticated public access and PR body-bearing responses. Per-repository listing does not establish an account/team inbox. |
| [GitHub user tokens](https://docs.github.com/en/apps/creating-github-apps/authenticating-with-a-github-app/generating-a-user-access-token-for-a-github-app), [refresh](https://docs.github.com/en/apps/creating-github-apps/authenticating-with-a-github-app/refreshing-user-access-tokens) | Confirmed app/user access intersection, registration-enabled device flow, provider-returned polling/expiry information, device-flow refresh without a client secret and invalidation of the old token pair after rotation. |
| [GitHub native-client guidance](https://docs.github.com/en/enterprise-cloud%40latest/apps/creating-github-apps/about-creating-github-apps/best-practices-for-creating-a-github-app), [revocation](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/token-expiration-and-revocation) | Confirmed user-token/platform-storage guidance, prohibition on shipping an app private key, and account-settings revocation of an app authorization and its tokens. |
| [Google scopes](https://developers.google.com/workspace/calendar/api/auth), [events list](https://developers.google.com/workspace/calendar/api/v3/reference/events/list) | Confirmed owned-calendar read scope and primary-calendar selector. The packet correctly discloses that a primary-only query does not narrow the provider grant to that calendar. |
| [Google desktop OAuth](https://developers.google.com/identity/protocols/oauth2/native-app), [OIDC](https://developers.google.com/identity/openid-connect/openid-connect) | Confirmed PKCE, token refresh, programmatic revocation and stable validated subject identity. The desktop page's Token revocation section supports the cited endpoint; no citation correction is needed. |
| [Google incremental sync](https://developers.google.com/workspace/calendar/api/guides/sync) | Confirmed invalid-token 410 and full resync guidance. The packet confines replacement to provider-derived state and grants no workspace/user-file deletion authority. |
| [RFC 8252](https://www.rfc-editor.org/info/rfc8252/) | Confirmed external user-agent and PKCE requirements for public native clients. The packet leaves the actual registered flow and callback validation untested. |
| [Microsoft Graph permissions](https://learn.microsoft.com/en-us/graph/permissions-reference#calendarsreadbasic) | Confirmed delegated Calendars.ReadBasic and exclusions for body, attachments and extensions. The packet labels its original evidence as a search result and does not claim investigated Microsoft authentication or meeting-link sufficiency. |

The packet records source URLs, retrieval dates, failures and limitations. External pages are not byte-pinned or archived. Their current contents support the checked claims but do not establish provider behavior in an Arcified build.

Read the three exact prepared headers listed in `sources.json`. `BrowserWindowInterface` supports the fixed desktop Profile constraint; `NotificationDisplayService` describes a Profile-bound platform/Message Center service and incomplete enumeration support; `OSCryptAsync` documents encryption failure without an available encryption provider. Their hashes matched both recorded base and result identities. These declarations do not prove storage protection, notification privacy or click routing at runtime. The existing private operation boundary includes `Operation::kIntegrationCache`; no replacement classifier or policy was introduced.

## Owner-decision quality and remaining gates

The four coordinator questions in `decision.md:88` through `decision.md:91` and `result.json` state recommendations and tradeoffs without filling in answers:

1. Approve or reject deferral of all real GitHub/calendar connections for the initial native release. The packet recommends deferral with ordinary bookmarks/folders retained.
2. If proceeding, approve or reject the exact GitHub.com selected-repository PR read and Google primary-calendar owned-calendar read plus `openid` candidates, with the listed wider routes deferred.
3. Approve or change Profile-bound session-only credentials/data, manual refresh and notifications off. Persistent storage, background work and reminders remain separate choices.
4. Name the registration, consent-review and support owner. Fork-owner control is a recommendation only.

Actual answers and their issue record remain required before connection. Later implementation must prove consent, isolation, credential handling, failure presentation and notification privacy on each target OS. Same-window multi-Profile behavior and exact live saved-page handoff remain production gates. The packet preserves Model, metadata/codec, immutable Profile targets, private/download classification and root-owned destructive/migration policy. It reserves 40751 without implementing a command, preserves Library 40701 and names the required future ActionItem mapping and string prefix.

## Independent checks and limits

Checks performed:

1. `git status --short` and `git branch --show-current`. Confirmed the assigned branch and untracked shared production directories. No Git mutation occurred.
2. `sha256sum` on all seven packet files, the five absolute root-authority paths and the three absolute prepared-header paths listed in `sources.json`. Every digest matched the supplied or packet-recorded identity.
3. Hosted JavaScript over complete `fast_filesystem.fs_read_many` results. All seven packet reads completed. The five JSON files parsed and matched two-space `JSON.stringify` formatting with final newlines. Packet whitespace and local Markdown links passed. Complete issue and manifest object comparisons passed. Result evidence paths resolved; all criteria and platform/native OPEN states were retained.
4. Public primary-source web retrieval and focused section checks described above. Both assigned standalone Arc article failures were reproduced. No authenticated provider request occurred.

No runtime, native build/test, browser/visual, benchmark or remote CI execution was authorized or performed. No owner answers, account access, installed functionality or release readiness were verified. These correctly recorded limits are not findings against this decision packet.
