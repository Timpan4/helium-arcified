# Linux native run, 2026-10-23

Source preparation passed through GN. Baseline browser compilation is running
in root thread `6733348c-3696-4b31-834c-2bbca45e70d2`. No browser binary, launch
or UX acceptance is claimed yet. The owner authorized heavy CPU work only in
this root thread at 21:33 UTC.

## Inputs and preparation

- Fork commit: `05ace22efb8564d0294b8efa1b3f3651b76cef07`.
- Chromium: `155.0.8059.79`.
- Linux tooling: `b154afccb44729359c816754c7741efb3cb8f497`.
- Tooling checkout: `/home/timpan4/.t3/builds/helium-linux-platform-155`.
- Work directory: `/home/timpan4/.t3/builds/arcified-linux-155-native-v1`.
- Source directory: the work directory's `src`.

The repository preparation command completed all 17 stages, including archive
verification, version identity, pruning, strict common and Linux patch
application, substitutions, translations, resources, versioning and GN. GN
reported 35,690 targets from 5,018 files. These counts describe generation,
not successful compilation or test coverage.

```sh
env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ python3 -B arcified/native/tooling/linux.py prepare \
  --platform-tree /home/timpan4/.t3/builds/helium-linux-platform-155 \
  --work-dir /home/timpan4/.t3/builds/arcified-linux-155-native-v1 --execute
```

The work directory retains `preparation.json`, all stage logs, exact downloaded
archives and checksum sidecars. The fork had tracked documentation/component
changes; the manifest records preparation input hashes. No patch, build flag,
download definition or preparation-tool input changed during the run.

| Artifact | SHA-256 |
| --- | --- |
| `preparation.json` | `65273bd34c421ebda14437d1f6c8b7f5f9ad013dbb4117f2ff9637f6ba9c8f29` |
| `src/out/Default/args.gn` | `ac7f2384727cdf4e4b8ecc70f9ac2300f5284a25363f43e4d656273c177130e1` |

## Compilation

The current baseline uses the pinned Linux flags, source-bundled Siso, Clang,
Rust and sysroot. No global package or configuration change was made. The
source-bundled `autoninja` chooses its normal concurrency. At launch, the host
reported 16 CPUs, 30 GiB RAM and 243 GiB free on the work volume. These are
observations, not a resource policy or performance result.

Run from the prepared source directory:

```sh
env -u PYTHONEXECUTABLE -u __PYVENV_LAUNCHER__ \
  SISO_PATH=/home/timpan4/.t3/builds/arcified-linux-155-native-v1/src/third_party/siso/cipd/siso \
  python3 -B third_party/depot_tools/autoninja.py -C out/Default chrome chromedriver
```

Output is retained as `build-baseline.log` in the work directory. The baseline
does not include the unapplied destruction draft or other unregistered feature
snippets. Preserve this tree and logs when diagnosing build failures.

## Remaining proof

Compilation, native launch, navigation, extension loading, download and test
profile restart remain open for issue #16. Native sidebar/Space integration,
same-window multi-Profile ownership and Linux UX still need implementation and
interaction checks. Windows/macOS builds and runtime evidence remain separate
requirements. Source preparation does not close those gates.
