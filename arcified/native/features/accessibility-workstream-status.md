# Accessibility workstream status

Base: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. Branch: `feat/accessibility-workstream`.

Three local components are implemented, tested and independently reviewed, ready for root integration. None has native acceptance. No implementation or review task remains pending. Each primary is T3 provider `codex`, model `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`, verified on assignment and completion.

| Issue | Owned directory | T3 task ID | Stage |
| --- | --- | --- | --- |
| #149 accessibility | `accessibility/` | `node:delegated-task:command%3Amcp%3A70b6c1d7-692b-4195-bb48-e2459f9941cc%3Adelegate-task%3Aaccessibility-stream-accessibility-primary-v1` | Reviewed local component |
| #113 ime-localization | `input/` | `node:delegated-task:command%3Amcp%3A70b6c1d7-692b-4195-bb48-e2459f9941cc%3Adelegate-task%3Aaccessibility-stream-ime-localization-primary-v1` | Reviewed local component |
| #35 window-chrome | `window_chrome/` | `node:delegated-task:command%3Amcp%3A70b6c1d7-692b-4195-bb48-e2459f9941cc%3Adelegate-task%3Aaccessibility-stream-window-chrome-primary-v1` | Reviewed local component |

Review task: `node:delegated-task:command%3Amcp%3A70b6c1d7-692b-4195-bb48-e2459f9941cc%3Adelegate-task%3Aaccessibility-workstream-independent-review-v1`, verified GPT-6.1 High/default, completed clean with no actionable findings. The reviewer read all 12 component files and reran all three strict C++17 fixture executables, Chromium formatting on all nine C++ files and whitespace checks on all 12 additions. All primaries also passed ASan/UBSan. The component files remained unchanged through review. Their READMEs record exact commands, source evidence and coverage limits.

## Shared boundaries

- Accessibility borrows a complete visible sidebar preorder, caller focus/liveness facts and advertised labeled commands. It returns semantics, intents and focus-repair suggestions. The organizer/native host supplies the authoritative labeled preorder and owns focus application, restoration and announcements. Layout supplies visibility and current geometry, without another tree model or focus snapshot.
- Input borrows native post-IME event facts and UTF-16 text. Unknown, active or stale text-system ownership prevents command routing. Native text controls own composition, candidates, editing, bidi and selection. Linux/mac candidate-popup false values do not establish inactivity.
- Window chrome borrows current post-layout window-local DIP geometry and native eligibility/exclusions. It classifies caption regions without owning window lifetime, persistence, scale conversion, focus or native frame behavior.
- The commands area's `PageCommandTarget`, `ResolvePageCommandTarget` and `RevalidatePageCommandTarget` remain the sole shared lifecycle page-target resolver. Native actuation separately checks actual binding and operation generation.

## Blockers and next actions

Root integrates this reviewed branch, then coordinates production adapters at the accepted seams. No component integrates the native host yet. #149 depends on folders/sidebar layout; #113 on command bar and positive native input nonownership evidence; #35 on installed Linux, Windows and macOS builds. Full preparation/builds remain deferred. Platform accessibility, IME, window-management, mixed-scale and dialog/focus behavior remain open. All three issues stay open. No remote publication is authorized.
