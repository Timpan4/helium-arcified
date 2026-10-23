# Commands workstream status

Checkpoint: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. Branch: `feat/commands-workstream`.

Local components: 3 proven by focused synthetic checks and independently reviewed, awaiting root integration, 0 in progress, 0 not started. Native acceptance: 0 accepted, all 3 open.

| Issue | Task ID | Stage | Exclusive files in this directory |
| --- | --- | --- | --- |
| #77 command-bar | `node:delegated-task:command%3Amcp%3Ac8e2a2c9-e40f-4ec0-b89d-3a61f8f8dcac%3Adelegate-task%3Acommands-primary-command-bar-v1` | Local component complete, native open | `command_bar.*`, `command-bar.md` |
| #109 command-ranking | `node:delegated-task:command%3Amcp%3Ac8e2a2c9-e40f-4ec0-b89d-3a61f8f8dcac%3Adelegate-task%3Acommands-primary-command-ranking-v1` | Local component complete, native open | `command_ranking.*`, `command-ranking.md` |
| #110 site-search | `node:delegated-task:command%3Amcp%3Ac8e2a2c9-e40f-4ec0-b89d-3a61f8f8dcac%3Adelegate-task%3Acommands-primary-site-search-v1` | Local component complete, native open | `site_search.*`, `site-search.md` |

All assignments verified: provider `codex`, model `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`. Shared checkout; coordinator owns commits, review report, this status, `command_target.*`, `workspace_results.*` and README.

Accepted adjacent contract: commands owns read-only page-target resolution from explicit focused PageId and expected Attachment. Navigation and transients accepted. Model remains sole ownership/navigation authority; native host owns actual focus and native binding/generation validation. Bar uses opaque caller surface and request tokens, and consumes semantic key actions after input #113 arbitration. Ranking projects one explicitly nonprivate Profile per call. Site search consumes native-authoritative service snapshots and emits provider/query intent without URL expansion or duplicate CRUD/persistence.

Blockers: temporary-tab/native host ownership extraction, native adapters, native URL/security/IME/focus/accessibility behavior and installed Linux/Windows/macOS checks remain unproven. Same-window multi-Profile production feasibility is open. Full Chromium preparation/builds, benchmarks, remote writes and real browsing data are deferred. Unit fixtures cannot close issues.

Evidence: all three primaries report strict C++17 synthetic and ASan/UBSan checks and Chromium formatting passed. Coordinator's page-target and ranking-to-bar integration fixtures also pass strict C++17 and ASan/UBSan. Full-area Chromium format, core/C++ analyzer plus use-after-move lint and staged whitespace checks pass. Workspace results preserve existing saved identities and change presentation identity when the action or target changes. This is local component evidence only.

Independent review completed with no actionable findings: `node:delegated-task:command%3Amcp%3Ac8e2a2c9-e40f-4ec0-b89d-3a61f8f8dcac%3Adelegate-task%3Acommands-independent-review-round-1`, verified `codex` / `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`. See [review evidence](review-round-1.md). No source fix or pending child work remains.

Commits: shared target is `a2cd18c07807399874b9c38702c36671b9ae8326`; the feature components, adapter and final records are in the commit containing this status. All changed paths are in this directory. No lifecycle/tooling/patch/root edits or remote writes occurred.

Next owner action: root integrates the local commits and shared resolver dependency. Future native adapter/host work must satisfy the deferred source/build and platform gates; unit evidence does not close any issue. Open product decisions remain unchanged.
