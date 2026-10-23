<!-- arcified-backlog:window-chrome -->

Part of [Track native builds and platform integration](https://github.com/Timpan4/helium-arcified/issues/3) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Make custom browser chrome coexist with each OS's window management.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Test move/resize/maximize/minimize/fullscreen and platform snap/tiling with correct hit regions.
- [ ] Handle mixed display scales, display removal and small windows without offscreen or unreachable controls.
- [ ] Restore window geometry safely and retain permission dialogs, popups and tab focus on the right window.

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

- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)
- [Dia: changelog](https://www.diabrowser.com/changelog)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
