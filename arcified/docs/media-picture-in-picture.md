# Picture-in-picture source binding

Status: reusable standalone C++ action planner for [issue 114](https://github.com/Timpan4/helium-arcified/issues/114), 2026-10-23. Native wiring and acceptance remain open. This slice implements source binding, action targeting and stale-plan refusal. It does not create a PiP window, controller, renderer, audio stream or media session.

The [publication brief](research/arc-dia/publication/pip.md) and `pip` entry in [backlog.json](research/arc-dia/backlog.json) require controls to keep targeting the source through tab, Space and window changes. The implementation uses [lifecycle contract v3](lifecycle-contract.md), without changing ownership, focus, navigation, persistence or activity policy. The [native ownership design](native-space-integration.md) and its [pinned references](native-space-source-references.md) still block production same-native-window, multi-Profile acceptance. Logical model transitions do not resolve that dependency.

## Local behavior

[picture_in_picture.h](../native/features/media/picture_in_picture.h) and [picture_in_picture.cc](../native/features/media/picture_in_picture.cc) expose three functions in `arcified::media`:

1. `BindPictureInPictureSource` validates an observed native video PiP session against an existing lifecycle page and its immutable Profile. It returns owned references, `PictureInPictureSource`, containing only the native session token, `PageId` and `ProfileId`. Empty identities, missing pages, Profile mismatches and document PiP are refused.
2. `PlanPictureInPictureAction` checks that binding against a fresh optional native session observation and looks up the page's current attachment in `Model::pages()`. It returns an owned plan for toggle play/pause, close surface only, close surface and pause, or return to source. No selected-tab, URL, title, tab-index or selected-Space fallback exists. Unavailable play/pause remains unavailable even when a prior plan allowed it.
3. `RevalidatePictureInPictureAction` replans against the current model and a new native observation. It refuses an ended/replaced session, source or Profile mismatch, missing page, lost capability or changed attachment. A moved source can be explicitly replanned with the same binding and session token. An old plan cannot focus the old attachment.

Space selection, including selection of a different Profile's Space in the same logical window, leaves the binding valid and never retargets actions to that Space. A successful handoff of the same page and native session retains the token; a fresh plan uses the new window and Space. Keeping a temporary page also retains its PiP source identity. Closing a page and reopening its saved item with a fresh `PageId` leaves the old binding invalid, even when its saved destination is unchanged.

Both close intents remain explicit. The component selects no default playback behavior. A close plan addresses the native PiP surface and never calls lifecycle `ClosePage`. Planning, binding and revalidation accept only a const model. They cannot mark a page quiescent, alter activity, approve cleanup, move focus, navigate or mutate ownership. Unknown and protected activity remain unchanged, including after a close-and-pause plan. Planning a native pause is not evidence of actual pause or safe page destruction.

Document PiP is explicitly refused. Its child WebContents and different control semantics must not be presented as this video-control implementation.

## Pinned native evidence and reuse

The engine pin is Chromium `155.0.8059.79`, resolved to `a49e2e3d4addfd84e765b52afe230bedb81096db` by the existing [source identity record](native-space-source-references.md#source-identity). The following files were read from that exact commit using read-only Gitiles HTTP on 2026-10-23. The web reader returned an inaccessible-URL error, so `curl` plus base64 decoding supplied the source text in memory. No Chromium checkout, archive, source preparation or native build was performed.

| Source | Observation and integration consequence |
| --- | --- |
| [picture_in_picture_window_controller.h](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/public/browser/picture_in_picture_window_controller.h), declarations for creation, close, focus and `GetWebContents` | Controllers are associated with source WebContents. Reuse the existing controller and source association. Do not create another player or infer a source from the active tab. |
| [video_picture_in_picture_window_controller.h](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/public/browser/video_picture_in_picture_window_controller.h), command declarations | Native video PiP supplies `TogglePlayPause`, `Play` and `Pause`. The adapter uses those capabilities rather than a second playback engine. |
| [video_picture_in_picture_window_controller_impl.cc, lines 104 to 125](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/browser/picture_in_picture/video_picture_in_picture_window_controller_impl.cc#104) | `FocusInitiator` activates the source WebContents. Return closes PiP without pausing and focuses that initiator. Native host/Space presentation and Profile correctness still require the ownership integration. |
| [Same implementation, lines 225 to 280](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/browser/picture_in_picture/video_picture_in_picture_window_controller_impl.cc#225) | Toggle uses the native player's current state. Play/pause routes to the existing MediaSession handler or existing player remote. The planner stores no playing/paused state. |
| [Same implementation, lines 291 to 348](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/browser/picture_in_picture/video_picture_in_picture_window_controller_impl.cc#291) | Native entry checks embedder enablement and actual entry success. A new session can replace `active_session_` while retaining the window/controller. Therefore `PageId`, controller pointer and surface presence alone cannot validate a queued action. |
| [Same implementation, lines 389 to 398, 662 to 708 and 733 to 744](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/browser/picture_in_picture/video_picture_in_picture_window_controller_impl.cc#389) | Service deletion, WebContents destruction and leaving PiP end native state. Close optionally pauses the existing player and shuts down its PiP session. Play/pause presentation depends on native handler/button state. Observations and capability must be invalidated or refreshed by the adapter. |
| [picture_in_picture_window_manager.cc, lines 100 to 115](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/picture_in_picture/picture_in_picture_window_manager.cc#100) | The video observer closes PiP on primary-page change and WebContents destruction. Do not promise PiP survives document replacement. Rotate the adapter token for any subsequent session. |
| [Same manager, lines 194 to 247](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/picture_in_picture/picture_in_picture_window_manager.cc#194) | Native UI exit distinguishes close only, close and pause, and close and focus opener. It also notifies the auto-PiP helper of user closure. Preserve that path and its side effects when dispatching an explicit intent. |
| [Same manager, lines 765 to 778](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/picture_in_picture/picture_in_picture_window_manager.cc#765) | Native auto-PiP permission UI can allow, block or close the surface. This planner grants no permissions and does not replace that UI. |

A local patch search for PiP/control names found only adjacent disposition handling in [add-flag-to-convert-popups-to-tabs.patch](../../patches/ungoogled-chromium/add-flag-to-convert-popups-to-tabs.patch). Its PiP disposition remains separate from popup conversion. That patch was read; the search is not an audit of every patch body or proof of a prepared tree.

The eventual native dispatch mapping is source-backed, but is not wired by these files:

| Planned action | Existing native operation |
| --- | --- |
| `kTogglePlayPause` | Source `VideoPictureInPictureWindowController::TogglePlayPause()`. |
| `kCloseSurfaceOnly` | Manager `ExitPictureInPictureViaWindowUi(UiBehavior::kCloseWindowOnly)`. |
| `kCloseSurfaceAndPause` | Manager `ExitPictureInPictureViaWindowUi(UiBehavior::kCloseWindowAndPauseVideo)`. |
| `kReturnToSource` | Manager `ExitPictureInPictureViaWindowUi(UiBehavior::kCloseWindowAndFocusOpener)`, after validating the source's current host/context presentation route. |

The manager targets its current controller. The adapter must verify that its source WebContents and session match the plan immediately before any manager call, on the same serialized UI sequence. Calling it with an old binding would otherwise risk closing the replacement session.

## Native adapter contract and open integration

1. Join lifecycle `PageId` and exact `ProfileId`, including private-context distinctions, to the authoritative native page registry. Resolve the current TabModel/WebContents and native Profile/BrowserContext afresh. No second registry or cross-child targeting API is introduced here; this uses the same lifecycle ID/lookup contract as other media features.
2. Observe actual successful native video PiP entry before binding. Supply a nonempty, never-reused adapter token for that native session. Rotate it on player, document or WebContents replacement, even when the logical `PageId`, controller or overlay window survives. Retain it only while the same session survives a same-page handoff. This code does not allocate tokens or observe Chromium callbacks.
3. Publish a fresh `NativePictureInPictureSession` only while the native session and surface are live, enabled and usable. Supply `std::nullopt` on session end, destruction, revoked availability or failed entry. Derive `can_toggle_play_pause` from native control availability. Do not infer permissions, availability or quiescence from audio, visibility, URL or stored metadata.
4. Preserve the binding captured by each originating UI action. Obtain a current observation when planning and another when revalidating immediately before dispatch. Resolve and validate actual native weak handles, session, Profile, current attachment and native host against the plan. Serialize those checks with actuation. The standalone model cannot prove native/model agreement or prevent a native race by itself.
5. A changed attachment invalidates an existing plan. Explicit replanning can target the retained page in its new attachment. Return must present that source context and activate its actual current window, never transfer/reopen the page merely to satisfy a stale window target. Do not speculatively call `SelectSpace` and claim native focus succeeded.
6. Dispatch through the existing native controls. Native failure or cancellation reports failure and commits no lifecycle transition. Keep native permissions, capture protections, primary-page-change closure, media-key routing and auto-PiP user-close notification. No second audio stream, media-session ownership or renderer is required by the planner.

These are integration obligations, not implemented Chromium callbacks. In particular, the native session/controller API does not expose the new adapter token for free. The native bridge must observe replacement and own that association. The tests supply synthetic observations; they do not prove observation fidelity, native actuation, renderer continuity or the absence of duplicate audio.

Activity, cleanup and destructive-page policy remain OPEN. Native window ownership is blocked as recorded for issue 72 and the pinned ownership design. Linux, Windows and macOS still need installed-build evidence for source identity and Profile isolation, handoff survival, stale UI events, fullscreen/display changes, return focus, permission/capture behavior, keyboard/OS media controls and accessibility. No native acceptance criterion for issue 114 is closed by these checks.

## Focused synthetic checks

[picture_in_picture_test.cc](../native/features/media/picture_in_picture_test.cc) drives the public planner with lifecycle v3 and synthetic `.test` destinations. Its seven fixtures cover binding validation; Space/Profile selection without retargeting; same-page handoff versus stale attachment; ended and replaced sessions; native source/Profile/kind/capability failures; closed/reopened source identity; and temporary-to-kept identity with non-destructive close intents. The replacement fixture supplies different tokens for player, document and contents replacement. It tests stale-token refusal, not real native event observation.

Run from the repository root with the existing compiler and formatter:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/media/picture_in_picture.h \
  arcified/native/features/media/picture_in_picture.cc \
  arcified/native/features/media/picture_in_picture_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/media/picture_in_picture.cc \
  arcified/native/features/media/picture_in_picture_test.cc \
  -o /tmp/arcified-pip-test
/tmp/arcified-pip-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/media/picture_in_picture.cc \
  arcified/native/features/media/picture_in_picture_test.cc \
  -o /tmp/arcified-pip-sanitized
/tmp/arcified-pip-sanitized
```

Local results: Clang warning-as-error compilation, Chromium formatting check, normal execution and ASan/UBSan execution all passed. Both executables printed `PiP source binding: 7 synthetic fixtures passed`; sanitizer execution emitted no diagnostics. Actual run artifacts are `/tmp/arcified-pip.qhxHAn/pip-test` and `/tmp/arcified-pip.qhxHAn/pip-sanitized`. These are Linux-host standalone C++ results only. No native build/download/preparation, remote CI, real browsing data, provider access, global installation, commit, push, PR, issue write or merge was performed for this slice.
