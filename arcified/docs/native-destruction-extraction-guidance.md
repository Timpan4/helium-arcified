# Native destruction authority extraction

Recorded 2026-10-23. Recommend a construction-injected destruction callback contract implemented by the existing `Browser` window owner, with final Browser deletion remaining in `BrowserManagerService`. Remove Browser-specific close, feature-teardown and deletion decisions from `BrowserWidget`. Do not introduce another Browser owner, registry or prospective shared-window framework.

A bounded source-only patch can be authored now from the retained implementation bodies. It cannot yet be called a complete, buildable or accepted lifecycle change. The factory declaration, Views destruction contracts, test bodies and GN wiring need the small pinned source follow-up below. In particular, construction-time native loss and deletion from reentrant observers are unresolved acceptance cases, not cases that a weak callback alone solves. These gaps do not require a full Chromium download before beginning the patch.

This slice preserves ordinary windows, including forced legacy close on native loss. The later target remains the same actual OS window across immutable Profile contexts, with exact same-Profile TabModel, WebContents, controller and document identity. Hidden, backing or replacement browsing windows do not satisfy that target. This extraction proves none of that later feasibility.

## Source basis

The [inventory](native-space-consumer-inventory.md) and [review](reviews/native-space-consumer-review.md) identify the extraction. This note independently traces the following retained production passages at Chromium `155.0.8059.79`, commit `a49e2e3d4addfd84e765b52afe230bedb81096db`. Coordinates refer to retained Chromium, before downstream patch application.

| Source | Observed contract |
| --- | --- |
| [BrowserWidget][widget], 103 to 132 and 394 to 406 | Destructor marks closed, closes owned widgets synchronously, then tears down Browser features before BrowserView and widget members disappear. Native loss clears the native-widget pointer, forces Browser closing, calls the Views base callback, then requests synchronous manager deletion. |
| [Window factory][factory], 31 to 84 | Chooses WebUI or Views. Views creates BrowserView, transfers the widget into it, then initializes the native widget. Later factory code still accesses the view/widget. |
| [Browser implementation][browser], 233 to 392; [header][browser-header], 267 to 311 | Browser owns `window_`, features, tabs and keepalives. Manager ownership starts after the constructor returns. Destructor forces closing when needed, resets the window, evaluates download cancellation, destroys features and sends `WindowClosed`. The weak factory is destroyed only after the destructor body. |
| [Manager implementation][manager], 35 to 177; [header][manager-header], 97 to 115 | Existing unique ownership, removal before destruction and subscriptions that survive Browser destruction. `Shutdown()` destroys extracted entries directly. OTR/printing cleanup and application-exit work belong to `DeleteBrowser()`, not every destruction path. |
| [UnloadController][unload], 109 to 188 and 942 to 983 | Forced close bypasses warnings. Delete-scheduled state prevents repeated closing after completion. Completion sets that state before observers, then computes shutdown and posts deletion using a weak Browser. |
| [BrowserView][view], 1103 to 1162 and 4248 to 4259 | Child views require their Profile during destruction. `DeleteBrowserWindow()` saves placement and resets the widget; widget destruction destroys BrowserView. Code must not use the view afterward. |
| [Feature implementation][features], 1127 to 1258 | Entire ordered pre-window hook, including conditional `BrowserElementsViews::TearDown()` before animation and actor-border teardown. |
| [Creation implementation][creation], 140 to 164 | Desktop creation is synchronous. The callback overload posts a raw Browser pointer after creation, not a weak pointer. |

Root owns the review's documentation corrections. Preserve the whole feature hook; do not reconstruct its member order from the catalog. This note does not modify retained evidence or resolve the later DevTools/session consumers.

## Selected seam

Use `Browser` because it already owns the window and features whose lifetime is being coordinated. Use `BrowserManagerService` because it already owns the Browser. The new contract communicates destruction events between those existing owners and the widget. A new object that merely stores `BrowserView*` and calls `browser_view_->browser()` would leave authority in the view relationship and fails this extraction.

Proposed API shape, with names subject to the pinned header follow-up:

```cpp
struct BrowserWindowDestructionCallbacks {
  base::OnceClosure before_view_tree_destruction;
  base::OnceCallback<base::OnceClosure()> begin_native_destruction;
};
```

`Browser` creates this move-only callback bundle after `features_->Init(this)` and supplies it to `BrowserWindow::CreateBrowserWindow`. The factory passes it to the `BrowserWidget` constructor before `InitBrowserWidget()`. Do not add a setter that can be forgotten until after native initialization. Do not obtain callbacks by finding a Browser through a native property or the active view. Keep the custom-window test restriction intact.

The first closure runs the existing feature pre-window hook against the constructing Browser's own `features_`. Consume the closure before invoking external cleanup, so reentry cannot run it twice. It remains callable during `Browser::~Browser()` until `window_.reset()` finishes. A weak binding invalidated at destructor entry would incorrectly skip this cleanup.

`begin_native_destruction` invokes a Browser-owned operation that prepares forced close and returns a one-shot completion closure. The completion requests deletion of that exact Browser through its existing manager. It must not look up whichever Browser is now presented. Construct the completion token before calling unload code, and move it to a stack local in the widget before crossing the Views base callback. After invoking completion, access neither the widget nor Browser, because completion may synchronously delete both.

The bundle is a notification contract, not a second owner. Prefer defining it at the BrowserWindow boundary rather than making `browser.h` depend on a Views widget type. Retrieve `browser_window.h` before selecting its exact declaration location. Do not add a generic observer bus or a public method to every BrowserWindowInterface implementation solely to transport these callbacks.

After extraction, the two widget destruction methods must contain no `browser_view_->browser()` lookup, `UnloadController::From`, feature lookup or manager deletion call. Other widget theme, command and layout dependencies remain for later work. Removing every Browser include is not the success criterion.

## Required order and invalidation

1. **Native loss.** Clear `browser_native_widget_` as today. Consume the native callback. The owner sets `force_skip_warning_user_on_close(true)` and calls `OnWindowClosing()`. Run `Widget::OnNativeWidgetDestroyed()` before the owner's final synchronous deletion request. A second native notification must not repeat Browser close/deletion authority. Do not move the feature hook into this notification; widget destruction has its own required ordering.
2. **Widget destruction.** Keep `set_widget_closed()`, then `ForEachOwnedWidget(... CloseNow())`, then the consumed pre-view callback. Only afterward may normal derived-member/base destruction remove widget state and child views. An owned dialog may call back while closing, so the owner, features and Profile must still exist throughout that loop. Moving the feature hook to `Browser::~Browser()` before window reset, or to `BrowserView::~BrowserView()`, is too early or too late respectively.
3. **Ordinary posted deletion.** Preserve `UnloadController` as the sole authority for delete-scheduled state, close notification and normal unload decisions. Preserve the run-loop return before ordinary final deletion. Do not add a second closed/session state machine to the callback bundle. Native synchronous deletion must leave its already-posted task harmless through invalidation, not cancel normal async semantics globally.
4. **Owner-driven destruction.** At Browser destructor entry, mark final destruction as underway before forced close can reenter. Suppress new deletion requests against that owner, while keeping the pre-view callback available through `window_.reset()`. Preserve subsequent download checks, `features_.reset()`, tab-empty check, `WindowClosed`, member destruction and keepalive release. A dedicated destruction-callback lifetime or explicit owner state can separate this cleanup permission from deletion permission. Do not indiscriminately invalidate every Browser weak pointer and assume cleanup still works.
5. **Manager deletion and shutdown.** Keep entry extraction and activation-order removal before destroying Browser; keep subscriptions alive until Browser destruction completes. Repeated deletion of an already-extracted entry remains a no-op. Direct `Shutdown()` must still reach Browser's cleanup without requiring a prior native event or posted task. Do not route shutdown through `DeleteBrowser()` merely to share code, since that would add its OTR/printing/app-exit effects to a different path.
6. **Terminal invalidation.** No final-deletion closure may reacquire a manager through a dying Profile. Consume or invalidate it before releasing Browser ownership. Pre-view cleanup expires only after window teardown. Existing Browser weak pointers must be dead when posted work can next run after final destruction. Nested event loops during the destructor need an explicit check; eventual weak-factory member destruction alone does not prove this.

The proposed names and owner state are design guidance, not APIs observed in Chromium. Do not introduce an independent `is_closed` flag that disagrees with UnloadController. Any new state should describe only ownership teardown or an active native callback, with transitions located at the corresponding owner boundary.

## Reentry is an ownership question

The manager's extraction prevents repeated destruction after deletion has begun. It does not keep a Browser alive while a native callback is still running. Likewise, the weak Browser in the posted task protects a later task, not the current widget stack.

`OnWindowCloseComplete()` invokes close observers before it finishes using `browser_`. A synchronous observer deletion can therefore invalidate the UnloadController as well as BrowserWidget. The callback proposal must not claim to fix this by checking the completion closure's weak pointer only after the base callback. That would be too late.

The implementation must establish the actual Views/observer contract from the missing sources and tests. If deletion during this bracket is permitted, protect the owner through the forced-close/base-callback bracket. The smallest suitable mechanism is a scoped deletion deferral associated with the existing manager-owned entry, with completion draining the pending request synchronously after the base callback. It must retain existing ownership and subscriptions, not copy the Browser into another registry. A Browser-local guard that `DeleteBrowser()` and `Shutdown()` never consult cannot provide this protection.

That mechanism is conditional, not permission to add a speculative manager subsystem. In particular, returning early from keyed-service `Shutdown()` and allowing Profile teardown while a deferred Browser survives is invalid. Resolve nested shutdown and the exact lifetime of such a scope before implementing it. If the platform contract prevents a given reentry, cite and test that contract instead. Until one of these routes is demonstrated, arbitrary reentrant deletion remains an acceptance blocker for the source patch.

For ordinary close-observer reentry outside the protected native bracket, capture liveness before notification and avoid member access after an observer destroys the owner. Changes here require the pinned `unload_controller.h` and its actual test fixture. Preserve normal recursive tab closing: a blanket guard around every `OnWindowClosing()` call would block its intentional close-all-tabs completion path.

## Construction and partial teardown

Callback registration must happen while Browser is still constructing. Manager lookup is not registration: `Browser::Create()` calls `AddBrowser()` only after `new Browser(...)` returns, and `DeleteBrowser()` returns if the Browser is absent. Registering callbacks only in `AddBrowser()` leaves native initialization unprotected.

The retained constructor completes `features_->Init()` before entering the window factory. Thus a widget created by that factory has the pre-window feature stage available even though post-window initialization has not run. Preserve the full existing hook, including its unconditional accesses to embedder features, bookmark-bar state, sign-in state and browser elements. Do not call it on an arbitrary half-completed `features_->Init()` merely because `features_` is non-null.

There is a concrete unresolved path after native initialization: [BrowserWidget][widget], 220 to 242, calls `Init()` and later dereferences `browser_native_widget_`; native destruction sets that pointer to null. The factory subsequently accesses widget/native state, and Browser subsequently opens the session record and initializes post-window features. Installing a weak callback before `Init()` does not make these continuations safe.

The source-only implementation must distinguish registering authority early from completing deletion early. Never delete the Browser while its constructor is on the stack. If the missing Views contract permits synchronous native loss during initialization, record the loss in the constructing owner and stop initialization-dependent work at the actual safe return boundaries. Prove cleanup ownership of BrowserView and its widget before adding early returns. Do not silently ignore the event, publish a usable dead window, run post-window initialization after pre-window teardown, or add a null-return convention without tracing callers.

This may require a construction-failure result and readiness publication change. It is not established by the retained sources alone. In particular, the desktop callback creation overload posts a raw pointer, so returning a Browser that will be deleted before that callback runs is unsafe. The [public creation contract][creation-header], 200 to 245, also needs reconciliation with any failure result. Keep this case open rather than inventing a retry, a replacement native window, early manager publication or a new failure policy in this extraction.

## Small pinned source follow-up

Read individual files at the same commit. Do not fetch a full archive or prepare a tree for this step. These are retrieval targets, not assertions of reviewed coverage or GN target names.

| Input | Purpose and provenance |
| --- | --- |
| `chrome/browser/ui/browser_window.h`, `browser_window_deleter.h` and the implementation defining `BrowserWindowDeleter::operator()` | Factory declaration, virtual deletion contract and exact deleter dispatch. The includes and observed `DeleteBrowserWindow()` supply the edge; locate the operator implementation from the pinned UI listing rather than guessing its filename. |
| `chrome/browser/ui/unload_controller.h` | Actual lifetime fields, weak factory, observer declarations and available hooks before modifying completion/reentry handling. |
| `chrome/browser/ui/webui_browser/webui_browser_window.h` and its implementation located from the pinned directory listing | The factory's alternate production branch has a separate window lifetime. Confirm that leaving this branch unchanged and dropping an unused callback bundle has no cleanup side effects, or identify its existing notification route before changing it. |
| `ui/views/widget/widget.h` and `widget.cc` | `CLIENT_OWNS_WIDGET`, `Init`, `CloseNow`, destructor and `OnNativeWidgetDestroyed()` contracts. Follow their actual native-backend calls only as needed to settle initialization and reentry. |
| `chrome/browser/ui/views/frame/browser_native_widget_factory.h` and `.cc` | Initialization factory and any injection hook for a native-loss test. Their use is explicit in the retained widget implementation. |
| `chrome/browser/ui/views/frame/browser_widget_browsertest.cc` | First test fixture to inspect for native close and widget lifecycle coverage. Pinned frame listing blob `dd521ecd5afb787f74ad5de8866ecad4faf7a770`. |
| `chrome/browser/ui/views/frame/browser_view_browsertest.cc` | Owned-view teardown, placement and ordinary-window behavior. Listing blob `2c21baca429f506dd8738977053a05b738217ed4`. |
| `chrome/browser/ui/views/frame/browser_view_focus_interactive_uitest.cc` | Inspect only if modal/child teardown needs its focus fixture. Listing blob `f749d23ce237f8878919b0f27e213f1c899c74fc`. Its name is not proof of lifecycle coverage. |
| `chrome/browser/ui/views/frame/BUILD.gn` | Production/test source wiring. Listing blob `ee117be4ec882f665722a5ed44780980edf7de3c`. |
| Pinned `chrome/browser/ui/` directory listing, then its relevant build/test entries; `chrome/test/BUILD.gn` if referenced | Locate actual Browser, manager and unload fixtures and final test targets. Do not invent filenames or a test filter from class names. Follow GN imports to the owning target only where the source lists require it. |

The frame names and blobs above come from the retained [frame listing][frame-tree]. None of these test bodies or build definitions was available in the inspected capture. If Views requires additional platform code, follow its concrete Aura/Windows/macOS destruction edges; that is a bounded source follow-up, not an exhaustive platform audit.

Review downstream hunk interaction before finishing the source patch. A header search across the local patch set found widget changes in `native-frame-materials.patch` and `add-flag-for-incognito-themes.patch`, and unload changes in `add-flag-for-close-confirmation.patch`. The inspected material hunk changes initialization; the close-confirmation patch adds warning state and callbacks. Preserve the forced-close bypass and reconcile context at its quilt position. Many other patches touch BrowserView. This header search is not patch application evidence.

## Rejected alternatives

| Alternative | Source-backed reason |
| --- | --- |
| Move the old widget statements into a helper taking BrowserView | Still derives Browser lifetime authority from the presented view. Removes no ownership dependency. |
| Put everything in manager `DeleteBrowser()` | Misses construction and direct `Shutdown()`; pre-view teardown also has a precise widget-local position. |
| Observe Widget destruction after construction | Too late for native initialization; a generic observer phase is not proven equivalent to the existing pre-member teardown point. |
| Replace synchronous native deletion with another posted task | Weakens the explicit legacy expectation in the native callback. |
| Use the normal user-close path for native loss | Permits warnings/vetoes after the native window is already gone. Preserve the forced path without making it future shared-host policy. |
| Move features or Profile retention to a new process host now | No host construction, enabled-feature closure or OTR lifetime proof exists in this slice. Adds ownership before removing the observed dependency. |
| Reuse one weak callback for deletion and pre-view cleanup | Their invalidation boundaries differ during direct Browser destruction. |

## Patch acceptance gates

1. Author the downstream source-only patch without editing retained evidence. Inspect the missing declarations and every affected construction caller before finalizing API changes. Retain WebUI and test-window behavior explicitly; the factory's WebUI branch must not silently discard required lifetime work or acquire duplicate teardown. Do not infer that WebUI is disabled.
2. Demonstrate dependency removal in the production diff. Browser supplies destruction authority at construction; widget destruction no longer recovers a Browser from BrowserView. Preserve feature-hook order, manager ownership/subscriptions, immutable Profile, session close placement and keepalives. Review all changed hunks against the preceding downstream patches.
3. Add focused behavior tests in the retrieved real fixtures. Required cases are ordinary async close, native loss with tabs, native loss after deletion was posted, repeated close/reentrant observer requests, manager shutdown/direct owned-test destruction, owned modal-widget closure before feature teardown, and native loss during initialization. Assert observable ordering, one final destruction/close notification, no stale posted access and no early Profile release. Distinguish session `WindowClosing` from `WindowClosed`; recursive tab closing already revisits the former path, so an invented blanket once-only assertion would not describe the baseline.
4. Resolve the construction and reentry gaps above in source and tests. State unsupported injections explicitly if the platform contract rules them out. A synthetic callback counter alone cannot prove BrowserView, manager, session or Profile lifetime. Test-only custom windows cannot stand in for the production Views construction route.
5. In the separately authorized prepared tree, verify quilt application, compile the actual affected targets and run the focused lifecycle tests. Record actual fixture names, filters, GN targets and platform configurations after discovery. Source-only patch inspection and application to a sparse copy do not satisfy this gate.
6. Keep native acceptance open until the affected ordinary-window interactions are checked on Linux, Windows and macOS. Full Chromium preparation/download/build, native runs, heavy work and remote CI remain owner-deferred. No new permission is needed to author the local source-only patch within this scope.

This guidance writes only this document. Local reference targets were checked for existence and whitespace was checked. No repository Markdown formatter or linter configuration was found. It ran no compilation, patch application, native test or browser interaction. It does not claim a complete partial-construction recovery design, exhaustive consumer coverage or shared-window feasibility.

[widget]: research/native-space-consumers/chromium/chrome/browser/ui/views/frame/browser_widget.cc
[factory]: research/native-space-consumers/chromium/chrome/browser/ui/views/frame/browser_window_factory.cc
[browser]: research/native-space-consumers/chromium/chrome/browser/ui/browser.cc
[browser-header]: research/native-space-consumers/chromium/chrome/browser/ui/browser.h
[manager]: research/native-space-consumers/chromium/chrome/browser/ui/browser_manager_service.cc
[manager-header]: research/native-space-consumers/chromium/chrome/browser/ui/browser_manager_service.h
[unload]: research/native-space-consumers/chromium/chrome/browser/ui/unload_controller.cc
[view]: research/native-space-consumers/chromium/chrome/browser/ui/views/frame/browser_view.cc
[features]: research/native-space-consumers/chromium/chrome/browser/ui/browser_window/internal/browser_window_features.cc
[creation]: research/native-space-consumers/chromium/chrome/browser/ui/browser_window/internal/create_browser_window_non_android.cc
[creation-header]: research/native-space-consumers/chromium/chrome/browser/ui/browser_window/public/create_browser_window.h
[frame-tree]: research/native-space-consumers/chromium/chrome/browser/ui/views/frame/_tree.json
