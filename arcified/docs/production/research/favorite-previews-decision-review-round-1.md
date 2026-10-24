# Favorite previews decision review, round 1

Verdict: clean within the documentary decision scope. No actionable findings. Owner acceptance and native acceptance remain OPEN. This review does not approve an integration or close issue #28.

Reviewed on 2026-10-24. Target: `arcified/native/production/favorite-previews-decision/`, baseline `a4b6b480d3de07857ab6e939149254a8359e5449`. Independent first review, with no prior findings. Only this review report was written.

## Authority and coverage

Read the full assigned leaf object from the designated root worktree's `arcified/docs/production/research/assignment.json`, its `non-ai-production-workstreams.json`, lifecycle contract v3, native integration plan and execution state. The packet's assignment snapshot equals the authoritative leaf object. Its empty dependencies, decision classification and command allocation 40740 agree with that authority.

The recorded input hashes matched during this review. These identify inspected bytes. They do not freeze concurrently maintained root controls or establish current release readiness.

| Assigned criterion | Review result and packet locator |
| --- | --- |
| December 16, 2021 reference, later integrations and uncertainty | Satisfied for documentary research. `decision.md` lines 10 through 20 uses the required history and later first-party notes. Current service behavior, complete historical payloads and platform parity remain unproven. |
| Per-site support/defer/replace, data/actions, disable and stale behavior | Satisfied as recommendations. `decision.md` lines 24 through 36 supplies these fields for every inventoried preview site. Candidate fields and domains are proposals, not recovered schemas or enabled routes. |
| Owner records accepted scope before integration; generic Live Folders do not imply coverage | Correctly OPEN. `decision.md` lines 63 through 67 and `result.json` require an actual owner record before implementation. `ownerDecisionRecorded` is false. GitHub replacement requires separate Live Folders acceptance. |

Every packet file was read in full:

| File | Coverage |
| --- | --- |
| `ownership.json` | Reviewed. Identity, packet boundary, command allocation and OPEN native status. |
| `assignment.snapshot.json` | Reviewed. Exact equality with the authoritative leaf object. |
| `source-identities.json` | Reviewed. Input identities, baseline and explicit non-acceptance purpose. |
| `inputs.sha256` | Reviewed. Corresponds to the input manifest; every entry independently checked. |
| `sources.json` | Reviewed. First-party URLs, dated locators, retrieval limitations and documentary scope. |
| `decision.md` | Reviewed. Full inventory, adjacent features, recommendations, lifecycle boundaries, owner questions and remaining gates. |
| `checks.md` | Reviewed. Artifact checks are distinguished from unrun native/account/visual checks. |
| `result.json` | Reviewed. Summary, evidence references, conditional integration requests, owner-decision state and remaining criteria. |

## Primary-source verification

Public `web.run` open/find calls independently retrieved these first-party pages on 2026-10-24. No browser interaction or account access occurred.

1. [Arc release archive](https://start.arc.net/release-notes), December 16, 2021. Confirms Notion, Figma and Linear hover previews, described jointly through recents and common actions. People Cards are separately described. Exact actions and permissions remain unresolved in the packet.
2. [2022 macOS notes](https://resources.arc.net/hc/en-us/articles/20498417809815-Arc-for-macOS-2022-Release-Notes), February 16, March 17, June 2, August 11, October 27 and November 17. Support the calendar countdown, badges, preview preferences, Outlook Mail authentication fix, Outlook Calendar fix and Linear creation-command distinctions. The music-icon status reference is also present. A command or badge is not used to infer a full preview schema.
3. [2023 macOS notes](https://resources.arc.net/hc/en-us/articles/20498377604887-Arc-for-macOS-2023-Release-Notes), March 30, May 25, August 17 and October. Confirm Gmail hover/character fixes, Confluence recent-page preview and the separate Max preview family.
4. [2024-2026 macOS notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes), April 11, May 23, June 13 and June 20, 2024. Confirm PR/review-request Live Folders, explicit GitHub Preview deprecation, the Google Live Calendar surface and preview-selection settings.

The packet records historical U1/U2 404 responses and a U3 direct-open timeout. Those particular failures were not independently reproduced in this round. The retrieved archive and release notes independently support the inventory and GitHub retirement. The packet treats retrieval failures as limits rather than proof of service retirement. Present availability and undocumented providers remain unknown.

## Site and scenario disposition

| Site or family | Reviewed disposition and safeguards |
| --- | --- |
| Notion | Proposed defer. Workspace recents/navigation candidate, disable/disconnect, dismissal on lost access, explicit stale/unavailable result. |
| Figma | Proposed defer. Team/account file destinations, no content or writes, disable/disconnect and account-change/failure handling. |
| Linear | Proposed defer. Scoped issue destinations, creation separately excluded, workspace disable and failure distinct from empty. |
| Confluence | Proposed defer. Selected tenant/spaces, navigation only, tenant disable and stale/access-loss handling. |
| Gmail | Proposed defer. Reduced identity/count candidate explicitly distinguished from historical payloads, no mail mutations, account disable and unknown count on failure. |
| Outlook Mail | Proposed defer. Reduced identity/count candidate, consumer/organizational scope unresolved, account/tenant disable and unknown on failure. |
| GitHub | Proposed replacement through separately accepted Live Folders. Selected repository PR/review data, navigation only, provider/account/repository disable and explicit stale state. No implementation or approval implied. |
| Calendars, badges, People Cards, AI, local folders and media status | Explicitly separated and routed or deferred. None fills another preview's acceptance gap. |

The shared state proposal distinguishes disabled, disconnected, loading, current, successful empty, stale/error, revoked and unavailable. It covers account/tenant changes, stale completion after disable, keyboard/pointer equivalence, activation-time validation and actual Profile ownership. Cache retention and revocation cleanup remain owner choices. No polling interval, quota, storage policy or automatic remote action is invented.

The inspected Favorites and folder-preview headers and reports support the narrow foundation claims. They are pure projections, reuse canonical lifecycle identities and shared metadata, and explicitly lack native proof. The packet creates no competing Model, metadata codec, private/download classifier, page registry or native route. It preserves immutable Profile targets, same-window multi-Profile requirements and saved-page handoff. No legacy command or string is added; Library 40701 remains untouched.

## Checks run in this review

Commands ran from the coordinator worktree:

```sh
sha256sum arcified/native/production/favorite-previews-decision/*
sha256sum --check arcified/native/production/favorite-previews-decision/inputs.sha256
jq -e . arcified/native/production/favorite-previews-decision/ownership.json arcified/native/production/favorite-previews-decision/assignment.snapshot.json arcified/native/production/favorite-previews-decision/source-identities.json arcified/native/production/favorite-previews-decision/sources.json arcified/native/production/favorite-previews-decision/result.json > /dev/null
for file in arcified/native/production/favorite-previews-decision/*; do
  git diff --no-index --check /dev/null "$file"
  packet_check_status=$?
  printf '%s %s\n' "$packet_check_status" "$file"
done
```

All packet hashes matched the coordinator's supplied review identities. Every recorded input hash matched. jq parsed all five JSON files. Whitespace checks emitted no diagnostics; each new-file comparison returned 1 because the file differs from `/dev/null`.

A `functions.exec` check using `fast_filesystem.fs_read_many` read all eight packet files without errors or truncation. PASS for canonical two-space JSON, LF/final newlines, trailing whitespace, exact authoritative issue equality, required result fields, evidence-path existence within the packet, `nativeAcceptance: OPEN` and `ownerDecisionRecorded: false`. Source review covered every row and all three criteria. No foundation test was rerun or counted as native evidence.

## Remaining gates and coordinator questions

These are accurately tracked limits, not findings:

1. Record the owner's actual scope answer. Recommended question: accept deferring Notion, Figma, Linear, Confluence, Gmail and Outlook Mail previews, and replacing GitHub hover preview only through separately accepted Live Folders? The tradeoff is loss of inline account recents and mail badges while ordinary Favorites remain the proposed navigation path.
2. If a site is required, name its provider/domain/tenant and accepted fields/actions. The packet recommends an explicit row with navigation-only actions. No account access follows from this review.
3. Before implementing a selected provider, settle authentication, storage, retention, revocation, refresh and private-context availability through integration-policy. Profile ownership is already authoritative; provider/account disable and stale presentation remain proposed integration details.
4. Native acceptance stays OPEN on Linux, Windows and macOS. Actual patches and owning GN test registration become applicable after scope acceptance and implementation. Same-window multi-Profile isolation, saved-page continuity, installed interaction, account/disable races, stale failures, accessibility and restart behavior need real native evidence. No builds, runtime tests, visual checks, accounts, remote writes or prepared-source changes ran in this review.

The packet is suitable for coordinator consolidation and owner consideration at the inspected identities. It is not implementation, release or account-integration approval.
