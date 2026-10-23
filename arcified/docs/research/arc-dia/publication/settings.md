<!-- arcified-backlog:settings -->

Part of [Track appearance, input and onboarding](https://github.com/Timpan4/helium-arcified/issues/7) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Provide searchable settings with direct links from commands and controls.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Expose Profile-specific versus browser/device-specific scope, including search engine, download location, archive, previews and AI.
- [ ] Search, change, reset and restart preserve intended values and do not erase unrelated data.
- [ ] Keyboard, screen-reader, error and unavailable-platform states work consistently across OSes.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Attach Spaces to isolated browser Profiles](https://github.com/Timpan4/helium-arcified/issues/48) (`profiles`)
- [Navigate and run browser actions from one command bar](https://github.com/Timpan4/helium-arcified/issues/77) (`command-bar`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Keyboard Shortcuts](https://resources.arc.net/hc/en-us/articles/20595231349911-Keyboard-Shortcuts)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)
- [Dia: changelog](https://www.diabrowser.com/changelog)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
