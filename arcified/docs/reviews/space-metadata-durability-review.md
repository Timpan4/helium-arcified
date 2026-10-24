# Space metadata durability review

Result: no actionable findings in the reviewed uncommitted change against
`05ace22efb8564d0294b8efa1b3f3651b76cef07`. This is a bounded independent source
review of the standalone codec and admission composition, not native acceptance.
Reviewed on 2026-10-23.

## Scope and authorities

Reviewed the complete HEAD diff for `native/features/durability`,
`native/features/migrations` and `native/integration/persistence`, including all
changed tests. Read the unchanged migration implementation and the relevant
complete implementations alongside the diff. Reviewed the matching
[session durability](../features/session-durability.md),
[schema migrations](../features/schema-migrations.md),
[context persistence composition](../features/context-persistence-composition.md)
and [Space management](../features/space-management.md) documents, and the
[implementation response](space-metadata-durability-response.md).

Canonical authorities were `native/model/lifecycle.h`, `lifecycle.cc`,
`docs/lifecycle-contract.md`, and
`native/features/space_management/space_management.{h,cc}`. Profile preparation
was also traced to check that the composition still uses the shared context
classifier. Unrelated execution catalogs, research/publication artifacts and
native patch drafts are outside this review.

## Reviewed behavior

| Concern | Evidence and assessment |
| --- | --- |
| Complete canonical metadata | `session_snapshot.h:56` uses the existing `space_management::Metadata`. `BuildModel` at `session_snapshot.cc:59` calls canonical `ValidateMetadata` after establishing every Model Space. Missing, duplicate and unknown entries refuse. Unattached Spaces are included. No persistence path adds defaults or calls reconciliation. |
| Exact values and order | `Writer::OptionalText`, `Reader::OptionalText`, and the metadata section at `session_snapshot.cc:352` and `:433` retain byte strings, absent versus present-empty values, and vector order. No Profile or attachment grouping affects display order. Fixtures use independent shared-Profile identities and Unicode. |
| Complete provenance | `SnapshotMatchesModel` at `session_snapshot.cc:272` validates and normalizes the graph through the existing Model projection and codec, while retaining exact metadata order and optionals. `context_admission.cc:103` binds that comparison to the borrowed current Model and metadata before checking current native and peer observations. Capture, encode, prepare and restore reach this admission. |
| Current candidate commit | `CommitPreparedWrite` at `session_snapshot.cc:512` retains exact staged-image comparison, decodes the prepared candidate and admits that decoded candidate. It does not require an older original's metadata to match the current graph. `CompleteMetadataAndCandidateProvenance` tests stale candidates with retained and newly composed admission, plus successful new-candidate promotion over an older original and restore/recapture of its metadata. |
| Borrowed lifetimes | `context_admission.h:59` explicitly requires Model, metadata and source to remain alive and serialized without mutation/reentrancy through immediate use. Lambdas capture those objects by reference and re-read them. All actual composition callers use live fixture members or named local/static objects. Temporary metadata passed directly to synchronous capture is copied into the owned snapshot. `PreparedWrite` retains bytes, not callbacks. No new dangling use was found under the documented contract. |
| Restore adoption | `RestoreSnapshot` at `session_snapshot.cc:449` returns `Result<RestoredSession>` containing both the staged Model and exact metadata. All actual callers use this result; no Model-only overload or conversion remains. Native navigation evidence requirements and unknown restored activity remain intact. |
| Parser and resource safety | Optional tags use the existing strict 0/1 tag parser. String lengths are checked against remaining bytes before conversion/allocation; record counts are checked against remaining input, with incremental parsing stopping on failure. No allocation reserves an attacker-supplied count. Checksum, complete syntax, trailing-byte refusal and full graph/metadata validation remain in sequence. The new metadata section introduces no recursive format or arbitrary product cap. |
| Compatibility and preservation | `schema_gate.h:16` registers only schema 2/contract 3. Actual schema-1 bytes match the baseline fixture, remain unsupported in either slot, and are retained exactly. No predecessor transform or fabricated identity/order is introduced. Compatibility preparation still checks both images before the shared writer; unresolved candidates and changed images refuse. |
| Privacy and omitted payloads | Existing Profile, Page and Window checks are unchanged in substance and still call shared preparation. Missing/nonregular evidence refuses. `UnsupportedPeerPayloads` requires explicit absence of folders, custom theme/palette payload beyond `theme_key`, split presentation and other omitted payloads. Unknown/present reports refuse, including empty graphs. No second registry or classifier, cleanup, destruction or populated migration policy was added. |

The prior mandatory whole-graph and current decoded-candidate commit admission
has not regressed in the reviewed diff. Existing empty/Window-only, stale
attachment and every-durable-record provenance fixtures remain meaningful.
New tests exercise public failure and round-trip behavior rather than merely
duplicating encoder logic. In particular, checksum-valid malformed fixtures reach
the metadata validator or tag/count parser instead of stopping at checksum
rejection.

## Independent light checks

`git rev-parse HEAD` matched the requested baseline. An untruncated repository
search for the changed APIs across C++ headers/sources and GN files found actual
declarations/callers only in the reviewed durability, migration tests and
persistence composition files. No remaining native `Schema1PeerPayloads`
reference or unconverted restore caller was found. A broader source-reference
search found no additional build target requiring the new link dependency;
updated focused commands include `space_management.cc`.

Compared the historical empty fixture directly with
`git show 05ace22efb8564d0294b8efa1b3f3651b76cef07:arcified/native/features/durability/session_snapshot_test.cc`.
A light independent Node `node:zlib.crc32` assertion probe passed for all fixed
payloads: schema 1 `f3be633e`, schema 2 `440738e9`, unknown metadata Space
`0b6883e3`, invalid optional tag `08ec578d`, and impossible count `5b12ed16`.

Scoped `git diff --check HEAD` passed for all reviewed native directories and
the four matching feature documents. No compile, native build, CI, sanitizer,
formatter or analyzer run was repeated by this reviewer. The strict C++17,
fail-fast ASan/UBSan suites, formatting and scoped tidy results are implementation
evidence recorded in the response, not independently rerun results here.

## Acceptance still open

Native UI, actual disk staging/readback/replacement and crash durability,
installed metadata adoption and restart, production native binding freshness,
and Windows/macOS compiler/runtime acceptance remain OPEN. Linux standalone
evidence does not close those gates. The synchronous parser has no browser
scheduling, cancellation or deployed resource policy; its input-proportional
checks do not establish interactive performance for arbitrary large images.
Unsupported peer payloads and predecessor migration policy remain unresolved.

Only this review report was written. No implementation files, canonical
authorities, commits, PR state or remote resources were changed. The parent owns
fixes, later review rounds and native acceptance.
