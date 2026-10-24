<!-- arcified-backlog:automatic-pip -->

Part of [Track media controls](https://github.com/Timpan4/helium-arcified/issues/9) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Optionally show media controls or PiP when leaving an active media page.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Global/per-site settings clearly enable/disable automatic behavior without breaking manual PiP.
- [ ] Resolve competing players and preserve source tab/Profile ownership on return, resize and move.
- [ ] Keep keyboard/OS media actions and active-work archive/discard protection intact.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Keep video available in picture in picture](https://github.com/Timpan4/helium-arcified/issues/114) (`pip`)
- [Control background audio from the browser shell](https://github.com/Timpan4/helium-arcified/issues/115) (`media-controls`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
