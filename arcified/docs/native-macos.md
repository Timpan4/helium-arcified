# Downstream macOS preparation plan

Native acceptance for issue [#18](https://github.com/Timpan4/helium-arcified/issues/18) is **OPEN**. This foundation checks local source and patch identities and prints a reproducible macOS preparation plan. The owner deferred full builds and heavy preparation. No Chromium archive, prepared source tree, GN result, native build, installed browser, signing result, or macOS interaction evidence exists from this work.

## Read-only preparation tools

Run from this fork with its existing Python component tooling:

```sh
python3 -B arcified/native/tooling/macos.py preflight
python3 -B -m unittest discover -s arcified/native/tooling -p test_macos.py -v
```

`preflight` records the actual host OS, machine and Python version. It reuses `input_identity.py` for the fork commit, tracked-change status, working-input SHA-256 digests, ordered patch digests and repository/path checks. Named input directories include their untracked working files. A commit alone does not identify the downstream working inputs. Patch-file consistency uses the fork's existing `devutils/check_patch_files.py`, which checks readable patches, missing/unused files and duplicate series entries. These checks do not apply patches to Chromium.

Both commands are lightweight local checks. The CLI downloads nothing, creates no work/cache/output directory and invokes no macOS executable. It suppresses import bytecode writes. `toolchain.status` remains `UNVERIFIED`, even on macOS. The host flag proves only OS and machine identity, not Xcode, SDK, Metal, APFS, compiler, GN or runtime availability. Source preparation, patch application, GN generation, native build, launch and overall native acceptance stay `OPEN`.

An existing Chromium source tree can be checked without modification:

```sh
python3 -B arcified/native/tooling/macos.py preflight --source-tree /absolute/path/to/src
```

This compares `chrome/VERSION` with the fork's `chromium_version.txt`, rejects existing `HELIUM_` version markers and records VERSION and DEPS digests. Matching version markers do not prove archive integrity, provenance, pristine source content or patch applicability.

## Compatible tooling and source evidence

Inspected read-only on 2026-10-23. The newest exact engine match in the inspected `imputnet/helium-macos` main history is [`9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28`](https://github.com/imputnet/helium-macos/commit/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28). Its `helium-chromium` gitlink is `b5145ab1047297970aceb9ca9f48722f6926cc66`, whose [Chromium version](https://github.com/imputnet/helium/blob/b5145ab1047297970aceb9ca9f48722f6926cc66/chromium_version.txt) is exactly this fork's `155.0.8059.79`.

The next platform revision, [`d049c7d9c42f2c4d6d039e6bb05f3c67ddf15524`](https://github.com/imputnet/helium-macos/commit/d049c7d9c42f2c4d6d039e6bb05f3c67ddf15524), changes the gitlink to `832faa7822566270e10c0d3262f702f9d0fea81f`, targeting [155.0.8059.84](https://github.com/imputnet/helium/blob/832faa7822566270e10c0d3262f702f9d0fea81f/chromium_version.txt). Inspected current main head `9aa975a56de8c2376d7c22f71da564b9426cda5f` uses `f29a25968e483f8bb85e47829ccd487bbe229199`, targeting [156.0.8078.47](https://github.com/imputnet/helium/blob/f29a25968e483f8bb85e47829ccd487bbe229199/chromium_version.txt). Those newer revisions are incompatible with the declared exact engine pin. Version agreement selects the older tooling revision; it does not prove that all its patches apply to this fork.

Authoritative pinned inputs:

- [devutils/shared.sh](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/devutils/shared.sh) defines preparation order, host archive selection, target mapping, resources, release flags and GN. [env.sh](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/env.sh) defines tool paths. [build.sh](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/build.sh) continues into compilation, signing and packaging, so it is not a preparation-only entry point.
- [downloads.arm64.ini](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/downloads.arm64.ini) and [downloads.x64.ini](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/downloads.x64.ini) select host tools archives. [downloads.ini](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/downloads.ini) pins Sparkle source and its SHA-512 hash.
- [flags.macos.gn](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/flags.macos.gn), [patches/series](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/patches/series) and [resources](https://github.com/imputnet/helium-macos/tree/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/resources) supply platform flags, ordered patches and assets.
- [docs/building.md](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/docs/building.md) describes the platform prerequisites. Reading [CI setup](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/.github/actions/prepare-environment/action.yml) is documentary evidence only; no workflow was dispatched.

Use an existing, isolated, clean checkout at the pinned platform commit. Do not initialize or substitute its `helium-chromium` submodule. Optional `--platform-tree` preflight verifies platform HEAD and the recorded gitlink without opening the submodule. It hashes preparation scripts, manifests, flags, revision, resources, documentation and complete ordered platform patches, and rejects tracked changes and nonignored untracked files. Common patches, resources, utilities and configuration always come from this explicit `Timpan4/helium-arcified` checkout containing `macos.py`. No shared helper change is needed.

## Review a plan

1. Select an intended macOS host architecture and target architecture explicitly. Use `arm64` for Apple Silicon and Chromium's `x64` spelling for Intel `x86_64`. Host selection controls executable tools in the downloaded archive; target selection controls GN `target_cpu`. Each host supports either target per the pinned script. The [Chromium 155 arm Mac instructions](https://chromium.googlesource.com/chromium/src/+/155.0.8059.79/docs/mac_arm64.md) describe cross compilation in either direction. This foundation produces separate architecture plans, with no universal-binary merge stage.

2. Choose a new, nonexistent work directory outside both input repositories. The source path must contain no spaces, as required by the pinned [Chromium macOS instructions](https://chromium.googlesource.com/chromium/src/+/155.0.8059.79/docs/mac_build_instructions.md). For an Apple Silicon host targeting Intel:

   ```sh
   python3 -B arcified/native/tooling/macos.py prepare \
     --platform-tree /absolute/path/to/pinned-helium-macos \
     --work-dir /absolute/path/to/new-macos-work \
     --host-arch arm64 --target-arch x64
   ```

3. Retain the JSON for review. `prepare` creates nothing and has no `--execute` option. Each stage is `NOT_RUN`; overall preparation is `PLANNED` and execution is `UNIMPLEMENTED_OWNER_DEFERRED`. Operations such as `mkdir-new`, `copy-tree-new` and `concat-utf8` are declarative recipes, not an installed executor or shell script. A Linux-generated plan records Linux paths and interpreter identity. Regenerate it on the approved Mac before any future execution.

4. Leave source retrieval and every preparation stage unexecuted until the owner authorizes the host and resources. A future executor must enforce the report's environment, input-preservation and failure contracts. It must implement the declared filesystem operations, verify identities before and after execution, and retain per-stage logs and results. This foundation supplies no executor and no automatic cleanup or resume.

Work-path isolation checks existing ancestors by filesystem identity, including the nearest existing ancestor when work and its parents do not exist. This rejects alternate-case aliases into either input on case-insensitive volumes while accepting distinct directories on case-sensitive volumes. These checks create no directories.

The ordered recipe describes directory creation, verified retrieval, unpacking, source-marker checks, pruning, patch application, substitutions, translations, version injection, resources, flags and GN generation. Download manifest order is fork `downloads.ini`, platform `downloads.<host>.ini`, platform `downloads.ini`, then fork `deps.ini`. The fork's retrieval utility checks archive hashes. Unpack does not recheck them, so a future executor must protect the verified cache between retrieval and extraction. Archive availability, sizes and hash sidecars remain unverified here. There is no fallback to a full Chromium clone.

Pruning retains contingent paths with `--keep-contingent-paths`, matching the pinned macOS script and preserving archive-provided host tools. It uses this fork's strict pruning CLI. Missing listed files cause failure rather than silent skipping. Patch application adds `--no-fuzz` and applies the complete fork series first, then the platform series, preserving both orders. Neither series is merged, refreshed, filtered or edited. A synthetic GNU patch test on Linux proves this ordering in a fixture. It does not prove macOS patch utility behavior or full Chromium applicability.

Domain and name substitution backups stay in the new work directory. Resources are copied there before generation. Common resources are generated, then macOS resources and common resources are installed in that order, matching the pinned script without changing either input repository. The version utility receives `--tree` pointing at this fork and `--platform-tree` pointing at the pinned tooling.

GN concatenates fork `flags.gn` and platform `flags.macos.gn`, then selects the explicit target and release PGO phase `chrome_pgo_phase=2`. This preserves the upstream release requirement rather than claiming a cheaper build. Overrides set `enable_sparkle=false`, `include_branded_entitlements=false`, `use_remoteexec=false` and an empty `cc_wrapper`. The plan records a UTF-8 concatenation recipe and expected `args.gn` SHA-256, without printing flag contents that could contain API credentials. No environment values or repository remotes are collected.

The final planned command is archive-provided `buildtools/mac/gn gen out/Default --fail-on-unused-args`. Upstream's `configure_siso.py` call, remote Linux compiler retrieval and ambient cache detection are omitted. Chromium's pinned [siso.gni](https://chromium.googlesource.com/chromium/src/+/155.0.8059.79/build/toolchain/siso.gni) accepts explicit `use_siso=true` for GN generation without backend configuration. Siso configuration and its actual local compilation path remain deferred. No compilation, packaging, installation, signing, notarization, provisioning or launch stage is emitted.

## Host and toolchain requirements remain unverified

The platform's pinned building guide states macOS 12+, Xcode 26, Python 3.13, Homebrew tools including zstd and coreutils, requests/Pillow, and the Metal toolchain. Its CI [Xcode script](https://github.com/imputnet/helium-macos/blob/9e160dda4fbb8bd329f108c5d1fda2f8eb2a6d28/.github/scripts/github_prepare_xcode.sh) selects Xcode 26 and modifies the host. None of those installation, deletion, configuration or download commands is run or included in the plan. The read-only code can use the fork's Python 3.10 baseline; that alone does not establish a macOS preparation environment.

Chromium 155's [mac_sdk_overrides.gni](https://chromium.googlesource.com/chromium/src/+/155.0.8059.79/build/config/mac/mac_sdk_overrides.gni) sets SDK minimum `15`. Its [mac_sdk.gni](https://chromium.googlesource.com/chromium/src/+/155.0.8059.79/build/config/mac/mac_sdk.gni) records official SDK `26.5`, build `25F70`, including the universal SDK reference for Intel hosts, and deployment/runtime minima `13.0`. The exact SDK assertion depends on build branding and configuration. These are different contracts. The pinned Helium guide's macOS 12 statement does not establish Chromium 155 runtime support, and its Xcode 26 setup does not prove SDK 26.5 availability. Actual SDK selection, downstream branding, Metal, GN and compilation must be checked on the approved Mac before claiming compatibility.

Chromium also requires an APFS work volume. Record the real OS, filesystem, selected Xcode/developer directory, SDK version/build, Metal, Python, extractor/zstd, patch implementation, archive-provided compiler and GN identities. Record exact host and target architecture separately. No toolchain probe or native interaction ran on this Linux host. Storage and compilation resource policy remain owner decisions; this work invents no build budget or concurrency setting.

The pinned platform series contains native menu, keychain, fullscreen, Sparkle and signing integration. Its presence is source evidence only. The plan disables the optional updater and branded entitlements. It accesses no keychain, signing certificate, Apple account, provisioning profile or signing secret. An unsigned local development artifact is distinct from a distributable signed/notarized release. No artifact of either kind was produced, and no quarantine or security-policy bypass is specified.

Failures in local preflight exit nonzero without a successful JSON report. Future execution must stop on failure or interruption, retain source edits, substitutions/backups, verified archive sidecars, input manifest and stage logs, and leave acceptance open. Do not reuse upstream stamps, delete failed work or substitute an upstream submodule to continue past a failure.

## Focused checks performed

On Linux, the focused `test_macos.py` suite passes its 14 tests. It covers working-file identities, exact source-version and gitlink rejection, patch-file failures, dirty platform rejection, all host/target pairs, read-only planning, secret omission, work-directory guards, resource/flag recipes, CLI execution rejection and bytecode suppression. Work isolation tests use the real adapter with emulated case-insensitive `stat`/`lstat` lookup for aliases into both inputs and separate work. Distinct case-sensitive siblings are tested on the Linux fixture volume. This is not native APFS evidence. Synthetic fixtures remain under `/tmp/arcified-macos-*` for inspection. Its patch test actually applies ordered synthetic fork and platform patches through this fork's inherited utility with GNU patch. It does not apply the real Chromium patches.

The default CLI preflight passes on the actual downstream checkout. With a small isolated checkout at the selected macOS pin, preflight validates and hashes all 346 fork and 23 platform series entries. Both cross-architecture CLI plans succeed, leave every stage `NOT_RUN` and leave their requested work directories absent. The platform checkout remains clean. No submodule or Chromium source was fetched. YAPF with `.style.yapf` and Pylint pass on the two new Python files using the existing local check environment, YAPF 0.43.0 and Pylint 4.1.3. These are focused Linux tooling results, not native build evidence.

## Evidence still required for issue #18

All native acceptance remains **OPEN**:

1. On an authorized Mac, verify the actual toolchain and host/target architecture, archive integrity, preparation result, full ordered patch application and GN generation. Retain fork and tooling identities, working-input digests, archive hashes, SDK/compiler versions, logs, failures and final `args.gn` identity. Local marker checks and synthetic patches do not satisfy this requirement.
2. After separate build authorization, retain the exact compilation command, approved resource policy, logs, exit status and application/binary digests. Distinguish the unsigned development route from any separately approved signing, notarization, packaging, installation or distribution work.
3. Launch that exact build in a native macOS graphical session with an isolated profile. Exercise navigation, an extension, a completed download, native application menus and profile restart with persisted state. Retain screenshots or a recording and source/build/profile identity. Record failure and recovery behavior, keyboard/focus and accessibility behavior, display/session details and unavailable cases. Real keychain behavior and profile persistence need native evidence; mock-keychain runs cannot establish them.

Core browser UX, native platform integration and measured performance precede optional AI. This preparation enables no AI service, cross-device sync or required browser account. Extension-only checks and Linux tests cannot close the native criteria.
