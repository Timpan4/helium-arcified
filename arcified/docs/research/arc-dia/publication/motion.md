<!-- arcified-backlog:motion -->

Part of [Track appearance, input and onboarding](https://github.com/Timpan4/helium-arcified/issues/7) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Make core transitions communicate selection, position and continuity without delaying interaction.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Sidebar hide/reveal and Space switches keep stable content anchoring and accept input during animation. Peek and split tickets own their own interrupted-transition checks.
- [ ] Honor reduced motion and interrupted/reversed transitions without stale final state.
- [ ] Record native visual checks and frame traces on Linux, Windows and macOS; choose durations from design validation rather than invented policy.
- [ ] Expose a disable option for app-generated sounds/haptics and keep loading/privacy feedback understandable without them.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Resize, hide and reveal the sidebar without moving focus](https://github.com/Timpan4/helium-arcified/issues/51) (`sidebar-layout`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)
- [Dia: changelog](https://www.diabrowser.com/changelog)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
