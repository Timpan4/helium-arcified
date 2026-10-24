# Focused checks

Scope: decision artifacts and light source identity/syntax checks only. Native acceptance OPEN.

Completed before writing: fast_filesystem read the assigned full issue object, root manifest, lifecycle contract, native integration design and execution state. It inspected prepared and existing foundation source. Prepared source/version and baseline were confirmed read-only. The nonexistent `docs/features/commands.md` lookup returned an actionable missing-file error; the actual command_target header, implementation and test supplied the foundation evidence instead.

External reference documents were read through web tools. The guessed YAB `/changelog/0-36` path was inaccessible. Following the official changelog link resolved `/changelog/0.36`. No reference app, account or UI ran.

From repository root:

```sh
git rev-parse HEAD
node --check arcified/native/production/focus-routing-contract/verify-packet.mjs
node arcified/native/production/focus-routing-contract/verify-packet.mjs
sha256sum --check arcified/native/production/focus-routing-contract/identities.sha256
```

Observed results:

| Check | Outcome and proven scope |
| --- | --- |
| Baseline | Exit 0. HEAD equals the assigned baseline. |
| Node syntax | Exit 0, no diagnostics. Packet auditor parses. |
| Packet audit | Exit 0. 31 synthetic rows cover all nine command families and required entry/exit, stale and unsupported states. All native platform results remain OPEN. |
| Source identity | Exit 0. All 20 listed hashes matched, including the 9 prepared-source/version files. |

The packet audit was rerun after recording these results so the final JSON and check record were included.

JSON uses canonical two-space formatting with a final LF. The packet audit checks canonical JSON, fixture/table coverage and targets, unresolved question references, all per-platform OPEN states, evidence paths, LF endings, final newlines and trailing whitespace. It does not run the specified routes. Markdown was formatted as plain repository-style prose and tables. No Markdown formatter/linter is declared by the inspected root/native evidence or available on PATH. `command -v node bun prettier markdownlint-cli2` found node and bun only, exit 1 because the formatter/linter were absent. No dependencies were installed.

The hash command records exact bytes of all listed authority, foundation and prepared-source inputs. It does not assert whole-tree cleanliness or binary/build provenance. Authority files may change under root ownership after this evidence snapshot. Reconcile relevant drift before implementation.

No prepared or production native source writes, native setup/build/test execution, pure-component reruns, benchmark, remote CI, browser/visual run, staging or commits occurred. No production patch means sparse-apply and native syntax/test registration checks are inapplicable. Native Linux/Windows/macOS behavior, origin/credential targeting, accessibility, modal/fullscreen/close interactions, same-window multi-Profile isolation and live saved-page handoff remain unverified.

## Compatibility Q2 consolidation

Read compatibility result.json/Q2 and defects.json/D1 through fast_filesystem. Sibling packet remained unchanged. Added Q5/Q6, external-event target constraints and eight synthetic route variants. D1 remains SOURCE_RISK_NOT_RUNTIME_REPRODUCED; native source was not independently re-inspected for this follow-up.

Additional command from repository root:

```sh
sha256sum --check arcified/native/production/focus-routing-contract/compatibility-inputs.sha256
```

Follow-up results:

| Check | Outcome and proven scope |
| --- | --- |
| Node syntax | Exit 0, no diagnostics after consolidation. |
| Expanded packet audit | Exit 0. 31 focus rows and 8 external-route rows have consistent targets/question references, canonical JSON and OPEN platform states. This is artifact validation only. |
| Compatibility input identity | Exit 0. Both compatibility input hashes matched their recorded snapshot. No compatibility file was changed by this leaf. |

Earlier 20-source hash evidence remains historical and unchanged; this follow-up changes only packet decision artifacts. The expanded audit was rerun on the final recorded result and check artifacts.
