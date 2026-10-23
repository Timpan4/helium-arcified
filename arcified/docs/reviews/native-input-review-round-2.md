# Native input review, round 2

Result: both prior shared-input findings are cleared in the reviewed snapshot. No new concrete correctness issue was found in scope. Default Pylint remains failing; the documented scoped checks pass. This review does not close native platform acceptance.

Reviewed on 2026-10-23 at fork HEAD `09670c97b1c308d0fb31869b3a9a9bc6969bc47f`, including the uncommitted files identified below. The four fix files were inspected in full: `input_identity.py`, `test_input_identity.py`, `devutils/check_patch_files.py`, and `devutils/tests/test_check_patch_files.py`. Linux/Windows callers and tests, the existing lint policy, the first Windows review and the fix response were also inspected. Linux/Windows adapter and test hashes match the first Windows review. No production code, prior report, GitHub state or PR was changed. This report is the only repository file written by this review.

`macos.py` and `test_macos.py` were excluded from reading, lint, formatting and execution because their work-path fix is assigned separately. The parent's final combined suite remains outstanding. The earlier 38-test, three-preflight and eight-native-file lint claims are historical fix-response evidence, not checks independently repeated here.

## Finding disposition

### Cleared: P1, Windows nested patch paths

`check_unused_patches()` now compares filesystem-relative entries, series entries and the series filename using `as_posix()`. This fixes native Windows backslash names disagreeing with the repository's slash-separated series. The separate readability and duplicate checks remain in place, and unused warnings remain sorted. Patch identity still preserves parsed series order rather than sorting it.

The component regression calls the actual checker. Its mocked filesystem enumeration supplies `PureWindowsPath` relative names for both a nested patch and a nested series filename, while the series is read from a real fixture. It first accepts the referenced patch, then checks the exact sorted warnings for unused nested patches. This is a targeted separator fixture, not a native Windows filesystem run. As an independent regression-strength check, executing the original checker from `git show HEAD:devutils/check_patch_files.py` in memory against this same new test produced `Unused patch: group\change.patch` and the expected assertion failure. No working file was reverted.

The shared tests also run the real checker subprocess through `patch_identity()` for nested ordered patches and unused nested patch rejection. Existing Linux negative fixtures still reject duplicate, missing and malformed patches. The Linux and Windows dependent-patch tests still use the real inherited patch utility on small synthetic files and prove fork-before-platform order with `--no-fuzz`.

### Cleared: P2, directory links omitted from identity

`_build_input_files()` checks existence and resolved containment before deciding whether to descend or hash. A directory symlink is rejected before traversal. Comparing its resolved and absolute path additionally rejects directory aliases without relying on Python's newer junction API. Escaping files/directories fail containment, contained file links hash target bytes under the input's logical name, and broken links fail instead of disappearing from the report. Cache exclusions and sorted output remain intact; a directory merely ending in `.pyc` is still traversed.

The shared regressions exercise `tree_identity()` with real Git fixtures and filesystem links, without replacing traversal, Git identity or patch validation. They cover internal directory links, links selected directly as an input, ancestor cycles, escaping directory/file links, changed effective file-link content, broken links and cache filtering. All symlink fixtures executed on this Linux host; none was skipped. Independent probes using the complete existing Windows fork fixture also confirmed that both `linux.preflight()` and `windows.preflight()` reject a contained directory link through the real shared helper. Only each adapter's fork location was substituted for that probe.

No claim is made that check-then-read traversal is an atomic snapshot under concurrent input mutation. The input manifest and preparation contracts require unchanged inputs. Native Windows junction behavior remains unexecuted.

## Independent checks

Commands ran with inherited `PYTHONEXECUTABLE` and `__PYVENV_LAUNCHER__` cleared, so Python subprocesses did not launch the T3 AppImage. Python was 3.14.8. Existing `/tmp/arcified-native-checks.vKx5Ht/bin/python` supplied Pylint 4.1.3, Astroid 4.3.4 and YAPF. No packages were installed.

1. `PYTHONPATH=arcified/native/tooling python3 -B -m unittest test_input_identity test_linux test_windows -v` passed 27 tests: eight shared, eight Linux and 11 Windows. Expected negative-fixture warnings/errors occurred. No tests were skipped.
2. `python3 -B devutils/tests/test_check_patch_files.py` passed both component tests. The original-checker regression-strength probe and both adapter directory-link probes described above passed their expected-failure assertions.
3. Real default Linux and Windows CLI preflights exited zero. Both identified Chromium `155.0.8059.79` and 346 common patches. Both left native build/launch `OPEN`; Windows left source preparation `OPEN` and toolchain `UNVERIFIED`. These commands read the current fork, without platform checkouts, downloads or preparation.
4. YAPF `--style .style.yapf --diff` produced no changes for the six shared/Linux/Windows Python files and both changed devutils files. `git diff --check` passed for tracked changes. YAPF supplies the formatting evidence for the untracked reviewed Python files.
5. Combined default Pylint on those six native files, with `--persistent=n`, exited 10. It reported `E0401` for `_common` in `input_identity.py` and `R0801` for duplicated platform recipe/CLI/report fragments and a test patch fixture. These diagnostics remain present under that invocation.
6. On the same six files, `PYTHONPATH=devutils:utils:arcified/native/tooling` and `--persistent=n --disable=duplicate-code` passed. On the two changed devutils files, `PYTHONPATH=devutils:utils` and `--persistent=n --disable=wrong-import-position,duplicate-code` passed. No remaining diagnostics appeared in either scoped check.

## Pylint scope assessment

The fix response accurately distinguishes a failing default invocation from passing scoped lint. `devutils/run_devutils_pylint.py` already disables `wrong-import-position` and `duplicate-code` and supplies import paths for the inherited utilities. The component exclusions therefore match existing devutils policy. Supplying the real utility paths resolves static import discovery; it does not disable `import-error`, and the passing production checker subprocesses independently exercise runtime imports. Keeping `devutils` before `utils` selects the intended `third_party.unidiff` package.

Using the same duplicate-code exclusion for the new native tooling is an explicitly documented scoped convention, not proof that an existing repository-wide native lint target covers it. The native files retain their existing local disables for deliberate imports/test conventions. The disabled duplication is still present and inspected. It revealed no additional concrete defect in these fixes. The report must continue to say default Pylint fails; neither scoped pass establishes repository-wide lint success. The active macOS code was not included in this assessment.

## Source snapshot

SHA-256 values for the reviewed working files, in addition to the HEAD above:

```text
18a236cd6a51f02ea4c71b9fa1464035faebd2c31d2e5c8fd0aae81ee9215746  arcified/native/tooling/input_identity.py
50583b187d17ab72943f354385e35f02826f8c5412f45d4e3eb26fd73fd20039  arcified/native/tooling/test_input_identity.py
c3e5c524ef4d76c8e0c3f636aff02205a1a1422931e05e24e7655bdc34497a6b  devutils/check_patch_files.py
954f412a9eac99b79cb2f2f2891113f3f3c0304d4fc2cf160996c6d1114e5a40  devutils/tests/test_check_patch_files.py
28d6c752689d74218f2de7c4a66d22a23ead7ee2cf2d2b399936336cfff299ae  arcified/native/tooling/linux.py
90be5305c743fc4c70ef86e82318fdee1326fcfc00b5ef39eab8692ae09c317b  arcified/native/tooling/test_linux.py
856e68daf7c682eaca0ae3cb80f52089edb7ab7720da796482bbaaf14ea0dbc3  arcified/native/tooling/windows.py
1abae9de41824b528d5a5d76a36399c6471ef848210ed2112eeaa17915d186d4  arcified/native/tooling/test_windows.py
5a648d10a83af45ea25ef2cf1452967104b200bf89cb045e0da9fdcf0988810b  arcified/docs/reviews/native-input-fix-response.md
088540cee36e39df3dac0707ce73d5dc5600e13576334411a72af9650a486f29  devutils/run_devutils_pylint.py
```

## Remaining native limits

Python 3.10 execution, native Windows path/junction execution and macOS host execution remain unverified. The APIs used by this fix exist in the repository's Python 3.10 baseline, but that is source inspection rather than a baseline runtime test. This Linux review proves the synthetic contracts and default local preflights above. It does not prove actual pinned platform preparation or patch applicability to Chromium.

No full builds, source downloads, real source preparation, GN generation, global installs, remote CI, signing, packaging, native browser launch or interaction checks ran. Those operations remain deferred. Toolchains, real ordered patch/preparation results, binary identity and native smoke/accessibility evidence remain open. The separate macOS work-path finding and the parent's post-stabilization combined verification are outside this result.
