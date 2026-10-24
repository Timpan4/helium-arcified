<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Linux release packet

Issue [#131](https://github.com/Timpan4/helium-arcified/issues/131) remains OPEN.
This packet prepares policy-independent native regression tests. It does not
implement an installer or select a package format, application identity,
signing identity, update provider, cleanup, rollback or data-disposition policy.
No native build, browser test, installation or publication ran.

Baseline is `a4b6b480d3de07857ab6e939149254a8359e5449`. The inspected downstream
source is Chromium `155.0.8059.79` at
`/home/timpan4/.t3/builds/arcified-linux-155-native-v1/src`, accessed read-only.
Exact file identities and authority snapshot hashes are in
[provenance.json](provenance.json). These identify the inspected files, not the
entire source tree, a built binary, a package or a signature.

## Prepared changes

[production.patch](production.patch) adds a Linux-only source to the existing
`//chrome/test:browser_tests` target. It adds no product runtime behavior.

- [prepared-base/chrome/test/BUILD.gn](prepared-base/chrome/test/BUILD.gn) is the
  exact prepared build file. Its SHA-256 is
  `38933520829a78288766cd702e665076f62e4ac720c82ca816d342a59a874099`.
- [prepared-result/chrome/test/BUILD.gn](prepared-result/chrome/test/BUILD.gn)
  registers the native test under the existing `is_linux` condition.
- [prepared-result/chrome/browser/arcified/release_linux_browsertest.cc](prepared-result/chrome/browser/arcified/release_linux_browsertest.cc)
  is new GPL-3.0-only source. Its base is absent, not an empty pre-existing file.

The tests use real `InProcessBrowserTest`, `Profile`, `PrefService`,
`PathService` and filesystem operations. They neither construct fake native
objects nor call the standalone lifecycle Model.

| Native test | Intended assertion, execution still OPEN |
| --- | --- |
| `UsesExplicitUserDataDirectory` | The actual test Profile lives below the absolute directory requested by the browser-test launcher. |
| `PRE_RetainsProfileDataAfterRestart` | Write test-owned files and real Profile preferences, then await the native preference-store write callback. |
| `RetainsProfileDataAfterRestart` | After a real PRE_ process restart, read the retained Profile path, file content and preferences. This stage does not seed or repair them. |

The launcher overrides the data directory before native startup. These tests
therefore do not prove default application identity or environment-variable
precedence. PRE_ runs use the same binary. They do not prove package upgrade,
crash recovery, workspace/session durability, schema migration or uninstall.
The fixture uses the launcher's test directory only. No real user Profile is
opened or changed by the packet checks.

No legacy command or ActionItem is needed for these test-only changes.
Assigned primary command `40794` is reserved but unused; Library `40701` is
untouched. No UI strings are added. Future UI strings must use
`IDS_ARCIFIED_RELEASE_LINUX` prefixes.

## Checks performed

On 2026-10-24, the following light checks exited 0:

1. `sh -n arcified/native/production/release-linux/verify-packet.sh`.
2. `sh arcified/native/production/release-linux/verify-packet.sh /home/timpan4/.t3/builds/arcified-linux-155-native-v1/src`.

The second command verifies the recorded packet SHA-256 values, compares the
prepared build-file base with the read-only source, checks and applies the patch
in a fresh packet-owned sparse directory, compares both results byte-for-byte,
and checks reverse application. It uses `git apply --whitespace=error` for
focused whitespace validation. C++ formatting passed with clang-format 23.1.3
and Chromium style. The prepared GN file passed the source tree's
`buildtools/linux64/gn format --dry-run`. The verifier removes only the scratch
directory it created. It never applies a patch to the shared source tree.

These results prove artifact consistency and formatting. C++ compilation, GN
generation/dependency checking, native test execution, visual behavior and
independent review remain OPEN. No pure-component test is offered as native
completion. [checks.json](checks.json) records the distinction.

## Coordinator integration

1. Review `production.patch` and reconcile its build-file insertion with other
   packets. Compare the base hash before applying. If the common build file has
   changed, preserve concurrent registrations and rebase the insertion rather
   than copying the complete result file over it. The new native source must
   still be absent before adding it.
2. Integrate and register the patch through the root-owned source workflow,
   preserving quilt order and upstream licenses. This primary did not edit
   `patches/series`, commit or change shared ledgers.
3. Have root thread `6733348c-3696-4b31-834c-2bbca45e70d2` build the existing
   `browser_tests` target and run
   `browser_tests --gtest_filter='ArcifiedReleaseLinuxBrowserTest.*'` from its
   chosen native output directory. Use the normal test launcher so PRE_ stages
   retain their test directory. Record the exact source and binary identities,
   command and result. This is a request for root integration, not an execution
   performed by this primary.
4. After owner decisions and prerequisite evidence, perform the actual package
   lifecycle checks below in the approved supported environments. The native
   tests above supplement that evidence and cannot replace it.

Source inspection also found that `ChromePaths.DefaultUserDataDir` in the
prepared `chrome_paths_unittest.cc` still expects upstream Chromium branding and
`CHROME_CONFIG_HOME`. The actual prepared `chrome_paths_linux.cc` uses
`net.imput.helium` and `HELIUM_CONFIG_HOME`. This discrepancy is reported for
coordinated identity work. This packet does not change that existing test or
assert an Arcified default namespace.

## Open acceptance and decisions

| Issue criterion | State and exact remaining work |
| --- | --- |
| Supported format and fork-owned identity | OPEN owner decision. Declare format, supported distributions/environments and architectures, package name, executable/desktop identity, install path and user-data namespace. The prepared packaging metadata still says `helium`, `Helium`, `/opt/chromium.org/chromium` and `net.imput.helium`, with upstream maintainer/support fields. Do not publish these as an approved Arcified identity. |
| Clean installation | OPEN. Build the chosen package, inspect its contents and metadata, install it in an approved disposable environment and launch the installed executable. Verify the declared native identity, registration, actual user-data location and lack of collision with existing installations and data. The test harness is not the installed launch path. |
| Upgrade and uninstall data choices | OPEN owner decision and native execution. State how existing Helium/Arcified data is treated, what uninstall/purge preserves or removes, and approved rollback/downgrade behavior. Test the real package transitions with approved fixtures after session-durability and schema-migrations are proven. Do not infer cleanup permission from this test's scratch cleanup. |
| Provenance, smoke checks and credential exclusion | File source identities recorded. Native smoke checks and release-artifact audit OPEN. Once a package exists, record source/patch/build/package identities, dependency and architecture evidence, installed-path smoke results and applicable trust checks. Inspect the actual artifact for unintended signing/release credentials. No credentials, keys, installer or release artifact are created by this packet. |

The dependencies `native-linux`, `session-durability` and `schema-migrations`
remain OPEN. Same-window multi-Profile switching and live saved-page handoff
remain OPEN. The lifecycle Model, metadata/codec, command targeting and
private/download classifiers are unchanged. No Windows/macOS SDK, runtime,
signing or installer result is claimed.
