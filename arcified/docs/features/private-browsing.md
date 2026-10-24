# Private-context boundary

Status: reusable standalone C++ privacy preflight for [issue 98](https://github.com/Timpan4/helium-arcified/issues/98), 2026-10-23. Native call sites and installed acceptance remain open on Linux, Windows and macOS. This component does not create a private Profile, own pages, write storage or protect an unwired native path.

The requirements are the [publication artifact](../research/arc-dia/publication/private-browsing.md) and matching `private-browsing` record in [backlog.json](../research/arc-dia/backlog.json). Existing ownership remains in [lifecycle Model](../../native/model/lifecycle.h) under [contract v3](../lifecycle-contract.md). [Native Space integration](../native-space-integration.md) and its [source references](../native-space-source-references.md) retain the unproven same-window multi-Profile host gate. A WebView attachment cannot satisfy that gate.

## Shared context facts

[context_boundary.h](../../native/features/private_browsing/context_boundary.h) is the authoritative shared facts/classification seam. `ClassifyContext(const NativeContextFacts&)` is inline. Consumers need no `.cc` linkage or operation-policy dependency. It uses the existing typed ID template but introduces no registry or lifecycle state.

`NativeContextFacts` contains a closed `NativeContextKind`, an optional exact live `NativeContextId`, and optional `original_profile` mapping metadata. The kinds distinguish regular, OTR, Guest original, Guest OTR, system original, system OTR, other and unknown. Classification yields regular, OTR, Guest, unsupported or unknown. Invalid enum values, missing/empty native identity, and regular contexts without a nonempty Profile mapping classify as unknown. Guest and system OTR stay distinct from ordinary OTR even though all refuse ordinary persistence.

`NativeContextId` identifies the exact actual native Profile/BrowserContext instance. The native caller supplies a non-reused live identity, invalidates it on destruction and retains native lifetime responsibility. The identity must distinguish an original Profile, its primary OTR Profile and sibling non-primary OTR Profiles. It must also distinguish OTR instances created at different times. Multiple native windows may share one actual OTR context; a window ID does not identify a private session. Neither a filesystem path, `GetOriginalProfile()`, `IsSameOrParent()` nor an OTR ID by itself supplies exact equality. There is no identity allocation, serialization or registry here.

For a regular context, `original_profile` maps that regular Profile to its lifecycle ID. For OTR, it may identify the original Profile for caller lookup. It never converts the OTR context into that original, establishes native equality or authorizes service access. Private classification does not require an ordinary Profile mapping. Absence of a mapping cannot make private data eligible.

Profiles owns normal target/model validation and the eligible regular Profile snapshot. It consumes these facts by const reference and uses this classifier. Classification alone does not prove that the mapped Profile exists in Model, that the native object is live, or that it matches a requested Space/page. The native caller must supply facts from the actual source object, rather than the selected window, a display name, URL, saved destination or an old selection snapshot.

## Operation decisions

[operation_boundary.h](../../native/features/private_browsing/operation_boundary.h) and [operation_boundary.cc](../../native/features/private_browsing/operation_boundary.cc) expose `CheckOperation`, `CheckBatch` and `CheckUserFile`. Results are typed decisions. There is no generic boolean that grants every capability.

| Operation | Known regular context | Known ordinary OTR context | Guest, system, other or unknown |
| --- | --- | --- | --- |
| Transient organization | Privacy eligible | Privacy eligible within the same exact context | Refused with a distinct reason |
| Workspace persistence, Archive, capture persistence, browsing history, AI history, integration cache, diagnostics, workspace export | Privacy eligible, subject to the owning feature's checks | Refused | Refused |
| Optional remote activity, including AI context or public share | Requires separate request-specific authorization | Refused | Refused |
| Deliberate download, save file, clipboard copy or selected-content export | Explicit intent required | Explicit intent required | This component withholds eligibility |

`kEligible` is a privacy precondition only. It does not approve a navigation, transfer, native command, automatic cleanup, Profile migration or remote request. Regular context alone always returns `kRequiresRemoteAuthorization` for optional remote activity. The caller still needs the relevant user permission, provider opt-in, content selection, security checks and capability. AI history and AI context transmission are separate checks; neither authorizes the other. These APIs execute no provider requests.

Transient organization means in-memory presentation references within an exact context, such as ordering or grouping visible private tabs. It never means keeping a private page in an ordinary Space, creating a pin/Favorite, saving a capture to Library, registering Recent files, transferring a tab, changing Profile or navigating. Such ordinary workspace edits require the persistence check and are refused for OTR. No private Space/page ownership model or alternate focus/navigation store is added. Guest behavior is distinguished and conservatively refused by this downstream component until separately integrated; this does not disable Chromium's existing Guest/download commands.

`CheckBatch` checks the complete context provenance of a proposed payload. Empty provenance refuses. Any private or unknown source refuses the entire ordinary export, cache, history record, diagnostic payload or remote request. It never returns a filtered partial payload. Repeated exact identities with conflicting kind or original-Profile facts refuse as inconsistent. Transient organization across distinct exact contexts refuses, including sibling OTR contexts with the same original Profile. Regular batches may contain distinct Profiles, but their target binding, export permissions and destination selection still belong to Profiles and the owning feature.

## Deliberate user output

`UserFileRequest` is a separate API with an exact native context, a closed action kind and an intent value. `kExplicitWithPersistenceNotice` means the caller observed a deliberate user command for the selected content in that exact context and explained that the output may remain after private browsing ends. Missing or invalid intent refuses. A different or empty context identity refuses, even when both contexts share an original Profile. The native caller must revalidate context lifetime before use. Invalid action/operation enums refuse.

The successful result is `kPersistentUserCopy`, rather than ordinary persistence eligibility. The caller may proceed only with that chosen download/file/clipboard/selected-content export through native security, destination and cancellation handling. It cannot reuse the result for a workspace dump, background export, public share, provider upload, Archive, Library, Recent files, diagnostics or integration cache. A capture written only to a chosen user file needs this check; storing that capture in Library needs the ordinary capture-persistence check and remains refused for OTR. A public share needs the remote check and remains refused.

Saved output can outlive the private session. Closing a private window does not promise to erase downloaded files, saved files, clipboard contents or exported copies. The component does not delete them on shutdown. OS file indexing, clipboard history, synchronization configured by the user, external application copies and network observers are outside its erasure guarantees. Deliberate output never grants automatic secondary registration or payload logging. Native file and clipboard behavior still needs installed proof.

## Native source grounding

The inspected engine pin is Chromium `155.0.8059.79`, commit `a49e2e3d4addfd84e765b52afe230bedb81096db`, as recorded in the existing [source identity](../native-space-source-references.md#source-identity). The files below were read through Gitiles `?format=TEXT` with base64 decoding. No Chromium tree was downloaded, prepared or built. Web-reader access to Gitiles failed; direct read-only HTTP succeeded.

1. [Profile declarations](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/profiles/profile.h) distinguish OTR IDs, original Profile, regular/incognito/Guest/system kinds and primary/non-primary OTR. Guest covers both original and OTR objects. The shared enum preserves those distinctions instead of using contradictory caller booleans.
2. [Profile implementation](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/profiles/profile.cc) derives regular/incognito/Guest predicates from browser Profile type, OTR from its OTR identity, and has separate restrictions on browser-window creation. Non-OTR alone does not establish a regular browsing Profile. The future native capture must check Guest and system, then OTR, then regular, otherwise other; it must never create an OTR object just to inspect facts.
3. [OTR implementation](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/profiles/off_the_record_profile_impl.cc) returns the original's path, returns the original Profile separately, owns OTR preferences and shuts down its storage partitions. Its same-or-parent comparison explicitly accepts the original. These are reasons to preserve exact native identity and not treat path/parent equality as private eligibility.
4. [BrowserContext](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/public/browser/browser_context.h) states that OTR does not imply Incognito and exposes context-scoped services. [SessionServiceFactory](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/sessions/session_service_factory.cc) selects original regular Profiles. [HistoryServiceFactory](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/history/history_service_factory.cc) redirects regular-family lookups to the original. Factory lookup success therefore cannot substitute for checking the source context before writing downstream data.
5. Google's [Incognito behavior documentation](https://support.google.com/chrome/answer/95464?hl=en) explicitly retains downloaded files and deliberately saved bookmarks after the session. The downstream boundary is stricter for organization: it refuses private-to-ordinary pin/workspace conversion. Its deliberate user-output exception is explicit product behavior from issue 98, rather than a claim that Chromium automatically erases all disk or clipboard output.

## Caller integration contract and open proof

Before collecting or serializing content, the native caller resolves authoritative source contexts and complete provenance. It calls the operation boundary before touching ordinary stores, logs, cache queues, export serialization or optional remote preparation. This includes URLs, titles, captures, derived summaries and error payloads. Delaying the check until a final write can already leak data into intermediate buffers, diagnostics or integration caches. Rejection should expose a generic typed reason without logging the private payload.

For asynchronous operations, the caller must revalidate exact context lifetime, source document/content selection, model target and current permissions before each external effect. These owned facts/results are snapshots, not reservations, revocable permission tokens or commit acknowledgments. The component has no native event bridge, registry, lease or stale-event reconciliation. All provenance must remain attached to derived data. Missing provenance is unknown, never regular. Explicit local-copy intent applies only to that command's selected content and must not survive a changed selection or context.

Native integration must visibly and accessibly label private presentation using the actual selected context, including keyboard switches, dialogs and delayed callbacks. It must not label the whole OS window by an original Profile or accidentally route services through regular chrome. Same-window multi-Profile ownership is still unproven. This slice supplies no WebView facade or native UI.

Installed Linux, Windows and macOS checks remain open for private indication/accessibility, isolation, ordinary-store exclusion, Archive, AI context/history, integration caches, diagnostics, mixed-source exports, cancellation, asynchronous context destruction and user-file persistence. Use synthetic `.test` pages and temporary fixtures, never personal browsing data. No installed acceptance checkbox is closed by these standalone tests. Activity detection, cleanup/destruction and populated migration/source-disposition policy remain OPEN; there is no close, delete, reassignment or migration executor here.

## Focused checks

The standalone tests inspect public decisions, without launching a browser, reading browsing data, writing user files or calling a provider. `context_boundary_test.cc` compiles alone and covers the shared header seam. `operation_boundary_test.cc` covers ordinary sinks, remote authorization separation, complete batch refusal, inconsistent facts, exact-context organization, deliberate output, Guest/unsupported cases and invalid enum values.

Run from the repository root using the existing toolchain:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/private_browsing/*.h \
  arcified/native/features/private_browsing/*.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/private_browsing/context_boundary_test.cc \
  -o /tmp/arcified-private-context-test
/tmp/arcified-private-context-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/private_browsing/operation_boundary.cc \
  arcified/native/features/private_browsing/operation_boundary_test.cc \
  -o /tmp/arcified-private-operation-test
/tmp/arcified-private-operation-test
```

Both executables passed locally with strict Clang and GCC C++17 warnings. Both also passed Clang AddressSanitizer/UndefinedBehaviorSanitizer checks using `-g -fsanitize=address,undefined -fno-omit-frame-pointer`. Chromium-style formatting passed. These prove synthetic classification and preflight decisions only; Windows/macOS compiler checks and native behavior are unverified. No lifecycle/tooling/patch-series/workflow/execution-catalog changes or remote writes are included.
