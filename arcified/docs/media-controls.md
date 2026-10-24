# Background media controls

Issue [115](https://github.com/Timpan4/helium-arcified/issues/115) requires source-aware background media controls on Linux, Windows and macOS. This slice provides a reusable, standalone C++ projection and route planner. It does not bind Chromium objects, dispatch commands or establish native acceptance.

## Ownership and observation boundary

The lifecycle model remains the sole owner of page, Profile, Space and window identity. Media rows resolve `PageId` through a fresh model lookup. They never infer identity from a URL, title, row index or the selected Space. Multiple media pages remain separate, including identical destinations in different Profiles. Returning to a source plans a route to its current attachment. It does not transfer a saved page, open a second instance, select a Space, focus a window or reload a sleeping page.

The agreed `TabStatus` observation API owns loading, crash, sleeping, audio, mute and capture signals. Media controls consume it read-only. Caller-owned media session observations describe current native session identity, source name, playback and supported play/pause actions. They are inputs to a projection, not another tab-status store or ownership registry.

The native registry is the only document and WebContents replacement authority. The host must invalidate tab status at replacement and reject obsolete callbacks. It must verify that every supplied media observation belongs to the currently registered document/WebContents and Profile, then repeat this check before native actuation. A media session identity is not a document token. This standalone component cannot detect a same-URL reload, an obsolete callback relabeled with a current page ID, or a caller reusing an old snapshot as fresh input.

Output mute and capture controls are different operations. Muting tab output must not claim to mute a microphone, stop capture or end a call. This component provides no camera, microphone, hang-up or stop-capture dispatch.

## Local behavior and checks

`ProjectMediaPage` returns a row or an explicit refusal. `ProjectMedia` considers all current model pages without filtering to the selected Space or window and omits unresolvable sources. Its order follows model identity order, not user preference or playback priority. Capture-only, audible and muted pages qualify without a media session. A current session also qualifies, including a paused session. Unobserved signals stay unknown.

Play is offered only for a paused, controllable session advertising play. Pause requires a playing, controllable session advertising pause. Output mute requires a known mute state. Both crash and sleep must be explicitly off before playback or output controls appear. Sleeping, crashed and unknown readiness retain a Return route and report playback as unknown. Return plans no wake or reload. Source names come from host-filtered display metadata; the typed source IDs let a later native view resolve the actual tab, Space, Profile and window labels without treating names as identity. English control names are reference strings, not localized native resources.

`PlanMediaAction` compares the requesting row's complete `PageStatus` snapshot with a fresh read. It then checks current media session identity and capabilities. A transferred source, changed signal, changed Profile expectation, ended/replaced session or revoked capability is refused. Empty media session IDs, duplicate session IDs across pages, multiple sessions for one page and mismatched immutable Profiles are refused. Plans carry owned source snapshots and explicit actions, never ambiguous toggles. `RevalidateMediaPlan` repeats local planning with fresh inputs immediately before host actuation. Passing it does not reserve a page or acknowledge a native operation. Native failure/cancellation records no speculative playback, mute, focus or lifecycle success here.

The focused [synthetic fixture](../native/features/media/media_controls_test.cc) uses reserved `.test` destinations and synthetic IDs. It checks multiple sessions plus capture-only media, identical URLs across Profiles, background Spaces/windows, accessible control text, transfer to a current attachment, stale Profile/signal snapshots, malformed/duplicate/replaced/ended sessions, revoked capabilities, sleeping/crashed/unknown signals, same-URL navigation with mandatory host invalidation, closed saved pages and reopen under a fresh page ID. It also checks that planning leaves selection, navigation, capture/mute observations and unknown/protected lifecycle activity unchanged. These tests do not inject native callbacks or dispatch native actions.

Run from the repository root with the existing compiler and formatter:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/media/media_controls.h \
  arcified/native/features/media/media_controls.cc \
  arcified/native/features/media/media_controls_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/media/tab_status.cc \
  arcified/native/features/media/media_controls.cc \
  arcified/native/features/media/media_controls_test.cc \
  -o /tmp/arcified-media-controls-test
/tmp/arcified-media-controls-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/media/tab_status.cc \
  arcified/native/features/media/media_controls.cc \
  arcified/native/features/media/media_controls_test.cc \
  -o /tmp/arcified-media-controls-sanitized
/tmp/arcified-media-controls-sanitized
```

Both executions printed `media-controls synthetic checks passed`, with exit status 0. AddressSanitizer/UndefinedBehaviorSanitizer reported no diagnostics. The formatter check passed. Compiler warnings treated as errors supply the focused C++ diagnostic check; no native build target or new lint dependency was added. No repository Markdown formatter/linter was found for these docs. Whitespace and local-link checks cover the document artifact only.

## Pinned source evidence

The engine pin is Chromium `155.0.8059.79`, commit `a49e2e3d4addfd84e765b52afe230bedb81096db`, as recorded in [native Space source references](native-space-source-references.md). Individual source files were read from Gitiles, without preparing or downloading a native tree. The web reader could not open Gitiles; read-only HTTP with base64 decoding succeeded. The following observations guide the seam rather than proving runtime integration.

| Source at the pinned commit | Observed contract and consequence |
| --- | --- |
| [content/public/browser/media_session.h](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/public/browser/media_session.h) | One MediaSession per WebContents. `GetIfExists`, request-ID lookup and synchronous action queries can support a future native bridge. Projection must not create a session merely to inspect a page. |
| [services/media_session/public/mojom/media_session.mojom, lines 186 to 192 and 279 to 281](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/services/media_session/public/mojom/media_session.mojom#186) | Playback state differs from audio-focus state. Controllability and supported-action notifications are separate facts. Audible or capturing does not imply play/pause support. |
| [chrome/browser/ui/global_media_controls/media_notification_service.cc, lines 141 to 173 and 766 to 786](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/global_media_controls/media_notification_service.cc#141) | The service supports Profile source filtering and resolves active item IDs back to WebContents. The future adapter must preserve the actual Profile and explicit source rather than use the active tab. |
| [components/global_media_controls/public/media_session_item_producer.cc, lines 59 to 86 and 141 to 146](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/global_media_controls/public/media_session_item_producer.cc#59) | Session playback and actions arrive independently; the existing producer reconciles observer ordering. This planner needs a coherent current host snapshot, not a new playback cache or an invented expiry timer. |
| [content/public/browser/web_contents.h, lines 955 to 961](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/public/browser/web_contents.h#955) | `SetAudioMuted` affects output, not capture. `IsCurrentlyAudible` is a separate observation. |
| [chrome/browser/ui/tabs/tab_muted_utils.cc, lines 17 to 40](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/tabs/tab_muted_utils.cc#17) | Native tab muting records reason metadata before changing WebContents. A future adapter must preserve native mute policy and metadata rather than claim success from a plan. |
| [chrome/browser/media/webrtc/media_stream_capture_indicator.h, lines 35 to 39, 92 to 118](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/media/webrtc/media_stream_capture_indicator.h#35) | Capture categories and stop capture are explicit native operations. Native capture indicators also inform discard eligibility. Displaying a signal here must not replace those checks. |
| [Helium toolbar-button-prefs.patch, lines 116 to 165](../../patches/helium/ui/toolbar-button-prefs.patch) | Helium already modifies the global media toolbar visibility controller. Reuse that native presentation path during later integration; a second toolbar/controller is not supplied here. |

The guessed `chrome/browser/ui/global_media_controls/media_session_item_producer.cc`, `components/global_media_controls/public/media_session_item.cc` and `chrome/browser/ui/tabs/alert/tab_alert.cc` paths returned 404 and supply no evidence. The resolved producer path above was read successfully.

## Native acceptance remains open

No Chromium tree preparation, build, installed-browser interaction, remote CI or real browsing data was used. Linux, Windows and macOS keyboard navigation, screen-reader reachability, localization, focus, OS media controls, asynchronous failure handling and native dispatch remain unverified. Accessible control names are reusable output, not proof of accessibility in a native view.

The [ownership design](native-space-integration.md) still requires a production mechanism for multiple Profiles in the same actual OS window. This component does not establish that mechanism or bypass its Profile constraints.

Activity, cleanup and destructive-action policy remains OPEN. Projection and planning never call `ReportActivity`, `ClosePage`, `Reset` or cleanup. Unknown and protected lifecycle activity survive unchanged. Even a silent, paused or muted page is not certified quiescent. Issue 115's cleanup-protection acceptance remains blocked on the owner's policy and native enforcement evidence.
