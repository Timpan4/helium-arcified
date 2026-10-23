# Organization folders

Status: standalone C++ metadata component for [issue 107](research/arc-dia/publication/folders.md). This implements a useful subset of the [folders backlog record](research/arc-dia/backlog.json). It does not implement a native sidebar or establish native acceptance. The issue remains open.

## Authority and stored state

[Folders](../native/features/organization/folders.h) references the authoritative [lifecycle model](../native/model/lifecycle.h), under [contract version 3](lifecycle-contract.md). Each Space has an ordered root list. Every folder has a stable `FolderId`, label, expansion flag and ordered children. Each child is either a `FolderId` or `lifecycle::ItemId`. Parent and Space membership follow from that tree. An entry has one location, and moving it preserves its identity and subtree order. There is no nesting limit or recursive traversal.

Only Space-kept persistent items can be placed. Profile-owned items and shortcut aliases belong to the separate Profile Favorites presentation. Folder metadata stores no saved destination, saved-item custom label, Profile mapping, live page, navigation, activity, window attachment, focus, selection or renderer state. The same Space hierarchy serves every attached window. Temporary tabs remain window-local in lifecycle and never enter a folder.

The approved contract makes saved Space items shared across windows and lets selection transfer their existing live page while preserving state. Hierarchy changes neither request nor perform that transfer. Same-window cross-Profile Space switching remains a native engineering gate. The [native integration design](native-space-integration.md) and [source references](native-space-source-references.md) identify the immutable desktop Browser/Profile coupling. This component does not resolve that coupling or change authentication isolation.

## Commands and reads

The embedding coordinator serializes folder commands with lifecycle changes on one writer sequence. It allocates stable, non-reused folder IDs. Empty IDs/labels and currently duplicate folder IDs are refused. Labels are display text, never identities. No normalization, truncation, import parser or allocator is implemented.

Commands take a current `const lifecycle::Model&` and own all other arguments. `Placement` names a Space, an optional destination folder, and an optional direct sibling to insert before. Omitting the folder selects the root; omitting the sibling appends. An anchor must already be a direct destination child. Moving before oneself in the same container succeeds without changing order. Anchors avoid caller index arithmetic when removal changes positions.

| Operation | Implemented behavior |
| --- | --- |
| `CreateFolder` | Adds an expanded empty folder to the requested position. Folder IDs are unique across this component's Spaces. |
| `Rename` | Changes the folder label only. |
| `SetExpanded` | Changes one folder's expansion metadata. |
| `SetAllExpanded` | Expands or collapses all folders in one existing Space, leaving other Spaces alone. An unrepresented existing Space succeeds without creating a tree. |
| `PlaceItem` | Places an unrepresented Space-kept item or relocates its existing entry in the same Space. Never creates a lifecycle item. |
| `MoveFolder` | Relocates one folder with its descendants, retaining internal order. Self-parenting and movement under any descendant are refused. |
| `Snapshot` | Returns an owned `Hierarchy` for one Space or an explicit validation error. An existing Space without metadata returns empty roots/folders. |
| `ReadFolder` | Resolves the folder's containing Space and returns that Space's validated hierarchy. |

`SnapshotResult` is `std::variant<FolderStatus, Hierarchy>`. A successful read contains `Hierarchy`, never `FolderStatus::kOk`. `Hierarchy` exposes `space`, ordered `roots` and a folder map whose values hold `label`, `expanded` and ordered `children`. `Locate(entry)` returns a parent folder or root and direct-child index. `Ancestors(entry)` returns root-to-parent folder IDs, excluding the entry itself. Both return `nullopt` for an absent entry; a represented root entry has an empty ancestor vector. Earlier snapshots remain owned values unaffected by later commands. Consumers must use the current serialized lifecycle view rather than treating a previous snapshot as a reservation.

Every read and mutation validates the requested Space and all represented item references within it against the supplied model. A missing Space returns `kSpaceNotFound`. Missing items return `kItemNotFound`; items no longer owned by that Space return `kItemOwnerMismatch`. Validation failure leaves metadata unchanged and returns no partially valid hierarchy. A stale tree in another Space does not block this Space. A stale tree is not silently pruned, migrated or repaired. The coordinator must resolve it under a future lifecycle agreement. Supplying an unrelated model that happens to reuse the same IDs cannot be detected here; the caller must supply the authoritative workspace model.

Missing folders/anchors return `kNotFound`, invalid labels/IDs return `kInvalidInput`, and duplicate IDs return `kDuplicateId`. Invalid descendant moves return `kCycle`. Folder moves between Spaces and placement targeting a folder in a different Space return `kCrossSpaceUnsupported`. An item currently belonging to another Space/Profile returns `kItemOwnerMismatch`. If lifecycle has externally moved a represented item to a different Space, placing another reference there is also refused with `kCrossSpaceUnsupported` rather than duplicating the stale reference.

The class exposes no import setter or mutable storage views. Internal topology comes only from the validated commands. Owned snapshots may be inspected by preview and transient multi-select consumers; changing a snapshot cannot change the component's metadata.

## Explicit merge contract for unrepresented items

Folder metadata is a partial organization of lifecycle items. Omission means unplaced, never deleted, archived or hidden. `Snapshot` does not automatically insert missing items or assign them a user order.

1. A caller rendering the complete saved-item view reads a current validated snapshot and the same authoritative lifecycle view.
2. It retains all represented item identities in their explicit hierarchy positions.
3. It appends remaining Space-owned lifecycle `ItemId`s in the caller's canonical order for unplaced items. It checks that this order covers the remaining identities without duplicates. Map key order is not user order. A caller lacking such an order must leave that presentation requirement unresolved, rather than omit items or pretend an arbitrary order was chosen by the user.

Folder-only preview may intentionally read one subtree without merging other saved items. Item display labels come from the saved-item label owner, supplied separately to consumers. Folder ancestry and labels can provide accessibility context without copying item labels or navigation into this component.

## Focused synthetic checks

[folders_test.cc](../native/features/organization/folders_test.cc) uses synthetic IDs and reserved `.test` destinations. Its seven fixtures check mixed root/folder ordering, item reparenting, subtree moves, later-sibling anchors and self-anchor no-ops; cycles and refusal without partial mutation; rename and Space-scoped expansion; invalid identities, ownership and anchors; stale lifecycle moves/removal without silent pruning; shared Space metadata alongside local temporary pages and a protected live saved page; and nested ancestry/read queries. Equal saved URLs retain distinct item identities. Unplaced lifecycle items remain present. Live navigation, saved destination, Profile, attachment and protected activity stay intact during organization commands. A subsequent synthetic lifecycle transfer leaves the hierarchy intact.

Run from the repository root with the existing toolchain:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/organization/folders.h \
  arcified/native/features/organization/folders.cc \
  arcified/native/features/organization/folders_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/organization/folders.cc \
  arcified/native/features/organization/folders_test.cc \
  -o /tmp/arcified-organization-folders-test
/tmp/arcified-organization-folders-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/organization/folders.cc \
  arcified/native/features/organization/folders_test.cc \
  -o /tmp/arcified-organization-folders-sanitized
/tmp/arcified-organization-folders-sanitized
```

These are standalone component checks using the existing lifecycle implementation, not Chromium build or installed-browser checks. Native platform, visual and accessibility behavior remain unverified.

## Remaining gates and decisions

The issue's native acceptance needs keyboard/context-menu/drag parity, opening folder contents through the approved lifecycle path, and Linux, Windows and macOS installed-build interaction evidence. No UI or native page actuation is added here.

Cross-Space folder moves require agreement with lifecycle about every referenced item's ownership and failure handling. Same-Profile availability does not authorize bypassing the current lifecycle move restrictions. Populated cross-Profile migration/source disposition remains open. This component refuses all cross-Space folder moves and supplies no batch transaction or rollback promise.

Folder deletion, undo and restore semantics remain open under the destructive-action contract. There is no removal API, implicit cascade or page closure. Persistence needs an approved versioned schema, durable ID allocation, whole-graph validation, session joins and recovery behavior before hierarchy restoration can be claimed. No disk format, journal, native adapter, build wiring or restart acceptance is implemented. Unit evidence does not close issue 107.
