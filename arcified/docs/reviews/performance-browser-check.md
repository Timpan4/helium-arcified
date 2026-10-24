# Focused fixture browser check

Performed on 2026-10-23 after explicit owner approval for tiny synthetic checks in T3 preview. This is functional fixture QA, not a benchmark, Arc/Dia observation, native Arcified build or platform performance result. Production fixture code was unchanged after [clean independent re-review](performance-review-round-2.md).

## Environment and inputs

Used a new T3 incognito tab on Linux, T3 Code Nightly 0.1.3 with Electron 44.9.3 and Chrome 152.0.7977.167. The local Node 24.22.1 fixture server listened on its OS-assigned port, `http://127.0.0.1:44047`, with the documented no-store policy. No real profile, account or public site was used.

The workload reused regression-test inputs: identity progress, zero rows/text repetitions, batchRows 1, three requests and seven ASCII bytes per response. These are tiny test choices, not defaults, limits or performance targets. Typed state was the literal synthetic-only.

## Observed checks

- Clicking the empty form stayed on the choice page and reported all six required fields missing. Each labeled input was filled through T3 native interaction tools.
- Submitting the choices opened the expected workload URL. The title and visible ready status appeared; the page made three selected local payload requests.
- Typing synthetic state and clicking Echo input displayed Echo 1. The exported observations contained both echo-handler and echo-raf-callback; those callbacks do not establish screen-presentation latency.
- Export raw page observations produced two completed downloads named page-observations.json. Parent read both actual UTF-8 download files using fast_filesystem. Each JSON reported outcome ready, completedRows 0, completedRequests 3, consumedBytesDecimal 21, request IDs 0/1/2 and the synthetic-only input. A temporary observer also inspected the production export Blob and was restored immediately. Blob inspection was not used as a substitute for verifying the saved files.
- Paired-document navigation reached progress-next. Back returned to progress with ready status, but the synthetic input was empty and Echo reset. This run therefore does not establish form restoration or BFCache persistence; no cause is inferred.
- T3's environment-port shortcut initially used localhost, which the fixture intentionally rejects with HTTP 403. The valid printed 127.0.0.1 endpoint returned 200. No Host/security rule was changed.

The parent-owned server, PID 368151 in terminal session 59776, was stopped by SIGINT after checks and exited 0. No user process was stopped and no artifacts were deleted. The incognito QA tab retains its last synthetic page; the server is no longer listening.

## Retained local evidence

- Choice-form screenshot: /home/timpan4/.t3/userdata/browser-artifacts/browser-screenshot-127-0-0-1-mvl06yan-f4c126dd.png
- Ready screenshot: /home/timpan4/.t3/userdata/browser-artifacts/browser-screenshot-127-0-0-1-mvl09wjq-37bfe6ca.png
- Echo/export screenshot: /home/timpan4/.t3/userdata/browser-artifacts/browser-screenshot-127-0-0-1-mvl0atfx-19cf215a.png
- First saved JSON: /home/timpan4/.t3/userdata/server-browser/downloads/469d65390cc4ce8e0a0b7026bb94504028c17270e2d421ff78ecccadce961c70/5ab38ff2-9b97-43d9-ae13-fc373fb9482c
- Second saved JSON: /home/timpan4/.t3/userdata/server-browser/downloads/469d65390cc4ce8e0a0b7026bb94504028c17270e2d421ff78ecccadce961c70/c240a5ed-c7fa-4391-ac4f-31c239f2c16d

These are host-local raw artifacts, not portable repository inputs. The steps and observed semantic fields above allow repetition without relying on their paths.

## Remaining proof

Actual browser cancellation during row construction, streaming or a held frame, background throttling, BFCache/restore behavior and nonzero-row layout remain unverified by this small check. The held-frame cancellation regressions use production logic with stubbed boundaries and pass; that result stays distinct from real browser scheduling. Native command/sidebar actions, clock correlation, OS/process resource collection and all comparative performance remain unverified. No distribution, speedup, memory budget or latency target is established.
