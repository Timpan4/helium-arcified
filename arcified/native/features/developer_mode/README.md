# Developer-mode DevTools session gate

Preparatory local C++ component for [issue 39](https://github.com/Timpan4/helium-arcified/issues/39), against lifecycle contract version 3. This is executable session validation and diagnostic subscription lifetime behavior. It is not a Chromium adapter, native DevTools opening/docking implementation or installed-browser acceptance evidence.

## Public interface and ownership

`Target` contains the shared `commands::PageCommandTarget` plus host-issued native instance, document and binding identities. `ValidateTarget` delegates all lifecycle validation to `commands::RevalidatePageCommandTarget`, then checks native identity agreement, inspection permission and actual tools visibility. It never derives focus from selected Space or URL text. Split callers supply the exact pane's page.

`DevToolsSession::Start` validates before requesting a diagnostic subscription. A failed source returns no session. Each session mints an opaque `UpdateToken`, passed to the source for stamping callbacks. `AcceptUpdate` requires an immutable event target/token and separately obtained current host facts. It rejects a wrong page, Profile, attachment, native instance, document, binding or subscription. A late callback from a different subscription cannot cancel the current session. A current subscription's invalid target or revoked capability ends its session.

`End` terminally rejects updates before requesting cancellation, including during synchronous cancellation reentry. It requests cancellation once, remembers the host acknowledgement and never upgrades `kUnconfirmed` to success. Destruction requests cancellation for an active session. A new session is required after hide, close or invalidation. This object owns only its diagnostic subscription and immutable binding snapshot. The lifecycle Model remains the sole page, Profile, attachment and navigation owner. No focus registry, navigation copy, persistence or second target resolver is added.

`DiagnosticSource` and `DiagnosticSubscription` are an area-specific integration boundary for optional diagnostic update work. They are not implemented for Chromium here. Synthetic tests verify requests and acknowledgements, not that native timers, observers or frontend work actually stopped. `kConfirmedStopped` is a host assertion about this subscription only. Queued events can still arrive. On `kUnconfirmed`, the host must retain its cleanup obligation outside the released handle and expose/reconcile failure. The gate does not detach or destroy any inspected page, and does not equate a cancellation request with native DevTools before-unload or close success.

## Required native host contract

1. Resolve the focused native page/pane using the host's authoritative focus owner. Supply its shared page command target. On the serialized UI sequence, validate the native TabModel/WebContents association and real inspected Profile/BrowserContext, not a browser-wide changing Profile getter. Retain exact private/guest identity. The frontend's native Profile-selection behavior needs a separate audit.
2. Supply nonempty, non-reused native instance and document identities. Replace the instance identity when WebContents is replaced. Supply a fresh `BindingId` on every attachment rebind or transfer, including transfer away and back with the same object/document. Subscription provenance is independent of binding generation.
3. Before any native main-frame navigation commit is reported to the lifecycle model or any later diagnostic event is consumed, call `End(kNavigationCommitted)` on the old session. This includes reloads with identical URL text and same-document commits. Start a new session only from current facts if diagnostics should resume. Before binding invalidation, transfer, replacement, page destruction or Profile shutdown, end the old session synchronously. Observe native tools hide/close and end the corresponding session immediately, without waiting for another update to arrive.
4. Stamp every callback with the target and `UpdateToken` captured at subscription creation. Never replace the stamp with current target data. Obtain current native facts independently immediately before `AcceptUpdate`, then consume accepted data without interleaving target/lifecycle mutations. Use weak callback lifetime handling so no callback calls a destroyed session. Keep cancellation handles valid for the session lifetime and do not destroy/reenter destruction of the session from `Cancel`.
5. `Subscribe` runs synchronously on that sequence without reentering target/lifecycle mutation or delivering callbacks before return. On failure it leaves no subscription work behind. `Cancel` must not throw. Report confirmed stop only after that subscription's native work actually stops; route unconfirmed cleanup to host-owned recovery.

Lifecycle v3 has no navigation revision or native binding generation. It cannot independently detect a commit or transfer ABA when the caller supplies stale native facts and omits invalidation. This gate deliberately stores no navigation string copy, and URL comparison would miss same-URL reloads anyway. The navigation test proves the required invalidation sequence, not automatic detection from Model. The ABA test proves rejection with a changed host binding token. It cannot prove the host rotates or supplies that token correctly. These event-ordering obligations are native integration gates.

## Source-backed reuse

Read-only source inspection used the repository's Chromium `155.0.8059.79` pin, commit `a49e2e3d4addfd84e765b52afe230bedb81096db`, from the [source ledger](../../../docs/native-space-source-references.md). Gitiles `?format=TEXT` responses for `devtools_window.h` and `.cc` were read directly over HTTP on 2026-10-23 after the web reader could not open Gitiles. No source archive/preparation, patch application, dependency addition or Chromium build occurred.

| Source | Observed reuse point and limit |
| --- | --- |
| [devtools_window.h, lines 121 to 159](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/devtools/devtools_window.h#121) | Lookup and open overloads accept inspected WebContents explicitly. A future adapter should preserve exact pane targeting rather than substitute the current browser tab. Merely having an overload does not prove docked integration. |
| [devtools_window.cc, lines 637 to 647](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/devtools/devtools_window.cc#637) | Existing frontend lookup compares inspected WebContents. This component does not duplicate that registry. |
| [devtools_window.cc, lines 857 to 882](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/devtools/devtools_window.cc#857) | The native path derives frontend Profile from BrowserContext, preserves certain OTR variants and otherwise uses original Profile. The shared target must still describe the inspected page's exact real Profile. This source does not prove downstream private/guest behavior or same-window multi-Profile ownership. |
| [devtools_window.h, lines 343 to 355](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/devtools/devtools_window.h#343), [devtools_window.cc, lines 1724 to 1735](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/devtools/devtools_window.cc#1724) | Native close has a frontend/before-unload lifecycle and distinct docked/undocked paths. Diagnostic cancellation is not native close acknowledgement. |
| [multi-contents-view.patch, lines 169 to 178](../../../../patches/helium/ui/multi-contents-view.patch#L169) | Existing DevTools WebView construction takes `browser_view->GetProfile()`. This is a concrete docking integration constraint. It does not establish safe mixed-Profile presentation. |

The [native integration design](../../../docs/native-space-integration.md) requires extracting actual OS-window ownership from Profile-bound browsing contexts. This gate does not provide that extraction or make a view-only adapter sufficient. Shared page targets and this feature's native identities are preparatory inputs to that future work.

## Dispositions and remaining acceptance

| Brief requirement | Disposition in this slice |
| --- | --- |
| Correct tab/split target and Profile | Local validation and subscription targeting implemented and tested. Native focused-pane resolution, real Profile binding and actuation remain open. |
| Dock/undock, view-source and keyboard shortcuts | Reuse candidate native DevTools targeting identified. No commands, shortcuts or docking UI wired. Installed native proof remains open. |
| Hide/close cleanup | Terminal update rejection, exactly-once cancellation request and honest acknowledgement implemented. Native hide observation, stop execution, cancelled frontend close and recovery remain open. |
| Full URL and site-specific diagnostics | No UI or copied evidence path implemented. Credentials, query/fragment secrets, headers and page content are never read or copied here. A future evidence feature must redact secrets before copying; this slice makes no redaction claim. |
| Profile/origin opt-in and localhost activation | Deferred. No automatic localhost activation, hostname heuristic or preferences are added. A future opt-in must use authoritative Profile plus full origin, including scheme/host/port, and distinguish local servers on different ports. Native origin identity cannot be inferred from lifecycle URL strings. Persistence and activation policy remain open. |
| JSON readability | Deferred pending inspection of the pinned native JSON facilities. No formatter, response interception, extension installation or parity claim. |
| Portrait capture | Deferred pending inspection of native device emulation/capture and the separate capture area. No dimensions, capture/export behavior or parity claim is invented. No extension is installed. |

Linux, Windows and macOS installed native builds must prove the focused-pane open/dock/undock/shortcut interaction, Profile isolation, same-window Space switching, saved-page transfer/replacement/navigation races, hide/close cancellation and actual background-work shutdown. Focus/accessibility, restart/persistence and native failure evidence are unavailable in this slice. No issue should close from these synthetic checks.

## Focused checks

Fixtures use synthetic IDs and `.test` destinations, with no real browsing data, local-server connection or AI access. They cover exact pane identity despite matching URL text; wrong identity, denied capability and hidden tools; stale current facts and separately stale event stamps; lifecycle transfer/closure; subscription failure; confirmed/unconfirmed cleanup, reentry and destruction; required same-URL navigation invalidation; late subscription callbacks after reopen; and transfer ABA with a host binding generation. They drive public APIs and observe the requested target, acceptance result, cancellation reason and acknowledgement.

In the integrated tree, from repository root:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/developer_mode/devtools_session.h \
  arcified/native/features/developer_mode/devtools_session.cc \
  arcified/native/features/developer_mode/devtools_session_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic -I . \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/commands/command_target.cc \
  arcified/native/features/developer_mode/devtools_session.cc \
  arcified/native/features/developer_mode/devtools_session_test.cc \
  -o /tmp/arcified-developer-mode-test
/tmp/arcified-developer-mode-test
```

During this shared-area implementation, commands lives in the verified read-only peer checkout. The actual focused compile/run uses that source without copying it:

```sh
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I . -I /home/timpan4/.t3/worktrees/helium-arcified/feat-commands-workstream \
  arcified/native/model/lifecycle.cc \
  /home/timpan4/.t3/worktrees/helium-arcified/feat-commands-workstream/arcified/native/features/commands/command_target.cc \
  arcified/native/features/developer_mode/devtools_session.cc \
  arcified/native/features/developer_mode/devtools_session_test.cc \
  -o /tmp/arcified-developer-mode-test
/tmp/arcified-developer-mode-test
```

Repeat that command with `-g -fsanitize=address,undefined -fno-omit-frame-pointer` and output `/tmp/arcified-developer-mode-sanitized`, then run the resulting binary for the focused sanitizer check. Static analysis uses `clang-tidy --checks='clang-analyzer-*,bugprone-*'` on this feature's `.cc` and test with `-- -std=c++17 -I . -I /home/timpan4/.t3/worktrees/helium-arcified/feat-commands-workstream`. The initial clang-tidy invocation reported `Error: no checks enabled`; the explicit check selection enables actual analysis without changing project or host configuration. The explicit Clang driver mode follows the existing lifecycle check's host linker requirement. These are standalone Linux compiler checks, not a Chromium build or Windows/macOS runtime result.

Recorded results on 2026-10-23: the peer-dependent strict Clang compile/run, ASan/UBSan compile/run and Chromium format check passed. Both binaries printed `developer-mode session behavioral checks passed`. Clang-tidy completed with exit 0 and test-fixture warnings for potentially throwing static string/ID initialization, optional access after the custom assertion helper, and exceptions escaping the standalone test runner. It reported no diagnostic in `devtools_session.cc`; this is not a warning-free test-lint claim. The fixtures follow the existing lifecycle standalone harness style. Independent review and integrated-tree/native checks remain with the coordinator.
