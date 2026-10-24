<!-- arcified-backlog:schema-migrations -->

Part of [Track data migration and privacy](https://github.com/Timpan4/helium-arcified/issues/5) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Migrate supported on-disk workspace versions through an installed browser upgrade.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Use fixtures for each supported upgrade, corrupt data, interrupted migration and unknown future versions.
- [ ] Preserve the last valid original and expose recovery instead of overwriting data on failed migration.
- [ ] Document rollback/downgrade compatibility and verify a native upgrade preserves names, ordering and current selection.

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
