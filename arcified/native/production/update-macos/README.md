<!-- SPDX-License-Identifier: GPL-3.0-only -->

# macOS update unavailable-path packet

Issue [#167](https://github.com/Timpan4/helium-arcified/issues/167) remains OPEN. This packet implements the policy-independent native unavailable path. It does not implement or prove an authenticated update channel. Production integration and independent review remain with the release coordinator.

The native macOS `VersionUpdater::Create()` binding reports `FAILED` with `IDS_ARCIFIED_UPDATE_MACOS_UNAVAILABLE`, hides updater promotion and provides no available version, update progress, rollback, wipe or relaunch-ready result. The existing About page hides `DISABLED`, so that status would conceal the explanation. This binding invokes no update, installation, restart, Profile, workspace or cleanup service. `PromoteUpdater()` has no effect while unavailable. Callback arguments are owned by the call, so a promotion callback may destroy the updater before the status callback runs.

The macOS help build boundary rejects `enable_updater=true` and rejects `enable_sparkle=true` when that platform argument exists. There is no new enablement flag or endpoint. The owner must approve the infrastructure and the coordinator must replace this temporary unavailable binding with reviewed real service integration before enabling updates. Do not bypass these assertions to produce a release.

## Authorities and source identity

Baseline is `a4b6b480d3de07857ab6e939149254a8359e5449`. Prepared Chromium is `155.0.8059.79`, inspected read-only at `/home/timpan4/.t3/builds/arcified-linux-155-native-v1/src`.

Read authorities under `/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/arcified/docs/`:

1. `production/release/assignment.json` and `non-ai-production-workstreams.json` assign issue #167 to this packet and reserve heavy native work to root thread `6733348c-3696-4b31-834c-2bbca45e70d2`.
2. `lifecycle-contract.md`, `native-space-integration.md` and `execution-state.json` retain the shared Model, metadata/codec, actual Profile identities, saved-page handoff and command-target contracts. None is modified or replaced here. Native same-window multi-Profile and saved-page handoff stay OPEN.
3. This checkout's `arcified/AGENTS.md` retains the fork/upstream boundary.

`base/` contains byte-exact snapshots of the three modified prepared-source files. `result/` contains their complete results and the two new native C++ files. `sources.sha256` records all base/result SHA-256 values and the patch identity. New C++ files have no base counterpart. Existing upstream notices are preserved. The resource result also gains a final newline; its existing LF line endings are unchanged. New Arcified code and packet material use GPL-3.0-only.

Supporting prepared-source identities:

| Source | SHA-256 |
| --- | --- |
| `chrome/browser/ui/webui/help/version_updater_mac.mm` | `83e74ff4ded0d1b831866c6c63618b028f27a2b4262182e8458ec1e451ae6b22` |
| `chrome/browser/ui/webui/help/version_updater.h` | `f188c9280b79a7205db53ca0c984d7b7c8c6f3232ee0e0ac00f935bed0372279` |
| `chrome/browser/updater/updater_no_updater.cc` | `30277e391cd4a01f1c7d434c922fe064ff55b756d94b53d4d6fdd8fe1046399d` |
| `chrome/browser/buildflags.gni` | `aacb7077932338a8cc1f6eae82a8eb14b498ed7213fdc4d59576cbcb06c5debf` |
| `chrome/browser/ui/webui/settings/about_handler.cc` | `81c01528c735195bc980f0826eade20e4b36535eb4f7c6253c4032a5d7f4a4b2` |
| `chrome/browser/resources/settings/about_page/about_page.ts` | `10ce9b8be3b7d31138c1dc56936bbf829a78738c86bb78fc3edcaad79036870c` |

The read-only [pinned macOS platform series](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/patches/series) includes separate [Sparkle startup integration](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/patches/helium/macos/updater/sparkle2-integration.patch) and an [About updater disable patch](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/patches/helium/macos/updater/disable-default-updater.patch). This explains why disabling only the About check would not establish that background updates are unavailable. These are source observations, not macOS execution or an approved provider selection.

The issue's [Dia security page](https://www.diabrowser.com/security) describes security maintenance, and its [bulletins](https://www.diabrowser.com/security/bulletins) publish security explanations. They do not authorize Arcified's channel, signing keys, provider or rollback policy. No Dia runtime or update-signature behavior is inferred from these documents.

## Native tests and checks

The packet registers `version_updater_arcified_mac_browsertest.cc` in `//chrome/browser/ui/webui/help:browser_tests` and adds that target to the macOS portion of `//chrome/test:browser_tests`. These are actual `InProcessBrowserTest` sources using a real browser, Profile, PrefService and WebContents. They do not construct fake host/service/store objects or call `Model::OpenItem`.

1. `UnavailableCheckRetainsProfileAndLivePage` drives the real `VersionUpdater::Create(page)` binding. It checks explicit failure, hidden promotion, empty version, zero progress/size, no rollback/wipe flags, repeated checks and direct promotion. It checks one real Profile preference and the live page's identity, Profile, URL and tab count remain unchanged. It does not prove durable workspace preservation or absence of every possible filesystem/network side effect.
2. `PromotionCallbackCanDestroyUpdater` destroys the real updater inside its promotion callback and requires the failure result to arrive exactly once.

Both tests are AUTHORED AND REGISTERED, NOT RUN. After approved integration and macOS compilation, the focused runner command is `out/Default/browser_tests --gtest_filter='ArcifiedMacUpdateBrowserTest.*'`. Only the root may arrange heavy work. This packet does not run GN generation, compilation, native tests, installation, signing, CI or visual automation.

Completed lightweight checks on Linux:

`sha256sum --check sources.sha256` passed for every recorded source and the production patch.

1. `cmp` matched every base snapshot against its prepared-source file.
2. Chromium-style `clang-format --dry-run --Werror` passed for both new sources with clang-format 23.1.3 and the prepared tree's `.clang-format`.
3. Prepared GN `2562 (cfcd774b98f3)` formatting and dry-run checks passed for both changed `BUILD.gn` files. This checks GN syntax/format, not target resolution or assertion execution.
4. `xmllint --noout` passed for the complete resulting `generated_resources.grd`. GRIT generation and translation delivery remain unverified.
5. `git apply --check --whitespace=error-all --directory=arcified/native/production/update-macos/base arcified/native/production/update-macos/production.patch` passed from the shared checkout.
6. GNU patch 2.8 applied `production.patch` to a packet-local copy of `base/` with `--batch --fuzz=0 -p1`; `diff -r` matched every reconstructed file to `result/`. This is sparse source applicability, not full platform patch preparation.

## Coordinator integration

1. Review `production.patch` and `sources.sha256`. Reconcile the shared `chrome/browser/ui/webui/help/BUILD.gn`, `chrome/app/generated_resources.grd` and `chrome/test/BUILD.gn` with sibling packets. Preserve Linux and Windows bindings and all existing tests. Do not replace common files wholesale from this packet after sibling integration.
2. Refresh this delta against the complete macOS-prepared tree after preserving the current fork and platform patch order. The platform Sparkle patch also edits the macOS help source selection and imports `sparkle.gni`; its original hunk conflicts with this packet's replacement block. Preserve the platform's existing import so `enable_sparkle` is visible, keep the two assertions in an unconditional `if (is_mac)` scope, and select only `version_updater_arcified_mac.cc` while approval is absent. The old platform disable patch edits `version_updater_mac.mm`, which this binding no longer compiles. Do not register competing `VersionUpdater::Create()` implementations. Full platform applicability remains OPEN.
3. Keep `enable_updater=false` and `enable_sparkle=false` for this unavailable path. The existing macOS preparation foundation already disables Sparkle. Do not choose a feed, service, signing identity, key, package format or data policy to satisfy these gates.
4. The entry point remains existing About settings. Prepared `chrome/app/chrome_command_ids.h:239` defines `IDC_ABOUT=40018`; `chrome/browser/ui/actions/chrome_action_id.h:163` maps `E(kActionAbout, IDC_ABOUT)`. No new legacy command is necessary. Assigned primary 40828 remains unused and Library 40701 is untouched.
5. Obtain independent review, real macOS GN dependency checks and the registered browser test results. Visually verify the About-page unavailable message and hidden promotion/relaunch controls on macOS. The frontend source supports showing this `FAILED` message, but the interaction has not been exercised.

## Remaining acceptance and precise decisions

All three issue acceptance criteria remain OPEN:

1. The fork owner must approve channel/service ownership, fork application identity and signing/key infrastructure before publication. `release-macos` #133 must provide the authorized artifact identity and release/signing path. No provider, certificate, key, endpoint or package format is chosen here.
2. The approved native service must authenticate installed and candidate artifacts and metadata, enforce approved compatibility/downgrade behavior and reject tampering. Owner decisions still needed include trust roots, key rotation/revocation ownership and allowed recovery/downgrade behavior. No signature or update validation has been run.
3. Session durability and approved installer, rollback and data-disposition behavior must exist before interruption/restart/rollback acceptance. Run those paths on the supported macOS artifacts while retaining real Profile/workspace data. The authored unavailable-path tests are not evidence for those paths. Windows/macOS SDK, runtime, signing and installer acceptance stay OPEN.

Only the assigned packet directory was written. No shared ledger, prepared source, patch series, commit, PR, remote issue, release or live host configuration was changed.
