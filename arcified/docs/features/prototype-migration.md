# Arcified Lab snapshot conversion

Status: implementation slice for [issue 76](https://github.com/Timpan4/helium-arcified/issues/76). Native import and platform acceptance remain open.

## Source contract

The source is the actual `AppState` value in [extension/src/state.ts](../../extension/src/state.ts), stored under `arcified-lab-v1`. Its shape is `{ "spaces": [{ "id": "...", "name": "...", "urls": ["..."], "savedAt": "..." }] }`. The current [panel](../../extension/src/panel.ts) has no export handler. This slice must receive explicitly supplied data, not read extension storage or imply a shipped export UI.

The source contains URL snapshots only. It contains no live pages, titles, browsing history, form data, cookies, account sessions, Profile mapping, folders, splits or Favorites. `savedAt` is the snapshot save time, not a page creation or last-use time. The extension currently generates it with `Date.toISOString()`. Older normalized entries can have an empty string. A source snapshot does not establish its current navigation.

Conversion preserves names and array order rather than running the extension's lossy `normalizeStored`. That helper can discard malformed records and truncate arrays. A migration must report unsupported or malformed data without silently losing it. Duplicate display names and duplicate URL strings do not identify the same Space or item. Original Space IDs and URL positions supply source identity; imported native identities require the import planner's explicit mapping.

## API and mapping

`prototype_migration::Convert(optional<string_view> json, string_view source_namespace)` in [converter.h](../../native/features/prototype_migration/converter.h) returns one of `Converted`, `Unavailable`, or `Error`. `nullopt` reports unavailable extension data; an empty string is malformed JSON. An available empty `spaces` array remains available. The caller must retain a nonempty stable namespace for the source installation or collection. A filename, display name, URL or native Profile is not that identity.

The JSON input is the storage value itself, not the enclosing `chrome.storage.local.get` result. Required fields are `spaces` and each record's `id`, `name`, and `urls`. An absent or empty `savedAt` produces `kTimestampUnavailable` and no timestamp. A nonempty string is retained verbatim as opaque source metadata, without date interpretation or a claim that the timestamp is valid. A non-string timestamp is an error. Empty IDs/names, duplicate source IDs, unknown fields, duplicate object keys and invalid JSON values fail conversion. Error fields use known schema paths and never copy imported values or unknown key names.

The local parser accepts only this source shape. It decodes [JSON string escapes and Unicode](https://www.rfc-editor.org/rfc/rfc8259#section-7), rejects malformed UTF-8 and unpaired surrogates, and does not recursively parse arbitrary nested values. Extra fields fail rather than silently dropping a potentially newer source feature. Missing fields and malformed bytes return a parser location. Conversion then calls the real [portable validator](../../native/features/portability/format.h); a portable failure returns no document and includes that validator's field/code.

| Source | Portable value | Identity and consequence |
| --- | --- | --- |
| Explicit stable namespace | One unnamed portable Profile | Source grouping only. Native import requires an explicit real Profile binding. No authentication or native Profile is inferred. |
| Space `id` | Length-delimited, namespace-scoped `SpaceId` | Original ID is also provenance. Duplicate source IDs fail, even with different names. |
| Space array position | Space vector order and provenance ordinal | Preserves source order; no native sidebar order is installed. |
| Space `name` | Space name | Retained exactly, including Unicode and whitespace. Names do not merge records. |
| Space `savedAt` | Optional `saved_at` | Opaque nonempty source string, never native lifecycle time. |
| URL at each array position | Space-owned saved item | Exact URL text, source Space ID and URL ordinal. Duplicate URLs retain separate identities. No live page or navigation is created. |

The portable provenance source includes the stable namespace. Length-delimited namespace/Space components prevent separator ambiguity; item IDs include URL position. Repeating the same conversion produces the same portable IDs and bytes. A changed URL at the same source position retains its source identity but changes content, so the import planner must review the conflict. Changing the namespace makes a distinct source. The converter has no import-history database and does not claim repeat-import idempotence. The [workspace import planner](../../native/features/workspace_import/) owns target mapping and duplicate/conflict reporting; native import must never silently replace an existing target.

No extension array/name truncation is reapplied. The converter does not impose a new record or byte quota. The future ingestion UI must handle cancellation and resource exhaustion without altering original storage or the export.

## Boundaries

The converter does not access files, extension storage, imported paths, URLs, browser objects, providers, accounts or sync. All text stays inert. It does not create tabs or Profiles, mutate the lifecycle model, persist workspace state, install dependencies, or clear originals. A failed conversion must return no portable document. Keep both original extension storage and the supplied export until a future native import is verified by the user.

The common validator accepts a conservative lexical HTTP(S) subset and rejects executable/local/path schemes, URL userinfo and malformed destination strings. Paths and queries can still contain secrets that this syntax check cannot identify, so a future export review must disclose them. This is not WHATWG/GURL parsing, native origin containment, safe navigation authorization or proof of a reachable destination. Nothing is fetched. Imported HTTP(S) destinations, names and opaque metadata remain untrusted. Archives, CSS, attachments and HTML document formats are unsupported inputs. Text that resembles HTML in a name remains text; an eventual UI must render it as text.

The [lifecycle contract](../lifecycle-contract.md) remains authoritative. Contract v3 describes synthetic transitions, not native acceptance or an on-disk format. Activity protection, cleanup, destructive actions and populated cross-Profile migration remain open. The [native ownership evidence](../native-space-integration.md) does not establish same-window multi-Profile feasibility.

## Native gaps

The explicit extension export UI, native input acquisition, Profile choice, durable ID allocation, import preview/confirmation, import transaction and crash recovery, source-retention verification, and installed Linux/Windows/macOS behavior require their owning integrations. No unit test can close those gaps. Platform visual and accessibility checks require installed native builds and remain open.

## Focused checks

Run from the repository root:

```sh
bash arcified/native/features/prototype_migration/run-tests.sh
bash arcified/native/features/prototype_migration/run-tests.sh \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer
bun test arcified/native/features/prototype_migration/fixture_shape.test.ts
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/prototype_migration/converter.h \
  arcified/native/features/prototype_migration/converter.cc \
  arcified/native/features/prototype_migration/converter_test.cc
bash -n arcified/native/features/prototype_migration/run-tests.sh
```

The C++ runner compiles this converter with the actual common format implementation and lifecycle source. It reads only the checked-in synthetic fixture, checks every incomplete prefix of that fixture, preserves duplicate-name/URL identity and metadata, exercises the real portable round trip, and rejects malformed/unsupported source data. The Bun check proves the synthetic fixture survives the actual extension normalizer unchanged. These checks do not read real extension storage and are not native browser builds or native import tests.

Verified locally on Linux on 2026-10-23: strict C++17 linked runner, AddressSanitizer/UndefinedBehaviorSanitizer runner, Chromium `clang-format` check, Bash syntax, and the Bun fixture-shape test all passed. No shell-specific linter is installed; Bash syntax is the runner check. Independent review is commissioned by the coordinator and is not claimed here.
