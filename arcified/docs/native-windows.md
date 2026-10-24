# Downstream Windows preparation plan

Issue [#17](https://github.com/Timpan4/helium-arcified/issues/17) remains open. This foundation records downstream input identity and prints a Windows x64 source preparation plan. Execution is deferred by the owner. There is no Chromium download, prepared tree, GN result, native build, installed browser, or Windows interaction evidence from this work.

## Read-only commands

Run from the fork root with its existing Python tooling:

```sh
python3 -B arcified/native/tooling/windows.py preflight
python3 -B -m unittest discover -s arcified/native/tooling -p 'test_*.py' -v
```

`preflight` hashes working inputs, records Git commit and tracked-change status, and checks patch-file consistency through the repository's `devutils/check_patch_files.py`. It uses the same `input_identity.py` helper as Linux. The common input set, ordered patch digests, repository-root and path checks, and optional source-marker behavior are shared. Untracked files under the named input directories are hashed too. The commit alone does not identify uncommitted downstream work.

Preflight creates no source/cache/output directory, downloads nothing, and runs no Windows tools. It reports the actual host system, machine, and Python version. `toolchain.status` remains `UNVERIFIED`, including on Windows. A Windows host flag identifies only the OS and architecture. It proves neither an installed toolchain nor a working executable. Reports leave `source_preparation`, `native_build`, and `native_launch` at `OPEN`.

An existing unmodified Chromium tree can be checked locally:

```sh
python3 -B arcified/native/tooling/windows.py preflight --source-tree /absolute/path/to/src
```

This compares `chrome/VERSION` with the fork's `chromium_version.txt`, rejects injected `HELIUM_` markers, and records VERSION and DEPS digests. Version markers do not prove source provenance, pristine content, patch applicability, or Windows execution.

## Compatible platform pin and current-source evidence

Inspected read-only on 2026-10-23. The fork targets Chromium `155.0.8059.79`. The newest compatible revision in the inspected `imputnet/helium-windows` main history is `555c9467373779ea030d6b555d7bd05b48d96d8c`. Its `helium-chromium` gitlink is `b5145ab1047297970aceb9ca9f48722f6926cc66`, whose [Chromium version](https://github.com/imputnet/helium/blob/b5145ab1047297970aceb9ca9f48722f6926cc66/chromium_version.txt) matches exactly. This is also the upstream source identity used by the Linux preparation foundation.

The next Windows revision, [11a292ea63c107e8ed7f5edb5883df10f977b845](https://github.com/imputnet/helium-windows/commit/11a292ea63c107e8ed7f5edb5883df10f977b845), changes the gitlink to `832faa7822566270e10c0d3262f702f9d0fea81f`, targeting [155.0.8059.84](https://github.com/imputnet/helium/blob/832faa7822566270e10c0d3262f702f9d0fea81f/chromium_version.txt). The inspected current head, `e4b527255e3e0546ac140b7cb52d285fadb2d7d0`, targets [156.0.8078.47](https://github.com/imputnet/helium/blob/f29a25968e483f8bb85e47829ccd487bbe229199/chromium_version.txt). Neither is an exact engine match. Version agreement justifies the older pin; application of every Windows patch to this downstream fork remains unverified.

Authoritative pinned inputs:

- [build.py](https://github.com/imputnet/helium-windows/blob/555c9467373779ea030d6b555d7bd05b48d96d8c/build.py) defines the x64 tarball route, preparation order, patch executable, resource order, Visual Studio environment, GN, and subsequent compilation.
- [downloads.x64.ini](https://github.com/imputnet/helium-windows/blob/555c9467373779ea030d6b555d7bd05b48d96d8c/downloads.x64.ini) selects the Windows x64 tooling archive. [downloads.ini](https://github.com/imputnet/helium-windows/blob/555c9467373779ea030d6b555d7bd05b48d96d8c/downloads.ini) pins PortableGit, NSIS, and WinSparkle sources and their SHA-256 hashes.
- [flags.windows.gn](https://github.com/imputnet/helium-windows/blob/555c9467373779ea030d6b555d7bd05b48d96d8c/flags.windows.gn), [patches/series](https://github.com/imputnet/helium-windows/blob/555c9467373779ea030d6b555d7bd05b48d96d8c/patches/series), and [resources](https://github.com/imputnet/helium-windows/tree/555c9467373779ea030d6b555d7bd05b48d96d8c/resources) supply platform configuration and assets.
- [Platform prerequisites](https://github.com/imputnet/helium-windows/blob/555c9467373779ea030d6b555d7bd05b48d96d8c/README.md) and [CI environment setup](https://github.com/imputnet/helium-windows/blob/555c9467373779ea030d6b555d7bd05b48d96d8c/.github/actions/prepare-environment/action.yml) describe the Windows environment. Reading CI is not a CI run.

Use an existing, isolated, clean checkout at that platform revision. Do not initialize or use its `helium-chromium` submodule. Preflight verifies HEAD and the gitlink directly from Git metadata, hashes the platform inputs, and rejects tracked changes and nonignored untracked files. The fork containing `windows.py` supplies all common patches, utilities, resources, and configuration. No upstream submodule is substituted for it.

## Review a preparation plan

1. Choose a new, nonexistent work directory outside both input repositories. On the intended Windows host, paths can use native Windows syntax. The command is also readable on Linux, where its paths and interpreter describe only a plan generated on that host:

   ```sh
   python3 -B arcified/native/tooling/windows.py prepare \
     --platform-tree /absolute/path/to/pinned-helium-windows \
     --work-dir /absolute/path/to/new-windows-work
   ```

2. Retain the JSON for review. `prepare` creates nothing and has no `--execute` option. Its ordered steps are declarative operations, not a shell script. They describe creating isolated directories, retrieving and verifying archives with the fork's `utils/downloads.py`, unpacking, checking source markers, pruning with build dependencies retained, applying patches, domain/name substitution, translations, version injection, resources, flags, and GN generation. The manifest order is fork `downloads.ini`, Windows `downloads.x64.ini`, Windows `downloads.ini`, then fork `deps.ini`. The `retrieve` command checks all archive hashes before the separate `unpack` step, which does not recheck them. A future executor must protect the cache from changes between these stages. Archive availability and checksum sidecars have not been downloaded or verified here.

3. Keep source retrieval and all preparation on hold until the owner authorizes a suitable Windows host and resources. A future executor must implement the emitted operation types, enforce the execution contract, and record stage results. This foundation does not include that executor. Do not invoke upstream `build.py` as a preparation shortcut. It uses its own source layout and continues into compilation.

The patch command explicitly selects the archive's `third_party/git/usr/bin/patch.exe` and `--no-fuzz`. It applies the fork's complete existing series first, then the pinned Windows series, preserving each order. No series is merged, refreshed, filtered, or edited. Platform patches include installer, updater, and DRM integration; their presence is source evidence, not acceptance of those features or approval to package, sign, publish, or enable services.

The plan copies Windows resources and common fork resources to separate work directories before generating them. It installs Windows resources before common resources, matching the pinned script, without generating files in either input repository. GN flags concatenate fork `flags.gn` and platform `flags.windows.gn`, then append `is_official_build=true`, `enable_winsparkle=false`, and `use_remoteexec=false`. The Windows flags override the fork's PGO phase with `chrome_pgo_phase=2`, as upstream production does. The plan preserves that requirement rather than claiming a cheaper compilation configuration. It records the expected UTF-8 `args.gn` digest and concatenation recipe without printing flag contents that could contain credentials.

Remote execution, host cache detection, updater keys, signing credentials, and service credentials are not inherited by the plan. The plan omits upstream's `configure_siso.py` call, which selects a Google RBE project even with an empty REAPI address. Chromium's pinned [siso.gni](https://chromium.googlesource.com/chromium/src/+/155.0.8059.79/build/toolchain/siso.gni) allows the explicit `use_siso=true` platform flag without that setup for GN generation. Backend selection and validation remain part of the deferred compilation route. GN requires an already verified Visual Studio developer environment and process-local `DEPOT_TOOLS_WIN_TOOLCHAIN=0`. No environment values, repository remotes, authenticated URLs, or secret values are collected. A future executor must enforce this environment contract before running commands. No compilation, packaging, installation, signing, or launch stage exists here.

## Host/toolchain and failure limits

The route covers native Windows x64 only. The pinned prepared archive does not cover ARM64. Linux or WSL preflight and GNU patch fixture tests cannot validate Windows paths, `patch.exe`, extraction, GN, SDK detection, or browser behavior.

The [Chromium 155.0.8059.79 Windows instructions](https://chromium.googlesource.com/chromium/src/+/155.0.8059.79/docs/windows_build_instructions.md) require Windows 10 or newer, Visual Studio 2026 version 18.0.0 or newer with Desktop development with C++ and MFC/ATL components, Windows 11 SDK 10.0.28000.2270, and SDK debugging tools 10.0.26100.3323 or newer. Clang-cl uses Visual Studio headers, libraries, and tools. Record exact installed versions and the archive-provided compiler/tool identities on the approved host. The platform CI checks an SDK include directory named `10.0.28000.0`; that directory name alone does not prove the required SDK servicing version.

The pinned platform README specifies Python 3.11 or newer, 7-Zip, requests, Pillow, Windows Developer Mode for symbolic links, and lifted Windows path-length restrictions. Its CI selects Python 3.12. The read-only identity code can run with the fork's Python 3.10 baseline, but that does not certify the Windows preparation environment. Confirm the actual extractor and zstd path, filesystem and symlink policy, source/output storage, and graphical session before execution. No package installation or host configuration change is authorized by these instructions. The upstream resource guidance is not a measured resource budget for this fork; the owner still needs to approve one.

There is also a known utility difference: the pinned Windows `build.py` tolerates absent pruning-list entries and calls a newer `prune_files` signature. This fork's CLI currently fails when pruning entries are absent. The plan uses the fork's existing strict CLI. If it fails on the prepared Windows archive, retain that failure and review it rather than silently skipping entries or substituting upstream utilities. Full archive preparation is required to settle applicability.

Local preflight failures exit nonzero and produce no successful JSON report. A future executor must stop on every failed or interrupted stage, save input identities and per-stage logs, retain download cache/checksum sidecars, source edits, and backups, and leave native acceptance open. It must compare identities before and after the run, use no upstream stamps to skip stages, refuse existing work directories, and preserve failed trees for inspection. No automatic deletion or resume is specified.

## Evidence still required for issue #17

All native acceptance remains **OPEN**:

1. On an authorized Windows host, record the actual OS, filesystem policy, Python/extractor, Visual Studio, Windows SDK, compiler and GN identities. Retain fork/platform revisions, working-input digests, complete ordered patch results, verified archive hashes, preparation logs, and final `args.gn`. Prove real source preparation and GN generation.
2. After separate build authorization, retain the exact compilation command, resource policy, logs, exit status, and executable digest. No browser build or artifact exists from this foundation.
3. Launch that exact binary with an isolated test profile on Windows. Retain screenshots or a recording and binary/profile identity. Exercise navigation, completing a download, loading an extension, window minimize/maximize/restore/close, and profile restart with persisted state. Record failures, keyboard/focus and accessibility behavior, display/session details, and unavailable checks. Extension tests and Linux fixture tests cannot supply this evidence.

Release identity, installation, signing, updater channel, publication, and optional AI behavior require their own decisions. This foundation enables none of them.
