# Arc export format research

Issue: [27](https://github.com/Timpan4/helium-arcified/issues/27), `arc-import-format`.
Baseline: `a4b6b480d3de07857ab6e939149254a8359e5449`.
Research date: 2026-10-23 UTC, 2026-10-24 Europe/Stockholm.
Native acceptance: **OPEN**.

The official desktop export is documented as a Markdown link list. No owner-authorized export bytes were supplied or found in the scoped checkout inspection. Therefore this packet supports a documentary inventory and a proposed preview contract, but establishes no supported Arc build, export grammar, or converter. Fixture acquisition, byte-level field discovery and owner mapping decisions remain open.

## Authority and source identity

The complete assigned issue object and exact local file SHA-256 identities are retained in [sources.json](sources.json). The assignment and root workstream manifest are from the root control worktree `t3-a4238261`, not copied assumptions from a neighboring implementation packet. Root lists this research contract as `awaiting_evidence`, consumed by `arc-import`. The issue has no declared blocking implementation dependencies.

The root lifecycle contract is semantic version 3. Each Space chooses one real Profile; several Spaces may share it. Favorites, sign-ins and site settings are Profile-scoped. Themes are Space-scoped. Names and URLs do not establish identity. Same-window multi-Profile switching and existing live saved-page handoff are approved requirements whose native completion is still open. Cleanup, destructive actions and populated cross-Profile migration/source disposition remain undecided. This packet changes none of those authorities.

### Primary documentary findings

1. [How Do I Export My Data from Arc?](https://resources.arc.net/hc/en-us/articles/25583851606039-How-Do-I-Export-My-Data-from-Arc), revised 2026-10-22. macOS uses Arc settings, Advanced, Export Data. Windows uses settings, Advanced, Export Spaces and tabs, Export Data. The documented filename is `Arc Data Export.md`. Each Space has a section containing links for Favorites, pins, folders and other sidebar tabs. Arc describes manual reuse of these links rather than bookmark-file import. Folder clipboard export is a separate route; macOS offers Markdown with titles. Easels have a separate PNG route. Boost export is unavailable. This article supplies no sample bytes, parser grammar, explicit format version or numeric resource limits. Its accidental `http://Export.md` link is not a fixture download and was not followed.
2. [Move from Arc to Dia](https://resources.arc.net/hc/en-us/articles/44413129117591-Move-from-Arc-to-Dia), revised 2026-10-22. Dia discovers data through the installed Arc app; no portable export is required. The vendor describes macOS and Windows imports, including cookies and other browsing data. Spaces become Dia Profiles, Favorites become icon-only pins, and nested folders flatten into separate Tab Groups. Spaces from one Arc Profile retain shared browsing data; distinct Arc Profiles stay separate. Custom names are retained. Easels, Boosts, payment methods and autofill do not transfer. The process is documented as copying Arc data. These are vendor claims about Dia's direct importer, not byte-level observations or an Arcified capability.
3. [Arc Sunset: Key Dates and What to Expect](https://resources.arc.net/hc/en-us/articles/44412972248471-Arc-Sunset-Key-Dates-and-What-to-Expect), revised 2026-10-22. The vendor names December 22, 2026 as the desktop access cutoff. It distinguishes locally stored browsing data from account-data requests and describes mobile open-tab export separately. This creates an acquisition dependency, not authorization to run Arc, sign in, request account data or change a live installation.

The public Help Center JSON API returned the primary articles successfully. The original import-into-Arc citation returned 404. HTML retrieval was inaccessible or challenged, so no page rendering is claimed. API retrieval URLs, article revisions, byte lengths, response hashes and limitations are recorded in `sources.json`. Export article body SHA-256 was independently repeated during this investigation. Full copyrighted article bodies are not redistributed here. Third-party search results were discovery leads only.

## Corpus and observed fields

[fixtures.json](fixtures.json) records an empty corpus honestly. No original or derivative export is present, no application was run, and no export was changed. The existing extension `prototype_migration/fixtures/snapshots.json` contains AppState snapshots; it is not Arc export evidence. Prior macOS bundle identities in `artifacts.md` establish shipped artifacts only. Their version numbers cannot be attached to an export that was never obtained.

| Family | Documented portable category | Actual export observation and limits |
| --- | --- | --- |
| Spaces | Space sections | No observed heading syntax, IDs, order, empty-Space behavior, colors or theme fields. |
| Favorites | Link entries | No observed category syntax, shared identity, Profile association or custom-name encoding. |
| Pins | Link entries | Saved destination versus current navigation is unknown. No observed ID, title grammar or duplicate handling. |
| Folders | Included in link-list description | Nesting, folder IDs, empty folders and order are unobserved. Dia's flattening rule does not prove export structure. |
| Other sidebar tabs | Link entries | Temporary versus saved, window association and open/closed status are unobserved. |
| Profiles | No portable structure specified | Display names or section positions cannot establish real security/storage ownership. |
| Splits | No portable fields specified | Membership, pane order, orientation, ratio and focus are unobserved. Do not reconstruct splits from adjacency. |
| Sessions | No portable fields specified | No observed page/window identity, history, selection, scroll, forms or restore state. Link import cannot promise live-session recovery. |
| Format and limits | Markdown filename and link-list workflow | Encoding, dialect, escaping, format marker and supported Arc builds remain unknown. No parser bound or numeric quota is proposed. |

"Not specified" means documentary silence, not proof a field is absent from every exporter version. Byte-level field names and version support are intentionally empty. An owner's later fixture must be inspected before any field is called supported or absent in that specific version.

## Proposed mapping and preview contract

This is a recommendation for owner review, not an approved schema or implemented importer. Keep the conversion route unavailable until fixture evidence supports its exact input grammar. A chosen file is data only; it must never be executed or used to discover additional local files or accounts.

1. Establish source provenance first. Retain the untouched original with owner-controlled custody. Record explicit inspection/retention authority, Arc version/build, OS, export route, date, locale if known, byte length and SHA-256. Keep unknown values unknown. Built-in desktop export, clipboard Markdown, mobile lists and installed-browser files are separate source kinds. Each redacted derivative needs its own identity and transformation record. Prefer an owner-produced export containing synthetic browsing data so an inert original can be retained without publishing personal links.
2. Parse without native mutation or network access. Inventory the exact source constructs and unrecognized sections, with positions and field-presence states. Preserve labels, source order, duplicates and empty structures where actually observed. Never infer missing metadata, fetch link targets or favicons, or silently turn malformed structure into a successful partial import. Report unknown grammar or version as unsupported. Resource constraints need the existing project contract or measured evidence before values are chosen.
3. Preview every mapping and omission. Show source identity, observed source location, category, target Profile/Space, proposed destination and outcome. Reuse the existing `mapped`, `new`, `duplicate`, `conflict` and `unmapped` planning vocabulary. An ambiguous Profile stays unresolved until the owner chooses it; equal names or URLs never justify merging identities. Escape labels and expose unknown/unrecognized source content as inert text. Logs should contain structural diagnostics rather than sensitive URLs or imported labels.
4. Propose Space sections as new Spaces with explicit target Profile assignments. Do not copy Dia's Space-to-Profile presentation rule into Arcified's different ownership model. Propose confirmed pins as closed saved destinations kept in their Space. Map confirmed Favorites to Profile shortcuts only when Profile ownership and any shared identity are established or explicitly resolved. Do not deduplicate repeated Favorites across Spaces by URL. Folder ancestry and order require observed syntax and a coordinated organization representation; they must remain visibly unmapped if the destination cannot preserve them.
5. Keep other sidebar links unresolved until their category and intended treatment are established. Offer a reviewed saved-link conversion only if the owner chooses it. Do not fabricate temporary tabs or native pages from a list. Splits and session state stay unsupported unless future fixtures demonstrate the required fields and the target has a proven representation. Imported provenance labels do not introduce a new lifecycle state.
6. Make cancellation side-effect free and source-preserving. Preview is not a transaction token, URL approval or reservation. Revalidate destination state, explicit mapping and native URL policy before any later apply. A durable coordinator-owned import transaction must define failure/restart/re-import behavior before native writes are enabled. Keep a retained source-to-destination association for repeat imports; a content digest identifies a particular input, not object equivalence across re-exports. No source cleanup, browser shutdown or credential migration follows from accepting a preview.

The portable link route makes no credential, cookie, history, extension, autofill, payment, downloaded-file or live-session promise. These are unavailable capabilities in this proposal, not claims that every one is absent from unseen bytes. Easel PNG and unavailable Boost export are separate documented outcomes. Do not offer Dia's GitHub Live Folder service behavior from a link alone or connect an account through this leaf.

## Reuse and integration requests

Source inspection read the existing `portability/format.h`, `workspace_import/planner.h` and `prototype_migration/converter.h`. Portable format version 1 and lifecycle contract version 3 are Arcified contracts, not Arc export version labels. The portable DTO already carries names, record order and provenance, but it contains no folder tree, split or session payload. The planner exposes metadata-omission notices and calls its manifest a pure preview. The prototype converter accepts explicitly supplied extension AppState JSON. None parses the documented Arc Markdown export, persists a native import, or proves native behavior. Their pure suites were not rerun.

1. Coordinator obtains the authorized inert export corpus and consolidates owner answers. Use the intake requirements in `fixtures.json`; keep originals unchanged. Do not reuse private installed-browser files or bundle resources as portable fixtures.
2. Data area's `arc-import` consumer must keep supported input versions empty and route unavailable until this source contract is backed by real fixtures. Share this packet's distinction between direct import and portable export. A Markdown file extension alone must not enable conversion.
3. Data/core coordinators decide how verified folder hierarchy/order and locally retained import associations compose with existing metadata, codec, organization and native session-durability services. Request source-backed interfaces rather than another Model, codec, host, service or store. Preserve private/download classifiers. Unsupported routes remain unavailable.
4. After the evidence and owner decision gates, the implementation owner registers meaningful parser and native transaction tests in actual project targets. Retain fixture provenance and test malformed/unknown grammar, duplicate labels/URLs, missing Profile mapping, nested/empty folders, cancellation, destination changes, partial failure, repeat import and restart without source changes. Real native acceptance must also preserve same-window multi-Profile isolation and live saved-page handoff; these gates are not closed by a successful pure import plan.

## Decisions for coordinator consolidation

1. "Which Arc OS/builds and export routes should the first supported corpus cover, and can you provide owner-authorized inert exports with their provenance?" Recommendation: start with built-in desktop Export Data for every intended source platform/build; treat clipboard and mobile output separately. Tradeoff: narrower initial coverage, with no undocumented format promise.
2. "Once those fixtures are inspected, should the first converter support a preview of proven link/Space organization with explicit Profile mapping, while leaving unproven split, session and credential data unavailable, or should conversion remain deferred until richer export evidence exists?" Recommendation: accept the narrower preview contract after fixture review. Tradeoff: useful saved-link recovery with visible omissions rather than native-session restoration. This recommendation does not approve flattening folders, assigning missing Profile ownership or discarding source data.

No answer is recorded on the owner's behalf. Do not reopen the root's settled Profile/Space ownership choices or infer cleanup/source-disposition policy from import approval.

## Acceptance and verification limits

| Criterion | State and evidence |
| --- | --- |
| Retain provenance and inert fixtures; distinguish Dia direct import | PARTIAL. Primary source identities and route distinction recorded. Actual export provenance and inert fixtures await authorized inputs. |
| Enumerate observed fields without invented structure | PARTIAL. All requested families inventoried, with documentary categories separated from unobserved bytes. Exact fields and versions remain open. |
| Unsupported data and mapping/preview contract for review; unchanged originals | PROPOSAL DELIVERED. Unsupported/unsettled data and owner questions recorded. No originals supplied or changed; owner approval remains open. |

No Chromium base/result source file, production patch, ActionItem, message resource or native test registration applies to this research-only delivery. Command 40705 stays reserved and Library 40701 is untouched. Adding a stub converter or fabricated native test would not satisfy the missing evidence. The prepared Chromium source tree was not accessed or modified. No native setup, preparation, compilation, tests, benchmarks, remote CI, browser or visual execution ran. Linux, Windows and macOS native import acceptance are all OPEN.

Focused packet checks and exact outcomes are recorded in [checks.md](checks.md). Those checks establish document/JSON consistency and source identity, not a working converter or native acceptance.
