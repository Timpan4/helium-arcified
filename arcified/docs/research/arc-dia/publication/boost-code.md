<!-- arcified-backlog:boost-code -->

Part of [Track creation and live integrations](https://github.com/Timpan4/helium-arcified/issues/10) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Treat user-authored site CSS or scripts as a separately enabled advanced capability.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] Clearly show site scope, code capability, enable state and reset; never execute imported code just by previewing it.
- [ ] Separate shareable visual styles from executable code and require inspection/confirmation before enabling imported scripts.
- [ ] Respect browser privilege boundaries and test that site code gains no browser-chrome or other-Profile access.
- [ ] Require a reviewed per-origin/per-device permission model, imported scripts disabled until explicit enablement, a global disable path and documented CSP/frame behavior.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Customize a site's appearance with a reversible Boost](https://github.com/Timpan4/helium-arcified/issues/54) (`boost-style`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Arc feature evidence is macOS-heavy or macOS-only as recorded in the dossier. Windows parity is not assumed; Linux is a downstream design target.

- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for macOS 2023 Release Notes](https://resources.arc.net/hc/en-us/articles/20498377604887-Arc-for-macOS-2023-Release-Notes)

## Open decisions and limits

Script enablement is a separately reviewed per-device/origin capability; no imported/synced code executes by default.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
