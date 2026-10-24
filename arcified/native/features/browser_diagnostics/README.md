# Browser diagnostics protocol

Local reusable C++ component for [issue 142](https://github.com/Timpan4/helium-arcified/issues/142), using lifecycle contract version 3. This implements command planning, asynchronous trace protocol transitions and an aggregate metadata serializer. It does not open the native task manager, record a native trace, redact a trace or implement a diagnostics UI. Issue 142 and its [command-bar dependency, issue 77](https://github.com/Timpan4/helium-arcified/issues/77), remain open.

The scope follows the [publication brief](../../../docs/research/arc-dia/publication/browser-diagnostics.md) and `browser-diagnostics` entry in [backlog.json](../../../docs/research/arc-dia/backlog.json). The [ownership design](../../../docs/native-space-integration.md) and [pinned source ledger](../../../docs/native-space-source-references.md) require native ownership extraction, not a view-only adapter. Their reviewed baseline predates lifecycle v3. This component reads the current const `Model` and does not change that design or claim its native feasibility.

## Public interface and ownership

`PlanTaskManager(Model, WindowId, Capability)` returns a plan only when the explicit logical window exists and the host declares the native command available. Its scope is always `kBrowserProcesses`. This means the current browser instance and its participating child processes, potentially including other windows and Profiles. The invocation window is neither a process filter nor a Profile/page restriction. No process-kill action is provided.

The command dispatcher must replan at actuation and verify the real native window and host capability. The component does not find an active page, infer focus, read URLs or maintain a second lifecycle target resolver. If a future task-manager adapter selects a page's row, it must use the coordinator-owned `PageCommandTarget` and `RevalidatePageCommandTarget` seam, then validate native TabModel/WebContents identity and generation. Those shared interfaces are not imported from an unverified peer path here.

`TraceSession::Start(Model, WindowId, Capability, TraceId)` validates the logical invocation context and emits a browser-wide `kStart` action. The session retains no Model reference, window, page, Profile, attachment, navigation or focus state. Native trace ownership must outlive closure of the invocation window. The host supplies capabilities, serializes commands/events on the native UI sequence, arbitrates the global recorder against all other clients, allocates non-reused session IDs and executes emitted actions. There is no native implementation behind these actions yet.

Trace IDs are unsigned integers that must strictly increase for accepted starts on the same object. Failed validation does not consume an ID. Global allocation and non-reuse across object lifetimes remain host responsibilities. Each emitted action also has a private in-process completion token. Copies preserve the token; retries create a fresh token. Native acknowledgements must carry the actual emitted `TraceAction`, so equal session numbers on another object and delayed callbacks from an earlier operation cannot complete the current one. These tokens are not an IPC or persistence format. `TraceSession` cannot be copied or moved into a second protocol owner.

## Trace behavior

| Input and current state | Result and required native work |
| --- | --- |
| Start from idle, ready, cancelled or confirmed start failure | Clear prior metadata, enter starting, emit `kStart`. Pending or unresolved native work refuses another start. |
| Confirmed native start acknowledgement | Enter recording. A confirmed rejection enters start-failed and asserts that this session owns no recorder or trace data. An uncertain failure uses `ReportNativeFailure` instead. |
| Stop while recording | Enter stopping, emit `kStopForSummary`. The request alone provides no output. |
| Cancel while starting | Enter cancel-pending-start, retain the pending start token. Successful start acknowledgement emits `kStopAndDiscard`; confirmed rejection completes cancellation without stopping another client's recorder. |
| Cancel while recording | Enter discarding, emit `kStopAndDiscard`. Cancellation is incomplete until native stop and data discard are acknowledged. |
| Cancel while stopping | Enter cancel-pending-stop and retain the existing stop token. Do not dispatch another stop. Successful stop and discard acknowledgement drops the incoming summary and completes cancellation. |
| Repeated cancel while pending | No duplicate native action and no fabricated completion. |
| Native failure during start, recording, stop or discard | Enter cleanup-required, invalidate the failed operation and refuse new starts and exports. While recording, report a recorder/service failure with the acknowledged start action; while an action is pending, use that action. |
| Explicit cancel from cleanup-required | Emit `kReconcileAndDiscard` with a new operation token. The host must reconcile only this session's recorder and data. No automatic retry or timeout is introduced. |
| Native stop acknowledgement without a pending cancel | Enter ready with aggregate summary only. The acknowledgement asserts successful native stop finalization and disposal of all raw session data. |
| Native discard acknowledgement | Complete cancellation only for the current discard/reconciliation operation. |
| Cancel after ready | Drop the summary and enter cancelled. Native stop and discard were already acknowledged. |

Wrong-session commands, stale operation completions and completions of the wrong kind return explicit statuses without changing state. A native failure is deliberately distinct from a confirmed start rejection. A late success acknowledgement cannot resolve an operation already failed and replaced by reconciliation.

The protocol has no automatic shutdown cleanup. Destroying this object does not stop a recorder. The eventual host must retain its resource owner and callback routing until stop/discard succeeds or visibly unresolved recovery is handed to an owner. Losing a callback must leave work pending, not manufacture success. Trace configuration, categories, buffer policy, raw-data disposal and native error reporting need a reviewed host implementation before `Capability::kAvailable` is justified.

## Metadata export contract

`ExportSummary()` returns data only in ready state. Its fixed JSON schema contains a constant kind, constant browser-process scope, elapsed microseconds, process count and event count. The three counters are unsigned integers. Tests include zero and the full representable counter value without narrowing. There is no title, URL, origin, page/Profile/window ID, process ID, filename, user label, event argument, raw byte buffer or arbitrary string input.

This is an allowlisted aggregate metadata contract, not generic redaction. The native host must supply measured aggregates with the stated meaning. Encoding arbitrary content into numbers violates the contract. Even measured counts can reveal activity, so export remains an explicit caller action. No file is written, uploaded or sent to AI. There is no account or sync dependency.

`AcknowledgeStop` and `AcknowledgeDiscard` are trusted native reports, not proof that this library deleted native data. A future host must use a controlled sink, avoid retaining raw trace content, and verify final disposal before acknowledging. Do not feed arbitrary trace JSON or page content to this component and describe the resulting metadata as a redacted trace. A downloadable raw trace needs a separate reviewed content/configuration/export policy and native evidence; it is outside this slice.

## Source evidence

Read directly through Gitiles HTTP with base64 decoding on 2026-10-23. Chromium is pinned to `155.0.8059.79`, commit `a49e2e3d4addfd84e765b52afe230bedb81096db`, matching the source ledger. The web reader could not open Gitiles; direct read-only HTTP succeeded. No source checkout, build preparation or browser execution was performed.

1. [browser_commands.cc, `OpenTaskManager`, lines 2687 onward](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/browser_commands.cc#2687) dispatches to `chrome::ShowTaskManager(browser, start_action)` on desktop. This is a candidate existing command route, not a command wired by this component.
2. [task_manager_view.cc, lines 127 to 180](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/task_manager_view.cc#127) reuses the singleton task-manager view, selects the active tab row and uses the invoking browser's native window as dialog context. [Lines 692 to 696](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/task_manager_view.cc#692) obtain active WebContents from that browser's tab strip. This selection is not a page/process isolation filter. Its BrowserWindowInterface coupling still needs the host extraction from the pinned design. The macOS branch at lines 800 onward is conditional; no Cocoa/Views parity is inferred.
3. [task_manager_interface.h](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/task_manager/task_manager_interface.h) exposes a task-manager instance, process-tree rows, tasks sharing a process, title and Profile-name getters, and a WebContents-to-task lookup. Native IDs are not lifecycle IDs. The export surface intentionally offers no getters for those identifying strings.
4. [tracing_controller.h](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/public/browser/tracing_controller.h) specifies browser UI-thread calls/callbacks, asynchronous tracing across processes and asynchronous stop/flush. Its endpoints receive raw trace chunks and final completion. It offers string/file endpoints and a privacy-filtering flag, but those declarations establish no general secret-redaction guarantee. This component uses none of those endpoints and exports no native trace.
5. [tracing_controller_impl.cc, lines 230 to 277](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/browser/tracing/tracing_controller_impl.cc#230) can reject start or update process-filtered tracing already in progress. The eventual host must arbitrate ownership and avoid reconfiguring another client's session. [Stop, lines 280 onward](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/browser/tracing/tracing_controller_impl.cc#280), may reject requests. [Lines 339 to 378](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/browser/tracing/tracing_controller_impl.cc#339) route tracing failure through `CompleteFlush`, which also invokes endpoint finality. Endpoint finality alone cannot establish successful recording. Extracting reliable success/failure and disposal evidence remains native work; blindly treating `ReceivedTraceFinalContents()` as `AcknowledgeStop` would violate this contract.
6. Helium's [custom-keyboard-shortcuts.patch, lines 382 to 383](../../../../patches/helium/core/custom-keyboard-shortcuts.patch#L382) retains the task-manager shortcut label. [context-menu.patch, lines 321 onward](../../../../patches/helium/ui/layout/context-menu.patch#L321) retains a `CanOpenTaskManager()` menu condition. These adjacent patches were read only. They are reuse leads, not proof of applied patches or command-bar integration.

## Focused checks

Run from the repository root using the installed compiler. Explicit `--driver-mode=g++` follows the existing lifecycle check documented for this T3 environment.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/browser_diagnostics/diagnostics.h \
  arcified/native/features/browser_diagnostics/diagnostics.cc \
  arcified/native/features/browser_diagnostics/diagnostics_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/browser_diagnostics/diagnostics.cc \
  arcified/native/features/browser_diagnostics/diagnostics_test.cc \
  -o /tmp/arcified-browser-diagnostics-test
/tmp/arcified-browser-diagnostics-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/browser_diagnostics/diagnostics.cc \
  arcified/native/features/browser_diagnostics/diagnostics_test.cc \
  -o /tmp/arcified-browser-diagnostics-sanitized
/tmp/arcified-browser-diagnostics-sanitized
```

The behavioral fixtures exercise browser-wide task-manager planning across Profile Space selection, unavailable capability and closed-window refusal; explicit start/stop phases; exact metadata schema; wrong session/foreign action/stale callback rejection; cancel during start, recording and stop; confirmed start rejection; failed native cleanup and delayed retry completions; native service failure; metadata cancellation; and cleanup after invocation-window closure. These are synthetic protocol reports, not a simulated recorder or native deletion test.

Local results on 2026-10-23, x86_64 Linux, Clang and clang-format 23.1.3: formatting check exited 0; strict C++17 compilation and the behavioral executable exited 0; AddressSanitizer/UndefinedBehaviorSanitizer compilation and execution exited 0 with no diagnostics. Both executables printed `browser diagnostics behavioral checks passed`. This is standalone local C++ evidence, not an installed Linux browser result. Independent review belongs to the integrating coordinator and is still pending.

## Native acceptance still open

Installed Linux, Windows and macOS builds must verify native task-manager opening, command/shortcut dispatch, intended scope presentation, success/error/cancel behavior, trace ownership conflicts, raw-data handling, focus, keyboard/accessibility feedback, and output/cancellation using a public synthetic scenario. Native build/source identity and visual behavior evidence are absent here. The same actual OS window across Profile Space switches still requires genuine ownership extraction and authentication-isolation proof. No interface, endpoint declaration, local compile or protocol fixture closes those gates.
