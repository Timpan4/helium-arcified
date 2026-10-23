# Snapshot admission enforcement response

Implemented 2026-10-23 against the adapter reviewed at `0ec717d3`. The bounded
pure API gaps in [context-persistence-review.md](context-persistence-review.md)
are closed by the focused checks below. Independent root review remains pending.
Production persistence and native security acceptance remain open.

## Changes

`SnapshotAdmission::snapshot_may_persist` is a required whole-`DurableSnapshot`
callback. Common admission invokes it for capture, encoding, write preparation
and restore, including empty and Window-only graphs. Missing callbacks refuse.
Existing Profile/Page callbacks and unsupported-payload attestation remain
mandatory.

The integration callback first compares the supplied snapshot's complete durable
graph with the bound operation model. `SnapshotMatchesModel` uses the existing
lifecycle reconstruction/validation, the shared capture projection and the same
private encoder used by public encoding. It compares complete canonical bytes,
not checksums alone. It neither recurses through capture/admission nor exposes
an encoding bypass. Sequence order is normalized through the lifecycle model;
split membership order remains significant. Window identities and selection,
attachments, Page owners and all other schema-1 records participate. Native
navigation/activity remain outside the durable projection.

After graph comparison, the callback runs the existing complete native and peer
checks. Shared Profile preparation/classification, exact context consistency,
Window coverage and explicit peer absence are unchanged. Empty graphs now reread
peer reports at use. No registry, classifier or codec was copied or added.

`CommitPreparedWrite(current_staged, prepared, admission)` has no two-argument
overload. It compares both staged images exactly, decodes the prepared candidate
with the existing decoder, then runs common admission on that decoded snapshot
before returning promotion bytes. `PreparedWrite` retains source/staged bytes,
never callbacks. Refusal changes neither original nor candidate.

Schema 1, lifecycle contract 3, the wire format and owner policies are unchanged.
No schema predecessor, capacity, retry policy or other numerical limit was added.

## Caller migration and regression coverage

Repository-wide text searches, including ignored text outside `.git` and
`node_modules`, found these complete API call sites:

- `native/features/durability/session_snapshot_test.cc`: synthetic admission
  includes the whole-graph callback; every commit supplies admission. Missing
  callback and Window-only refusal checks cover the public operations, plus
  successful same-graph commit and exact source/staged preservation.
- `native/features/migrations/schema_gate_test.cc`: migrated synthetic admission
  and both commit callers; missing whole-graph callbacks refuse through the gate.
- `native/integration/persistence/context_admission.cc`: composed admission now
  supplies complete graph comparison followed by current native/peer checks.
- `native/integration/persistence/context_admission_test.cc`: same-graph commit,
  old-attachment snapshot with fresh facts for a different graph, Window-only
  rejection, retained empty-graph peer change, native/peer refusal after prepare,
  every durable record category and original/candidate preservation. A changed
  candidate graph also commits successfully against a different original,
  exercising candidate admission rather than original admission.
- `native/features/migrations/schema_gate.{h,cc}`: inspected; already forwards
  the admission object unchanged to durability preparation. No compatibility
  overload or separate gate admission path is needed or retained.

Paths above are relative to `arcified/`. Updated feature contracts and commands
in `context-persistence-composition.md`, `session-durability.md` and
`schema-migrations.md`. The original review remains a historical record.

## Checks

Passed on Linux with Clang and clang-format 23.1.3. Each focused executable passed
strict C++17 and fail-fast ASan/UBSan execution without diagnostics. The following
reproduces the source lists, flags and output paths used, from the repository root:

```sh
check() {
  name=$1
  shift
  clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
    "$@" -o "/tmp/arcified-$name-test" &&
    "/tmp/arcified-$name-test" &&
    clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
      -g -fsanitize=address,undefined -fno-sanitize-recover=all \
      -fno-omit-frame-pointer "$@" -o "/tmp/arcified-$name-sanitized" &&
    ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
      "/tmp/arcified-$name-sanitized"
}
check session-durability \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/durability/session_snapshot_test.cc
check schema-gate \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/features/migrations/schema_gate_test.cc
check context-persistence \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/profiles/preparation.cc \
  arcified/native/features/private_browsing/operation_boundary.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/integration/persistence/context_admission.cc \
  arcified/native/integration/persistence/context_admission_test.cc
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/durability/session_snapshot.h \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/durability/session_snapshot_test.cc \
  arcified/native/features/migrations/schema_gate.h \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/features/migrations/schema_gate_test.cc \
  arcified/native/integration/persistence/context_admission.h \
  arcified/native/integration/persistence/context_admission.cc \
  arcified/native/integration/persistence/context_admission_test.cc
git diff --check -- arcified/native/features/durability \
  arcified/native/features/migrations arcified/native/integration/persistence \
  arcified/docs/features/context-persistence-composition.md \
  arcified/docs/features/session-durability.md \
  arcified/docs/features/schema-migrations.md
git diff --no-index --check -- /dev/null \
  arcified/docs/reviews/snapshot-admission-response.md
```

The suites print `session durability synthetic checks passed`,
`schema compatibility fixtures passed` and
`composed persistence synthetic checks passed`, respectively. Existing independent
wire/checksum, corruption, recovery, stale-source and navigation-evidence fixtures
also pass. Formatting and Git whitespace checks pass. No separate Chromium lint
or full build was run.

## Guarantees still open

Actual native liveness, observation completeness and freshness are not established
by callback presence or coherent fixture strings. The host must resolve current
native objects and retain them, serialize model/native/peer observations without
reentrancy, and prevent changes from final admission through native staging and
replacement. Admission is no transaction reservation and cannot certify a
caller's claimed facts. Those obligations remain OPEN.

File IO, locking, flush/rename/crash durability, restart recovery, real multi-Profile
Window hosting, cross-Profile browser isolation, close-undo and Linux/Windows/macOS
platform acceptance remain OPEN. No native producer, full Chromium preparation or
build, heavy download, live-browser operation, remote write or commit ran. The
worktree changes are ready for root's independent review.
