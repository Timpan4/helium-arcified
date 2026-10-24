# Context persistence composition review

Reviewed 2026-10-23. No actionable code or contract defect found within the stated pure adapter scope. The join is useful and safe when its borrowed objects remain alive and the caller supplies a coherent, complete observation for the exact graph being processed. This finding does not establish production persistence or native security readiness.

## Scope and source

Independently reviewed the complete uncommitted `context_admission.h`, `context_admission.cc`, `context_admission_test.cc` and `context-persistence-composition.md`. Read the actual Profile preparation, shared classifier, durability admission and schema gate seams to trace the join. Did not repeat a general review of the underlying features.

Reviewed Git blob hashes, unchanged through verification:

- `arcified/native/integration/persistence/context_admission.h`: `e602d09104c886c0aa38e5030f422c5d34878168`.
- `arcified/native/integration/persistence/context_admission.cc`: `72197ee440fb72e4ffa2a423976e3923e45bc623`.
- `arcified/native/integration/persistence/context_admission_test.cc`: `8a31624486fbe52f88a254719e2aa9b6f49c5246`.
- `arcified/docs/features/context-persistence-composition.md`: `cc7ef9b53fcd58648854500ffd492bb2b45290fb`.

## Join correctness

`CheckAll` in `context_admission.cc:18` checks the entire supplied model, rather than only the IDs requested by durability. Its Profile loop uses `PrepareRegularProfile` for every registered Profile, including unattached and unreferenced Profiles. The temporary identity set rejects two lifecycle Profiles claiming the same native identity. It is invocation-local consistency evidence, not another registry.

The Window loop requires a nonempty report, prepares each reported context through the same Profile API, checks its exact identity against the independent Profile observation, rejects repeated Profile entries and requires coverage of every attached Space's Profile. Unknown additional contexts refuse. Empty Windows still require an observed registered regular Profile. Multiple Windows sharing the same regular Profile context are permitted, which is consistent with native Profile identity being distinct from Window identity.

The Page loop checks every current attachment and calls `PreparePage`. It requires a regular classification and exact identity equality with the Profile observation. Window membership then follows from the checked attachment's Profile and the Window coverage check. This covers saved and temporary Pages outside the selected Space. Preparation supplies ownership and context validation; the adapter adds no competing classifier or codec. The current workspace-persistence operation boundary agrees with this regular-only decision.

Peer metadata reports default to unknown and require explicit absence for every named schema 1 omission. The model cannot establish those absences. Runtime context IDs remain only in observations and temporary consistency maps. They are not passed into snapshot records or the encoder.

## Lifetimes, stale facts and ambiguity

The lambdas in `context_admission.cc:95` borrow the actual model and source objects. Capturing reference parameters this way does not leave a reference to an expired parameter variable, but it also does not extend either object's lifetime. Copying `SnapshotAdmission` copies these borrows. Temporaries, destroyed or moved-away sources, asynchronous retention and concurrent mutation remain invalid uses under `context_admission.h:58`.

Each invoked Profile/Page callback rereads the complete bound graph and source. A changed model attachment with the old Page observation refuses. Missing identities, mismatched facts/bindings, cross-Profile substitution, different exact regular objects, duplicate Profile identity claims and repeated Window entries refuse. A coherent set of stale reports can still pass. The source must withhold an observation when its native authority cannot resolve a unique live binding. Neither fixture maps nor equal runtime strings prove object liveness, complete native enumeration or freshness.

The documented UI-sequence serialization must also exclude reentrancy through source reads. It must cover composition, callback execution and the immediate operation. Native object retention, non-reused IDs and peer-store scope are caller obligations, explicitly stated in the composition document. There is no hidden keepalive or reservation in this adapter.

## Shared enforcement gaps, confirmed independently

These are production integration requirements, not newly discovered violations of the adapter's stated preconditions.

1. **Snapshot provenance is not enforced.** `SnapshotAdmission` sees Profile IDs and Page ID/Profile pairs. In a focused probe, I captured the fixture's original snapshot, transferred the saved Page to the other Window, updated its observation and composed fresh admission. Encoding and restoring the original snapshot with that fresh admission succeeded and restored the old attachment. Thus fresh composition alone is insufficient. The caller must separately establish equality with the exact snapshot graph. The document explicitly requires that equality and describes this gap at `context-persistence-composition.md:103`.
2. **Commit admission is not enforced.** A second probe prepared a write, changed the Page observation to OTR and confirmed fresh composition refused. `CommitPreparedWrite(prepared.staged_image(), prepared)` still succeeded. That function checks exact staged images, not current contexts, peer stores or the graph. The call-site duty at document line 93 and the shared-gap section correctly disclose this. Merely calling `ComposeSnapshotAdmission` again would not compare prepared content with the current graph.
3. **Empty graphs have no use-time callback.** A probe composed admission for an empty model with explicit peer absence, then changed the folder report to present. Fresh composition refused, but capture with the retained admission succeeded because durability invoked no Profile/Page callback. The peer-absence boolean is a composition-time value. This violates the caller's no-mutation/fresh-composition duties and does not contradict the document's narrower statement that composition itself checks empty graphs. It reinforces why callbacks must not be treated as a transaction permit.

The phrase "recheck the complete graph at use" means the caller-bound model whenever a callback executes. It does not establish equality with an arbitrary `DurableSnapshot`, promise invocation for an empty graph or cover commit. Read with the explicit preconditions and gap section, the document does not claim those unavailable guarantees. Preserve these qualifications in any downstream completion claim.

## Bounded shared follow-up

Before claiming mechanically mandatory admission throughout persistence, make the following separate shared API change. No shared API was edited in this review.

1. Add a required whole-snapshot admission callback, for example `std::function<std::optional<Failure>(const DurableSnapshot&)>`, to the existing admission seam. Invoke it from the common admission path for capture, encode/write preparation and restore, including snapshots with no Profiles or Pages. Missing callbacks must refuse. The integration callback must compare the supplied snapshot's complete durable graph with the bound model, including Window identities, attachment and Page-owner records, then apply current native and peer-store checks. Reuse shared record/validation logic and the existing codec. A callback that only reruns `CheckAll(model, source)` would leave the provenance gap open.
2. Require fresh admission as an argument to `CommitPreparedWrite`. After exact staged-image validation, use the existing decoder on the prepared candidate and run the same whole-snapshot admission before promotion. Refusal must return without modifying either image. Do not retain borrowed admission callbacks inside `PreparedWrite`. The caller must keep the current model/source alive and serialize native staging, final validation and replacement. This closes the pure commit bypass while leaving actual native freshness and IO guarantees with the host.
3. Add focused shared regression cases for the older-attachment snapshot with fresh admission, a Window-only graph, empty-graph peer-state change, and native/peer refusal after preparation. Assert preservation of original and candidate images on refusal. These are bounded seam changes; they need no new registry, classifier or format.

## Verification and limits

Independently compiled and ran the documented focused source list with Clang 23.1.3, C++17 and `-Wall -Wextra -Werror -pedantic`. It printed `composed persistence synthetic checks passed`. Independently repeated it with `-g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer`, `ASAN_OPTIONS=halt_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1`. It passed without sanitizer diagnostics. Chromium `clang-format --dry-run --Werror` passed for the three C++ files. Git whitespace checks of all four untracked inputs against `/dev/null` produced no diagnostics.

The existing fixtures exercise the eligible join, every fixture Profile/Page/Window's missing observations, nonregular classifications, identity confusion, explicit peer absence, stale attachment refusal, runtime-ID exclusion and in-memory refusal preservation through capture, encode, preparation and restore. The additional stdin-only probe harness reused the fixture and confirmed the three API limits above. Its success demonstrates those limits, not enforcement of the missing guarantees. No test source was changed.

No full Chromium build, native-browser interaction, production binding producer, real file IO, restart durability, cross-Profile browser isolation, platform acceptance or close-undo protocol was verified. Linux fixture and sanitizer success supplies no Windows/macOS or native security proof. Only this review report was written in the repository.
