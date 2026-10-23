<!-- arcified-backlog:share-quote -->

Part of [Track navigation and multitasking](https://github.com/Timpan4/helium-arcified/issues/6) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Offer explicit copy actions for a plain URL, Markdown link and selected text with attribution.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Preview or clearly label the copied format and keep copied text tied to the current page/pane.
- [ ] Avoid hidden tracking/authenticated URL data and provide clean-link behavior without breaking legitimate navigation.
- [ ] Clipboard failures, selection changes and private content have safe visible behavior; support keyboard invocation.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Open and close temporary tabs within a Space](https://github.com/Timpan4/helium-arcified/issues/49) (`temporary-tabs`)

## Evidence and design intent

REA 6.7.0 bundle/resource observations recorded in artifacts.md; no application execution or native-performance measurement.

Arc feature evidence is macOS-heavy or macOS-only as recorded in the dossier. Windows parity is not assumed; Linux is a downstream design target.

- [Arc: Share Spaces Folders Splits with Anyone](https://resources.arc.net/hc/en-us/articles/19228534606743-Share-Spaces-Folders-Splits-with-Anyone)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Official Arc download](https://releases.arc.net/release/Arc-1.170.0-89123.dmg)
- [Official Dia download](https://releases.diabrowser.com/release/Dia-latest.dmg)

Static artifact lead: official Arc 1.170.0 build 89123 macOS DMG SHA-256 `5cf6d057e8944f847dac83d89b4d30d7a49c8974d02d29b35d2cf5afce9e36c1`; its MainMenu.nib SHA-256 `c05560f6665c062b78dd326c4ce56307154010f981a45f4959671fb460ab1963` has retained menu/action declarations (REA evidence `ev_cfff947c28d386fd3ea781438da8f9bf06f232c17b7f9431381cf4bf35d6306f`). This supports workflow discovery only, not callable/current behavior. Official Dia 1.54.2 build 89247 macOS DMG SHA-256 `7f0cb70555190ee1d36563b09a4ed5d8146e2eeaa4b3430bcb38bbce7e6ad913`; the portable compiled-interface decoder recovered updater dialogs, not its product UI. No runtime, native performance, signature verification or accepted decompilation result is claimed. Full native app inventories and partial Arc resource decoding succeeded; native analysis timed out. No proprietary implementation is copied.

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
