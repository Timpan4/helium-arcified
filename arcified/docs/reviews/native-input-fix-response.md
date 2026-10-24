# Native input review fix response

Implemented the accepted P1 and P2 findings from `native-windows-review.md` on 2026-10-23. Shared helper API signatures and patch series order are unchanged. Parent owns integration and fresh independent review.

## Changed files and finding evidence

- `devutils/check_patch_files.py`: compare filesystem-relative patch names, parsed series names and the series file itself using slash paths. Readability, duplicate, missing and malformed checks remain intact. Unused-patch warnings retain sorted order.
- `devutils/tests/test_check_patch_files.py`: add native Windows path semantics using `PureWindowsPath` for nested filesystem entries and a nested series file, against the actual checker and slash-separated series contents. Before the fix, it failed with `Unused patch: group\change.patch`. It now accepts the referenced patch and still reports unused nested patches in sorted order.
- `arcified/native/tooling/input_identity.py`: validate every directory before descending. Escaping inputs fail containment validation. Internal directory links, including cycles, fail with `Directory links are not supported as build inputs`. Resolved-path comparison also rejects directory aliases such as junctions without requiring APIs newer than Python 3.10. Contained file links hash their effective contents; broken links fail clearly. Cache exclusions and sorted file output are preserved.
- `arcified/native/tooling/test_input_identity.py`: eight shared regressions cover nested series order, unused nested patches, content identity, internal/top-level/cyclic directory links, escaping directory/file links, effective file-link content, broken links and cache filtering. Before the fix, directory-link and broken-link cases were accepted. All now pass. New fixture directories remain under `/tmp/arcified-input-identity-*` and `/tmp/arcified-windows-patch-paths-*` for inspection.
- This response file records the implementation evidence. No platform implementation, platform test, native guide, independent review report, execution ledger or GitHub state was changed.

## Verification

Ran on Linux with Python 3.14.8, using the existing `/tmp/arcified-native-checks.vKx5Ht/bin/python` environment for YAPF and Pylint. No packages were installed. Commands cleared inherited `PYTHONEXECUTABLE` and `__PYVENV_LAUNCHER__` so subprocesses used Python rather than the T3 AppImage. Logs and default preflight reports remain in `/tmp/arcified-input-fix.3eiXRy/`.

1. `python3 -B -m unittest discover -s arcified/native/tooling -p 'test_*.py' -v`: 38 tests passed, comprising eight shared, eight Linux, 11 Windows and 11 macOS tests. Negative fixtures emitted expected patch errors. This covers synthetic dependent patch application, plan-only behavior and shared identity, without real preparation.
2. `python3 -B devutils/tests/test_check_patch_files.py`: both component tests passed, including Windows nested-path semantics and sorted unused warnings.
3. Each real default CLI, `python3 -B arcified/native/tooling/{linux,windows,macos}.py preflight`, exited zero and identified Chromium `155.0.8059.79` with 346 common patches. Native build and launch stayed `OPEN`; Windows/macOS toolchains stayed `UNVERIFIED` and source preparation stayed `OPEN`.
4. YAPF `--style .style.yapf --diff` passed on all eight native Python files and both affected devutils files. `git diff --check` passed for tracked changes; untracked native Python files were covered by YAPF.
5. Combined default Pylint reproduced exit 10, `E0401` for `_common` and `R0801` for duplication across platform files. This default invocation is not clean. With `PYTHONPATH=devutils:utils:arcified/native/tooling`, combined Pylint over all eight native files passed with `--persistent=n --disable=duplicate-code`. The disable matches existing `devutils/run_devutils_pylint.py` policy; it does not resolve platform duplication. No error diagnostic was disabled.
6. Pylint over both affected devutils files passed with `PYTHONPATH=devutils:utils` and `--persistent=n --disable=wrong-import-position,duplicate-code`, matching the same existing policy. Path order matters because both utility trees contain a `third_party` package. Pylint was 4.1.3 with Astroid 4.3.4. Passing scoped lint does not imply a passing default invocation or repository-wide lint.

## Remaining native gaps

Python 3.10 runtime execution was unavailable; changes use syntax and pathlib APIs supported by the repository's `.python-version` baseline. Windows separator coverage uses pure path semantics on Linux. Native Windows junction handling and Windows/macOS host execution remain unverified.

No native builds, downloads, real source preparation, GN generation, installation, remote CI, signing or browser smoke tests ran. Issue #17 native acceptance remains open. Real ordered patch applicability, toolchains, preparation outputs, binary identity and native interaction evidence still require the separately authorized platform work.
