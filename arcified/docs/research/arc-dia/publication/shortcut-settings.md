<!-- arcified-backlog:shortcut-settings -->

Part of [Track appearance, input and onboarding](https://github.com/Timpan4/helium-arcified/issues/7) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Show browser actions and allow conflict-aware shortcut customization.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Search actions, record/reset a binding and explain collisions with browser, webpage and OS commands.
- [ ] Display correct platform modifiers and keep an accessible route to every core action.
- [ ] Bindings survive restart and work in dialogs, split panes and text fields with documented precedence.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Boot a downstream native Arcified build on Linux](https://github.com/Timpan4/helium-arcified/issues/16) (`native-linux`)
- [Boot a downstream native Arcified build on Windows](https://github.com/Timpan4/helium-arcified/issues/17) (`native-windows`)
- [Boot a downstream native Arcified build on macOS](https://github.com/Timpan4/helium-arcified/issues/18) (`native-macos`)

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
