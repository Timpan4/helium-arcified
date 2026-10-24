# Context persistence composition

[context_admission.h](../../native/integration/persistence/context_admission.h)
and [context_admission.cc](../../native/integration/persistence/context_admission.cc)
compose the actual lifecycle v3, Profile preparation, shared native context
classifier and schema 2 durability APIs. This is a standalone admission adapter.
It implements no native registry, classifier, file IO, migration or second codec.

`ComposeSnapshotAdmission(model, metadata, source)` returns either a durability
failure or the mandatory `SnapshotAdmission` callbacks. The source is an interface to the
native host's existing live binding authority. The adapter owns no source facts
and caches no prepared targets or allow decisions. Its local identity maps exist
only during a consistency check.

The required whole-snapshot callback compares the supplied durable graph with
the bound operation Model and exact canonical ordered metadata before reading
current native facts and peer stores.
`SnapshotMatchesModel` reuses durability's lifecycle validation, canonical model
projection and existing codec without calling capture or public encoding. It
compares every durable field, including Window identity/selection, attachments,
Page owners, split membership order and Space metadata. Graph record order is
normalized; metadata display order and absent versus empty optionals remain exact;
navigation and activity remain excluded. There is no second codec or registry.
Missing callbacks refuse, including for empty and Window-only snapshots.

## Admission behavior

Every registered Profile requires an explicit exact native binding through
`profiles::PrepareRegularProfile`. This includes unreferenced Profiles and
Profiles whose Spaces have no attachment. Different lifecycle Profiles cannot
claim the same native context ID, even when their individual facts appear
regular. `ProfileId` alone never establishes eligibility.

Every logical Window requires a nonempty, complete caller report of its exact
native contexts. Each context must match its independently observed regular
Profile binding. Every attached Space's Profile must appear in that report.
Repeated Profile entries, missing Window reports and unknown additional contexts
refuse. This also covers Windows without Pages. A Window with no attachments
still needs a regular native context associated with a registered Profile.
The selected Space or legacy Browser Profile cannot stand in for the complete
Window report. This interface does not prove a production multi-Profile Window
host exists.

Every Page requires its own native observation, including saved Pages, temporary
Pages and Pages outside the selected Space. The observed attachment must match
the model's current attachment. `profiles::PreparePage` resolves the immutable
Page Profile and current attachment. The resulting exact native identity must
match the independently observed Profile and the Window report. A sibling OTR
object, another regular object, cross-Profile substitution, missing identity,
unknown classification, guest or system context refuses.

Both preparation paths use the existing `private_browsing::ClassifyContext`.
The adapter contains no copied kind-to-policy switch. The fixtures also check
the actual workspace-persistence operation boundary for the rejected kinds.
There is no private-source filtering or selected-Space shortcut. Refusal returns
no snapshot, encoded bytes, prepared write or restored model from the relevant
durability call.

## Schema 2 feature completeness

The caller supplies complete `space_management::Metadata`, validated by the
canonical `ValidateMetadata` against Model. It covers every Space including
unattached Spaces. Order and each optional name/icon_key/theme_key participate
in whole-snapshot admission at capture, encode, prepare, restore and candidate
commit. Changed or stale metadata refuses; no reconciliation or defaults run.
Restore returns `RestoredSession{model, space_metadata}`, so callers must adopt
both values together.

`UnsupportedPeerPayloads` replaces `Schema1PeerPayloads`. It reports folders,
custom theme/palette payload beyond theme_key, split presentation and other
omitted payloads. Every field defaults to unknown; unknown and present refuse.
Space metadata is represented by the canonical value, with no absence flag.
Reports cover the whole operation scope, including restore. An empty Model
cannot establish absence. Native feature hosts must retain unsupported payloads
and refuse this path. Synthetic absent reports do not establish installed
browser completeness.

## Required native call-site duties

1. Implement `NativeContextSource` over the sole live native binding authority.
   Resolve the actual Profile object for each requested Profile and Page, and
   every exact context bound to each Window. Keep runtime context IDs distinct
   across native objects, invalidate them on destruction and never reuse them.
   Never derive equality from Profile paths, `GetOriginalProfile()`, durable
   IDs, URLs or a selected legacy Browser. Missing or ambiguous bindings must
   produce no observation, rather than a permissive guessed report.
2. Serialize the model, native observations and peer stores on the host's UI
   sequence. Keep their native objects alive and prevent mutation or reentrancy
   throughout composition and the immediate durability operation. Each read must
   belong to that same coherent observation. Freshness, completeness and actual
   liveness remain host responsibilities; mutually consistent stale reports
   cannot prove them.
3. Compose fresh admission for the exact graph involved in each capture,
   encode, write or restore. Keep the Model, metadata and source alive until
   callbacks finish. The callbacks borrow all three and recheck the complete
   graph and metadata at use.
   Do not retain them across asynchronous work, use a prepared target as a
   permit, or substitute a previously captured model for current native facts.
   Whole-snapshot admission executes at use even when there are no Profile or
   Page records. Retained callbacks recheck peer absence and Window contexts.
4. Preserve exact original and candidate byte images on every refusal. Use
   `migrations::CompatibilityGate` for version inspection and write preparation.
   Unsupported images and unresolved candidates remain explicit recovery cases.
   A readable snapshot proves neither native admission nor commit. Native IO must
   serialize writers, verify staged data and provide its platform-specific
   flush/replace and recovery guarantees.
5. Revalidate current native admission and exact source/graph identity immediately
   before native staging and commit. Prepared bytes are not a freshness token.
   Abort and retain source data if contexts, attachments, feature state or source
   images change. `CommitPreparedWrite(current_staged, prepared, admission)`
   requires current admission, compares exact staged bytes, decodes that candidate
   with the existing codec, then admits the same snapshot before promotion.
   `PreparedWrite` retains bytes only, never borrowed callbacks.
6. During restore, resolve fresh native contexts for every durable identity and
   establish the exact restore graph before using admission. A durable snapshot
   contains no native context identity. Obtain actual browser-policy-validated
   navigation commits and provide every `RestoredNavigation` report exactly
   once. The adapter opens no URL and grants no native restore authorization.

## Enforcement and remaining guarantees

Capture, encode, write preparation and restore share whole-snapshot admission.
Commit requires an admission argument and uses that same path after candidate
decoding. Fresh facts for a different bound graph cannot authorize an older
snapshot. Empty graphs cannot bypass use-time peer checks. These pure API gaps
from the [review](../reviews/context-persistence-review.md) are addressed in the
[follow-up response](../reviews/snapshot-admission-response.md).

The API cannot prove that callbacks resolve actual live native objects or that
their reports are fresh. Serialization, no reentrancy and native object retention
must cover final admission through real IO replacement. Mutually consistent
stale reports still supply no native liveness evidence. These host duties remain
open, along with IO durability and Linux, Windows and macOS acceptance.

`window_restore::CaptureEligibility` is a separate close-undo boundary. This
slice inspected its actual header but does not implement or authorize that
protocol, persist its runtime `NativeRestoreId` joins, or equate it with session
snapshot restore.

## Focused verification

[context_admission_test.cc](../../native/integration/persistence/context_admission_test.cc)
uses the actual Profile preparation, private operation boundary, snapshot codec,
restore and schema gate implementations. It exercises eligible multi-Profile
synthetic facts, an unattached Profile, an empty Window, every Page's refusal,
all defined nonregular kinds and an invalid kind, missing reports, wrong Profile
and exact context bindings, duplicate context identity claims, stale attachment
observations, explicit unsupported feature reports, and native navigation
evidence. Refused capture, encode, write and restore retain the input model,
navigation and committed source bytes. Commit refusals after native/peer changes
retain both original and candidate bytes. The fixtures also cover fresh facts
for an older attachment snapshot, every durable record category, retained
empty-graph peer changes, Window-only graphs and valid same-graph commits.
Metadata fixtures cover every Model Space, including unattached Spaces,
independent identities sharing a Profile, Unicode and absent versus empty
values. Missing/duplicate/unknown metadata refuses; valid forged or stale
order/display values refuse through current admission, including after staging.
A new candidate with current metadata can replace an older committed original.
Future schemas, unresolved candidate images and changed source bytes remain
preserved. Changing only runtime native
IDs produces identical encoded bytes, and their marker strings are absent.

Run from the repository root with the existing local toolchain:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/integration/persistence/context_admission.h \
  arcified/native/integration/persistence/context_admission.cc \
  arcified/native/integration/persistence/context_admission_test.cc

clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/space_management/space_management.cc \
  arcified/native/features/profiles/preparation.cc \
  arcified/native/features/private_browsing/operation_boundary.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/integration/persistence/context_admission.cc \
  arcified/native/integration/persistence/context_admission_test.cc \
  -o /tmp/arcified-context-persistence-test
/tmp/arcified-context-persistence-test
```

```sh
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/space_management/space_management.cc \
  arcified/native/features/profiles/preparation.cc \
  arcified/native/features/private_browsing/operation_boundary.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/integration/persistence/context_admission.cc \
  arcified/native/integration/persistence/context_admission_test.cc \
  -o /tmp/arcified-context-persistence-sanitized
ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 /tmp/arcified-context-persistence-sanitized
```

The focused strict compilation, fixture execution, fail-fast ASan/UBSan run and
Chromium formatter check passed on Linux with Clang/clang-format 23.1.3. Git
whitespace checks passed for the changed files. No full Chromium preparation,
native browser build, download or live-browser test ran. Windows and macOS,
production native binding producers, actual file IO, restart durability,
cross-Profile isolation, browser security and close-undo acceptance remain
unverified. [The metadata response](../reviews/space-metadata-durability-response.md)
records the scoped analyzer checks and exact test evidence. The independent
[review](../reviews/space-metadata-durability-review.md) found no actionable
findings in the standalone change. Native acceptance remains open.
