# Appearance workstream status

Baseline: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. Branch: `feat/appearance-workstream`.

Local components proven complete: 3. Reviewed components awaiting root integration: 3. In progress: 0. Not started: 0. Native issue acceptance: open for all three. All primary and independent review tasks completed without pending child runs.

All primary assignments were accepted through T3 and verified as provider `codex`, model `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`.

| Issue | Task ID | Owned paths | Stage |
| --- | --- | --- | --- |
| [Space themes #52](https://github.com/Timpan4/helium-arcified/issues/52) | `node:delegated-task:command%3Amcp%3A61ace16c-e235-4508-9b0c-b56fadb689cd%3Adelegate-task%3Aappearance-space-themes-primary-v1` | `native/features/space_themes/`, `docs/features/space-themes.md` | Clean independent review, local commit `827256606ebe4b087188f1687c354c711bc1ea02` |
| [Motion #83](https://github.com/Timpan4/helium-arcified/issues/83) | `node:delegated-task:command%3Amcp%3A61ace16c-e235-4508-9b0c-b56fadb689cd%3Adelegate-task%3Aappearance-motion-primary-v1` | `native/features/motion/`, `docs/features/motion.md` | Clean independent review, local commit `e2057bd12cddbf51a3341db02a087bc1831fbba3` |
| [Shortcut settings #34](https://github.com/Timpan4/helium-arcified/issues/34) | `node:delegated-task:command%3Amcp%3A61ace16c-e235-4508-9b0c-b56fadb689cd%3Adelegate-task%3Aappearance-shortcut-settings-primary-v1` | `native/features/shortcut_settings/`, `docs/features/shortcut-settings.md` | Independent review finding corrected, re-review clear, local commit `cc43738b011a25935f4aafab10eea8f88b0f56ec` |

Paths are relative to `arcified/`. Children share this checkout but own disjoint paths. The coordinator owns this status artifact, independent review artifacts and local commits. No child commits or shared lifecycle, tooling, patch-series, workflow, instruction or root-catalog changes are authorized.

## Interfaces

Layout owns layout geometry, visibility/reveal/resize, tab presentation and focus integration. The accepted seam commits layout visibility/preferences immediately. Its output describes hidden, pinned or temporary-overlay presentation with caller-derived effective width; focus mode projects suppression/reveal over current preferences. Motion owns only normalized scalar presentation progress, starting reversal from the last accepted displayed sample. Externally supplied phases/timing drive samples. Per-run tokens reject superseded callbacks. Reduced motion settles the target and invalidates the active run. Cancellation invalidates motion; the caller snaps to a freshly queried authoritative layout projection. Motion completion never commits layout or Space state.

Themes remain Space-scoped and consume lifecycle identities without creating a Space registry. The accepted component resolves palettes and owns a caller-created draft preview; revert reads current durable theme from its caller. Names, icons, persistence and native material integration remain outside this component. Shortcut work adds an owned, unapplied patch for read-only assignment/reset conflict previews against Helium's existing Profile-keyed service using native accelerator equality. That service remains the binding, persistence and dispatch authority. Accessibility owns focus targets/routing. The parent and adjacent coordinators were informed of these accepted scopes.

No new owner of persistence, page identity, navigation, focus, caption or display metrics is introduced. Forced-contrast colors and reduced-motion settings come from authoritative native inputs.

The accepted window-chrome boundary classifies captions from supplied current window-local DIP geometry and native eligibility/interactive exclusions. The renderer supplies mirrored/clipped post-layout geometry. Appearance does not classify caption hits or own geometry. Accessibility returns semantics/intents; the native host applies focus and announcements.

Layout accepted the concrete `motion/presentation_transition.h` API without a dependency between modules. It plans composition tests against synchronous layout projection and caller-driven cancellation/reset. Motion passed clean independent review, including seven standalone C++17 sanitizer fixtures, Chromium formatting, explicit clang-tidy analyzer/bugprone and whitespace checks. See [review evidence](reviews/appearance-motion-review.md).

Themes passed clean independent review; see [review evidence](reviews/appearance-themes-review.md). Shortcut completed a correction and independent re-review with no new actionable findings; see [review evidence](reviews/appearance-shortcuts-review.md). All review assignments were verified as `codex`, `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`. Review artifacts retain task IDs, frozen hashes, checks and limits. The shortcut native gtests are authored but uncompiled and unregistered, and its patch is unapplied to the build.

## Blockers and next actions

Shortcut round 1 found one source-based P2 in the uncompiled native reset fixture: a hardcoded New Tab default can be locked without a macOS main menu. The coordinator changed it to select actual service-confirmed modifiable defaults and synchronized the embedded patch. Re-review confirmed the fix. A qualifying default on every target platform still requires native test execution.

Space themes depends on native Spaces. Motion depends on sidebar layout. Shortcut acceptance depends on installed native builds on Linux, Windows and macOS. Same-window multi-Profile host extraction remains unproven. Full Chromium preparation/builds, heavy downloads and remote CI are deferred. Local unit checks cannot close these issues.

Next: root integrates the reviewed commits. Future approved native work must establish production host ownership, connect the components to authoritative services/rendering, register/apply the shortcut artifact and gtests, and collect installed-build behavior/visual/restart evidence on each target platform. No issue is closed by this work.

## Meaningful checks

| Component | Proven local checks | Native gap |
| --- | --- | --- |
| Motion | Seven strict C++17 ASan/UBSan fixtures, Chromium formatting, clang-tidy analyzer/bugprone, independent source/API review | Driver/OS observation, rendered continuity/input/anchoring, frame traces, sound/haptic controls |
| Themes | Strict C++17 behavior and ASan/UBSan fixtures, Chromium formatting/analyzer, independent contrast math and pinned-source checks | Renderer/native roles, editor/name/icon, materials, OS notifications and durable restart |
| Shortcuts | Portable C++17 collector fixtures/analyzer, C++ formatting, Node syntax, pinned-source forward/reverse application, embedded identity and unchanged mutations | Unapplied artifact, uncompiled/GN-unregistered native gtests, frontend/Mojo/context routing and restart |

All three lack installed Linux/Windows/macOS acceptance. Exact commands and source evidence are in their feature documents. Source/doc whitespace checks pass; the patch's required blank context prefixes are verified by `git apply --whitespace=error` in its validator rather than treated as ordinary source whitespace. No shared lifecycle/tooling/series/workflow/catalog files changed. No pushes, PRs, issue updates, merges or native browser runs occurred.
