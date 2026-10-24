# Release production source handoff

The seven assigned leaves have source packets. All seven issues remain OPEN. These packets do not deliver signed packages or authenticated updates. The root baseline built successfully according to the owner's resumption message. Feature compilation, native tests, installed behavior and visual acceptance are still OPEN.

Worktree: `/home/timpan4/.t3/worktrees/helium-arcified/feat-non-ai-release-production`.
Branch: `feat/non-ai-release-production`.
Assignment baseline: `a4b6b480d3de07857ab6e939149254a8359e5449`.

Reviewed source commit: `6f5cb45380855370372bebcf3debceea1882a5aa`. A following documentation commit records this result. No source acceptance beyond the scopes below is implied.

`status.json` records review state and commit status. `task-briefs.json` preserves original primary briefs and correction-review briefs. `review-evidence.json` retains independent findings, accepted responses and completed review results. All seven packet source reviews and the combined-source review are clean. Original packet statuses describe primary completion; the coordinator evidence records subsequent fixes and reviews.

## Integration artifact

Use `integrated.patch` for the combined source delta, with exact SHA-256 identities in `integrated-identities.json`. It reconstructs 26 source files from the recorded prepared bases. The coordinator reconciled the shared About page, generated resources, updater build rules and browser-test registration. Individual packet patches are retained as evidence and alternatives for root reconciliation. Do not apply them again after the combined patch.

The reviewed combined patch SHA-256 is `fff0a1bd84be5de897109032deadc0548339b464b352962f71c3bbea29c81bd8`. Temporary sparse/reconstruction check directories remain untracked and are excluded from the commit. Cleanup was rejected because the command runner prohibits `rm -f`-style commands; no alternate deletion was attempted. The seven committed packets retain every source base/result snapshot needed to reproduce the combined result.

1. Confirm the destination source files match the recorded base identities. If root or another area changed a shared file, reconcile the small delta against that source and retain other changes. Do not replace common files wholesale with packet snapshots.
2. Root owns the authorized prepared integration tree and final quilt registration. Keep the existing upstream patch order. This coordinator did not register anything in `patches/series`.
3. Refresh the macOS delta after the complete pinned platform series. Its Sparkle help build-file patch overlaps the unavailable binding. Preserve the platform import of `sparkle.gni`, keep the updater guards unconditional within `is_mac`, and compile exactly one `VersionUpdater::Create()` implementation. Full platform applicability remains OPEN.
4. Generate and compile the supported native targets, then run the focused tests below. Retain actual source, build and binary identities. Root alone owns heavy checks.
5. Run the actual affected interactions on supported OS builds. The unavailable About message, hidden promotion/relaunch controls and reentry need visual/accessibility evidence. Installer and signing acceptance requires real installed artifacts.

No new legacy command is introduced. Assigned release command IDs remain reserved and unused; the unavailable-update packets use the existing About route. Library 40701 and all authoritative lifecycle, metadata/codec, command-target and private/download classifiers remain unchanged.

## Packet scope and native checks

| Issue | Prepared scope | Root native check |
| --- | --- | --- |
| #131 release-linux | Test-only real Profile placement and PRE_ restart persistence | Existing `browser_tests`, filter `ArcifiedReleaseLinuxBrowserTest.*`. This does not test package upgrade or uninstall. |
| #132 release-windows | Registered installer setup-layout validation and existing fallback behavior | Existing `installer_util_unittests`, filters `*GetInstalledDirectoryTest*:*GetChromeInstallPathWithPrefsTest*`. |
| #133 release-macos | Read-only Security.framework app-signature verifier and explicit-policy audit CLI | `//chrome/installer/mac:arcified_signature_verifier_tests`; separate `arcified_signature_artifact_tests` requires an owner-approved immutable signed app and requirement. Notarization and installer trust are separate. |
| #41 engine-updates | Read-only provenance audit, procedure, engine identity/navigation/private-cookie regression sources | Existing `browser_tests`, filter `ArcifiedEngineUpdateBrowserTest.*`. Execute and record a declared engine update separately. |
| #165 update-linux | Native About updater explicitly unavailable; no authenticated channel | Existing `browser_tests`, filter `ArcifiedUpdateLinuxBrowserTest.*`. |
| #166 update-windows | Unbranded native About updater explicitly unavailable; no authenticated channel | Existing `unit_tests`, filter `ArcifiedVersionUpdaterWinTest.*`; existing `browser_tests`, filter `ArcifiedUpdateWindowsBrowserTest.*`. |
| #167 update-macos | Native About updater unavailable and updater-enable guards | Existing `browser_tests`, filter `ArcifiedMacUpdateBrowserTest.*`; complete platform overlay reconciliation first. |

## Source evidence

Independent packet reviews found four P1 source defects. The Windows browser-test source set now defines `HAS_OUT_OF_PROC_TEST_RUNNER`. The Linux fixture now obtains its poller-disable token through `MakeScopedDisableForTesting()`; the new GRIT part is registered in `generated_resources.grd.gritdeps`. The macOS fixture now uses `BrowserWindowInterface::GetProfile()`. Refreshed patches and identities include these corrections.

The revised packets and combined delta passed forward/reverse sparse applicability, complete base-to-result reconstruction, affected C++/GN formatting, whitespace checks and checksum validation. The prepared GRIT dependency checker passed against coordinator-owned sparse inputs with read-only links to unchanged prepared parts. This dependency parse does not generate GRIT resources or build a browser. The engine audit's four parser tests and unchanged clean packet checks remain recorded in their packet evidence. YAPF/Pylint were unavailable, as recorded in the engine packet.

Historical preparation records fork commit `05ace22efb8564d0294b8efa1b3f3651b76cef07` with tracked changes. It differs from the assignment baseline. Version markers and a successful baseline build do not prove these feature packets were compiled. Root workstream and execution-state hashes in initial packet evidence identify historical snapshots; those control files later changed. Assignment and prepared source identities were checked independently in source reviews.

## Open acceptance

The owner must settle fork-owned application/registration/data identity, Linux package format and supported environments, Windows install scope/package format, macOS package/support matrix, authorized signing/notarization and expected signers, update channel/service/key ownership and rotation, downgrade/rollback compatibility, and cleanup/data disposition. These packets select none of those policies.

Clean install, registration/default-browser behavior, upgrade, uninstall data choices, signature/notarization verification, authenticated metadata/artifact validation, tamper and incompatible-downgrade rejection, interrupted update, restart, approved rollback and retained Profile/workspace data remain OPEN. Native platform, session-durability and schema-migrations prerequisites still apply. Same-window multi-Profile and live saved-page handoff remain production gates. Windows/macOS SDK, runtime, signing and installer results remain OPEN.

The engine procedure still needs an executed declared update, full conflict/refresh evidence, current authoritative security assessment and focused native results on each target OS. The retained security snapshot is dated evidence, not an assertion that the pinned engine is current.

No prepared-tree writes, heavy native checks, installations, service/global configuration/auth/signing changes, CI dispatch, remote publication or global series changes were performed by this coordinator.
