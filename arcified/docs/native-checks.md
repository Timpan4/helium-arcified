# Native foundation checks

[The manual workflow](../../.github/workflows/arcified-native-checks.yml) prepares lightweight validation for issues 16, 17, 18 and 24. It has only `workflow_dispatch`, read-only repository permissions, no submodule checkout and no Chromium source-validation or browser build job. Existing workflows and required checks are unchanged. Publication, dispatch and broader integration belong to the parent task. No workflow was dispatched during this implementation.

## Coverage and prerequisites

| Check | Future hosted execution | What it exercises |
| --- | --- | --- |
| Python components | Ubuntu 26.04 x64, Windows Server 2025 x64, macOS 26 Intel | Every `test_*.py` under `arcified/native/tooling`, including shared input identity, Linux, Windows and Mac preparation contracts. Each host runs all component suites. |
| Devutils component | Same hosts | The script entrypoint in `devutils/tests/test_check_patch_files.py`, which currently invokes both function tests. These functions are not discovered by `unittest`. |
| Standalone C++ | Same hosts | Compile and execute `lifecycle.cc` with `lifecycle_test.cc`. The current model has 13 v2 fixtures, including borrowed arguments and Profile/Space transition groups. |
| C++ sanitizers | Linux and macOS | Compile and execute the same fixtures with AddressSanitizer and UndefinedBehaviorSanitizer. Windows sanitizer coverage is not supplied. |
| Scoped style | Linux | YAPF using `.style.yapf`, the reviewed scoped Pylint checks, and Chromium-style `clang-format` on the standalone C++ files. |

Python selection reads [`.python-version`](../../.python-version), currently 3.10. This is component compatibility policy, not a claim that the native Windows or Mac browser preparation toolchains support that baseline. The hosted images supply Git, patch and C++ tools. Missing tools fail visibly; these runners do not install compilers or change privilege settings.

Tests create temporary synthetic Git repositories and patch small fixture files. Some existing identity/devutils fixtures deliberately retain their temporary directories. The Windows C++ runner also retains its fresh output directory and prints its path. There is no source retrieval, native preparation entrypoint, GN generation, browser compilation or browser launch.

When eventually dispatched, each component job prints the actual Python version and executable, OS/architecture, runner image identifiers, commit SHA, Git and patch versions. Linux/Mac steps print Clang's version; Mac also prints the selected Xcode and SDK versions. Windows prints PowerShell/.NET, compiler path, VC toolset and SDK identifiers, plus MSVC's `/Bv` output during compilation. The style job prints YAPF, Pylint/Astroid and clang-format versions. Keep the Actions logs as the execution record. Image labels and action tags do not establish actual tool versions.

## Exact local invocations

Run from the repository root with existing tools. No installation is required for the Python tests beyond the repository's vendored modules, Python, Git and patch:

```sh
python3 -B arcified/native/checks/python_checks.py test
```

The runner clears inherited `PYTHONEXECUTABLE` and `__PYVENV_LAUNCHER__` for its children. On a T3 host that injects these variables, clear them before launching the interpreter as well:

```sh
env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ \
  python3 -B arcified/native/checks/python_checks.py test
```

On Windows, use PowerShell and the installed Git for Windows patch executable. This changes only the current process environment:

```powershell
$ErrorActionPreference = 'Stop'
Remove-Item Env:PYTHONEXECUTABLE, Env:__PYVENV_LAUNCHER__ -ErrorAction SilentlyContinue
$git = (Get-Command git.exe -ErrorAction Stop).Source
$env:PATCH_BIN = Join-Path (Split-Path (Split-Path $git)) 'usr\bin\patch.exe'
if (-not (Test-Path -LiteralPath $env:PATCH_BIN -PathType Leaf)) {
    throw "Git for Windows patch.exe is unavailable at $env:PATCH_BIN"
}
python -B arcified/native/checks/python_checks.py test
if ($LASTEXITCODE -ne 0) { throw "Python component checks failed: $LASTEXITCODE" }
```

For Linux or macOS, the following uses the [existing model warning and sanitizer policy](lifecycle-contract.md). Use a fresh directory to avoid stale binaries:

```sh
set -eu
check_output_dir=$(mktemp -d)
clang++ --version
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc arcified/native/model/lifecycle_test.cc \
  -o "$check_output_dir/lifecycle-test"
"$check_output_dir/lifecycle-test"
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc arcified/native/model/lifecycle_test.cc \
  -o "$check_output_dir/lifecycle-sanitized"
"$check_output_dir/lifecycle-sanitized"
```

For Windows, use the standalone runner from PowerShell:

```powershell
./arcified/native/checks/windows_model.ps1
```

It discovers installed C++ tools with `vswhere.exe`, starts Visual Studio's Developer PowerShell for an amd64 host and target in the current process, and compiles with native MSVC arguments. `/TP` treats the `.cc` sources as C++; `/std:c++17 /EHsc /W4 /WX /permissive-` requests C++17, exception semantics, strict warnings and conformance. `/W4 /WX` preserves warning failures, but its diagnostics differ from Clang's. No Unix shell, Unix compiler flags, hardcoded Visual Studio release directory or `/tmp` path is used. It checks compilation and executable exit codes separately. It does not change execution policy if PowerShell refuses the installed developer script.

## Scoped formatting and lint policy

With an existing interpreter containing the project's lint tools:

```sh
python3 -B arcified/native/checks/python_checks.py lint
clang-format --version
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/model/lifecycle.h \
  arcified/native/model/lifecycle.cc \
  arcified/native/model/lifecycle_test.cc
```

The Python command checks all native tooling Python files, the new check runner and the two affected devutils files with YAPF. It then checks native tooling with Pylint's `--persistent=n --disable=duplicate-code`, devutils with `--persistent=n --disable=wrong-import-position,duplicate-code`, and the new runner without diagnostic exclusions. It sets `PYTHONPATH` in `devutils`, `utils`, `arcified/native/tooling` order using the host path separator. The order matters because both utility trees contain `third_party` packages.

These exclusions and import paths reuse [the reviewed scoped policy](reviews/native-input-fix-response.md) and [`run_devutils_pylint.py`](../../devutils/run_devutils_pylint.py). No error diagnostic is disabled. Default combined Pylint is not clean: the recorded default invocation reports `_common` import resolution (`E0401`) and cross-file duplication (`R0801`). Passing this workflow does not mean default combined or repository-wide lint passes.

Only a future manual style job creates an isolated environment under `RUNNER_TEMP` and installs `pylint` and `yapf` constrained by [`.requirements.txt`](../../.requirements.txt). Its existing pins include Pylint 2.16.2, Astroid 2.14.2 and YAPF 0.32.0. No dependency versions are added or upgraded here. The job does not reuse the shared CI setup action, which installs host packages and is outside this workflow's scope. No packages were installed during local implementation or validation.

## Platform availability and coverage gaps

All tests remain selected on every OS. The runner fails on test errors, assertion failures, empty discovery and skipped required fixtures. In particular, existing symlink fixtures may skip when the host lacks symlink support or privileges. The runner prints their test IDs/reasons and returns failure. Windows may require Developer Mode or the symbolic-link privilege; granting either remains an owner/host decision. There is no automatic elevation or privilege change.

The existing `test_macos.MacosToolingTests.test_distinct_case_sensitive_siblings_allow_new_work` skips itself with `Fixture volume has case-insensitive lookup` when its fixture volume cannot represent distinct sibling names differing only in case. The runner reports and permits only that exact test ID/reason pair. This is an explicit availability gap, not evidence that this scenario ran. The same test must still execute on a case-sensitive volume, as it did in local Linux validation. Other Mac isolation and case-insensitive emulation fixtures remain selected. No platform-wide exclusion or `continue-on-error` hides failures.

Git for Windows must include `usr\bin\patch.exe` at the path derived from `git.exe`. The workflow sets `PATCH_BIN` directly, without assuming a Unix PATH or installing MSYS tools. Windows directory junction behavior is not covered by a junction-specific fixture. Symlink checks do not prove junction handling. Real macOS/APFS lookup, compiler/SDK availability, Windows execution policy, and runtime differences still need their own host runs. The selected Mac matrix entry covers Intel only; Apple Silicon and Windows ARM execution remain outside this matrix.

## Version sources checked

On 2026-10-23, the official [checkout release](https://github.com/actions/checkout/releases/tag/v7.0.1) and [setup-python release](https://github.com/actions/setup-python/releases/tag/v7.1.0) identified the action versions used here. The [runner image catalog](https://github.com/actions/runner-images#available-images) lists the selected labels. The official [Ubuntu image inventory](https://github.com/actions/runner-images/blob/main/images/ubuntu/Ubuntu2604-Readme.md), [Windows image inventory](https://github.com/actions/runner-images/blob/main/images/windows/Windows2025-VS2026-Readme.md) and [Mac image inventory](https://github.com/actions/runner-images/blob/main/images/macos/macos-26-Readme.md) informed tool availability, which must still be checked at execution.

The existing Python pins were checked against the publishers' [Pylint metadata](https://pypi.org/project/pylint/2.16.2/) and [YAPF metadata](https://pypi.org/project/yapf/0.32.0/); repository policy controls these retained versions. Windows setup follows Microsoft's [C++ tool discovery](https://github.com/microsoft/vswhere/wiki/Find-VC), [Developer PowerShell](https://learn.microsoft.com/en-us/visualstudio/ide/reference/command-prompt-powershell), [warning options](https://learn.microsoft.com/en-us/cpp/build/reference/compiler-option-warning-level) and [source-type options](https://learn.microsoft.com/en-us/cpp/build/reference/tc-tp-tc-tp-specify-source-file-type) documentation.

## Local evidence and remaining execution

Prepared on Linux x86_64 with Python 3.14.8, Git 2.56.0, GNU patch 2.8 and Clang/clang-format 23.1.3. An already existing lint environment supplied YAPF 0.43.0, Pylint 4.1.3 and Astroid 4.3.4. These local tool versions differ from the future CI Python/lint selection.

| Validation | Result |
| --- | --- |
| Component runner on Linux | All 41 discovered tests passed with no skips, plus both devutils script functions. |
| Standalone strict C++ compilation/run on Linux | All 13 v2 fixtures passed. |
| Standalone ASan/UBSan compilation/run on Linux | All 13 v2 fixtures passed with no sanitizer diagnostics. |
| Scoped YAPF/Pylint command | Passed on the focused files with the existing local tool versions above. Default combined lint remains failing as recorded in the reviews. |
| Chromium-style C++ formatting | Passed for all three model files. |
| Runner result handling | Small mocked checks passed for success, assertion failure, missing symlink support, the exact case-insensitive-volume skip, an unexpected skip reason, and empty discovery. |
| Workflow static checks | Read the trigger and job definitions; `bash -n` passed for every Bash block and the one-line lint command. New-file whitespace checks produced no diagnostics. |

Windows PowerShell/MSVC and macOS execution remain unexecuted. PowerShell and actionlint were unavailable locally, so PowerShell parsing and Actions schema validation are unverified. Python 3.10 and the exact repository-pinned lint environment remain unexecuted locally. The Actions workflow has not run. These checks do not prove native browser preparation, archive integrity, real Chromium patch applicability, GN generation, browser compilation or launch, BrowserContext/window integration, authentication isolation, renderer/history preservation, persistence, UI/UX, accessibility or performance. They do not close the native platform issues or issue 24.
