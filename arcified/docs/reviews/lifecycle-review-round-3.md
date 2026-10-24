# Native lifecycle v3 review, round 3

Result: clean within the standalone version 3 contract/model scope at checkpoint `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. No actionable implementation or code/contract mismatch was found. This accepts the reviewed ownership model and synthetic routing decision behavior. It does not establish native browser behavior or close issue 24.

Reviewed independently on 2026-10-23. Activity, cleanup, destructive actions, and populated cross-Profile migration/source disposition remain **OPEN**. The approved Profile/Space mapping, settings scopes, outbound-link branching, saved-page handoff, and same-window Space switching remain approved.

## Revision and coverage

Repository HEAD matched the requested checkpoint. `git diff` against that checkpoint was empty for all six reviewed artifacts below. All three files in `arcified/native/model/`, the entire contract, policy response, and round 2 report were read. The local issue body artifact was also read to check acceptance and closure boundaries.

| Artifact | Checkpoint Git blob |
| --- | --- |
| [lifecycle.h](../../native/model/lifecycle.h) | `d3a7aed6f1caddd40f5422bc6e5c1f5019636338` |
| [lifecycle.cc](../../native/model/lifecycle.cc) | `7ffe87eb49d0cf4a92f83058b7c348ed3882012f` |
| [lifecycle_test.cc](../../native/model/lifecycle_test.cc) | `af3a52f39d323e079b3799b306e1e756c9205893` |
| [Lifecycle contract](../lifecycle-contract.md) | `d6110f065107622b70a413ac9816d612f257fbca` |
| [Policy response](lifecycle-policy-response.md) | `68b877aad420feb15524788e5402bd905e2cc1a4` |
| [Round 2 review](lifecycle-review-round-2.md) | `fc6314003edb8b7b33c0b3e3f48b92835dc86b41` |

This report is the only repository write from this review. Implementation, supplied fixtures, previous reports, and unrelated working-tree files were left unchanged. No commit, remote write, PR linking/watching, full build, installation, or cleanup was performed.

## Code and contract assessment

Each `Space` stores exactly one existing `ProfileId`; multiple Spaces can share it. Profile-owned saved items and shortcut aliases are visible through attached Spaces in that Profile. Cross-Profile opening, alias creation, live-page transfer, and direct item movement remain refused. Reassignment requires an empty, unattached Space, so it cannot relabel a live page. The contract now records Favorites, site settings and sign-ins as Profile-scoped, and themes as Space-scoped. Settings, credentials and theme payloads are explicitly outside this identity model. Their absence does not contradict the scoped implementation claim.

`PlanSavedPageLink` is const and takes owned source/destination inputs. It authorizes a plan only for a saved live source, a nonempty validated destination, `NavigationKind::kLink`, and caller-supplied `SiteRelation::kOutbound`. It copies the source's immutable Profile and current attachment into an owned result. It does not register IDs, create pages, record navigation, change activity, transfer the source, or alter split references. Unknown/protected activity does not prevent planning because this operation leaves the source intact.

The planner never compares destination text with either saved or current navigation. Classification and navigation-kind provenance belong to the authoritative browser caller, relative to the current committed document. Same-site and unknown classification yield distinct refusals. Redirect, script, typed and other triggers remain outside the approved scope. Refusal does not authorize an in-place fallback or impose a browser-wide navigation block. Invalid enum values also failed closed in the supplemental tests.

Source attachment takes precedence over selected-Space presentation. Supplemental tests selected a Space in another Profile while planning from saved sources in the original Profile. Plans still used each source's actual attachment and Profile. Profile-owned Favorites also produced plans in their current Space and window, rather than an inferred owning Space.

The plan owns its strings and IDs. Tests retained it after source transfer, navigation, page erasure, and destruction of the originating model. Its old attachment and destination remained intact without sanitizer diagnostics; replanning used the new attachment. These results demonstrate snapshot lifetime, not freshness authorization. The contract correctly requires the native caller to revalidate source identity, current document, Profile, attachment, classification and model preconditions before actuation. `OpenTemporary` is a separate event-reporting command and does not validate a plan. No stale-plan transaction or redirect policy is implied.

The version 2 borrowed-ID repair remains present: mutating commands and private page/reference erasure helpers own command values before mutation. Saved-page handoff preserves page identity, navigation, activity and Profile while pruning old split references. Same-window Space selection changes selection without reassigning page Profiles. All supplied regressions for these paths passed under sanitizers in this round; the prior independently cleared P1 is not reopened.

## Focused verification

Ran on Linux with existing Clang and clang-format 23.1.3:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/model/lifecycle.h \
  arcified/native/model/lifecycle.cc \
  arcified/native/model/lifecycle_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc arcified/native/model/lifecycle_test.cc \
  -o /tmp/arcified-lifecycle-round-3-sanitized
/tmp/arcified-lifecycle-round-3-sanitized
```

Formatting passed. All 16 supplied v3 fixtures passed with warnings treated as errors, AddressSanitizer and UndefinedBehaviorSanitizer. No sanitizer diagnostic occurred. No separate unsanitized or Windows/macOS compiler run was performed in this review.

An independent supplemental harness was compiled from standard input with the same C++17 warning and sanitizer flags, linking `lifecycle.cc` and including its header. The executable is retained at `/tmp/arcified-lifecycle-round-3-adversarial`; no supplemental source file was written. It passed these checks:

1. A complete Cartesian matrix over the harness's declared inputs: Space-owned saved source, Profile-owned saved source, temporary source, missing source and empty source ID; every named navigation kind plus an invalid enum; every named site relation plus an invalid enum; empty/current-document/other validated target strings; and all three activity states. All 1,080 decisions matched the permitted success boundary. Exactly 12 returned plans. Each plan's source, Profile, attachment and destination matched the source and request. A snapshot of every field in all eight public state collections was identical before and after every decision, including refusals.
2. Getter-sourced source IDs and destination strings, owned result lifetime, transfer/navigation staleness, replanning, duplicate page/tab IDs and missing-attachment reporting failures. Each reporting refusal preserved the complete model snapshot. A separate successful temporary report used the original Profile and the refreshed attachment. Protected source navigation and saved destination remained distinct; close and Reset still refused protected work.
3. Shared-Profile Favorite/alias visibility, absence from the other Profile, and cross-Profile opening/alias/transfer/move/reassignment refusals. The complete snapshot remained unchanged across those refusals.

Observed supplemental output:

```text
PASS routing matrix 1080 decisions, 12 plans
PASS owned plan lifetime, transfer/navigation staleness and report refusals
PASS shared-Profile visibility and cross-Profile refusal snapshot
```

These are synthetic checks of the named cases, supported by inspection of every model command. They are not exhaustive command-sequence exploration or native failure/cancellation tests.

## Open policy and native boundaries

The contract and policy response preserve the owner's open safety decisions. Conservative activity refusals do not approve actual detection signals, automatic cleanup, destructive-action confirmation or overrides. URL-only copy/reopen retains originals and does not settle populated migration or source removal. Same-site routing, additional live instances, undo/restoration and transient/split presentation also remain unresolved where recorded. None was silently enabled by the planner.

No real Profile/BrowserContext, document identity, WebContents, renderer, authentication context or native window was exercised. URL validation, authoritative site classification, redirects, native preparation/actuation/reconciliation, asynchronous ordering, persistence and non-reused ID allocation remain adapter responsibilities. Source-document revalidation needs native evidence because the plan contains no document identity or commit token. The reviewed native design's future adapter must explicitly adopt v3.

Native saved-page transfer, same-native-window cross-Profile presentation and cookie/storage/sign-in isolation remain unverified on Linux, Windows and macOS. No browser or UI interaction was performed. These future integration checks do not turn already approved product decisions back into open questions, and a full Chromium build was not required for this scoped review. Issue 24 remains open for its outstanding owner choices and platform scenario evidence.
