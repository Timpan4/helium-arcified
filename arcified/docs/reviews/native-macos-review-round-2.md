# Native macOS review, round 2

Result: the prior macOS P2 work-path isolation finding is cleared in this snapshot. No new concrete correctness finding was found in the reviewed scope. Native acceptance for issue #18 remains `OPEN`.

Reviewed independently on 2026-10-23 at fork HEAD `09670c97b1c308d0fb31869b3a9a9bc6969bc47f`, including the uncommitted files identified below. Read `macos.py`, `test_macos.py` and `native-macos.md` in full, the original issue's local publication specification, the prior macOS review and isolation response, and the current shared helper and its round-2 review. Only this report was written in the repository. Production code, prior reports, GitHub and PR state were unchanged.

## Finding disposition

### Cleared: P2, alternate-case work paths inside an input repository

`prepare()` retains the lexical equality and containment checks, then checks each candidate work ancestor against both input roots with `Path.samefile()`. The walk includes the work path itself and every parent. `FileNotFoundError` skips a missing component without ending the walk, so paths below several missing parents still reach the existing input root. Filesystem identity catches alternate-case aliases even when `resolve()` preserves their spelling. Other filesystem errors propagate to the CLI's error handling rather than producing a successful plan. The implementation never lowercases paths, preserving distinct case-sensitive siblings.

The existing-directory rejection also remains in place. An existing ancestor of an input is rejected by lexical containment or the existing-work check; a nonexistent directory cannot already contain an existing input. CLI paths still pass through `resolve()`, preserving symbolic-link normalization. These are planning checks, not an atomic filesystem reservation. A future executor must revalidate identities and create new directories safely under the documented unchanged-input contract.

The regression calls the real `resolve()` and `prepare()` with real synthetic repositories and real preflight checks. Only filesystem lookup is emulated: `os.stat` and `os.lstat` redirect alternate-case input components to their real identities while preserving the resolved spelling. It covers both fork and platform inputs, with work directly below each alias, below an existing child, and below several missing parents. Separate work still returns a plan without creating parents. The distinct-case sibling test exercised actual separate directories on this Linux volume and was not skipped.

As an independent regression-strength check, temporarily replacing only `_has_input_ancestor()` with a false result in memory restored the old lexical-only decision. Running the alias regression then produced exactly six assertion failures, each `ValueError not raised`, with no errors. No working file was reverted or edited for this probe. The unmodified guard passes the same test in both successful suite runs below.

## Regression assessment

The public helper signatures and CLI normalization remain intact. Architecture validation still precedes planning. Work isolation, spaces and existing-directory rejection all precede preflight and plan construction. The adapter creates no work directory and has no execution option.

Source/patch checks, independent host/target selection, manifest and preparation order, fork-before-platform patch application, work-local resource generation and backups, flag recipes and digests, secret omission and acceptance states passed the focused contracts. Synthetic ordered patches actually apply through the inherited patch utility on Linux. They do not establish full Chromium patch applicability or native macOS patch behavior.

`native-macos.md` accurately explains existing-ancestor identity checks, case-sensitive sibling acceptance, Linux lookup emulation and the lack of native APFS evidence. It continues to distinguish a declarative plan from executed preparation, GN generation, compilation, signing and launch. No change to the source helper API or preparation order was needed for this fix.

The shared P1/P2 disposition is retained from [native-input-review-round-2.md](native-input-review-round-2.md). Its reviewed `input_identity.py` digest matches this review's snapshot. The shared/Mac integration run below adds current Mac caller evidence. This review does not repeat the prior Linux/Windows/component checks or claim a new all-platform combined result. The shared report's failing default combined Pylint result remains separate from this passing Mac-only scoped invocation.

## Independent checks

All Python commands cleared inherited `PYTHONEXECUTABLE` and `__PYVENV_LAUNCHER__`. Existing Python tooling was used without installation. No cleanup command ran; synthetic fixtures remain for inspection.

1. `env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ python3 -B -m unittest discover -s arcified/native/tooling -p test_macos.py -v` passed all 14 tests with no skips. Negative patch fixtures emitted their expected diagnostics.
2. `env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ PYTHONPATH=arcified/native/tooling python3 -B -m unittest test_input_identity test_macos -v` passed all 22 tests with no skips. This exercises the current shared input validation together with the Mac adapter, including directory links, effective file-link content, nested patches and the six work-alias subcases.
3. The in-memory lexical-only probe described above ran `test_case_insensitive_aliases_into_either_input_reject_new_work` with `mock.patch.object(macos, "_has_input_ancestor", return_value=False)`. It asserted six expected failures, zero errors and `ValueError not raised` in every failure. The probe command exited zero after checking those results.
4. The actual CLI default `preflight` and both cross-architecture `prepare` invocations exited zero through subprocesses. The commands were:

   ```sh
   python3 -B arcified/native/tooling/macos.py preflight
   python3 -B arcified/native/tooling/macos.py prepare --platform-tree /tmp/arcified-macos-platform.yujJQ0 --work-dir /tmp/arcified-macos-review-round-2-lemvo1oi/arm64-x64 --host-arch arm64 --target-arch x64
   python3 -B arcified/native/tooling/macos.py prepare --platform-tree /tmp/arcified-macos-platform.yujJQ0 --work-dir /tmp/arcified-macos-review-round-2-lemvo1oi/x64-arm64 --host-arch x64 --target-arch arm64
   ```

   All three validated 346 common patch entries. Each plan validated 23 platform entries at `9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28`, reported `PLANNED`, kept all 20 stages `NOT_RUN`, all native acceptance fields `OPEN` and the toolchain `UNVERIFIED`. Both work directories remained absent, their new fixture parent stayed empty, and platform `git status --porcelain` was empty before and after. These are Linux CLI planning results using the existing pinned checkout, with no source preparation or downloads.
5. `env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ /tmp/arcified-native-checks.vKx5Ht/bin/python -B -m yapf --style .style.yapf --diff arcified/native/tooling/macos.py arcified/native/tooling/test_macos.py` exited zero with no changes.
6. `env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ PYTHONPATH=devutils:utils:arcified/native/tooling /tmp/arcified-native-checks.vKx5Ht/bin/python -B -m pylint --persistent=n arcified/native/tooling/macos.py arcified/native/tooling/test_macos.py` exited zero with no diagnostics. No duplicate-code exclusion was needed for this two-file invocation. This is not a repository-wide lint result.
7. `git diff --check` passed for tracked changes. The untracked reviewed Python files have the YAPF evidence above.

## Reviewed snapshot

SHA-256 values:

```text
0119df0a6060fc99b59ae7ba8bf4e247ca5e417603865c7c269b55b48612b130  arcified/native/tooling/macos.py
9dcde3c892f2fe616ec3547687b3af6caaa7dc817fcf5c333b31f3bb8ccff8c2  arcified/native/tooling/test_macos.py
f090a047fb0db7daf9834d76747aef203b5ca70db53c1e2a4f026877511fdc49  arcified/docs/native-macos.md
18a236cd6a51f02ea4c71b9fa1464035faebd2c31d2e5c8fd0aae81ee9215746  arcified/native/tooling/input_identity.py
```

## Remaining native limits

Case-insensitive behavior is emulated on Linux. Real macOS/APFS lookup, native host architecture, Windows junction behavior, Python 3.10 execution, selected Xcode/SDK and Metal remain unverified. Pin/source identities and synthetic contracts do not prove archive availability or integrity, real source preparation, complete Chromium patch applicability, GN generation or compilation.

No heavy preparation, source/archive downloads, SDK changes, full builds, global installs, signing, notarization, packaging, native browser launch, visual/accessibility checks or remote CI ran. Those remain deferred. Closing P2 clears this planning defect only. Issue #18 still needs separately authorized native preparation, toolchain and build evidence, followed by navigation, extension, completed download, native menus, profile-restart persistence, failure, keyboard/focus, accessibility and applicable keychain evidence from the exact resulting binary. Unsigned development artifacts must remain distinct from distributable releases.
