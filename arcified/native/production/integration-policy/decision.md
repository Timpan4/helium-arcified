# Integration policy decision packet

Issue: [26](https://github.com/Timpan4/helium-arcified/issues/26).
Baseline: `a4b6b480d3de07857ab6e939149254a8359e5449`.
Research date: 2026-10-23 UTC, 2026-10-24 in Stockholm.
Status: decision ready for coordinator review, owner approval outstanding.
Native acceptance: OPEN.

## Decision and authority

Recommend an explicit owner deferral of real provider connections for the initial native release. Keep ordinary GitHub folders/bookmarks and a calendar bookmark usable without an integration. If the owner wants integration work next, approve the narrow candidates below as a separate implementation scope. This recommendation is not an owner deferral or permission to connect.

The assigned issue object is reproduced in [authority.json](authority.json). The root manifest reserves command 40751 but does not approve a provider, account or scope. No owner decision was found in the read authority; live issue comments were not fetched. The coordinator must consolidate the questions below and record the owner's actual answer in issue 26 before real accounts or services are connected. No issue write is performed here.

Root lifecycle v3 already approves one Profile per Space, shared Profiles, Profile-scoped sign-ins/settings/Favorites, same-window multi-Profile switching and live saved-page handoff. Its native integration design leaves the latter behaviors unproven. Integration must preserve those contracts and the existing Model, metadata/codec, command targets and private/download classifiers. It cannot solve routing by moving a page into another Profile, opening a substitute window, cloning live state or declaring `Model::OpenItem` success to be native success.

## Supported facts and their limits

Arc's macOS release notes describe PR/review Live Folders in April 2024, Google Calendar favorites with upcoming meetings and Join in June, and team review requests in July. They establish documentary precedent only. The assigned standalone Live Folder and Live Calendar help articles returned 404. Arc scopes, token storage, current service availability and Linux/Windows behavior cannot be inferred. [Arc release notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)

GitHub's pull-request list endpoint accepts fine-grained tokens with repository "Pull requests" read permission. It also permits unauthenticated public-resource reads. Its response may include PR bodies, so read-only permission does not mean metadata-only transmission. [GitHub pull requests](https://docs.github.com/en/rest/pulls/pulls)

Google distinguishes owned-event read access from access to events on all accessible calendars. Free/busy is a smaller alternative for availability, but does not supply the meeting details needed for the proposed Join workflow. Listing a selected calendar uses its ID, including the `primary` alias. [Google scopes](https://developers.google.com/workspace/calendar/api/auth), [events list](https://developers.google.com/workspace/calendar/api/v3/reference/events/list)

## Provider candidates, currently unavailable

Every row is a proposed scope. No row is approved, implemented or runtime-tested.

| Provider or route | Proposed domain and access boundary | Recommendation and tradeoff |
| --- | --- | --- |
| GitHub.com | User authorization at `github.com`; data at `api.github.com`. Read PRs in explicitly selected installed repositories. GitHub App repository permission "Pull requests: read"; record any provider-imposed metadata permission in the final consent inventory. No Contents, write, workflow, notifications-inbox, email, organization membership or team-read permission requested. | Candidate for a later opt-in Live Folder. Selected-repository scope avoids an all-repository integration. Per-repository PR listing is not proof of an account-wide review-request inbox or team discovery. Those features remain deferred. |
| Google Calendar | Authorization at `accounts.google.com`; token/refresh/revoke at `oauth2.googleapis.com`; data at `www.googleapis.com/calendar/v3/`; ordinary calendar page at `calendar.google.com`. Proposed scopes `openid` and `https://www.googleapis.com/auth/calendar.events.owned.readonly`. Query only `primary` initially. | Candidate for a later read-only primary-calendar view. The owned-events scope authorizes other owned calendars too; querying only primary is an application restriction, not a narrower grant. It excludes shared-calendar support. No calendar-list, settings, contacts, Gmail, Drive, ACL or event-write permission. |
| Google shared calendars | Would require a new decision, potentially `calendar.events.readonly` plus `calendar.calendarlist.readonly` for a picker. | Defer. Broader grant exposes accessible-calendar events and subscription metadata. A picker does not narrow that provider grant. |
| Microsoft Outlook/365 | No supported domain, tenant, OAuth registration or scope approved. Microsoft Graph has a delegated `Calendars.ReadBasic` option, but meeting-link sufficiency and authentication were not established here. | Defer. No claim of Teams calendar-provider support follows from a Teams meeting URL in a Google event. [Microsoft permission reference](https://learn.microsoft.com/en-us/graph/permissions-reference) |
| GitHub Enterprise Server, custom GitHub hosts, sovereign Microsoft clouds, Apple/iCloud, arbitrary CalDAV or subscription feeds | No approved hosts, discovery, credential handling or scopes. | Unavailable pending explicit provider/domain/security decisions. No wildcard host matching, site scraping, cookie reuse, automatic account discovery or provider substitution. |

These domains identify API/auth candidates, not an exhaustive allowlist for the provider's interactive sign-in pages. Verify the registration's current redirects and authority metadata before implementation. Never attach bearer credentials to a different origin or follow cross-origin redirects with credentials.

A calendar Join URL is untrusted event data. Validate it with native URL/security and external-protocol rules on explicit user activation. Do not automatically launch a meeting, accept an invitation, grant microphone/camera access or choose an external application. The calendar leaf owns meeting selection/declined/cancelled/recurrence/time-zone UX. The focus-routing leaf owns activation targeting. Neither approval is implied by a provider scope.

## Authentication and ownership proposal

GitHub App device authorization is a documented native-client candidate: enable it in an owner-controlled registration, show the official verification destination, honor the returned authorization interval and expiry, and support cancellation. Device-flow user tokens can be refreshed without a client secret. This avoids shipping an app private key or introducing a token broker. Registration ownership, permissions and install/repository selection still need approval and an actual protocol check. Do not translate GitHub App permissions into a classic OAuth `repo` scope. [GitHub user tokens](https://docs.github.com/en/apps/creating-github-apps/authenticating-with-a-github-app/generating-a-user-access-token-for-a-github-app), [refresh](https://docs.github.com/en/apps/creating-github-apps/authenticating-with-a-github-app/refreshing-user-access-tokens)

For Google, propose a registered desktop authorization-code flow with PKCE and a provider-supported redirect. Use an external authorization surface, bind the callback to the originating Profile and pending connection, validate state and the ID token, and confirm the resulting account before attachment. Use the validated issuer/subject as identity; email/display name is presentation, not an ownership key. No email/profile scope is proposed. Registration and consent verification requirements remain implementation prerequisites. [Desktop OAuth](https://developers.google.com/identity/protocols/oauth2/native-app), [Google OIDC](https://developers.google.com/identity/openid-connect/openid-connect), [RFC 8252](https://www.rfc-editor.org/info/rfc8252/)

Recommended connection identity is the exact regular Profile, provider authority and verified provider account identity. Spaces sharing that Profile may use its connection but retain their own view/filter presentation. A different Profile gets separate authorization even for the same remote account. The browser's own sign-in and a provider connection are separate. A selected Space, display name, window or default OS browser must never select the credential.

Carry connection identity and an invalidatable operation identity through requests, callbacks and notification activation. Revalidate Profile/connection liveness before publishing results. Cancel or discard callbacks after disconnect, account replacement, Profile shutdown or a superseding request. Moving Space metadata does not migrate a connection or change a live page's credentials.

Reuse the existing private-browsing boundary, including `Operation::kIntegrationCache`. Do not introduce another private/guest classifier, store or remote-authorization authority. Private, Guest and unsupported contexts get no connection or ordinary integration-cache route until their existing authority permits it; this packet grants no exception.

## Storage, refresh and revocation proposal

Recommend session-only credentials and provider data for the first approved implementation, with user-triggered refresh. No provider payload, join URL or token enters workspace metadata, its codec, export, sync, ordinary history, diagnostic logs or the Model. Disclose that read endpoints can return more fields than the view needs. Retain only the fields needed for the approved view after parsing, and avoid logging raw responses.

Persistent credentials, an offline disk cache and refresh after restart are separate owner choices. If later approved, reuse reviewed Profile-owned credential protection, record actual key-provider behavior per OS and refuse persistence when protection is unavailable. Chromium's `OSCryptAsync` header says encryption fails without a suitable provider. Its existence does not prove that this build has protected storage or that tokens are isolated by Profile. [Native source identities](sources.json)

Do not invent a cache retention duration, event horizon, refresh cadence, retry count, request quota or background budget. These remain absent from this decision. Manual refresh avoids a product polling schedule; the later UI must still disclose the selected query range. If background updates are requested, the owner must decide their triggers and stale-age presentation before implementation. Device authorization polling is a distinct protocol requirement and follows the provider response, not a product timer.

Use token response expiration and granted permissions as authority. A refresh token is a credential, not permission to run unapproved background work. Only refresh for an approved user action while the connection is enabled. Serialize token replacement for the connection. GitHub rotation invalidates the old token pair; Google refresh/revoke follows its documented endpoint. Refresh denial or revoked access requires reconnect, never silent scope escalation. [GitHub rotation](https://docs.github.com/en/apps/creating-github-apps/authenticating-with-a-github-app/refreshing-user-access-tokens), [Google refresh/revoke](https://developers.google.com/identity/protocols/oauth2/native-app)

Propose explicit controls for disabling updates, disconnecting a local connection and revoking remote authorization. Their effects must be explained separately. Disabling stops work without asserting remote revocation. Disconnect immediately stops local use and invalidates callbacks. Google documents programmatic revocation; GitHub's provider settings are a remote revocation route. Show pending/failed/unconfirmed remote revocation honestly when offline. [Google revocation](https://developers.google.com/identity/protocols/oauth2/native-app), [GitHub revocation](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/token-expiration-and-revocation)

Recommend clearing the connection's own transient credentials, derived cache and notifications on explicit disconnect. This is an owner proposal, not permission for deletion, Profile removal or remote grant changes now. Explain that revoking an app authorization may affect other uses of the same grant, including separately connected local Profiles. Preserve ordinary bookmarks, saved items, source pages, downloads and user files. Root retains destructive-action and migration/source-disposition policy.

## Stale, offline and notification behavior proposal

| State or event | Required presentation in a future approved implementation |
| --- | --- |
| Unavailable, deferred or never connected | Name the unsupported/deferred route. Offer ordinary bookmark/folder use. Do not show a functioning account or silently connect. |
| Awaiting consent, cancelled or denied | Show pending or cancelled/denied. No account data appears until verified account attachment succeeds. |
| Connected, no successful fetch | Show "Not checked" or the fetch error. Do not interpret missing data as no PRs or meetings. |
| Successful fetch | Show "Last checked" with actual successful-check time and connection context. Empty results require a successful complete response for the selected filter. No freshness promise is invented. |
| Refresh in progress or partial/paginated response | Retain the prior complete snapshot with its check time. Mark updating/incomplete; partial data must not assert completeness. |
| Offline, request failure or sleep/resume without revalidation | Mark displayed data stale with reason and last check. Distinguish no cached data. Suppress stale meeting reminders and automatic Join readiness. Explicitly opening the calendar remains available. |
| Expired token, revoked grant, account mismatch or insufficient permission | Stop affected requests. Show reconnect or access restricted. A forbidden response may be an organization/SSO restriction, not proof that the whole token was revoked. Never broaden the grant automatically. |
| Provider throttling or service failure | Preserve stale state and honor provider retry information. No fixed backoff or retry quota is specified here. |
| Invalid Google sync token, if incremental sync is later selected | Mark rebuilding and replace only the provider-derived view after resync. Google's 410 guidance does not authorize deletion of workspace items or user files. [Google sync](https://developers.google.com/workspace/calendar/api/guides/sync) |
| Disable/disconnect/shutdown | Stop refresh and prevent late callbacks or stale notification actions from selecting another account/Profile. Remote revocation status is independent. |

Recommend native notifications off until separately enabled. The conservative opt-in mode sends a generic integration reminder without event title, attendees, repository/PR title, account/email, join URL or sensitive icon. Keep details inside the correct Profile after explicit activation. Any richer notification content needs a separate owner decision and visible disclosure that the OS notification center, lock screen or history may retain it.

Do not rely on a lock-state signal or OS preview setting to redact a payload after sending it. Reuse Chromium's Profile-bound `NotificationDisplayService`; it may use a platform center or Chrome's Message Center, and not every backend can enumerate displayed notifications. Source inspection does not prove lock-screen privacy, withdrawal or click routing. Use an opaque connection/event reference in activation, revalidate current state and route to the originating Profile and intended host. If that route is unavailable, show the action unavailable instead of opening in another Profile.

## Questions for the coordinator to consolidate

1. Do you approve deferring all real GitHub and calendar connections for the initial native release? Recommended: defer and retain ordinary folder/bookmark workflows. Alternative: approve the named later implementation scope below, which adds account and service dependencies.
2. If proceeding, do you approve only GitHub.com selected-repository PR read access and Google primary-calendar owned-event read access plus `openid`, with Enterprise/custom hosts, shared calendars, team discovery, Outlook/365 and iCloud deferred? Recommended: this narrow set. It omits broader inbox/team/shared-calendar behavior.
3. Do you approve Profile-bound, session-only credentials/data with manual refresh and no native notifications initially? Recommended: yes for a first implementation. Persistent protected credentials/cache, background updates and generic native reminders need separately recorded choices; no retention or polling value is presumed.
4. Who owns the GitHub App and Google desktop-client registrations, consent review and support obligations? Recommended: the fork owner controls them. No app, cloud service, credential or broker is created by this packet.

A sufficient issue decision records the owner's exact approval or deferral, approved providers/domains and permission text, account/Profile scope, storage/refresh/notification choices, registration owner, exclusions and any unresolved choices. The coordinator may use this packet to draft that record, but must not fill in the answers.

## Native work and remaining proof

This is a decision leaf. A production patch, prepared-source result diff, GN test registration, ActionItem mapping and localization changes are inapplicable until implementation is approved. No new string uses another leaf's namespace. Any later implementation must use `IDS_ARCIFIED_INTEGRATION_POLICY`, use only 40751 if a legacy command is necessary with its actual declarative ActionItem mapping, preserve Library 40701, and use ActionItems or another coordinated ID for other actions.

[Verification](verification.md) keeps runtime scenarios open on every target platform. Same-window multi-Profile selection and live saved-page handoff remain production gates. Local source hashes and packet checks establish documentary traceability only. They establish no native provider connection, protected token storage, OAuth success, native notification privacy or installed behavior.
