# Space metadata durability response

The standalone schema-2 codec and admission join preserve complete canonical
Space metadata with the lifecycle graph. The focused Linux checks below passed
on 2026-10-23. Native UI, disk durability and installed restart acceptance remain
open. The independent [review](space-metadata-durability-review.md) found no
actionable findings in this standalone change.

## Changes and contract

`DurableSnapshot::space_metadata` uses the existing
`space_management::Metadata` ordered vector and its `SpaceMetadata` /
`DisplayIdentity` types. `BuildModel` calls the existing `ValidateMetadata`,
requiring exactly one entry per Model Space, including unattached Spaces.
Missing, duplicate and unknown entries fail with `kInvalidMetadata`.
Persistence does not reconcile records, add defaults or maintain another
registry.

Schema 2 encodes each Space ID and optional name, icon_key and theme_key after
the existing lifecycle records. Presence tags preserve absent versus present
empty strings. Text bytes and vector order survive unchanged. Spaces sharing a
Profile keep independent identities.

`CaptureSnapshot(model, metadata, admission)` validates both values.
`SnapshotMatchesModel(snapshot, model, metadata)` compares the complete durable
graph and exact metadata through the existing model projection and codec.
Graph record order normalizes; metadata order and ordered split membership do
not. Navigation and activity remain outside the durable graph.

`ComposeSnapshotAdmission(model, metadata, source)` borrows all three objects
and rechecks complete graph/metadata provenance and current native/peer facts.
Capture, encode, prepare, restore and candidate commit use that admission.
Commit admits the decoded candidate, so an older original does not prevent a
valid new metadata candidate, and current facts cannot admit a stale candidate.

`RestoreSnapshot` returns `Result<RestoredSession>` with both `model` and
`space_metadata`. All actual callers were migrated. There is no Model-only
overload or conversion. `UnsupportedPeerPayloads` replaces
`Schema1PeerPayloads`; there is no Space-display absence flag. Folders, custom
theme/palette payload beyond theme_key, split presentation and other omitted
payloads still require explicit absence. Missing or nonregular native context
evidence still refuses. No privacy classifier was copied or weakened.

Schema 1 is the actual local format at
`05ace22efb8564d0294b8efa1b3f3651b76cef07`. It omitted display values and order.
No lossless transform is registered, and no values or migration are invented.
The exact historical empty fixture and CRC remain unchanged and unsupported.
Compatibility inspection refuses it in either store slot while retaining exact
original and candidate bytes. Cleanup, destruction and populated migration
policy remain unsettled.

The interrupted implementation was inspected before editing. Its logic and
fixtures were retained. This continuation completed verification, corrected the
callback lifetime comment and finished matching feature documentation.

## Focused evidence

Used existing Clang, clang-format and clang-tidy 23.1.3 on Linux. These are
standalone C++17 checks, not Chromium builds. Each compilation and executable
below exited 0 with no compiler or sanitizer diagnostics.

| Suite | Strict and sanitizer executable output |
| --- | --- |
| Durability | `session durability synthetic checks passed` |
| Migrations | `schema compatibility fixtures passed` |
| Persistence composition | `composed persistence synthetic checks passed` |

Exact compilation and execution commands, run from the repository root:

```sh
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/space_management/space_management.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/durability/session_snapshot_test.cc \
  -o /tmp/arcified-space-metadata.Mt7h1qaN/session-durability-strict && \
  /tmp/arcified-space-metadata.Mt7h1qaN/session-durability-strict
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/space_management/space_management.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/durability/session_snapshot_test.cc \
  -o /tmp/arcified-space-metadata.Mt7h1qaN/session-durability-sanitized && \
  ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 /tmp/arcified-space-metadata.Mt7h1qaN/session-durability-sanitized
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/space_management/space_management.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/features/migrations/schema_gate_test.cc \
  -o /tmp/arcified-space-metadata.Mt7h1qaN/schema-gate-strict && \
  /tmp/arcified-space-metadata.Mt7h1qaN/schema-gate-strict
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/space_management/space_management.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/features/migrations/schema_gate_test.cc \
  -o /tmp/arcified-space-metadata.Mt7h1qaN/schema-gate-sanitized && \
  ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 /tmp/arcified-space-metadata.Mt7h1qaN/schema-gate-sanitized
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/space_management/space_management.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/profiles/preparation.cc \
  arcified/native/features/private_browsing/operation_boundary.cc \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/integration/persistence/context_admission.cc \
  arcified/native/integration/persistence/context_admission_test.cc \
  -o /tmp/arcified-space-metadata.Mt7h1qaN/context-persistence-strict && \
  /tmp/arcified-space-metadata.Mt7h1qaN/context-persistence-strict
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/space_management/space_management.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/profiles/preparation.cc \
  arcified/native/features/private_browsing/operation_boundary.cc \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/integration/persistence/context_admission.cc \
  arcified/native/integration/persistence/context_admission_test.cc \
  -o /tmp/arcified-space-metadata.Mt7h1qaN/context-persistence-sanitized && \
  ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 /tmp/arcified-space-metadata.Mt7h1qaN/context-persistence-sanitized
```

All commands used strict C++17 warnings. Sanitizer commands used
`-fsanitize=address,undefined -fno-sanitize-recover=all`,
`ASAN_OPTIONS=halt_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1`.

Chromium formatting passed for all nine owned header/source/test files:

```sh
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
```

Scoped lint used this exact profile separately for each translation unit listed
below:

```sh
clang-tidy --checks='-*,clang-analyzer-*,bugprone-use-after-move' \
  --warnings-as-errors='*' "$file" -- -std=c++17 -Wall -Wextra -Werror -pedantic
```

- `arcified/native/features/durability/session_snapshot.cc`
- `arcified/native/features/durability/session_snapshot_test.cc`
- `arcified/native/features/migrations/schema_gate.cc`
- `arcified/native/features/migrations/schema_gate_test.cc`
- `arcified/native/integration/persistence/context_admission.cc`
- `arcified/native/integration/persistence/context_admission_test.cc`

All six exited 0. Only `context_admission_test.cc` reported
`Suppressed 1 warnings (1 NOLINT).`, the existing documented invalid-enum fixture
suppression. There were no unsuppressed diagnostics. Tracked scoped
`git diff --check` and the response's new-file whitespace check passed.

An untruncated repository search of C++ headers/sources and GN files found
actual API declarations and callers only in durability, migrations tests and
persistence composition. A separate native C++ search found no remaining
`Schema1PeerPayloads` reference. No caller outside the owned source required
editing.

The fixtures cover complete ordered metadata, unattached Spaces, independent
shared-Profile identities, Unicode, absent/empty fields, missing/duplicate/unknown
entries, forged valid metadata, stale order/display values after staging, both
retained and freshly composed current admission, and successful promotion of a
new metadata candidate over an older original. Refusals check retained source,
prepared and staged images. Restore/recapture checks preserve the metadata.

The historical fixture was compared with `git show 05ace22e:arcified/native/features/durability/session_snapshot_test.cc`.
Node v24.22.1's independent `node:zlib.crc32` also verified all fixed fixture
payloads and expected CRC values:

| Payload | CRC-32 |
| --- | --- |
| Actual schema-1 empty image | `f3be633e` |
| Schema-2 empty image | `440738e9` |
| Unknown metadata Space | `0b6883e3` |
| Invalid optional-value tag | `08ec578d` |
| Impossible record count | `5b12ed16` |

Commands for future focused runs remain in
[session durability](../features/session-durability.md),
[schema migrations](../features/schema-migrations.md) and
[context persistence composition](../features/context-persistence-composition.md).

## Limits and root handoff

No native UI or disk acceptance is claimed. No Chromium preparation, build,
download, remote CI, browser session or real browsing data was used. Windows and
macOS compiler/runtime behavior remains unverified. The synchronous standalone
codec has no browser scheduling/cancellation or resource policy.

Root still needs production native binding producers, serialized Model/metadata
and native observation lifetimes, durable file staging/readback/replacement,
native navigation joins, and installed order/display restoration on Linux,
Windows and macOS. Native testing needs that integration and a built browser;
no heavy native build was started in this child. Unsupported peer payloads and
the old-schema upgrade gate remain open. No owner policy for cleanup,
destruction or populated migration was selected.

Changes remain uncommitted. No remote writes occurred. Lifecycle,
space_management source, execution state, tooling, the native patch draft and
shared catalogs were not edited by this continuation.
