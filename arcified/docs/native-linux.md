# Downstream Linux preparation

Issue [#16](https://github.com/Timpan4/helium-arcified/issues/16) remains open. This repository supplies Chromium patches and configuration, not Chromium sources or a native Arcified executable. The tooling below establishes input identity and a preparation route. No real Chromium source preparation, browser compilation, or native launch has been verified by this change.

Core browser UX, platform integration, and performance take priority over AI. This route adds no AI integration, cross-device sync, or required browser account.

## Local preflight

Run from the downstream repository root, using Python 3.10 or newer, as specified by the repository's `.python-version`:

```bash
python3 arcified/native/tooling/linux.py preflight > /tmp/arcified-linux-identity.json
python3 -m unittest discover -s arcified/native/tooling -p 'test_*.py' -v
```

Preflight performs no downloads, extraction, GN generation, compilation, or launch. It reuses `devutils/check_patch_files.py` to check patch references, duplicates, unused files, and unified-diff readability. It records the fork commit, whether tracked files have changes, SHA-256 digests of preparation inputs, and every patch in series order. The working-file digests include uncommitted build-input changes; the commit alone does not identify those changes. Repository remotes, credentials, and environment variable values are not recorded.

This proves patch-file consistency only. It does not prove that patches apply to Chromium. JSON reports keep `native_build` and `native_launch` set to `OPEN`.

For an existing **unmodified** source tree, an optional marker check is available:

```bash
python3 arcified/native/tooling/linux.py preflight \
  --source-tree /absolute/path/to/chromium-src > /tmp/arcified-source-identity.json
```

This checks `chrome/VERSION` against `chromium_version.txt`, rejects already injected Helium versioning, and records VERSION and DEPS digests. These markers do not establish pristine source provenance. The executable preparation route instead retrieves fresh archives with the existing download utility's checksum verification.

## Pinned Linux tooling

The supported preparation route currently targets Linux x86_64. This follows the pinned platform repository's `chromium-linux` archive, which explicitly selects `linux-x64`; it is not a claim about other architectures.

The downstream Chromium version is `155.0.8059.79`. The selected `imputnet/helium-linux` revision is `b154afccb44729359c816754c7741efb3cb8f497`. Its `helium-chromium` gitlink is `b5145ab1047297970aceb9ca9f48722f6926cc66`, whose `chromium_version.txt` matches the downstream version. A newer inspected Linux revision, `473c0f72d7db2e8492f0fa975b8544bc719bb027`, targets Chromium `156.0.8078.47`, so the preparation command rejects it. Matching versions justify this starting pin; actual patch application and build compatibility still need source evidence.

Authoritative files inspected read-only:

- [Pinned production stage order](https://github.com/imputnet/helium-linux/blob/b154afccb44729359c816754c7741efb3cb8f497/scripts/build.sh).
- [Pinned source retrieval, patch ordering, resources, flags, and GN commands](https://github.com/imputnet/helium-linux/blob/b154afccb44729359c816754c7741efb3cb8f497/scripts/shared.sh).
- [Pinned Linux archive definition](https://github.com/imputnet/helium-linux/blob/b154afccb44729359c816754c7741efb3cb8f497/downloads.ini) and [build environment](https://github.com/imputnet/helium-linux/blob/b154afccb44729359c816754c7741efb3cb8f497/docker/build.Dockerfile).

On a separately authorized build host, obtain an isolated checkout of the small platform tooling repository at that revision. Do not initialize its `helium-chromium` submodule. Pass the platform checkout explicitly; the downstream script always uses the fork containing `linux.py` for Chromium version, common patches, utilities, resources, and configuration. It never substitutes the platform's upstream submodule for the fork.

```bash
git clone --no-checkout https://github.com/imputnet/helium-linux.git /absolute/path/to/pinned-helium-linux
git -C /absolute/path/to/pinned-helium-linux checkout --detach b154afccb44729359c816754c7741efb3cb8f497
```

## Review and execute preparation

1. Select a Linux x86_64 build host with its dependencies already provisioned. Use an isolated downstream checkout at the intended fork commit and the clean pinned Linux tooling checkout. The pinned Dockerfile documents the platform environment. This script neither installs dependencies nor creates a container. Record the actual environment image digest and installed tool versions separately. The upstream Dockerfile uses a mutable Debian tag and package repositories, so a commit pin alone does not make the environment byte-reproducible.
2. Choose a **new, nonexistent** work directory outside both repositories. Keep both input checkouts unchanged throughout preparation. Print and review the JSON plan. This command performs only local preflight and creates no work directory:

   ```bash
   python3 arcified/native/tooling/linux.py prepare \
     --platform-tree /absolute/path/to/pinned-helium-linux \
     --work-dir /absolute/path/to/new-arcified-linux-work \
     > /tmp/arcified-linux-plan.json
   ```

3. On that authorized build host, run the same command with `--execute` to retrieve and unpack the full source archives, apply patches, and generate GN output. **Do not run this step on the resource-constrained implementation machine. Full browser builds are prohibited on that machine.** Preparation is also substantial source I/O even though it does not compile:

   ```bash
   python3 arcified/native/tooling/linux.py prepare \
     --platform-tree /absolute/path/to/pinned-helium-linux \
     --work-dir /absolute/path/to/new-arcified-linux-work \
     --execute > /tmp/arcified-linux-result.json
   ```

The command uses `utils/downloads.py` for the fork's base source archive, Linux platform archive, and `deps.ini` assets, retaining their download cache and checksum sidecars. It checks source version markers before modifying source. It then follows the pinned production sequence: pruning with build dependencies retained, patching, domain and name substitution, translations, version injection, resources, flags, and GN generation.

`utils/patches.py apply --no-fuzz` applies the fork's complete `patches/series` in its existing order, then the pinned Linux series. It does not merge, refresh, or edit either series. There are currently no Arcified native feature patches. Future native changes must first be implemented and tested against real source, then carried in the fork's downstream patch group with coordinated series entries.

Resources are copied into the work directory before generation, so generated icons do not modify the fork's resource inputs. GN arguments concatenate `flags.gn` and `flags.linux.gn`, followed by the platform archive's `target_cpu = "x64"` and `v8_target_cpu = "x64"`. This is the pinned production configuration. It does not auto-enable host compiler caches, remote execution, or PGO. No Ninja, Siso, autoninja, browser compilation, packaging, installation, or browser launch step exists in this script.

`preparation.json` is saved in the work directory before execution and after each stage. Each stage has its own log. Only completion through GN yields `PREPARED_NOT_BUILT`. A failed or interrupted stage leaves the work directory, logs, backups, download cache, and failure status for inspection. The command never deletes a source tree or trusts upstream `.downloaded.stamp`/`.patched.stamp` files to skip stages. It refuses any existing work directory. After a failure, inspect the named log and choose a new directory for a corrected run; cleanup of the failed tree is an owner action.

For additional source applicability validation, `devutils/validate_patches.py --local` needs an unmodified source tree. The repository's shared CI retrieves a tree and runs that validation, but does not build a browser. Do not point it at already patched/prepared sources or treat that CI as native build evidence.

## Evidence still required for issue #16

All issue acceptance remains **OPEN** until native evidence is available. Local preflight and synthetic fixture tests cannot close these items.

1. On an authorized build host, retain `preparation.json`, all stage logs, archive checksum sidecars, the fork and platform revisions, working-input digests, final `args.gn`, environment identity, and the exact compilation command and exit status. Confirm the complete common and Linux series applied. Source preparation and GN generation are currently unverified against Chromium.
2. Compile the prepared source on that host using the pinned platform build toolchain and a separately approved resource policy. Preserve build logs and the resulting executable's digest. Do not invoke the unmodified platform `scripts/build.sh` or `docker-build.sh` as a continuation of this route: they choose their own source paths and include compilation. There is no downstream build or launch artifact yet.
3. Launch the resulting browser with a new test profile and retain a screenshot or recording plus exact binary/profile identity. Exercise native navigation, loading an extension, completing a download, and profile restart with persisted state. Record failures, accessibility checks, session/display configuration, and any unavailable platform evidence. Never substitute extension-only checks for these native checks.

Build-host needs are a provisioned Linux x86_64 environment, approved source/cache/build storage and compilation resources, and a graphical session for launch verification. No host resources, package installation, release identity, signing, update channel, publication, or optional AI behavior have been approved by this foundation. Select those when the corresponding work is authorized; do not infer a resource budget from the local tests.
