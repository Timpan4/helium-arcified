# Workspace import planning

Status: preparatory standalone C++ slice for [issue 136](https://github.com/Timpan4/helium-arcified/issues/136). No native import, merge transaction or platform acceptance is implemented.

The authoritative requirements are the [publication brief](../research/arc-dia/publication/workspace-import.md) and the exact `workspace-import` item in [backlog.json](../research/arc-dia/backlog.json). Ownership follows [lifecycle contract v3](../lifecycle-contract.md) and the [model](../../native/model/lifecycle.h). The current v3 model is synthetic and its fresh independent review is pending.

## Scope and safety boundary

The feature validates a copied portability payload and prepares an owned, inert preview against a const lifecycle model. It does not open tabs, allocate live page identities, attach windows, select Spaces, mutate saved destinations or apply a merge. There is no import-owned workspace, navigation state or durable mapping store. Discarding a preview cancels planning without changing the source bytes or destination model.

Source Profile identities never identify local security contexts implicitly. Every imported Profile requires an explicit mapping to an existing destination Profile. Each Space has exactly one mapped Profile. New saved items remain closed metadata. Existing saved pages retain their current navigation, immutable Profile, activity and attachment, even when the preview reports a duplicate or conflict. Selection and actual native opening are separate future actions.

The extension [snapshot shape](../../extension/src/state.ts) stores `id`, `name`, `urls` and `savedAt` for each Space. It has no authoritative Profile, persistent item identity or native live state. Prototype conversion belongs to the separate migration slice. Import must never infer a Profile from a name, URL, browser window or credential store.

## Preview API and repeat imports

[planner.h](../../native/features/workspace_import/planner.h) consumes the export-owned [portability DTO and codec](../../native/features/portability/format.h). `PreviewCopy` decodes supplied bytes through the real codec. `Plan` also validates typed DTO inputs before producing a manifest. Invalid encoding, graph references, text, destinations or versions return the common structured error without a partial manifest. Neither entry point reads a file or credential store.

`Mapping` explicitly binds every source Profile to an existing local Profile and every Space, saved item and shortcut alias to a destination ID. Source IDs are never installed implicitly. Omitted mappings produce unmapped rows. Unknown source keys produce mapping problems. Empty targets, missing expected records and competing mappings to one target cannot produce successful new/duplicate rows. Multiple source Profiles may bind to one existing Profile only through these explicit caller bindings.

| Intent and destination | Report |
| --- | --- |
| Create, target ID unused | New metadata candidate with mapped ownership. |
| Create, target ID exists | Conflict even when content is identical. Equality is not identity. |
| Match existing, target missing | Conflict. A retained association cannot recreate a deleted record implicitly. |
| Match existing, mapped owner and supported payload identical | Duplicate. Space Profile, saved destination and owner, or alias Profile and target must match as applicable. |
| Match existing, content or owner changed | Conflict. No replacement, movement, Profile reassignment or source removal. |
| Required parent mapping unresolved/conflicting | Dependent record is unmapped with a blocked-dependency reason. |

Repeat imports require caller-retained source-to-destination associations and the correct intent. Self-asserted file provenance does not authenticate an association. Changed source records are compared against the current destination, so changes to a mapped saved URL, owner or alias target are conflicts. Distinct explicit records with equal URLs remain distinct. New target IDs must come from the eventual coordinator's never-reused allocator; the planner checks current collisions but cannot prove historical non-reuse.

`Manifest.source` owns all validated source fields and vector order. `unmapped_metadata` identifies names, source timestamps, provenance and record order that lifecycle cannot store or compare. A duplicate report covers supported lifecycle fields only. It does not claim those metadata fields were persisted or equal to native values. `HasUnresolvedRecords` checks mapping/record problems; callers must separately present the metadata notices. A preview does not become a durable record when this function returns.

## Untrusted input

The common format accepts a documented conservative lexical HTTP(S) subset. Its destination checks do not establish WHATWG/GURL validity, browser navigation permission, origin classification or safety of a remote resource. Real native URL policy must run before any later selected-tab opening. The planner never navigates, fetches, resolves a host, runs a script or renders imported text. Preview clients must render names and provenance as text, including HTML-looking labels. No archive, attachment, HTML document or CSS payload can be extracted through this DTO. Unsupported external files are rejected by the codec; no filesystem path is interpreted, joined or opened. Path/origin containment for a future attachment format remains open rather than implied by this metadata-only slice.

## Native integration still required

The lifecycle model has `AddSpace`, `AddItem` and `AddShortcut`, but no atomic import transaction, persisted provenance, never-reused ID allocator, journal, rollback or crash recovery. Sequentially issuing those methods is not a recoverable merge. A future workspace coordinator must stage a validated copy and preview, preserve mappings with provenance, revalidate destination state, and atomically commit or recover metadata plus mapping associations before reporting success. The planner supplies no commit token or reservation. Failure, cancellation or interruption must preserve originals and existing workspace state.

The [pinned ownership evidence](../native-space-source-references.md) and [native integration design](../native-space-integration.md) establish that desktop Browser, chrome and session paths assume an immutable Profile. Same-window multi-Profile feasibility remains unproven. Importing metadata does not resolve that gate. Native GURL/security policy, real Profile resolution and lifetime, lazy loading on explicit selection, accessibility, file-picker/copy handling, journal recovery and installed Linux, Windows and macOS evidence remain open.

Activity protection, cleanup, destructive actions and populated cross-Profile migration remain OPEN under the lifecycle contract. This feature adds no override, source removal, reassignment or silent conflict replacement. No browsing data, credentials, accounts, sync or AI providers are accessed.

## Focused verification

Run from the repository root with the existing Clang toolchain. The runner follows the lifecycle contract's explicit Clang driver mode and Chromium formatting policy. It creates and removes only its own temporary executable directory.

```sh
bash arcified/native/features/workspace_import/run_checks.sh
bash arcified/native/features/workspace_import/run_checks.sh --sanitize
```

[planner_test.cc](../../native/features/workspace_import/planner_test.cc) drives the real sibling `Project`, `Encode`, `Decode` and `Validate` implementation. It checks explicit destination ownership, distinct equal-URL identities, retained repeat mappings, changed-content and Profile conflicts, collision propagation, absent/invalid mappings, preserved metadata notices, all interrupted prefixes of actual encoded fixture bytes, malformed input, unsupported versions, unsafe destinations, dangling references, cancellation and stale-preview replanning. Existing protected pages retain their saved/current destinations, Profile, attachment, activity and selection. The sanitizer mode enables AddressSanitizer and UndefinedBehaviorSanitizer with failing diagnostics.

The runner also links the real prototype converter. A composed synthetic scenario converts duplicate-name/equal-URL snapshots, explicitly maps them, previews new records, sets up prior imported metadata, previews duplicates and reports a changed source URL as a conflict without replacing the destination. That setup is fixture construction, not a production Apply or transaction.

These are synthetic planner, converter and codec checks, not an import transaction or native host/build test. Native file preservation/recovery under interrupted writes, installed UI/accessibility behavior and Linux/Windows/macOS native acceptance remain unverified. There is no UI in this slice to visually verify.

On 2026-10-23 the ordinary and ASan/UBSan runner modes passed using the real sibling codec and lifecycle implementation, including the direct encoded-version rejection cases. Chromium formatter verification and strict C++17 compiler warnings passed. `bash -n` accepted the local runner. Per-file `git diff --no-index --check /dev/null` checks emitted no whitespace diagnostics for all five new files; exit status 1 denotes new-file differences. Local documentation targets were confirmed present. This evidence proves the standalone preview behavior above; it does not close issue 136.
