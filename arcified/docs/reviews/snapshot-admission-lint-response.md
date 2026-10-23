# Snapshot admission lint response

Implemented and checked on 2026-10-23. The affected integration test passes the
requested strict C++17, fail-fast ASan/UBSan, Chromium formatting and exact
clang-tidy profile. One intentional enum injection has a checker-specific,
single-line suppression. Fresh independent closure remains the parent's next
step.

## Exact change

The reviewed `native/integration/persistence/context_admission_test.cc` blob was
`0917d9a7dda660d666fa532e69943aca3233bab6`. The final Git blob hash is
`1bc9308f72f2ba8033428aa1d3139dc2a17d8fd0`.

In `SharedClassifierRefusals`, replaced the inline
`static_cast<NativeContextKind>(-1)` with a local named constant:

```cpp
  // C++17 permits this for a scoped enum's fixed int underlying type.
  // Intentionally unnamed to exercise fail-closed boundary handling.
  // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
  constexpr NativeContextKind kUnrecognizedKind{-1};
```

The existing refusal loop now uses `kUnrecognizedKind`. The value `-1` comes
from the reviewed fixture. No test case or assertion was removed. The direct
list initializer requires a fixed underlying type and a non-narrowing value,
so it also gives a compiler error if a future type change violates those
requirements. No production enum, classifier, admission code or global check
configuration changed. The shared `context_boundary.h` blob remains
`c20ddbf27e6f250b87cfe6281de9a8bc828390e8`.

## Language validity and checker contract

The original cast is well-defined, rather than undefined behavior. C++17 draft
N4659 [dcl.enum paragraphs 5 and 8](https://timsong-cpp.github.io/cppwp/n4659/dcl.enum)
gives a scoped enum without an explicit underlying type a fixed `int` underlying
type, and gives a fixed enum all values of that underlying type. Thus `-1` is a
valid value of this C++ type, although no declared enumerator names it.
[expr.static.cast paragraph 10](https://timsong-cpp.github.io/cppwp/n4659/expr.static.cast)
permits the original conversion within the enum's value range.
[dcl.init.list paragraph 3.7](https://timsong-cpp.github.io/cppwp/n4659/dcl.init.list)
permits the replacement direct-list initialization without narrowing. Here the
input and underlying type are both `int`.

LLVM's [EnumCastOutOfRange checker documentation](https://clang.llvm.org/docs/analyzer/checkers.html#optin-core-enumcastoutofrange-c-c)
defines its policy as detecting integer conversions with no corresponding
enumerator and explicitly distinguishes that policy from undefined behavior.
The [LLVM 23.1.3 checker implementation](https://github.com/llvm/llvm-project/blob/llvmorg-23.1.3/clang/lib/StaticAnalyzer/Checkers/EnumCastOutOfRangeChecker.cpp#L65-L72)
collects declared enumerator values. Its later matching logic reports when none
can equal the injected value. It does not exempt scoped enums with fixed
underlying types. The diagnostic is therefore expected under this policy even
though the test intentionally supplies valid C++ outside the named domain.

Reproduced the parent's exact failure on the original test at line 245, exit 1:

```text
The value '-1' provided to the cast expression is not in the valid range of values for 'NativeContextKind' [clang-analyzer-optin.core.EnumCastOutOfRange,-warnings-as-errors]
```

A separate temporary C++17 probe with a scoped enum confirmed that explicit
casts, local `const` direct-list initialization and local `constexpr` direct-list
initialization all produce this diagnostic. Changing syntax alone does not
resolve the checker policy conflict. The final representation names the test's
intent and retains transparent initialization. It uses the documented
[check-specific NOLINTNEXTLINE mechanism](https://clang.llvm.org/extra/clang-tidy/index.html#suppressing-undesired-diagnostics)
on that initializer only. It does not hide the conversion through runtime
indirection, move it outside analyzed functions or weaken the shared classifier.

## Preserved behavior coverage

The loop retains `kUnknown`, OTR, both Guest kinds, both System kinds, `kOther`
and the unrecognized value. Each still checks shared operation-policy refusal
and injects the kind into an unattached Profile, each fixture Page and the empty
Window's context. `RefusalPreserves` still checks composition, retained-admission
capture, encode, gate preparation, restore and prepared commit refusal. It
asserts unchanged original/candidate bytes and succeeds again after restoring
the observations. The named unknown and unsupported states do not replace the
unrecognized value's coverage of the classifier's final unknown fallback.

## Executed checks

Used the existing Linux Clang, clang-tidy and clang-format 23.1.3. Commands ran
from the repository root with the integration source list already recorded in
[snapshot-admission-response.md](snapshot-admission-response.md):

```sh
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/profiles/preparation.cc \
  arcified/native/features/private_browsing/operation_boundary.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/integration/persistence/context_admission.cc \
  arcified/native/integration/persistence/context_admission_test.cc \
  -o /tmp/arcified-snapshot-admission-lint.YyU0JHxG/context-persistence-test && \
  /tmp/arcified-snapshot-admission-lint.YyU0JHxG/context-persistence-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-sanitize-recover=all \
  -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/profiles/preparation.cc \
  arcified/native/features/private_browsing/operation_boundary.cc \
  arcified/native/features/durability/session_snapshot.cc \
  arcified/native/features/migrations/schema_gate.cc \
  arcified/native/integration/persistence/context_admission.cc \
  arcified/native/integration/persistence/context_admission_test.cc \
  -o /tmp/arcified-snapshot-admission-lint.YyU0JHxG/context-persistence-sanitized && \
  ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  /tmp/arcified-snapshot-admission-lint.YyU0JHxG/context-persistence-sanitized
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/integration/persistence/context_admission_test.cc
clang-tidy --checks='-*,clang-analyzer-*,bugprone-use-after-move' \
  --warnings-as-errors='*' \
  arcified/native/integration/persistence/context_admission_test.cc \
  -- -std=c++17 -Wall -Wextra -Werror -pedantic
git diff --check -- \
  arcified/native/integration/persistence/context_admission_test.cc
git diff --no-index --check -- /dev/null \
  arcified/docs/reviews/snapshot-admission-lint-response.md
```

Both integration executables exited 0 and printed
`composed persistence synthetic checks passed`, with no compiler or sanitizer
diagnostics. Formatting and tracked whitespace checks exited 0. Clang-tidy
exited 0 and reported exactly `Suppressed 1 warnings (1 NOLINT).` The response's
new-file whitespace check emitted no diagnostics; its exit 1 denotes a new-file
difference.

## Evidence limits and handoff

The prior clean correctness and contract reviews apply to the original test
blob and other reviewed inputs. This response does not claim they independently
reviewed the lint change. The parent's previously passing five translation units
were not rerun through clang-tidy here; only the changed test translation unit
was analyzed. The focused integration executables compiled all sources listed
above and ran the complete integration fixture suite. Separate durability and
schema-gate executables were not rerun for this test-only change.

Native observation liveness, freshness and completeness, real persistence IO,
crash durability and Linux/Windows/macOS browser acceptance remain unverified.
No source preparation, full Chromium/native build, live browser operation,
remote write or commit ran. Only this response and the owned integration test
were changed in the repository. The parent owns fresh independent closure.
