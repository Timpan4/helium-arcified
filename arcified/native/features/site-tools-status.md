# Site-tools workstream

Base: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. Branch: `feat/site-tools-workstream`.

Local components: three implemented and independently reviewed, awaiting root integration. None is released. Native acceptance: none proven; all three issues remain open.

| Issue | Owned paths | Component and stage | Native blocker and next action |
| --- | --- | --- | --- |
| [38, site-controls](https://github.com/Timpan4/helium-arcified/issues/38) | `site_controls/` | Page Info/site-settings/print preparation and revalidation implemented; nine fixtures independently pass normally and under ASan/UBSan; clean independent review | Native selected-page identity, capability and anchor integration; installed interaction checks on Linux, Windows and macOS |
| [39, developer-mode](https://github.com/Timpan4/helium-arcified/issues/39) | `developer_mode/` | DevTools subscription gate with separate event stamps, subscription and binding identities; eight fixtures pass normally and under ASan/UBSan; clean independent review | Native DevTools dispatch, docking and actual observer cancellation; installed platform checks |
| [142, browser-diagnostics](https://github.com/Timpan4/helium-arcified/issues/142) | `browser_diagnostics/` | Task-manager planning, token-bound trace control and metadata-only output; eight fixtures pass normally and under ASan/UBSan; clean independent review | Issue 77 command dispatch, real recorder/artifact handling and installed output/cancellation checks |

All primaries use T3 provider `codex`, model `gpt-6.1-sol`, options `reasoningEffort=high`, `serviceTier=default`. Accepted task IDs:

- Site-controls: `node:delegated-task:command%3Amcp%3Ae1b606aa-1f63-4c26-942a-878e2a3afd9a%3Adelegate-task%3Asite-tools-site-controls-primary-c9748ec8`.
- Developer-mode: `node:delegated-task:command%3Amcp%3Ae1b606aa-1f63-4c26-942a-878e2a3afd9a%3Adelegate-task%3Asite-tools-developer-mode-primary-c9748ec8`.
- Browser-diagnostics: `node:delegated-task:command%3Amcp%3Ae1b606aa-1f63-4c26-942a-878e2a3afd9a%3Adelegate-task%3Asite-tools-browser-diagnostics-primary-c9748ec8`.

## Interface boundary

The lifecycle model remains the page, Profile, attachment and navigation authority. Native services own focus, origin/document identity, capabilities, records and persistence. Session state records only the feature's own operation. No shared focus registry, navigation copy, database or root execution catalog is introduced.

The accepted shared command contract is `arcified::commands::PageCommandTarget{PageId,ProfileId,Attachment}`, `ResolvePageCommandTarget(Model,focused_page,expected_attachment)` and `RevalidatePageCommandTarget`. Shared commands supply the sole lifecycle target validation at actuation. Site-tools retains feature-specific document, anchor, capability and session checks; the native executor verifies TabModel/WebContents binding and generation. The header and implementation were verified read-only in the commands worktree at `arcified/native/features/commands/command_target.{h,cc}`. Site-controls and developer-mode use that dependency without copying it; root must integrate commands first. Local checks can compile its source and add the commands checkout to include paths. Browser-wide diagnostics needs no page target.

Library coordination agrees that existing native services provide scoped records. Capture/media/created-item providers remain unavailable without a concrete native provider. Profiles and accessibility coordinators have been sent the same boundary for conflict checks.

The diagnostics dispatch proposal passes explicit `WindowId` and host capabilities to the planner; the native host executes task-manager/trace actions and reports completion. It requires no shared edits here. Browser-wide task-manager scope must remain visible. Metadata-only synthetic output is separate from raw native trace export.

Same-window multi-Profile production ownership remains unproven. Host extraction, installed native checks and heavy build preparation are outside this workstream's authorized checks. Activity protection, cleanup, destructive actions and populated cross-Profile migration policy remain open.

## Review and verification

Both independent T3 reviews completed cleanly with `codex/gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`. Review task suffixes are `site-tools-site-controls-review-round1` and `site-tools-sessions-review-round1`, under the same delegated-task prefix as the primary IDs above. All primary and review tasks are terminal with no pending child runs.

Reviewers checked every implementation, test and README, reran strict C++17 compilation and all 25 behavioral fixtures normally and under ASan/UBSan, and passed Chromium formatting for all nine C++ files. They checked the documented native claims against pinned Chromium source. The coordinator confirmed all twelve reviewed Git blob hashes unchanged. Exact focused commands and source links are in each feature README. These are local synthetic C++ checks on Linux, not native browser builds or installed cross-platform tests.

DevTools requires synchronous host invalidation before navigation, binding changes and hide/close. Supplying stale facts while omitting invalidation cannot be detected by the gate. Native binding generation and per-subscription identity serve separate purposes. Localhost opt-in, JSON readability and portrait capture remain deferred.

Tracing endpoint finality can follow `OnTracingFailed` through `CompleteFlush`; it cannot prove success. The host must separately confirm successful stop and raw-data disposal. Metadata-only summary output does not redact or export native traces. Native trace arbitration and recorder/artifact handling remain unimplemented.

## Next action

Root integrates commands before site-tools, then runs the combined-tree focused commands recorded in the READMEs. After separately authorized native ownership/dispatch work, verify the affected interactions in installed Linux, Windows and macOS builds. Keep issues 38, 39 and 142 open until their full acceptance evidence exists.
