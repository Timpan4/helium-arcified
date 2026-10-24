# Split component review

Round 1 task: `splits-independent-review-round1` under the task ID prefix recorded in [status.md](status.md). Reviewer assignment verified as `codex/gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`. Reviewer completed without pending children.

## Accepted findings

1. Geometry zero-delta resize can reproject into active-only fallback. With horizontal weights `{a:1,b:2,c:1}`, viewport `{400,500}`, minima `{100,80}` and divider `5`, initial extents are `{100,190,100}`. Using resize output as weights leaves a trailing extent of `99.999999999999943`, incorrectly failing the minimum. Add both-orientation zero/clamped resize regressions and conserve representable bounds.
2. Geometry max normalization underflows a significant ratio before multiplication. Weights `{a:1e-100,b:1e300}`, viewport `{1e300,500}`, minima `{1e-200,80}`, divider `0` incorrectly allocate the first pane its minimum instead of its representable proportional extent near `1e-100`. Preserve significant products portably; widening alone does not cover MSVC.
3. Persistence documentation must state schema 1 explicitly omits orientation/weights, refuses unsupported feature stores through completeness attestation, and leaves future serialization to root. Add a public regression for ambiguity precedence when competing saved weights include an invalid value.

No correctness finding in layout or persistence reconciliation. Strict C++17 fixtures and formatting passed for all components, and geometry ASan/UBSan passed. Reviewer reproduced both geometry defects through public API harnesses despite existing fixture success. All native acceptance remains open.

## Current disposition

Both followups completed. Geometry now evaluates significant products with `frexp`/`scalbn` using portable `double` operations and reserves suffix minima with representable boundaries. New both-orientation zero/clamped resize and extreme-ratio fixtures passed strict C++17 and ASan/UBSan. Persistence docs state the agreed durability restriction; a valid/NaN competing-weight regression confirms ambiguity takes precedence. The reconciliation implementation is unchanged.

Independent round 2 (`splits-independent-review-round2`, same verified GPT-6.1 High/default assignment) completed without pending children or actionable findings. It reviewed all final code and issue docs, confirmed both geometry fixes and persistence ambiguity precedence, and reran all strict C++17 fixtures, geometry ASan/UBSan and Chromium formatting. Its separate public-API harness passed 360 zero-resize reprojections across both orientations, fractional dimensions and extreme values. This is the review's finite fixture set, not a product limit or coverage of all floating-point inputs.

The coordinator confirmed all reviewed code/doc hashes unchanged and reran Chromium formatting before staging. Native acceptance remains open; no native build, UI, accessibility, restart round trip, Windows/MSVC or macOS execution is claimed.

## Final reviewed blobs

| File | Git blob |
| --- | --- |
| `layout.h` | `a241a0d2e7861dd64cac45882de6cf20c4c8a081` |
| `layout.cc` | `1e592cf9f4188bb5c20b01ce609ded0a13321921` |
| `layout_test.cc` | `1216efc61b95af9c93f98aa67e0231a8903656a0` |
| `geometry.h` | `ac702baf9ce8604b0d4cee97cd43df67213c9648` |
| `geometry.cc` | `a1398bd516ced143a5acfceba072396a43617a60` |
| `geometry_test.cc` | `fa14942cb0b538087029e003b706b44811bec11b` |
| `persistence.h` | `b0c2573f99cac7dd03190917d81827d359310607` |
| `persistence.cc` | `9876ef46438a444364fa60008c26ac3257fb5c60` |
| `persistence_test.cc` | `ddef1e4325c5f2bfab28a8de2d110364e8abddae` |
| `horizontal.md` | `9cb2b8180820ff122bfe1058fdfe08198db2d2c2` |
| `vertical.md` | `f737ac6490313cf37a0f6da3cece8a8c8fdd1f2f` |
| `persistence.md` | `aeb89c52e65db833a21f89e47b07597eb98957ee` |
