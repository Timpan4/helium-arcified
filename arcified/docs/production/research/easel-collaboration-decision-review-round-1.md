# Easel collaboration decision, independent review round 1

Result: **clean bounded research review, no actionable findings**. The packet is
ready for coordinator consolidation as documentary research and a recommendation.
Issue #30 is not complete. The owner's collaboration choice remains PENDING,
the third acceptance criterion remains partial, and native acceptance remains
**OPEN** on Linux, Windows and macOS.

Reviewed on 2026-10-23 UTC against the complete assigned issue object and root
authority in `/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/`.
The assigned baseline is `a4b6b480d3de07857ab6e939149254a8359e5449`.
This review changed only this report. It did not modify the packet, shared
controls, prepared Chromium source or PR state, and did not delegate.

## Findings and acceptance coverage

No severity/path/line findings require correction within this research packet.

1. The historical workflow and its limits are supported.
   [decision.md](../../../native/production/easel-collaboration-decision/decision.md)
   lines 11-22 separate permissioned shared editing and presence from local
   boards, read-only published snapshots and independent exported copies.
   It explicitly labels the shared-editing sequence a documentary reconstruction,
   leaves invitation UI and simultaneous-edit correctness unobserved, and does
   not infer a protocol or editable Arc export format. Acceptance criterion 1
   is delivered at documentary scope.
2. Deferral is a recommendation, with concrete prerequisites and a tradeoff.
   Decision lines 26-37 require a demonstrated task, accountable service owner
   and access/retention decisions before implementation. It neither interprets
   broad non-AI work authorization as service approval nor selects a provider,
   retention period, cleanup rule or synchronization mechanism. Acceptance
   criterion 2 is delivered as requested.
3. Owner choice is honestly incomplete. Decision lines 32-43 and
   [result.json](../../../native/production/easel-collaboration-decision/result.json)
   lines 118-128 preserve PENDING and identify the missing response. The fallback
   distinguishes proposed portable board/image export from verified capability.
   It preserves originals, reports unsupported content, and promises neither
   shared identities/history/presence migration nor recovery of unavailable
   hosted data. Acceptance criterion 3 remains partial pending the owner.

The packet covers the relevant documentary scenarios: granting, removing and
restoring editor access; presence; local editing; explicit snapshot publication
and later-edit isolation; the service boundary of revocation; independent file
copies; image export versus editable migration; missing or unsupported export;
and the conditional decision to investigate live collaboration. Decision line 51
also leaves reference presence, concurrent editing, reconnect/conflicts,
revocation and export access unobserved. These are explicit evidence limits,
not defects in a decision-only packet.

## Authority, sources and scope

The issue object in
[authority.json](../../../native/production/easel-collaboration-decision/authority.json)
matches the authoritative assignment exactly, including all acceptance criteria,
empty dependency list and reserved command ID 40735. Its copied owner-decision
and open-decision lists also match `execution-state.json`. The root manifest
allocates this leaf to the research coordinator and keeps native acceptance OPEN.
Neither the applicable lifecycle ledger nor those owner-decision records settles
the collaboration choice.

Independent primary-source retrieval reproduced the material historical claims:

1. The [Arc macOS release notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
   document a presence-cursor repair under March 14, 2024, v1.34.0, and restoration
   of removed edit permissions under February 8, 2024, v1.29.0. The retrieved page
   placed these at lines 837-842 and 883-890 respectively. These entries support
   historical presence and permissioned shared editing, not runtime correctness
   or present service availability.
2. Direct retrieval of [Easels: Capture & Create](https://resources.arc.net/hc/en-us/articles/19231142050071-Easels-Capture-Create)
   returned HTTP 404. Search returned first-party text identified as crawled five
   months earlier, supporting Mac-only authoring/editing, viewing in other
   browsers and PNG export. This reproduces the packet's stated cache limitation.
3. The cited sunset URL remained inaccessible through the retrieval tool.
   The packet derives no shutdown timeline or service/export availability claim
   from that failure. It does not convert inaccessible evidence into feature
   absence.

The existing Easel, board-export, board-sharing and shared-board issue artifacts
support the packet's separation of local authoring/export, snapshot publication
and live collaboration. The UX dossier separates Easel presence from collection
snapshots and retains schema/protocol uncertainty. The artifact dossier describes
static inspection, not application execution. None is presented as native proof.

Decision lines 47-51 preserve the lifecycle Model, metadata/codec, command
targets, private/download classifiers and actual Profile identity. They retain
same-window multi-Profile switching and exact live saved-page handoff as root
production gates. No fake service, success stub, alternate ownership model or
command wiring is introduced. Empty prepared-source/patch/native-test fields
are appropriate for this decision leaf; adding a native capability is unjustified.

## Independent light verification

1. `fast_filesystem.fs_read_many` read every packet file, the full assignment,
   root manifest, lifecycle contract, native integration authority, execution
   state and recorded local foundation documents. Scoped `fs_rg` checks located
   the relevant UX and artifact claims. Reads were complete, without truncated
   file content.
2. `sha256sum` over the seven explicit packet paths returned exit 0 and matched
   every review input supplied by the coordinator:

   ```text
   ddd42f2c40302d2c02b75c2c4bb34beb8224731e90f917be98097f88c84524d3  authority.json
   586231e52f8bea808c93bb2ac23144868eb29810a16c8f2a70269dde286a1ac6  checks.md
   a9c70f52758938c8ec9855676e907f6260043c4c2a3238f44da6d6c55dd1ad5e  decision.md
   821056f0d031689ca0d9f78447e594f7283f6b408d5f8aa7f13e73e120b4af84  ownership.json
   8ee7bb945cbdebf0c722a076e029d0c8de870ce6ce9ecb2c9d0343569cf4b6b3  result.json
   ed0df710af86e5123552bbc17a6667eab3e7e76ee90429e94da1238d13c69296  source-identities.sha256
   793e82542a74dfa7b8bde7422acb3880a21dbb0368aa4d8bbbbb8538a77e0d04  sources.json
   ```

3. `sha256sum -c arcified/native/production/easel-collaboration-decision/source-identities.sha256`
   returned exit 0. All 14 recorded root/foundation source identities matched.
4. Hosted JavaScript independently parsed every packet JSON file, compared its
   bytes with `JSON.stringify(value, null, 2) + "\n"`, and checked every packet
   file for final LF, CR characters, trailing whitespace and conflict markers.
   All checks passed. Exact issue and owner-ledger comparisons passed.
5. No native preparation, build, tests, benchmarks, CI, browser/visual interaction,
   reference-app execution or account access ran. The prepared source remained
   outside this review's execution path. The primary's unavailable Markdown
   formatter/linter is recorded in `checks.md`; this review does not claim those
   tools ran.

## Remaining owner and native gates

The coordinator should consolidate the existing questions without recording an
answer on the owner's behalf:

1. "Do you approve deferring live board editing and presence, while retaining
   local boards/export and treating any hosted snapshot sharing as a separate
   decision?" Recommendation: defer. Tradeoff: no live teamwork.
2. If deferral is rejected: "Which concrete multi-user board task requires live
   editing or presence, who owns the service, and what access, exposed-data,
   retention, revocation and service-exit model do you approve?" Recommendation:
   settle these before implementation. Live collaboration stays unavailable.

Actual owner choice must reach the authoritative decision record through the
coordinator/root's authorized workflow. Local board/export/snapshot production
verification remains with its owning leaves. Linux, Windows and macOS native
acceptance, same-window multi-Profile behavior and exact live saved-page handoff
remain **OPEN**. This clean research review approves no service, native feature,
issue closure or runtime parity claim.
