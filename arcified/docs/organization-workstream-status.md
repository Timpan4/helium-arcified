# Organization workstream

Checkpoint: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. Branch: `feat/organization-workstream`.

## Assignments

All three T3 primaries were accepted with actual provider `codex`, model `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`.

| Issue | Task ID suffix | Owned files under `native/features/organization/` | Stage |
| --- | --- | --- | --- |
| #107 folders | `organization-folders-primary-20261023` | `folders.h`, `folders.cc`, `folders_test.cc` | Reviewed local component, ready for root integration |
| #158 folder-preview | `organization-folder-preview-primary-20261023` | `folder_preview.h`, `folder_preview.cc`, `folder_preview_test.cc` | Reviewed local component, ready for root integration |
| #68 multi-select | `organization-multi-select-primary-20261023` | `multi_select.h`, `multi_select.cc`, `multi_select_test.cc` | Reviewed local component, ready for root integration |

The common full task ID prefix is `node:delegated-task:command%3Amcp%3Aaf93d686-be0b-4ce9-8cfa-5e68501cec4f%3Adelegate-task%3A`. Each primary also owns its corresponding `organization-*.md` component document. The coordinator owns this status record.

## Interfaces

`organization::Folders` owns Space-kept hierarchy, sibling order, folder labels and expansion metadata, referencing authoritative lifecycle IDs. Validated owned `Hierarchy` snapshots feed preview and multi-select. No saved destination, live navigation, Profile ownership or focus is copied. Unplaced items remain intact; complete views require a caller-supplied canonical order for omitted items.

Folder preview computes search and active-descendant projections without changing expansion, focus or page state. Item display labels come from the tabs area's presentation seam. Selection owns only transient selected IDs and its range anchor. Move plans are owned snapshots for same-Space organization edits, not native transactions or execution authority.

Tabs accepted the shared contract. Organization supplies Space-kept folder order. Pins exposes context-free `DescribeSavedItem(ItemId)` and caller-value optional `custom_label` with reversible edit values, without storage/history. Preview accepts display labels supplied by that seam. Favorites provides a stateless Profile item/alias projection and validates optional exact caller-authoritative order. Temporary order and active page remain native host inputs. No competing order/label registry is introduced.

Tabs reported reviewed commit `5bf00173d40d9e4f5e2e4e56d2b72d9048a35863`. Its real API is `pins/saved_item.h`: `SavedItemMetadata{optional<string> custom_label}` and `DescribeSavedItem(Model, ItemId, metadata, default_label)`. Root must integrate that branch and adapt its owned label result into preview's `map<ItemId, string>` input. This cross-branch join has not been compiled here.

## Open proof and next action

Reviewed local components: 3. Awaiting root integration/release: 3. In progress: 0. Not started: 0. Native issue acceptance: 0 of 3 proven.

Independent review task suffix: `organization-independent-review-round1-20261023`, using the same task prefix and verified model/options above. It completed with no actionable local findings and no pending children. All primaries completed without pending children. The [review evidence](organization-review.md) pins reviewed source hashes. Folders passed strict Clang/GCC C++17 and ASan/UBSan checks. Preview and multi-select passed strict Clang C++17 and ASan/UBSan checks. The independent reviewer reran all three strict Clang C++17 ASan/UBSan test executables. The coordinator and reviewer checked Chromium formatting across all nine C++ files and staged whitespace; both passed.

Next: root integrates this area branch and tabs' label seam, then verifies the combined caller path. No native interaction or platform evidence exists for these new components. Local commits do not close the issues.

Native host ownership extraction, production event/command wiring, persistence and recovery, pointer/keyboard/menu/accessibility integration, and Linux/Windows/macOS installed-build evidence remain open. Cross-Space batch coordination and populated cross-Profile migration are not supplied. Destructive actions, activity protection and cleanup policy remain owner decisions. Full Chromium preparation/build and remote CI are deferred.
