<!-- arcified-backlog:command-bar -->

Part of [Track navigation and multitasking](https://github.com/Timpan4/helium-arcified/issues/6) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Open a native command bar for URLs, searches, tab switching and discoverable browser commands.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Keyboard invocation, typing, selection and Escape restore predictable focus and retain an abandoned query where appropriate.
- [ ] Clearly distinguish search, navigate, switch-to-tab and execute-command results with accessible labels.
- [ ] URL parsing, private browsing and commands with side effects follow existing browser trust boundaries.
- [ ] Rank contextual actions visibly and show what Enter will do. Opening or typing in the field does not itself send page text or read clipboard contents implicitly.
- [ ] Do not create a tab, history entry or workspace mutation until the user chooses an action; Escape restores the initiating surface.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Open and close temporary tabs within a Space](https://github.com/Timpan4/helium-arcified/issues/49) (`temporary-tabs`)

## Evidence and design intent

YAB official manual/product documentation, reviewed 2026-10-23. Its inconsistent version labels and vendor size/performance claims do not establish runtime results.

YAB sources describe macOS/WebKit. Arc/Dia documentation contributes only where explicitly cited. Linux and Windows are independent downstream targets.

- [start.arc.net command-bar-actions](https://start.arc.net/command-bar-actions)
- [Arc: Site Search Directly Search any Website](https://resources.arc.net/hc/en-us/articles/20855018192791-Site-Search-Directly-Search-any-Website)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)
- [Dia: changelog](https://www.diabrowser.com/changelog)
- [YAB: manual / the field](https://yetanotherbrowser.com/manual/the-field)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
