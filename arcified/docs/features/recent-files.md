# Recent files

Issue [53](https://github.com/Timpan4/helium-arcified/issues/53), key `recent-files`. This slice implements reusable C++ eligibility, type filtering, action preparation and host-report validation. It has no native provider or UI integration. The issue remains open.

## Scope and ownership

Requirements come from the [publication acceptance criteria](../research/arc-dia/publication/recent-files.md) and the `recent-files` entry in [backlog.json](../research/arc-dia/backlog.json). Downloads is a prerequisite. The [v3 lifecycle core](../../native/model/lifecycle.h) supplies `ProfileId` and typed identity machinery. This feature neither mutates that core nor adds file records to its browsing-item graph.

The accepted handoff is caller-fed data only. Native services own records, persistence, actual Profile/OTR context, navigation and focus. Library projects scoped provider results or opens an existing native provider. There is no shared Library database/API, dependency on a Library header, or claim that this feature is a Library provider. Capture, media and created-item providers remain unavailable until concrete native providers exist. The downloads peer owns native download records and persistence.

The [native ownership design](../native-space-integration.md) and [pinned source references](../native-space-source-references.md) constrain future hosting. Their Chromium pin is `155.0.8059.79`, commit `a49e2e3d4addfd84e765b52afe230bedb81096db`. Those documents establish source observations about ownership, not a working native download adapter. The same-window multi-Profile host remains unproven. This slice adds no Chromium patch or native API claim.

## Caller-fed interface

The [header](../../native/features/recent_files/recent_files.h) and [implementation](../../native/features/recent_files/recent_files.cc) expose pure functions without a service facade or store.

1. Supply a `FileObservation` for a native record. `FileReference` contains the existing `ProfileId`, an opaque `RecordId` and an opaque `Revision`. These are references, not copied native records or paths. The caller must map actual Profile/OTR contexts to distinct lifecycle IDs and scope access against that actual context. Nonempty text alone does not establish native identity or permission. Namespace record keys if multiple native sources can share a key, and never reuse an identity for a replacement record. Change the revision whenever native identity, file location, access, completion, safety or capability observations change. No timestamp, retention cutoff or list size is imposed.
2. Report independent completion, safety, access and freshness facts. Their defaults are unknown, and supported actions default empty. Only explicit complete, allowed, accessible and current facts qualify. Missing includes moved/deleted files whose original reference no longer resolves. Unknown type is unsupported. Native code owns type classification and OS trust checks; no filename/MIME heuristic in this component grants safety. `TypeSelection` explicitly chooses supported categories. Empty selection shows no files. Invalid enum values and empty IDs fail closed.
3. Call `FilterRecentFiles` with caller-ordered observations. It preserves recency order and returns only eligible references, plus input indexes and reasons for exclusions. It excludes all copies of duplicate keys within a Profile, even if one revision appears safe and another is blocked. Foreign Profile references never enter the returned file list. The caller must avoid rendering rejected input metadata. Rejection indexes are diagnostics, not additional recent-file rows.
4. On an explicit open, reveal, copy-file or drag-out request, re-resolve the native record and call `PrepareAction` under the actual Profile scope. It checks the selected identity/revision, current eligibility, user selection and the requested native capability. It returns refusals or an owned `PreparedAction` containing the operation ID and full observation. Preparation performs no OS action and grants no security authority. There is no remove-reference or file-deletion action in this API. Any future remove-reference operation removes only a service-owned reference; it must never delete the native file. Deletion policy remains open.
5. Before native actuation, the eventual caller must recheck actual record/file identity, accessibility, safety, scope and native capability. It must handle races at the OS boundary using the native integration's supported identity/handle mechanisms, preserve trust prompts, and report actual failure. A snapshot cannot make filesystem operations atomic. The component performs no IO, clipboard write, shell launch or drag operation.
6. Feed a `HostReport` and a fresh observation into `ReconcileHostResult`. It rejects mismatched operation, action or observation, invalid values, changed revisions and changed facts, including capability changes with an unchanged revision. Profile checks run again. Missing, denied, partial, quarantined and blocked observations cannot reconcile to success. `kSubmitted` remains submission; `kCompleted` is only a trusted caller assertion of observed completion. An adapter whose OS API reports dispatch only must use submission or unverified, never completion. Cancellation, unsupported operations and failures remain distinct. No report alone proves an OS action worked.

The native caller allocates non-reused operation IDs, serializes conflicting operations, drops late reports after cancellation/closure, and consumes each report once. These pure functions retain no pending-operation registry and cannot detect a replay after the caller has forgotten its operation. Fact snapshots catch observable changes, while revisions must also identify changes that later revert to the same values. There are no callbacks, retry policies, background scans, recency database, path persistence, browsing-data access or focus/navigation ownership here.

## Focused verification

[Synthetic fixtures](../../native/features/recent_files/recent_files_test.cc) exercise public APIs with invented identities and no filesystem records. They check explicit type selection and order, all supported categories/actions, unknown defaults, invalid IDs/enums, Profile separation, duplicate/conflicting records, concurrent safety/progress/access exclusions, stale selection, unsupported actions, native submission versus completion, failures/cancellation, mismatched results and changed observations. These are component checks, not OS or native browser checks.

Run from the repository root with the existing toolchain:

```sh
recent_files_check_dir=$(mktemp -d /tmp/arcified-recent-files.XXXXXX)
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/recent_files/recent_files.h \
  arcified/native/features/recent_files/recent_files.cc \
  arcified/native/features/recent_files/recent_files_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror \
  -Wswitch-enum -pedantic \
  arcified/native/features/recent_files/recent_files.cc \
  arcified/native/features/recent_files/recent_files_test.cc \
  -o "$recent_files_check_dir/recent-files-test"
"$recent_files_check_dir/recent-files-test"
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror \
  -Wswitch-enum -pedantic -g -fsanitize=address,undefined \
  -fno-omit-frame-pointer \
  arcified/native/features/recent_files/recent_files.cc \
  arcified/native/features/recent_files/recent_files_test.cc \
  -o "$recent_files_check_dir/recent-files-sanitized"
"$recent_files_check_dir/recent-files-sanitized"
clang-tidy \
  arcified/native/features/recent_files/recent_files.cc \
  arcified/native/features/recent_files/recent_files_test.cc \
  --checks='-*,clang-analyzer-*' --warnings-as-errors='*' \
  -- -std=c++17 -Wall -Wextra -Werror -Wswitch-enum -pedantic
```

Executed locally on 2026-10-23 with Clang, clang-format and clang-tidy 23.1.3. Strict compilation and fixtures, ASan/UBSan fixtures, Chromium formatting and analyzer checks passed. The deliberate invalid-enum fixture helper has a narrowly scoped `NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)` because those unsupported values are the test input; tidy reported seven suppressed instances and no remaining diagnostics. Local documentation link targets exist. New-file whitespace checks emitted no diagnostics; their exit status 1 denotes differences from `/dev/null`. No native provider, browser interaction or desktop-platform acceptance ran.

## Acceptance still open

| Acceptance | Component evidence | Remaining proof |
| --- | --- | --- |
| Accessible files and user-selected supported types | Caller-fed eligibility/filter fixtures | Actual native source, freshness/access probes, selection UI and persistence owned by native services |
| Native drag-out, copy, open and reveal | Capability validation and request/result correlation fixtures | Actual clipboard file payloads, drag destinations, launch/reveal behavior and OS failure/cancellation reporting |
| Missing/moved/deleted and denied never report false success | Missing/denied/stale refusals and late-result fixtures | Filesystem identity/races, permissions changing at actuation and restart reconciliation |
| Partial/quarantined/blocked are not safe completed files; reference removal differs from deletion | Independent exclusions, default unknown and no deletion action | Native download safety/trust mapping, visible states and service-owned reference removal behavior |

Linux, Windows and macOS installed native checks all remain open, including visual interaction, keyboard/accessibility, focus return, Profile/OTR separation, restart behavior and actual native file handling. Linux standalone compilation is not Linux native acceptance. No browser was built or launched. Full Chromium preparation/build, downloads, heavy source retrieval, benchmarks, global installs and remote CI are deferred. No real browsing data or AI provider was accessed. Activity, cleanup, destructive actions and populated cross-Profile migration remain open.
