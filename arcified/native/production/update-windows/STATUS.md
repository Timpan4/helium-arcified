# Update-windows status

Status: prepared native unavailable path. Not compiled, executed, independently reviewed, integrated, published or accepted. Issue 166 remains OPEN.

## Actual checks

1. Read the authoritative release `assignment.json`, `non-ai-production-workstreams.json`, lifecycle contract, native Space integration design and execution state from the root coordinator checkout. Confirmed the shared baseline with `git rev-parse HEAD`, which returned `a4b6b480d3de07857ab6e939149254a8359e5449`. Read the public issue and cited Dia pages. No owner approval of a release channel or signing infrastructure was found in the task controls.
2. SHA-256 comparison passed for all four existing-file bases against the read-only prepared source. The result identities cover all seven production/test/integration files. `source-identities.json` records the exact hashes and inspected native binding/version-monitor context.
3. `clang-format -style=Chromium` applied to all three new C++ sources. `clang-format --dry-run --Werror -style=Chromium` passed on those sources with clang-format 23.1.3. No C++ compilation or clang-tidy was run without a Windows compilation database.
4. Prepared-source GN 2562 formatted both result `BUILD.gn` files. `gn format --dry-run` passed on both. This is GN syntax/format checking only; no GN generation or native setup was run.
5. `xmllint --noout` passed on the result `generated_resources.grd`. The pinned prepared-source TypeScript library's `transpileModule` reported zero syntax error diagnostics for the result About page. Node parsed the native browser test's embedded JavaScript successfully. This did not run GRIT, TypeScript semantic checking, WebUI build/tests or a native browser.
6. `git diff --no-index --check base result` emitted no whitespace diagnostics. Its exit status was 1 because the directories differ. The production patch passed sparse `git apply --check`; applying it inside a packet-owned copy of `base/` and comparing with `diff -r` reconstructed all of `result/` byte for byte. An initial apply attempt from a nested repository directory skipped the paths; the byte comparison exposed that and the corrected repository-root `--directory` check passed. The temporary verification copy was removed.

All task writes are confined to this packet. The prepared source remains read-only. No shared ledger, quilt series, commit, native preparation/build/test, remote CI, installation, service, global configuration, authentication, signing infrastructure, GitHub write or publication was performed.

## Open acceptance and decisions

The owner must approve channel/service ownership and signing/key infrastructure before publication. Exact product identity, installer/package format, trust/signing identity and key handling, activation privileges, downgrade compatibility, rollback and user-data disposition remain decisions for the coordinator and owner. This packet does not settle them.

Authenticated artifact/metadata verification, tamper rejection, incompatible downgrade rejection, interrupted/failed update, restart, approved rollback and retained Profile/workspace data remain unimplemented or unverified at the actual update boundary. `release-windows` and `session-durability` remain dependencies. Same-window multi-Profile switching and saved-page handoff remain OPEN.

Three unit tests and one native browser test are authored and registered in the patch, but none ran. Windows compilation, SDK/runtime, installed application, signature/installer, visual interaction and accessibility-runtime checks remain OPEN. The root owns native builds and tests; the coordinator owns reconciliation and independent review.
