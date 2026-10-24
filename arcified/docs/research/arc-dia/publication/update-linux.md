<!-- arcified-backlog:update-linux -->

Part of [Track distribution and engine maintenance](https://github.com/Timpan4/helium-arcified/issues/14) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Implement the approved linux update channel with a data-preserving, authenticated failure path.

Type: implementation. Phase: Distribution.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] The fork owner approves channel/service ownership and signing/key infrastructure before any publication.
- [ ] Verify installed-to-updated artifact identity/signatures and reject tampered metadata or incompatible downgrades.
- [ ] Exercise interrupted/failed update, restart and the approved rollback behavior while retaining Profile/workspace data.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux.

Verify the affected interaction in an installed native build on Linux. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Install a Linux Arcified package safely](https://github.com/Timpan4/helium-arcified/issues/131) (`release-linux`)
- [Recover the native workspace after an interrupted write](https://github.com/Timpan4/helium-arcified/issues/50) (`session-durability`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)
- [Dia: security](https://www.diabrowser.com/security)
- [Dia: security / bulletins](https://www.diabrowser.com/security/bulletins)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
