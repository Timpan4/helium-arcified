# Issue 20 fixture progress response

The accepted P2 defect is fixed. `completedRequests` advances immediately after successful body draining and length validation, before recording `request-body-consumed` and awaiting a frame. A separate loop-local request ID preserves zero-based event correlation. No dependency or production test API was added.

Changed files:

- `arcified/performance/workload.mjs`: move the completion boundary and keep request IDs stable.
- `arcified/performance/workload.test.mjs`: import the production module, hold its frame callback and inspect JSON from its export handler. Check progress after each consumed response, cancellation retaining two responses and 14 bytes, stable event IDs, no third request, frame removal and absent readiness. A second case cancels the final frame after a zero-byte response and retains one completed request without readiness.
- `arcified/performance/README.md`: document the boundary, focused command and test limits.
- `arcified/docs/performance-protocol.md`: clarify retained completion and cancellation semantics.
- `arcified/docs/reviews/performance-progress-response.md`: this implementation response. The independent review remains unchanged.

Checks on Node `v24.22.1`:

```sh
node --test arcified/performance/workload.test.mjs
node --test arcified/performance/check.test.mjs arcified/performance/workload.test.mjs
node --check arcified/performance/config.mjs
node --check arcified/performance/server.mjs
node --check arcified/performance/workload.mjs
node --check arcified/performance/workload.test.mjs
```

Before the production fix, both new regressions failed with `completedRequests: 0` instead of `1` while the first post-body frame was pending. After the fix, the combined command passed all six tests. All listed syntax checks passed. No formatter or linter is configured for these standalone fixtures; none was installed.

The regression uses Node response streams and Blob serialization with stubbed DOM, fetch and frame scheduling boundaries. It proves the production progress/export/cancellation control flow under that schedule. It does not prove browser rendering, actual download delivery, trusted input, browser event scheduling or background frame throttling. Direct-browser cancellation during row construction, streaming and frame waits, history/restore, downloaded JSON and other UI checks remain open for parent coordination. No native performance evidence, benchmark load, real profile, full build, global package or remote CI was used. No artifacts were deleted. Parent owns independent re-review and GitHub work.
