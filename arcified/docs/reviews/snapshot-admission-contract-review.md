# Snapshot admission contract review

Reviewed 2026-10-23. No actionable correctness, standards, privacy-contract or caller-migration defect found in the requested pure admission changes. The required whole-snapshot callback and candidate commit admission close the three reproduced API gaps in [context-persistence-review.md](context-persistence-review.md). This conclusion does not establish native persistence or browser security acceptance.

## Scope and exact inputs

Compared the scoped working files with baseline `0ec717d36788af297d68d658be80d3f1846b2a06`. Inspected durability admission, canonical projection, shared encoding, commit, schema gate forwarding, affected tests and feature contracts, and [snapshot-admission-response.md](snapshot-admission-response.md). Traced lifecycle reconstruction and Profile preparation where needed for the new composition. Reused the earlier review of unchanged underlying primitives instead of repeating their general audit.

Read the repository and Arcified `AGENTS.md` instructions, the supplied global coding standards and the feature check instructions. The standalone C++ slice documents strict compiler warnings and Chromium formatting; no separate C++ lint target was exercised.

HEAD initially equaled the baseline. During review it advanced to `96852a785138ab5fa016ecc4abf86f19bb83e068` through an unrelated layout-composition commit. That commit changes none of these inputs. All scoped hashes matched their initial values after testing. Paths below are relative to `arcified/`; hashes are Git blob hashes of the reviewed working files, including the untracked response.

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

## Contract and caller findings

1. `Admit` checks the whole-snapshot, Profile and Page callbacks before invoking any of them. A missing whole callback cannot preserve the old per-record bypass, even when there are no Profile or Page records. Capture calls common admission directly. Encode and restore validate then admit. Preparation delegates to encoding. Commit decodes and admits its prepared candidate after exact comparison of both staged images. There is no two-argument commit overload.
2. `SnapshotMatchesModel` rejects invalid or unsupported snapshots through the existing `BuildModel`. It projects the reconstructed and bound models through the same `ProjectModel`, then compares complete encoded bytes. The equality includes all schema-1 fields and variant tags, rather than only checksums. Map/set reconstruction normalizes record order; split member vectors retain their order. Lifecycle attachment and page-opening commands do not silently change Window selection or accepted owner records. Navigation and activity remain outside this projection by design.
3. Composed whole admission compares that graph before running `CheckAll`. Fresh coherent facts for a differently attached graph therefore cannot admit an older snapshot. Empty graphs still read peer reports. Window-only graphs still require current Window context observations. The shared Profile preparation and classifier remain the native-policy seams; no registry, classifier switch or duplicate codec was introduced.
4. Commit admits the decoded candidate, rather than the original or an unrelated supplied snapshot. The changed-attachment fixture successfully promotes a new graph over a different original, while refusing the old prepared candidate under admission for the new graph. Native and peer changes after preparation refuse without changing original or candidate bytes. `PreparedWrite` owns byte images only and retains no callback or native reference.
5. Repository-wide searches included ignored text outside `.git` and `node_modules`, completed without truncation or search errors, and covered `SnapshotAdmission`, `CommitPreparedWrite`, snapshot operations and header references. The admission producers are the composed adapter and explicitly synthetic builders in durability/schema tests. Commit callers are confined to durability, schema-gate and context-admission tests; every current caller supplies admission. The schema gate forwards admission unchanged. No production native caller was found. The old two-argument example in the prior review is a historical reproduction, not an unmigrated caller.
6. The added functions keep graph validation/projection/encoding inside durability and native observation policy inside the composition adapter. Their public surface is traceable. Borrowed callbacks still require live, stable model/source objects; copying an admission object does not extend their lifetime. The headers and feature document explicitly require serialized use, no reentrancy and object retention.

There is avoidable repeated work, but no new correctness finding: encoding validates a snapshot before the composed graph comparison reconstructs it again, and the composed Profile/Page callbacks each repeat the full native/peer scan after whole admission already performed it. This follows the existing per-record callback design. Canonical comparison also materializes projected records and encoded strings. These are qualitative costs, not measured latency or throughput claims. They do not justify a competing codec, cached permit or additional registry. No benchmark load was run.

## Coverage and verification

The new checked-in tests assert observable success/refusal and preservation, including the older-attachment snapshot with fresh wrong-graph facts, absent whole callbacks, Window-only refusal, retained empty-graph peer changes, and OTR/native/peer refusal after preparation. Each durable record category has a valid changed-graph refusal fixture. Candidate success against a different original guards against admitting the wrong image.

Independently compiled and executed the complete context-admission fixture suite with supplemental stdin-only probes. The temporary harness reused the unchanged fixture functions, changed its entry point to a void helper, then called that helper and the probes from a new `main`. No repository test input was edited. The first harness attempt failed to compile because renaming `main` removed its implicit-return rule and the harness lacked a namespace alias. Correcting those harness errors produced both passing runs below.

Actual independent runs used Clang 23.1.3 on Linux, `--driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic`, and the documented integration source list: lifecycle, Profile preparation, private operation boundary, durability, schema gate and context admission. The second build additionally used `-g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer`, with `ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1`. Both printed:

```text
composed persistence synthetic checks passed
independent snapshot admission contract probes passed
```

The supplemental probes reversed record vectors in a graph containing shortcuts and two splits, confirmed graph equality and encoding acceptance, then reversed one split's member order and confirmed valid-graph provenance refusal. They also removed the whole callback for an empty graph and confirmed refusal from encode, prepare, restore and commit.

Independently ran Chromium `clang-format --dry-run --Werror` on all nine scoped C++ headers/sources/tests. It passed with clang-format 23.1.3. Scoped `git diff --check` passed. The response's separate untracked-file whitespace check emitted no diagnostics; its exit status 1 represents the new-file difference.

Reused the primary's reported strict and fail-fast ASan/UBSan results for the separate durability and schema-gate suites, including unchanged corruption, checksum, recovery and stale-source fixtures. Those lower-level executables were not independently rerun in this review. The three feature documents and response commands use the final API and correct source lists. Their schema-1, lifecycle-v3 and unchanged-wire claims agree with the diff; the encoder body was extracted into a private helper without changing field order or bytes.

## Native obligations remain open

Callback presence proves invocation, not truthful native facts. Equal canonical graphs prove durable graph equality, not observation freshness, complete native enumeration, liveness or authorization. The host must retain actual native objects and serialize model/native/peer observations without reentrancy through final admission, staging and replacement. Coherent stale reports can still pass, as the documents disclose.

Actual IO, writer exclusion, staged readback, flush/replace/crash behavior, restart recovery, multi-Profile Window hosting, cross-Profile browser isolation, close-undo and Linux/Windows/macOS installed acceptance remain open. No native producer, Chromium preparation/build, heavy download, browser interaction or benchmark was run. This review made no commit, remote write or PR link/watch. The only repository output is this review.
