<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Repeatable engine update procedure

This is the coordinator's execution procedure. This packet performed only the
read-only audit and local packet checks in `checks.json`. The declared audited
revision is Chromium 155.0.8059.79, not a completed old-to-new update.

## Declare the source and security change

1. Read current authoritative [Chrome desktop release notices](https://chromereleases.googleblog.com/search/label/Desktop%20Update),
   the linked Chromium change log and security issue list. Distinguish desktop
   stable, early stable, extended stable, mobile and OS-specific releases.
   Record retrieval date, notice publication date, URLs, old revision and new
   revision per target OS in a copy of `update-record-template.json`.
2. Resolve the prepared-source archive and its publisher hash manifest through
   the existing fork and platform download manifests. Preserve the manifests,
   hashes, upstream tag/commit, fork commit plus working inputs, platform commit,
   dependency versions and ordered patch identities. Record what was actually
   verified, including unavailable upstream or restricted security details.
   A publisher checksum is not a signing or provenance policy decision.
3. Review the security changes against downstream patches and enabled features.
   Record critical fixes, downstream adaptations and unresolved exposure or
   backport questions with primary links. Refresh this review at execution
   and publication. Dia's documented schedule is reference evidence only.
   Do not invent an Arcified cadence or select delivery/signing infrastructure.

## Prepare and reconcile patches

1. Preserve the working baseline and prepare the selected revision in a fresh,
   coordinator-owned work directory using existing platform native tooling.
   Do not reuse the read-only shared tree or destructive cleanup as a shortcut.
   Run the existing preflight before preparation. The Linux route is
   `arcified/native/tooling/linux.py`; use its plan before `--execute`.
   Windows and macOS use their existing native platform routes and require
   their actual SDK/runtime environments. An unavailable route stays OPEN.
2. Keep preparation stage order from the platform tooling: verified retrieval
   and extraction, source-version check, pruning, strict fork then platform
   patch application, domain/name substitutions, translations, downstream
   version stamp, resources and GN arguments/generation. Preserve all logs and
   input identities. The current Linux patch step uses
   `utils/patches.py apply --no-fuzz`; do not weaken it to obtain a green run.
3. For each failure, record patch path and before SHA-256, source revision,
   failing file/hunk, exact strict-apply diagnostic, upstream API/behavior
   change, proposed resolution and native regression cases. Keep the original
   patch and failed-source evidence. Do not drop a hunk, accept fuzz, or record
   an unresolved conflict as fixed because a later stage passed.
4. Reconcile against the new native services and APIs. Preserve lifecycle Model,
   metadata/codec, command targets, private/download classification and Profile
   isolation. Do not fabricate success for unavailable bindings. Record every
   refreshed patch's before/after SHA-256, changed hunks and explanation.
   If quilt is used in an isolated maintenance tree, retain the existing series
   order and review its refreshed diff. Quilt bookkeeping is not proof of
   successful replay through the actual preparation path.
5. Replay the complete ordered fork and platform patch stack from fresh verified
   source after refresh, then repeat later preparation stages. Attach the strict
   replay log and identity record. A sparse packet apply checks this packet's
   mechanics only; it cannot prove that the full Chromium stack reapplies.
   The coordinator alone registers reconciled production patches.

## Prove the native result

1. Generate and build the actual native browser plus registered browser tests
   with the target platform toolchain. Preserve GN arguments and their SHA-256,
   toolchain/SDK versions, source/input identity, build logs and binary hashes.
   Retain failed build results with the failing source/API and resolution.
2. Run `ArcifiedEngineUpdateBrowserTest.*` in the resulting
   `browser_tests` executable. On Windows the executable is
   `browser_tests.exe`. Use the existing platform test runner and its defaults;
   add no packet-specific retry, timeout or parallelism policy. Refresh the
   test's declared Chromium version together with the target revision record.
3. Run all affected integrated native tests for lifecycle admission, real
   Profile bindings, private/download classifiers, workspace metadata/codec,
   commands, sessions/restore and changed native seams. Use each integrated
   feature packet's exact registered targets and filters. Do not substitute
   standalone Model tests, synthetic host objects or extension checks.
4. Verify the installed native interaction on Linux, Windows and macOS:
   startup/version identity, existing Profile data, real command targets,
   normal/private isolation, Space switching, saved-page state/handoff,
   failure/cancellation and restart/restore behavior. Preserve protected work.
   Record visual evidence and exact observed failures. Unimplemented
   same-window multi-Profile or saved-page handoff stays OPEN, even when
   generic Chromium navigation/cookie tests pass.
5. Keep each OS result separate. A compile-only result, absent SDK, unrun test,
   missing installed artifact or unresolved behavior is OPEN. Signing,
   notarization, installer identity, update channel, rollback and data-disposition
   policies belong to their approved release workstreams. Do not choose them
   while executing an engine-source update.

## Release record

1. Complete the update record with source/download and binary identities,
   ordered fork/platform patches, each conflict and refresh, strict replay
   evidence, native build/test/visual results and remaining security questions.
   Keep the historical result separate from later runs. Protect secrets and
   private browsing/Profile content in evidence.
2. Obtain the coordinator's independent review and authorized publication path.
   Publish the reviewed conflict record and release provenance with the release.
   A local packet, JSON record or pending native test does not satisfy publication.
   Keep issue 41 open until the declared update and all target native results
   are proven. No remote write or publication is performed by this packet.
