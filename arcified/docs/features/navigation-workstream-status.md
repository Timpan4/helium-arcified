# Navigation workstream

Checkpoint: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. Branch: `feat/navigation-workstream`.

## Current state

All three local components are independently reviewed, committed and awaiting root integration. None remain in progress or not started; none are integrated/released here. Native acceptance remains open for all three issues. All three primary tasks and both review tasks completed without pending children.

| Issue | Owned paths | Stage | T3 task ID |
| --- | --- | --- | --- |
| #78 tab-switcher | `native/features/recent_switcher/`, `docs/features/tab-switcher.md` | Reviewed; commit `6b5e16de853898c8957ef9099ff30df00492fed6` | `node:delegated-task:command%3Amcp%3A0fd6fee9-5dcb-4aaf-b98a-f8730c060fd2%3Adelegate-task%3Anavigation-tab-switcher-primary-v1` |
| #100 tab-order-navigation | `native/features/ordered_navigation/`, `docs/features/tab-order-navigation.md` | Reviewed; commit `cb96921948acdd0ba5920918d4be212528274b05` | `node:delegated-task:command%3Amcp%3A0fd6fee9-5dcb-4aaf-b98a-f8730c060fd2%3Adelegate-task%3Anavigation-tab-order-primary-v1` |
| #33 find | `native/features/local_find/`, `docs/features/find.md` | Reviewed; commit `5f154076e5f14cffa220602bbfe115b6040e1788` | `node:delegated-task:command%3Amcp%3A0fd6fee9-5dcb-4aaf-b98a-f8730c060fd2%3Adelegate-task%3Anavigation-find-primary-v1` |

Each assignment was verified as provider `codex`, model `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`. Children shared this isolated area checkout with disjoint file ownership. The coordinator committed only unchanged, independently reviewed files.

Independent review task for local find and ordered navigation: `node:delegated-task:command%3Amcp%3A0fd6fee9-5dcb-4aaf-b98a-f8730c060fd2%3Adelegate-task%3Anavigation-find-order-review-round1`, verified with the same provider/model/options, completed clean with no pending children. All eight reviewed Git blobs matched immediately before commits. Both primaries passed warning-clean C++17, ASan/UBSan and Chromium formatter checks on synthetic fixtures. Coordinator clang-tidy `--checks='clang-analyzer-*' --warnings-as-errors='*'` passed over all three components' implementation and test files with C++17 strict warning flags. Cached whitespace checks passed. The host's clang-tidy default has no enabled checks, so explicit analyzer checks were used. These checks prove component behavior and source quality, not native acceptance.

Switcher independent review task: `node:delegated-task:command%3Amcp%3A0fd6fee9-5dcb-4aaf-b98a-f8730c060fd2%3Adelegate-task%3Anavigation-switcher-review-round1`, verified `codex`, `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`, completed clean. All four reviewed blobs matched before commit. Its primary completed strict Clang C++17, 13 synthetic fixture functions, ASan/UBSan, formatter and new-file whitespace checks. No native runtime result is claimed. Exact component check commands and coverage are in the linked feature documents.

## Interfaces

Lifecycle v3 remains the semantic ownership authority. Native focus, current navigation and page registry remain host authorities. Features consume identities and authoritative snapshots; they return decisions, not native completion acknowledgments. Layout owns presentation only. No other worktree is edited.

Accepted feature-local contracts: the recent switcher freezes eligible identities and explicit requested Space for one hold from caller MRU, then checks a caller context guard. Kept items resolve their owning Space; Profile-owned saved items retain explicit caller destination; temporaries require original attachment. Ordered navigation selects from a caller revisioned sidebar projection and returns identity/reveal metadata. Local find maps intents and sampled native binding/IME state to pinned Chromium controller operations without storing query/results.

Accepted shared contract, owned by commands at `native/features/commands/command_target.h/.cc`: `PageCommandTarget{PageId page, ProfileId profile, Attachment attachment}`, `ResolvePageCommandTarget(Model, focused_page, expected_attachment)`, and `RevalidatePageCommandTarget(Model, target)`. This reads lifecycle state only. Caller owns authoritative focus; executor checks native TabModel/WebContents binding and operation generation immediately before effects. It decides no modifier policy or transfer. Feature plans remain local intent metadata and consume this resolver at actuation after root integration. Root, all primaries, tabs, accessibility and layout were informed. No competing resolver, focus owner or header copy is introduced here.

Organization owns Space folder hierarchy/order metadata referencing lifecycle IDs. Ordered navigation consumes that caller projection and ancestry, returning reveal hints without changing expansion. Labels come from the tabs seam. Folder preview and multi-select do not own page selection, focus or navigation.

Input #113 owns post-native-IME arbitration. Local find consumes positive caller eligibility; unknown, active or stale input ownership defers native. Linux/macOS false candidate-popup reports cannot prove inactivity. This seam was forwarded to command bar through its coordinator.

## Blockers and next actions

Temporary-tab native integration blocks #78 and #100 acceptance. Lifecycle v3 fresh review was pending at this workstream's checkpoint and remains a root-owned gate. All three issues require native interaction and visual evidence on Linux, Windows and macOS. Same-window multi-Profile production ownership remains unproven. Full native preparation/builds and heavy downloads are deferred. No issue can close from standalone component tests.

Next action belongs to root: integrate the three reviewed component commits and this status artifact, then assemble organization/layout/tabs/input/commands adapters without copying state owners. Production host ownership extraction, native execution/recovery, focus/accessibility publication and installed platform evidence remain separate work. No issue was closed. No pushes, PRs, remote issue changes or merges were performed.
