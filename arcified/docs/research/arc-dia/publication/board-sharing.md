<!-- arcified-backlog:board-sharing -->

Part of [Track creation and live integrations](https://github.com/Timpan4/helium-arcified/issues/10) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Publish a read-only board snapshot through the approved sharing service without implying live collaboration.

Type: implementation. Phase: Optional capability.

Disposition: Recommended integration or evidence slice. Improvements take precedence over literal replication.

## Acceptance criteria

- [ ] Preview supported board content and external media before the explicit publish action.
- [ ] Verify another browser can view the snapshot and owner revocation removes future service access.
- [ ] Authentication never transfers and later board edits remain private unless a new snapshot is explicitly published.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Decide hosting and access for revocable content sharing](https://github.com/Timpan4/helium-arcified/issues/22) (`shared-board`)
- [Export an editable research board and a visual snapshot](https://github.com/Timpan4/helium-arcified/issues/164) (`board-export`)

## Evidence and design intent

Direct reference sources below describe the workflow family; current native behavior and performance are unverified.

Arc/Dia supported desktop references as cited. Linux is an independent downstream verification target.

- [Arc: Easels Capture Create](https://resources.arc.net/hc/en-us/articles/19231142050071-Easels-Capture-Create)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Share Spaces Folders Splits with Anyone](https://resources.arc.net/hc/en-us/articles/19228534606743-Share-Spaces-Folders-Splits-with-Anyone)

## Open decisions and limits

Prove the named contract on applicable native platforms; retain unsupported and unavailable paths explicitly.

Proposed work after listed prerequisites and recorded owner decisions.
