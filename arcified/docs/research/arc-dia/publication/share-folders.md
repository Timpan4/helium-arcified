<!-- arcified-backlog:share-folders -->

Part of [Track creation and live integrations](https://github.com/Timpan4/helium-arcified/issues/10) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Export an inspectable portable snapshot of a Space, folder, split or selected tabs, retaining supported organization.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] Preview included URLs and custom names, remove sensitive links and explain what is not transferred.
- [ ] Produce a portable link bundle first; hosted sharing is a separate explicit service decision.
- [ ] Recipient import preserves supported order/organization and validates schemes without executing content.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Organize pinned tabs into nested sidebar folders](https://github.com/Timpan4/helium-arcified/issues/107) (`folders`)
- [Restore a saved split's layout and active pane](https://github.com/Timpan4/helium-arcified/issues/169) (`split-persistence`)
- [Export portable Arcified workspace data](https://github.com/Timpan4/helium-arcified/issues/73) (`export`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Arc feature evidence is macOS-heavy or macOS-only as recorded in the dossier. Windows parity is not assumed; Linux is a downstream design target.

- [Arc: Share Spaces Folders Splits with Anyone](https://resources.arc.net/hc/en-us/articles/19228534606743-Share-Spaces-Folders-Splits-with-Anyone)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
