# Snapshot admission lint review

Reviewed 2026-10-23. No actionable finding in the final test-only lint change.
The direct-list initializer is valid C++17, the checker-specific suppression is
justified intentional boundary coverage, and every prior refusal case and
assertion remains present. Independent focused compilation, execution,
sanitizers, formatting and the exact analyzer profile passed.

## Scope and exact inputs

This is bounded closure of the lint delta, not another general admission audit.
The original outcome remains mandatory whole-snapshot provenance and decoded
candidate admission at commit, covering old attachments, empty graphs and native
OTR changes after preparation. The original briefs, findings, adversarial
evidence and limits remain in
[snapshot-admission-correctness-review.md](snapshot-admission-correctness-review.md)
and [snapshot-admission-contract-review.md](snapshot-admission-contract-review.md),
with implementation context in
[snapshot-admission-response.md](snapshot-admission-response.md).

Compared the actual Git blobs
`0917d9a7dda660d666fa532e69943aca3233bab6` and
`1bc9308f72f2ba8033428aa1d3139dc2a17d8fd0` directly with `git diff`.
There is one hunk: four lines introduce the commented local constant, and one
line replaces the inline cast with that constant. The rest of the test is
byte-for-byte unchanged. The working test matches the latter blob.

Independently hashed all thirteen inputs pinned by both prior reviews. All
twelve other entries still match their prior hashes. Paths below are relative
to `arcified/`; these are Git blob hashes, not commit IDs.

| File | Current blob hash |
| --- | --- |
| `native/features/durability/session_snapshot.h` | `a9575d2ac7bfee06a958c5fbe702901a37db2031` |
| `native/features/durability/session_snapshot.cc` | `b4ea8d306ad0b223a7be4b44d52d24213f6f1893` |
| `native/features/durability/session_snapshot_test.cc` | `c4f84edaf1bef20957e11d8399955fcb04e760a5` |
| `native/features/migrations/schema_gate.h` | `c2bf17e3bdd13f5a341bc4ea34b2e4a4ba08477e` |
| `native/features/migrations/schema_gate.cc` | `2de10d80b0feb4e3d8f090a4ce013fd115c99ada` |
| `native/features/migrations/schema_gate_test.cc` | `71c239e0d02bef2ec3b0e79f1e037221df9b5320` |
| `native/integration/persistence/context_admission.h` | `bc5cf454c9912678753015cee477a582397cf96e` |
| `native/integration/persistence/context_admission.cc` | `12196511af86c78bf64ef65dde3d814235b5688d` |
| `native/integration/persistence/context_admission_test.cc` | `1bc9308f72f2ba8033428aa1d3139dc2a17d8fd0` |
| `docs/features/session-durability.md` | `4bb5d4168d582816889f0ef6e93245e708176856` |
| `docs/features/schema-migrations.md` | `5ba7a6212ccf9609731f053ff2d0cc581ea18356` |
| `docs/features/context-persistence-composition.md` | `3e1e855dba60ecf1a37aee5a5a1d8be78a2c1707` |
| `docs/reviews/snapshot-admission-response.md` | `8967c62a0db6842d6e41073b0c50188e18188016` |

Additional inputs:

| File | Blob hash |
| --- | --- |
| `docs/reviews/snapshot-admission-lint-response.md` | `3b014d3228f21fa0c5922c6e5fc24393099ac07a` |
| `native/features/private_browsing/context_boundary.h` | `c20ddbf27e6f250b87cfe6281de9a8bc828390e8` |
| `docs/reviews/snapshot-admission-correctness-review.md` | `0eeb9d61fcb381ed7b4b7a47188400757b0fda5d` |
| `docs/reviews/snapshot-admission-contract-review.md` | `e5eefb60a902d0d9ce3811e3d28464aebfabb148` |

The shared classifier hash matches the lint response. A separate diff against
the prior reviews' baseline `0ec717d36788af297d68d658be80d3f1846b2a06`
also found no changes in lifecycle headers/implementation, Profile preparation
headers/implementation or the private-browsing subtree. No production fix or
classifier weakening accompanies this lint change.

## Language and suppression findings

`NativeContextKind` is a scoped enum with no explicit underlying type. C++17
therefore fixes its underlying type as `int`; its values include all values of
that type. The unnamed value `-1` is representable and valid for this C++ type.
It is outside the declared enumerator set, which is the deliberate test input.
This follows N4659 [dcl.enum paragraphs 5 and 8](https://timsong-cpp.github.io/cppwp/n4659/dcl.enum).

`constexpr NativeContextKind kUnrecognizedKind{-1};` uses direct-list
initialization with a single `int` element and no narrowing conversion. That
form is expressly permitted for a fixed-underlying enum by C++17
[dcl.init.list paragraph 3.7](https://timsong-cpp.github.io/cppwp/n4659/dcl.init.list).
The actual final translation unit also compiled under strict C++17 flags.
Neither the old cast nor its replacement introduces an out-of-range language
value or undefined enum behavior here.

LLVM documents `optin.core.EnumCastOutOfRange` as enforcing correspondence to a
declared enumerator, explicitly distinguishing that policy from undefined
behavior. The diagnostic is expected for this deliberately unnamed value.
[LLVM checker contract](https://clang.llvm.org/docs/analyzer/checkers.html#optin-core-enumcastoutofrange-c-c).
The exact `NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)` targets
only that check on the initializer, using the documented
[next-line suppression mechanism](https://clang.llvm.org/extra/clang-tidy/index.html#suppressing-undesired-diagnostics).
It does not suppress compiler type errors, other analyzer checks or the loop's
assertions. No wildcard, enclosing suppression or global configuration change
is involved. This is a justified test exception, not concealment of a type bug.

## Preserved refusal coverage

The loop still contains `kUnknown`, `kOffTheRecord`, both Guest kinds, both
System kinds, `kOther` and the same unnamed `-1` value. Its operation-policy
assertion remains, followed by injection into the unattached Profile, each of
`kSaved`, `kTemporary` and `kHomePage`, and the empty Window's context. Inspection
of the unchanged shared classifier confirms that the unnamed input reaches its
final `kUnknown` fallback when identity facts are otherwise present. Replacing
it with the named `kUnknown` would lose that distinct coverage; this change
retains it.

`RefusalPreserves` is unchanged. For every injected case it checks composition
refusal, then refusal through retained-admission capture, encode, gate
preparation, restore and prepared commit. It retains the original/candidate
image, source, navigation and recovery-after-restoring-observations assertions.
The complete test entry point and all other fixtures are also unchanged by the
blob diff. No fail-closed case or preservation assertion was removed.

## Independently executed checks

Used the existing Linux Clang, clang-format and clang-tidy 23.1.3. Rebuilt and
ran the complete composed integration fixture with the exact seven-source list
in [snapshot-admission-lint-response.md](snapshot-admission-lint-response.md):
lifecycle, Profile preparation, private operation boundary, durability, schema
gate, context admission and its test.

1. Strict build and execution used `--driver-mode=g++ -std=c++17 -Wall -Wextra
   -Werror -pedantic`. The executable
   `/tmp/snapshot-admission-lint-review.vKUaWlTG/context-strict` exited 0 and
   printed `composed persistence synthetic checks passed`.
2. A separate build added `-g -fsanitize=address,undefined
   -fno-sanitize-recover=all -fno-omit-frame-pointer`. Execution used
   `ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1`. The executable
   `/tmp/snapshot-admission-lint-review.vKUaWlTG/context-sanitized` exited 0 with
   the same output and no sanitizer diagnostics.
3. `clang-format --style=Chromium --dry-run --Werror` passed on the changed test.
   The tracked test's `git diff --check` also passed.
4. The exact focused analyzer invocation below exited 0 and reported exactly
   `Suppressed 1 warnings (1 NOLINT).` There were no unsuppressed diagnostics.

```sh
clang-tidy --checks='-*,clang-analyzer-*,bugprone-use-after-move' \
  --warnings-as-errors='*' \
  arcified/native/integration/persistence/context_admission_test.cc \
  -- -std=c++17 -Wall -Wextra -Werror -pedantic
```

The thirteen scoped file hashes and the additional input hashes were checked
again after these executions and remained unchanged. These are fresh reviewer
runs, not the primary's reported check results.

## Reused evidence and open limits

Reused the prior correctness and contract reviews for the unchanged production
implementation and their strict/sanitized adversarial probes. Did not rerun the
separate durability/schema suites, the supplemental provenance probes or the
parent's other analyzer translation units. Reused the lint response's original
cast diagnostic and syntax-comparison experiment; did not reconstruct those
experiments independently. The fresh full integration executions above cover
the final initializer and all checked-in composed fixtures.

Native truth, liveness, completeness and freshness remain OPEN, as do object
retention, serialized execution without reentrancy, actual IO/locking/flush/
replacement/crash durability and Linux/Windows/macOS browser acceptance. This
review proves only the bounded pure-test lint closure. No native producer,
Chromium preparation/full build, browser interaction, remote write, PR link/watch
or commit was performed. The only repository file written by this review is
this report; source files and prior reports were preserved.
