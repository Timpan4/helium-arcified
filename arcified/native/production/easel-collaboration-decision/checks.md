# Checks and verification limits

Worktree: `/home/timpan4/.t3/worktrees/helium-arcified/feat-non-ai-research-production`.
Baseline: `a4b6b480d3de07857ab6e939149254a8359e5449`.
Research retrieval time: 2026-10-23 22:29:28 UTC.

## Commands and outcomes

1. `git status --short; git rev-parse HEAD; git branch --show-current`
   returned exit 0. HEAD matched the baseline and branch was
   `feat/non-ai-research-production`. Existing untracked production directories
   belong to shared coordinator work. No git write was performed.
2. `command -v node bun prettier markdownlint-cli2`
   returned exit 1 because the formatter/linter executables were absent.
   Node and Bun were available. No dependency was installed.
   Root native-integration documentation records no declared Markdown
   formatter/linter. The extension package declares only code build/typecheck/test
   scripts, which do not validate this decision packet and were not run.
3. `sha256sum` with the explicit source paths in
   `source-identities.sha256` returned exit 0 and generated the identities retained
   there and in `sources.json`.
4. `sha256sum -c arcified/native/production/easel-collaboration-decision/source-identities.sha256`
   returned exit 0. All 14 recorded source files matched their exact bytes,
   including root authority and the existing foundation evidence. This proves
   source identity at verification time, not that those sources implement native UX.
5. `git diff --no-index --check /dev/null arcified/native/production/easel-collaboration-decision/decision.md`
   returned exit 1 with no output. No whitespace diagnostic was emitted; exit 1
   reflects the new-file difference in no-index mode.
6. `fast_filesystem.fs_read_many` read the packet's JSON/Markdown/hash files.
   Hosted JavaScript parsed JSON with `JSON.parse`, required canonical
   `JSON.stringify(value, null, 2) + "\n"`, and checked final newlines and absence
   of trailing spaces. Final validation also compared the copied issue object
   with the previously read assignment and checked required result fields,
   pending owner choice, dependency list and native acceptance. Passed.
7. Hosted JavaScript extracted every relative Markdown link in `decision.md`,
   normalized it against this packet path, and
   `fast_filesystem.fs_stat_many` checked each target. The initial pass found
   the planned `checks.md` and `result.json` absent before they were authored.
   The final pass after authoring found every linked file present.
8. `fast_filesystem.fs_list` inspected the final packet inventory. Authored files
   are `authority.json`, `sources.json`, `source-identities.sha256`,
   `decision.md`, `checks.md` and `result.json`. The existing
   `ownership.json` remains unchanged.

## Retrieval limitations

The assigned Easels URL returned HTTP 404 when directly opened. Search returned
cached first-party text for that same URL, described by the tool as crawled five
months earlier. The macOS release-note page opened successfully and contained
the dated presence and edit-permission entries cited in the decision.
The sunset page could not be retrieved. No assertion of current vendor service
availability or shutdown/export behavior was verified.

A scoped search initially included a nonexistent `arc.md` path, which returned an
actionable path error. Directory inspection identified the existing `arc-ux.md`;
that file was read with fast_filesystem. The corrected coverage/UX search
completed without truncation or errors.

## Checks deliberately not run

No preparation, patch application, build, native test, benchmark, remote CI,
account access, publication, browser or visual interaction ran. No prepared-tree
files were modified. Native patch/test registration and syntax checks for native
code are inapplicable because this is a decision leaf with no code changes.
Existing pure model/component tests were not repeated and are not evidence of
native collaboration, same-window multi-Profile switching or saved-page handoff.

Linux, Windows and macOS runtime/visual acceptance remain **OPEN**. Independent
review is for the coordinator to arrange; this primary did not delegate or
claim an independent review.
