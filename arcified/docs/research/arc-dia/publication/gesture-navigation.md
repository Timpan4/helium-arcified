<!-- arcified-backlog:gesture-navigation -->

Part of [Track native builds and platform integration](https://github.com/Timpan4/helium-arcified/issues/3) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Make native gesture navigation intentional, cancellable and compatible with webpage scrolling.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Define gesture ownership for Space changes, page history and horizontal webpage content, with pointer/keyboard alternatives.
- [ ] Test begin/update/cancel/commit, direction and multiple displays on supported native devices.
- [ ] Keep selected page/focus stable through interrupted gestures and record hardware/OS limitations.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Create and switch live Spaces in the native sidebar](https://github.com/Timpan4/helium-arcified/issues/32) (`spaces`)
- [Resize, hide and reveal the sidebar without moving focus](https://github.com/Timpan4/helium-arcified/issues/51) (`sidebar-layout`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [start.arc.net release-notes](https://start.arc.net/release-notes)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
