<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Checks performed

All commands below ran locally from the shared release checkout. No native preparation, build, native test execution, installation, signing, CI dispatch or remote write ran.

1. SHA-256 comparison: all three prepared-base copies match their read-only prepared-source files. The unchanged GN evidence copy also matches its source. Exact identities are in manifest.json.
2. Focused Chromium formatting: installed clang-format version 23.1.3 returned exit 0 for these checks after applying its output through the editing tool:

~~~sh
clang-format --style=Chromium --dry-run --Werror --lines=123:139 arcified/native/production/release-windows/prepared-result/chrome/installer/util/helper.cc
clang-format --style=Chromium --dry-run --Werror --lines=20:35 arcified/native/production/release-windows/prepared-result/chrome/installer/util/helper.h
clang-format --style=Chromium --dry-run --Werror --lines=119:167 --lines=489:514 arcified/native/production/release-windows/prepared-result/chrome/installer/util/helper_unittest.cc
~~~

3. Forward and reverse sparse patch checks returned exit 0. They inspect patch context/applicability without modifying either snapshot:

~~~sh
git apply --check --directory=arcified/native/production/release-windows/prepared-base arcified/native/production/release-windows/release-windows.patch
git apply --reverse --check --directory=arcified/native/production/release-windows/prepared-result arcified/native/production/release-windows/release-windows.patch
~~~

4. Focused patch whitespace lint returned exit 0:

~~~sh
git apply --check --whitespace=error-all --directory=arcified/native/production/release-windows/prepared-base arcified/native/production/release-windows/release-windows.patch
~~~

5. git diff --check returned exit 0. It does not inspect untracked files, so the explicit patch whitespace check above is the relevant source-diff check.
6. git apply --numstat returned exit 0 and reported the intended files:

~~~text
14  1  chrome/installer/util/helper.cc
9   3  chrome/installer/util/helper.h
72  0  chrome/installer/util/helper_unittest.cc
~~~

The native Windows test source is registered in the existing target, not executed. Compilation, behavioral tests, package trust/signature verification and installed UX remain OPEN. No full-file formatter or Windows compiler lint claim is made.
