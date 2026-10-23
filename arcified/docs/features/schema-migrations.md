# Stored schema compatibility

Status: local C++ compatibility and recovery component for [issue 99](https://github.com/Timpan4/helium-arcified/issues/99). Installed native upgrades remain open on Linux, Windows and macOS.

## Ownership and supported formats

The [session snapshot component](../../native/features/durability/session_snapshot.h) owns the only durable workspace format, decoder, graph validation and byte-image write protocol. The [lifecycle model](../../native/model/lifecycle.h) remains the ownership model. Migration admission adds no serializer, navigation state, persistence file layout or native session store.

The supported-format registry contains only schema 1 with lifecycle contract 3. Schema 1 is the initial local implementation, not a shipped historical format. Lifecycle contract versions 1 and 2 were semantic model revisions and have no supported disk upgrade path. There are no registered predecessor transformations. An installed upgrade cannot be claimed until an actual prior format, its fixtures and a native integration exist.

## Admission and recovery

`CompatibilityGate` owns the exact `durability::StoreImage` presented for inspection. It inspects the original and optional interrupted candidate through `ProbeEnvelope` and `DecodeSnapshot`. It reports absent, current, corrupt, future schema, unsupported schema and unsupported contract separately. A recognizable future header blocks write admission even when its body is incomplete or a current image exists beside it. An unrecognizable or incomplete header remains corrupt input, never an empty store.

Valid current images are available as explicit read-only recovery choices. The gate never promotes a candidate, falls back silently, removes an artifact or re-encodes the original. The original and candidate bytes remain accessible unchanged, including binary bytes and corrupt input. A valid candidate is not proof of a completed write or of newer workspace state.

`PrepareWrite` compares both observed images exactly with the owned source. It refuses changed input and unsupported versions, then calls the session component's `PrepareWrite`, which refuses unresolved recovery and invalid target snapshots. Only a genuinely empty store or a valid current original without a candidate can prepare a write. The session component owns staging and commit comparison. This is an in-memory preflight, not a filesystem lock or atomic compare-and-swap. The eventual IO adapter must serialize writers and retain the original through durable staging, readback, commit and failure recovery.

The gate passes `SnapshotAdmission` unchanged to the session writer. Missing or rejecting Profile/page eligibility callbacks refuse persistence. Root integration must bind those callbacks to the single native private-browsing classifier, including exact page contexts. Schema compatibility is not privacy admission. The caller must also explicitly attest that unsupported Space metadata and split presentation payloads are absent. This component cannot serialize or silently discard those peer features. Decoded recovery metadata alone does not authorize restoration or persistence.

## Rollback and downgrade

Current-version admission is a no-op on the original bytes. A successful byte-image commit leaves its source in `PreparedWrite`; an IO adapter must preserve that source before replacing on-disk data. This component neither implements backup retention nor authorizes deleting recovery data.

A downgraded reader refuses any unregistered schema or contract, preserves its exact input and offers only explicitly validated recovery choices. It must not rewrite a future schema using schema 1, silently select an older image, or label unsupported bytes corrupt merely because a full decode is unavailable. Rolling back metadata cannot undo server effects or restore live forms, authentication, renderer state or navigation history. Native Profile-local session storage remains authoritative for navigation restoration.

## Evidence and remaining acceptance

The [issue brief](../research/arc-dia/publication/schema-migrations.md) and `backlog.json` require supported-upgrade fixtures, preservation on failure, rollback documentation and installed upgrade proof for names, order and current selection. Synthetic compatibility fixtures cover only the local byte-image boundary. No historical upgrade is fabricated to satisfy that requirement.

The [native integration plan](../native-space-integration.md) requires validated workspace/session joins and an actual native host that survives cross-Profile Space switching. Its [pinned source references](../native-space-source-references.md#c4-navigation-and-session-boundaries) identify Profile-local `SessionService` ownership and selection records. The existing [lazy restore patch](../../../patches/helium/core/lazy-session-restore.patch) restores selected tabs and split order, but does not implement workspace schema migration. These are source observations, not installed upgrade results.

Open acceptance includes actual predecessor migrations, names and presentation order absent from the current lifecycle snapshot, native session joins, durable file replacement and rollback behavior, recovery UI and accessibility, and installed native upgrades on all target platforms. No Chromium preparation, build, download, benchmark, real browsing data, provider access or external write is needed for these local checks.

## Focused checks

`schema_gate_test.cc` uses synthetic `.test` destinations and fixed fixture identities. It executes current-format inspection without changing original bytes, absent versus empty input, valid/truncated/empty interrupted candidates, corrupt original with valid candidate, CRC/truncation/trailing-data failures, unsupported headers in either image, stale preflight and staging inputs, and the shared prepare/commit protocol. Privacy callbacks are synthetic allowlists, not a test of native private browsing. Separate missing/denying Profile and page callbacks and unconfirmed feature absence prove that the gate preserves the writer's refusals.

Unsupported fixtures change only the actual schema-1 envelope version fields. They include incomplete bodies deliberately, because a recognizable unsupported header must remain distinguishable from corruption. They are not claimed as prior shipped formats or successful migrations. A valid original beside an unsupported candidate still refuses writes.

Run from the repository root with the existing toolchain:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/migrations/schema_gate.h \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/features/migrations/schema_gate_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/features/migrations/schema_gate_test.cc \
  -o /tmp/arcified-schema-gate-test
/tmp/arcified-schema-gate-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/features/migrations/schema_gate_test.cc \
  -o /tmp/arcified-schema-gate-sanitized
/tmp/arcified-schema-gate-sanitized
```

The strict-warning compile, fixture executable, AddressSanitizer/UndefinedBehaviorSanitizer execution and Chromium formatting check passed locally with Clang/clang-format 23.1.3. These checks exercise the shared C++ codec and byte-image protocol. They do not test disk durability, native recovery UI or installed browser upgrades. Windows/macOS compiler and runtime checks remain open.
