# Issue 20 independent performance artifact review

Disposition: changes requested for a partial-progress recording defect. The research protocol covers the issue's acceptance areas at the specification level. This review establishes no benchmark result or native platform behavior.

## Scope and evidence

Reviewed `arcified/docs/performance-protocol.md`, every file under `arcified/performance/` and the original issue artifact at `arcified/docs/research/arc-dia/publication/perf-baseline.md`. Checked the cited patch-series entries and source evidence for memory-saving defaults and lazy restore. This is the first independent review, with no prior findings.

Only this review file was added. No implementation, GitHub or PR changes were made. No browser, real profile, benchmark load, full build, global installation or remote CI was used.

## Finding

### P2. Count consumed requests before the cancellable frame wait

Evidence: `arcified/performance/workload.mjs:112` increments `completedRequests` in the `for` update expression. The body is already drained and length-checked before `request-body-consumed` at line 130, but line 133 awaits `nextFrame(controller.signal)` before that increment. The cancellation handler at line 60 aborts that wait. The catch handler then records the unchanged count at line 144, and export copies it at line 70.

Consequently, cancellation after a body finishes but before the next frame produces contradictory partial evidence. For example, after the first chosen response is consumed, cancellation in that interval records its body-consumed event and bytes but reports `completedRequests: 0`. Exporting while the same wait is pending also understates progress. Frame throttling can extend this interval. The cancellation outcome itself remains correctly distinct from readiness.

Record the completed-request count at successful body consumption, before yielding, while preserving a separate request identity for event correlation. Add a focused regression check that holds the post-body frame pending, cancels, and inspects exported progress. This finding follows from source control flow; it was not reproduced in a browser during this review. It blocks a clean artifact review because retained completed work is part of the cancellation contract.

## Acceptance coverage

| Original requirement | Artifact coverage | Assessment |
| --- | --- | --- |
| Fixtures, process accounting and milestones for startup, activation, navigation, command search, scroll, memory and idle | The scenario table names actions, correctness checks, endpoints and competing resource limits for every scenario. The synthetic page supplies titles, ordered rows, form/scroll state, echo, navigation and controlled local response bodies. Native surfaces and OS collection remain explicitly separate. | Covered as a protocol and fixture definition, subject to the progress defect above. No scenario was executed here. |
| Hardware/OS/build, engine/settings, blocking/extensions, accounts/network/cache and matched fork/Helium | Preparation procedure and run/session templates record these controls, pairing, build identity, independent cache axes, restore state and process membership. Linux, Windows and macOS collection candidates are distinguished. | Covered at artifact level. Reference machines, matched binaries and completed manifests remain owner/run inputs. |
| Perceived response, input-ready, paint, network and quiescence; retained raw traces/distributions; evidence-based targets | The milestone table distinguishes these endpoints and their clocks. The protocol requires raw event locators, clock mappings and uncertainty, partial/failed observations, process/resource definitions, paired distributions and limiter evidence. | Covered as a collection and analysis contract. No traces, distributions, overhead estimates or targets were produced. |

The empty templates and explicit workload inputs are intentional configuration points, not evidence of a completed session. Owner-selected sizes and run conditions must be recorded before measurement. Their absence does not require inventing benchmark sizes or targets for this research artifact. The issue's named decision/evidence artifact and scenario coverage exist; the artifacts do not close native integration or platform verification work.

## Implementation assessment

- Endpoint and clock definitions correctly reject DOM changes, document input delivery, animation-frame callbacks and Node response completion as proof of screen presentation. Missing observations remain distinguishable from zero. CPU denominators, current versus peak memory, shared pages, transient processes, GPU uncertainty and server/observer costs are addressed.
- The CLI binds to `127.0.0.1`. Requests require a loopback Host, allow only GET/HEAD, use an explicit static-asset map and validate payload byte choices. No filesystem path comes from the request. CSP restricts resource connections to the same origin. The inspected code does not proxy remote requests or expose repository files outside its asset map.
- Configuration rejects missing, duplicate, unknown, fractional, negative and unsafe integer choices. Text and response bytes are deterministic for a fixed configuration. Layout, lifecycle, cache and timing still depend on the recorded browser environment. The positive batch size ensures progress; safe integers and TCP port width are representation constraints. Tiny test values are identified as test inputs. No unsupported workload quota or latency target was found.
- Construction yields at frame boundaries, fetches use an abort signal, and pending frame waits remove/cancel their callbacks. Synchronous text allocation and row batches can delay cancellation, as the README and protocol disclose. Background frame throttling can delay progress. These mechanisms were read, not exercised in a browser.
- Exports retain page observations, marks, resource/navigation/paint entries and exact byte totals, while disclosing buffer overflow, observer cost, blob retention and loss of in-memory evidence on navigation. External tracing is required for navigation and startup. The completed-request defect is the identified inconsistency in partial progress.
- No accidental private data was found in the protocol or any fixture file after full text inspection and a scoped path/credential/URL search. Templates contain null placeholders; fixture text is synthetic; served URLs are loopback and documentation links are public. No unrelated browser configuration files were opened. Future exports deliberately include the full fixture URL and synthetic state field, so this finding covers the checked-in artifacts only.

## Checks performed and their limits

`node --test arcified/performance/check.test.mjs` passed all four tests. These exercise explicit workload parsing, allowlisted assets, tiny deterministic payloads including zero bytes, malformed payload choices, HEAD, method/Host rejection, static ETag revalidation and required cache selection.

`node --check` passed for `config.mjs`, `server.mjs` and `workload.mjs`. No formatter or linter was installed or run. No browser test was run. The HTTP/config tests never import `workload.mjs`, so they do not test DOM construction, frame scheduling, cancellation, observation export or history behavior. They also do not establish stream-abort cleanup or CLI signal shutdown. Passing those tests cannot clear the finding or prove native behavior.

## Remaining direct proof

1. In an isolated synthetic browser, check the form, validation/error display, deterministic title/rows and paired URL, explicit zero-work readiness, echo/focus behavior and the downloaded observation JSON. Compare expected counts and byte totals with retained events.
2. Check cancellation during row construction, during a response and while a frame is pending, including the post-body interval identified above. Verify partial evidence, absent success milestones and that no further chosen work continues. Check background throttling and foreground recovery without treating frame callbacks as presentation evidence. Use deliberately small contract inputs rather than a benchmark load.
3. Check navigation, back/forward and reload with synthetic form/scroll state. Verify visibility/history observations and `pageshow.persisted` where a BFCache restoration actually occurs. Preserve pre-navigation exports or external traces. Check actual page and sidebar scrolling separately when native verification is authorized.
4. Before any comparative run, fill the owner choices and confirm actual native actions, input/presentation capture, clock correlation, process/tab mapping and resource collectors on each target platform and build. Startup/restore, loaded/deferred/hibernated activation, native command search, scroll presentation, memory lifecycle and idle accounting remain unverified. An unavailable command surface or counter must remain an explicit gap.

These checks remain open. Full builds and benchmark collection remain deferred, consistent with the requested research scope.
