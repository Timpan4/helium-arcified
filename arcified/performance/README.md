# Local synthetic performance fixtures

These files support the [performance protocol](../docs/performance-protocol.md).
They do not launch browsers, import profiles, collect OS counters or report benchmark scores.
They use Node's built-in modules and the installed Node runtime, with no package installation.

1. Copy and fill `session.template.json` and `run.template.json` into an operator-chosen results directory outside the repository. The templates have no workload defaults. Keep only public or synthetic data.
2. Start the loopback-only server from the repository root. Choose its port and cache mode explicitly:

   ```sh
   node arcified/performance/server.mjs --port 0 --cache no-store
   ```

   TCP port `0` asks the OS to allocate an available port. Read the printed origin. For saved-session URLs, choose and record a stable available port instead. `no-store` requests no HTTP storage; `revalidate` allows storage but requires validation and supports ETags for static assets. Neither policy establishes a cold filesystem, empty browser cache or BFCache hit. Redirect stdout to an operator-chosen raw log if running a measurement.
3. Open the printed origin in an isolated synthetic browser profile when a benchmark run is authorized. Fill every workload choice in the form. Save the resulting full URL. Create the manifest's exact tab/window sequence manually with the same URLs on both builds. Nothing opens a tab automatically.
4. Use the page's identity, state field, ordered rows, echo button and paired navigation link for the protocol scenarios. Payload requests are sequential, begin after row construction, and drain a same-origin ASCII stream. There is no periodic fixture work after readiness. Explicitly record zeros when omitting rows, text or requests. `batchRows` is positive so construction progresses; JavaScript safe integers and TCP port width are representation constraints, not workload caps.
5. Cancel remaining work with the page button. It aborts fetches and pending frame waits. It does not remove completed rows or stop the browser. Export observations after cancellation or completion. Ctrl+C stops this server and its connections. Keep partial artifacts and mark the run cancelled.

The page yields between chosen row batches and after each consumed response. A response counts as completed immediately after its body is drained and its length validated, before the frame wait. Export and cancellation retain that count and the bytes already consumed. Request events use stable zero-based request IDs. Cancellation during the final frame wait remains cancellation even when all chosen responses have completed. Each batch and string allocation is synchronous. An excessive choice can delay cancellation or exhaust resources; choose sizes deliberately on the reference machine. Background frame throttling can postpone readiness. No timer decides readiness or quiescence. The export includes page input event timestamps, handler/frame callbacks, response-header/body observations, navigation/resource/paint entries, visibility/history events, progress and synthetic form/scroll state. Byte totals are exact decimal strings, avoiding accumulated integer rounding. It excludes typed keystroke contents, but exports the explicitly entered synthetic state field. Do not type real data.

Exporting allocates JSON and a download blob. Do it after the observation window. Each download blob is retained until page exit. Page observations and marks also consume memory and alter input/scroll paths. Compare identical instrumentation and use a separate minimally instrumented run to estimate its effect. Browser resource-timing buffers use the browser's default capacity. An overflow marker means the export may be incomplete; retain an external trace. Navigation destroys in-memory observations unless BFCache retains the document. Export before leaving when needed, and record an external trace for navigation/startup.

`request-closed.finished` in server logs means Node finished writing its response. It does not prove client consumption, on-screen paint or network-wide inactivity. The server streams using Node's writable buffer size and applies no synthetic delay or throughput shaping. Its CPU/storage/stream overhead shares the test machine and must be accounted separately. The fixtures model DOM/text and controlled local transfer, not real sites, media, GPU-heavy content or the entire public web.

Focused checks, without opening a browser or creating a benchmark load:

```sh
node --test arcified/performance/check.test.mjs arcified/performance/workload.test.mjs
node --check arcified/performance/config.mjs
node --check arcified/performance/server.mjs
node --check arcified/performance/workload.mjs
node --check arcified/performance/workload.test.mjs
```

The HTTP/config tests make tiny loopback requests to check explicit choices, deterministic payload bytes, serving boundaries, cache behavior and errors. The workload tests import production `workload.mjs` with isolated DOM/frame boundaries and real response bodies. They hold the post-body frame, export progress through the page handler, cancel and check retained response/byte counts, request IDs, removal of the frame, no future requests and absent readiness. They also check cancellation after the final zero-byte response. These tests do not measure performance or prove browser rendering, browser event scheduling/cancellation, session restoration or any native platform behavior. Those remain direct-run checks in the protocol.

## Focused browser evidence

The parent also checked tiny synthetic form/ready/echo flows and read actual downloaded JSON in an authorized T3 incognito tab. See [the browser QA record](../docs/reviews/performance-browser-check.md) for exact inputs and limits. This is fixture functionality evidence, not a benchmark or native Arcified result. Browser cancellation and restoration remain qualified in that record.
