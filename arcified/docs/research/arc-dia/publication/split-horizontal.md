<!-- arcified-backlog:split-horizontal -->

Part of [Track navigation and multitasking](https://github.com/Timpan4/helium-arcified/issues/6) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Create a horizontal split through a command, menu or drag target.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Each pane has independent navigation, focus and permissions while belonging to the intended Space/Profile.
- [ ] Resize, swap, focus, close and unsplit retain intended tab identity and surviving content.
- [ ] Keyboard and assistive technology can operate pane boundaries and determine which pane is active.
- [ ] Verify origin identity, permission bubbles and browser-owned dialogs remain bound to the intended page/pane through site-controls and security-surfaces.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Open and close temporary tabs within a Space](https://github.com/Timpan4/helium-arcified/issues/49) (`temporary-tabs`)
- [Define which page, pane or overlay receives a browser action](https://github.com/Timpan4/helium-arcified/issues/44) (`focus-routing-contract`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [start.arc.net master-multitasking](https://start.arc.net/master-multitasking)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)
- [Dia: changelog](https://www.diabrowser.com/changelog)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
