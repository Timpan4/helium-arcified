# Foundations publication review, round 2

Result: both original P3 findings are closed. No new blocking aggregate defect was found. Two nonblocking documentation findings remain at the exact candidate below. The source remains suitable for bounded foundation publication, subject to root's publication and required-check gates. This report does not accept native features, installed UX or issue completion.

Reviewed on 2026-10-23 in `/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261`.

| Boundary | Exact identity |
| --- | --- |
| Candidate | `d226b789e368a1d59574a3cd723588e0e9f493cc` |
| Previous reviewed candidate | `05ace22efb8564d0294b8efa1b3f3651b76cef07` |
| Local `origin/main` and merge base | `09670c97b1c308d0fb31869b3a9a9bc6969bc47f` |
| Metadata followup | `b47468aec9b5aaa9a3f8fddad584e3681c223cbf` |
| Mandatory admission followup | `f634070a269e928632dd178855604d840d8b07dd` |

The candidate is frozen. Findings and relative-link checks use its Git objects, not later working-tree edits. The working `execution-state.json` changed during this review; that later change is outside this result. No fetch or remote inspection occurred. Only this report was written. No commit, PR linking/watching, remote write, authentication change, preparation, compilation, native test or heavy check was performed.

## Original findings closed

1. The obsolete admission description is replaced in [parallel-integration.md](../parallel-integration.md), line 25. It identifies whole-snapshot and current decoded-candidate commit admission at `f634070a`, links the correctness and contract reviews, records the separately reviewed metadata followup, and marks layout and inventory integrated. Native retention, freshness, serialization, real IO and same-window cross-Profile feasibility remain open. Both cited commits are candidate ancestors. This closes original finding 1 without treating the pure admission API as a native implementation.
2. [execution-state.json](../execution-state.json), lines 254 and 255, explicitly identifies the lifecycle-test packet as local, uncommitted and excluded from frozen foundation evidence. The obsolete next-action reliance on a committed completed packet is removed. Git-tree inspection confirms the packet remains absent from this candidate. This closes original finding 2. Neither that packet nor the unaccepted destruction patch is counted as committed source evidence here.

## Remaining nonblocking findings

1. **P3, reconcile current progress statements with completed evidence.** [execution-state.json](../execution-state.json), line 130, newly says Space metadata changes are in progress and require fresh review. Line 136 and its completed assignment correctly record reviewed commit `b47468ae`. Line 80 still asks children to finish metadata and destruction review and root to prepare Linux, although those review/preparation stages are recorded complete elsewhere in the same snapshot. [execution.md](../execution.md), line 15, still groups Linux with unverified preparation/GN; lines 51 and 55 describe the closed inventory and metadata work as continuing or unfinished. Update these current-status statements to distinguish completed standalone reviews and Linux preparation from open native implementation, compilation and runtime proof. Keep historical review/assignment records historical. This is contradictory progress documentation, not evidence that the codec or preparation failed.
2. **P3, mark excluded targets in the newly published destruction review.** [native-destruction-extraction-review.md](native-destruction-extraction-review.md), lines 63, 64 and 73, defines relative links to `arcified/native/patches/window_lifetime/browser-destruction-authority.patch`, `arcified/docs/features/native-destruction-extraction.md` and `arcified/native/patches/window_lifetime/inputs/ui/views/widget/widget.cc`. All three targets are absent from the candidate tree. Its draft/unaccepted verdict and retained hashes correctly avoid native acceptance, but a fresh clone cannot inspect these linked artifacts. Add an explicit local/uncommitted/excluded evidence note and render unavailable paths as such, or defer this review's publication with the separately reviewed packet. Do not add the unaccepted patch merely to satisfy links. This does not reopen original finding 2, which concerned the execution-state packet claim.

## Metadata followup and aggregate scope

Read the independent [metadata review](space-metadata-durability-review.md), its [response](space-metadata-durability-response.md), and the four matching feature documents. The metadata implementation was not reviewed again. The only native changes since the previous aggregate candidate are the eight durability, schema-gate and persistence-composition files introduced by `b47468ae`; none changed between that commit and this candidate. Its four feature documents and two retained review/response files also remain the metadata followup's committed inputs.

The aggregate contract is accurately bounded. Schema 2 preserves canonical ordered Space metadata through the standalone codec and staged restore. The supported-format header registers the current schema with lifecycle contract 3. Schema 1 is described as an actual prior local format, with no invented lossless upgrade or shipped-browser migration claim. Unsupported originals and candidates remain recovery cases. Folder, custom theme/palette, split presentation and other omitted payloads still need explicit absence. Production binding, serialized borrowed lifetimes, disk durability, installed adoption/restart and platform acceptance remain open.

A complete focused search of native C++/GN/tooling and workflow inputs found no remaining native `Schema1PeerPayloads` reference or additional restore caller outside the reviewed durability/composition sources. The documented durability, migration and composition compile commands include the added `space_management.cc` dependency. No production GN registration, new workflow coverage or applied browser feature is inferred. Workflows, production patch inputs, preparation tooling and the original captured-source/license packet are unchanged since the previous aggregate review.

The [first aggregate review](foundations-publication-review.md) remains the retained evidence for the original 51 components, its 170-artifact identity comparison, final-review hashes and original source/license accounting where inputs are unchanged. Its schema-1 discussion is historical to `05ace22e`; the metadata review supersedes that limitation only for the standalone schema-2 join. No component or license check is relabeled as a new execution here. Counts remain 51 reviewed local components integrated, zero remaining local components to integrate and zero native issues accepted. The non-AI ledger remains zero proven done, one locally done awaiting publication, 56 in progress and 78 not started.

## Publication and Linux evidence

The [Linux run record](../native-linux-run-20261023.md) names preparation commit `05ace22e`, Chromium `155.0.8059.79`, Linux platform commit `b154afccb44729359c816754c7741efb3cb8f497`, local paths, exact preparation/build commands and retained artifact hashes. Read the complete local `preparation.json`: all 17 steps are `PASSED`, preparation is `PREPARED_NOT_BUILT`, and native build/launch remain `OPEN`. Its fork/platform identities agree with the report. Independent SHA-256 checks match both recorded `preparation.json` and `args.gn` hashes. The GN log confirms 35,690 generated targets from 5,018 files. These are preparation evidence, not compilation or native feature coverage. This review did not inspect the live build process or certify the documented running compilation.

The root-only heavy-work authorization, explicit manual resume, disabled/disarmed game guard and retained interruption ledger agree across the new records. Both frozen state JSON files parse successfully. The historical interruption entries do not authorize restarting completed or unrelated work. This review checked the records, not live scheduler state, and exercised none of the recorded permissions.

The publication block correctly records GitHub's missing OAuth workflow scope, pending owner authentication approval and no successful implementation push/PR/CI publication. No credential workaround is proposed or attempted. Remote state and approval remain root-owned and unverified here. Browser launch, installed UX, Windows/macOS execution and issue closure remain open.

## Checks and limits

| Check | Result and coverage |
| --- | --- |
| Commit/base resolution and ancestry | Full identities above resolved; previous candidate and both followup commits are candidate ancestors. Local `origin/main` is the merge base. No remote freshness or mergeability claim. |
| Frozen delta and unchanged-input comparison | Enumerated the complete delta since `05ace22e`; metadata native inputs are unchanged after `b47468ae`. Original research/license and production preparation/patch/workflow inputs are unchanged. |
| Relative Markdown targets | Checked inline and reference-style relative file targets in all 12 new/changed Markdown files since `05ace22e` against the candidate Git tree, recognizing directory targets and URI schemes. Of 65 targets, 62 exist and three are the excluded targets in remaining finding 2. Anchors, external URLs and T3 thread availability were not validated. |
| State JSON | Parsed both frozen JSON documents; checked counts, root ownership, guard/resume state and publication block. No live orchestration inspection. |
| Linux retained evidence | Read complete preparation manifest, final GN arguments and GN log; both published artifact SHA-256 values match. No preparation or build rerun. |
| `git diff --check 05ace22e..d226b789 -- arcified/docs arcified/native` | Exit 0. Checks the scoped committed followup, not retained upstream capture formatting. |
| Focused dependency/API search | Untruncated search over 227 matching native/workflow files, checked against an empty candidate-to-working native diff. No additional restore caller or stale native peer type found. No compilation result is claimed. |

No component suite, sanitizer, formatter or analyzer was rerun. The metadata response retains those implementation checks; its independent review explicitly distinguishes them from its own light checks. No Markdown formatter/linter is declared for this report path in the retained review evidence. Root owns fixes, publication authentication, actual PR checks and native acceptance. The two P3 findings above prevent calling this a fully clean documentation closure, while leaving the bounded foundation verdict nonblocking.
