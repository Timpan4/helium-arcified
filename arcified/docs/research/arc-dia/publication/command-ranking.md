<!-- arcified-backlog:command-ranking -->

Part of [Track navigation and multitasking](https://github.com/Timpan4/helium-arcified/issues/6) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Search native open/pinned tabs with unambiguous switch-versus-open results.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Show source Space/Profile and whether a result switches to a live tab or opens its saved destination; exclude isolated/private data.
- [ ] Handle duplicate titles, renamed pins, long URLs and empty/loading results without selection jumping.
- [ ] Use a repeatable corpus to verify ranking and instrument result latency as the workspace grows.
- [ ] Use explicit Switch/Open saved destination verbs with Profile/Space/window provenance and a Reveal in Workspace action; duplicate titles do not hide action meaning.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Navigate and run browser actions from one command bar](https://github.com/Timpan4/helium-arcified/issues/77) (`command-bar`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [start.arc.net command-bar-actions](https://start.arc.net/command-bar-actions)
- [Arc: Site Search Directly Search any Website](https://resources.arc.net/hc/en-us/articles/20855018192791-Site-Search-Directly-Search-any-Website)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)
- [Dia: changelog](https://www.diabrowser.com/changelog)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
