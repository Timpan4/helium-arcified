# Layout, split and motion composition P3 fix

Addressed the accepted P3 from
`layout-split-composition-review.md` on 2026-10-23. Only
`arcified/native/integration/layout/layout_split_composition_test.cc` and
this note were edited for this fix. The original review, feature document,
runner and production components were left unchanged.

## Change

`MotionCannotReplayObsoleteSemanticBounds` now compares
`transition.value().value()` with
`Decoration(sidebar->layout()).value()` immediately after successful
`ReduceMotion()` and after each rejected stale `Finish` attempt, for the
restore token and the previously canceled show token.

The public transition getter and `Decoration` both return `motion::Progress`,
whose `value()` exposes the numeric scalar. These assertions observe the
decorative endpoint through the public API. In this scenario the scalar is
zero before restoring the docked sidebar, and the current semantic layout
requires one. Clearing the run without settling its value therefore fails
the immediate assertion. The assertions after each stale completion require
that endpoint to remain unchanged.

All existing semantic content bounds, split geometry, focus state and
fixture ownership/reference assertions remain intact. No production logic,
test setup or completion attempts changed.

## Checks

Ran on Linux x86_64 with Clang, clang-format and clang-tidy 23.1.3:

1. `bash arcified/native/integration/layout/run_checks.sh` exited zero.
   Its strict C++17 compile uses `-Wall -Wextra -Werror -pedantic`.
   Both the strict and ASan/UBSan executables printed
   `layout/split composition: all scenarios passed`. The sanitizer compile
   uses `-fno-sanitize-recover=all -fno-omit-frame-pointer`, and execution
   sets `ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1`.
   No sanitizer diagnostics appeared.
2. The same runner passed
   `clang-format --style=Chromium --dry-run --Werror` on the changed test.
3. The same runner passed
   `clang-tidy --checks='-*,clang-analyzer-*,bugprone-*' --warnings-as-errors='*'`
   on the changed test, using the strict compile flags above. It reported
   22 warnings suppressed from non-user code and no unsuppressed diagnostic.

Verified test SHA-256:

```text
23e7e43a234337bb23e9da9278b82d43bf7dc8987b1b72a0ce9e486adf779929  layout_split_composition_test.cc
```

## Limits

These are standalone synthetic C++ integration checks. No broader unchanged
component suite, native UI interaction, Chromium/browser build, Windows or
macOS execution, OS reduced-motion observation or asynchronous native
callback lifetime was tested. No production mutation test was run; the
no-settle failure described above follows from the fixture's distinct
starting scalar and semantic endpoint. No Markdown linter was run.

No heavy download, remote write or commit was performed. A fresh independent
closure review remains for the parent to request.
