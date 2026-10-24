<!-- arcified-backlog:call-controls -->

Part of [Track media controls](https://github.com/Timpan4/helium-arcified/issues/9) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Expose a compact call indicator and available mute/camera/return controls without moving the call page unexpectedly.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Show the correct source tab/Profile and protect the active call from auto-archive/discard.
- [ ] Switch Spaces/windows and return to the correct call; deliberate leave/close follows normal unsaved/capture prompts.
- [ ] Verify supported sites and OS media/control paths; unsupported controls remain honest rather than emulated silently.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Control background audio from the browser shell](https://github.com/Timpan4/helium-arcified/issues/115) (`media-controls`)
- [Show tab loading, audio, capture and attention state](https://github.com/Timpan4/helium-arcified/issues/71) (`tab-status`)

## Evidence and design intent

YAB official manual/product documentation, reviewed 2026-10-23. Its inconsistent version labels and vendor size/performance claims do not establish runtime results.

YAB sources describe macOS/WebKit. Arc/Dia documentation contributes only where explicitly cited. Linux and Windows are independent downstream targets.

- [YAB: manual](https://yetanotherbrowser.com/manual)
- [YAB: changelog](https://yetanotherbrowser.com/changelog)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
