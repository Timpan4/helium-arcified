<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Engine update production packet

Issue [41](https://github.com/Timpan4/helium-arcified/issues/41) remains OPEN.
This packet implements read-only provenance collection, an update procedure and
registered native browser regression sources. It does not execute an engine
update or prove native Arcified behavior.

The declared audited revision is Chromium 155.0.8059.79. The prepared source
reports Helium 0.19.2.1. The assignment baseline is
`a4b6b480d3de07857ab6e939149254a8359e5449`. The historical preparation report
records a different fork commit, `05ace22efb8564d0294b8efa1b3f3651b76cef07`,
with tracked changes. A prepared version marker does not establish that all
baseline foundations are integrated in that tree.

## Contents and integration

1. Review `production.patch`, `base/`, `result/` and `source-identities.json`.
   The base BUILD.gn is an exact copy of the prepared source file. The new test
   file has no base. Existing Chromium files retain their upstream license;
   the new test and tooling are GPL-3.0-only.
2. Reconcile the BUILD.gn edit with other packets. It adds the test file and
   its direct dependencies to the existing
   `//chrome/browser/profiles:browser_tests` target. That target is already
   included by `//chrome/test:browser_tests` at prepared
   `chrome/test/BUILD.gn:1891`. There is no separate unreferenced test target.
3. The coordinator applies or refreshes the patch in its writable integration
   source, then registers it in the downstream patch group in preserved order.
   This packet does not edit `patches/series` or the read-only prepared tree.
   Do not copy the full result BUILD.gn over concurrent integrated edits.
4. Run the focused browser filter
   `ArcifiedEngineUpdateBrowserTest.*` on each supported target OS after the
   coordinator's native build. Record executable identity, build arguments,
   command, exit status and test output in that update's evidence. GN formatting
   and sparse patch application are not GN generation, compilation or execution.
5. Use the existing real Arcified feature tests and installed-browser checks
   described in `PROCEDURE.md` once their bindings exist. These additional
   regression tests cover engine identity, ordinary navigation and cookie
   isolation only. They do not close lifecycle, classifier or product acceptance.

No UI command is added. Legacy ID 40737 is reserved for this issue and unused.
Library ID 40701 and all existing ActionItem mappings remain unchanged. There
are no product strings, service/provider choices, signing identities or update
delivery configuration in this packet.

## Read-only audit

From the assignment checkout, run:

```sh
python3 -B arcified/native/production/engine-updates/audit.py \
  --source-tree /home/timpan4/.t3/builds/arcified-linux-155-native-v1/src
```

The command checks the prepared Chromium pin against `chromium_version.txt`,
requires a complete numeric Chromium/Helium version stamp, invokes the existing
patch-file validator and records the ordered fork patches and working input
hashes. It writes JSON to stdout and does not download, prepare, build, install,
modify the source tree or start a browser. It always leaves native execution
and update acceptance OPEN. `audit-155.0.8059.79.json` is the actual local run.
Hashes identify the files read. They do not establish archive authenticity,
a pristine source tree or an atomic snapshot of a concurrently changing tree.

## Security and open acceptance

`security-review.json` records primary-source observations retrieved
2026-10-24, including later stable security changes and inaccessible primary
pages. The audited pin is older than the observed stable release. No new target
is silently selected, and no downstream vulnerability or backport status is
inferred from version numbers alone.

Linux, Windows and macOS native build, test and installed interaction acceptance
remain OPEN. Same-window multi-Profile and saved-page handoff remain OPEN.
The coordinator owns heavy execution, independent review, integrated source,
patch registration and publication. The update procedure needs a declared
target, its actual patch-conflict/refresh record and native results on every
target OS. Extension CI cannot supply those results.
