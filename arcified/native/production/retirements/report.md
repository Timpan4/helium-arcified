# Retirement and service dispositions

Research packet for [issue 23](https://github.com/Timpan4/helium-arcified/issues/23), recorded 2026-10-24 against baseline `a4b6b480d3de07857ab6e939149254a8359e5449`.

All requested families have an explicit proposed disposition in [dispositions.json](dispositions.json). The owner has already excluded iCloud and new cross-device sync. Local import/export and migration require no Arcified account. Other proposals remain recommendations for coordinator consolidation, not invented owner approval. Native acceptance is **OPEN** on Linux, Windows and macOS.

## Authority and scope

The full assigned issue object and its control-manifest entry are retained in [assignment-snapshot.json](assignment-snapshot.json). [source-identities.sha256](source-identities.sha256) identifies the read authority, foundation documents and prepared-source files. Shared files can change after this observation; these identities do not claim an atomic snapshot of the root's active work.

The authoritative lifecycle contract remains version 3. Keep the existing Model, metadata/codec, page identities, immutable Profile ownership, private/download classifiers and real command targets. Same-window multi-Profile presentation and live saved-page handoff remain production gates. A synthetic `Model::OpenItem` result cannot pass them. This packet changes none of these systems.

Root owns lifecycle ownership/lifetime reports and integrated patch registration. The area's separate `focus-routing-contract` leaf owns focus-routing decisions. This retirement packet neither changes that contract nor supplies an alternative router.

The existing [foundation coverage matrix](../../../docs/research/arc-dia/coverage-matrix.md), [Arc dossier](../../../docs/research/arc-dia/arc-ux.md) and [artifact report](../../../docs/research/arc-dia/artifacts.md) were reviewed. Their recommendations, pure components and retained menu declarations are reused as background. They do not prove native implementation. In particular, static legacy Notes/Boost menu entries do not establish reachable or working features. No artifact download or reference-app execution was repeated.

## Primary-source facts

Retrieval date is 2026-10-24. These are documentary vendor statements, not runtime observations. The source IDs below resolve to exact URLs, retrieval routes, section locators and limitations in [sources.json](sources.json).

| Source | Supported finding |
| --- | --- |
| [ARC-NOTES](https://resources.arc.net/hc/en-us/articles/19233788518039-Phasing-Out-Arc-Notes) | macOS Notes phase-out began March 14, 2024. April 4 retained viewing without creating Notes; April 18 required export to view. Export is a ZIP containing HTML files. New Note can route to a configured external document service. No export fixture was inspected. |
| [ARC-2021](https://resources.arc.net/hc/en-us/articles/20498463803799-Arc-for-macOS-2021-Release-Notes) | October 27, 2021 introduced Reader Mode beta; a later annotation marks it disabled June 6, 2024. December 16, 2021 documented Notion/Figma/Linear Favorite previews for recent pages/common actions and People Cards on Twitter/LinkedIn/GitHub. The [historical rollup](https://start.arc.net/release-notes) corroborates these entries. Later People Cards availability is unsettled. |
| [ARC-MAC](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes) | February 29, 2024 removed the Recent Tab indicator. March 28 removed Paste & Go and Command Bar actions education, while retaining action search. May 23 deprecated GitHub Previews in favor of GitHub Live Folders. June 6 removed Reader Mode beta; June 20 directed users from iCloud Sync to Arc Sync. September 26 stopped automatic cross-device JavaScript Boost enablement. October 10 deprecated Legacy Boost Builder in favor of Create Boost. August 28, 2025 removed Ask on Page from Mac Max. These dates do not establish equivalent Windows removals. |
| [ARC-WIN](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes) | July 11, 2024 launched Ask on Page through Control-F, with settings controls. July 18 fixed subsequent queries after a blank query. These historical entries do not settle present Windows availability. |
| [ARC-SUNSET](https://resources.arc.net/hc/en-us/articles/44412972248471-Arc-Sunset-Key-Dates-and-What-to-Expect) | The announced schedule ends new accounts October 22, 2026, mobile AI/sign-in/sync November 23, data requests December 15, and desktop operation/accounts/services December 22. The publisher says account data is deleted within 30 days after deactivation. Arc Search iOS remains a browser; Android removal is announced for December 22. Mobile export produces link lists. Desktop users are told to migrate or save links before shutdown. None of these future events was observed. |
| [ARC-TO-DIA](https://resources.arc.net/hc/en-us/articles/44413129117591-Move-from-Arc-to-Dia) | Dia advertises installed-Arc import on macOS/Windows, requiring Arc to remain installed and be closed on Windows when its Cookies file is locked. Spaces become Dia Profiles; folders become pinned groups and nesting is flattened. It advertises bookmarks/history/passwords/cookies/extensions/open tabs, but excludes Easels, Boosts, payment methods and autofill. It says importing leaves Arc unchanged. This is not a portable Arc export schema or Arcified compatibility proof. |
| [DIA-CHANGELOG](https://www.diabrowser.com/changelog) | September 24, 2026, Mac 1.50.0 says Memory is being retired and existing data deleted. September 10 describes Microsoft work-tool connectors. December 4, 2025 describes creating documents, slide decks and tickets from the Command Bar. These are vendor features, not Arcified obligations. |
| [DIA-SECURITY](https://www.diabrowser.com/security) | The undated FAQ still describes server-generated Memory summaries stored locally and a disable setting. It also describes automatic Morning Brief context use. This conflicts with the dated Memory retirement announcement. Neither source proves the installed state or completed deletion. |

The initial full-slug Notes page returned 404, and initial sunset/migration opens failed. Article-ID routes recovered all three pages. These were retrieval failures, not proof of feature removal. Both attempted Max help routes remained 404. The missing page does not resolve Windows Ask on Page status. Publisher pages were read as parsed text; no raw HTTP source-byte identity is asserted.

## Proposed downstream outcomes

The table names surviving workflows. Each record in [dispositions.json](dispositions.json) also records migration limits, evidence IDs, reference-platform limits, related work and separate native states for Linux, Windows and macOS.

| Family | Disposition and surviving workflow |
| --- | --- |
| Notes | **Import.** Preserve legacy HTML and originals. Offer local view/export and an explicit external-document destination through the Notes leaf. A new native editor is outside this recommendation. |
| Reader Mode beta | **Replace.** Investigate Chromium's existing reading-mode path. Keep the original page available when unsupported. No universal page/PDF/private-page coverage is promised. |
| Paste & Go | **Replace.** Keep explicit paste/navigation and investigate reuse of the existing user-invoked native path. Add no passive clipboard collection. |
| Recent Tab indicator | **Replace.** Use accessible active status, recent switching and ordered selection. Indicator retirement does not retire recent navigation. |
| Command Bar education | **Replace.** Use discoverable verbs, keyboard hints and local onboarding. Unsupported commands remain unavailable. |
| GitHub Previews | **Replace.** Ordinary GitHub bookmarks/folders survive. Optional Live Folders require the separate named-provider decision. No provider, scopes or credentials are approved here. |
| iCloud sync | **Defer, explicitly excluded by owner.** Keep local Profiles, recovery and import/export. Do not implement iCloud or a new cross-device sync backend, or require an Arcified account. This is not a queued sync proposal. |
| Legacy Boost Builder | **Replace.** Prefer reversible declarative styling/zaps under the Boost leaves. Imported scripts remain inert until separate capability approval. Conversion and full legacy compatibility are unproven. |
| Ask on Page | **Defer to later AI scope.** Local find survives. Provider access and page-content use require separate consent and implementation. Keep the Windows uncertainty visible. |
| Arc sunset/migration | **Import.** Use a local preview/report workflow, observed formats and the existing metadata codec. Do not adopt Dia's mapping as Arcified policy. Unsupported data stays explicitly unmapped; source originals are preserved. |
| People Cards | **Defer.** Use the website's own profile pages/bookmarks. No enrichment service, identity match, scraping or provider policy is invented. Historical status is explicit; exact later retirement is unknown. |
| Historical Favorite-site previews | **Defer.** Notion, Figma and Linear remain ordinary favorites. Per-site recent-page/action integrations belong to issue 28 and provider policy. Generic folders, AI summaries and calendar previews do not satisfy this feature. |
| Dia Memory | **Defer.** Keep the conflicting documents separate. Ordinary local history controls survive; personal-summary collection is not part of this release recommendation. |
| Mobile Arc Search | **Defer, outside desktop scope.** No mobile-only AI, gestures or app parity is implied. User-provided link exports may be investigated locally without sync. |
| Dia work connectors/decks | **Defer, outside core non-AI scope.** Ordinary website use survives. Connectors, generated reports/decks, Morning Brief and enterprise administration require separate product decisions. |
| Unavailable vendor services | **Defer.** Retain local files/links where actually available, and report unavailable routes. No fabricated host, store, remote recovery or service replacement is supplied. |

## Native source leads and implementation boundary

The prepared tree is strictly read-only at `/home/timpan4/.t3/builds/arcified-linux-155-native-v1/src`. Its `chrome/VERSION` reports Chromium 155.0.8059.79. Exact full-file byte identities and equal observed base/result hashes are in [native-source-identities.json](native-source-identities.json).

| Prepared file and inspected location | What source inspection supports |
| --- | --- |
| `chrome/browser/ui/browser_actions.cc:435` through line 518 | Reading-mode ActionItems invoke `ReadAnythingEntryPointController` through a `BrowserWindowInterface*`. This is a reuse lead, not proof that the current Helium configuration exposes or correctly routes the feature. |
| `chrome/browser/ui/read_anything/read_anything_entry_point_controller.cc:1` through line 130 | The controller is real Chromium code and accesses Profile-owned preferences and tab-related interfaces. Later integration must audit service/feature availability and the real selected context. No universal distillation or offline claim follows. |
| `chrome/app/chrome_command_ids.h:261` and `chrome/browser/ui/actions/chrome_action_id.h:190` | Chromium already declares `IDC_PASTE_AND_GO` and its `kActionPasteAndGo` mapping. Reuse existing native behavior rather than assigning this retirement leaf's reservation to paste. Availability and invocation still need runtime checks. |

No native patch, GN registration, native test change or sparse-apply check is applicable to this research/decision leaf. It adds no runtime behavior. Creating a retirement command, service facade or synthetic test would not settle the research decisions or prove surviving workflows. Related implementation owners must supply actual patches, prepared-source base/result identities and meaningful registered native tests for their changes.

No command is allocated or modified here. If a future retirement-specific legacy command is justified, only reserved 40798 may be used, with an actual declarative ActionItem mapping. Library 40701 remains untouched. Other new actions must use ActionItems or obtain a coordinated ID. No strings are added; any eventual retirement-owned strings use `IDS_ARCIFIED_RETIREMENTS`. No new source code or vendor resources are copied.

## Questions for coordinator consolidation

These are recommendations to send through the coordinator, not owner answers.

1. Accept the local retirement package: Notes import/view/export without a new editor, Chromium reading-route investigation, explicit paste/navigation, active/recent navigation, discoverable local command help, and reversible declarative Boost styling? Recommended. The tradeoff is narrower legacy parity and explicit unsupported states.
2. Defer People Cards and historical Notion/Figma/Linear previews, while leaving any GitHub Live Folder or Favorite integration to issues 26 and 28's named-provider decisions? Recommended. Ordinary bookmarks/folders continue; authenticated enrichment waits for its own approved data/action scope.
3. Keep Ask on Page and Dia Memory in later AI scope, and keep mobile Arc Search and Dia work connectors/decks outside this desktop non-AI deliverable? Recommended. This avoids claiming unavailable vendor functionality and retains ordinary local find/history/site use.

Do not ask again whether to build sync. Its exclusion and account-free local migration are settled. Do not reopen activity/cleanup/destruction or populated cross-Profile migration decisions in this leaf.

## Acceptance and handoff

[verification-scenarios.json](verification-scenarios.json) covers every disposition with required observable success, failure/unsupported behavior, and explicit Linux/Windows/macOS states. All native execution and release states remain open. No build, native test, benchmark, browser, reference app, login or external service was executed.

The research inventory and supported documentary facts are complete for the assigned scope. Root acceptance of recommendations, inert owner-authorized export fixtures, import/schema mapping, actual native implementations, independent review and installed-platform results remain open. Fixture acquisition/mapping belongs to the relevant leaf, not an implied dependency blocking this research inventory.

[checks.md](checks.md) records exact light checks and their outcomes. [result.json](result.json) is the coordinator handoff. Only this packet was written. No root controls, shared ledgers, patches, series, staging, commits or remote state were changed.
