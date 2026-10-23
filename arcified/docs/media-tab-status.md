# Tab status projection and action routes

Preparatory component for [issue 71](https://github.com/Timpan4/helium-arcified/issues/71), based on the [publication brief](research/arc-dia/publication/tab-status.md) and `tab-status` entry in [backlog.json](research/arc-dia/backlog.json). Started at checkpoint `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. This is executable standalone C++ behavior, not a Chromium integration or native acceptance result.

## Implemented behavior

[`TabStatus`](../native/features/media/tab_status.h) accepts full signal snapshots for existing lifecycle `PageId` values. `TabSignals` represents loading, crashed, sleeping, audible, muted, microphone, camera, tab capture, screen/window capture and attention independently. Each uses `Signal::kUnknown`, `kOff` or `kOn`. A missing observation remains unknown. Muted does not mean microphone/camera muted, an inaudible page can still capture, and a crash/sleep report never erases a capture report automatically. The host must supply truthful observations, including when native sources disagree or become unavailable.

`Read(const lifecycle::Model&, const lifecycle::PageId&) const` returns `std::optional<PageStatus>`. The owned result contains `page`, `profile`, `attachment` and `signals`. Profile and attachment come from the current model on every read. Only signal observations are cached. There is no copied navigation, saved destination, ownership graph, selected page, native object registry or persistence state. A saved-page transfer retains its signal identity and the next read reports its new attachment. A closed page returns no result even before stale observations are pruned. A reopened item uses a fresh page ID and starts with unknown signals.

`Observe` returns changed, unchanged or page-not-found. It does not invoke views or layout callbacks. Hidden rows need no mounted view to keep observations current. `Invalidate` removes a page's observations; `PruneClosed` removes snapshots for pages no longer in the model. These are projection-cache operations, never lifecycle close/discard commands.

`DescribeStatus` supplies a semantic `IndicatorKind`, tri-state value and an explicit textual accessible name for every signal, including unknown/off states. Names include "Audio playing", "Tab audio muted", "Microphone in use", "Tab being captured" and "Loading status unavailable". Active selection is a fresh caller fact supplied separately to this function. It is never derived from selected Space or stored here. Semantic keys and text allow distinct shapes/labels without color. English names are reference presentation strings. Native localized resources, actual icons, accessibility nodes, keyboard handling and announcements remain unwired.

## Stable-page action validation

`PageAction` carries the expected `PageStatus` shown by the UI and an explicit `PageActionKind`. `ResolveAction` reads the model/status again. Missing pages and any changed expected Profile, attachment or signal yield explicit refusal. Mute and unmute are separate intents. A signal change cannot flip the meaning of an old toggle. A transferred row must be refreshed before another action is planned.

For route planning, mute/unmute require known matching mute state and explicitly non-crashed/non-sleeping state. Stop requires loading on and crash/sleep off. Retry requires crash on and sleep off. Unknown required facts yield `kSignalUnavailable`; sleeping yields `kSleeping`; an inapplicable action yields `kNotApplicable`. Locate can plan a route for an unknown, crashed or sleeping page without changing focus, switching a Space, waking a page or reloading it. Each supported action has a textual accessible name.

Successful resolution returns only `ResolvedPageAction {page, profile, attachment, kind}`. It does not actuate the browser or grant permission to bypass URL policy, repost confirmation, native reload restrictions or user prompts. A returned route is not a reservation, transaction or native success. No API calls `Model::ReportActivity`, changes activity, approves quiescence, resets a page or enables cleanup. Activity, destructive-action and cleanup policy remain **OPEN** under the [lifecycle contract](lifecycle-contract.md).

## Required host freshness boundary

The host's single native page registry remains the document/native-object authority described in the [ownership design](native-space-integration.md#proposed-ownership-seam). This component deliberately has no generation allocator, document identity or native handle map. `PageId` alone cannot distinguish successive documents, same-URL reloads or replaced WebContents.

1. Serialize reports, model changes and command resolution on the host UI sequence. Resolve the event's `PageId`, real Profile, current attachment and native document/object through the one registry. Reject obsolete callbacks before calling `Observe`, including callbacks from a replaced document that retains the same `PageId`.
2. Call `Invalidate` before navigation or native-content replacement, including reload, discard replacement and crash replacement. Publish a complete fresh snapshot from accepted current sources. Unsupported or temporarily unavailable values stay unknown. Do not replay old-document reports or seed a replacement with the old snapshot. The host owns recovery if an observation source disappears or a navigation fails.
3. Bind each interactive row to its `PageStatus` and the native registry's existing document identity. On activation, compare that originating identity with fresh caller-owned registry facts, actual Profile/attachment and current native capabilities. A fresh `Read` is necessary but cannot perform the document check. Matching signals do not prove the same document. Navigation away and back to identical state is intentionally not detectable here.
4. Run `ResolveAction` immediately before native actuation without yielding or retaining the plan across a queue boundary. If execution is deferred, revalidate the originating document through the registry and resolve again. Dispatch only to that resolved native page. Never fall back to a current tab, row index, matching URL/title, selected Space or selected Profile. Native failure/cancellation leaves model and observations unchanged until authoritative events report an actual outcome.
5. Read native selection when describing the active indicator. Reconcile changed observations for visible presentations and refresh hidden presentations when shown. An unchanged report need not invalidate a view. Ownership moves and selection changes still need their own model/native notifications even when signal snapshots are unchanged.

`const TabStatus&` and `Read` are the agreed read-only consumption seam for media controls. The consumer supplies its own fresh native media-session facts. Status code does not add a second playback/session store, allocate document tokens or depend on a sibling feature's API. Same-native-window production ownership across Profiles remains unproven.

## Pinned source evidence

The existing [source references](native-space-source-references.md) pin Chromium `155.0.8059.79` to commit `a49e2e3d4addfd84e765b52afe230bedb81096db`. The files below were inspected individually through Gitiles HTTP with base64 decoding of `?format=TEXT`. The web reader could not access Gitiles; direct read-only HTTP succeeded. No source tree, archive, preparation or browser build was downloaded. An initial guessed `chrome/browser/ui/tabs/tab_renderer_data.cc` path returned 404 and supplies no evidence.

| Inspected source | Evidence and integration consequence |
| --- | --- |
| [`content/public/browser/web_contents.h`](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/public/browser/web_contents.h#807), loading declarations around 807 to 833, audio around 955 to 961, crash around 978 to 981, stop at 1168 to 1169 | Loading, current audibility, tab output mute and crash are separate queries. Loading UI has its own query. Decide explicitly which native loading meaning is reported; do not label a recent-audio indicator as currently audible. Tab output mute does not mute capture. `Stop` is a page operation, not an active-tab command. |
| [`content/public/browser/web_contents_observer.h`](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/public/browser/web_contents_observer.h#409), loading around 409 to 420, audio/mute around 775 to 819, replacement around 1000 to 1017 | Load notifications need not alternate. Sample current facts rather than toggling a boolean for each event. Audio and mute have distinct notifications. Discard can replace WebContents, requiring registry reconciliation and observation invalidation. |
| [`chrome/browser/ui/tabs/alert/tab_alert_controller.cc`](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/tabs/alert/tab_alert_controller.cc#72), notifier around 72 to 86, accessible text around 215, alert reads around 294 to 307, capture around 370 to 422, audio around 457 to 465 | Native alerts retain multiple states and have localized accessibility resources. The single-alert callback compares the chosen highest-priority alert, so it is insufficient for all independent signal changes. Capture and current mute/audio must be observed at their underlying sources. Recently audible state intentionally differs from current audibility. |
| [`chrome/browser/media/webrtc/media_stream_capture_indicator.h`](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/media/webrtc/media_stream_capture_indicator.h#54), observer declarations around 54 to 67 and queries around 96 to 113 | Separate native audio/video use, source mirroring and window/display queries support truthful full capture snapshots. Here `tab_capture` means the source tab is being mirrored. Capturing another tab and being captured are different facts; this field must not silently conflate them. `display_capture` combines current window/display capture, as the native controller does. |
| [`chrome/browser/resource_coordinator/tab_lifecycle_unit_external.h`](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/resource_coordinator/tab_lifecycle_unit_external.h#26) and [`lifecycle_unit_state.mojom`](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/resource_coordinator/lifecycle_unit_state.mojom#9) | `GetTabState` exposes current ACTIVE/FROZEN/DISCARDED state. A later adapter can present discarded as sleeping, with unavailable lifecycle source remaining unknown. Frozen and discarded differ. `WebContents::WasDiscarded` around 872 to 881 describes prior discard provenance, so it alone is insufficient to certify current sleep state. This component does not invoke discard eligibility or `DiscardTab`. |
| [`chrome/browser/ui/tabs/tab_strip_model.h`](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/tabs/tab_strip_model.h#438), active queries around 438 to 442, attention setter around 466 to 468, foreground query around 525 to 526 | Native selection is a separate fact. Attention is explicitly reported to the native tab model. No initial attention getter/observer integration is implemented here; it stays unknown until the host has authoritative evidence. Split foreground and active selection must not be treated as equivalent. |
| [`chrome/browser/ui/tabs/tab_muted_utils.cc`](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/tabs/tab_muted_utils.cc#17), `SetTabAudioMuted`, and [Helium mute patch](../../patches/helium/core/mute-tab-context-menu.patch), around patch lines 64 to 85 | Reuse the native per-tab muting helper/reason bookkeeping at actuation. The existing context menu can target multiple selected indices, so forwarding an old row index to it would not establish this component's single-page targeting guarantee. |
| [`content/public/browser/navigation_controller.h`](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/public/browser/navigation_controller.h#600), reload around 600 to 608 | Native retry must resolve this page's navigation controller and preserve native reload/repost handling. Returning a retry route does not reload or validate prompt behavior. |

The inspected [Helium hibernate patch](../../patches/helium/core/hibernate-tab-context-menu.patch), around patch lines 92 to 121, uses native foreground/discard eligibility checks. Status observations never replace those protections. No patches or patch order changed.

## Focused synthetic checks

Run from the repository root with the existing compiler and formatter. These commands compile the standalone component and lifecycle model, not the native browser.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/media/tab_status.h \
  arcified/native/features/media/tab_status.cc \
  arcified/native/features/media/tab_status_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/media/tab_status.cc \
  arcified/native/features/media/tab_status_test.cc \
  -o /tmp/arcified-tab-status-test
/tmp/arcified-tab-status-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/media/tab_status.cc \
  arcified/native/features/media/tab_status_test.cc \
  -o /tmp/arcified-tab-status-sanitized
/tmp/arcified-tab-status-sanitized
```

On Linux x86_64 with Clang 23.1.3, strict compilation and both runs passed all seven fixtures; ASan/UBSan emitted no diagnostics. Chromium-style formatting passed. The fixtures cover unknown states, every independent signal transition and textual status, hidden-Space reporting/deduplication, invalidation before a synthetic navigation, explicit stale Profile/attachment/signal refusals, cross-Profile selection without retargeting, saved-page transfer, identical-URL identity separation, action gating, closed/reopened pages and unchanged lifecycle activity/navigation/ownership. They never connect to a website, use personal browsing data or dispatch native operations.

Hidden-view evidence is limited to observing and reading a page while its source Space is unselected, with no view callbacks in the API. No rendering/layout cost, frame timing, memory use or performance improvement was measured. Synthetic status and action-name strings do not prove screen-reader or keyboard accessibility.

## Native gaps and blockers

The native observation bridge, registry/document checks, native action dispatch, localized sidebar views, actual hidden-folder updates/layout behavior, failure/repost dialogs, focus/locate interaction and installed accessibility behavior remain unwired. There is no GN target, quilt patch or production browser integration in this slice. Linux, Windows and macOS native interaction, visual, persistence and platform failure evidence remain open. The same-window multi-Profile host gate remains open under the pinned ownership design. No native acceptance or issue closure follows from these standalone checks. Native preparation/builds, remote CI, global installs and publication remain deferred.
