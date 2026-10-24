<!-- arcified-backlog:prototype-migration -->

Part of [Track data migration and privacy](https://github.com/Timpan4/helium-arcified/issues/5) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Move a user's existing extension snapshots into the native browser through an explicit export/import path.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Map names, URL order and available timestamps while explaining that snapshots contain no live sessions.
- [ ] Keep original extension storage and export intact until the user verifies the import.
- [ ] Cover malformed values, duplicate names, unavailable extension data and repeat import without silent replacement.
- [ ] Treat imported archives, HTML, URLs, CSS and attachments as untrusted and inert. Enforce path/origin containment, no automatic scripts/remote fetches and transactional failure recovery.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Recover the native workspace after an interrupted write](https://github.com/Timpan4/helium-arcified/issues/50) (`session-durability`)

## Evidence and design intent

The linked fork source is baseline evidence. This issue proposes native integration or a design contract; it does not claim reference-app execution.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
