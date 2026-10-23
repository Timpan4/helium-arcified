<!-- arcified-backlog:little-arc -->

Part of [Track navigation and multitasking](https://github.com/Timpan4/helium-arcified/issues/6) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Provide an optional Little Arc style window for links opened by another app.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Default-browser handoff chooses the intended Profile and opens one focused lightweight window.
- [ ] A visible action promotes the page into a selected Space, preserving navigation state when supported.
- [ ] Close, always-open-in-main-browser, repeated links, fullscreen and multiple displays have predictable behavior.
- [ ] Cover keyboard quick lookup without an external URL, duplicate links, multiple lightweight windows, promotion history and Archive provenance. Decide idle cleanup explicitly.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Move tabs and Spaces between native windows](https://github.com/Timpan4/helium-arcified/issues/72) (`window-ownership`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Arc feature evidence is macOS-heavy or macOS-only as recorded in the dossier. Windows parity is not assumed; Linux is a downstream design target.

- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Air Traffic Control Automate Your Link Routing](https://resources.arc.net/hc/en-us/articles/22932014625431-Air-Traffic-Control-Automate-Your-Link-Routing)

## Open decisions and limits

No official Windows/Linux parity was established. Invocation, reuse and idle policy are downstream design choices.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
