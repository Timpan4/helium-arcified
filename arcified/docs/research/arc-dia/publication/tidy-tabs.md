<!-- arcified-backlog:tidy-tabs -->

Part of [Track optional intelligence](https://github.com/Timpan4/helium-arcified/issues/11) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Offer user-invoked grouping of selected temporary tabs with readable group names.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] Preview proposed groups and unaffected tabs; apply/reject individual changes.
- [ ] Preserve tab identity and manual organization; every applied organization operation is undoable.
- [ ] Use only explicitly selected metadata/context and keep private/Profile boundaries intact.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Organize pinned tabs into nested sidebar folders](https://github.com/Timpan4/helium-arcified/issues/107) (`folders`)
- [Choose and inspect @tab and @space context before sending](https://github.com/Timpan4/helium-arcified/issues/88) (`context-selector`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Arc Max Boost Your Browsing with AI](https://resources.arc.net/hc/en-us/articles/19335160678679-Arc-Max-Boost-Your-Browsing-with-AI)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)
- [Dia: changelog](https://www.diabrowser.com/changelog)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
