# Issue 24 ownership core review

Reviewed: 2026-10-23. First independent review of the uncommitted `lifecycle-contract.md`, `lifecycle.h`, `lifecycle.cc` and `lifecycle_test.cc`, against the original [issue body artifact](../research/arc-dia/publication/lifecycle-contract.md). Repository HEAD was `09670c97b1c308d0fb31869b3a9a9bc6969bc47f`; these artifacts were untracked, so that commit does not identify their reviewed contents.

Result: changes required. One confirmed P1 finding affects both page and window closing. No implementation files were changed.

## Finding

### P1: Copy close-command IDs before mutating the state that can supply them

Locations: [ErasePage and ClosePage](../../native/model/lifecycle.cc#L235), [CloseWindow](../../native/model/lifecycle.cc#L258), and the public [reference getters](../../native/model/lifecycle.h#L178).

The public getters expose const references to page attachments and split page IDs. The close APIs accept IDs by const reference, but then mutate or erase the storage that may contain the supplied ID. These calls are valid public API usage; there is no caller requirement to copy IDs before invoking a command.

1. `ClosePage(model.splits().at(split).pages.front())` can close a protected page other than the requested page. With split references `[first, second]`, `first` Quiescent and `second` Protected, `ClosePage` validates `first`. `ErasePage` then compacts the split vector with `std::remove` at line 238. Moving `second` into the first slot changes the referenced argument to `second`. The final `pages_.erase(page)` at line 245 deletes `second` without checking its activity. The command returns `kOk`, retains `first`, and leaves a split reference to the deleted `second`. A standalone reproduction produced:

   ```text
   requested quiescent first close: first exists=1, protected second exists=0, remaining split reference=second
   ```

2. `CloseWindow(model.pages().at(first).attachment().window)` uses freed memory. With the window's pages Quiescent, the preflight succeeds and `ErasePage` destroys the page holding the `window` argument. The attachment loop then reads that argument at line 272. A separate standalone run with AddressSanitizer and UndefinedBehaviorSanitizer terminated with:

   ```text
   ERROR: AddressSanitizer: heap-use-after-free
   Model::CloseWindow .../lifecycle.cc:272:20
   freed by Model::ErasePage .../lifecycle.cc:245:10
   called by Model::CloseWindow .../lifecycle.cc:270:5
   ```

Take an owned copy of the command ID before any mutation, for example by accepting the affected IDs by value. Ensure the private erase helper cannot retain an alias into a split vector either. Add regression fixtures that pass IDs directly from the public getters. For page close, assert that the requested page disappears, the protected sibling survives, and every remaining split reference resolves. For window close, pass the window ID from a page attachment and verify complete local cleanup under sanitizers. Include the single-reference split case, where deleting the layout can also invalidate the borrowed page ID.

The current close fixtures pass independent namespace constants such as `kMailPage` and `kFirst`, so they do not exercise this lifetime boundary. This finding concerns implemented invariants and protected-work handling, not an unresolved product choice.

## Other reviewed behavior

The status-returning refusal paths inspected validate references, Profile compatibility and activity before changing state. Subject to the finding above, the synthetic model keeps saved destinations separate from current navigation, gives shared saved items one identity, retains temporary tabs in their original attachment, preserves Profile identity on live pages, and treats splits as page references. Cross-Profile copy uses the saved destination and retains the source. Deletion/reassignment refusals prevent the modeled reference graph from becoming orphaned. No additional concrete invariant failure was found in this review.

The documentation explicitly identifies one Profile per Space, cross-window activation, same-site routing, active-work policy, migration, native window Profile scope and split presentation as proposals or unresolved decisions. It does not record those recommendations as owner approvals. The two owner-approved ownership rules are distinguished from that evaluation model. Native reads, synthetic execution and native runtime evidence are also kept separate.

The single-live-page policy means selecting a saved item from another window only returns its existing location. Closing that owning window drops its live state while retaining its saved destination. This is consequential for Arc-style multi-window browsing, but the contract exposes it and leaves focus, extra instances and restoration to owner decisions and future adapters. It is not evidence that switching, rendering or session restoration works in a browser. The same distinction applies to conservative close/reset refusals and closed-alias attachment requirements.

## Verification and remaining limits

Independently ran these focused checks on this Linux host with Clang 23.1.3:

```sh
clang-format --dry-run --Werror --style=Chromium \
  arcified/native/model/lifecycle.h \
  arcified/native/model/lifecycle.cc \
  arcified/native/model/lifecycle_test.cc
clang++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc arcified/native/model/lifecycle_test.cc \
  -o /tmp/arcified-lifecycle-review-fixtures
/tmp/arcified-lifecycle-review-fixtures
```

Formatting passed. All eight supplied fixtures passed. Additional small public-API reproductions were compiled from standard input with `-g -fsanitize=address,undefined` against the unchanged `lifecycle.cc`. They demonstrated the wrong protected-page deletion and the window-close use-after-free described above. The sanitizer failure is a confirmed model defect, not a Chromium integration result.

No heavy build, browser interaction, native Chromium integration/runtime, persistence/recovery, authentication isolation, Windows/macOS or cross-platform verification was performed. GCC was not rerun; the worker's reported missing `cc1plus` supplies no GCC evidence. AI and cross-device sync remain outside scope. Issue 24's remaining owner decisions and native evidence are still open.
