# Layout workstream status

Branch: `feat/layout-workstream`. Base: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`.

Local components: 3 reviewed and ready for root integration, 0 in progress, 0 not started. Native acceptance: 0 proven, 3 open.

| Issue | Primary task ID | Owned paths | Stage |
| --- | --- | --- | --- |
| #51 sidebar-layout | `node:delegated-task:command%3Amcp%3A37b7582d-d88d-4f65-bb15-2d75d26e60df%3Adelegate-task%3Alayout-sidebar-layout-primary-v1` | `arcified/native/features/layout/sidebar/`, `arcified/docs/features/sidebar-layout.md` | Reviewed local component |
| #104 tab-layout-choice | `node:delegated-task:command%3Amcp%3A37b7582d-d88d-4f65-bb15-2d75d26e60df%3Adelegate-task%3Alayout-tab-layout-choice-primary-v1` | `arcified/native/features/layout/tab_presentation/`, `arcified/docs/features/tab-layout-choice.md` | Reviewed local component |
| #82 focus-mode | `node:delegated-task:command%3Amcp%3A37b7582d-d88d-4f65-bb15-2d75d26e60df%3Adelegate-task%3Alayout-focus-mode-primary-v1` | `arcified/native/features/layout/focus_mode/`, `arcified/docs/features/focus-mode.md` | Reviewed local component |

All actual primary and reviewer assignments are provider `codex`, model `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`, verified from T3 acceptance and terminal status. All tasks completed without pending child runs. Shared-checkout file ownership stayed disjoint. The coordinator owns this artifact and reviewed commits.

## Accepted interfaces

Sidebar owns side, visibility, resize interaction and geometry. Its presentation override distinguishes normal, suppressed and reveal-as-overlay. Focus mode supplies that override without modifying normal preferences or native focus. Dialog and fullscreen facts control focus/command priority without restoring docked geometry. Tab presentation borrows host-owned ordered identities and selection and consumes caller fit metrics. It does not own workspace state or serialization. Its effective horizontal presentation suppresses the vertical sidebar even during focus-mode reveal; vertical presentation passes the current chrome override through.

The lifecycle model remains the ownership/navigation authority. Native host owns focus, page activation, actual bounds, fullscreen and dialogs. Accessibility consumes host/organizer labeled rows directly and resolves focus intents. Navigation supplies semantic destinations, not layout state. Splits project from current caller bounds. Motion consumes immediate layout output and never delays semantic commits. Appearance supplies colors/materials, not geometry. Durability owns eventual serialization and IO.

## Completed evidence and native blockers

[Independent combined review](layout-review.md) is clean for all nine C++ files and three feature docs. The reviewer matched frozen Git blobs and independently passed all three strict C++17 ASan/UBSan executables and Chromium formatting. Parent verification also passed these checks, production Clang analyzer/bugprone, whitespace and local doc links/anchors. These prove local presentation semantics and composition, not native browser behavior.

Root can integrate the reviewed local components. Sidebar depends on native Spaces. Tab-layout choice additionally depends on window chrome; focus mode depends on sidebar. Same-window multi-Profile production ownership extraction is unproven. Chromium preparation/builds and installed Linux/Windows/macOS visual checks remain deferred. Preference durability/export, native command/menu/accelerator/accessible-exit/focus/AX/capture wiring and actual page/scroll/split/fullscreen/dialog/monitor behavior remain open. Do not close any issue from these unit tests.
