# Native macOS isolation fix response

Implemented the accepted P2 from [native-macos-review.md](native-macos-review.md) on 2026-10-23. Parent owns integration and fresh independent review. Issue #18 native acceptance remains `OPEN`.

## Change

`macos.py` retains its lexical overlap checks and adds filesystem identity checks with `Path.samefile()` over the work path and its existing ancestors. Missing components are skipped, so a new path with missing parents still reaches its nearest existing ancestor and the input root. Other filesystem errors propagate. Paths are never lowercased. Separate directories with different case remain valid on case-sensitive volumes.

`test_macos.py` adds regressions through real `resolve()` and `prepare()` calls, without mocking preflight or the guard. Emulated `os.stat` and `os.lstat` map alternate-case fork/platform components to their real filesystem identities while preserving resolved spelling. For each input, tests reject new work directly below the alias, below an existing child, and below multiple missing parents. Separate work remains `PLANNED`, all stages remain `NOT_RUN`, all native acceptance fields remain `OPEN`, and no work parents are created. Real distinct case-sensitive siblings also remain valid. Fixtures now remain under `/tmp/arcified-macos-*` for inspection.

`native-macos.md` clarifies the identity check and records the updated coverage. Public signatures, tooling pins, architecture selection, source/flag/resource recipes and the shared helper API are unchanged. No independent report, shared helper, Linux/Windows code or ledger was edited.

## Verification

Ran on Linux with existing Python tooling and no package installation:

1. Before changing the guard, `env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ python3 -B -m unittest discover -s arcified/native/tooling -p test_macos.py -k case_insensitive_aliases -v` failed all six alias subcases with `ValueError not raised`. The fixture remains at `/tmp/arcified-macos-r7wo8erj`.
2. After the fix, `env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ python3 -B -m unittest discover -s arcified/native/tooling -p test_macos.py -v` passed all 14 tests with no skips. Negative patch fixtures emitted expected diagnostics. This also exercises the current shared helper through real synthetic preflight checks.
3. With launcher overrides cleared, `/tmp/arcified-native-checks.vKx5Ht/bin/python -B -m yapf --style .style.yapf --diff arcified/native/tooling/macos.py arcified/native/tooling/test_macos.py` exited zero with no changes.
4. With launcher overrides cleared and `PYTHONPATH=devutils:utils:arcified/native/tooling`, the same environment's `python -B -m pylint --persistent=n arcified/native/tooling/macos.py arcified/native/tooling/test_macos.py` exited zero with no diagnostics.

Case-insensitive coverage is filesystem-semantics emulation on Linux, not a native macOS/APFS run. Case-sensitive sibling acceptance ran on the real Linux fixture volume. No native source preparation, downloads, SDK/toolchain checks, GN generation, builds, signing, launch, global package changes or remote writes ran. The adapter remains plan-only and creates no work directory.
