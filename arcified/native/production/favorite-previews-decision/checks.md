# Packet checks

Scope: documentary decision evidence only. No production code changed. No applicable project Markdown/JSON formatter or linter was found in tracked configuration; the extension's scripts target its TypeScript build/tests. No tools or dependencies were installed.

## Commands and outcomes

Run from `/home/timpan4/.t3/worktrees/helium-arcified/feat-non-ai-research-production`.

```sh
git rev-parse HEAD
git branch --show-current
git status --short -- arcified/native/production/favorite-previews-decision
```

Observed baseline `a4b6b480d3de07857ab6e939149254a8359e5449`, branch `feat/non-ai-research-production`, packet untracked. No staging, commit or branch mutation occurred.

```sh
command -v node jq prettier markdownlint
git ls-files '*prettier*' '*markdownlint*' '*editorconfig*' '*biome*'
```

Node and jq were available. Prettier and markdownlint were absent. No matching tracked formatter/linter configuration was returned. Applicable JSON files use jq-compatible two-space formatting and final newlines. Markdown tables, headings and links were reviewed as source, without rendering.

```sh
sha256sum --check arcified/native/production/favorite-previews-decision/inputs.sha256
```

PASS. All recorded input byte identities matched. Hashes cover the root authorities, read foundation headers/reports and read-only prepared chrome/VERSION. This proves input identity at check time, not root native behavior or a prepared-tree build.

```sh
jq -e . arcified/native/production/favorite-previews-decision/ownership.json arcified/native/production/favorite-previews-decision/assignment.snapshot.json arcified/native/production/favorite-previews-decision/source-identities.json arcified/native/production/favorite-previews-decision/sources.json arcified/native/production/favorite-previews-decision/result.json > /dev/null
```

PASS. JSON syntax parsed for every packet JSON file.

```sh
for file in arcified/native/production/favorite-previews-decision/*; do
  git diff --no-index --check /dev/null "$file"
  packet_check_status=$?
  printf '%s %s\n' "$packet_check_status" "$file"
done
```

PASS. No whitespace diagnostics. Each individual command returned 1 because `/dev/null` differs from a new file; the loop completed with exit 0. The individual check exit statuses and final outcome are retained in the task tool results.

The first attempt assigned the exit code to zsh's read-only `status` variable and stopped with `zsh:3: read-only variable: status`. The corrected command above uses `packet_check_status` and was rerun. The failed attempt supplied no complete whitespace-check evidence.

## Programmatic packet check

The focused `functions.exec` check reads every packet file with `fast_filesystem.fs_read_many`, refuses truncated/error results, parses every JSON object, compares each JSON file byte-for-byte with `JSON.stringify(value, null, 2) + "\n"`, checks LF/final newline/trailing whitespace, validates required result fields, resolves all local Markdown links and evidence paths inside the packet, and checks exact issue-object equality with the authoritative issue loaded from the assigned root snapshot. PASS.

It also checks that every named preview site has a matrix row and that `nativeAcceptance` remains `OPEN` in ownership and result. These are artifact consistency checks. They do not test account integrations or native scenarios.

## Source checks and unavailable execution

Public `web.run` open/find/search checked S1-S4. Sections and retrieval limitations are recorded in sources.json. U1/U2 returned `(404) Not Found`. U3 direct open returned `(400) Timeout fetching`; the archive entry was readable through S1. No absence search is treated as feature retirement; only the explicit GitHub release note supports that judgment.

Linux, Windows and macOS native execution, visual/browser checks, reference app execution, account reads, credential access, native patch application, GN test registration/builds, benchmarks and remote CI did not run. They are unnecessary or unauthorized for this decision packet. Native acceptance remains OPEN. The coordinator owns independent review and owner-decision consolidation.
