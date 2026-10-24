# Split presentation recovery

Issue [169](https://github.com/Timpan4/helium-arcified/issues/169),
`split-persistence`. Local component at checkpoint
`c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`, 2026-10-23.

`ReconcileRestoredLayout` recovers split presentation metadata against an
already-restored lifecycle graph. It is a pure C++17 component. It does not
implement a durable codec, file IO, journaling, native session restoration or
native acceptance.

## Ownership boundary

[The lifecycle model](../../model/lifecycle.h) owns pages, split membership,
membership order and attachments. [The presentation core](layout.h) owns
`Orientation` and `LayoutMetadata`. This component owns no additional membership,
navigation, focus or selection state. Its output is a snapshot to reconcile
again after a model mutation.

The coordinator confirmed `features/durability/session_snapshot.h` as the single
durable schema owner. Its schema version 1 is independent of lifecycle semantic
contract version 3. Schema version 1 explicitly omits split orientation and
relative weights. Stores requiring unsupported feature state must be refused
through feature-completeness attestation. Root owns future `LayoutMetadata`
serialization through the durability-owned schema; no metadata extension wire
shape is frozen. The durability owner must handle encoding, decoding, version
checks, unknown versions and any future metadata migration. This component adds
no local codec, workspace schema, version registry or focus registry, and does
not interpret either version.

The caller must restore and authorize lifecycle identities before this call. It
resolves the saved split and attachment through its authoritative restored
mapping and supplies the resulting `SplitId`. `RestoredPageMap` associates saved
`PageId` references with caller-restored `PageId` identities. This component never
infers identity from matching strings, URL, title, a native tab index or an absent
mapping. It never allocates, clones, reopens, transfers or closes a live page.
An identical old/current `PageId` still needs an explicit mapping entry.

Selection remains entirely external until its owner and scope are settled. No
selected pane field, focus fallback or selection serialization is introduced.
The [pinned native ownership design](../../../docs/native-space-integration.md)
and [source references](../../../docs/native-space-source-references.md) remain
the native integration boundary. Same-window multi-Profile ownership is unproven.

## Reconciliation contract

Input consists of a const lifecycle model, a restored split reference,
`LayoutMetadata` keyed by saved page references, and the authoritative page
mapping. Output is `variant<LayoutError, RestoredLayout>`, where `RestoredLayout`
contains reconciled metadata and recoverable per-reference issues.

1. Use `ReconcileLayout` with the saved orientation to validate the split and
   orientation and obtain defaults for canonical members. A missing split or
   invalid orientation returns the core error without model mutation.
2. Resolve each saved metadata key only through the supplied mapping. Diagnose
   empty references, missing mappings, missing target pages, target attachment
   mismatch and pages outside the requested split. A rejected reference never
   assigns a weight to a current member.
3. Group eligible references by current target identity. Distinct saved sources
   mapping to one member make every competing weight ambiguous. Report each
   source and retain that member's default. Do not select a winner by map order.
4. For an unambiguous member, preserve its finite positive relative weight
   exactly. Diagnose an invalid weight and keep the default. Canonical members
   with no accepted saved weight also keep the default.

The dimensionless default `1.0` comes from the coordinator-approved presentation
core contract. It is equal to other defaults, not necessarily to explicitly
weighted survivors. No normalization, required sum, pane cap, retry count or
input-size policy is added. Work and temporary storage derive from the supplied
metadata and canonical split size.

Each rejected source gets one diagnostic. Reference validity precedes membership,
ambiguity precedes weight validity. Therefore an ambiguous source with an invalid
weight reports ambiguity; an out-of-split source with an invalid weight reports
its reference problem. Diagnostics are deterministic for the same inputs.
`ResolveLayout` projects output weights in canonical lifecycle order; the saved
metadata key order never changes pane order. Recovering a single-member split
does not choose a native single-pane presentation policy.

## Local verification

Run from the repository root:

```sh
clang++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/splits/layout.cc \
  arcified/native/features/splits/persistence.cc \
  arcified/native/features/splits/persistence_test.cc \
  -o /tmp/arcified-split-persistence-test
/tmp/arcified-split-persistence-test
clang-format --dry-run --Werror --style=Chromium \
  arcified/native/features/splits/persistence.h \
  arcified/native/features/splits/persistence.cc \
  arcified/native/features/splits/persistence_test.cc
```

The synthetic tests cover both orientations, canonical ordering, partial recovery,
defaulting new/missing members, explicit identity mapping, empty references,
missing target pages, wrong windows/Profiles, same-attachment nonmembers,
ambiguous targets (including mixed valid/invalid competing weights),
zero/negative/non-finite weights and exact preservation of finite extreme
weights. They check protected page/navigation/Profile and selected
Space preservation, plus reconciliation after actual lifecycle close/transfer
operations. These are local semantic checks, not browser or restart tests.

## Remaining acceptance

All native issue criteria remain open. Durable metadata encoding/decoding and
restart round trips await root-owned `LayoutMetadata` serialization through the
durability-owned schema and integration. Schema version 1 does not satisfy split
presentation persistence acceptance.
Restored split/attachment mapping and native session evidence belong to that
caller. Selection restoration awaits the external owner/scope decision. Relative
size recovery under actual native geometry, small-window fallback after restart,
and installed Linux, Windows and macOS behavior have not been tested. No native
adapter, browser build, real browsing data or platform acceptance is claimed.
