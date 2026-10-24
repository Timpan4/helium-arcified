# Splits workstream

Checkpoint: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. Branch: `feat/splits-workstream`.

## Current state

Three primaries, both fix followups and both review rounds completed, all verified as `codex`, `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`. Local components reviewed and passing focused checks: 3. Awaiting root integration: 3. In progress: 0. Not started: 0. Native acceptance proven: 0.

| Issue | Task ID suffix | Exclusive ownership | Stage |
| --- | --- | --- | --- |
| #81 split-horizontal | `splits-split-horizontal-primary-v1` | `layout.h`, `layout.cc`, `layout_test.cc`, `horizontal.md` | Clean local review |
| #112 split-vertical | `splits-split-vertical-primary-v1` | `geometry.h`, `geometry.cc`, `geometry_test.cc`, `vertical.md` | Fixed, clean local review |
| #169 split-persistence | `splits-split-persistence-primary-v1` | `persistence.h`, `persistence.cc`, `persistence_test.cc`, `persistence.md` | Clean local review |

Task ID prefix: `node:delegated-task:command%3Amcp%3A40b0fcf0-9ec2-48cf-8de1-cd9004ca059e%3Adelegate-task%3A`.

## Boundaries

Lifecycle `Model` owns pages, ordered split membership, attachments and navigation. Split metadata references those identities. Active page comes from the caller, never a second focus registry. Layout area owns reversible visibility suppression; native host owns bounds and scroll. Durability area owns the only workspace schema, file IO and recovery. Split persistence only validates and reconciles presentation metadata against authoritative restored identities. No shared lifecycle, tooling or patch files will change without a concrete proposal to the parent.

## Blockers and next actions

Accepted core metadata is orientation and positive finite PageId-keyed relative weights. Missing current-member weights use dimensionless `1.0`, preserving valid explicit survivor weights. Geometry consumes caller bounds, minima and active PageId. Restore reconciliation consumes the caller's old-to-restored PageId map and canonical split, reports invalid/missing/ambiguous references, and defaults affected weights without changing pages. Conflicting sources for one target cannot choose a weight by key order. No selection or wire envelope lives here.

Durability accepted `LayoutMetadata` and owns `features/durability/session_snapshot.h`, its schema, codec and recovery. Its initial schema explicitly omits split metadata and requires feature-completeness attestation, refusing unsupported stores instead of silently dropping them. This area adds no wire envelope; root owns later serialization integration. Metadata restart proof remains open. Workspace focus/selection scope is still pending and is absent from these components.

Each component passed its standalone strict C++17 compile/run and Chromium format validation. Geometry also passed ASan/UBSan after both numerical fixes. Independent round 1 findings, fixes, clean round 2 and reviewed hashes are recorded in [review.md](review.md). Followup task suffixes are `splits-geometry-review-fixes-v1`, `splits-persistence-review-followup-v1`, and review suffixes are `splits-independent-review-round1` and `splits-independent-review-round2`. No source changes followed the clean review. Root can integrate the local components, then own future native/session integration.

Reviewed layout peer boundary at `127bf18939ebcfb21b8bc9351d35946d19d2c7ac`: host composes focus chrome override, tab sidebar override and sidebar context, then supplies sidebar `layout().content` dimensions to split projection and translates local output by that rectangle's origin. Suppression preserves metadata/pages; current bounds trigger reprojection. No source overlap or peer edits.

All native acceptance stays open. Production same-window multi-Profile ownership extraction, native focus/security/permission integration, session joins and installed Linux/Windows/macOS interaction evidence are absent. Chromium preparation/builds, heavy downloads, benchmarks and remote CI remain deferred. Local component tests cannot close the issues.
