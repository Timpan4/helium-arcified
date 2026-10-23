<!-- arcified-backlog:reference-windows-artifacts -->

Part of [Track reference evidence and release coverage](https://github.com/Timpan4/helium-arcified/issues/15) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Acquire supported official Windows packages for the Windows reference scenarios.

Type: research. Phase: Evidence, decisions and gates.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Record official acquisition URL, exact version/build, package type, architecture and SHA-256 before inspection.
- [ ] Use REA's supported archive/PE/resource routes and state signatures, extraction and runtime limitations separately.
- [ ] Do not reuse macOS hashes as Windows evidence or run installers without a supported authorized test environment.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Close with the named decision/evidence artifact and scenario coverage. Owner decisions are recorded in the issue. Unknown native behavior and unresolved service choices remain visible.

## Prerequisites

No blocking implementation issue. Record any named owner decision before enabling optional behavior.

## Evidence and design intent

REA 6.7.0 bundle/resource observations recorded in artifacts.md; no application execution or native-performance measurement.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Official Arc download](https://releases.arc.net/release/Arc-1.170.0-89123.dmg)
- [Official Dia download](https://releases.diabrowser.com/release/Dia-latest.dmg)
- [start.arc.net release-notes](https://start.arc.net/release-notes)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Dia: changelog](https://www.diabrowser.com/changelog)

Static artifact lead: official Arc 1.170.0 build 89123 macOS DMG SHA-256 `5cf6d057e8944f847dac83d89b4d30d7a49c8974d02d29b35d2cf5afce9e36c1`; its MainMenu.nib SHA-256 `c05560f6665c062b78dd326c4ce56307154010f981a45f4959671fb460ab1963` has retained menu/action declarations (REA evidence `ev_cfff947c28d386fd3ea781438da8f9bf06f232c17b7f9431381cf4bf35d6306f`). This supports workflow discovery only, not callable/current behavior. Official Dia 1.54.2 build 89247 macOS DMG SHA-256 `7f0cb70555190ee1d36563b09a4ed5d8146e2eeaa4b3430bcb38bbce7e6ad913`; the portable compiled-interface decoder recovered updater dialogs, not its product UI. No runtime, native performance, signature verification or accepted decompilation result is claimed. Full native app inventories and partial Arc resource decoding succeeded; native analysis timed out. No proprietary implementation is copied.

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
