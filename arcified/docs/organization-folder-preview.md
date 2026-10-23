# Folder preview projection

Status: standalone preparation for [issue 158](https://github.com/Timpan4/helium-arcified/issues/158). Native acceptance remains open on Linux, Windows and macOS.

The [issue brief](research/arc-dia/publication/folder-preview.md) and matching `folder-preview` record in [backlog.json](research/arc-dia/backlog.json) require searching nested folder contents, exposing hierarchy, and revealing an active saved descendant without expanding the folder. The standalone query consumes the folders component's validated `Snapshot(model, SpaceId)` and the authoritative [lifecycle model](../native/model/lifecycle.h). It owns no folder tree, lifecycle, navigation, focus, or persistence state.

The caller supplies display labels keyed by `ItemId`, the current attachment and an optional observed active `PageId`. Display labels are presentation data, never identities or copied navigation. The query returns owned projections carrying saved item identity, an existing live page identity when present, ordered placement and folder ancestry. Results are observations, not browser commands, reservations, or commit tokens. Callers must query again and revalidate IDs before acting after a mutation.

`ProjectFolderPreview` in [folder_preview.h](../native/features/organization/folder_preview.h) returns `PreviewResult`, a variant of propagated `FolderStatus`, `PreviewError`, or `FolderPreview`. It refuses an unattached context, a folder absent from that Space, an absent active page, or an active page attached elsewhere. The validated snapshot refuses missing or moved lifecycle references. A folder query lists that folder's represented descendants only. Unplaced saved items remain in lifecycle and need explicit merging by a complete sidebar listing, in caller-supplied canonical order. This query does not invent their placement.

An empty search lists every saved descendant in hierarchy order, including children of collapsed nested folders. A nonempty search applies a literal, case-sensitive UTF-8 substring to caller-supplied item labels. This deliberately supplies no locale or ranking policy. Native search matching can be refined during integration. Missing descendant labels cause refusal rather than deriving display text from URLs or current navigation.

The active descendant projection is independent of the search filter. It appears only when the requested folder is collapsed and the supplied active page is a saved descendant in the caller's attachment. An active temporary page is valid but produces no saved descendant. The query does not infer active selection from page creation, map order or folder expansion. Background opens therefore cannot choose another active item. Renaming a label changes only display and matching. Drag-out and reordering belong to the folders component; a fresh projection reflects its new placement while retaining item and page IDs.

## Native work still required

Pointer and keyboard invocation, native popup/tree rendering, accessibility roles and announcements, selection and focus restoration, drag interaction, and display-label event wiring remain unimplemented. An ancestry projection supplies data for assistive technology, not proof of an accessible interface. Native integration must use existing native command and page identity paths described in [native-space-integration.md](native-space-integration.md) and its [source references](native-space-source-references.md), and preserve Profile isolation and saved-page transfer semantics. The same actual native window across Profile switches remains a feasibility gate.

No native browser was built or launched for this component. Synthetic tests cannot prove live renderer/history preservation, restart durability, accessibility or installed browser behavior. Issue 158 must remain open until its installed-build interaction and failure cases pass on Linux, Windows and macOS with source/build identity and visual evidence.

## Focused synthetic checks

[folder_preview_test.cc](../native/features/organization/folder_preview_test.cc) exercises the public query with synthetic `.test` destinations. Checks cover nested order and full ancestry; literal UTF-8 matching and duplicate labels; collapsed active projection independent of filtering; background opens retaining caller-observed selection; renamed labels and folder labels; drag-out to another folder and Space root; live identity in another window; temporary selection; closed pages; missing labels; stale item references and ownership; wrong folder/attachment context; and unrepresented saved items. Owned results retain their observed labels and placement after later changes. Querying leaves modeled navigation, protected activity, attachment, selected Space and folder expansion unchanged.

Local Linux checks passed with Clang and clang-format 23.1.3. The compiler check uses C++17, strict warnings, ASan and UBSan. No sanitizer diagnostics were emitted. These checks compile the standalone lifecycle, folders and preview components together, not Chromium.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/organization/folder_preview.h \
  arcified/native/features/organization/folder_preview.cc \
  arcified/native/features/organization/folder_preview_test.cc
preview_check_dir=$(mktemp -d /tmp/arcified-folder-preview.XXXXXX)
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/organization/folders.cc \
  arcified/native/features/organization/folder_preview.cc \
  arcified/native/features/organization/folder_preview_test.cc \
  -o "$preview_check_dir/folder-preview-test"
"$preview_check_dir/folder-preview-test"
```
