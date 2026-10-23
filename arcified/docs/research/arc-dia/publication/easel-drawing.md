<!-- arcified-backlog:easel-drawing -->

Part of [Track creation and live integrations](https://github.com/Timpan4/helium-arcified/issues/10) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Extend a local board with original drawing and layout tools. The board-export slice owns portable export.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] Add/edit freehand marks and arrange text, images, video/link items with undo and non-pointer alternatives.
- [ ] Drawing and alignment preserve editable content, support undo and remain usable at different display scales.
- [ ] Handle missing media, scaling and offline access without losing editable board data.
- [ ] Keep embeds sandboxed and permissions scoped; revalidate source origin/selected region before refresh. Imported/changed content cannot silently execute or fetch another source.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Collect links, text and captures on a local visual board](https://github.com/Timpan4/helium-arcified/issues/86) (`easel`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Arc feature evidence is macOS-heavy or macOS-only as recorded in the dossier. Windows parity is not assumed; Linux is a downstream design target.

- [Arc: Easels Capture Create](https://resources.arc.net/hc/en-us/articles/19231142050071-Easels-Capture-Create)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
