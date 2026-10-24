# Light verification

Recorded 2026-10-24. These checks verify research-packet structure and source identities, not browser behavior.

## Exact commands and outcomes

All shell commands ran from `/home/timpan4/.t3/worktrees/helium-arcified/feat-non-ai-research-production`.

1. `git rev-parse HEAD` returned `a4b6b480d3de07857ab6e939149254a8359e5449`. `git branch --show-current` returned `feat/non-ai-research-production`. No Git mutation followed.
2. `command -v node bun prettier markdownlint markdownlint-cli2 jq sha256sum` found Node, Bun, jq and sha256sum. Prettier and Markdownlint were absent. The command exited 1 because some requested executables were absent. Scoped `fast_filesystem.fs_list` found no repository Markdown/Prettier configuration; the existing extension package declares no documentation formatter/linter. No dependency was installed.
3. `sha256sum` was run with each absolute path now listed in [source-identities.sha256](source-identities.sha256). The output supplied the recorded full-file byte identities, not a build identity. These hashes do not attest runtime behavior.
4. `sha256sum --check arcified/native/production/retirements/source-identities.sha256` initially exited 1. All inputs matched except the shared root manifest, which changed while primaries were active. The manifest was re-read in full with `fast_filesystem.fs_read_file({path, max_bytes:150000})` without truncation. Its changes added settings verification and shared dependency records, including the native session-durability API for export/import consumers. Retirement key, issue, command ID and ownership were unchanged. The new observed manifest digest is recorded; no root file was edited.
5. `git diff --check -- arcified/native/production/retirements` exited 0. The packet is untracked, so this does not lint its new contents. The content lint below supplies that check.
6. `git status --short -- arcified/native/production/retirements` showed only the untracked packet. `git diff --stat -- patches/series patches/arcified` produced no output at that observation. Other primaries' packet changes were not examined or altered.
7. `node --check arcified/native/production/retirements/packet-check.js` is the focused syntax command for the packet linter. Its executed result is recorded in [checks.json](checks.json).
8. The source-identity command in step 4 was rerun after recording the inspected manifest update. Its final executed result is in [checks.json](checks.json).
9. JSON was formatted with the host JavaScript formatter `JSON.stringify(value, null, 2) + "\n"` and written through `apply_patch`. Packet contents were then read only through `fast_filesystem.fs_read_many`. The exact content/coverage lint is [packet-check.js](packet-check.js), executed through Node with its data supplied from fast_filesystem. The exact host orchestration was:

```js
const files = load("finalRead");
const code = files.find(x => x.path.endsWith("/packet-check.js")).content +
  "\nconst result = checkPacket(" + JSON.stringify(files) + ");\n" +
  "process.stdout.write(JSON.stringify(result));\n" +
  "if (result.failures.length) process.exitCode = 1;\n";
const quote = s => "'" + s.replace(/'/g, "'\"'\"'") + "'";
await tools.exec_command({
  cmd: "node --input-type=commonjs -e " + quote(code),
  max_output_tokens: 2500
});
```

Here `files` contains the complete UTF-8 outputs from `fast_filesystem`; Node performs no generic local file reads. A prior attempt to transfer a function value through hosted JavaScript failed serialization, so it was replaced by this Node invocation. The linter checks JSON format/syntax, final newline/LF/trailing whitespace, Markdown table columns, all assigned disposition families, evidence source IDs, research-only/open native states, settled sync exclusion and handoff evidence paths. `fast_filesystem.fs_stat_many({paths: parsedOutput.localLinkTargets})` checks every local Markdown target. Final counts/outcomes are in [checks.json](checks.json). This is a packet lint, not a native test suite.

## Scope and unavailable checks

The prepared reading-mode controller was first looked up under an obsolete side-panel path; `fs_rg` returned an actionable missing-file error. The corrected `chrome/browser/ui/read_anything/read_anything_entry_point_controller.cc` was read and hashed. No missing-file result was accepted as proof of feature absence.

Primary-source page retrieval and route repairs are documented in [sources.json](sources.json). Notes, sunset and migration were recovered through article-ID routes; Max help remained unavailable. No raw remote byte hash is claimed.

No Chromium setup/preparation/build/native test, reference application, browser/visual interaction, benchmark, heavy CPU check, remote CI, account or service was run. No native patch/test registration exists for this research-only leaf, so patch syntax/sparse apply, GN registration and C++ compile checks are inapplicable. Native source leads and full base/result identities remain source-only evidence. Installed-platform behavior, actual import compatibility and native acceptance remain **OPEN**.
