<!-- arcified-backlog:private-browsing -->

Part of [Track data migration and privacy](https://github.com/Timpan4/helium-arcified/issues/5) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Preserve Chromium private-browsing semantics alongside Spaces, Library and optional services.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Clearly distinguish private windows and prevent their tabs, captures, history and workspace edits from entering ordinary persistence, exports or optional remote capabilities.
- [ ] Define supported organization actions without silently converting a private page into a persistent pin or public share.
- [ ] Apply the isolation policy to Archive, AI history/context, integration caches and diagnostics. Deliberately saved downloads/files/clipboard/exports may persist and must not be promised erased at session end.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Attach Spaces to isolated browser Profiles](https://github.com/Timpan4/helium-arcified/issues/48) (`profiles`)
- [Settle native tab ownership and cleanup semantics](https://github.com/Timpan4/helium-arcified/issues/24) (`lifecycle-contract`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Profiles Separate Work Personal Browsing](https://resources.arc.net/hc/en-us/articles/19227964556183-Profiles-Separate-Work-Personal-Browsing)
- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
