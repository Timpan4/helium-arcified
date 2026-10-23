# Command ranking core

Preparatory C++ component for issue 109. `RankCommands` projects the authoritative
[lifecycle Model](../../model/lifecycle.h) at contract version 3, using ephemeral
caller labels. It is independent of the command bar and performs no native
action. Native acceptance on Linux, Windows and macOS remains open.

## Scope and privacy boundary

Every request explicitly names one Profile, a caller attachment and a privacy
classification supplied by the real BrowserContext boundary. Unknown and private
requests return `kPrivacyExcluded` before constructing results. Missing Profiles,
unattached callers and Profile mismatches return typed errors. The request
attachment establishes the caller window and allowed Profile. It does not limit
discovery to the selected Space.

Saved items are discoverable across Spaces in that Profile, including closed
items whose owning Space is unattached to the caller window. Temporary pages
must also belong to the caller window. Same-Profile temporary pages in another
Space of that window are included. Other windows' temporary pages and all other
Profiles' data are excluded before label matching. Labels cannot add candidates
or grant visibility. Shortcut aliases do not duplicate their target's row.

The lifecycle model has no privacy metadata. The native adapter must bind exact
Profile/OTR/guest contexts to distinct lifecycle Profile IDs and classify the
request authoritatively. Passing a false `kRegular` assertion or merging private
and regular contexts into one ID cannot be detected here. The synthetic checks
prove filtering under that boundary contract, not native privacy isolation.
Cross-Profile discovery is intentionally unavailable. A separate caller scope
does not authorize reading another Profile's labels or browsing data.

## Result and ranking contract

Each `RankedCommand` includes its Profile ID, display provenance and a typed
`reveal` payload. `RevealSavedItem` carries ItemId, authoritative ItemOwner and
optional live PageId/attachment. `RevealTemporaryPage` carries TemporaryTabId and
live PageId/attachment. The payload supplies the identities for the secondary
"Reveal in Workspace" action without inventing a window for a closed item.
Profile-owned shortcuts have no owning Space. Their live attachment, when
present, is location provenance rather than ownership.

`verb()` derives "Switch" for a live target and "Open saved destination" for a
closed saved target. Duplicate titles never replace these verbs or provenance.
`identity()` returns the typed ItemId or TemporaryTabId. It stays stable across
label/navigation updates and saved close/reopen/transfer. Keep changes temporary
ownership into saved ownership, so the key then changes deliberately. Result
order is not a selection policy. The command bar owns selection stability.

Matching folds ASCII case and otherwise preserves UTF-8 bytes. The whole query
must match contiguously in an explicit item label, page label, saved destination
or current navigation of an included live page. Provenance labels and opaque IDs
are not search fields. The best match determines exact, prefix, then substring
order. Equal matches use typed lifecycle identity order, with saved items before
temporary tabs. An empty query returns all eligible results in that identity
order. These tiers are explicit ordering, with no weighted score or result cap.
There is no tokenization, locale collation, Unicode case folding, fuzzy search,
URL classification, recency inference or persistence.

An explicit item label takes display precedence over a live page label, then the
saved destination. A temporary page uses its page label or current navigation.
Empty/missing loading labels use those fallbacks. Both renamed pins and live
titles remain searchable. Long destinations are searched in full. Rendering,
safe text display, visual truncation and accessibility belong to the native UI.

The per-call output vector is also used to join saved live pages by ItemId.
There is no retained ownership, navigation or search index. Model writes must be
serialized by the caller while ranking reads its const views. Work consists of
scanning items/pages, joining saved pages through binary search in the output,
matching eligible text and sorting the matches. For instrumentation, time the
complete `RankCommands` call with the embedding platform's monotonic clock and
record synthetic corpus size plus query/result identity. That boundary excludes
native event delivery, UI scheduling, rendering, accessibility and execution.
No native latency measurement, benchmark or timing threshold is claimed here.

## Executor adapter boundary

The coordinator approved keeping this component independent of command-bar
headers. A future adapter maps `identity()` to the bar's row identity, displays
verb/provenance and forwards the typed `reveal` payload for selection or reveal.
The coordinator owns `command_target.*` revalidation. No cross-child header
dependency is introduced by this component.

Before execution, revalidate the item/page, exact Profile/privacy context and
current native liveness/location. A result snapshot is neither a reservation nor
permission to navigate, attach a Space or transfer a page. Missing attachment
preparation belongs to the executor. Closed targets use the saved destination.
Saved live selection uses the approved `OpenItem`/`OpenShortcut` semantics to
preserve the existing PageId, current navigation, immutable Profile and activity.
Do not reopen its current URL or create another live page. Temporary selection
focuses the existing local page and never transfers it.

Native preparation, cancellation/failure reconciliation, actual focus and URL
policy remain with the native integration. Calling the in-memory lifecycle
transition alone does not prove native success. The pinned
[ownership design](../../../docs/native-space-integration.md) and
[source references](../../../docs/native-space-source-references.md) identify
the required window/Profile ownership work. Activity, cleanup, destructive
actions and populated migration decisions remain open.

## Focused synthetic checks

Run from the repository root using the existing local C++17 toolchain. The
driver mode follows the lifecycle core's documented Clang link requirement.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/commands/command_ranking.h \
  arcified/native/features/commands/command_ranking.cc \
  arcified/native/features/commands/command_ranking_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/commands/command_ranking.cc \
  arcified/native/features/commands/command_ranking_test.cc \
  -o /tmp/arcified-command-ranking-test
/tmp/arcified-command-ranking-test
clang-tidy --checks='clang-analyzer-*,bugprone-*,performance-*' \
  arcified/native/features/commands/command_ranking.cc \
  arcified/native/features/commands/command_ranking_test.cc -- -std=c++17
```

The installed `clang-tidy` defaults enable only compiler diagnostics, which
cannot run as a tidy check set. The command explicitly selects analyzer,
bug-prone and performance checks for these standalone files. Address/undefined
sanitizers can run the same fixture by adding
`-g -fsanitize=address,undefined -fno-omit-frame-pointer` to the compile command
and using a separate output executable.

The repeatable `.test` corpus checks exact/prefix/substring order independently
of insertion order, duplicate titles, renamed pins, long destination tails,
empty/loading labels, ASCII folding and the stated Unicode limitation. Fixtures
also cover privacy and Profile exclusion, window-local temporaries, unattached
closed discovery, alias identity, Profile shortcut provenance, fresh results
after mutation and saved live identity preservation through a synthetic
selection. No real browsing data, provider access or network is used.

Native integration, installed-build visual/keyboard/accessibility checks,
native privacy/failure behavior and latency evidence remain open on Linux,
Windows and macOS. These standalone fixtures do not close issue 109.

Local verification on 2026-10-23 passed the strict C++17 compile/run above,
the same fixture with AddressSanitizer/UndefinedBehaviorSanitizer, and Chromium
format validation. `clang-tidy` completed with enum-storage-size advisories in
the new header and shared lifecycle header, plus throwing static-initialization
advisories for synthetic fixture constants. Those declarations follow the
lifecycle fixture's existing style; no shared files were changed. No analyzer
defect was reported. Separate new-file whitespace checks emitted no diagnostics.
