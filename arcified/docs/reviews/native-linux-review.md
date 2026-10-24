# Native Linux preparation review

Result: clean review of the preparation foundation. No blocking correctness, side-effect, input-selection, patch-order, failure-preservation, or provenance defect found in the reviewed snapshot. This does not accept the native build or launch requirements of [issue #16](https://github.com/Timpan4/helium-arcified/issues/16).

Reviewed on 2026-10-23 against fork HEAD `09670c97b1c308d0fb31869b3a9a9bc6969bc47f`, including the uncommitted files below. Review scope was `arcified/native/tooling/linux.py`, `arcified/native/tooling/test_linux.py`, `arcified/docs/native-linux.md`, and the added Linux-guide paragraph in `arcified/docs/native-integration.md`. Lifecycle and performance implementations were not inspected.

## Findings

No actionable findings. The following evidence supports accepting this as a partial foundation:

- `linux.py:94` records working-input digests and ordered patch digests as well as Git identity. It labels source VERSION/DEPS checks as markers, without claiming pristine provenance or applicability. The guide distinguishes a pinned platform commit from a reproducible host environment.
- `linux.py:125` requires platform revision `b154afccb44729359c816754c7741efb3cb8f497`, verifies its Helium gitlink, and rejects tracked platform changes. GitHub's pinned gitlink metadata independently identifies `b5145ab1047297970aceb9ca9f48722f6926cc66`; that Helium commit's Chromium version is `155.0.8059.79`.
- `linux.py:148` selects this fork's utilities, common configuration, resources, assets manifest, and common patches explicitly. The platform's `helium-chromium` submodule is neither initialized nor used as the common-input source. `utils/patches.py` iterates the supplied groups in order and preserves each series order. The plan uses the fork group before the Linux group, with `--no-fuzz`.
- The planned sequence matches the pinned production preparation order through GN. Resources are generated in a work-directory copy. GN flags omit the platform script's optional host cache, remote execution, and PGO detection, as the guide explains. The plan has no compilation, packaging, installation, or browser-launch command.
- `linux.py:197` refuses existing work directories and overlap with either repository. Planning returns before work-directory creation or downloads. Execution requires `--execute` and Linux x86_64. Failed subprocesses stop later stages and retain logs, inputs, cache, and a failure manifest. Existing upstream stamps do not bypass stages.
- The inherited download utility verifies archive hashes during `retrieve`, before the planned `unpack` stage. The guide correctly requires retaining checksum sidecars and separately recording environment identity. No claim of real archive verification or patch application follows from the dry-run.

Authoritative platform sources inspected read-only: [production stages](https://github.com/imputnet/helium-linux/blob/b154afccb44729359c816754c7741efb3cb8f497/scripts/build.sh), [shared preparation functions](https://github.com/imputnet/helium-linux/blob/b154afccb44729359c816754c7741efb3cb8f497/scripts/shared.sh), [archive definition](https://github.com/imputnet/helium-linux/blob/b154afccb44729359c816754c7741efb3cb8f497/downloads.ini), [Linux flags](https://github.com/imputnet/helium-linux/blob/b154afccb44729359c816754c7741efb3cb8f497/flags.linux.gn), [patch series](https://github.com/imputnet/helium-linux/blob/b154afccb44729359c816754c7741efb3cb8f497/patches/series), and [build environment](https://github.com/imputnet/helium-linux/blob/b154afccb44729359c816754c7741efb3cb8f497/docker/build.Dockerfile). The [pinned Helium version](https://github.com/imputnet/helium/blob/b5145ab1047297970aceb9ca9f48722f6926cc66/chromium_version.txt) was checked separately.

## Independent verification

1. `python3 -B -m unittest discover -s arcified/native/tooling -p 'test_linux.py' -v` passed all eight tests on Python 3.14.8. These exercise real inherited patch checking and a synthetic dependent common/platform patch pair, content identity, malformed/missing/duplicate patches, source-marker rejection, a dry-run with mocked preflight, GN argument output, work-directory rejection, and subprocess failure preservation. They do not exercise a real successful preparation. The failure test covers a nonzero subprocess exit, not every interruption or filesystem failure.
2. `python3 -B arcified/native/tooling/linux.py preflight` passed and reported 346 common patches, Chromium `155.0.8059.79`, and native build/launch `OPEN`.
3. The plan command passed using the existing clean checkout `/tmp/arcified-linux-platform.sVQiH3` at the required revision. It reported seven Linux patches and `PLANNED`. The selected work path `/tmp/arcified-linux-review-not-created` remained absent. Its commands were inspected, not executed. Review JSON outputs were written under `/tmp/arcified-linux-review-*.json`.
4. Additional in-memory mock checks confirmed that preflight rejects a different platform HEAD and tracked platform changes. The platform checkout was not modified for these checks.
5. Whitespace checks produced no diagnostics for the reviewed changes. YAPF and Pylint were unavailable in the current interpreter (`No module named yapf` and `No module named pylint`). The implementing worker's formatter/linter results were therefore not independently reproduced. No packages were installed.

Snapshot SHA-256 values:

```text
a39ba7d753f843ae072ad427f2405803f180fa161a592f99b4eb5908924ad7dc  arcified/native/tooling/linux.py
feb677e5f7fdbed2ad1e44667355740fdace917ea1574e6fec4b06986fd37bfb  arcified/native/tooling/test_linux.py
0629705cf5f4f0cddc969a67c3f8250915178c977648b923420c8c8b6df612f8  arcified/docs/native-linux.md
7d4074f5d9939e2e230835e06b91939ce895c69e93f2ad6f58cb22df3875750e  arcified/docs/native-integration.md
```

## Remaining native acceptance

Source preparation, application of the complete series to real Chromium, GN generation, browser compilation, and native launch remain **OPEN**. No full Chromium download, heavy preparation, build, launch, or remote CI run occurred in this review. Matching source versions and synthetic patch tests do not establish compatibility of the complete patch stack.

Issue #16 still needs an authorized build host, retained source/patch/build identity and logs, a resulting binary and launch artifact, and native navigation, extension, download, profile-restart, failure, and accessibility evidence. The guide leaves this work open correctly. This review requires no prohibited build on the current machine.
