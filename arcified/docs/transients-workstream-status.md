# Transients workstream

Base: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. Branch: `feat/transients-workstream`.

Reviewed local components: 3. Awaiting root integration: 3. In progress: 0. Not started: 0.
Native acceptance proven: 0 of 3. All issues remain open.

| Issue | T3 primary task ID | Owned paths | Stage |
| --- | --- | --- | --- |
| [Peek #79](https://github.com/Timpan4/helium-arcified/issues/79) | `node:delegated-task:command%3Amcp%3A1cd1016a-71bc-43db-862e-2ae7c1ce570f%3Adelegate-task%3Atransients-peek-primary-20261023-v1` | `arcified/native/features/peek/`, `arcified/docs/features/peek.md` | Clean independent review; committed `82e1f82a08892969be463cdcbe437bd6249be26f` |
| [Little Arc #111](https://github.com/Timpan4/helium-arcified/issues/111) | `node:delegated-task:command%3Amcp%3A1cd1016a-71bc-43db-862e-2ae7c1ce570f%3Adelegate-task%3Atransients-little-arc-primary-20261023-v1` | `arcified/native/features/little_arc/`, `arcified/docs/features/little-arc.md` | Clean independent review; committed `9328e0a74607254a9f28fe7c4373014c1fef4e2c` |
| [Air Traffic #80](https://github.com/Timpan4/helium-arcified/issues/80) | `node:delegated-task:command%3Amcp%3A1cd1016a-71bc-43db-862e-2ae7c1ce570f%3Adelegate-task%3Atransients-air-traffic-primary-20261023-v1` | `arcified/native/features/air_traffic/`, `arcified/docs/features/air-traffic.md` | Clean independent review; committed `a4d33249dce55e430ec55d43e210eff62040776e` |

All assignments verified as provider `codex`, model `gpt-6.1-sol`, options `reasoningEffort=high`, `serviceTier=default`.

## Shared interfaces

Lifecycle identities and graph remain authoritative. Features must not duplicate page ownership, current navigation, Profile records, persistence or focus state. No shared core changes are authorized without a concrete proposal to root.

Proposals sent to adjacent coordinators: commands owns page/overlay command focus; profiles supplies authoritative target identity and privacy eligibility; workspace owns attachment and native-success transitions.

Accepted internal boundary: Air Traffic returns a routing choice containing Profile, optional Space and surface. The caller resolves and provisions the actual attachment. Little Arc consumes explicit `{ProfileId, Attachment}` and plans entry/promotion against the const lifecycle graph. Same-attachment expansion keeps page and host; temporary cross-attachment promotion remains unsupported. Private confirmation, unavailable capability and stale targets return explicit outcomes rather than silent fallback. Peek and Archive do not enter external routing. Modules remain uncoupled; no second ownership or navigation store is introduced.

Accepted commands seam: `PageCommandTarget{PageId, ProfileId, Attachment}` resolves from an explicit focused page and expected attachment, then revalidates against the lifecycle graph. No selected-item or global-focus inference. Native execution separately validates object binding and operation generation against replacement or ABA transfer. Transient presentation and focus entry/exit remain caller-owned.

## Blockers and next actions

Same-window multi-Profile production ownership is unproven. Native builds, platform UI evidence and heavy preparation are deferred. Temporary-page transfer, active-work dismissal, idle cleanup, destructive actions and populated cross-Profile migration must not acquire invented policy through these components.

Little Arc independent review was clean and reran six standalone fixtures in ordinary and ASan/UBSan builds, Chromium formatting, explicit clang-tidy and whitespace checks. Review task: `node:delegated-task:command%3Amcp%3A1cd1016a-71bc-43db-862e-2ae7c1ce570f%3Adelegate-task%3Atransients-little-arc-review-r1-20261023`, verified GPT-6.1 High/default. No native acceptance follows from these checks.

Peek review: `node:delegated-task:command%3Amcp%3A1cd1016a-71bc-43db-862e-2ae7c1ce570f%3Adelegate-task%3Atransients-peek-review-r1-20261023`, clean. Air Traffic review: `node:delegated-task:command%3Amcp%3A1cd1016a-71bc-43db-862e-2ae7c1ce570f%3Adelegate-task%3Atransients-air-traffic-review-r1-20261023`, clean. Both verified GPT-6.1 High/default. Both reviewers reran passing ordinary/sanitized fixtures, formatter and analyzer checks. No actionable findings or outstanding fixes remain in these bounded components.

Root can integrate the three component commits in branch order. Exact focused commands and coverage are in the feature documents. No shared lifecycle, tooling, patches, workflows, instructions or execution catalog changed.

Next native work requires owner-coordinated production ownership extraction, real event/URL authority, attachment provisioning, prepare/commit and stale-event reconciliation, focus/security surfaces and installed Linux/Windows/macOS checks. Little Arc temporary promotion and Peek dismissal need the unresolved ownership/activity policy. Air Traffic still needs native configuration/confirmation UI and durable rule storage. No native acceptance or issue closure is inferred from local tests.
