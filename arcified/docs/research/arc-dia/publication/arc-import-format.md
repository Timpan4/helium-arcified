<!-- arcified-backlog:arc-import-format -->

Part of [Track reference evidence and release coverage](https://github.com/Timpan4/helium-arcified/issues/15) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Inspect owner-authorized Arc exports and document versions, supported fields and limits before promising a converter.

Type: research. Phase: Evidence, decisions and gates.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Retain export provenance and inert fixtures; distinguish portable exports from Dia's direct installed-browser import.
- [ ] Enumerate observed Spaces/Favorites/Profile/split/session fields without inventing absent structures.
- [ ] Record unsupported data and a proposed mapping/preview contract for owner review; keep originals unchanged.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Close with the named decision/evidence artifact and scenario coverage. Owner decisions are recorded in the issue. Unknown native behavior and unresolved service choices remain visible.

## Prerequisites

No blocking implementation issue. Record any named owner decision before enabling optional behavior.

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Import Bookmarks Logins History Extensions from Your Previous Browser](https://resources.arc.net/hc/en-us/articles/19335089616791-Import-Bookmarks-Logins-History-Extensions-from-Your-Previous-Browser)
- [Arc: Move from Arc to Dia](https://resources.arc.net/hc/en-us/articles/44413129117591-Move-from-Arc-to-Dia)
- [Arc: Arc Sunset Key Dates and What to Expect](https://resources.arc.net/hc/en-us/articles/44412972248471-Arc-Sunset-Key-Dates-and-What-to-Expect)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
