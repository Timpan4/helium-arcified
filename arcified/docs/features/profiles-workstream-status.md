# Profiles workstream status

Checkpoint: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. Branch: `feat/profiles-workstream`.

All three local components are implemented, independently reviewed, fixed and committed for root integration. No local component remains in progress. All three issues retain blocked native acceptance and have no release evidence.

| Issue | Primary task ID suffix | Owned paths | Stage |
| --- | --- | --- | --- |
| #48 profiles | `profiles-workstream-primary-profiles-20261023-v1` | `native/features/profiles/`, `docs/features/profiles.md` | Reviewed, committed `cd5ef303` with shared classifier |
| #103 profile-safety | `profiles-workstream-primary-profile-safety-20261023-v1` | `native/features/profile_safety/`, `docs/features/profile-safety.md` | Reviewed, committed `94695e08` |
| #98 private-browsing | `profiles-workstream-primary-private-browsing-20261023-v1` | `native/features/private_browsing/`, `docs/features/private-browsing.md` | Reviewed, committed `17d27544`; classifier in `cd5ef303` |

Paths are relative to `arcified/`. Task IDs share prefix `node:delegated-task:command%3Amcp%3A58528686-9d16-4f5c-b2ad-7f9b4a69dc10%3Adelegate-task%3A`. T3 accepted and verified all assignments as provider `codex`, model `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`.

## Accepted boundaries

Lifecycle Model v3 remains the ownership and navigation authority. Components consume const graph inputs and existing typed IDs. No parallel ownership, persistence, navigation or focus store is authorized here.

Private browsing owns `private_browsing/context_boundary.h`: caller-supplied exact live native context identity, explicit regular/OTR/guest/system/unknown classification, and privacy eligibility. Profiles consumes those facts for model target validation, without another classifier. Runtime context identity is neither a durable ID nor an original-Profile alias. Durability owns the durable schema; workspace owns projections and native handoff preparation. Adjacent coordinators and root received this contract before implementation.

Portability accepted synthetic metadata projection only. Native export remains gated on verified regular native eligibility. Durability capture must require eligibility for every captured Profile, including unattached Profiles, and reject missing/private/guest/system/unknown facts. Neither area serializes runtime context identity.

Profile safety owns an ID-only impact preview. It has no destructive execution API. Reassignment, cleanup, activity protection, deletion and populated migration policy remain open.

## Blockers and next actions

All components passed strict C++17 synthetic fixtures, ASan/UBSan, Chromium formatting and whitespace checks. Private fixtures also passed GCC strict C++17. All primary/review tasks completed without pending child runs. Independent review task suffixes use the same task prefix and verified model/options above: `profiles-safety-review-round1-20261023`, `profiles-safety-review-round2-20261023`, `profiles-context-review-round1-20261023`, and `profiles-private-review-round1-20261023`. Durability accepted per-Profile and per-page admission callbacks, with empty-window admission for undo. Callbacks alone do not prove native eligibility.

Context/preparation review was clean. Safety review requested sibling-Space live-work exclusion coverage; the added fixture passed independent closure review and strict C++17, ASan/UBSan and format checks. Private review found no code defect and requested correction of a stale-context refusal claim; the documentation now assigns lifetime revalidation to the native caller. All committed diffs passed whitespace checks.

Root integrates the three ordered feature commits, then wires authoritative native call sites when authorized. This branch adds only area-owned feature code, tests and documentation. It changes no shared lifecycle, tooling, patch series, workflow, instruction or root execution catalog. Local commits do not release or close issues.

Native host ownership extraction, actual Profile binding, platform persistence and installed Linux/Windows/macOS interaction checks remain unproven. Chromium preparation/builds and heavy downloads are deferred. Unit tests cannot close these issues. No pushes, PRs, remote issue updates or merges are authorized.
