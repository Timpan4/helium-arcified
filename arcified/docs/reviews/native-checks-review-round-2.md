# Native checks independent review, round 2

Reviewed on 2026-10-23. The original P2 is resolved. No new actionable findings were found in the affected paths. This conclusion covers the lightweight workflow and its UBSan failure behavior, not hosted execution or native browser acceptance.

## Scope and regression assessment

Re-read `.github/workflows/arcified-native-checks.yml`, both files in `arcified/native/checks/`, `arcified/docs/native-checks.md`, the [original review](native-checks-review.md), and the [fix response](native-checks-fix-response.md).

The shared Linux/macOS sanitizer compilation and the documented local command both include `-fno-sanitize-recover=undefined`. The sanitizer executable remains a direct Bash command under `set -eu`, with no error suppression. Its nonzero status therefore fails the step.

Reversing only the new flag and its line wrapping in the current workflow reproduced the original review's workflow blob hash, `42caf2ee8296609032e7d62daa571fabfc53d328`. This establishes that the workflow has no other changes since that review. Both runners and all three standalone model files also match their original reviewed blob hashes. The documentation diff adds the matching flag and explanation, updates the fixture count to 16 v3 fixtures, and records the overflow probe. No regression was found in these changes.

The manual trigger, read-only permissions, disabled checkout credential persistence, platform selection, skip policy, scoped lint policy and Windows runner remain as previously reviewed. The workflow still contains no Chromium retrieval, preparation or browser build step.

## Independent focused verification

Used existing Clang 23.1.3 on Linux x86_64. Compiled this standalone source from standard input with the workflow's exact C++17 warning and sanitizer flags:

```cpp
#include <limits>
int main() {
  volatile int value = std::numeric_limits<int>::max();
  volatile int result = value + 1;
  (void)result;
  return 0;
}
```

Both builds used `--driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer`. The revised build additionally used `-fno-sanitize-recover=undefined`. Each execution cleared `UBSAN_OPTIONS` for the child process.

| Check | Observed result |
| --- | --- |
| Deliberate overflow with recovery disabled | Printed the signed-overflow UBSan diagnostic and exited 1. |
| Control with the new flag omitted | Printed the same diagnostic and exited 0. |
| Revised executable followed by a continuation marker inside `bash -e -o pipefail` | Bash exited 1; the continuation marker did not execute. |
| `bash -n` on all four explicit workflow Bash bodies | All passed. This checks Bash syntax, not Actions schema or expressions. |

The harness asserted all three exit statuses and exited 0. Probe outputs remain in `/tmp/tmp.8ViVDWmdEL`. No package installation was needed.

Reused the passing evidence in the original review and fix response rather than rerunning unchanged suites. Those reports record 41 Python tests plus both devutils function checks, 16 strict C++ fixtures, 16 sanitizer fixtures, scoped Python lint and C++ formatting. The fix response records the normal sanitizer fixtures passing with the revised flags. This round independently executed the overflow and Bash checks above; it does not claim a fresh execution of the normal suites or exact pinned tools.

## Remaining validation gaps

1. The workflow has not been dispatched. Hosted Linux, Windows PowerShell/MSVC and macOS compiler/sanitizer runtime behavior remain unverified. Windows symlink privileges, real macOS volume behavior and the documented platform coverage gaps remain open.
2. Actions schema/expression validation and PowerShell parsing remain unverified. Passing `bash -n` does not close either gap.
3. Python 3.10 and the exact Pylint 2.16.2, Astroid 2.14.2 and YAPF 0.32.0 environment remain unverified. Existing local success uses different versions.
4. Chromium source provenance, preparation, patch applicability, GN generation, browser builds and native browser acceptance remain open. Standalone fixtures do not prove browser integration, isolation, persistence, UI/UX, accessibility or performance, and do not close issues 16, 17, 18 or 24.

## Reviewed source identity

Working-tree `HEAD` was `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. These blob hashes identify the reviewed contents independently of commits or uncommitted work:

| File | Git blob hash |
| --- | --- |
| `.github/workflows/arcified-native-checks.yml` | `456c5a3e57c63aceccf1c65b43f949a06c9c3a33` |
| `arcified/native/checks/python_checks.py` | `ffd581aaa8580dec28043675240f62956e6ba1b7` |
| `arcified/native/checks/windows_model.ps1` | `35fdd70d657fa14edf32e221e57452c63b74288b` |
| `arcified/docs/native-checks.md` | `dd091d16474aecf1cb7e699ac4fd3eb578a578f7` |
| `arcified/native/model/lifecycle.h` | `d3a7aed6f1caddd40f5422bc6e5c1f5019636338` |
| `arcified/native/model/lifecycle.cc` | `7ffe87eb49d0cf4a92f83058b7c348ed3882012f` |
| `arcified/native/model/lifecycle_test.cc` | `af3a52f39d323e079b3799b306e1e756c9205893` |
| `arcified/docs/reviews/native-checks-review.md` | `eb1da6002772db15435c465a164c63e452108fac` |
| `arcified/docs/reviews/native-checks-fix-response.md` | `23c8ac611c1c1f6251b41b2155fe1e257d9e55bc` |

Only this round-two report was written in the repository. The original reports were preserved. No implementation edits, commits, remote writes, PR linking/watching, remote CI, Chromium retrieval, preparation or browser build ran.
