# Windows update packet

This packet implements an explicit unavailable path in the real native Settings About updater for unbranded Windows builds. It is policy-independent work for [issue 166](https://github.com/Timpan4/helium-arcified/issues/166). It does not deliver an authenticated update channel or close the issue.

The existing Windows GN selection uses `VersionUpdaterBasic` when `is_chrome_branded` is false. That updater reports `NEARLY_UPDATED` when `UpgradeDetector` observes an installed-version change. `get_installed_version_win.cc` obtains version information from `InstallUtil`; this path does not establish an authenticated Arcified artifact identity. The Google-branded implementation uses Google Update and is not a fork-owned service.

The packet selects `ArcifiedVersionUpdaterWin` for the existing unbranded Windows factory. `AboutHandler::RequestUpdate` continues to call the real `VersionUpdater::Create` implementation through its existing native binding. The factory returns `DISABLED` with a localized explanation and no update version, progress, payload size, rollback, device wipe, or promotion. It does not contact a service, read an update manifest, stage an installer, modify Profile/workspace data, or request a restart. The Windows About page displays the explanation through its existing sanitized status message and polite live region. The existing status handling leaves the relaunch action hidden.

This change concerns the About updater only. Other upgrade notifications, executable swapping, manual installation, enterprise update behavior and browser restart paths are outside this patch. It does not authenticate those paths or disable all update infrastructure. An owner-approved transport must later replace this unavailable implementation at the actual download/install boundary.

## Packet contents

- `base/` contains exact prepared-source files for each existing file changed by the patch.
- `result/` contains those result files and the new production and native test sources, at their intended Chromium paths.
- `production.patch` applies relative to a Chromium source root. It is not registered in `patches/series`.
- `source-identities.json` contains SHA-256 identities, command disposition and native test registrations.
- `STATUS.md` records actual checks and open acceptance.

Existing Chromium copyright and license headers remain intact. The new Arcified C++ files use GPL-3.0-only. Existing LF line endings remain LF; the result adds the missing terminal newline to `generated_resources.grd`.

## Integration requests for coordinator and root

1. Independently review the packet against baseline `a4b6b480d3de07857ab6e939149254a8359e5449` and prepared Chromium `155.0.8059.79`. Compare every base SHA-256 in `source-identities.json` to the destination. Reconcile concurrent changes to `chrome/test/BUILD.gn`, `chrome/browser/ui/webui/help/BUILD.gn`, the About page and generated strings by merging the small hunks. Do not overwrite common files with the packet copies.
2. Have the root apply `production.patch` to its authorized integration source and register the reconciled downstream patch in the existing quilt order. This implementation task did neither. The factory substitution and test registrations are conditional on `is_win && !is_chrome_branded`; Linux, macOS, ChromeOS and Google-branded Windows source selection remains as before.
3. Generate and check the Windows GN graph using the approved Windows build configuration, then build the existing `unit_tests` and `browser_tests` targets. The prepared registration adds `//chrome/browser/ui/webui/help:arcified_windows_unit_tests` to `//chrome/test:unit_tests`, and `//chrome/browser/ui/webui/help:arcified_windows_browser_tests` to `//chrome/test:browser_tests`. GN formatting here checks syntax, not dependency resolution, linkability or Windows compilation.
4. Run `unit_tests.exe --gtest_filter=ArcifiedVersionUpdaterWinTest.*` and `browser_tests.exe --gtest_filter=ArcifiedUpdateWindowsBrowserTest.*` in that Windows build. The unit sources cover the unavailable result, callback-triggered destruction, and repeated requests. The browser source navigates a real browser to `chrome://settings/help`, sends `refreshUpdateStatus`, and observes the real `AboutHandler` event. It substitutes no host, service, store or updater. These tests cover the unavailable entry point, not update application or retained-data durability.
5. Visually verify the Windows About page on the supported OS. Confirm the unavailable explanation is visible, exposed in the existing live region, and offers no update relaunch. Check repeated navigation and refresh. No visual, accessibility-runtime or Windows browser evidence exists from this task.
6. Retain the unavailable path until the owner approves the service/channel owner, artifact/package identity and format, signing identity and key custody/rotation, installer activation privileges, downgrade compatibility rules, and rollback/data disposition. Integrate the release-windows and session-durability work before exercising real updates. No provider, endpoint, certificate, installer format, cleanup or rollback policy is selected here.

No new command is required. This uses the existing Settings About entry point and binding. Assigned primary legacy ID `40829` remains unused, Library `40701` is unchanged, and no new legacy command or ActionItem mapping is introduced.

## Required acceptance after the decisions

1. Prove fork-owned installed-to-updated application identity, OS trust validation, artifact signatures and authenticated metadata. Exercise modified and substituted metadata and artifacts, including validly signed artifacts from an unauthorized identity, wrong product/channel/architecture, revoked trust and key rotation. Reject incompatible downgrades at the real installation boundary without rewriting user data.
2. Exercise interruption and failure during download, verification, staging and activation, including process termination and restart. Check actual Profile/workspace data and session recovery in the installed Windows application. The unavailable implementation has no update transaction and proves none of these cases.
3. Exercise the explicitly approved rollback policy and compatibility behavior. Preserve protected originals and user data according to that approval. Do not infer rollback authorization from Chromium's enterprise rollback enum or implement automatic deletion, cleanup, backup or downgrade policy in this packet.
4. Keep same-window multi-Profile switching, saved-page handoff, native session durability, Windows SDK/runtime/signing/installer results and all issue acceptance open until their actual native paths are proven.

The [Dia security page](https://www.diabrowser.com/security) and [security bulletins](https://www.diabrowser.com/security/bulletins) were read as documentary references. They describe security practices and fixes, not an Arcified Windows service, authorized signing infrastructure or native Windows update evidence.
