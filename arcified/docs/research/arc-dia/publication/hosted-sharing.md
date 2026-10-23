<!-- arcified-backlog:hosted-sharing -->

Part of [Track creation and live integrations](https://github.com/Timpan4/helium-arcified/issues/10) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Using the approved service, publish one explicitly selected Space/folder/split/link snapshot and revoke it.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] Preview the included content, publish only the approved snapshot and view it from another browser without transferring authentication.
- [ ] Show snapshot versus live-edit semantics and verify revocation removes future service access. Already downloaded recipient copies cannot be recalled.
- [ ] Handle failed/ambiguous publish and repeated actions without duplicate public snapshots, using synthetic/public fixtures.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Decide hosting and access for revocable content sharing](https://github.com/Timpan4/helium-arcified/issues/22) (`shared-board`)
- [Share a folder or split as an inspectable set of links](https://github.com/Timpan4/helium-arcified/issues/172) (`share-folders`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Arc feature evidence is macOS-heavy or macOS-only as recorded in the dossier. Windows parity is not assumed; Linux is a downstream design target.

- [Arc: Share Spaces Folders Splits with Anyone](https://resources.arc.net/hc/en-us/articles/19228534606743-Share-Spaces-Folders-Splits-with-Anyone)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Easels Capture Create](https://resources.arc.net/hc/en-us/articles/19231142050071-Easels-Capture-Create)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
