# Arc import format independent review, round 1

Review date: 2026-10-24, Europe/Stockholm.
Issue: [27](https://github.com/Timpan4/helium-arcified/issues/27), `arc-import-format`.
Baseline: `a4b6b480d3de07857ab6e939149254a8359e5449`.
Disposition: **CLEAN within the research scope. No actionable findings.**

This review accepts the documentary inventory and qualified proposal. It does not close issue 27, establish a supported export format, approve an owner decision, or prove a converter. Native acceptance remains **OPEN** on Linux, Windows and macOS.

## Reviewed input

All six packet files were read through `fast_filesystem`. Independent `sha256sum` output matched every frozen review identity:

| File under `arcified/native/production/arc-import-format/` | SHA-256 |
| --- | --- |
| `checks.md` | `b11fc280ff0379b2914321ebddfeda7f747bb9afb6e2a019a340b9e52c787414` |
| `fixtures.json` | `d355134998f62d542c00399d54aa0cc10557c14d09e9f06bc629bd4891c0b978` |
| `ownership.json` | `1ecf09a7910ae279f4340e497aeb2c2f5b0580015352b3ace054efb834b595db` |
| `report.md` | `22b69f0edf84ee3ccb9b40a957c35e9df0e3a22ea98a17603d72ceaecf6cc39a` |
| `result.json` | `0228250b0b81a67e8de34b0b61ddb301c5a0061573ab964c36b4eae02d2321f9` |
| `sources.json` | `e7fede9eabba4f11c1b29449b8329f2462c761dd8600c01a74dc5a5bd61a9bc5` |

The full assigned issue object matches the authoritative `assignment.json`. The root workstream manifest identifies this research contract as `awaiting_evidence`, consumed by `arc-import`. The root lifecycle contract, native-space integration document and execution state were inspected. All twelve local file identities recorded in `sources.json.localSources` independently matched, including those root authorities and the inspected foundation headers.

## Findings

No actionable finding, severity or correction location.

The evidence supports these bounded conclusions:

1. **Documentary claims are supported.** The official [export article](https://resources.arc.net/hc/en-us/articles/25583851606039-How-Do-I-Export-My-Data-from-Arc), [Dia migration article](https://resources.arc.net/hc/en-us/articles/44413129117591-Move-from-Arc-to-Dia) and [sunset article](https://resources.arc.net/hc/en-us/articles/44412972248471-Arc-Sunset-Key-Dates-and-What-to-Expect) were retrieved independently through their public Help Center JSON API. Their revision timestamps, response lengths and response SHA-256 values match `sources.json`. The export body also matches its recorded 4229-byte identity and SHA-256 `ee474965451c44d9d3b961a3dafe7f8d075d4ed796966bc1cf51e93eecbad666`. These checks support `report.md` lines 18 through 22. They establish documentation identity, not exporter behavior. No full article body is copied into this review.
2. **Portable export and installed-browser import remain separate.** The article descriptions support the route distinction. Dia's direct import does not supply a portable schema, Profile mapping rule for Arcified, or credential-import capability. The proposal does not inherit Dia's folder flattening or shutdown workflow.
3. **Format uncertainty is explicit.** `fixtures.json` lines 5 through 9 contain empty original, derivative, build, version and grammar lists. Every field family has `byteObserved: false`. `report.md` lines 28 through 40 distinguish documented categories from unobserved syntax and explicitly distinguish documentary silence from proven absence. The extension AppState fixture was inspected and correctly excluded from Arc exporter evidence. No Arc build is inferred from a bundle identity or article revision.
4. **The preview proposal preserves source and authority.** `report.md` lines 44 through 53 qualify the entire contract as a recommendation. Original custody, byte identity, derivative provenance, inert parsing, visible omissions, explicit Profile assignment, unresolved folder representation, destination revalidation and cancellation are covered. Source order, labels and duplicate entries are not used to invent identity or silently discard data. Unknown formats keep the route unavailable. No destructive, source-disposition or credential policy is approved.
5. **Lifecycle and foundation reuse are accurately bounded.** The inspected `portability/format.h`, `workspace_import/planner.h` and `prototype_migration/converter.h` support the description at `report.md` line 57. The proposal preserves real Profile ownership, Profile-scoped Favorites, distinct saved destinations and live pages, existing planning outcomes, metadata-omission reporting and the pure-preview boundary. Lines 61 and 62 retain metadata/codec and classifier integration, same-window multi-Profile isolation and live saved-page handoff as coordinated gates. No competing Model, codec, host, service or store is introduced.

## Acceptance coverage

| Assigned criterion | Review conclusion |
| --- | --- |
| Retain export provenance and inert fixtures; distinguish portable exports from Dia direct import | Route distinction and provenance intake are supported. Actual export provenance and fixtures remain OPEN because no original exists in this packet. |
| Enumerate observed Spaces/Favorites/Profile/split/session fields without inventing absent structures | All requested families are covered. Documentary categories are separated from byte observations. Exact fields, grammar, versions and builds remain OPEN. |
| Record unsupported data and a proposed mapping/preview contract for owner review; keep originals unchanged | Proposal and unsupported/unsettled data are delivered. No original was supplied or modified. Owner review remains OPEN. |

`result.json` accurately retains these remaining criteria and per-platform OPEN states. Missing exports, runtime proof and owner decisions are explicit verification limits, not defects in this research-only packet. Native base/result files, a production patch, ActionItems, message resources and registered native tests are inapplicable to this delivery. The packet makes no native-completion claim.

## Checks and limits

1. `fast_filesystem` read and packet inventory checks completed without truncation or file errors. The inventory contains exactly the six frozen packet files.
2. `sha256sum` over the six packet files passed against the supplied frozen hashes. `sha256sum` over every path in `sources.json.localSources` passed against all twelve recorded identities.
3. Independent primary retrieval used `curl -fsS` for these exact URLs, with a draining Node reader to parse the article, calculate SHA-256 and inspect documentary text:
   - `https://resources.arc.net/api/v2/help_center/en-us/articles/25583851606039.json`
   - `https://resources.arc.net/api/v2/help_center/en-us/articles/44413129117591.json`
   - `https://resources.arc.net/api/v2/help_center/en-us/articles/44412972248471.json`

   All commands exited 0. All response identities matched. The web tool first returned "is not accessible via this tool" for the API URLs; direct public HTTPS retrieval succeeded. No authenticated route or browser was used.
4. Primary/coordinator formatting evidence was reviewed and reused for unchanged frozen inputs. This review did not rerun foundation suites or treat document checks as implementation tests. The review report received a direct full-text LF, final-newline, trailing-whitespace and conflict-marker check through `fast_filesystem`.

No prepared Chromium source access or mutation, native setup, build, test, benchmark, remote CI, browser/visual execution, account action, live-host change, packet modification, staging, commit, publication, issue mutation or PR operation was performed. Only this requested review report was written.

## Coordinator handoff

Keep the `arc-import` converter unavailable and supported formats/builds empty. Route the packet's existing questions to the coordinator without recording an owner answer:

1. Which Arc OS/builds and export routes should the first supported corpus cover, and can the owner provide authorized inert exports with provenance? The recommendation to start with built-in desktop exports containing synthetic browsing data is adequately qualified; clipboard and mobile routes remain separate.
2. After fixture inspection, should the owner approve a preview of proven link/Space organization with explicit Profile mapping and visible unsupported fields, or defer conversion? The narrower-preview recommendation does not authorize inferred Profile ownership, folder flattening, session reconstruction, source removal or credential migration.

No correction is required for this round. Corpus acquisition, byte-level discovery, owner decisions and actual native acceptance remain unfinished.
