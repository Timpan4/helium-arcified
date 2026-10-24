# Focused packet verification

Date: 2026-10-23 UTC, 2026-10-24 Europe/Stockholm.
Scope: research evidence under `arcified/native/production/arc-import-format/`.
Native acceptance remains OPEN.

## Source retrieval

These exact primary retrieval commands completed successfully. Their response bytes were parsed with Node and SHA-256 identities are retained in `sources.json`.

```sh
curl -fsS 'https://resources.arc.net/api/v2/help_center/en-us/articles/44413129117591.json'
curl -fsS 'https://resources.arc.net/api/v2/help_center/en-us/articles/44412972248471.json'
curl -fsS 'https://resources.arc.net/api/v2/help_center/en-us/articles/25583851606039.json'
```

They were run as the producer in a draining Node pipeline. The exact export identity repetition command was:

```sh
curl -fsS 'https://resources.arc.net/api/v2/help_center/en-us/articles/25583851606039.json' | node -e 'let s="";process.stdin.on("data",x=>s+=x);process.stdin.on("end",()=>{const a=JSON.parse(s).article;console.log(JSON.stringify({id:a.id,locale:a.locale,created_at:a.created_at,updated_at:a.updated_at,bodySha256:require("node:crypto").createHash("sha256").update(a.body).digest("hex"),bodyBytes:Buffer.byteLength(a.body),bodyLinks:[...a.body.matchAll(/href="([^"]+)"/g)].map(x=>x[1])},null,2));});'
```

Outcome: PASS. Export article body was 4229 UTF-8 bytes, SHA-256 `ee474965451c44d9d3b961a3dafe7f8d075d4ed796966bc1cf51e93eecbad666` on both retrievals. This verifies the fetched documentary body, not an export or runtime behavior. No link target was opened by the pipeline. Responses were inspected, not copied wholesale into product source.

Initial failures: web fetch of the inbound-import citation returned 404; the migration and sunset pages were inaccessible through web tooling. `curl -IL` against the migration HTML returned 403, Cloudflare challenge. The inbound-import public API also returned 404 and its downstream JSON parser failed on empty input. The method then changed to the public article/search API, which returned primary documentation successfully. No account or browser was used. Third-party search claims were not accepted as format evidence.

## File inventory and authority

Read/list/search operations used `fast_filesystem`. Complete issue selection came from `assignment.json` key `arc-import-format`. Root manifest and applicable lifecycle/native-space/execution-state authority were inspected. Scoped filename searches found issue export documentation and the extension AppState fixture, with no supplied Arc desktop export. The initial exclusive packet held only `ownership.json`.

`sha256sum` ran over each path listed in `sources.json.localSources`. All returned hashes are retained there, including the exact inspected baseline headers and root-control snapshots. Shared control files may evolve after this snapshot. No hash here claims they are frozen or proves native execution.

## JSON and whitespace checks

The following commands ran from the coordinator worktree and passed:

```sh
jq -e '.issueKey == "arc-import-format" and .nativeAcceptance == "OPEN" and (.supportedArcBuilds | length == 0) and (.supportedExportVersions | length == 0) and (.originals | length == 0) and (.fieldCoverage | map(.byteObserved) | all(. == false))' arcified/native/production/arc-import-format/fixtures.json
jq -e '.issueKey == "arc-import-format" and .nativeAcceptance == "OPEN" and (.ownerQuestions | all(.status == "OPEN")) and .implementationEvidence.productionPatch == null' arcified/native/production/arc-import-format/result.json
jq -e '.issueAuthority.object.key == "arc-import-format" and (.externalSources | length == 3) and (.localSources | length > 0)' arcified/native/production/arc-import-format/sources.json
git diff --check -- arcified/native/production/arc-import-format
```

Each jq assertion returned `true`. `git diff --check` returned exit 0 without output, but does not inspect untracked files. It is supplemental only.

Formatting uses canonical two-space JSON with a final LF. New Markdown uses ordinary headings, blank-separated paragraphs/lists and valid table delimiters. No Markdown formatter/linter executable or root formatting configuration was discovered. No dependency was installed. The packet-wide hosted JavaScript check below reads files through `fast_filesystem` and covers the untracked text directly:

```js
// files are complete fs_read_many results for this packet.
for (const file of files) {
  if (file.error || file.truncated || file.binary) throw Error(file.path);
  if (/\r/.test(file.content) || !file.content.endsWith("\n")) throw Error(file.path);
  if (/[^\S\n]+$/m.test(file.content)) throw Error(file.path);
  if (/^(<<<<<<<|=======|>>>>>>>)/m.test(file.content)) throw Error(file.path);
  if (file.path.endsWith(".json")) {
    const value = JSON.parse(file.content);
    if (file.content !== JSON.stringify(value, null, 2) + "\n") throw Error(file.path);
  }
}
```

The final check also resolves every `evidencePaths` entry and Markdown link within the packet against its complete file inventory. It checks source references, empty fixture/version claims and OPEN acceptance states. These are document consistency checks, not implementation tests.

Outcome: PASS across all six packet files, including unchanged `ownership.json`. Four JSON files parsed and matched canonical formatting; two Markdown files passed full-text whitespace/conflict-marker and local-link checks. All five result evidence paths resolved. The source inventory contains the assigned issue, three primary articles and twelve inspected local file identities. Empty corpus/version claims and per-platform OPEN acceptance assertions passed.

## Verification not performed

No converter exists in this packet. No parser fixture test, native test registration, source preparation, sparse apply, Chromium build/test, benchmark, remote CI, reference-app execution, browser/visual interaction, account action or live-host change was applicable or performed. The prepared Chromium tree was not accessed. Passing checks cannot close Linux, Windows or macOS native import acceptance, same-window multi-Profile isolation, live saved-page handoff, restart durability or owner decisions. No existing pure foundation suite was rerun or presented as native evidence.
