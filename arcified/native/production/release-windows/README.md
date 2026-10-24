<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Windows release production packet

This packet hardens the native Windows installer target lookup for issue [132](https://github.com/Timpan4/helium-arcified/issues/132). It is ready for coordinator integration and independent review. Signed-package and installed Windows acceptance remain OPEN. None of the issue's three acceptance criteria is complete.

The production code reads real product registration through ProductState. Before removing path components from UninstallString, it now requires the documented `<version>\Installer\setup.exe` suffix. It uses existing installer constants, Windows case-insensitive comparisons and Chromium's base::Version parser. An unrelated executable or malformed layout no longer selects an arbitrary ancestor directory as the registered install target.

Existing absolute-path, parent-reference, volume-root and directory-existence checks remain. The setup version may differ from the Clients version, and this lookup does not require the setup executable to exist, preserving repair and interrupted-update behavior. Invalid registration returns the existing empty result. GetChromeInstallPathWithPrefs then follows its existing preference/default fallback. This can select a different install location; it does not recover an existing Profile or establish that it is safe to install there. Recovery and user-facing diagnostics remain open. No cleanup, data migration, downgrade authorization or trust bypass is added.

# Files and source identity

prepared-base/ contains exact copies of the read-only prepared files. prepared-result/ contains their proposed results. release-windows.patch changes only:

1. chrome/installer/util/helper.cc, production registration-layout validation.
2. chrome/installer/util/helper.h, the actual lookup/fallback contract.
3. chrome/installer/util/helper_unittest.cc, native registry/filesystem regression cases.

manifest.json records base/result/patch SHA-256 identities, controls and open acceptance. The copies match Chromium 155.0.8059.79 in the prepared source. Shared checkout HEAD was the assigned baseline a4b6b480d3de07857ab6e939149254a8359e5449. Existing upstream copyrights and license notices are preserved. New Arcified packet material and downstream additions use GPL-3.0-only.

evidence/installer-util.BUILD.gn is an unchanged source snapshot proving helper_unittest.cc is already registered in the Windows //chrome/installer/util:installer_util_unittests target. Do not copy this evidence snapshot over the coordinator's BUILD.gn.

# Native tests supplied

The added tests drive existing public installer APIs using real ProductState, Windows registry APIs redirected by RegistryOverrideManager and real temporary directories. They contain no fake host/service/store and no model-success simulation.

1. RejectsMalformedRegisteredSetupLayout rejects wrong executable/installer names, invalid versions and incomplete layouts in both registry scopes.
2. AcceptsCaseAndRegisteredVersionSkew preserves mixed-case names, paths containing spaces and a valid setup version different from the registered product version.
3. MalformedRegistrationUsesNormalTarget checks that the actual setup-target API uses its existing preference/default result instead of reusing an unrelated registered ancestor.

The existing parameterized suites register these cases for user/system scopes and preference variants. Their sources are supplied and registered, but they were not compiled or executed. Source/format/apply checks do not prove registry behavior or installation on Windows.

# Coordinator integration

1. Reconcile the three source files with other Windows packets. Compare current file hashes with manifest.json. If another packet changed these files, merge the small validation/test changes by content; do not replace common files wholesale.
2. Review and apply release-windows.patch from the Chromium source root under the root thread's authority. This implementation task did not modify the prepared tree or patches/series. The coordinator owns downstream patch registration and independent review.
3. Once a Windows SDK/build is authorized, build the existing //chrome/installer/util:installer_util_unittests target. Run installer_util_unittests.exe with --gtest_filter="*GetInstalledDirectoryTest*:*GetChromeInstallPathWithPrefsTest*". Keep the raw result and actual build identity. Existing tests also cover missing-directory, traversal, relative-path, volume-root and wrong-scope behavior.
4. Resolve owner decisions below before packaging or signing. Run installed-product scenarios with synthetic Profiles, including clean install, default-browser registration and OS prompts, upgrade/restart, retained workspace data, interrupted install/update and explicit rollback/downgrade refusal/failure. Record visual/accessibility behavior and package plus installed-binary trust results against the approved expected signer. A valid signature from an arbitrary publisher is insufficient.
5. Retain prerequisite failures and unavailable platform results as OPEN. Same-window multi-Profile and saved-page handoff remain unresolved. This packet does not weaken their contract.

No browser command is needed for installer target validation. Command 40797 remains reserved and unused. No legacy command, ActionItem, string ID or change to Library 40701 is introduced.

# Decisions and acceptance still open

The assignment does not authorize a signing provider/identity, package format, install scope, cleanup, rollback or data disposition policy. The prepared chrome/install_static/chromium_install_modes.h still exposes Chromium product paths, AppUserModelID and ProgIDs. That is source evidence, not an approved distinct Arcified identity or proof of a working installer.

The coordinator needs owner decisions for the supported package format/install scope, fork-owned Windows identity/registration namespace, authorized signing/release infrastructure and expected signer, and rollback/downgrade/data disposition. No provider, certificate, trust root, package type, GUID or Profile path is chosen here.

Existing downgrade admission in setup_main.cc still uses IsDowngradeAllowed(prefs) and reports HIGHER_VERSION_EXISTS when applicable. This packet neither enables that preference nor claims it implements Arcified schema-safe rollback.

All three acceptance criteria remain OPEN: authorized signing and signatures; clean install/default-browser/upgrade/retained Profile data; and explicit rollback/downgrade plus visible update/security failure behavior. Windows SDK/compiler/runtime, signer, installer, OS trust, installed identity, visual/accessibility and persistence checks were not run. Native-windows, session-durability and schema-migrations remain blockers. Linux source checks cannot close Windows acceptance.

Lifecycle Model, metadata/codec, command targets and private/download classification are unchanged. This packet creates no second workspace model or persistence authority.
