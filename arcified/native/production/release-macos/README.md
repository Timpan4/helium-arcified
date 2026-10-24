# macOS release signature packet

Issue [133](https://github.com/Timpan4/helium-arcified/issues/133). Repository baseline `a4b6b480d3de07857ab6e939149254a8359e5449`. Native and release acceptance remain **OPEN**.

This packet implements a policy-independent, read-only native static-signature verifier and command-line audit executable. It calls CoreFoundation and Security.framework on the actual application bundle. It does not create a signing identity, sign an artifact, install an application, or settle the package/update/data policy.

## Files and integration

`base/chrome/installer/mac/BUILD.gn` is the exact captured prepared-source input. `result/` holds that file's complete proposed result and the new C++ files. New files have no base file; their manifest base path and SHA-256 are null. `production.patch` changes only `chrome/installer/mac/`. `source-identities.json` records every source result and the inspected authorities.

1. Reconcile the BUILD.gn addition with other packets. Require its recorded base SHA-256 before applying to a coordinator-owned source tree. Do not overwrite a concurrent change with this full-file result.
2. Apply the production patch in that coordinator-owned tree and retain existing quilt order when the coordinator later registers it. This packet does not edit `patches/series`.
3. On an authorized macOS toolchain, build `chrome/installer/mac:arcified_signature_audit`, `chrome/installer/mac:arcified_signature_verifier_tests` and `chrome/installer/mac:arcified_signature_artifact_tests` in the coordinator's declared output directory.
4. Run the native negative test binary. Run the separate artifact binary and audit tool with an immutable owner-supplied signed app, the approved bundle ID, and a file containing the approved signing requirement expression. No default or upstream signer is selected.

Invocation arguments for both the artifact-test binary and audit tool:

```text
--app-bundle=/absolute/path/to/approved.app
--bundle-id=OWNER_APPROVED_BUNDLE_ID
--signing-requirement-file=/absolute/path/to/approved-requirement.txt
```

The requirement file contains an expression in Apple's code-signing requirement language, without the display prefix `designated =>`. Its approved content must bind the owner's certificate/signer and distribution class. Do not derive expected identity or policy from the artifact being checked. The literal `true` used in negative fixtures is deliberately permissive test input, never a release policy.

## Implemented behavior and limits

The verifier rejects missing or malformed policy, relative/parent-reference paths, invalid bundle identifiers, non-application bundles, unexpected identifiers, and native signature-check failures. It combines the supplied requirement with `anchor trusted`, the exact signing identifier, and the signed Info.plist identifier. Parentheses retain these constraints when the supplied expression contains `or`. Identifier characters follow Apple's documented alphabet. No product identity is invented.

Native validation requests all architectures, nested-code validation, strict bundle checks, and no network access. It retains native OSStatus failures. It does not disable executable/resource checks or modify quarantine, system trust, profiles, preferences, application bytes, or LaunchServices state.

A successful result means local static-signature validation against the provided requirement for those unchanged bytes. It does not establish notarization, current revocation, package-signature validity, approved release class, installation safety or working browser behavior. A trusted development certificate can satisfy an insufficient caller policy. The owner requirement and independent release gates must exclude development distribution where required. No code here labels an app as a signed release.

Apple specifies that static validation is secure only while the code cannot be modified concurrently. Run it on a quiescent artifact and on the actual installed path after the approved install/upgrade. The caller must control writes during checks. The helper performs no locking, retry, installation or process termination.

## Native test registration

`//chrome/installer/mac:arcified_signature_verifier_tests` registers the actual native verifier tests for missing policy, path/configuration errors, malformed native requirement syntax, non-app input, unexpected bundle identity and unsigned app rejection. The unsigned fixture is a real filesystem bundle with a script executable, not a fake signing service. It reads back executable bytes to check preservation.

`//chrome/installer/mac:arcified_signature_artifact_tests` registers a separate real-artifact test. It requires all invocation inputs above, checks the signed app against the approved policy, and checks that the same app fails the native `false` requirement. Missing inputs fail rather than skip. No signing certificate, credential or test artifact is generated.

These sources were not built or executed. Ad hoc signature rejection, alternate-architecture tampering, nested-code/resource tampering, wrong signer, revocation, and real installed-path behavior remain unverified and require native fixtures/results. The use of native API flags is source evidence, not a passing runtime test.

## Identity and support evidence

The inspected prepared `chrome/app/theme/chromium/BRANDING` still names Helium and `net.imput.helium`, including an upstream team identifier. This is baseline evidence, not authorization to use that identity or any signing credential. No branding change is proposed before the owner chooses Arcified's namespace and its relationship to existing user data, keychain access and registration.

Prepared `build/config/mac/mac_sdk.gni` declares `mac_deployment_target = "13.0"` and `mac_min_system_version = "13.0"`. Prepared `app-Info.plist` uses `CHROMIUM_MIN_SYSTEM_VERSION` for `LSMinimumSystemVersion`. These are inspected source defaults, not a supported-OS promise or generated-artifact evidence.

The existing macOS preparation foundation documents arm64 and x64 build plans. No architecture was built here. The owner must declare which architectures and OS versions ship. Record each produced Mach-O slice, generated minimum-OS metadata, SDK/toolchain identity and actual launch results on each approved target. Universal-binary signature checking validates included slices; it does not prove that every promised slice exists or runs. Preserve OS security prompts and native incompatible-OS rejection.

The plist template declares HTTP and HTTPS handlers. That declaration does not prove registration, consent, default-browser selection or external-link dispatch on an installed Arcified app. No custom protocol is added.

## Open acceptance and owner decisions

All issue acceptance criteria remain open:

1. Approve the fork-owned product/app/helper bundle namespace, display names, signing requirement and authorized infrastructure, including team/certificate ownership, entitlements and notarization access. Decide the package format and install location/flow. No provider, signing identity or package format is selected in this packet.
2. Produce the approved artifacts through that infrastructure. Retain artifact hashes, source provenance, signature and notarization evidence, package signatures where applicable, and independent installed-path verification. Exercise unsigned, ad hoc, wrong-signer and tampered artifacts. Keep native warnings and refusal behavior intact.
3. Test a clean install through the approved user flow, native HTTP/HTTPS and approved protocol registration, explicit default-browser consent, cancellation/refusal and actual externally dispatched navigation. Check keyboard/focus/accessibility and security prompts in the app.
4. Prove upgrade from a declared prior installed version with existing Profile/Space/session data. Close the session-durability and schema-migrations prerequisites first. The owner must settle any namespace/data migration, cleanup, rollback/downgrade and data-disposition decisions before those paths are implemented. Preserve originals and unsaved work. Verify both installed signatures and reopened data after upgrade.
5. Prove the approved architecture/OS matrix on macOS SDK/runtime hosts. Close native-macos acceptance. Same-window multi-Profile switching and live saved-page handoff remain OPEN; this release helper makes no lifecycle or native-ownership claim.

No browser command, ActionItem, Model, metadata/codec, command target, private classifier or download classifier changes. Assigned legacy ID 40795 is unused because this tool is standalone. Library40701 stays untouched. No browser resource strings are added, so no IDS prefix allocation is needed.

## Checks performed here

- clang-format 23.1.3 with the prepared Chromium .clang-format, followed by `--dry-run --Werror`: passed for every new C++ source/header.
- Prepared GN's `gn format` and `gn format --dry-run`: passed for the result BUILD.gn. This parses/formats GN syntax; it is not GN generation or a target-resolution/build check.
- `git apply --check --whitespace=error` against the isolated captured base: passed, including additions. No prepared source was modified.
- SHA-256 capture: base BUILD.gn exactly matches the prepared input; all result files and inspected control/source identities are recorded.

No SDK/native preparation, compile, native test, installation, signing, notarization, CI or browser interaction ran. No commit, remote write, shared-ledger edit or quilt registration occurred. The coordinator owns integration and independent review.

## API authorities

Read on 2026-10-24. Apple's [static validation documentation](https://developer.apple.com/documentation/security/secstaticcodecheckvalidity(_:_:_:)) describes static validation, all-architecture checking, requirements and concurrent-modification limits. Its [validation flags](https://developer.apple.com/documentation/security/static-code-validation-flags) define nested-code and strict checks. Its [requirement language](https://developer.apple.com/library/archive/documentation/Security/Conceptual/CodeSigningGuide/RequirementLang/RequirementLang.html) defines `anchor trusted`; [bundle identifier documentation](https://developer.apple.com/documentation/bundleresources/information-property-list/cfbundleidentifier) defines permitted characters. These sources support API use, not release acceptance.
