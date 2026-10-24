# Native checks independent review

Reviewed on 2026-10-23. One P2 defect needs correction before relying on the sanitizer job. The remaining unavailable checks are validation gaps, not demonstrated source defects.

Scope: `.github/workflows/arcified-native-checks.yml`, both runners in `arcified/native/checks/`, and `arcified/docs/native-checks.md`. Supporting component tests, repository Python/lint policy and current publisher references were inspected. No workflow dispatch, push, installation, Chromium source retrieval, browser preparation or Chromium build ran. Only this report was changed by the reviewer.

## Actionable finding

### P2: UBSan diagnostics can leave the sanitizer step green

Location: `.github/workflows/arcified-native-checks.yml:63`, with execution at line 66. The local reproduction in `arcified/docs/native-checks.md:61` has the same behavior.

The compiler command enables `-fsanitize=address,undefined` but does not disable UBSan recovery. Many undefined-behavior checks print a diagnostic and continue. A test executable can therefore finish successfully after detecting undefined behavior. Bash's error handling cannot catch a diagnostic from a process that exits zero. This affects the Linux and macOS sanitizer step; it is not a finding that the current lifecycle model contains undefined behavior.

I compiled this independent fixture with the workflow's exact warning and sanitizer flags, using local Clang 23.1.3:

```cpp
#include <limits>
int main() {
  volatile int value = std::numeric_limits<int>::max();
  volatile int result = value + 1;
  (void)result;
  return 0;
}
```

It printed `runtime error: signed integer overflow` and exited **0**. Adding `-fno-sanitize-recover=undefined` to the same compilation made it exit **1**. LLVM documents recovery and this option in its [UBSan error-handling reference](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html#usage).

1. Add `-fno-sanitize-recover=undefined` to the workflow's sanitizer compilation and the documented local command.
2. Confirm the deliberate-overflow fixture exits nonzero with the revised flags, then rerun the standalone lifecycle fixtures. Keep this validation standalone; no Chromium job is needed.

## Workflow and platform assessment

The only event is `workflow_dispatch`. Permissions are `contents: read`; checkout disables persisted credentials and leaves submodule checkout off. There are no repository write commands, reusable setup actions that install host packages, source download/preparation entrypoints, GN invocations or Chromium compilation steps. The Python fixtures use synthetic inputs, including tiny real patch applications. The style job's future installation is confined to a virtual environment under `RUNNER_TEMP`.

The existing tracked workflows have no working-tree diff. This review did not query or change remote branch-protection settings, so it does not independently establish the state of required-check settings on GitHub.

The selected [checkout v7.0.1](https://github.com/actions/checkout/releases/tag/v7.0.1) and [setup-python v7.1.0](https://github.com/actions/setup-python/releases/tag/v7.1.0) releases exist and were marked latest when checked. The setup-python input is declared in its [tagged action metadata](https://github.com/actions/setup-python/blob/v7.1.0/action.yml). The [official image catalog](https://github.com/actions/runner-images#available-images) lists `ubuntu-26.04`, `windows-2025` and `macos-26-intel` with the intended x64 architectures. No invalid action reference or runner label was found.

GitHub's [shell semantics](https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax#exit-codes-and-error-action-preference) support the Python step's native exit propagation under default Windows PowerShell and the explicit Bash steps' failure handling. `!cancelled()` deliberately allows the standalone C++ and formatting checks to run after another step fails; it does not turn that earlier failure into success. There is no `continue-on-error`.

Platform details were assessed as follows:

| Area | Static assessment | Execution limit |
| --- | --- | --- |
| Linux | The quoted output paths and Clang arguments work locally. The [Ubuntu inventory](https://github.com/actions/runner-images/blob/main/images/ubuntu/Ubuntu2604-Readme.md) lists Clang and clang-format. | The actual hosted image and its selected tool versions have not run here. |
| Windows patch lookup | Deriving Git's root from `git.exe`, joining `usr\bin\patch.exe`, checking existence and setting `PATCH_BIN` avoids a Unix PATH assumption. A missing executable fails visibly. | The actual hosted Git path and patch execution remain untested. |
| Windows MSVC setup | `vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64` follows Microsoft's [discovery guidance](https://github.com/microsoft/vswhere/wiki/Find-VC). The [Windows inventory](https://github.com/actions/runner-images/blob/main/images/windows/Windows2025-VS2026-Readme.md) lists that component and VSWhere. | No PowerShell parser or MSVC runtime was available locally. |
| Windows compiler invocation | Developer PowerShell uses documented `-Arch amd64 -HostArch amd64 -SkipAutomaticLocation` options. Source paths remain separate array arguments, `/TP` selects C++, output goes into a fresh native temporary directory, and compiler/executable exit codes are checked separately. `finally` restores the working directory. | Quoting, execution policy, developer-shell initialization, compilation and executable failure propagation need a Windows run. |
| Windows environment logging | The runner prints selected OS/toolchain identifiers. It contains no environment enumeration, `cmd /c set` dump, secret context expression or execution-policy override. Microsoft's [Developer PowerShell reference](https://learn.microsoft.com/en-us/visualstudio/ide/reference/command-prompt-powershell) supports the initialization method used. | This is a source assessment, not an inspection of an actual Windows setup log. |
| macOS | Bash paths and Clang flags are appropriate for standalone execution. The [Intel image inventory](https://github.com/actions/runner-images/blob/main/images/macos/macos-26-Readme.md) lists Xcode and SDK installations. | Selected SDK/compiler behavior, sanitizer runtime and real APFS lookup remain unexecuted. |

No additional concrete Windows or macOS defect was established by source inspection. Windows symlink privileges may still prevent a complete job. The runner treats those skips as failures instead of granting privileges or claiming coverage. Its single permitted skip requires the exact case-sensitive-sibling test ID and the exact case-insensitive-volume reason. That exception explicitly leaves the case-sensitive scenario unexecuted on such a volume.

## Python baseline and scoped lint

`.python-version` selects Python 3.10, and the workflow reads that file rather than selecting the image default. The current [Python distribution manifest](https://github.com/actions/python-versions/blob/main/versions-manifest.json) includes 3.10 x64 distributions for Ubuntu 26.04 and Darwin. The Windows image inventory lists cached Python 3.10.11. These availability references do not prove successful baseline execution.

The style preparation command uses `pip install -c .requirements.txt pylint yapf`. This constrains Pylint to 2.16.2, its Astroid dependency to 2.14.2 and YAPF to 0.32.0 without installing unrelated requirements. Publisher metadata for [Pylint](https://pypi.org/project/pylint/2.16.2/) and [Astroid](https://pypi.org/project/astroid/2.14.2/) includes Python 3.10 support. The pinned [YAPF implementation](https://github.com/google/yapf/blob/v0.32.0/yapf/__init__.py) returns nonzero when `--diff` finds changes. No dependency-resolution or compatibility defect was established, but the exact pinned environment was not executed.

The scoped policy does not disable error diagnostics. It supplies the documented `PYTHONPATH` using `os.pathsep`, disables only `duplicate-code` for native tooling, and additionally disables `wrong-import-position` for the affected devutils files. The latter exclusions match the inspected existing `devutils/run_devutils_pylint.py` policy. The new runner has no command-line diagnostic exclusions. Every lint subprocess uses `check=True`, so diagnostics with a nonzero Pylint exit status fail the runner.

I also ran an unconfigured combined Pylint invocation over all eight native tooling files, the new Python runner and both affected devutils files. It exited **26**, reporting `E0401`, `C0413` and `R0801`. This wider invocation differs from the previously recorded native-only default command. It confirms that scoped success must not be described as default combined or repository-wide lint success. Scoped success fixes import search paths and accepts the stated non-error exclusions; it does not resolve duplicated platform code. The exact pinned Pylint/Astroid versions still need to confirm this policy behaves as intended.

## Independent local checks

These checks used existing tools on Linux x86_64. No packages were installed.

| Check | Observed result |
| --- | --- |
| `env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ python3 -B arcified/native/checks/python_checks.py test` | Exit 0 with Python 3.14.8. All 41 discovered component tests passed with no skips; the devutils entrypoint completed both function checks. Expected malformed/unused-patch diagnostics came from negative fixtures. |
| Tiny mocked runner result checks | Success and the exact documented volume skip returned 0. Assertion failure, exception, missing-symlink skip, unexpected skip reason and empty discovery returned 1. A mocked failing subprocess propagated `CalledProcessError` with its original return code. |
| `python_checks.py lint` in the existing `/tmp/arcified-native-checks.vKx5Ht` environment | Exit 0 with YAPF 0.43.0, Pylint 4.1.3 and Astroid 4.3.4. This does not validate the older CI pins. |
| Strict standalone C++ compilation and execution | Exit 0 with Clang 23.1.3 and the workflow's C++17 warning flags. All 16 current v3 fixtures passed. |
| Standalone ASan/UBSan compilation and execution | All 16 current v3 fixtures passed with no sanitizer diagnostics. The separate deliberate-overflow probe demonstrated finding P2 above. |
| `clang-format --style=Chromium --dry-run --Werror` | Exit 0 for the header, implementation and test file with clang-format 23.1.3. |
| `bash -n` | Exit 0 for all four explicit Bash run bodies, including the one-line lint command. This is Bash syntax validation, not Actions schema validation. |

Model policy adoption was concurrent and outside this review's editing scope. The worker's earlier 13 v2 fixtures and this review's 16 v3 fixtures describe different source states. The implementation owner should reconcile the coverage table after that adoption settles. These checks prove only the standalone target's observed behavior.

## Remaining validation and native acceptance gaps

The following are unavailable or deferred validation, separate from the reproduced P2 defect:

1. Actions schema/expression validation with actionlint remains unverified. Actionlint was unavailable, and no YAML parser was used. Manual source and publisher-reference checks are not a substitute for schema validation. The workflow has never been dispatched.
2. PowerShell parsing and actual Windows patch, Developer PowerShell, MSVC compile/run, symlink and exit-code paths remain unverified. No host privilege or execution-policy change is authorized by this review.
3. Actual macOS component and C++/sanitizer runs remain unverified. Real case-insensitive APFS behavior and the permitted volume skip need host evidence. Native Windows junction handling has no junction-specific fixture. The matrix covers neither Apple Silicon nor Windows ARM; Windows sanitizer coverage is also absent by design.
4. Python 3.10 execution and lint with Pylint 2.16.2, Astroid 2.14.2 and YAPF 0.32.0 remain unverified. Local Python 3.14.8 and newer lint-tool success cannot close these gaps.
5. Source provenance and preparation remain open on every platform. Synthetic patch ordering and input-digest tests do not establish archive availability/integrity, pristine Chromium sources, full ordered common/platform patch applicability, substitutions/resources, final `args.gn`, GN output or a reproducible prepared source tree. The platform guides retain these limits in [Linux](../native-linux.md), [Windows](../native-windows.md) and [macOS](../native-macos.md).
6. Native browser acceptance remains open. There is no evidence here of the required Chromium toolchain working, a browser binary/build identity, installation, signing, launch or BrowserContext/window integration. Standalone model tests do not prove authentication isolation, renderer/history preservation, persistence, native UI/UX, accessibility or performance. These checks cannot close issues 16, 17, 18 or 24.

## Reviewed source identity

The working tree contained uncommitted work. Parent `HEAD` was `09670c97b1c308d0fb31869b3a9a9bc6969bc47f`; Git blob hashes below identify the actual reviewed contents. They were unchanged across the review's final verification checks.

| File | Git blob hash |
| --- | --- |
| `.github/workflows/arcified-native-checks.yml` | `42caf2ee8296609032e7d62daa571fabfc53d328` |
| `arcified/native/checks/python_checks.py` | `ffd581aaa8580dec28043675240f62956e6ba1b7` |
| `arcified/native/checks/windows_model.ps1` | `35fdd70d657fa14edf32e221e57452c63b74288b` |
| `arcified/docs/native-checks.md` | `9c6ccdc864c2cf97ec0f5cca616094e740e979dd` |
| `arcified/native/model/lifecycle.h` | `d3a7aed6f1caddd40f5422bc6e5c1f5019636338` |
| `arcified/native/model/lifecycle.cc` | `7ffe87eb49d0cf4a92f83058b7c348ed3882012f` |
| `arcified/native/model/lifecycle_test.cc` | `af3a52f39d323e079b3799b306e1e756c9205893` |
