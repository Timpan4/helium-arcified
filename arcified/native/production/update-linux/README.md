# Linux authenticated-update availability packet

Issue [165](https://github.com/Timpan4/helium-arcified/issues/165). Status is an unapplied native source packet for the policy-independent unavailable state. Authenticated Linux updating and all three issue acceptance criteria remain OPEN. No native build, test run or visual verification is claimed.

The release coordinator baseline is `a4b6b480d3de07857ab6e939149254a8359e5449`. The exact prepared source is Chromium `155.0.8059.79` at `/home/timpan4/.t3/builds/arcified-linux-155-native-v1/src`. That tree was read only. All authored files are inside this issue directory.

## Behavior prepared

Linux's existing `VersionUpdaterBasic` maps `UpgradeDetector::is_upgrade_available()` to `NEARLY_UPDATED`. The detector receives installed-version notifications. `get_installed_version_linux.cc` asks the executable for its version; `installed_version_poller.cc` also reports unreadable versions and downgrades as installed-version changes. Those observations do not authenticate an artifact or release metadata.

The patch selects a Linux implementation of the existing `VersionUpdater` interface. It reports `DISABLED` with the localized explanation "Authenticated Arcified updates are not available in this build." It offers no download, install, restart, rollback, progress, candidate version, size or promotion action. There is no endpoint, key, provider, package format, enable switch or injected service. Component and extension updating are separate and unchanged.

The existing native AboutHandler forwards this callback through `update-status-changed`. Linux's existing About-page component displays the nonempty disabled-state explanation. The existing relaunch button remains hidden for that state. Chromium's external installed-version polling and other restart-notification surfaces are unchanged. This packet does not authenticate or govern externally installed executables.

No workspace Model, codec, metadata, page registry, Profile binding, command target, private/download classifier or data disposition changes. The missing same-window multi-Profile host and saved-page handoff remain OPEN.

## Files and identities

`production.patch` is the integration patch. `base/` contains the five exact prepared files being changed. `result/` contains all eight resulting files, including the three new Arcified files. `generated_resources.grd.gritdeps` registers the new GRIT part as a build input. `SHA256SUMS` records every base/result file and the patch. `evidence.json` joins each source-relative path to its base and result SHA-256 identities and records test registration and check scope. A null base identity means a new file.

Original upstream license headers are preserved. New Arcified source, test and string files carry `GPL-3.0-only`. The existing resource file's missing final newline is preserved. GN formatting also sorts an existing macOS source list in `settings/BUILD.gn`; this is a formatter-only change, with no source membership change.

The assigned command `40827` is unused. The packet uses the existing About route. Its actual native ActionItem mapping is `E(kActionAbout, IDC_ABOUT)` in `chrome/browser/ui/actions/chrome_action_id.h`. `chrome/browser/ui/browser_actions.cc` invokes `chrome::ShowAboutChrome(bwi)` for `kActionAbout`. Library `40701` and the command header are unchanged.

## Integration requests

1. Reconcile the patch into the root-owned prepared source after checking `evidence.json` identities. Merge common edits to `chrome/app/generated_resources.grd`, `chrome/browser/ui/webui/help/BUILD.gn`, `chrome/browser/ui/webui/settings/BUILD.gn` and `chrome/browser/resources/settings/about_page/about_page.ts`. Copy the new files at their source-relative paths. Do not replace common files wholesale if other issue packets changed them.
2. Root must validate GRIT resource allocation, GN dependencies and WebUI type checking. Native tests are registered under the existing `//chrome/browser/ui/webui/settings:browser_tests` target on Linux only. `chrome/test/BUILD.gn:2170` already links that target into `//chrome/test:browser_tests`; no extra top-level registration is needed. No quilt registration was made here.
3. Root may build its existing `browser_tests` target and run its resulting binary with `--gtest_filter=ArcifiedUpdateLinuxBrowserTest.*`. All native compilation, source preparation and execution remain root-owned. Obtain independent review before integration acceptance.
4. Visually check `chrome://settings/help`, navigate away, then return. Verify the explanation is visible and announced through the existing live status region, with no update relaunch action. This interaction has not been exercised in this thread.

## Registered native tests

The new `arcified_update_linux_browsertest.cc` uses `InProcessBrowserTest`, the actual browser WebContents, native `VersionUpdater::Create`, the process's real `BuildState` and the actual Settings About route. It creates no fake host, service, store or lifecycle success. The existing `InstalledVersionPoller::ScopedDisableForTesting` prevents executable polling from racing the installed-version observations delivered to the real service.

| Test | Intended proof when root executes it |
| --- | --- |
| `UnavailableWithAndWithoutWebContents` | The production factory returns disabled with the localized reason, no candidate/progress/rollback/powerwash, both with a real tab and with the interface's supported null WebContents. |
| `InstalledVersionSignalDoesNotAuthenticateAnUpdate` | Newer, lower and unreadable executable-version signals supplied to the real BuildState never turn the application updater into authenticated success. The version values are fixtures around the pinned 155 engine, not supported-version policy. |
| `AboutPageExplainsUnavailableAfterReentry` | Actual native callback text appears visibly in the Settings route and the relaunch button stays hidden, including after AboutHandler teardown and recreation through navigation. |

These sources are registered but uncompiled and unexecuted. They do not prove a signed update, interrupted installation, process restart, rollback or retention of Profile/workspace data. The re-entry fixture uses animation frames while waiting for rendering and the browser-test harness's existing timeout, with no custom limit.

## Checks performed

| Check | Result and scope |
| --- | --- |
| C++ formatting | PASS with clang-format 23.1.3 and the prepared source's `.clang-format`, both new C++ files. |
| TypeScript formatting | PASS on changed lines 233 through 243 with the prepared source's style. |
| GN formatting | PASS with prepared GN `2562 (cfcd774b98f3)` for both changed GN files. |
| XML parsing | PASS, `xmllint --noout --nonet`, new strings and modified full resources file. This does not run GRIT. |
| TypeScript syntax | PASS, prepared TypeScript 6.0.2 `transpileModule`, zero syntax diagnostics. No module resolution, type check or WebUI build. |
| Whitespace | PASS, `git diff --no-index --check -- base result` emitted no diagnostics. Its exit 1 denotes differing source trees. |
| Sparse reconstruction | PASS, `patch --batch --fuzz=0 -p1` on a fresh packet copy of every base file; recursive comparison against all result files exited 0. |
| Git patch check | PASS, `git apply --check --directory=arcified/native/production/update-linux/base arcified/native/production/update-linux/production.patch` from the coordinator checkout. |
| Prepared-source identity | PASS, all copied base SHA-256 identities match the corresponding prepared files. |

No C++ compilation, native test execution, GN generation, WebUI build, GRIT generation, runtime/visual test, installation, signing, publication, remote CI or host configuration change occurred. The worker used no native preparation and made no commit, shared-ledger edit or `patches/series` change.

## Open decisions and acceptance

The supplied root assignment and execution controls contain no approval for the Linux channel/service owner, release origin, package format/application identity, signing/key custody or rotation/revocation infrastructure. They also do not settle update activation, rollback compatibility, profile/schema recovery or data disposition. This packet selects none of those. Owner approval and the release-linux/session-durability results must precede a real updater and publication.

| Issue acceptance | Status |
| --- | --- |
| Approved service/channel ownership and signing/key infrastructure before publication | OPEN. Coordinator must obtain and record those exact owner decisions. No publication occurred. |
| Installed-to-updated artifact identity/signatures; tampered metadata and incompatible downgrades rejected | OPEN. A disabled native availability path is prepared. No artifact verifier, metadata protocol or downgrade decision is implemented. |
| Interrupted/failed update, restart and approved rollback retain Profile/workspace data | OPEN. Depends on approved Linux packaging, durable native sessions and explicit rollback/schema/data behavior. No updater, cleanup or rollback operation is implemented. |

Windows and macOS SDK/runtime/signing/installer acceptance remain OPEN. The documented reference sources are [Dia security](https://www.diabrowser.com/security) and [security bulletins](https://www.diabrowser.com/security/bulletins). They were read as workflow context, not evidence of a Linux update protocol or approval to use a particular signing system. The local prepared fork source and root controls govern this packet.
