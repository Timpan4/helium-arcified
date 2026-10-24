# Issue 24 ownership core fix response

Recorded: 2026-10-23. Scope: standalone contract/model version 2. Response to the accepted P1 in the unchanged [first independent review](lifecycle-review.md). This response is implementation evidence, not a second independent review or issue-closure claim.

## P1 response

Fixed the command lifetime boundary. All public mutating commands now take owned IDs, attachments and payloads by value. `ClosePage`, `CloseWindow`, private `ErasePage` and shared `RemovePageReferences` therefore retain stable command targets even when their inputs came directly from a split vector, page attachment, attachment set or map key. Read-only queries retain const-reference parameters. Getter views can still be invalidated after mutation; callers are not required to copy an argument before passing it into a command.

`v2/borrowed-page-close` passes `splits().at(split).pages.front()` directly. It verifies deletion of the requested quiescent page, survival of its protected sibling, validity and attachment consistency of every remaining split reference, removal of a separate single-reference split, protected-close refusal, and cleanup when the remaining single reference is closed. `v2/borrowed-window-close` passes a page attachment's window ID and later an attachment-set window ID directly. It verifies refusal before partial cleanup, complete successful local cleanup and survival of the other window's page and shared saved item. `v2/borrowed-mutation-inputs` covers analogous getter-sourced mutation arguments.

## Approved behavior added

The [contract](../lifecycle-contract.md) records exactly the new approvals: selecting an already-live saved tab brings that page to the current window while preserving its state, and Space switching stays in the same native window across Profiles while preserving Profile isolation. Other recommendations remain explicitly unapproved.

`TransferSavedPage` and existing-item selection through `OpenItem`/`OpenShortcut` retain `PageId`, current navigation, activity, immutable Profile and saved-item ownership. They change only the eligible saved page's attachment. All refusals precede mutation. Successful transfer prunes old split references and removes empty layouts within the synchronous single-writer transition. It leaves surviving siblings and layouts valid, creates no destination layout and copies no renderer/history state. Same-attachment selection preserves layouts. Temporary-tab migration is rejected. Space-kept items require their owning Space attached in the requesting window; standalone Profile items can transfer between same-Profile attachments.

`SelectSpace` changes only the selected attached Space of the given logical `WindowId`, including across Profiles. It neither changes any page's Profile nor migrates temporary tabs. Selection and saved-page transfer are separate model commands. Native focus, rendering, authentication and actual live-page transfer are not implemented by these logical updates.

Changed files: `arcified/native/model/lifecycle.h`, `lifecycle.cc`, `lifecycle_test.cc`, `arcified/docs/lifecycle-contract.md`, and this report. The independent review, Windows tooling and other workers' files were not edited. No GitHub, commit or integration action was taken.

## Focused verification

Ran from the repository root on Linux with existing Clang 23.1.3. Formatting passed. Both compiled runs passed all 13 version 2 fixtures. AddressSanitizer and UndefinedBehaviorSanitizer reported no errors. Compiler warnings are errors; no separate model linter/build target exists in this slice.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/model/lifecycle.h \
  arcified/native/model/lifecycle.cc \
  arcified/native/model/lifecycle_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc arcified/native/model/lifecycle_test.cc \
  -o /tmp/arcified-lifecycle-fix-test
/tmp/arcified-lifecycle-fix-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc arcified/native/model/lifecycle_test.cc \
  -o /tmp/arcified-lifecycle-fix-sanitized
/tmp/arcified-lifecycle-fix-sanitized
```

Both executables remain at those paths. No optional cleanup, global package installation, heavy preparation or full browser build was run. GCC was not rerun; Windows/macOS compilation and runtime remain unverified. Version 2 rejects version 1; this is a semantic revision, with no disk format or migration supplied.

## Remaining limits

Native work remains open on Linux, Windows and macOS: real Profile/BrowserContext and PageId binding, transfer of existing WebContents with renderer/history state intact, same-native-window cross-Profile presentation, authentication/cookie/storage isolation, native failure/cancellation coordination, before-unload/discard/activity detection, focus, accessibility and performance. Persistence, restore/restart durability, stale-event reconciliation, non-reused ID allocation, URL validation, asynchronous event ordering, migration transactions and journal recovery remain outside this in-memory model. No browser interaction or authenticated-page test ran.

Owner choices still open: one Profile per Space and settings scope, extra live instances, same-site routing, precise active-work signals and manual-close/Reset policy, populated Space reassignment and cross-Profile migration/source disposition, closing shared state and undo/restoration, and split/Peek/transient presentation. Existing conservative refusals are an evaluation subset, not approval of those product policies. Archive/automatic cleanup and other unimplemented lifecycle features have no inferred retention, timing or capacity policy. AI, provider accounts and cross-device sync remain outside scope. Parent owns further independent review, GitHub status and integration; issue 24 is not closed by these fixtures.

Reusable coding-rule proposal, based on P1: "When a mutating command can receive IDs or selectors borrowed from state it changes, snapshot those values before validation and mutation. Cover that boundary with a regression using arguments directly from public getters." No source standards file was changed.
