<!-- arcified-backlog:recent-files -->

Part of [Track library, files and capture](https://github.com/Timpan4/helium-arcified/issues/8) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Expose recent files on demand for quick opening and drag-out to other apps.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] List only accessible recent files and let users choose which supported types appear.
- [ ] Drag-out, copy, open and reveal work through each OS's native file integration.
- [ ] Missing/moved/deleted files and denied access never produce a misleading successful action.
- [ ] Do not present partial, quarantined or blocked files as completed and safe; distinguish remove-reference from real-file deletion.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Manage downloads with clear progress and recovery](https://github.com/Timpan4/helium-arcified/issues/36) (`downloads`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Arc feature evidence is macOS-heavy or macOS-only as recorded in the dossier. Windows parity is not assumed; Linux is a downstream design target.

- [Arc: Library A home for your downloads archived tabs easels and more](https://resources.arc.net/hc/en-us/articles/19230634389911-Library-A-home-for-your-downloads-archived-tabs-easels-and-more)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
