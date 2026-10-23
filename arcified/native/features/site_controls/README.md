# Site control command targeting

Preparatory local C++ slice for [issue 38](https://github.com/Timpan4/helium-arcified/issues/38), brief key `site-controls`. This directory does not install controls in Chromium or satisfy the issue's installed-browser acceptance criteria.

`PrepareCommand` captures an explicit native target for Page Info, site settings or print. `RevalidateCommand` checks a captured action against fresh host facts before native dispatch. Both return specific refusals. Neither function executes a command, resolves an origin, reads permissions, changes focus or mutates lifecycle state. No object retains an active page or owns navigation.

`Target::page_target` is the shared `arcified::commands::PageCommandTarget`. The shared `RevalidatePageCommandTarget` supplies the sole page/Profile/attachment validation for both captured and fresh targets. This area maps its errors to refusals and separately checks selected Space, native identity, capability and anchor facts. It implements no competing lifecycle resolver. The shared contract cannot detect native replacement or transfer away and back; host generation tokens remain necessary.

## Native contract

The caller supplies native selection, document, origin, security-state and surface identities. All tokens are opaque host identities, never URL text, formatted origin text, tab indices or persistent identifiers. The host retains the actual objects and origin values. A token cannot establish those facts by itself.

The host must reconcile lifecycle state and the real TabModel/WebContents/Profile binding, including object replacement, before supplying facts. Model Space selection can reject a wrong Space. It cannot establish which native pane has focus. Two pages can have identical destinations while requiring different command targets. Reloading the same URL can replace a document. A visible security-state change can invalidate a Page Info request without changing its origin or document.

Selection identities change when the command's native selected-page context changes, including a switch away and back. Surface identities change when the relevant native presentation binding changes. Document, origin and security-state identities follow authoritative browser events, including document replacement and visible-security updates. IDs are nonempty and never reused for different generations. The host supplies a fresh context at actuation, rather than returning the earlier captured record.

Every command needs explicit native availability for the exact target. An absent capability refuses. Native policy, guest/OTR restrictions, controller readiness, build flags, and whether a given surface supports a command belong to the host. This library does not turn a missing capability into a fallback on another page or Profile.

Page Info also requires an anchor identity. It represents a live native view configuration or the supported fallback rectangle and its native parent. The host binds it to the selected page/surface and renews it when the native anchor configuration changes. An unavailable or replaced anchor refuses. Site settings and print do not require a Page Info bubble anchor, but still require the selected native surface and their native browser/dialog ownership. This is not permission to bypass their existing native checks.

Preparation is not authorization or a reservation. The caller serializes revalidation and dispatch on its native command sequence, keeps the objects alive, and resolves the exact native target again before executing. Do not dispatch through a stale Browser's active tab or cached Profile. Revalidate after an asynchronous gap or reentrant callback. On refusal, discard the request and refresh the UI. On native cancellation/failure, report that result without marking any lifecycle transition successful.

Existing native bubble observers must continue handling page/security changes, hidden/destroyed contents, anchor destruction, and tab movement after dispatch. A pre-dispatch check does not replace those observers. This component neither displays security indicators nor invents permission/capture state, and does not authorize permission decisions or print content.

## Inspected source

The repository pins Chromium `155.0.8059.79`, resolved in the [source ledger](../../../docs/native-space-source-references.md) to `a49e2e3d4addfd84e765b52afe230bedb81096db`. The following immutable files were fetched directly from Gitiles with `?format=TEXT` and decoded in memory on 2026-10-23. This was read-only source inspection, without a prepared tree or patch-application claim.

| Source | Observation and consequence |
| --- | --- |
| [page_info_bubble_view.cc, line 395](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/page_info/page_info_bubble_view.cc#395) | `ShowPageInfoDialogImpl` takes explicit WebContents and a native virtual URL, resolves the anchor configuration or fallback rectangle, and obtains the native parent. Tokens must retain that native association; this component never reconstructs a virtual URL or origin. |
| [bubble_anchor_util_views.cc, line 35](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/bubble_anchor_util_views.cc#35), [line 180](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/bubble_anchor_util_views.cc#180) | Anchor resolution uses native BrowserView/location-bar state. The no-toolbar fallback uses native screen geometry, including RTL and display origin. No fabricated coordinates or generic unanchored Page Info fallback is supplied here. |
| [page_info_bubble_view_base.cc, line 62](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/page_info/page_info_bubble_view_base.cc#62), [line 79](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/page_info/page_info_bubble_view_base.cc#79) | A view anchor supplies its widget parent, including the macOS immersive overlay. The base observes primary-frame deletion, hidden contents, primary-page/security changes and contents destruction and closes the bubble. These native lifetime rules remain required after validation. |
| [page_info.cc, line 864](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/page_info/page_info.cc#864), [chrome_page_info_delegate.cc, line 363](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/page_info/chrome_page_info_delegate.cc#363) | Site settings uses the native Page Info site URL and resolves its browser from the associated WebContents. It also has an app-management route. It is not an arbitrary URL-string command. |
| [chrome_page_info_ui_delegate.cc, line 157](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/page_info/chrome_page_info_ui_delegate.cc#157), [line 310](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/page_info/chrome_page_info_ui_delegate.cc#310), [line 361](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/page_info/chrome_page_info_ui_delegate.cc#361) | Guest availability, permission-controller results and Profile derivation are native responsibilities. This library supplies no default permission state or guessed availability. |
| [browser_commands.cc, line 2427](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/browser_commands.cc#2427) | Print is build-conditional and reads the browser's active WebContents and Profile preference. A future executor must prove those native objects match the explicit target before using that route. A Page Info anchor is not part of this call. |
| [permission_prompt_desktop.cc, line 26](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/permissions/permission_prompt_desktop.cc#26) | Prompt browser ownership is recovered from the contents' TabInterface and can change after a tab move. Tab switching destroys the prompt while retaining the pending request. This code does not replace permission-prompt ownership with its selected-page command snapshot. |
| [helium-noise-page-info.patch](../../../../patches/helium/ui/helium-noise-page-info.patch), patch lines 532 onward | Helium extends Page Info with a Noise subpage and native announcement. The eventual integration must preserve native subpages and their security/settings behavior. This slice does not apply or modify the patch. |

The [integration design](../../../docs/native-space-integration.md) and lifecycle contract version 3 remain authoritative for ownership. The older integration document's unapproved Profile/Space wording predates the accepted v3 semantics; this slice uses the current contract. Same-window multi-Profile production feasibility remains unproven and requires ownership extraction.

## Focused checks

On 2026-10-23, Clang and clang-format `23.1.3` on Linux passed the following checks. Both executable runs printed `site-controls: 9 behavioral fixtures passed`. The sanitizer run emitted no diagnostics. These are synthetic C++ checks, not native browser tests or Windows/macOS compiler results.

The fixtures prove accepted command snapshots leave lifecycle ownership/navigation/activity unchanged; missing native selection and empty identity refusals; shared target error propagation; capability loss and unavailable/replaced anchors; same-URL reload, origin/security/presentation changes; distinct same-URL split targets; saved-page transfer and move-away/back generation invalidation; Profile-switch isolation of command targets; and page-close invalidation while the saved item survives. They supply synthetic native facts rather than proving a real host produces them correctly.

The shared commands component is not yet in this checkout. Local validation used its verified read-only source from the separate commands worktree, without copying or editing it. From this repository root, these exact commands passed:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/site_controls/command_target.h \
  arcified/native/features/site_controls/command_target.cc \
  arcified/native/features/site_controls/command_target_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I . -I /home/timpan4/.t3/worktrees/helium-arcified/feat-commands-workstream \
  arcified/native/model/lifecycle.cc \
  /home/timpan4/.t3/worktrees/helium-arcified/feat-commands-workstream/arcified/native/features/commands/command_target.cc \
  arcified/native/features/site_controls/command_target.cc \
  arcified/native/features/site_controls/command_target_test.cc \
  -o /tmp/arcified-site-controls-test
/tmp/arcified-site-controls-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I . -I /home/timpan4/.t3/worktrees/helium-arcified/feat-commands-workstream \
  arcified/native/model/lifecycle.cc \
  /home/timpan4/.t3/worktrees/helium-arcified/feat-commands-workstream/arcified/native/features/commands/command_target.cc \
  arcified/native/features/site_controls/command_target.cc \
  arcified/native/features/site_controls/command_target_test.cc \
  -o /tmp/arcified-site-controls-sanitized
/tmp/arcified-site-controls-sanitized
```

Root must integrate the commands dependency before site-tools. The corresponding combined-tree compile command below is documented for integration, and has not run in this separate checkout:

```sh
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic -I . \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/commands/command_target.cc \
  arcified/native/features/site_controls/command_target.cc \
  arcified/native/features/site_controls/command_target_test.cc \
  -o /tmp/arcified-site-controls-test
/tmp/arcified-site-controls-test
```

## Remaining native acceptance

Issue 38 stays open. There is no native adapter, compact-shell control, GN target, extension control, password/autofill integration, zoom, reader/translation control, blocked-popup control, permission/capture indicator, or browser-dialog implementation in this directory.

Installed Linux, Windows and macOS builds must verify Page Info identity, native permission/capture indicators, site settings and print through pointer and keyboard commands. Exercise same-URL distinct pages/Profiles, hostile synthetic origins, native pane focus, same-window Profile switches, saved-page transfer, reload/navigation while a menu or bubble is open, anchor destruction, fullscreen/display changes, Peek/splits, cancellation and unavailable controllers. Confirm prompts and dialogs remain associated with the correct native page, parent window and Profile, including accessibility focus/announcements. Retain native reload/unsaved-work protections. No browsing data, provider access, benchmark, heavy Chromium preparation or native build was used for this slice.
