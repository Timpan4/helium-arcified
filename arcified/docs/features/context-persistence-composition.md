# Context persistence composition

[context_admission.h](../../native/integration/persistence/context_admission.h)
and [context_admission.cc](../../native/integration/persistence/context_admission.cc)
compose the actual lifecycle v3, Profile preparation, shared native context
classifier and schema 1 durability APIs. This is a standalone admission adapter.
It implements no native registry, classifier, file IO, migration or second codec.

`ComposeSnapshotAdmission(model, source)` returns either a durability failure or
the mandatory `SnapshotAdmission` callbacks. The source is an interface to the
native host's existing live binding authority. The adapter owns no source facts
and caches no prepared targets or allow decisions. Its local identity maps exist
only during a consistency check.

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

## Schema 1 feature completeness

`Schema1PeerPayloads` requires a separate caller report for actual Space display
and order metadata, folders, themes and split presentation. Every report defaults
to unknown. Only explicit absence of all these payloads can set durability's
`unsupported_feature_payloads_absent` field. Unknown and present both refuse.
The reports cover the whole operation scope, including peer stores used during
restore. An empty lifecycle model does not prove absence.

The adapter does not inspect the peer stores or discard their data. Real Space
names, user order, folder state, themes and split presentation remain unsupported
by schema 1. Lifecycle split membership references do not establish absence of
split presentation payloads. A native feature host with any such state must
retain it and refuse this persistence path until a reviewed shared codec can
represent it. The eligible fixtures explicitly provide absent synthetic stores;
they do not claim these payloads are absent in an installed browser.

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
   encode, write or restore. Keep the model and source alive until callbacks
   finish. The callbacks borrow both and recheck the complete graph at use.
   Do not retain them across asynchronous work, use a prepared target as a
   permit, or substitute a previously captured model for current native facts.
   Composition itself checks Windows and peer payloads even for a graph on which
   durability would invoke no Profile or Page callbacks.
4. Preserve exact original and candidate byte images on every refusal. Use
   `migrations::CompatibilityGate` for version inspection and write preparation.
   Unsupported images and unresolved candidates remain explicit recovery cases.
   A readable snapshot proves neither native admission nor commit. Native IO must
   serialize writers, verify staged data and provide its platform-specific
   flush/replace and recovery guarantees.
5. Revalidate current native admission and exact source/graph identity immediately
   before native staging and commit. Prepared bytes are not a freshness token.
   Abort and retain source data if contexts, attachments, feature state or source
   images change. `CommitPreparedWrite` checks staged byte equality only.
6. During restore, resolve fresh native contexts for every durable identity and
   establish the exact restore graph before using admission. A durable snapshot
   contains no native context identity. Obtain actual browser-policy-validated
   navigation commits and provide every `RestoredNavigation` report exactly
   once. The adapter opens no URL and grants no native restore authorization.

## Shared API enforcement gaps

`SnapshotAdmission` receives only Profile IDs and Page ID/Profile pairs. It has no
whole-snapshot or Window callback, and its Page callback does not receive the
snapshot's owner or attachment. Thus the adapter checks the complete caller-bound
model and all Window contexts during composition and callback use, but cannot
compare an arbitrary snapshot's attachment graph against that model inside those
callbacks. Reusing admission with an older or different graph is outside its
contract. Fresh composition for the exact graph is mandatory.

If root requires mechanical enforcement across arbitrary durability callers,
the concrete shared change is an admission callback receiving the entire
`DurableSnapshot`, invoked even for an empty graph by capture, encode and restore.
That would expose Window and attachment provenance to the seam. A separate
admission/revalidation hook at `CommitPreparedWrite`, or a host transaction API
that enforces the same obligation, is needed to make commit revalidation
mandatory. No shared feature API was edited in this slice. These limits were
reported before proposing such changes. Runtime freshness must still remain
with the native host even with those hooks.

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
navigation and committed source bytes. Future schemas, unresolved candidate
images and changed source bytes remain preserved. Changing only runtime native
IDs produces identical encoded bytes, and their marker strings are absent.

Run from the repository root with the existing local toolchain:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/integration/persistence/context_admission.h \
  arcified/native/integration/persistence/context_admission.cc \
  arcified/native/integration/persistence/context_admission_test.cc

clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/profiles/preparation.cc \
  arcified/native/features/private_browsing/operation_boundary.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/integration/persistence/context_admission.cc \
  arcified/native/integration/persistence/context_admission_test.cc \
  -o /tmp/arcified-context-persistence-test
/tmp/arcified-context-persistence-test
```

Repeat the same source list and strict flags with
`-g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer`
and a separate output path. Run it with `ASAN_OPTIONS=halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1` for fail-fast execution.

The focused strict compilation, fixture execution, fail-fast ASan/UBSan run and
Chromium formatter check passed on Linux with Clang/clang-format 23.1.3. Git
whitespace checks passed for the changed files. No full Chromium preparation,
native browser build, download or live-browser test ran. Windows and macOS,
production native binding producers, actual file IO, restart durability,
cross-Profile isolation, browser security and close-undo acceptance remain
unverified. Root's independent code review remains pending.
