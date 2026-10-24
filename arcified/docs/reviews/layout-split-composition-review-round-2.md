# Layout, split and motion composition review, round 2

P3 is closed. No new finding or blocking regression was found in the revised
conformance test. Native UI, visual and platform verification remain OPEN.

Reviewed independently on 2026-10-23. Scope covers both files under
`arcified/native/integration/layout/`, the feature document
`arcified/docs/features/layout-split-composition.md`, and the prior review and
fix response. Only this report was written.

## P3 closure

The new assertions at
[test lines 469, 471 and 473](../../native/integration/layout/layout_split_composition_test.cc#L469)
compare the public transition scalar with
`Decoration(sidebar->layout()).value()`. Each obtains the current semantic
layout afresh. They run immediately after successful `ReduceMotion()`, after
the rejected restore-token `Finish`, and after the rejected canceled-show
`Finish`, respectively. The existing token-rejection assertions remain.

The scalar is explicitly checked as zero before restore. Retargeting in
animate mode creates a run without changing that scalar. Exiting focus has
already restored a docked sidebar, so the fresh decorative endpoint is one.
An implementation that clears the run and reports success without settling
its value now fails the immediate assertion. The following assertions also
require stale completion attempts to preserve the current endpoint.

These comparisons use exact endpoints supplied by `Progress::Start()` and
`Progress::End()`, not an approximate interpolation result. The inspected
public getter returns `Progress` by value; its scalar getter and
`Decoration` do not mutate semantic or transition state. No production
mutation test was run. The no-settle failure follows from the distinct
starting scalar and required endpoint.

## Preservation and regression review

Removing exactly the three added assertion lines in memory reproduces the
prior review's test SHA-256:

```text
5a9ab84966e2f2283eb5ce782e95bbb174c0b501c6b6467854aa275dac5c38ef
```

The current test SHA-256 matches the requested fix identity:

```text
23e7e43a234337bb23e9da9278b82d43bf7dc8987b1b72a0ce9e486adf779929
```

Thus the remaining test source, including setup, semantic actions, stale
completion attempts and ownership assertions, is byte-for-byte unchanged.
Reading the full revised test confirmed that the actual focus projection,
tab override, sidebar content and split geometry pipeline remains intact.
The const lifecycle fixture still outlives its borrowed tab projections;
order and selection owners remain stable. Accepted resize weights remain
the deliberate metadata update. Motion still receives only decorative
values and tokens, with no semantic state reference or commit callback.

The runner and feature document match their prior hashes:

```text
84532165dabf28296c9542de531b3f64b8dd06e6bede6cb3d35d78cc413abc47  run_checks.sh
ac9c2b6d90ccb385f80abeb4bd8caa59088b317b922f263ddc3a6726680d86a4  layout-split-composition.md
```

Each independent `git diff --exit-code` comparison returned zero for layout
against `127bf189`, splits against `eba06b39`, and motion against `e2057bd1`.
No component change was found. The runner's error propagation and temporary
directory cleanup are unchanged; the prior early-failure and TERM cleanup
evidence remains applicable with its stated signal-handling limits.

## Verification evidence and limits

This round performed full scoped source/document reading, exact hash checks,
in-memory reconstruction of the prior test identity, and the component
comparisons above. It did not rerun executable tests or tooling. It reuses
the primary's successful existing-runner results for the exact current test
hash, recorded in
[the fix response](layout-split-composition-fix.md): strict C++17,
fail-fast ASan/UBSan, Chromium formatting, and scoped analyzer/bugprone lint.
The unchanged runner/error/cleanup evidence comes from
[the original review](layout-split-composition-review.md).

The added checks close the reported endpoint coverage gap without changing
the exercised semantics. This remains synchronous standalone conformance
evidence. There is no production native host or facade. Native view placement,
actual focus/capture, dialog/fullscreen UI transitions, asynchronous animation
callback lifetime, rendering, accessibility, OS reduced-motion observation
and Linux/Windows/macOS browser verification remain OPEN. No visual check,
Chromium preparation/build, heavy download, commit, remote write, PR link or
PR watch was performed. Input files and prior reports were preserved.
