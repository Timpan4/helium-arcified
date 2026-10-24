# Snapshot admission correctness review

Reviewed 2026-10-23. No actionable correctness or security defect found in the
mandatory snapshot and commit admission changes within the stated pure API
scope. The three previously accepted repros now refuse in independently executed
tests. This finding does not establish native persistence or platform safety.

## Scope and exact sources

Reviewed the uncommitted changes against base and current HEAD
`0ec717d36788af297d68d658be80d3f1846b2a06`. Inspected the shared durability
admission, canonical graph comparison, commit path, schema gate forwarding,
affected callers/tests, feature contracts and implementation response. The
underlying lifecycle, Profile preparation and context classifier were traced
where needed for the new composition, without repeating their general review.

The following are Git blob hashes of the working files, not commit IDs. They
were unchanged through the independent checks. Paths are relative to `arcified/`.

| Reviewed file | Blob hash |
| --- | --- |
| `native/features/durability/session_snapshot.h` | `a9575d2ac7bfee06a958c5fbe702901a37db2031` |
| `native/features/durability/session_snapshot.cc` | `b4ea8d306ad0b223a7be4b44d52d24213f6f1893` |
| `native/features/durability/session_snapshot_test.cc` | `c4f84edaf1bef20957e11d8399955fcb04e760a5` |
| `native/features/migrations/schema_gate.h` | `c2bf17e3bdd13f5a341bc4ea34b2e4a4ba08477e` |
| `native/features/migrations/schema_gate.cc` | `2de10d80b0feb4e3d8f090a4ce013fd115c99ada` |
| `native/features/migrations/schema_gate_test.cc` | `71c239e0d02bef2ec3b0e79f1e037221df9b5320` |
| `native/integration/persistence/context_admission.h` | `bc5cf454c9912678753015cee477a582397cf96e` |
| `native/integration/persistence/context_admission.cc` | `12196511af86c78bf64ef65dde3d814235b5688d` |
| `native/integration/persistence/context_admission_test.cc` | `0917d9a7dda660d666fa532e69943aca3233bab6` |
| `docs/features/session-durability.md` | `4bb5d4168d582816889f0ef6e93245e708176856` |
| `docs/features/schema-migrations.md` | `5ba7a6212ccf9609731f053ff2d0cc581ea18356` |
| `docs/features/context-persistence-composition.md` | `3e1e855dba60ecf1a37aee5a5a1d8be78a2c1707` |
| `docs/reviews/snapshot-admission-response.md` | `8967c62a0db6842d6e41073b0c50188e18188016` |

The schema gate implementation and header are unchanged from the base. A
repository-wide text search, including ignored text and excluding `.git` and
`node_modules`, found the admission and commit symbols only in these components
and their documentation/prior review. There is no remaining two-argument commit
overload or unmigrated code caller. A compile-time invocability check independently
confirmed that the two-argument commit call is unavailable.

## Correctness and security findings

`Admit` requires the whole-snapshot, Profile and Page callbacks as well as the
unsupported-payload attestation. Its whole-snapshot callback runs before the
record callbacks, including when both record loops are empty. Capture, encoding,
preparation, restore and commit reach this common path. Invalid snapshots,
unsupported inputs and stale store images may refuse earlier; none succeeds by
skipping admission. Decode, recovery and gate inspection remain read-only
metadata operations and do not grant restore or write admission.

`SnapshotMatchesModel` validates and reconstructs the supplied snapshot through
the existing lifecycle implementation. It projects both graphs through the same
capture projection and compares the existing encoder's complete bytes. Every
schema-1 field participates: registered identities, Space Profiles, Window
selection, attachments, item owner variants and destinations, shortcuts, Page
Profiles and owner variants, and split attachments and ordered Page references.
Schema and contract mismatches fail validation. Model reconstruction normalizes
record order while preserving split membership order. The comparison grants no
permission and does not recursively invoke public capture or encoding.

The composed whole-snapshot callback compares provenance before calling the
existing complete native/peer check. Fresh coherent facts for a different bound
graph therefore cannot authorize an older snapshot. No runtime context identity
enters the projection or encoder. The existing runtime-ID exclusion and changed-ID
byte-equality tests passed independently.

`CommitPreparedWrite` first compares both staged slots exactly. It then decodes
the prepared candidate, including the existing checksum and graph validation,
and admits that decoded candidate before returning promotion bytes. It does not
admit the original in place of the candidate. The private preparation constructor
establishes candidate presence; the public API cannot fabricate arbitrary
prepared images. Refusals return without changing the source, prepared object or
caller's staged image. Successful promotion still leaves the prior images in the
prepared object. The compatibility gate continues to forward admission to the
shared writer and preserves unsupported/recovery states.

All composed callbacks borrow the actual model and source objects. Capturing
reference parameters does not borrow an expired parameter variable, and it does
not extend the referents' lifetimes. `PreparedWrite` stores byte images only. An
additional sanitized probe prepared a write in a scope, destroyed its admission,
model and source, then committed with fresh admission from new live objects. It
passed. This supports the stated absence of preparation-time callback retention,
not permission to invoke dangling callbacks.

The header and feature contract explicitly require a coherent serialized
operation, no mutation or reentrancy during reads/use, and native object
retention. Those duties also cover the gap between final admission and actual
replacement. The changes do not claim to enforce those host obligations or turn
callbacks into a reservation. No new in-scope failure or reentrancy contract
violation was found. Schema, wire encoding and owner policies remain unchanged.

## Independently executed verification

On Linux with Clang and clang-format 23.1.3, independently rebuilt and ran all
three source lists documented in
[snapshot-admission-response.md](snapshot-admission-response.md). Each passed
with `--driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic`, and again
with `-g -fsanitize=address,undefined -fno-sanitize-recover=all
-fno-omit-frame-pointer`, `ASAN_OPTIONS=halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1`. There were no sanitizer diagnostics.

| Executable source suite | Actual output from both executions |
| --- | --- |
| `session_snapshot_test.cc` | `session durability synthetic checks passed` |
| `schema_gate_test.cc` | `schema compatibility fixtures passed` |
| `context_admission_test.cc` | `composed persistence synthetic checks passed` |

The reviewer output paths were `/tmp/snapshot-admission-review-durability`,
`/tmp/snapshot-admission-review-schema` and
`/tmp/snapshot-admission-review-context`, with `-san` appended for sanitized
executables. These were new reviewer executions, not reused primary-agent logs.

The executed integration regressions establish the requested repro outcomes:

1. `ChangedAttachmentRequiresNewNativeObservation` captures/prepares the old
   graph, transfers the saved Page, supplies coherent updated facts and fresh
   admission, then rejects encoding, restore, gate preparation and commit of the
   old graph. It checks retained images and successfully commits/restores the
   updated candidate against the different old original.
2. `EmptyAndWindowOnlyGraphsRecheckAtUse` changes peer state after composition.
   Empty-graph capture, encode, preparation, restore and commit all refuse, with
   both prepared images retained. It also rejects an unobserved Window and an
   observed Window whose context changes to OTR.
3. `RefusalPreserves` prepares before changing native observations or peer state.
   Fresh composition refuses and commit with retained borrowing callbacks also
   refuses, including the Page OTR case. Source, original and candidate remain
   intact. Retained callbacks are tested adversarially to detect cached permits;
   the production contract still requires fresh composition for each operation.

An additional stdin-only reviewer harness reused the fixture without editing any
input. Its final source Git blob hash was
`b4a867242af413fcfbef9d32bac92e53415a70a0`. It linked the integration suite's
production source list, replacing the test translation unit with the stdin
harness, and passed the same strict and fail-fast sanitizer checks. Both
executables, `/tmp/snapshot-admission-review-probe` and its `-san` variant, printed
`independent snapshot admission boundary probes passed`. It established:

1. Valid split-order reversal, selection removal, equal-text item owner tag
   substitution, saved-to-temporary Page owner substitution and shortcut target
   substitution all refuse through encode, restore, gate preparation and commit.
   For each commit probe, permissive synthetic callbacks prepared a different
   valid candidate; actual composed admission for the original graph rejected
   it and preserved both slots.
2. Reversing every record collection while retaining split member order still
   matches and permits encode/restore.
3. Missing and rejecting whole-snapshot callbacks refuse through capture,
   encode, restore, direct preparation, gate preparation and commit for both an
   empty graph and a true Window-only graph with no Profiles or Pages. The
   rejecting callback observes the actual graph on each path.
4. Changed staged bytes refuse before invoking whole-snapshot admission.
   Duplicate-Profile snapshots refuse before that callback in encode, restore
   and gate preparation. An unchanged commit invokes it on the decoded candidate.
5. Prepared writes remain usable with fresh admission after destruction of all
   preparation-time borrowed objects, as described above.

Chromium formatting passed for the nine scoped C++ files, including the
unchanged schema gate header/implementation. Git whitespace checking passed for
the scoped tracked changes. The untracked implementation response produced no
whitespace diagnostics under `git diff --no-index --check /dev/null`; its status
1 represented the new-file difference.

## Reused evidence and limits

The original adapter's independently clean result and accepted baseline repros
come from [context-persistence-review.md](context-persistence-review.md). I did
not rerun the old implementation to establish their earlier acceptance. The
current refusals, focused suites and additional probes above were independently
executed. Lifecycle, Profile preparation, shared context boundary and private
operation boundary files were confirmed unchanged from the base and used in the
new builds; their wider guarantees rely on prior review rather than a new broad
audit.

Native observation liveness, completeness and freshness, serialized execution
without reentrancy, object retention, actual IO/locking/flush/replacement/crash
durability, and Linux/Windows/macOS browser acceptance remain open caller or
integration work. No production native source, live browser, actual persistence
IO or platform test was exercised. No full Chromium preparation/build, heavy
download, commit, remote write or PR link/watch was performed. Only this review
file was written in the repository; inputs and prior reviews were preserved.
