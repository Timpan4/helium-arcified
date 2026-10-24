<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Runtime reference independent review, round 1

Clean bounded review. No actionable findings in the reviewed research packet. This accepts its documentary preparation and honest blocked disposition only. It does not close issue 42, approve owner proposals, or establish reference/runtime/native behavior.

Reviewed on 2026-10-24 for [issue 42](https://github.com/Timpan4/helium-arcified/issues/42), baseline `a4b6b480d3de07857ab6e939149254a8359e5449`. No prior findings. Only this report was written. No packet, source, root control, account, runtime, build, visual session or remote state was changed.

## Authority and input identity

Read the full selected issue in `/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/arcified/docs/production/research/assignment.json`, the root workstream manifest, and applicable lifecycle, native integration and execution authorities through fast_filesystem. The packet's `authoritative-issue.json.issue` equals the complete selected assignment object, including all three acceptance criteria, dependency, source families, source URLs and command 40799.

All eight packet hashes independently matched the coordinator-supplied review inputs. Paths below are relative to `arcified/native/production/runtime-reference/`:

```text
5add3e7aef1a5be8063939ecc3b1f18b442e9f494c8cfcca7feffe11ca9317ee  ownership.json
7a5fbdcb8fad46806155c1d9878d23d4877f42ad8b6fb1c6b642602211a9e69a  authoritative-issue.json
83daf61cd9ae7e26c24ab237b29eb9f825df6cc19baef3e54ea30a7317784ee8  sources.json
3483669c398358d993dd7a58d85b812507998d50a97b57764b5b3503ff104c19  scenarios.json
20f97844ddd2e64627a81d12a69fc49cfcdd8777eeb2bb0417f827fb848a1b6d  README.md
b58319457e5f0296b6b9413582d48d42fa4a9e6fddb66f16911568468c1858fd  verify.jq
8902b00a5be89c6908baf3042c1cfdcfce8c046637e4047ca590f2f68b500b4d  result.json
512c28a9ce4ab1b855319e2aa4ec3ec2cbd9f53339d08930e88741500615ba17  checks.json
```

Independent `sha256sum` checks covered all eleven paths in `sources.json.localSources`. Assignment, lifecycle, native integration, execution and six foundation-document identities matched the packet. The root manifest matched its explicitly recorded later observation, `daf98f1cf993c1a21686289a88489bf001266d7fff4d85089d30055584c8a7b0`, rather than its initial hash. These identify inspected snapshots, not a frozen current release state.

## Criteria and scenario coverage

1. Criterion 1 is correctly OPEN. `sources.json:69` retains distinct recorded macOS Arc and Dia identities, hashes, architectures and acquisition limitations. They agree with `arcified/docs/research/arc-dia/artifacts.md`. No installer bytes, signatures or runtime were independently verified here. Windows artifact fields remain explicitly null and dependency blocked. Linux is explicitly a downstream verification target with no official reference product, as the authoritative criterion states.
2. Criterion 2 is correctly OPEN. Reviewed every scenario, not a sample. `scenarios.json` covers Peek/pin URL rules, tab handoff, folder preview, split transitions, sidebar reveal, media return and native dialogs. Every family has steps, capture requirements, unknowns, downstream constraints and all five platform entries. The matrix contains 14 macOS NOT_RUN rows, 14 Windows BLOCKED_PACKAGE_AND_RUNTIME rows and seven Linux NO_OFFICIAL_REFERENCE_PRODUCT rows. All capture arrays are empty. The reported 28 unexecuted product/platform rows and zero observations/passes are accurate.
3. Criterion 3 is correctly OPEN. `README.md:38` proposes acceptance amendments without modifying authority. The packet separates documentary macOS folder preview from unknown Windows preview, inaccessible login/runtime from retirement, and retained legacy selectors from reachable actions. Missing Peek documentation is not treated as proof of feature retirement. Retirement/service disposition is routed to issue 23. Runtime reconciliation and actual acceptance edits remain outstanding in `result.json:79`.

The capture contract at `scenarios.json:5` requires exact package/OS identity, authorization references, observed steps/outcomes, reachability, platform differences and capture identities. It explicitly separates inaccessible, retired, unsupported, not-found and unrun states. Empty source links for tab handoff preserve the lack of documentary proof instead of inventing reference behavior.

The all-active-leaf coverage requirement for release-ux-check is outside this issue 42 review. This review proves complete inventory coverage of this leaf's authoritative criteria only.

## Primary-source accuracy

Rechecked these first-party sources on 2026-10-24 using read-only web retrieval:

1. [Arc farewell](https://arc.net/) supports the packet's published signup closure and future desktop sunset/sign-in dates. The packet correctly calls them publisher statements, not observed login or shutdown results.
2. [Dia changelog](https://www.diabrowser.com/changelog) supports the platform-specific pinned-URL, reveal/split, transition and external-app-dialog leads. The packet preserves the distinction between the displayed Mac release label and the recorded DMG identity. Release notes are not substituted for package or runtime evidence.
3. [Pinned Tabs](https://resources.arc.net/hc/en-us/articles/19231060187159-Pinned-Tabs-Tabs-you-want-to-stick-around) and [Folders](https://resources.arc.net/hc/en-us/articles/19228419623447-Folders-Stash-Similar-Tabs-Together) were available as first-party search-cache text, reported crawled five months earlier. Their platform and preview claims agree with the packet. Direct retrieval failed with 404 for Pinned Tabs and a fetch timeout for Folders during this review. These results do not invalidate the packet's separately dated direct-fetch history or establish feature retirement.
4. [Master your multitasking](https://start.arc.net/master-multitasking) supports split creation and Mini Player/PiP entry as documentary leads. Its Mac shortcuts do not prove Windows parity or source-return identity, which the packet leaves unknown.

No remote page-byte archive or application-runtime proof was recovered. Recorded REA evidence and temporary artifact locations remain inherited provenance with disclosed availability limits. No installers were downloaded or run.

## Decisions and native boundary

The two owner questions in `result.json:28` are proposals routed through the coordinator, with recommendations and tradeoffs. Neither records an answer, enables a feature, nor invents a cleanup, destructive-action, provider, source-disposition or navigation policy.

`README.md:48` and the scenario constraints preserve the authoritative Model, metadata/codec, immutable Profile/page targets and existing classifier ownership. No competing store or routing authority is introduced. Root retains lifecycle ownership and focus-routing-contract receives the presentation questions. Same-window real multi-Profile support and exact live saved-page handoff remain genuine native gates. Model::OpenItem success, URL equality and screenshots alone cannot close them.

The research-only native patch/test/message/ActionItem disposition is justified. No command is implemented, so no legacy ActionItem mapping is missing. Library 40701 and this leaf's reserved 40799 are not changed. A fake production patch or synthetic native completion claim would be inappropriate here.

## Independent checks

Executed from the coordinator worktree:

```sh
sha256sum arcified/native/production/runtime-reference/{ownership.json,authoritative-issue.json,sources.json,scenarios.json,README.md,verify.jq,result.json,checks.json}
jq -s -e -f arcified/native/production/runtime-reference/verify.jq arcified/native/production/runtime-reference/authoritative-issue.json arcified/native/production/runtime-reference/sources.json arcified/native/production/runtime-reference/scenarios.json arcified/native/production/runtime-reference/result.json
for file in arcified/native/production/runtime-reference/*.json; do jq --indent 2 . "$file" | cmp - "$file" || exit; done
```

All exited 0; the contract check returned `true`. Also reran the exact recorded JSON-format command from checks.json successfully. Hosted JavaScript over complete fast_filesystem reads verified JSON parsing, LF/final-newline/trailing-whitespace rules, full selected-issue equality, exact remaining-criterion text, all row counts and empty captures. fast_filesystem metadata checks confirmed every result evidence path and README local-link target exists. These are light documentary checks, not native tests.

## Remaining gates and coordinator handoff

No correction is requested to the reviewed inputs. The following remain open:

1. Reconcile the concurrently delivered [Windows artifact packet](../../../native/production/reference-windows-artifacts/result.json). It exists at review time and reports descriptors/bootstrapper evidence while full browser payload hashes, signatures, installation and runtime remain open. This is a handoff update, not a defect in runtime-reference's explicitly dated NOT_PRESENT_AT_CHECK snapshot. The dependency packet was read for this state update, not independently accepted by this review.
2. Allocate supported macOS/Windows hosts and authorized test account/Profile references, resolve the historical-only fallback proposal, and authorize any later runtime session separately through the coordinator. No owner answer is supplied by this report.
3. Execute and retain the actual per-platform captures and scenario notes, or record precise inaccessible/retired/unsupported results. Reconcile affected acceptance criteria without claiming generic parity.
4. Prove downstream installed native behavior, including same-window multi-Profile isolation, exact-object saved-page handoff and focus/dialog/media targets. All three issue criteria and nativeAcceptance remain OPEN.
