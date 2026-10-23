# Implementation execution

The owner requested non-AI work in priority order, one GPT-6.1 High primary per task. AI has 23 later items; six sync proposals are excluded. Full browser builds are deferred. Current task scopes also prohibit heavy source preparation and remote CI execution.

## Current work

| Work item | State | Evidence and remaining work |
| --- | --- | --- |
| [Performance protocol #20](https://github.com/Timpan4/helium-arcified/issues/20) | Locally complete, awaiting publication | Six tests, four syntax checks and clean independent review. Parent verified tiny browser form/ready/echo and saved JSON. No benchmark result. |
| [Ownership contract #24](https://github.com/Timpan4/helium-arcified/issues/24) | Standalone v2 core reviewed; owner policy choices pending | P1 fixed, 13 fixtures plus supplemental sanitizer groups pass. Native state/auth transfer is unverified. |
| [Linux #16](https://github.com/Timpan4/helium-arcified/issues/16), [Windows #17](https://github.com/Timpan4/helium-arcified/issues/17), [macOS #18](https://github.com/Timpan4/helium-arcified/issues/18) | Partial preparation foundations independently reviewed | Shared path/provenance and Mac filesystem findings cleared. Native preparation, SDKs, GN, build and launch remain OPEN. Lightweight manual-only cross-platform checks are being prepared. |
| [Native Spaces #32](https://github.com/Timpan4/helium-arcified/issues/32) | Preparatory integration-seam analysis active | Ground the same-native-window multi-Profile requirement in pinned Chromium/Helium sources, with Astra guidance assigned. No fake adapter or native acceptance claim. |

## Owner decisions

Approved: shared saved tabs and window-local temporaries; existing saved-page handoff into the current window preserving state; same-window Space switching across isolated Profiles; full builds deferred; tiny synthetic T3 preview checks permitted.

Still pending: Profile/Space and settings mapping, outbound saved-tab link routing, and conservative activity/cleanup/migration defaults. Do not infer answers from elapsed time or selected recommendation defaults.

## Verification and counts

Of 135 non-AI items: 0 published/proven complete, 1 locally complete awaiting publication, 5 in progress and 129 not started. Partial scaffolding does not close native launch issues. Performance artifact completion establishes a protocol and tested fixture behavior, not performance parity.

Current correctness review findings are cleared. Default combined Pylint still reports documented import/duplication diagnostics; scoped project-policy lint passed. Native Windows, APFS/junctions, Xcode/SDK and actual browser feature integration remain unverified. The owned fixture server exited 0 after SIGINT; no full build, heavy preparation, remote CI, commit, push or PR ran.

[execution-state.json](execution-state.json) holds current task IDs and [fixture browser QA](reviews/performance-browser-check.md) records direct checks. Next: consume native seam/CI results, adopt policies after owner answers, review new changes and run focused integrated checks. All changes remain local and uncommitted. Parent owns integration and GitHub state.
