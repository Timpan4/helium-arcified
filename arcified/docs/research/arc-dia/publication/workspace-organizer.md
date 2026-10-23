<!-- arcified-backlog:workspace-organizer -->

Part of [Track spaces and tab lifecycle](https://github.com/Timpan4/helium-arcified/issues/4) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Show Profiles, Spaces, durable items and current window attachments without creating a second bookmark model.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Reveal ownership/provenance and identify unattached or inconsistent objects without exposing credentials.
- [ ] Preview cross-Profile reassignment using the approved reopen/deny transaction; cancel leaves identities unchanged.
- [ ] Block or stage referenced-Profile deletion and link ordinary items back to their sidebar location.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Settle native tab ownership and cleanup semantics](https://github.com/Timpan4/helium-arcified/issues/24) (`lifecycle-contract`)
- [Attach Spaces to isolated browser Profiles](https://github.com/Timpan4/helium-arcified/issues/48) (`profiles`)
- [Move tabs and Spaces between native windows](https://github.com/Timpan4/helium-arcified/issues/72) (`window-ownership`)
- [Reassign or delete a Profile without relabeling authenticated pages](https://github.com/Timpan4/helium-arcified/issues/103) (`profile-safety`)

## Evidence and design intent

YAB official manual/product documentation, reviewed 2026-10-23. Its inconsistent version labels and vendor size/performance claims do not establish runtime results.

YAB sources describe macOS/WebKit. Arc/Dia documentation contributes only where explicitly cited. Linux and Windows are independent downstream targets.

- [Arc: Profiles Separate Work Personal Browsing](https://resources.arc.net/hc/en-us/articles/19227964556183-Profiles-Separate-Work-Personal-Browsing)
- [YAB: manual / sidebar](https://yetanotherbrowser.com/manual/sidebar)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
