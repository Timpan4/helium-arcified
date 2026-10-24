<!-- arcified-backlog:onboarding -->

Part of [Track appearance, input and onboarding](https://github.com/Timpan4/helium-arcified/issues/7) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Guide first run through local Profile/Space setup and optional import, default-browser and AI choices.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Reach ordinary browsing without signing into Arc, Dia, an Arcified account or an AI service.
- [ ] Offer optional import/default-browser/AI setup through their completed flows; skipping them reaches usable local browsing and leaves a later entry point.
- [ ] Show the native Space/pin/temporary-tab model through a short interactive task and preserve progress safely.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Create and switch live Spaces in the native sidebar](https://github.com/Timpan4/helium-arcified/issues/32) (`spaces`)
- [Attach Spaces to isolated browser Profiles](https://github.com/Timpan4/helium-arcified/issues/48) (`profiles`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Import Bookmarks Logins History Extensions from Your Previous Browser](https://resources.arc.net/hc/en-us/articles/19335089616791-Import-Bookmarks-Logins-History-Extensions-from-Your-Previous-Browser)
- [Arc: Move from Arc to Dia](https://resources.arc.net/hc/en-us/articles/44413129117591-Move-from-Arc-to-Dia)
- [Arc: Arc Sunset Key Dates and What to Expect](https://resources.arc.net/hc/en-us/articles/44412972248471-Arc-Sunset-Key-Dates-and-What-to-Expect)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
