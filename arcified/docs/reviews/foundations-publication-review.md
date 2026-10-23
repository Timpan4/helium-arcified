# Foundations publication review

Result: no new blocking aggregate defect found in the frozen foundations candidate. The committed sources and retained independent reviews support publication as a foundation PR in `Timpan4/helium-arcified`, subject to the root's actual PR checks. This is a bounded source review, not certification of remote checks, a distributable browser, native feature completion or good installed UX. Two nonblocking evidence-document corrections are recorded below.

Reviewed independently on 2026-10-23 in `/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261`.

| Boundary | Exact identity |
| --- | --- |
| Candidate | `05ace22efb8564d0294b8efa1b3f3651b76cef07` |
| Local `origin/main` | `09670c97b1c308d0fb31869b3a9a9bc6969bc47f` |
| Merge base | Same as local `origin/main` above |

No fetch or remote state inspection occurred. Historical contents came from Git objects, rather than the changing checkout. Later metadata schema 2, game-resume state, the unaccepted native destruction draft and its additional source packet were excluded. Only this report was written in the repository. No installation, Chromium preparation/build, native browser interaction, remote write, commit, PR linking or watching occurred.

## Nonblocking findings

1. **P3, replace the obsolete current-admission description.** [parallel-integration.md](../parallel-integration.md), line 25, says the current shared interface cannot inspect snapshot attachments and prepared commit has no admission hook. That describes the earlier adapter, not this candidate. The committed `session_snapshot.h`, lines 66 and 150 to 152, requires whole-snapshot admission and a commit admission argument. `session_snapshot.cc`, lines 471 to 480, decodes and admits the candidate before promotion. The final admission reviews and `execution-state.json`, line 109, accurately describe this fix. Update the current integration paragraph to identify the cleared pure API gaps and link the final reviews, retaining the native liveness, serialization and IO limits. The same paragraph still calls the completed layout and inventory reviews active. This is contradictory status documentation, not a newly reproduced persistence defect.
2. **P3, distinguish excluded packet evidence from committed evidence.** [execution-state.json](../execution-state.json), line 227, records a completed source packet at `research/native-space-consumers/lifecycle-tests/README.md`, with 21 sources and three listings. That path and packet are absent from the candidate's Git tree. Line 77 also describes a completed test packet as an input to the next action. Mark this evidence as external to the frozen foundation, or add it in a separately reviewed later change. Do not count it as inspectable committed evidence for this PR. Its absence does not invalidate the original captured-source manifest or prove a native defect. No native destruction implementation is accepted by this review.

These corrections do not require repeating the component implementation reviews. Earlier workstream handoffs that say "awaiting root integration" are historical area records; the final machine-readable integration manifest supplies their aggregate disposition.

## Aggregate correctness and retained reviews

Read the committed execution state, execution narrative, parallel manifest and integration evidence; the area workstream/review records; final lifecycle, platform-tooling, native-check, admission, sidebar, layout and source-inventory closure reviews; and the relevant check scripts and workflows. Reused unchanged component evidence instead of treating it as a new implementation audit or execution result.

The manifest contains 17 areas, 51 distinct item keys and 51 corresponding primary assignments. Key-to-assignment sequences agree, every listed integration commit is an ancestor of the candidate, and every assignment retains native acceptance `OPEN`. The component accounting agrees with the execution state: 51 reviewed local components integrated, zero remaining local components to integrate, zero native issues accepted. The broader non-AI counts remain zero proven done, one locally done awaiting publication, 56 in progress and 78 not started.

An independent Git-object comparison collected native `.cc`, `.h`, `.patch`, `.mjs` and `.sh` paths changed by the manifest's source commits. Of those 170 artifacts, 166 match their recorded area source bytes. The four differences are the documented admission followup in `session_snapshot.h/.cc`, its test and `schema_gate_test.cc`. They are covered by the subsequent admission reviews. No unexplained area-source divergence was found. This comparison establishes identity for those artifacts, not behavior of every API combination.

Twenty-eight path/hash entries from the final admission, native-check and sidebar review tables match the frozen tree. The final admission review includes the changed production files, caller migration and test-only analyzer closure. The sidebar and layout reviews explicitly accept conformance scenarios rather than production adapters. The parent integration record separately retains combined commands/site-tools and capture/region sanitizer executions. Those executions were not rerun or relabeled as this review's results.

The model remains the semantic authority. Native observation, object retention, callback freshness, authoritative ordering/localization, serialized use and actual dispatch remain host duties. Schema 1 deliberately refuses unsupported peer payloads rather than claiming to persist Space metadata, folders, themes or split presentation. Its fail-closed limitation prevents accepting it as a complete browser session store. The mandatory admission followup does not supply a native producer or real filesystem durability.

## Attribution and distribution context

The original source manifest accounts for 75 production source files, 12 shallow listings and one tag record at Chromium `155.0.8059.79`, commit `a49e2e3d4addfd84e765b52afe230bedb81096db`. Independently checked every retained response's byte length and SHA-256, its adjacent retrieval metadata and each recorded source Git blob identity. All matched. Every one of the 75 source files retains its upstream copyright and BSD-style-license header. No extra file under the original `chromium/` capture directory escaped that accounting.

The supplemental [Chromium license](../research/native-space-consumers/licenses/chromium-LICENSE) retains the source/binary redistribution conditions, disclaimer and non-endorsement condition. Its byte length, SHA-256 and Git blob match its provenance receipt. The [capture README](../research/native-space-consumers/README.md) explicitly locates this license and explains that it is outside the original manifest count. The root licenses and existing Chromium patch series are unchanged. No attribution or missing-notice defect was found in this source-publication context.

This checks retained notices and internal capture consistency. It does not independently authenticate every response against a new upstream retrieval, repeat complete consumer analysis or audit a packaged binary's notices. The source records are research captures, not an applied Chromium tree. A future binary distribution requires its own packaging evidence. Keep the captures and supplemental license together when publishing this foundation.

## Checks performed in this review

| Check | Result and actual coverage |
| --- | --- |
| `git merge-base origin/main 05ace22` | Returns the base identity above. No merge conflict or remote-check state is inferred. |
| Manifest and Git-object checks | Assignment/key/ancestry checks, the source comparison and 28 final-review hash entries passed. Source-capture and license checks passed as described above. |
| `env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ python3 -B arcified/native/checks/python_checks.py test` | Exit 0 on Linux, Python 3.14.8. All 41 discovered tooling tests passed without skips, followed by both devutils function checks. Expected negative fixtures emitted diagnostics. This tests preparation contracts, not preparation of Chromium. |
| `env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ python3 -B devutils/lint.py` | Exit 0. All seven existing patch-series sanity checks passed. |
| Committed script syntax | All seven changed `.mjs` files passed Node module syntax checking; all four changed `.sh` files passed `bash -n`. This does not execute their feature tests or network retrieval. |
| Scoped `git diff --check origin/main 05ace22` | Passed for `.github`, native code/tooling, devutils and docs, excluding unchanged upstream formatting retained in captured Chromium source and the shortcut patch's required blank context prefixes. It is not a blanket raw-capture/patch whitespace claim. |
| Local documentation targets | No missing relative file target found in the scanned committed Markdown after recognizing directory links. The excluded packet reference in finding 2 is a JSON evidence string and remains absent. |

The Python and patch-lint runs used working files. A subsequent `git diff --name-only 05ace22 --` over their runners, tooling, affected devutils files and patch inputs was empty, establishing that these inputs match the frozen candidate despite unrelated concurrent work. No broad C++ suite or existing component sanitizer suite was rerun. Final C++ formatting/analyzer evidence is reused only where the retained reviewed identities match. No Markdown formatter or linter is declared for this report path; no tool was installed.

## Required-check gaps and release safety

The new [native-check workflow](../../../.github/workflows/arcified-native-checks.yml) remains manual-only, with read-only contents permission and checkout credential persistence disabled. Its C++ compile, sanitizer and formatting targets contain only `lifecycle.cc`, `lifecycle.h` and `lifecycle_test.cc`. It does not automate the 51 feature suites or the new composition suites. Passing it would leave those checks dependent on their retained focused evidence. It does not establish native integration or become an automatic PR gate merely by being added.

The selected [checkout release](https://github.com/actions/checkout/releases/tag/v7.0.1), [setup-python release](https://github.com/actions/setup-python/releases/tag/v7.1.0) and all selected labels in the [official runner catalog](https://github.com/actions/runner-images#available-images) were checked read-only. Their existence does not prove successful hosted execution. Actions schema/expression validation and PowerShell parsing remain unverified here because actionlint and PowerShell were unavailable. Hosted Linux, Windows/MSVC and macOS execution, Python 3.10 and the exact pinned lint environment remain unexecuted by this review. The documented skip/privilege and Intel-only Mac coverage limits remain material.

Existing automatic code-validation, source-validation, patch-lint and extension workflows are unchanged. The existing source-validation job retrieves/prepares Chromium on pushes and PRs; the new manual workflow neither introduces nor suppresses that heavy job. Root owns its actual CI result and any required-check or branch-protection decision. This review did not inspect remote protection settings or certify a green PR. The default combined Pylint failure recorded by earlier tooling reviews remains distinct from the passing scoped policy; no new remote failure or waiver is asserted here.

No production patch or `patches/series` entry changed. The existing release workflow and version inputs are unchanged, so this candidate introduces no new browser-release trigger or version bump. The real history query snippet remains unwired and uncompiled. The shortcut patch remains outside the production series, unapplied to a prepared browser and without compiled/GN-registered native gtests. Foundation files therefore supply no evidence that either path works in the browser.

The inherited Windows path-separator condition was addressed for the newly added tooling route and independently cleared; it is not an outstanding finding. Existing CI/source preparation costs and older lint/import limitations are likewise distinct from new aggregate regressions. Actual same-window multi-Profile hosting, complete consumer closure, authentication isolation, exact live-page transfer/recovery, persistence, installed platform behavior, accessibility, visual interaction and performance remain open integration requirements, not proven upstream defects.

The owner's requested outcome of all non-AI issues merged and working with good UX remains unfinished. This candidate can advance the reviewed foundations only. Do not use its local component counts, synthetic tests, captured source or this review to close feature/native issues or describe an installed browser as complete.
