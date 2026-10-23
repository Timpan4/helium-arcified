# Space management metadata

This slice of [issue 64](https://github.com/Timpan4/helium-arcified/issues/64) implements caller-owned Space display identity and order values, pure transformations, and an advisory lifecycle reference inventory. Standalone synthetic C++ checks pass. It does not implement a native Space management interface, removal, archive, migration or undo. The [durability join](session-durability.md) now preserves its canonical values in the standalone codec and staged restore; installed persistence remains open. The issue remains open.

## Authority and shared interface

The [lifecycle v3 contract](../lifecycle-contract.md) and [Model](../../native/model/lifecycle.h) remain the sole authority for Space existence, Profile mapping, ownership, navigation, attachments and selected Space. Every operation takes a const Model. No registry, selected-page store, allocator, observer, Profile settings store or navigation state is added. Organizer consumers can read lifecycle directly without depending on these transformations.

The coordinator accepted the metadata shape before implementation. The shared header is [space_management.h](../../native/features/space_management/space_management.h), with these exact types in `arcified::space_management`:

```cpp
struct DisplayIdentity {
  std::optional<std::string> name;
  std::optional<std::string> icon_key;
  std::optional<std::string> theme_key;
};
struct SpaceMetadata {
  lifecycle::SpaceId space;
  DisplayIdentity identity;
};
using Metadata = std::vector<SpaceMetadata>;
```

The vector sequence is display order. A name, icon or theme never identifies a Space or maps it to a Profile. Spaces sharing a Profile may have independent display identities and themes. Sign-ins, site settings and Favorites remain Profile scoped under v3. The header is the DTO owner; consumers must not define another copy of its shape.

Missing optional values request presentation defaults supplied by the caller. Present empty strings remain distinct from missing values. Names need not be unique. The component imposes no length, capacity, retention or resource-loading policy. Icon/theme keys are opaque display resource identities, not a file or network loading instruction. Encoding, resource resolution and presentation validation belong at their actual IO/UI boundaries.

## Pure operations

`ValidateMetadata` requires exactly one record per existing Model Space, including unattached Spaces. It reports duplicate or unknown input IDs, then missing Model IDs. Display payloads do not change lifecycle validity.

`ReconcileMetadata` returns an owned `Reconciliation` value. It preserves existing known records and their relative order. Unknown records move to the returned `removed` collection with their complete display payload for caller review or recovery. Missing Model Spaces append with unspecified display values in deterministic Model iteration order and are listed in `added`. This reported fallback is not claimed to be the user's desired order. Duplicate IDs, including duplicate unknown IDs, are ambiguous and refused. Reconciliation neither creates nor removes a lifecycle Space and is not permission to delete stale durable records.

`ReplaceIdentity` replaces a Space's complete display identity. To rename only, copy that Space's identity, change `name`, and pass the result. `ReorderSpaces` accepts a complete permutation of Model Space IDs. Both require already reconciled metadata and refuse unknown, duplicate or missing IDs. Neither operation changes Model or recreates items/pages. Input values are owned copies, including command IDs and display payloads borrowed from caller metadata.

The caller can retain the original value while preparing or declining a change, then explicitly adopt the returned value after its own coordination succeeds. Pure return values do not provide a native transaction, durable commit, recovery journal or undo guarantee. The coordinator must serialize access to Model and metadata and revalidate after intervening lifecycle changes. There is no asynchronous synchronization mechanism here.

## Advisory reference inventory

`InspectReferences` returns an owned `SpaceReferences` snapshot for an existing Space, or `nullopt` for a missing Space. It reports Space-owned kept items, pages attached to the Space, aliases targeting those kept items, attachments, splits and windows currently selecting the Space. Attached pages can include local temporary pages and Profile-owned shortcut pages. Their presence does not imply that the Space owns their persistent items. Same-Profile Spaces are not combined.

The snapshot is advisory. It may become stale after any lifecycle mutation. It is neither a reservation nor authorization, destructive eligibility, a disposition plan or native success. It contains existing lifecycle IDs only, with no copied destinations or navigation. Folders, routing rules, native-only references and Profile data are not represented in this inventory and are not claimed covered.

Activity detection, cleanup, archive/delete policies and populated cross-Profile migration remain **OPEN**. This component has no mutating lifecycle calls and does not compose conservative lifecycle removals into a product deletion policy. Profile safety peers own any future approved mutation/migration transaction.

## Persistence seam

The [schema-2 durability join](session-durability.md) uses this exact header and
`ValidateMetadata`. Capture requires the complete ordered vector. Encoding
preserves each optional name/icon_key/theme_key, including explicit empty strings,
and explicit display order. Restore returns the staged Model and metadata
together in `RestoredSession`. Whole-snapshot and candidate admission bind both;
missing, duplicate, unknown, forged or stale metadata refuses without changing
source images. No persistence path calls `ReconcileMetadata` or supplies defaults.

Schema 1 is an actual prior local format that omitted these values. No complete,
lossless prior-format transform is registered, so its exact images are retained
as unsupported. The upgrade gate remains OPEN. Folders, custom theme/palette
payload beyond theme_key, split presentation and other omitted payloads still
require explicit absence. Source/component codec checks prove neither installed
restart durability nor native adoption of restored values. Details and focused
evidence are in [the response](../reviews/space-metadata-durability-response.md).

## Focused synthetic checks

[space_management_test.cc](../../native/features/space_management/space_management_test.cc) uses synthetic IDs and reserved `.test` destinations without starting a browser or accessing browsing data.

| Check | Evidence covered |
| --- | --- |
| `IdentityAndOrderPreserveGraph` | Identity replacement, duplicate display names, independent metadata for shared Profiles, complete reorder and no-op reorder. Existing item/page addresses and IDs, saved/current navigation, protected activity, Profile mapping, window selection, local temporaries, aliases and splits survive. |
| `ReconcileWithoutLosingUnknownPayload` | Relative order and stale display payload retained, missing Space reported, reconciliation idempotent, later lifecycle additions/removals handled without metadata operations mutating Model. |
| `RefuseAmbiguousOrStaleInputs` | Duplicate/missing/unknown permutation entries and metadata, unknown target and duplicate stale records refused while caller input and populated lifecycle graph remain intact. |
| `DisplayValuesAndEmptyModels` | Empty graph/order behavior, missing target, explicit defaults, Unicode display name, present empty icon key and resetting optional values. |
| `InspectOnlyExistingLifecycleReferences` | Kept versus Profile-owned/attached page scope, aliases, multiple window attachments, splits, selected windows, missing Space, and owned snapshots remaining advisory after Model changes. |

Run from the repository root using the existing compiler and formatter, following the standalone model's toolchain convention:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/space_management/space_management.h \
  arcified/native/features/space_management/space_management.cc \
  arcified/native/features/space_management/space_management_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/space_management/space_management.cc \
  arcified/native/features/space_management/space_management_test.cc \
  -o /tmp/arcified-space-management-test
/tmp/arcified-space-management-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/space_management/space_management.cc \
  arcified/native/features/space_management/space_management_test.cc \
  -o /tmp/arcified-space-management-sanitized
/tmp/arcified-space-management-sanitized
```

Both test executables passed on the current Linux host, including AddressSanitizer/UndefinedBehaviorSanitizer. Strict compiler warnings and the Chromium-style format check passed. These checks prove in-memory metadata/ref-inventory behavior only. They do not prove native page/renderer continuity, persistence or native interactions.

## Remaining acceptance

The [issue brief](../research/arc-dia/publication/space-management.md) and matching backlog entry require native identity changes, disposition preview and recoverable cancellation/undo/failure. This slice proves pure metadata transformations preserving a populated Model graph and a limited advisory inventory. Native UI behavior, complete folder/routing impact, approved disposition semantics, transactional recovery and durable undo remain unimplemented.

Native host acceptance is blocked by the unproven same-window multi-Profile ownership extraction documented in [native-space-integration.md](../native-space-integration.md) and its [source references](../native-space-source-references.md). Those preparatory design artifacts discuss the earlier v2 review; this component explicitly targets the current v3 contract and does not claim to update or prove the host design. No hidden/replacement window, cloned/reopened saved page or metadata-only selection can pass that native gate.

Chromium preparation/builds and installed native checks are deferred. Linux, Windows and macOS native interaction, visual, accessibility, failure and restart acceptance remain open. No extension check or standalone fixture substitutes for them. No model, tooling, patch-series, workflow or execution-catalog change is included.
