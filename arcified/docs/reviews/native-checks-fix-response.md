# Native checks P2 fix response

Fixed on 2026-10-23. The sole actionable P2 in [the independent review](native-checks-review.md) was that UBSan could diagnose undefined behavior and still let the sanitizer executable exit successfully.

## Change

Added `-fno-sanitize-recover=undefined` to the shared Linux/macOS sanitizer compilation in `.github/workflows/arcified-native-checks.yml` and the local command in `arcified/docs/native-checks.md`. UBSan findings now terminate with a nonzero exit status, which the existing Bash step propagates. The normal strict build and Windows runner need no sanitizer change.

The documentation also records the current 16 v3 fixtures and the overflow check. Neither runner in `arcified/native/checks/` changed. The original review is unchanged; its Git blob hash before and after this fix is `eb1da6002772db15435c465a164c63e452108fac`.

## Overflow reproduction

Used Clang 23.1.3 on Linux x86_64 with this standalone source, supplied on standard input:

```cpp
#include <limits>
int main() {
  volatile int value = std::numeric_limits<int>::max();
  volatile int result = value + 1;
  (void)result;
  return 0;
}
```

The revised compilation used the workflow's warning and sanitizer flags:

```sh
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-sanitize-recover=undefined \
  -fno-omit-frame-pointer -x c++ - -o "$check_output_dir/overflow-disabled"
env -u UBSAN_OPTIONS "$check_output_dir/overflow-disabled"
```

The deliberate-overflow executable printed `runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'` and exited **1**. A separate control compiled with the same flags except for omitting `-fno-sanitize-recover=undefined` printed the same diagnostic and exited **0**. Both executions cleared `UBSAN_OPTIONS` only for the child process so inherited runtime settings could not supply the behavior being tested. The verification harness asserted both observed exit statuses. Outputs were created in the fresh temporary directory `/tmp/tmp.htdSsghyD7`.

## Focused verification

All checks below exited 0 on Linux. No packages were installed.

| Check | Result |
| --- | --- |
| `env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ python3 -B arcified/native/checks/python_checks.py test` | All 41 discovered tests passed with no skips, followed by both devutils function checks. Python 3.14.8, Git 2.56.0 and GNU patch 2.8. Negative fixtures emitted their expected diagnostics. |
| Strict C++17 compilation and execution using the documented flags | All 16 v3 lifecycle fixtures passed with Clang 23.1.3. |
| ASan/UBSan compilation and execution using the revised documented flags | All 16 v3 lifecycle fixtures passed without sanitizer diagnostics. |
| `env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ /tmp/arcified-native-checks.vKx5Ht/bin/python -B arcified/native/checks/python_checks.py lint` | Scoped YAPF/Pylint checks passed using the existing environment: YAPF 0.43.0, Pylint 4.1.3 and Astroid 4.3.4. |
| `clang-format --style=Chromium --dry-run --Werror` on the model header, implementation and test | Passed with clang-format 23.1.3. |
| `bash -n` on every explicit workflow Bash body | All four bodies passed, including the one-line lint command. This checks Bash syntax, not Actions schema or expression validity. |

## Remaining limits

This closes the reproduced P2 through standalone Linux execution. Hosted Linux, macOS and Windows runs remain unexecuted. Actionlint is unavailable locally, so Actions schema validation remains unverified. Python 3.10 and the repository-pinned lint versions remain unexecuted. The original review's other validation gaps remain open.

No Chromium source retrieval, preparation or full browser build, remote workflow dispatch, global installation, push, PR or commit ran. The parent owns integration.
