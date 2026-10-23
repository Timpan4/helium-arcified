<!-- arcified-backlog:release-linux -->

Part of [Track distribution and engine maintenance](https://github.com/Timpan4/helium-arcified/issues/14) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Distribute a distinct Linux package with user-data-preserving installation and a verified install path.

Type: implementation. Phase: Distribution.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Decide supported package/distribution format and fork-owned application identity before publishing.
- [ ] Verify clean install, upgrade and uninstall data choices on supported Linux environments.
- [ ] Report source provenance and native smoke checks; keep signing/release credentials out of artifacts.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux.

Verify the affected interaction in an installed native build on Linux. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Boot a downstream native Arcified build on Linux](https://github.com/Timpan4/helium-arcified/issues/16) (`native-linux`)
- [Recover the native workspace after an interrupted write](https://github.com/Timpan4/helium-arcified/issues/50) (`session-durability`)
- [Upgrade stored workspace schemas without losing the original](https://github.com/Timpan4/helium-arcified/issues/99) (`schema-migrations`)

## Evidence and design intent

The linked fork source is baseline evidence. This issue proposes native integration or a design contract; it does not claim reference-app execution.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
