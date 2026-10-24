# Favorite preview decision

Issue: [#28](https://github.com/Timpan4/helium-arcified/issues/28). Baseline: `a4b6b480d3de07857ab6e939149254a8359e5449`.
Status: recommendation ready for coordinator review, owner decision pending. Native acceptance: OPEN.

Recommend deferring every account-backed Favorite preview. For GitHub, recommend replacing the retired hover preview with the separately approved Live Folders workflow, if that workflow is accepted. Retain ordinary Profile Favorites as the surviving navigation route. This packet implements no integration and records no owner approval.

## Authority and evidence

The full assigned issue object is retained in [assignment.snapshot.json](assignment.snapshot.json). The root assignment, control manifest, lifecycle contract v3, native integration plan and execution state were read from the root worktree specified in the task. Exact input bytes are identified in [source-identities.json](source-identities.json) and [inputs.sha256](inputs.sha256). [sources.json](sources.json) records external URLs, section locators, retrieval status and limits. Retrieval date: 2026-10-23 UTC, 2026-10-24 Europe/Stockholm.

The December 16, 2021 archive names Notion, Figma and Linear hover previews for recent pages and common actions. It separately introduces People Cards on Twitter, LinkedIn and GitHub. The entry does not enumerate those common actions or their data permissions. [S1](https://start.arc.net/release-notes)

Later macOS notes establish Outlook Mail preview login fixes on August 11, 2022, Outlook Calendar recurring-event fixes on October 27, and a New Linear Issue command on November 17. They also document Google Calendar countdowns, Favorites badging for Slack, Twitter, GCal and Gmail, and a dedicated preview preferences section. A command or badge does not prove a hover preview's complete capabilities. [S2](https://resources.arc.net/hc/en-us/articles/20498417809815-Arc-for-macOS-2022-Release-Notes)

The March 30 and August 17, 2023 notes fix Gmail hover display and accented text. May 25 adds Confluence previews of recent pages with navigation. The October notes describe Max link-summary previews as a separate workflow. [S3](https://resources.arc.net/hc/en-us/articles/20498377604887-Arc-for-macOS-2023-Release-Notes)

GitHub Live Folders were introduced on April 11, 2024 for PRs and review requests. May 23 explicitly deprecates GitHub Previews in their favor. June 20 updates the preview-selection settings UI. June 13 describes the separate Google Calendar meeting/Join surface. These statements do not certify other previews as current. [S4](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)

The cited GitHub Live Folders and Live Calendars help URLs returned 404. The standalone 2021 help page timed out on direct open, although search returned its December entry and S1 supplied the archive. No account, provider API, app or browser interaction was used. Missing documentation is a retrieval limit, not proof that the feature was removed.

## Per-site recommendation

All scopes below are proposals for an owner decision. They grant no provider access. Candidate domains identify review targets, not a domain allowlist or supported production route. All account readers, mutations and preview-specific controls remain unimplemented.

| Site and evidence | Recommendation and surviving workflow | Candidate data and action scope if reconsidered | Disable control and stale behavior proposed |
| --- | --- | --- | --- |
| Notion, S1 | Defer account preview. Open the ordinary Favorite. | Selected Notion workspace only; recent page titles and destinations. Navigation only. Exclude page bodies, search across all workspaces, creation and editing. Candidate hosted domain: notion.so; custom domains require a separate decision. | Profile/provider disable plus workspace disconnect. Stop reads and dismiss rows on disable, sign-out, workspace change or lost permission. On refresh failure, identify unavailable/stale data rather than showing an empty workspace. |
| Figma, S1 | Defer account preview. Open the ordinary Favorite. | Selected account/team only; recent file names and destinations. Navigation only. Exclude canvas contents, thumbnails, comments, editing and file creation. Candidate domain: figma.com. | Profile/provider disable plus team/account disconnect. Remove active presentation on account change or revocation. Failed refresh must not present cached files as current or claim there are no recent files. |
| Linear, S1 and S2 | Defer account preview and browser-side issue creation. Open Linear for both tasks. | Selected workspace only; issue titles, identifiers and destinations. Navigation only. The historical New Linear Issue command is separate evidence and does not approve an API write. Candidate domain: linear.app. | Profile/provider disable plus workspace disconnect. Hide rows after account/workspace change or permission loss. Failed refresh shows stale/unavailable, not zero issues. Creation remains unavailable. |
| Confluence, S3 | Defer account preview. Open the ordinary Favorite. | Explicitly selected tenant and spaces; recent page titles and destinations. Navigation only. Exclude page bodies, creation, editing and tenant-wide search. Review hosted atlassian.net separately from self-hosted domains. | Profile/provider and tenant disable. Dismiss tenant rows on disconnect or permission change. Failure cannot present stale titles as current access or as a complete recent-page list. |
| Gmail, S3 | Defer mail preview. Open Gmail in its actual Profile. | If requested later, begin with account identity and unread count only, with explicit approval. Exclude subjects, senders, bodies, attachments, mark-read, compose, send and delete. Candidate domain: mail.google.com. This is a smaller proposed scope, not a claim about historical payloads. | Profile/provider and account disable. Stop reads and remove badges on disable/account change. A failed count read means unknown, never zero. No mail mutation on hover or row activation. |
| Outlook Mail, S2 | Defer mail preview. Open Outlook in its actual Profile. | If requested later, account identity and unread count only. Exclude message metadata/content and mail mutations. Consumer outlook.live.com and organizational outlook.office.com need separate scope and tenant decisions. | Profile/provider and account/tenant disable. Remove active presentation on disconnect or tenant change. Label failed reads unknown; never convert failure to an empty inbox. |
| GitHub, S4 | Replace historical hover preview with the separate optional Live Folders proposal. Until accepted and implemented, open the Favorite or a user-selected PR page. | Candidate github.com workflow: selected repositories, authored PRs and review requests, titles/status/destinations only. Navigation only. Exclude code contents, comments, merges, approvals and other writes. Enterprise hosts require a separate decision. | Provider/account/repository disable must be included in the Live Folders decision. Stop updates and hide rows on disconnect. Failed refresh cannot label PRs current, complete or absent. No automatic PR action. |

No API permission names, auth transport, storage retention, polling intervals, quotas or expiry thresholds are selected here. Those require a provider-specific contract and owner acceptance. The Notion/Figma/Linear archive gives a shared recent-page description, not a complete per-site schema. Mail fields and GitHub filters above are deliberate proposed reductions, not recovered Arc behavior.

## Adjacent features kept separate

| Reference | Disposition for this leaf |
| --- | --- |
| Google Calendar and Outlook Calendar, S2/S4 | Defer here and route to calendar plus integration-policy. Proposed scope belongs there: selected calendars, event timing and explicit Join navigation. No automatic joining, invitation writes or inference that Outlook has the later Google Live Calendar workflow. Calendar disable and stale countdown handling need their own decision. |
| Slack and Twitter, S2 | Evidence establishes Favorites badges only. Defer enrichment; ordinary site navigation survives. No message/timeline data or API actions are proposed. Any later badge needs provider disable and unknown-on-failure behavior, not a zero count. |
| Twitter, LinkedIn and GitHub People Cards, S1 | Defer service-dependent person enrichment through retirements. No person-profile fetch or action is accepted. These are not Favorite preview rows. |
| Max link summaries, S3 | Later AI scope. No page text is sent to a provider here. A summary card does not implement a Favorite integration. |
| Folder previews | Local saved-item hierarchy/search is a different component. It supplies no account recents, unread data or PR provider. Generic Live Folders cover none of the other site rows automatically. |
| Spotify, SoundCloud and Apple Music | S2 describes audible Favorite icon animation. Keep under media/status work; no account preview is inferred. |

## Integration and stale-state boundary

The existing Favorite foundation resolves Profile-owned item/alias identities against the lifecycle Model and reuses shared pins metadata. Folder preview is a pure projection over validated local folder state. Their documentation already keeps native behavior open. Reading these foundations is sufficient for this decision leaf; rerunning their synthetic tests would not prove account access or native integration.

A later accepted integration must use the existing Favorite reference and canonical ItemId. Provider rows must not create another lifecycle Model, metadata codec, page registry or saved destination. Favorites and sign-ins remain Profile-scoped; Spaces sharing a Profile share its real services. Distinct Profiles must not share an account snapshot merely because URLs or labels match. Private/guest and download classification remain with their existing authorities. Unsupported contexts stay unavailable until an authoritative route exists.

Propose explicit states for disabled, not connected, loading, current result, successful empty result, stale/error, permission revoked and unavailable provider. Only a successful authorized response can establish empty/current. After failure, show an error and last-success time if an accepted cache policy permits retained content. Otherwise show unavailable. Retention, revocation cleanup and source-data deletion remain owner decisions; this packet chooses no storage policy. Disabling must stop new reads and callbacks must not repopulate dismissed UI. Re-enabling must not silently authenticate.

Hover must not grant data access or perform a remote mutation. Pointer and keyboard invocation need the same content and actions, readable labels and focus restoration. Validate Profile/account/tenant and target identity again at activation. Native navigation must preserve security checks, the saved page's identity and the actual Profile. If a saved page is already live elsewhere, the accepted same-page handoff contract applies. Same-window multi-Profile presentation and live saved-page handoff remain native production gates.

No legacy command is added. 40740 stays reserved to this leaf; Library 40701 is untouched. Any later use of 40740 must include Chromium's actual declarative ActionItem mapping; other actions should use ActionItems or another coordinated ID. No strings are added; any later strings must use `IDS_ARCIFIED_FAVORITE_PREVIEWS_DECISION`. Newly authored Arcified code would use GPL-3.0-only.

## Owner questions for coordinator consolidation

1. Accept the recommended release scope: defer Notion, Figma, Linear, Confluence, Gmail and Outlook Mail account previews, and replace the retired GitHub hover preview only through separately accepted Live Folders? Recommendation: yes. This preserves ordinary Favorite navigation and avoids assuming account access; it gives up inline account recents and mail badges.
2. If any deferred site is required, which named provider/domain/tenant and read fields are accepted, and is navigation-only sufficient? Recommendation: select an explicit row and keep writes excluded. Broader content or actions require a separate permission and failure contract.
3. For each selected site, accept Profile/account ownership, provider and account disable, and explicit stale/unknown presentation, then settle authentication, credential storage, retention, revocation, refresh policy and private-context availability through integration-policy before implementation? Recommendation: yes. No interval or cache lifetime is invented.

The coordinator must retain the owner's answer and accepted scope in the authoritative issue/root decision record before an account integration is implemented. The task's empty dependency list is preserved. Cross-leaf integration requests are coordination needs, not retroactive blocking dependencies.

## Acceptance and remaining proof

| Criterion | Evidence and state |
| --- | --- |
| Historical source and later uncertainty | Documentary work complete in this packet, S1-S4. The inventory covers the explicitly named previews and separates adjacent badges, calendars, People Cards, AI and folders. It is not an exhaustive claim about undocumented integrations. |
| Per-site support/defer/replace, scope, disable and stale behavior | Recommendation supplied for every inventoried site. No acceptance or implementation is implied. |
| Owner records accepted scope before integration | OPEN. No owner response is present and no integration was implemented. Coordinator consolidates the questions. |
| Native patch and registered native tests | Inapplicable to this decision-only change. There is no production behavior to patch or test registration to add before the scope decision. No fake service/store or Model::OpenItem result is native evidence. |
| Prepared base/result files | Only read-only chrome/VERSION was inspected and hashed, confirming Chromium 155.0.8059.79 and Helium fields. No native source result files or patch exist for this nonimplementation leaf. |
| Linux, Windows, macOS | All native implementation, installed interaction, accessibility, account isolation, disable/revoke races, stale state and restart evidence remain OPEN. Historical reference here is macOS documentation; Windows parity is unproven and Linux is a downstream target. |

Future accepted scope needs real native tests for Profile/account isolation, account switch during fetch, disable/revocation races, successful-empty versus failed response, stale targets and removed permissions, offline recovery, keyboard/focus and actual navigation/handoff. Register tests in the actual owning GN targets after the implementation is chosen. Root controls native builds and platform execution. No build, native test, benchmark, remote CI or visual/browser execution ran here. Local artifact checks are recorded in [checks.md](checks.md).
