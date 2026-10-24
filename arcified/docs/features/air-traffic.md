# External-link routing

Status: reusable local C++ component for [issue 80](https://github.com/Timpan4/helium-arcified/issues/80), 2026-10-23. [RuleBook](../../native/features/air_traffic/routing.h) implements in-memory configuration and inspectable routing previews. It does not open links, implement a settings interface or satisfy installed native acceptance on Linux, Windows or macOS.

The [publication brief](../research/arc-dia/publication/air-traffic.md) and `air-traffic` entry in [backlog.json](../research/arc-dia/backlog.json) define the feature scope. [Lifecycle contract v3](../lifecycle-contract.md) owns Space/Profile identities and mapping. The component includes its authoritative header and reads the caller's `lifecycle::Model`; it has no second Space/Profile registry, page owner, current navigation, session store or persistence layer. Configuration contains typed destination references, not copies of lifecycle targets.

## Rules and precedence

Rules have a typed `RuleId`, a literal predicate and a destination. A destination references either a `SpaceId` or `ProfileId`, plus a main-window or lightweight-window surface. Main-window is the default surface. Each rule has one predicate; there is no implicit specificity score or hidden tie breaker.

1. Insert a rule at its zero-based precedence position. The first matching rule wins. `Move` changes that position; its argument is the final index in the resulting list.
2. Edit a rule by identity to retain its precedence. Erase it by identity to remove it. Empty rule IDs, empty literals, empty target IDs, duplicate rule IDs and invalid positions are refused without changing configuration. IDs borrowed from getters are safe command arguments because mutation inputs are copied before changes.
3. Set an explicit fallback destination for requests with no matching rule. No configured fallback produces `kNoFallback`. A failed selected rule never tries a lower rule or the fallback.
4. Pass a one-time destination override for an individual preview. It wins over rules and fallback, uses the same safety gates, and never changes configuration. The next request runs the configured rules again.

`PreviewExternal` returns all matching rules in precedence order, including shadowed rules and whether each target still exists. It also returns the selected candidate, its rule/fallback/override provenance, and a decision. This permits an eventual editor to explain overlapping rules before saving changes. The local component provides configuration commands and data for such an editor; there is no rendered editor or native preview UI yet.

## Browser authority and literal matching

The browser must authorize the exact external URL under its own URL, scheme and security policy, produce its canonical URL representation, classify its host, and supply trusted external-entry provenance. `BrowserFacts` carries those ephemeral inputs. Defaults are unknown and fail closed. An affirmative `NoHost` permits URL-only matching; an unknown host classification refuses routing. Empty canonical URL or canonical-host values are refused as incomplete inputs. These checks do not validate a URL.

Predicates compare either the caller's canonical host or canonical URL using case-sensitive byte equality (`kExact`) or a literal substring search (`kContains`). Exact host matching does not include subdomains. Contains can match any part of the chosen field, including a misleading host suffix or a URL path/query/fragment. It is an inspectable user routing preference, never site/origin classification, a domain-ownership check or a security permission. This component performs no parsing, decoding, normalization, public-suffix logic, DNS, network request or scheme guessing. The native editor must use browser canonicalization for exact rule values and clearly show the literal meaning of contains. No wildcard or regular-expression syntax is implied.

The fixture deliberately supplies host facts independently of URL text and verifies that matching trusts those facts. It does not establish how Chromium should validate or canonicalize URLs. The future browser adapter must derive the URL and host together from the same authoritative event. `kAllowed` is a caller assertion, not an authorization this library can confer. Do not log private URL facts or persist preview inputs as a second navigation record.

## Target resolution and private safety

Space destinations resolve through the current model's Space-to-Profile mapping on every preview. Profile destinations leave Space selection unspecified. Reassignment of an empty Space therefore changes the next routing choice to its current authoritative Profile. It never reprofiles a page. Missing selected targets produce `kTargetDeleted`, even if another rule or fallback could match.

The caller supplies current browser-authoritative privacy for the resolved Profile and capability availability for the chosen surface. Missing privacy callbacks and unknown privacy or capability produce explicit refusals. Unavailable capability also refuses. A main-window destination does not depend on lightweight support. A lightweight destination stays blocked until the caller affirmatively reports support, regardless of whether the external invocation entered through a main or lightweight path. Entry provenance does not rewrite the chosen destination surface.

Private targets from a rule, fallback or override return `PrivateConfirmationRequired`. They never return a directly usable regular-target decision or automatically fall back to another destination. This is a local request for explicit safe handling, not permission to launch private browsing. The component has no confirmation bypass, launch method, private persistence or private-window reuse policy. The caller must implement the approved private interaction, cancellation and browser policy before any launch, and revalidate the actual private context identity. A display label or original-Profile pointer cannot replace that identity.

`PruneDeletedTargets` explicitly removes rules whose referenced Space/Profile no longer exists and clears a deleted fallback. Its `PruneReport` lists removed rule IDs in former precedence order and includes the removed fallback destination. It receives a const model and cannot remove or change lifecycle targets. A preview itself never prunes. The caller must present the removal report and coordinate any future durable configuration update; this component provides neither persistence nor an event subscription. Pruning does not remove targets that merely have unknown privacy or unsupported capabilities. A stale override is refused for that request and is never stored. Lifecycle IDs must not be reused for new targets.

## Output and integration boundary

The coordinator approved this boundary during implementation: `ResolvedTarget { ProfileId, optional<SpaceId>, Surface }` is a routing choice, not a prepared attached target. The result contains no page identity, URL navigation record, window allocation or ownership transfer. It is an owned snapshot, not a reservation, launch token or proof that the target still exists.

The eventual caller/workspace integration must:

1. Retain the authoritative external event, honor its URL policy and present the routing explanation or one-time override through native UI.
2. Resolve a selected Space, or choose an appropriate Space for a Profile destination, then provision an actual `lifecycle::Attachment` through the workspace owner. Do not invent an attachment or select another Profile to make a route succeed.
3. Revalidate the exact event, target existence, current Space/Profile mapping, exact private context, privacy policy, capability and provisioned attachment before native actuation. Replan or report failure if they changed. This library supplies no asynchronous reconciliation or prepare/commit transaction.
4. Act through the browser's native navigation/security boundary and report only actual success. Failure or cancellation must not be reported as a created page or completed route.

Little Arc requires `{ProfileId, Attachment}` after this provisioning and revalidation. It consumes that already resolved target; it does not own another rule store or rerun a competing routing policy. The modules remain uncoupled. No shared interface, lifecycle, tooling, patch, workflow or execution catalog change is part of this slice.

Peek, Archive restore and other browser navigation return `kOutsideExternalEntry`, even with an override. Neither feature silently enters external routing. Saved-page outbound links retain their separate approved [lifecycle planner](../lifecycle-contract.md#navigation-and-transitions), which branches temporarily in the source page's current window/Space and immutable Profile. This component does not change that policy.

## Native source constraints and open acceptance

[Native Space integration](../native-space-integration.md) and its [pinned source references](../native-space-source-references.md) identify immutable desktop Browser/Profile ownership, native navigation/context boundaries, and Profile-local session ownership. In particular, references C1/C2 rule out claiming same-window multi-Profile support through a view adapter; C4 identifies the native navigation boundary. The integration design records an older v2 policy snapshot. Current approved mapping/settings and outbound-link decisions come from lifecycle v3 and the task brief, not that older snapshot.

Same-window multi-Profile native feasibility remains unproven and requires ownership extraction. This routing component neither implements nor bypasses that gate. It does not duplicate page ownership, transfer live saved pages, change authentication/site settings/Favorites scope, or store Space themes. Shared saved Space tabs, window-local temporaries and preserving the actual live page on saved selection remain lifecycle responsibilities.

| Scope | Evidence or remaining gap |
| --- | --- |
| Local configuration and preview | Focused C++ fixtures exercise create/edit/delete/reorder, exact/contains host and URL predicates, overlapping rules, explicit fallback, one-time override and no silent fallthrough. |
| Local safety | Fixtures cover unknown/rejected browser facts, excluded Peek/Archive provenance, current authoritative mapping, deleted targets and prune reports, private choices, and unknown/unavailable capabilities. |
| Native external handoff | OS default-browser entry, event provenance, Chromium URL/host authority, actual target provisioning and native navigation are not wired. |
| Native UI | Rule editor/preview, precedence explanation, removal notice, override and private-confirmation interactions need keyboard/focus/accessibility and visual evidence. |
| Durability and failures | Rule persistence, validation/migration at its future storage boundary, restart recovery, stale-event/cancellation races and failure reporting remain open. No rule capacity, timer or retry policy is introduced. |
| Platforms | No Chromium prep/build, installed native interaction or Linux/Windows/macOS acceptance ran. Local Linux compiler checks are not browser evidence. |
| Product policy | Activity/cleanup/destructive actions and populated cross-Profile migration remain OPEN. Private launch/reuse and lightweight lifecycle policy must come from their owners; a confirmation-needed result does not settle them. |

No real browsing data, providers, remote writes, accounts, sync or AI are used. This slice does not close issue 80.

## Focused checks

Run from the repository root with the existing C++17 toolchain. Driver mode follows the [lifecycle check instructions](../lifecycle-contract.md#synthetic-transition-fixtures-and-focused-checks). Only the new routing sources and the unchanged lifecycle implementation are linked. The executable uses synthetic IDs and reserved `.test` strings without starting a browser or accessing the network.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/air_traffic/routing.h \
  arcified/native/features/air_traffic/routing.cc \
  arcified/native/features/air_traffic/routing_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -Wswitch-enum -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/air_traffic/routing.cc \
  arcified/native/features/air_traffic/routing_test.cc \
  -o /tmp/arcified-air-traffic-test
/tmp/arcified-air-traffic-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -Wswitch-enum -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/air_traffic/routing.cc \
  arcified/native/features/air_traffic/routing_test.cc \
  -o /tmp/arcified-air-traffic-sanitized
/tmp/arcified-air-traffic-sanitized
clang-tidy --checks='-*,clang-analyzer-*' --warnings-as-errors='*' \
  arcified/native/features/air_traffic/routing.cc \
  arcified/native/features/air_traffic/routing_test.cc \
  -- -std=c++17 -Wall -Wextra -Wswitch-enum -pedantic
```

Recorded results: the normal and AddressSanitizer/UndefinedBehaviorSanitizer executables passed all focused routing fixtures. Chromium-style formatting and the named Clang static-analyzer checks passed with no diagnostics. Local documentation-link targets exist. These local checks establish only the behavior exercised above. There is no native browser or platform acceptance evidence.
