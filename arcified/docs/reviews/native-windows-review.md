# Native Windows preparation review

Result: changes requested. The Windows plan has a deterministic native Windows preflight failure, and the shared identity helper omits directory symlinks from its provenance checks. No implementation files were changed during this review.

Reviewed on 2026-10-23 against fork HEAD `09670c97b1c308d0fb31869b3a9a9bc6969bc47f`, including the uncommitted snapshot identified below. Scope covered `windows.py`, `test_windows.py`, `input_identity.py`, updated `linux.py` and `test_linux.py`, `arcified/docs/native-windows.md`, the original Windows issue specification, and the previous Linux review. Relevant inherited utilities and pinned upstream inputs were inspected. This is the first Windows review.

## Findings

### 1. [P1] Normalize patch paths before using the inherited checker on Windows

Location: `arcified/native/tooling/input_identity.py:23`, calling `devutils/check_patch_files.py`; underlying mismatch at `devutils/check_patch_files.py:89`.

`patch_identity()` unconditionally runs the inherited patch checker with `check=True`. That checker's unused-patch detection stores `str(path.relative_to(patches_dir))`, then subtracts the unmodified strings from `patches/series`. Native Windows produces backslashes for the former, while the actual common series contains forward-slash paths such as `ungoogled-chromium/disable-google-host-detection.patch`. These strings do not match. Valid nested patches therefore produce unused-patch warnings, the checker exits nonzero, and both Windows `preflight` and `prepare` fail before returning their report.

The mismatch was reproduced by calling the actual `check_unused_patches()` function with filesystem entries whose relative paths were `PureWindowsPath('group/change.patch')` and `PureWindowsPath('series')`, and a parsed series containing `group/change.patch`. It returned `True` and emitted `WARNING: Unused patch: group\change.patch`. This is a path-semantics reproduction on Linux, not a native Windows test. The ordinary fixtures use a top-level `change.patch`, which cannot expose the separator mismatch.

Normalize filesystem-relative paths and series entries into a common representation while preserving duplicate, missing, malformed, and unused-patch validation. Add a nested-path regression case that exercises Windows path semantics. This inherited checker behavior was harmless to the Linux-only route but blocks its new Windows caller.

### 2. [P2] Reject or identify directory symlinks in shared build inputs

Location: `arcified/native/tooling/input_identity.py:51` and `:55`.

`tree_identity()` uses `rglob('*')` without following directory symlinks and applies its resolved-path containment check only to entries for which `is_file()` is true. A nested directory symlink is therefore neither rejected nor hashed. This permits an input directory to reference files outside the declared repository without recording their content or detecting their subsequent changes. The fork accepts working changes, so tracked-change status does not repair this omission.

A temporary fixture with `fork/resources/linked` pointing to an external directory was accepted by the real `tree_identity(root, ['resources'])`. Its `files_sha256` was `{}`. Changing the external `asset.txt` left the entire returned identity unchanged. Git metadata and patch validation were mocked for this focused traversal reproduction. No repository inputs were altered. Resource-copy or resource-generation code can subsequently consume files through that symlink, so the report does not identify all effective preparation inputs.

Validate directory links before skipping non-files. Reject escaping links, and either reject directory links altogether or explicitly identify their effective contents with cycle handling. Apply the same rule to both platform callers. This finding concerns the shared helper now under review; it does not establish that the extraction newly introduced the behavior into Linux.

## Other reviewed behavior

- The chosen immutable Windows revision is compatible by declared engine version. The [pinned Git tree](https://api.github.com/repos/imputnet/helium-windows/git/trees/555c9467373779ea030d6b555d7bd05b48d96d8c) identifies gitlink `b5145ab1047297970aceb9ca9f48722f6926cc66`, whose [Chromium version](https://github.com/imputnet/helium/blob/b5145ab1047297970aceb9ca9f48722f6926cc66/chromium_version.txt) is `155.0.8059.79`. The [next inspected revision](https://api.github.com/repos/imputnet/helium-windows/git/trees/11a292ea63c107e8ed7f5edb5883df10f977b845) selects `832faa7822566270e10c0d3262f702f9d0fea81f`, targeting `155.0.8059.84`. Exact version agreement does not prove downstream patch applicability. This review did not independently traverse the complete platform history to prove the guide's newest-compatible claim.
- The plan explicitly uses the downstream fork for common inputs. Manifest ordering, fork-before-platform patch groups, the selected PortableGit patch executable, and Windows-before-common resource replacement agree with the [pinned build script](https://github.com/imputnet/helium-windows/blob/555c9467373779ea030d6b555d7bd05b48d96d8c/build.py). `--no-fuzz` is passed to the fork's actual CLI. Both synthetic dependent patch-pair tests passed, using Linux GNU patch.
- Windows resource generation operates on planned work-directory copies. Flags retain platform PGO requirements and append explicit updater and remote-execution settings. The [pinned Siso declaration](https://github.com/chromium/chromium/blob/155.0.8059.79/build/toolchain/siso.gni) permits an explicit `use_siso` argument without `.sisoenv`; this supports the narrow GN argument claim, without proving GN generation or a working compilation backend.
- No Windows execution option exists. Tests confirmed that planning creates no work directory and that `--execute` is rejected. Existing and overlapping work paths are rejected. The execution contract is declarative and has no executor enforcing it yet. No compilation, packaging, signing, installation, or browser launch is included.
- The guide accurately leaves actual archive extraction, strict pruning applicability, SDK/toolchain detection, complete patch application, GN, and Siso compilation setup unresolved. Its stated Visual Studio and SDK versions agree with the [pinned Chromium Windows instructions](https://github.com/chromium/chromium/blob/155.0.8059.79/docs/windows_build_instructions.md). Source-marker checks are explicitly limited to markers. No native acceptance is inferred from a plan or Linux-host tests.

## Independent verification

1. `env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ python3 -B -m unittest discover -s arcified/native/tooling -p 'test_*.py' -v` passed all 19 tests, including the original eight Linux contracts. The first invocation without the environment adjustment failed because Python's `sys.executable` identified the T3 AppImage and child commands launched that executable. Clearing inherited launcher overrides resolved the test-environment failure without a source change. Expected negative-fixture diagnostics occurred in the passing run.
2. Both real default preflights passed with the same environment adjustment. Each reported Chromium `155.0.8059.79` and 346 common patches. Windows reported `toolchain.status=UNVERIFIED` and source preparation, native build, and native launch `OPEN`. Linux reported native build and launch `OPEN`. No real pinned Windows checkout plan was independently generated; its plan contracts were exercised through the existing fixtures and inspected against pinned source.
3. The two additional targeted reproductions above demonstrated the separator mismatch and the directory-symlink identity omission. Neither is covered by the passing focused suite. No native Windows execution occurred.
4. YAPF `--diff` on all five reviewed Python files produced no changes, using the existing `/tmp/arcified-native-checks.vKx5Ht/bin/python` environment. No packages were installed. A combined default Pylint invocation from that environment returned exit 10, reporting `_common` import resolution (`E0401`) and cross-file duplicate-code diagnostics (`R0801`). Thus the worker's lint-pass claim was not independently reproduced with that invocation; these diagnostics are separate from the correctness findings above.
5. `git diff --check` passed for tracked changes. Since the reviewed tooling and Windows guide are untracked, that command does not establish their whitespace status. No full Chromium download, heavy preparation, native compilation, launch, or remote CI ran.

Reviewed snapshot SHA-256 values:

```text
856e68daf7c682eaca0ae3cb80f52089edb7ab7720da796482bbaaf14ea0dbc3  arcified/native/tooling/windows.py
1abae9de41824b528d5a5d76a36399c6471ef848210ed2112eeaa17915d186d4  arcified/native/tooling/test_windows.py
b22ba2e6599a94bd981ea838cccdffb4a7a9ffdc8f77aec01a655a203ce88ed5  arcified/native/tooling/input_identity.py
28d6c752689d74218f2de7c4a66d22a23ead7ee2cf2d2b399936336cfff299ae  arcified/native/tooling/linux.py
90be5305c743fc4c70ef86e82318fdee1326fcfc00b5ef39eab8692ae09c317b  arcified/native/tooling/test_linux.py
bfd5bcd0c1aec7f0eb1ce13db926874eef2b99fb7320e9e34dba7ef2e6ad242c  arcified/docs/native-windows.md
```

## Remaining native acceptance

Issue #17 remains open. After these foundation findings are resolved, separately authorized Windows work must still record the real host/toolchain, verified archives and source identity, complete ordered patch results, successful preparation and GN logs, and final flags. Full builds remain explicitly deferred. Compilation requires its own authorization, command and logs, exit status, and binary identity.

The resulting binary must subsequently be launched on Windows with an isolated profile and exercised for navigation, a completed download, extension loading, window controls, and profile restart with persisted state. Retain visual, failure, keyboard/focus, and accessibility evidence. No binary or native smoke evidence exists from this review. The previous Linux review's clean result does not settle these shared-helper findings or validate Windows behavior.
