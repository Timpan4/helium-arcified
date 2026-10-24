<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Verification record

Recorded: 2026-10-24. Issue 25 is a decision leaf. Native acceptance: OPEN.

## Checks actually run

The working directory was `/home/timpan4/.t3/worktrees/helium-arcified/feat-non-ai-research-production`.

1. `git status --short`, `git branch --show-current`, and `git rev-parse HEAD` confirmed branch `feat/non-ai-research-production`, baseline `a4b6b480d3de07857ab6e939149254a8359e5449`, and pre-existing untracked production directories. No stage or commit operation ran.
2. Exact prepared files were inspected with fast_filesystem. `sha256sum` over the paths in [prepared-source.sha256](prepared-source.sha256) exited 0. `sha256sum --check arcified/native/production/automation-contract/prepared-source.sha256` exited 0, with all 12 files reporting OK. This identifies the inspected bytes, not a complete source-tree baseline.
3. `git -C /home/timpan4/.t3/builds/arcified-linux-155-native-v1/src rev-parse HEAD` exited 128 with "not a git repository". [sources.json](sources.json) records this limitation rather than assigning a guessed upstream commit.
4. The source-identity semantic check below exited 0 and returned true.

```sh
jq -e '(.issueKey == "automation-contract") and (.authoritativeIssue.number == 25) and (.authoritativeIssue.primaryCommandId == 40710) and (.preparedSource.files | all(.[]; .baseSha256 == .resultSha256 and .status == "read_only_unmodified"))' arcified/native/production/automation-contract/sources.json
```

5. JSON was formatted with two-space indentation using `JSON.stringify(value, null, 2)` before apply_patch wrote it. Markdown uses LF lines and no trailing whitespace. No standalone Prettier or markdownlint executable was available, and no package was installed. Each newly authored file is checked with `git diff --no-index --check /dev/null <packet-file>`. For an added file, exit 1 with empty diagnostics means differences exist without whitespace errors. An initial extra blank line at decision.md EOF was reported and removed.
6. Final JSON syntax/required-field, packet link, formatting and whitespace checks are described in result.json after execution. These are document/identity checks only. ownership.json was pre-existing and left unchanged.

No formatter/linter for production code is applicable because this packet changes no production code. No native preparation, compilation, benchmark, browser execution, visual check, account operation or remote write ran.

## Native scenarios still required

Every cell below is OPEN on Linux, Windows and macOS. These are required evidence requests for the approved adapter, not registered or executed tests. Unsupported adapters must stay unavailable and show an explicit disposition.

| Scenario | Required evidence | Linux | Windows | macOS |
| --- | --- | --- | --- | --- |
| Enabled first command | Installed identified build/helper accepts only tabs.list and returns exact eligible handles. Record source, package, helper and extension identities. | OPEN | OPEN | OPEN |
| Default denial and explicit grant | No data before trusted browser UI selects a real helper, Profile and window. Decline preserves denial. | OPEN | OPEN | OPEN |
| Wrong caller or helper | Unlisted extension, missing/replaced manifest, unexpected resolved helper, denied OS/policy permission and forged invocation cannot gain browser data. Record same-user threat limits honestly. | OPEN | OPEN | OPEN |
| Profile isolation | Distinct synthetic Profiles, including two Spaces sharing a Profile, expose only the granted native context. Display labels and matching URLs cannot select a Profile. | OPEN | OPEN | OPEN |
| Same native window, multiple Profiles | Selecting another Profile in the same actual window never broadens an existing grant. Prove real BrowserContext identity and adapter enumeration after the ownership refactor. | OPEN | OPEN | OPEN |
| Live saved-page handoff | Moving the existing saved page preserves live identity/navigation and immutable Profile; stale/out-of-window inventory does not leak it. No duplicate, reopen or temporary-tab transfer. | OPEN | OPEN | OPEN |
| Private and guest exclusion | No private/guest handles, metadata, count or existence leak, including incognito extension settings and unknown classifier results. Reuse the authoritative classifier. | OPEN | OPEN | OPEN |
| Data minimization | Response contains no URL/title/favicon/origin, page content, saved destinations, download state, cookies or paths. Check ordinary and internal schemes without reading sensitive content. | OPEN | OPEN | OPEN |
| Forbidden commands | Unknown command, extra fields, arbitrary API names, numeric command IDs, JavaScript, content capture and mutation requests fail before actuation. Verify no focus/navigation/close side effect. | OPEN | OPEN | OPEN |
| Stale and destroyed objects | Window/Profile/tab destruction and scope changes during request produce structured refusal or a correctly scoped lifecycle-consistent result. No freed-object access or fallback. | OPEN | OPEN | OPEN |
| Revoke and disconnect | Revocation during request prevents delivery; port closure invalidates handles; browser restart does not reuse an old grant or session handle. | OPEN | OPEN | OPEN |
| Framing and errors | Existing native-messaging framing/bounds, malformed message, helper crash and denied registration produce safe structured failure without private diagnostics. No invented timeout/retry policy. | OPEN | OPEN | OPEN |
| Packaging | Verify real branding, user/machine registration, policy, extension permission and resolved executable. Explicitly test observed Chrome fallback search routes without mutating unrelated installations. | OPEN | OPEN | OPEN |
| Trusted grant UI | Installed native UI supports keyboard and assistive access to scope, denial and revocation. Capture authorized visual evidence separately. | OPEN | OPEN | OPEN |

No page-JavaScript, private-content or mutation success scenario is authorized by the proposed first command. If the owner expands the scope, add that capability's independent contract and native scenarios before enablement.

## Registration and integration status

This decision changes no patch or GN target. Existing native-messaging tests are source leads only. Issue 130/root must add meaningful native coverage to the actual adapter's browser-test registration after scope is approved; echo-host success cannot prove the tab inventory privacy contract.

The reserved command ID 40710 is unused in this packet. No ActionItem, message ID, command target, lifecycle source, metadata codec, private/download classifier, patch series or shared ledger changed. Same-window multi-Profile and live saved-page handoff remain production gates. No new host/service/store or Model::OpenItem result substitutes for them.
