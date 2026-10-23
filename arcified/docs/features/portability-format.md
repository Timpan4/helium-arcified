# Portable workspace format

Status: preparatory, synthetic C++17 metadata slice for [export #73](https://github.com/Timpan4/helium-arcified/issues/73). This is a reusable DTO, graph validator, byte codec and read-only lifecycle projection. It is not a native export command or a completed acceptance claim. Lifecycle semantic contract 3 remains synthetic and awaits fresh review. No installed native build, Linux/Windows/macOS export interaction, file picker, accessibility flow, destination replacement or interruption recovery is proven here.

## Authority and scope

[format.h](../../native/features/portability/format.h) owns the public API and [format.cc](../../native/features/portability/format.cc) owns its implementation. The [lifecycle model](../../native/model/lifecycle.h) and [contract](../lifecycle-contract.md) supply persistent identity and ownership. The export [publication brief](../research/arc-dia/publication/export.md) and exact `export` item in [backlog.json](../research/arc-dia/backlog.json) require supported names/order/URLs/metadata, validation, exclusion documentation and failure safety. This slice supplies the portable data boundary. Native source admission, UI and durable destination writes remain open.

The pinned [native integration design](../native-space-integration.md) and [source references](../native-space-source-references.md), especially C1, C3 and C4, keep actual Profile, WebContents, navigation controller and Profile-local session authority native. Their reviewed lifecycle design used contract 2. The current DTO explicitly names contract 3; it does not update or prove that native design. Same-window multi-Profile feasibility is unproven. Activity protection, cleanup, destructive actions and populated cross-Profile migration remain open under the current contract.

`Project(const lifecycle::Model&)` reads only `profiles()`, `spaces()`, `items()` and `shortcuts()`. Call it on the model's serialized owner sequence without concurrent mutations. It returns owned values, so later mutations cannot invalidate the result. It does not add an ownership store, navigation engine, allocator, persistence engine or replay operation.

The model has no private-context eligibility. `Project` is explicitly a synthetic metadata projection, with no host export entry point. A future native source adapter must consume the privacy authority's verified regular-context snapshot before projection or encoding. Unknown, off-the-record, guest and system contexts must fail closed at source admission. Neither serialized claims nor an ID/name can establish eligibility. This feature stores no context eligibility facts and cannot claim private-data exclusion from a `Model` alone.

## DTO and supported metadata

`arcified::portability::Workspace` contains `format_version = 1`, `lifecycle_contract_version = 3` and ordered vectors of the following records. IDs reuse the lifecycle model's strongly typed ID and owner types. They are source identities within the portable graph, not instructions to reuse IDs in a destination workspace.

| Record | Required fields | Optional metadata |
| --- | --- | --- |
| Profile | `id` | `name`, `provenance` |
| Space | `id`, `profile` | `name`, `saved_at`, `provenance` |
| Item | `id`, `owner`, `saved_destination` | `name`, `provenance` |
| Shortcut | `id`, `profile`, `target` | `provenance` |

An Item owner is exactly one `SpaceId` or `ProfileId`. A Shortcut is an alias to an existing Item, with the same effective Profile as its target. A standalone Profile shortcut is a Profile-owned Item. Those two representations retain their lifecycle meanings without copying a live page.

`Provenance` has nonempty UTF-8 `source` and `source_id`, plus optional unsigned 64-bit `ordinal`. The source namespace and source ID must survive unchanged. They are opaque origin labels, not authentication, authorization, native Profile discovery or automatic deduplication keys. Prototype migration sets `source` to `arcified-lab-v1:` followed by the caller namespace's byte length, a colon and the namespace. The original Space ID and URL ordinal retain distinct source entries, including identical URLs and duplicate display names. A synthetic grouping Profile requires an explicit destination binding. It cannot establish that a source had a native authentication Profile. Provenance collisions are left to source-specific validation and import reporting; common validation does not merge records.

Vector order is explicit portable record order, including the relative order of Items belonging to each owner. Encoding and decoding never sort, normalize or deduplicate records. Different IDs may share a name or URL. Names, timestamps and provenance never become identities.

The lifecycle model contains no display names, user presentation order, timestamps, folders, themes, custom titles or provenance. Native projection leaves all optional metadata absent and emits each map in its existing ID iteration order. This is deterministic serialization order, not UI order or evidence that UI order was recovered. A consumer with separately authoritative metadata may populate the optional fields before validation. This slice introduces no second metadata authority.

The actual [extension snapshot](../../extension/src/state.ts) is `{spaces:[{id,name,urls:string[],savedAt:string}]}` under `arcified-lab-v1`. It has source Space names, vector order, URL ordinals and `savedAt`, but no native Profile, Item or page identity. Its `savedAt` is represented only as optional opaque Space `saved_at`. Empty and absent strings stay distinct. No timestamp syntax, clock accuracy or lifecycle creation time is inferred. The prototype converter owns parsing explicitly supplied source-shaped JSON. This codec never reads extension storage, browsing data or accounts, and it is not a JSON parser.

## Wire format 1

The document starts with the exact ASCII bytes `arcified-workspace\n`, where the escape denotes one LF byte. It then contains fields in this order:

1. Format version, then lifecycle contract version.
2. Profile count, then that many Profile records.
3. Space count, then that many Space records.
4. Item count, then that many Item records.
5. Shortcut count, then that many Shortcut records. End of input is required.

A **number** is unsigned decimal ASCII followed by LF, with no sign, whitespace or leading zeros except the single digit `0`. It must fit unsigned 64-bit storage. Versions must match exactly. Counts are also bounded by the supplied remaining bytes because each record consumes bytes. There is no configurable record count, string length or file-size cap in this library. The reader does not reserve memory from an untrusted declared count.

A **text** field is a decimal UTF-8 byte length, a colon, exactly those bytes, then a comma. Length uses the same decimal rules as numbers. Text must contain valid Unicode scalar UTF-8 and no NUL. Embedded LF, colon and comma are permitted in general text. An **optional text** is a number `0` for absent, or `1` followed by a text field for present. An empty present text is `1\n0:,`, with one LF after `1`.

A **provenance** field is a number `0` for absent, or `1` followed by source text, source-ID text, then `0` for absent ordinal or `1` followed by an ordinal number. No other presence flag is accepted.

| Record | Wire field order |
| --- | --- |
| Profile | ID text, optional name, provenance |
| Space | ID text, Profile-ID text, optional name, optional saved_at, provenance |
| Item | ID text, owner-kind number (`0` Space, `1` Profile), owner-ID text, saved-destination text, optional name, provenance |
| Shortcut | ID text, Profile-ID text, target-Item-ID text, provenance |

The exact empty document is:

```text
arcified-workspace
1
3
0
0
0
0
```

It includes the final LF. There is no BOM, CRLF substitution, compression, encryption, checksum, extension block or ignored trailing data. Unknown versions and malformed/truncated encodings are rejected. The codec is deterministic and exact preserving, with no locale-sensitive conversion. A structurally valid document is not authenticated or evidence of a complete original workspace. A different valid graph or source may encode successfully. Future format or lifecycle changes require explicit compatibility review; this version does not auto-upgrade older contracts.

## Validation and URL boundary

`Validate(const Workspace&)` returns `std::optional<Error>`. It checks both versions, text, nonempty IDs, duplicate IDs within each typed namespace, the complete reference graph, effective Profile ownership and same-Profile aliases. Metadata names may duplicate or be empty. An Error contains an `ErrorCode` and a field/index path, never the source value, title or URL. Missing references and Profile mismatches are separate errors.

Saved destinations use a conservative, exact-preserving HTTP(S) interchange subset. It accepts lowercase `http://` or `https://`, an ASCII domain/localhost, dotted decimal IPv4 or bracketed IPv6, optional decimal port, and a path/query/fragment. Unicode path text remains exact UTF-8. IPv4 octets and IPv6 groups follow their address widths; ports fit the unsigned 16-bit port space. Domains reject empty labels and leading/trailing label hyphens. A final domain dot is preserved. IPv6 zone identifiers and alternate numeric IP spelling are unsupported. Percent escapes must contain two hexadecimal digits. ASCII whitespace/control, DEL, backslash, URL userinfo, empty authority, invalid IP syntax and empty/nondecimal/out-of-range ports are rejected. This is lexical validation, not DNS lookup, WHATWG canonicalization or Chromium `GURL` security policy. It neither opens nor authorizes a URL.

Non-HTTP(S) destinations, uppercase schemes, Unicode host spelling and browser-specific URL forms may be legitimate existing native destinations but are unsupported by format 1. They share `kInvalidDestination` with malformed inputs. The field path identifies the rejected destination without exposing it. A failed projection or encode rejects the whole value; it never silently omits, repairs or truncates a saved destination. A future browser adapter must report unsupported data to the user and obtain any conversion or omission decision through the owning workflow.

`Encode(const Workspace&) -> variant<string, Error>` validates before generating bytes. `Decode(string_view) -> variant<Workspace, Error>` parses framing and validates the complete DTO before returning any Workspace. Neither API exposes a partial Workspace on failure. The decoder checks numeric overflow and available bytes before copying text. C++ allocation failures propagate as allocation exceptions; no success value is returned. The library imposes no invented resource limit and supplies no asynchronous cancellation/yield mechanism. A future UI/streaming adapter must arrange cancellation, scheduling and any owner-approved resource bound before using this synchronous library on untrusted large input.

## Exclusions, source protection and durability

Projection excludes windows, selected Space, attachments, live pages, temporary tabs, current navigation, back/forward history, activity, split layouts, renderer state, unsaved forms and documents, downloads and native session records. Those values are not read or reconstructed. Reopening and Profile mapping belong to the destination/native authorities.

The DTO has no fields for cookies, passwords, credential stores, account sessions, site storage, extension installations/settings, AI provider keys or sync. No provider, account, service, sync or network call is introduced. This is a list of unrepresented stores, not a claim that arbitrary string metadata is secret-free. URLs, labels, source IDs and `saved_at` may contain sensitive text. URL userinfo is rejected, but query/path tokens and sensitive fragments cannot be detected reliably here. A source adapter must apply the user's approved export scope and privacy policy without assuming that successful validation removes secrets. The bytes are plaintext and unencrypted.

All APIs are pure with respect to the live model and filesystem. Returning an error, abandoning a generated value or cancelling before a caller writes it cannot mutate the source model or a destination file through this library. Focused tests verify runtime exclusion and unchanged model state. They do not prove a native export cancellation path.

The durability workstream's proposed `StoreImage` original/candidate staging and recovery seam concerns session snapshots. It does not provide arbitrary export-file replacement, filesystem flush, rename or crash guarantees. Export supplies a validated byte string only. It does not call that session store, implement a substitute writer, create temporary export files, truncate a destination or promise rollback. A future durability-owned native file adapter must prove unchanged existing destination on failure/cancellation and define the commit/cancellation boundary, atomic destination replacement, interrupted writes, platform-specific flush behavior and cleanup. Those acceptance requirements remain open. No successful unit test closes them.

## Focused verification

Run from the repository root:

```sh
bash arcified/native/features/portability/run-tests.sh
CXX=clang++ bash arcified/native/features/portability/run-tests.sh
clang-format --style=Chromium --dry-run --Werror arcified/native/features/portability/format.h arcified/native/features/portability/format.cc arcified/native/features/portability/format_test.cc
```

The area-local runner uses the existing C++ compiler and no downloads or installed dependencies. Its compile seam is:

```sh
c++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/portability/format.cc \
  arcified/native/features/portability/format_test.cc \
  arcified/native/model/lifecycle.cc -o <temporary-test-binary>
```

Pure DTO/codec consumers can link `format.cc` alone. Consumers that construct/mutate lifecycle fixtures also link `lifecycle.cc`.

Tests use only inert synthetic strings and reserved example domains. They cover a hand-authored golden document, explicit order and duplicate names/URLs, owner variants, source metadata and empty/absent distinction, maximum-width provenance ordinal, every truncated prefix of a populated encoding, framing/version/overflow failures, invalid UTF-8, duplicate identities, dangling references, cross-Profile aliases, supported and rejected destination syntax, saved/current-navigation separation, runtime exclusion and failure without model mutation. These checks prove the standalone contract slice. They do not prove native host feasibility, source eligibility, platform integration, durable file replacement or issue completion.
