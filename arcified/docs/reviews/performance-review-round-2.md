# Issue 20 independent performance review, round 2

Disposition: P2 is cleared by source inspection and focused tests of production progress/export/cancellation control flow. No new actionable finding was identified. All three research acceptance areas remain covered at specification level. This review establishes no benchmark measurement, direct browser behavior or native platform result.

## Scope

Read the [original issue](../research/arc-dia/publication/perf-baseline.md), [first independent review](performance-review.md) and [fix response](performance-progress-response.md). Inspected all five response-listed artifacts in full: `workload.mjs`, the new `workload.test.mjs`, fixture README, performance protocol and fix response. Also read the existing HTTP/config tests, workload configuration, workload HTML and session/run templates to check the test boundary and acceptance coverage.

Only this report was added. Prior reports and implementation were preserved. No GitHub action, PR linking/watching, browser access, unrelated browser files, real profile, benchmark load, full build, global installation or remote CI was used. The first review's broader artifact/privacy inspection was not repeated; the inspected response changes introduce no apparent private data.

## P2 resolution

In `arcified/performance/workload.mjs`, the request loop now uses its own zero-based `request` variable at line 112. After stream draining, the length check precedes `completedRequests += 1` at line 130. The body-consumed event follows, then the cancellable frame wait at line 134. There is no asynchronous boundary between successful length validation and incrementing the count. A failed or incomplete body cannot reach that increment.

The export handler reads the production count and exact byte total at line 70. Cancellation aborts the pending frame, and the catch handler retains the same count and bytes at line 145. Request event identity no longer depends on whether the frame finishes. The final abort check still prevents cancellation during the last frame wait from becoming `fixture-ready`.

The new tests exercise this production path rather than a copied counter implementation:

1. `workload.test.mjs:63` imports `workload.mjs` as a distinct module instance per case. Fetch is stubbed but returns real Node `Response` bodies. Production code drains and validates those bodies.
2. The frame stub retains callbacks without executing them. With zero rows, the first held callback is the post-body frame. The first test exports one completed response and 7 bytes before advancing that callback. It then holds the second frame and exports two responses and 14 bytes. It checks request-start, response-header and body-consumed IDs remain `[0, 1]`.
3. Export dispatches the production export button handler and parses the actual Blob intercepted at `URL.createObjectURL`. Cancellation dispatches the production cancel handler and lets its rejected frame reach the production catch handler. The assertions retain two completed responses and 14 bytes in the export and cancellation event, remove the pending frame, prevent a third fetch and exclude readiness.
4. The second case holds the final frame after one zero-byte response. Both pending and cancelled exports retain one completion and zero bytes, with no readiness event. Completion is therefore checked independently of a positive byte total.

These checks directly cover the reported undercount schedule. The response's claim that both tests failed before the fix was not independently rerun against an older implementation. The current source and passing assertions are sufficient to clear P2 at this control-flow boundary.

## Checks and claims

On Node `v24.22.1`, the combined command passed all six tests:

```sh
node --test arcified/performance/check.test.mjs arcified/performance/workload.test.mjs
```

`node --check` also passed separately for `config.mjs`, `server.mjs`, `workload.mjs` and `workload.test.mjs` under `arcified/performance/`. No formatter or linter was run; no configuration for these standalone fixtures was found in the fixture directory or repository root.

The README, protocol and response describe the corrected completion boundary consistently. They retain the distinction between body consumption, frame callbacks, readiness, presentation and native collection. No unsupported browser/native success claim was found in the response changes.

The stubbed tests prove production serialization and control flow under the held-frame schedule. They do not prove real browser rendering, download delivery, event scheduling, fetch abort during streaming, background throttling, row construction or history behavior. The cancellation cases do not release the final frame to exercise successful readiness. The existing HTTP/config tests remain separate server/config evidence.

## Research acceptance

| Issue acceptance area | Round 2 assessment |
| --- | --- |
| Fixtures, process accounting and milestones for startup, tab activation, navigation, command search, scrolling, memory and idle | Covered as definitions by the explicit synthetic workload, session/run templates, all seven scenario rows, milestone table and process/resource accounting procedure. Native surfaces and collectors remain execution requirements. |
| Hardware/OS/build, engine/settings, blocking/extensions, account/network/cache controls and matched fork/Helium baseline | Covered by preparation procedures and recording fields. Reference machines, matched builds, workload choices and completed paired runs remain inputs, not established results. |
| Separate perceived response, input-ready, paint, network completion and quiescence; retain raw traces/distributions; evidence-based targets | Covered by distinct endpoint/clock definitions, raw-artifact and distribution requirements, cancellation semantics and the requirement to derive targets from evidence and owner needs. No traces, distributions or targets were produced. |

The requested research artifact is ready at specification level after clearing P2. That judgment does not close native implementation or platform verification.

## Remaining direct proof

1. In an isolated synthetic browser, check form validation, title/row identity, zero-work readiness, echo/focus, paired navigation and the delivered observation JSON. Check cancellation during rows, response streaming and held frames, plus foreground/background transitions. Compare retained events, counts and bytes without treating frame callbacks as screen presentation.
2. Check navigation, back/forward and reload with synthetic form/scroll state. Observe history/visibility events and `pageshow.persisted` only where restoration occurs. Preserve evidence before navigation. These checks remain with the parent, including browser-automation permission.
3. Before comparative measurement, fill owner-selected run conditions and verify native actions, input/presentation capture, clock mapping, process/tab membership and resource collectors on Linux, Windows and macOS. Native startup/restore, activation, command search, sidebar/page scrolling, memory lifecycle and idle accounting remain unverified.

Full builds, benchmark collection and remote CI remain deferred within the stated research scope.
