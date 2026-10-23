# Session durability

Status: reusable standalone C++ snapshot, codec, and store-image protocol for
[issue 50](https://github.com/Timpan4/helium-arcified/issues/50). Native acceptance
is blocked. This code does not write files or restore a browser.

## Ownership and API

[`session_snapshot.h`](../../native/features/durability/session_snapshot.h)
uses the existing [lifecycle model](../../native/model/lifecycle.h), semantic
contract version 3. Snapshot schema version 1 is independent of that semantic
version. No prior durable schema or upgrade path is claimed.

`DurableSnapshot` is an owned dormant snapshot of existing lifecycle identities
and references. It records Profiles, Spaces and their Profile mapping, windows
and selected Space, attachments, saved destinations, Profile shortcut aliases,
page identity/Profile/owner, and split membership/order. It is neither another
active lifecycle model nor a navigation owner. `ValidateSnapshot` builds a
private staging model through the existing lifecycle commands to check the
complete graph. It additionally rejects duplicates in commands that ordinarily
allow idempotent attachment or split replacement. It rejects saved-page records
that would resolve to another owning Space or another already-live saved page.
Validation-only navigation never leaves that private model.

`CaptureSnapshot` omits `LivePage::current_navigation` and activity. Native
Profile-local session machinery remains responsible for navigation history and
committed navigation. Cookies, authentication, settings, forms, POST bodies,
renderer memory, native pointers, native context identities and raw native
SessionIDs have no fields in this format. A captured item retains its saved
destination even when its live page has navigated elsewhere. Equal destinations
remain separate identities. Temporary identities retain their own window
attachment. No expiry, cleanup, destructive action or populated cross-Profile
migration policy is introduced.

`RestoreSnapshot` requires an explicit `RestoredNavigation` report for every
recorded page, exactly once, with its original Profile and a nonempty
browser-policy-validated committed destination. Missing, extra, duplicate and
Profile-mismatched reports fail without changing caller state. Successful
restoration constructs a new lifecycle model with the original IDs and unknown
activity. This function does not invoke native navigation, submit forms, open a
window, activate a tab, or prove that a reported native object exists. The caller
must reconcile actual native success before replacing its authoritative model.
Do not use this restart seam for transferring an existing live page.

## Admission and unsupported feature data

`SnapshotAdmission` defaults to refusal. Capture, encoding, write preparation,
restore and commit require whole-snapshot admission as well as callbacks that
admit each exact Profile and page context. Whole-snapshot admission executes even
for empty and Window-only graphs. Missing callbacks refuse. Even Profiles with
no pages are checked. The trusted root adapter must compare complete durable
provenance to its bound operation graph, resolve live
native objects and use the single shared
`private_browsing/context_boundary.h::ClassifyContext`, accepting regular
contexts only and verifying identity, Profile mapping and liveness. The local
callbacks are an integration boundary, not a second privacy classifier. The
peer header was inspected in the profiles worktree; it is not copied here. The
[pure composition adapter](context-persistence-composition.md) binds graph
comparison and shared preparation; no production native source exists here.
Synthetic test callbacks prove only this admission protocol.
Callers must serialize changes and revalidate native
eligibility before performing real persistence or publication; preparation is
not a native-context reservation.

Schema 1 omits peer Space name/icon/theme payloads, Space presentation order,
split orientation/weights, selected page/pane, group payloads, undo history and
native session joins. It supplies no generic opaque extension payload or second
peer DTO/codec. Callers must inspect their feature stores and explicitly attest
that unsupported feature payloads are absent; missing/false attestation rejects
capture, encoding, preparation and restore. Record vector order is serialization
order, not user presentation order. Existing `Model::splits` membership order is
retained, without claiming presentation persistence. Binding the exact canonical
peer headers and evolving the format remains root integration work. A model
alone cannot establish that these external stores are empty.

Decode and recovery inspect previously stored metadata without native admission.
They do not publish it to a browser or authorize writing it back. Future schema
or semantic versions are refused, never normalized into the current format.

## Wire format

Schema 1 is an uncompressed binary artifact. Unsigned integers use little endian.
The fixed envelope contains ASCII `ARCSESS\n` at byte 0, a `uint32` schema version
at byte 8, and a `uint32` semantic contract version at byte 12. `ProbeEnvelope`
reads only this complete envelope, including unsupported version numbers. It
does not authenticate or validate the body.

The body contains eight record sequences in this order: Profiles, Spaces,
windows, attachments, items, shortcuts, pages, splits. Each sequence starts with
a `uint64` count. Each string starts with its byte length as `uint64`, then exact
bytes. Identity strings use the existing model's nonempty-ID rules and are not
interpreted as display names, URLs or native identities. Schema field order is
defined by the single encoder/decoder:

1. Profile: ID.
2. Space: ID, Profile ID.
3. Window: ID, selected-Space tag, optional Space ID.
4. Attachment: window ID, Space ID.
5. Item: ID, owner tag, owner ID, saved destination.
6. Shortcut: ID, Profile ID, target item ID.
7. Page: ID, Profile ID, owner tag, owner ID, attachment.
8. Split: ID, attachment, page-ID sequence in membership order.

Tags are one byte. Window tags are 0 absent and 1 present. Item tags are 0 Space
and 1 Profile. Page tags are 0 temporary-tab owner and 1 saved-item owner. Other
tag values are malformed. The final `uint32` is IEEE CRC-32 over every preceding
byte, using the reflected polynomial and initial/final complement described in
[RFC 1952 section 8](https://www.rfc-editor.org/rfc/rfc1952.html#section-8).
CRC detects accidental corruption; it supplies no authenticity guarantee.

The decoder checks the envelope, supported versions, checksum, complete syntax,
exact end-of-input and full graph. Lengths/counts cannot exceed what the actual
remaining input can represent. It does not allocate a record array from an
untrusted count. There is no invented product capacity or retention limit. This
synchronous standalone parser has not been wired to a browser sequence; native
integration must arrange cancellable, appropriately scheduled disk work and
decide any resource policy before exposing untrusted large artifacts.

## Store-image write and recovery protocol

`StoreImage` owns optional exact original bytes and optional candidate bytes.
These are transaction roles, not a selected backup retention policy. The
original is the only committed authority. Candidate presence means an unfinished
write, even when its bytes decode successfully. An absent original differs from
an empty/corrupt original.

1. `PrepareWrite` accepts a genuinely empty store, or a valid supported original
   without a candidate. Corrupt/unsupported originals and every existing
   candidate prevent preparation. It validates and admits the new snapshot,
   then returns a private `PreparedWrite` retaining exact source bytes and an
   image with the original unchanged and the encoded candidate added.
2. An eventual disk adapter must persist and validate the candidate while
   retaining the original. This code supplies no file IO, flush, rename,
   directory sync, process locking, crash-atomic replace or OS durability claim.
3. `CommitPreparedWrite` compares the caller's entire staged image with the
   prepared bytes. A changed original or candidate refuses commit. Its required
   current `SnapshotAdmission` argument then admits the candidate decoded by the
   existing codec. `PreparedWrite` stores no borrowed callback. Refusal preserves
   both images. Success
   returns a new image whose original is the candidate, with no candidate slot.
   The prepared object retains the prior original exactly. This is an in-memory
   result, not evidence that a disk commit occurred.

`Recover` decodes only the original. A candidate is retained and reported,
never auto-promoted or deleted. A corrupt original is an explicit failure even
if the candidate is valid; explicit recovery must decide its disposition while
preserving all bytes. Missing original plus candidate cannot establish that any
write committed. Successful recovery reports that native navigation and renderer
memory have not been restored. No function mutates its input images. Failed
decode, restore, preparation or commit therefore leaves the source accessible.

The migrations feature consumes this envelope/codec and owns compatibility
admission across both images, including refusal when a future candidate exists
beside a current original. `Recover` alone can inspect the committed original;
it is not that whole-store compatibility gate. Root integration must call the
compatibility gate before recovery/write orchestration. No historical migration
or alternative serializer is implemented here.

## Source reuse and remaining acceptance

The issue body, publication brief and `backlog.json` require last-valid-workspace
recovery without overwriting recoverable originals. The existing
[native source ledger](../native-space-source-references.md#c4-navigation-and-session-boundaries)
records Chromium's Profile-bound SessionService, tab/window association and
selection storage. Its session IDs are session-local, so they cannot be reused
as these durable IDs. The
[native integration design](../native-space-integration.md#persistence-and-restoration)
requires durable allocation, native session joins and cross-Profile host grouping
before browser restore can be claimed.

The inspected
[Helium lazy-session-restore patch](../../../patches/helium/core/lazy-session-restore.patch)
restores a nonempty selected navigation record first, skips empty navigation
records, and reconstructs native splits in tab-strip order. This slice reuses
the lifecycle model rather than replacing those native restore responsibilities.
No native source preparation or patch application ran.

Synthetic checks prove versioned encode/decode, graph validation, identity and
selected-Space preservation, navigation-evidence refusal, unknown restored
activity, admission refusal, every strict candidate truncation preserving the
committed original, every single-bit corruption of the fixture being rejected,
and exact-byte stale-source commit refusal. Independent checksum fixtures check
the wire envelope and a checksum-valid impossible record count. Missing whole
admission and rejected Window-only graphs refuse through the public persistence
operations, including commit. Integration fixtures cover graph provenance and
native/peer changes after preparation.

Issue 50 remains open. No installed-build Linux, Windows or macOS interaction,
visual/accessibility check, browser crash, controlled native disk interruption,
OS filesystem durability, native identity mapping or same-OS-window Profile
restore was tested. Space presentation order/page selection, native skipped-tab
reconciliation, allocation/tombstones, disk commit protocol, recovery UI and
canonical peer feature persistence remain integration needs. Unsaved renderer
memory and form resubmission are not supported recovery guarantees.

## Focused checks

Run from the repository root. These are synthetic standalone checks, not a
Chromium build or native browser test.

```sh
clang-format --style=Chromium --dry-run --Werror arcified/native/features/durability/session_snapshot.h arcified/native/features/durability/session_snapshot.cc arcified/native/features/durability/session_snapshot_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic arcified/native/model/lifecycle.cc arcified/native/features/durability/session_snapshot.cc arcified/native/features/durability/session_snapshot_test.cc -o /tmp/arcified-session-durability-test
/tmp/arcified-session-durability-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer arcified/native/model/lifecycle.cc arcified/native/features/durability/session_snapshot.cc arcified/native/features/durability/session_snapshot_test.cc -o /tmp/arcified-session-durability-sanitized
ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 /tmp/arcified-session-durability-sanitized
```
