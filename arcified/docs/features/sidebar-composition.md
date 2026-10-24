# Sidebar composition conformance

This slice adds [composed C++17 scenarios](../../native/integration/sidebar/sidebar_composition_test.cc) that call the reviewed components against one lifecycle v3 model. It adds no production facade. Native preparation, browser build and Linux/Windows/macOS platform acceptance remain **OPEN**. Fresh independent review of this join by the parent remains required. The earlier independent component reviews do not cover this integration.

## Supported join and remaining adapter input

The concrete UTF-8 join is `pins::DescribeSavedItem(model, canonical_item, metadata, default_label)` followed by an `ItemId -> label` map passed to `organization::ProjectFolderPreview`. The fixture's `Preview` method exercises that call chain. Folder queries require labels for every saved descendant, including descendants hidden by collapse or excluded by search. They do not require labels for unrelated items. A preview is neither a sidebar listing nor a visibility decision.

Favorite aliases first resolve through `favorites::ResolveFavorite` and `workspace_organizer::ResolveSidebar`. Both yield the same canonical `ItemId`; `favorites::DescribeFavorite` uses that item's metadata. The scenarios edit that canonical metadata through `pins::PlanLabelEdit` and `ApplyLabelEdit`, then observe the new label through the alias and folder preview. They retain the old owned preview to check snapshot behavior.

The accessibility contract requires a complete, already labeled visible logical preorder. Its names are borrowed UTF-16 views, whereas pins and folder previews use UTF-8 strings. The headers provide no native encoding adapter, localization provider, presentation ID bindings or complete sidebar ordering source. There is no production sidebar caller in this slice from which to extract duplicated adapter wiring. A combined production seam would therefore have to invent policy or accept all its answer as input. The existing functions are used directly instead.

The synthetic caller supplies explicit paired UTF-8/UTF-16 label fixtures and reuses the same native UTF-16 name for the alias and canonical row. After an edit, it explicitly supplies the new native name. This proves semantic changes given authoritative labels; it does **not** prove UTF conversion or automatic native label synchronization. The fixture row functions are scenario data, not a proposed host adapter or visibility algorithm.

The remaining native adapter must supply these facts on one serialized native/model sequence:

1. The actual requesting window/Space attachment, the canonical reference bound to each row, stable non-reused presentation IDs, and current native object validity. `RowId` must not be derived from an item name, URL or index. An alias retains its own row identity while using its target's metadata.
2. The complete visible preorder, including offscreen rows, with collapsed descendants excluded. Organization metadata describes explicitly placed saved items only. Unplaced saved items need the caller's canonical order; Favorites and temporaries need their own validated complete caller order. Search matches and workspace inspection iteration cannot stand in for that order. The fixture intentionally places `z-saved` before `a-unplaced` and includes the alias and temporary row.
3. Native UTF-16 display names corresponding to the current canonical saved label, localized Space/folder/temporary names and command names, parent links, optional selected/expanded states and advertised command capabilities. Keep all borrowed rows and backing strings immutable and alive through projection comparisons. Native encoding/localization remains an input, with no conversion, registry or fallback supplied here.
4. Actual native active-page and focused-row observations, text direction, and selection gestures. Selection, active page, focus suggestions and handoff focus intent remain separate. The caller must apply successful organization/native changes and then publish fresh rows. A collapsed folder's preview can identify an active descendant while that descendant has no visible semantic row.
5. Fresh row binding, command capability, attachment, native page/object and focus validation before dispatch. Resolve again after mutations and revalidate an unchanged handoff plan with current native source focus. After native success, report the observed lifecycle change and refresh the projection. These tests call `TransferSavedPage` only as a synthetic model observation; they provide no native executor, atomic batch move, native failure/cancellation transaction, ABA detection or focus application.

## Composed coverage

| Scenario | Observable evidence |
| --- | --- |
| Canonical alias labels and complete preorder | Favorite and workspace alias resolution agree on one item/page; canonical label edits reach alias and folder preview; semantic names change on both rows; hierarchy levels and sibling counts include unplaced/offscreen rows; Down reaches the unplaced item and End reaches the temporary row; surviving or outside focus is retained. |
| Selection, move intent and collapsed active descendant | Organization selection supplies the saved row's selected state; a live saved item produces a same-Space move preview without moving; a semantic collapse request leaves organization unchanged until explicitly applied; search can have no matches while the collapsed folder reports its active saved descendant; the fresh semantic projection excludes hidden descendants and suggests the surviving folder for lost child focus. |
| Live alias handoff and current attachment staleness | An enabled semantic alias command resolves to a live canonical item and prepares a handoff from its actual source into its owning Space in the requesting window; focus and split effects are explicit; changed source focus, detached requesting attachment and detached owning-Space destination refuse stale intent; an old command snapshot cannot authorize dispatch; refreshed capability refuses the request; after a synthetic transfer, sidebar resolution/inspection report the new attachment while the old plan becomes stale. Page identity, current navigation, protected activity, saved destination and caller-selected Space remain observable. |

The existing standalone tests remain the coverage for unchanged primitives, malformed inputs and their wider policy matrices. This file adds cross-area scenarios rather than copying those suites. It uses actual pins, Favorites, temporary-tabs, organization, accessibility, workspace inspection and window handoff implementations. It does not mock those calls.

## Focused checks

Run from the repository root with the installed Clang tools. No dependencies, full browser build, downloads, native UI, remote writes or commit are required.

```sh
sources=(
  arcified/native/model/lifecycle.cc
  arcified/native/features/pins/saved_item.cc
  arcified/native/features/favorites/favorites.cc
  arcified/native/features/temporary_tabs/temporary_tabs.cc
  arcified/native/features/organization/folders.cc
  arcified/native/features/organization/folder_preview.cc
  arcified/native/features/organization/multi_select.cc
  arcified/native/features/accessibility/sidebar_accessibility.cc
  arcified/native/features/workspace_organizer/inspection.cc
  arcified/native/features/window_ownership/handoff.cc
  arcified/native/integration/sidebar/sidebar_composition_test.cc
)
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  "${sources[@]}" -o /tmp/arcified-sidebar-composition-test
/tmp/arcified-sidebar-composition-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-sanitize-recover=undefined \
  -fno-omit-frame-pointer "${sources[@]}" \
  -o /tmp/arcified-sidebar-composition-sanitized
/tmp/arcified-sidebar-composition-sanitized
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/integration/sidebar/sidebar_composition_test.cc
clang-tidy --checks='-*,clang-analyzer-*' --warnings-as-errors='*' \
  arcified/native/integration/sidebar/sidebar_composition_test.cc \
  -- -std=c++17 -Wall -Wextra -Werror -pedantic
git diff --no-index --check /dev/null \
  arcified/native/integration/sidebar/sidebar_composition_test.cc
git diff --no-index --check /dev/null \
  arcified/docs/features/sidebar-composition.md
```

The per-file whitespace checks include untracked additions. Exit 1 denotes a new-file difference; whitespace diagnostics would indicate a failure. The explicit analyzer checks avoid relying on the host's empty default clang-tidy check selection. Undefined behavior is nonrecovering in the sanitizer invocation.

On 2026-10-23, Clang/clang-format/clang-tidy 23.1.3 on Linux x86_64 passed the commands above. Both executables exited 0 and printed `PASS: sidebar composed conformance scenarios`. ASan/UBSan emitted no diagnostics; undefined behavior was configured as nonrecovering. The Chromium formatting check and explicit analyzer lint exited 0 without diagnostics. Both new-file whitespace checks emitted no diagnostics and returned 1 for the new-file differences. The unchanged standalone primitive suites were reused as existing coverage, not rerun or claimed as new execution evidence.

These standalone scenarios cannot prove rendering, native keyboard dispatch, native attachment preparation, AX nodes/events, assistive technology behavior, focus application or browser/platform acceptance. The production host remains absent. No browser UI check ran. Native preparation/build and platform acceptance remain OPEN, and parent independent review of this join is still required. Only the scenario file and this document were added; there was no full build, download, remote write or commit.
