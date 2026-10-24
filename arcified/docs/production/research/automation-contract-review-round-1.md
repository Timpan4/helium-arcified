<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Automation contract independent review, round 1

Review date: 2026-10-24. Issue: [25](https://github.com/Timpan4/helium-arcified/issues/25).

Verdict: clean within the research and decision scope. No actionable findings. This accepts the packet as evidence for an owner decision, not as approval, implementation, native acceptance or issue completion. `RESEARCH_COMPLETE_OWNER_DECISION_PENDING` accurately describes the result. Native acceptance remains OPEN.

## Scope and inspected identities

Reviewed all six packet files, the full assigned issue object, root workstream controls, lifecycle contract, native Space integration authority, execution-state owner decisions and the cited artifact dossier. The packet's retained `authoritativeIssue` exactly matches the issue object in the inspected assignment, including every acceptance criterion, source URL, decision classification and command allocation. No nested delegation, packet modification, source modification or remote mutation occurred. This report is the only authored file.

Packet paths below are relative to `arcified/native/production/automation-contract/`. Reviewer SHA-256 results exactly matched all six identities supplied for this round.

| File | SHA-256 |
| --- | --- |
| ownership.json | `9619fc64aa7fffbc86f0ad8c95c2cc491610c1b138f0b5d353dc5b06ab09256b` |
| decision.md | `b90ad4c83c38c396c296d27664e84a04cd4fa2c5879c12f684fb21a7ecb756b3` |
| sources.json | `bc50e42987fb5b37c77d9e6bc7afd4358916dfb07a79a530522c7fe0bc961efd` |
| prepared-source.sha256 | `570e939a7f980e018e2440ded8f56f39d6e8c92aa8e102ca416b795b7f855a60` |
| verification.md | `cac6f1648c45ceabbffe39f6983718cbdee0d122b7d873c303b044f60844677b` |
| result.json | `c58ce8dd41b18de4e3adef581d675d4fbe8217036866f8fff132d3cf40b896f3` |

Root control paths below are relative to `/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/arcified/docs/`. These are inspected identities during this review. Concurrent root work can change them; they are not a frozen current release state.

| Authority | SHA-256 |
| --- | --- |
| production/research/assignment.json | `cc8ec03443b18a749b486e484a56b86218d8f41653970dea94185f76c9685354` |
| non-ai-production-workstreams.json | `daf98f1cf993c1a21686289a88489bf001266d7fff4d85089d30055584c8a7b0` |
| lifecycle-contract.md | `244488864d15d093f7f80e1cc40f490a979454e2a8e74db438c7e51283518a1c` |
| native-space-integration.md | `e5951ba265be3587cb1df8f2ab73ce6768071ac55b8d481240ed57819b39ab56` |
| execution-state.json | `dae2aed7885b1f03f5f96a1285265a1189b8776a6472880c0f12c94de7f7ad02` |

`execution-state.json` is at the docs root, not under production/research. The inspected owner ledger contains no automation-specific scope approval. The broad non-AI implementation authorization does not supply the issue's missing security decision.

## Acceptance coverage

All three assigned criteria were assessed. None is silently dropped or represented as native success.

| Authoritative criterion | Evidence and assessment | Remaining gate |
| --- | --- | --- |
| Choose a first command and document transport, identity and permissions per OS. | decision.md:23 describes Linux, Windows and macOS separately. decision.md:45 proposes the exact `tabs.list` request, opaque handles, explicit actual Profile/window scope, revocation and refusal behavior. Host manifest/origin identity is distinguished from helper attestation and Profile authorization. | Owner selection and actual package/helper/permission validation per OS remain OPEN. |
| Authorize JavaScript, private content and mutations separately; expose no general privileged execution endpoint. | decision.md:49 requires trusted browser UI and rejects request-driven scope expansion. decision.md:50 rejects arbitrary API/script/shell/numeric command dispatch. decision.md:59 separates metadata, content, script execution, private/guest access, mutations and external IPC. No production endpoint or adapter is added. | Any later capability expansion needs its own approved contract and native evidence. |
| Owner records scope approval or deferral; document unsupported OS routes honestly. | decision.md:75 and result.json:62 retain explicit owner questions and recommendations. result.json:102 keeps the owner record pending. Every platform state records unavailable adapters, no native execution and OPEN acceptance. AppleScript compatibility is deferred; Windows/Linux reference parity is not inferred. | The owner must record the exact choice or explicit deferral in issue 25 through the coordinator. This part of the criterion is not complete. |

The verification matrix at verification.md:25 covers the proposed command, default denial, wrong caller/helper, actual Profile isolation, same-window multi-Profile behavior, exact live saved-page handoff, private/guest exclusion, response minimization, forbidden commands, lifecycle races, revocation, framing/errors, packaging and accessible grant UI. Every scenario remains OPEN on every OS. These are future evidence requirements, not registered or passing native tests.

## Source accuracy and provenance

The reviewer inspected the claimed prepared-source seams and reran `sha256sum --check arcified/native/production/automation-contract/prepared-source.sha256` from the coordinator worktree. Exit 0, all 12 files OK. Version bytes identify Chromium 155.0.8059.79. Equal base/result hashes identify those files only; they do not prove whole-tree cleanliness, an upstream commit or a Windows/macOS build.

The source supports the packet's specific claims. `launch_context.cc` checks manifest name and allowed origin and supplies the origin argument and Windows parent-window hint. POSIX lookup includes the fork and Chrome fallback locations; the Windows source contains Chromium/Chrome registry lookup and direct/legacy process launch paths. `native_message_process_host.cc` has the existing receive-size bound. Tabs query enumerates tab lists after window-level Profile filtering, so it is correctly treated as a consumer requiring audit after ownership extraction. Cocoa enumeration and tab wrappers exist, and JavaScript execution checks the JavaScript/DevTools permissions before isolated-world execution. Existing send/connect tests are present, including `ConnectNative` at native_messaging_apitest_desktop.cc:144. None was executed.

Primary web sources were checked on 2026-10-24. [Chrome native messaging](https://developer.chrome.com/docs/extensions/develop/concepts/native-messaging) supports the described stdio transport, extension permission, manifest origins and browser-created port. These facts do not establish helper attestation or a per-Profile grant. [Tabs documentation](https://developer.chrome.com/docs/extensions/reference/api/tabs) supports the sensitive-field distinction and browser-session tab identity. [Chrome's remote debugging security explanation](https://developer.chrome.com/blog/remote-debugging-port) supports the risk rationale and Chrome 136 change, without proving fork switch behavior.

[Apple's entitlement documentation](https://developer.apple.com/documentation/bundleresources/entitlements/com.apple.security.automation.apple-events) again required JavaScript; its Markdown link failed. The indexed primary summary supports the limited entitlement and same-team statements. Actual TCC, sandbox, signing and entitlement behavior remains unverified. The Arc Developer Mode URL timed out in this review, so the primary's earlier 404 is retained only as its dated retrieval observation. The [Arc macOS release notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes) again returned no AppleScript match. Neither failure nor absence of a textual match proves lack of an OS automation mechanism.

The artifact dossier corroborates both recorded DMG hashes and the limited `Arc.sdef` presence claim. The packet does not pretend to have inspected that proprietary dictionary's commands or run Arc/Dia. It appropriately avoids treating Dia updater observations as proof of missing product functionality.

## Decision quality and integration limits

The three owner questions identify concrete choices and tradeoffs: approve the narrow design or defer, accept a concrete reviewed helper trust boundary or require a stronger design, and choose connection-lifetime grants or separately design persistence. Their recommendations remain proposals. In particular, connection lifetime is an unresolved authorization choice, not an invented enabled retention policy.

The packet preserves the authoritative Model, metadata/codec, immutable actual Profile targets and private/download classifiers. It requests real ownership integration rather than adding another registry or treating `Model::OpenItem` as native proof. Focus mutations remain excluded and refer to the focus-routing contract. Reserved command 40710 is unused, and Library 40701 is untouched. A native patch and test registration are inapplicable to this unapproved decision leaf; issue 130/root must supply them for an approved adapter.

Remaining gates are owner decisions, concrete extension/helper identity and installation trust, real per-OS packaging and policy behavior, native grant UI, privacy/error/lifecycle evidence, same-window multi-Profile isolation and exact live saved-page handoff. No account, host, runtime, build, benchmark or visual check ran. These explicit limits are not packet defects.

## Review checks

1. Fast-filesystem reads and scoped searches checked the authoritative issue, all packet files, source claims, root authority and artifact provenance. The retained issue object exactly matched the inspected assignment using parsed JSON comparison.
2. `sha256sum` over all six packet files matched the supplied review inputs. Root authority hashes above record the separately inspected control identities.
3. The prepared-source checksum command passed for every listed file. Source inspection stayed read-only.
4. Hosted JavaScript parsed packet JSON, checked canonical two-space JSON plus newline, and checked LF, a single final newline and absence of trailing whitespace in the five new evidence files. All passed. These are text checks, not native tests.
5. `git diff --no-index --check /dev/null arcified/docs/production/research/automation-contract-review-round-1.md` returned exit 1 with no diagnostics. For a new file, this records a difference without whitespace errors. No production formatter or native linter applies to this Markdown-only review.
