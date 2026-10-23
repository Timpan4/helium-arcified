# Parallel integration evidence

The parent integrated reviewed components for all 51 additional assigned issues on `feat/parallel-browser-work`. These are local slices. No native issue acceptance or browser release is claimed.

The [manifest](parallel-workstreams.json) retains every primary assignment, isolated area thread and source-to-integration commit. All seventeen area deliveries are integrated, including the reviewed source-only shortcut patch. That patch remains unapplied and its native gtests uncompiled. Region precision/doc defects were fixed and independently cleared before capture integration. All final area feature bytes match their reviewed branches/commits.

## Preserved review evidence

The parent read retained independent-review and workstream reports and compared integrated feature sources with reviewed area branch/commit bytes using `git diff --exit-code <reviewed-ref> HEAD -- <area paths>`. All comparisons exited 0. Unchanged passing component evidence is retained in the feature documents instead of being rerun and relabeled as a new result.

## New combined-tree checks

Commands and site-tools were developed in separate worktrees. The parent compiled their actual combined sources with Clang C++17, warnings as errors, AddressSanitizer and UndefinedBehaviorSanitizer, `-fno-sanitize-recover=undefined`, and `-fno-omit-frame-pointer`. Each program linked `lifecycle.cc`, the actual shared `commands/command_target.cc`, its feature implementation and focused test.

| Program | Result |
| --- | --- |
| `/tmp/arcified-combined-site-controls-sanitized` | Exit 0, nine behavioral fixtures passed. |
| `/tmp/arcified-combined-devtools-sanitized` | Exit 0, session behavioral checks passed. |
| `/tmp/arcified-combined-diagnostics-sanitized` | Exit 0, diagnostics behavioral checks passed. |

No sanitizer diagnostics occurred. This resolves the separate-checkout compile gap for those local combinations. It does not run native Page Info, DevTools, task manager or tracing.

## Remaining work

The sidebar task produced composed conformance scenarios because authoritative native ordering/binding/localization inputs are absent. Fresh independent review passed and the scenarios were committed at `34c97b01`. Mandatory whole-snapshot and current decoded-candidate commit admission were implemented and independently cleared at `f634070a`; the [correctness](reviews/snapshot-admission-correctness-review.md) and [contract](reviews/snapshot-admission-contract-review.md) reviews cover those pure API gaps. Schema-2 Space metadata preservation was independently cleared and committed at `b47468ae`. Native object retention, freshness, serialization and real IO remain open. Layout/split/motion conformance and the pinned consumer inventory are reviewed and integrated. Same actual native-window cross-Profile feasibility remains unproven.

Split geometry's two independently reproduced numeric defects are fixed and cleared by fresh review. The final reviewed sources were integrated unchanged. No rendered split or native resize check is claimed.

The [Linux run](native-linux-run-20261023.md) passed full source preparation through GN; root baseline compilation is running. Native runtime, real filesystem durability, installed Linux/Windows/macOS interaction, authentication isolation, rendered accessibility/focus behavior and performance remain open. Safety/cleanup/destructive/populated migration policies remain open. GitHub rejected foundation publication because the current credential lacks workflow scope; no implementation push, PR, CI dispatch or issue closure has succeeded.

## Final capture join

The parent compiled `lifecycle.cc`, actual `region_capture.cc` and `capture.cc`, and `capture_region_test.cc` with strict C++17 and fail-fast ASan/UBSan, then ran `/tmp/arcified-combined-picker-capture-sanitized`. It exited 0 and printed `capture/region seam fixture passed`, without sanitizer diagnostics. This checks synthetic coordinate/raster/preview composition, not native page acquisition or a visible picker.
