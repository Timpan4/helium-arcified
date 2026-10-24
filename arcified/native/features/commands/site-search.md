# Site search intent core

Status: standalone C++ implementation for issue [110](https://github.com/Timpan4/helium-arcified/issues/110), based on checkpoint `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. Native integration and acceptance remain open on Linux, Windows and macOS. This core does not implement an omnibox, provider settings, a browser adapter or URL expansion.

The owner approved a native-snapshot, explicit-selection, discovery and submit-intent boundary in the commands workstream. [The publication brief](../../../docs/research/arc-dia/publication/site-search.md) and the `site-search` backlog entry remain the acceptance authority. [Lifecycle contract v3](../../../docs/lifecycle-contract.md) owns Profile and page semantics. The pinned [native ownership design](../../../docs/native-space-integration.md) and [source references](../../../docs/native-space-source-references.md) still gate native host integration. The core reuses only the lifecycle identity type, never page ownership, persistence or current navigation.

## Minimal contract

1. The native adapter supplies `SnapshotData` for the actual Profile, including a caller-allocated `SnapshotRevision`, service readiness, eligible provider IDs, actual native labels, shortcuts and optional default provider ID. `ProviderSnapshot::FromNative` checks structural integrity and returns an immutable projection. This is not native URL or policy validation.
2. `Discover(prefix)` returns providers in supplied native order, with actual label, shortcut and default marker. An empty prefix lists every supplied provider. A nonempty prefix compares literal UTF-16 shortcut code units. Discovery does not choose, suggest query completions, normalize keywords, classify URLs or emit an intent. Duplicate labels/shortcuts remain distinct choices by provider ID.
3. After an explicit user choice, `Select(ProviderId, Input)` or `Select(DefaultProvider{}, Input)` captures Profile, revision, selected target, actual provider metadata and the exact query. `Selection` exposes this owned preview through const accessors. An empty query or active composition refuses selection. The default target resolves to the actual provider before preview; it is never shown as an unlabeled generic destination.
4. After explicit submission, `Submit(optional_selection, current_input)` checks service readiness, composition, presence of selection, exact Profile and revision, the selected default/provider identity and metadata, and the exact query. A mismatch returns a typed refusal. A successful `SearchIntent` contains Profile, revision, actual provider metadata and literal query only. It acknowledges no native navigation. No URL, template or POST body is represented here.
5. Cancel drops the caller's optional selection. Query edits require a fresh explicit selection/preview. Refresh replaces the entire snapshot from the authoritative native service. No method mutates providers, selects a new default or persists anything.

The adapter must allocate a fresh, never-reused revision for **every snapshot refresh**, including refreshes whose visible provider rows are unchanged, service recreation, policy/eligibility changes, native template edits and Profile/context changes. There is no local counter, timer, allocator or revision default in this core. Provider IDs bind to actual native provider identities within the actual Profile/service lifecycle; they must not be titles, shortcuts, URLs or reused row indices. The native adapter retains the exact Profile/OTR binding, without inventing an OTR persistence policy.

The caller must pass the latest authoritative snapshot to `Submit`. An old snapshot cannot detect that its source service changed. Submission additionally detects visible default, provider label/shortcut and Profile changes even if a caller mistakenly reuses a revision. It cannot detect an invisible template/policy change when the revision contract is violated. The returned intent is an owned snapshot, not a reservation, one-shot token or asynchronous completion protocol. The adapter must revalidate actual native state immediately before expansion/actuation and serialize explicit submissions through the real command path. It must not call submission from typing, highlighting, discovery, IME updates or snapshot refresh.

Literal queries preserve UTF-16, combining characters, non-Latin text, whitespace, punctuation, emoji and URL-looking text. No shortcut is stripped from a query, and no escaping or Unicode normalization runs here. The API accepts an already explicit provider/query mode. The native omnibox owns classification and query extraction for ordinary unscoped text. This prevents a local heuristic from treating a URL as a query or redirecting ordinary input merely because it contains a shortcut.

## Discovery without a native service

There are no built-in fallback engines or configuration files. A loading, unavailable or blocked service produces a distinct refusal from discovery, selection and submission. Such a snapshot must contain no provider rows/default. A ready service may legitimately contain no providers or no default; explicit providers can still be chosen when no default exists.

Synthetic snapshots make this boundary independently useful: tests exercise listing, literal shortcut-prefix discovery, actual labels, default marking and selecting the actual default, followed by native-authoritative add/edit/remove/default refreshes. These fixtures are not production search configuration. A future native keyboard UI can enumerate those labeled choices and use native platform bindings; focus movement alone must never submit. This core supplies no guessed Tab/Space modifier rule, key mapping, accessible announcement or native UI proof.

## Pinned source reuse

Chromium source was read through small read-only Gitiles file fetches at engine commit `a49e2e3d4addfd84e765b52afe230bedb81096db`, corresponding to the repository's `155.0.8059.79` pin. No prepared tree, native build or runtime behavior is claimed.

| Native authority | Observed source and adapter consequence |
| --- | --- |
| Provider storage, CRUD and readiness | [TemplateURLService header](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/search_engines/template_url_service.h) explicitly owns mutations and KeywordWebDataService persistence. Reuse `Add`, `ResetTemplateURL`, `Remove`, readiness and change observers. Do not create another provider store. Observe load/service/policy/context changes and publish fresh snapshots. |
| Default and managed settings | The same header exposes `GetDefaultSearchProvider`, `CanMakeDefault`, `SetUserSelectedDefaultSearchProvider` and `is_default_search_managed`. Default may be null, including when disabled by policy. Native settings validation and policy checks must own all add/edit/remove/default changes. The core only reconciles their resulting snapshots. |
| Provider identity and discovery | [TemplateURL](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/search_engines/template_url.h#787) exposes native names and keywords, `GetFullName` supplies keyword-chip text, and `CanBeUsedForKeywordMatching` describes native eligibility. TemplateURLService owns `AddMatchingKeywords` and `GetTemplateURLForKeyword`. Use their authoritative matching/conflict rules for native input, rather than treating this local literal-prefix list as an omnibox replacement. |
| Real URL expansion | [TemplateURLRef](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/search_engines/template_url.h#332) owns `SupportsReplacement`, `SearchTermsArgs` and `ReplaceSearchTerms`, including provider input encoding and POST content. Reuse the real Profile service's `SearchTermsData` and native expansion/navigation path. No hand-built query URL, percent-encoding engine or GET-only assumption belongs here. |
| Helium native shortcuts and controls | [add-native-bangs.patch](../../../../patches/helium/core/add-native-bangs.patch) already feeds bang providers into TemplateURLService, handles native labels/keywords and hides bang entries from ordinary engine lists. Its fetch path consults `ShouldFetchBangs`, uses cache-only loading when disallowed, and omits credentials. Reuse that authoritative source and its controls; do not fetch another manifest or expose a hidden/disabled/AI provider by blindly copying every service row. |
| Native remote suggestions | [SearchProvider](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/omnibox/browser/search_provider.cc#970) documents its OTR suggestion boundary before entering the native remote suggestions service. The standalone core has no networking, suggestions, preconnect, history writes or page-content access. Actual native privacy/suggestion control enforcement remains an adapter acceptance requirement. |

Source observations establish reuse boundaries only. In particular, the Helium patch was read, not applied or executed. Its observer, eligibility and service-lifetime behavior must be checked in the prepared native integration. This work accessed no browsing data or AI provider.

## Focused synthetic checks

[site_search_test.cc](site_search_test.cc) drives the public snapshot/discovery/selection/submission API. It covers malformed projections, distinct identities despite duplicate names/shortcuts, native ordering and default discovery, explicit selection, missing choices/defaults/providers, literal multilingual/URL-looking queries, active IME composition, exact query edits, native-authoritative provider/default changes, revision and Profile invalidation, unavailable/blocked service refusals, and owned metadata/query lifetime. Assertions compare concrete expected outcomes rather than exercising a mock native facade.

Run from the repository root with the existing local toolchain. The explicit Clang driver mode follows the existing lifecycle check instructions.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/commands/site_search.h \
  arcified/native/features/commands/site_search.cc \
  arcified/native/features/commands/site_search_test.cc
clang-tidy arcified/native/features/commands/site_search.cc \
  arcified/native/features/commands/site_search_test.cc \
  --checks='clang-analyzer-core.*,clang-analyzer-cplusplus.*,bugprone-use-after-move' \
  --warnings-as-errors='*' \
  -- -std=c++17 -Wall -Wextra -Werror -pedantic
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/commands/site_search.cc \
  arcified/native/features/commands/site_search_test.cc \
  -o /tmp/arcified-commands-site-search-test
/tmp/arcified-commands-site-search-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/features/commands/site_search.cc \
  arcified/native/features/commands/site_search_test.cc \
  -o /tmp/arcified-commands-site-search-sanitized
/tmp/arcified-commands-site-search-sanitized
```

On 2026-10-23, the format check, explicit clang-tidy checks, strict Clang C++17 executable and ASan/UBSan executable above passed on the local Linux host. The executables reported `site-search synthetic checks passed`. The host's default clang-tidy invocation enabled no checks; the documented invocation explicitly selects the analyzer and use-after-move checks. New-file `git diff --no-index --check /dev/null` checks emitted no whitespace diagnostics for each owned file; exit status 1 denotes the new-file differences. Local documentation link targets were checked with filesystem metadata.

These checks prove standalone decisions over synthetic snapshots. They do not prove actual CRUD, TemplateURLRef expansion, navigation, privacy controls or native UI acceptance. All remain blocked on the native adapter and permitted native integration work. Linux, Windows and macOS installed-build behavior, persistence/failure cases, keyboard/IME interaction, accessibility and visual evidence remain open. No root execution catalog, build/tooling/workflow, shared lifecycle, patch series or extension change is part of this slice.
