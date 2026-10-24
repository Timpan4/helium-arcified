# Capture workstream status

Checkpoint: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`.
Branch: `feat/capture-workstream`. Root integrates the branch.

## Assignments

All actual T3 assignments were verified as `codex`, `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`.

| Issue | Owned paths | Stage | Task ID |
| --- | --- | --- | --- |
| #37 capture | `native/features/capture/`, `docs/features/capture.md` | Reviewed and committed | `node:delegated-task:command%3Amcp%3A953350b1-de32-4537-9219-af420dfd10b6%3Adelegate-task%3Acapture-workstream-primary-capture-v1` |
| #60 region-capture | `native/features/region_capture/`, `docs/features/region-capture.md` | Fixed, re-reviewed and committed | `node:delegated-task:command%3Amcp%3A953350b1-de32-4537-9219-af420dfd10b6%3Adelegate-task%3Acapture-workstream-primary-region-capture-v1` |
| #53 recent-files | `native/features/recent_files/`, `docs/features/recent-files.md` | Reviewed and committed | `node:delegated-task:command%3Amcp%3A953350b1-de32-4537-9219-af420dfd10b6%3Adelegate-task%3Acapture-workstream-primary-recent-files-v1` |

Paths are relative to `arcified/`. Coordinator owns this record, integration review and local commits.

## Local commits

Integrate in branch order from the checkpoint. Capture's composed fixture links region's implementation from the later commit.

1. `a37736958f1d7bd74a097f4f70422532bfcfc1dd`: recent-file eligibility, filtering and action/result validation.
2. `cb24dbce56c1a9294d8d6920485d8d47a57d6d15`: capture planning, RGBA preview/crop, export reconciliation and composed fixture.
3. `2eea466e77957e7b2044240856a929c53623ed7b`: region geometry, conservative containment proof, cancellable picker and fixed handoff documentation.

## Accepted boundaries

Existing lifecycle/native services own page identity, navigation, focus, Profile/OTR, records and persistence. Library peer owns Library/downloads/history providers. No cross-area database, state API or shared-header edit is needed. Capture providers remain unavailable until a concrete native provider exists.

Capture owns checked CSS/raster plans and owned RGBA preview/export reconciliation. Targets reference lifecycle identities and host document plus visual revisions. Region owns native-view DIP selection, an inward half-open pixel crop and its exact displayed boundary. The host must map view DIPs to the exact visible-pane capture raster and revalidate target, revision, geometry and policy. No CSS-to-DIP equivalence is assumed.

The concrete seam is capture-local `VisibleRegion{target, raster_size, crop}`, accepted only for visible-pane scope. The host copies the original submitted target/document/visual revision and region output after coordinate mapping. Capture verifies the exact full raster size, validates incoming full RGBA bytes and copies only the bounded crop into preview. Full-page capture with a region is refused. The feature headers remain independent.

Recent-files consumes caller-supplied native observations and Profile-scoped opaque references/revisions. It owns eligibility/filter/action/result validation only. No paths, persistent records or deletion policy are introduced.

## Open work and evidence

Local components: 3 reviewed and committed awaiting root integration, 0 in progress, 0 not started. No local component is released. All three primaries and all review/fix tasks have completed. Coordinator rebuilt and ran the composed picker-to-RGBA fixture with strict C++17 and ASan/UBSan against the final region fix; it passed without diagnostics.

Recent-files review task: `node:delegated-task:command%3Amcp%3A953350b1-de32-4537-9219-af420dfd10b6%3Adelegate-task%3Acapture-workstream-review-recent-files-r1`. Actual assignment: `codex`, `gpt-6.1-sol`, high/default. Reviewer ran strict C++17, all seven fixture groups, ASan/UBSan, Chromium formatting and Clang analyzer. All passed, with only documented invalid-enum fixture suppressions. No actionable finding.

Region review task: `node:delegated-task:command%3Amcp%3A953350b1-de32-4537-9219-af420dfd10b6%3Adelegate-task%3Acapture-workstream-review-region-r1`. Actual assignment: `codex`, `gpt-6.1-sol`, high/default.

Region round 1 accepted findings: P1 subtractive cancellation near a raster pixel edge can retain a hidden/unselected pixel; P2 the doc seam omits the original selection target. Fix task `node:delegated-task:command%3Amcp%3A953350b1-de32-4537-9219-af420dfd10b6%3Adelegate-task%3Acapture-workstream-region-fix-r1` completed with verified GPT-6.1 high/default. The fix proves conservative DIP containment or refuses the candidate, covers 64-bit `long double`, and documents copying the original selection target.

Region re-review task: `node:delegated-task:command%3Amcp%3A953350b1-de32-4537-9219-af420dfd10b6%3Adelegate-task%3Acapture-workstream-review-region-r2`, verified GPT-6.1 high/default. Clean review. Original precision repro now returns empty. Strict, sanitized, 64-bit-long-double and composed fixtures, formatting and whitespace passed. Reviewer also ran a temporary exact-integer oracle over representative adjacent/fractional/subnormal/extreme inputs and found no returned crop outside exact affine or displayed containment. This sampled check is not exhaustive over floating-point inputs.

Capture review task: `node:delegated-task:command%3Amcp%3A953350b1-de32-4537-9219-af420dfd10b6%3Adelegate-task%3Acapture-workstream-review-capture-r1`. Actual assignment: `codex`, `gpt-6.1-sol`, high/default.

Capture review was clean. Reviewer ran strict C++17 with ASan/UBSan for capture and composed picker/capture fixtures, Chromium formatting, and read the pinned native contracts. All passed. The separate region precision finding was fixed and independently re-reviewed afterward.

Native acceptance: 0 of 3 proven. Library/provider handoff, real page acquisition, native clipboard/file dialogs and recent-file operations remain open. Installed Linux, Windows and macOS behavior, accessibility and visual checks remain open. Chromium preparation/build, heavy downloads, benchmarks, global installs and remote CI are deferred. Unit tests cannot close these issues.

Next action belongs to root: integrate the reviewed branch and coordinate later authorized native provider/host work. Native checks must confirm region's floating-point build contract, actual target/policy/frame provenance, OS interactions and Library/downloads handoffs. No push, PR, remote issue update or merge was performed or authorized.
