<!-- arcified-backlog:lifecycle-contract -->

Part of [Track reference evidence and release coverage](https://github.com/Timpan4/helium-arcified/issues/15) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Record a cross-platform contract for window-local temporary tabs, shared persistent items, Space/Profile ownership and transient views before freezing UI behavior.

Type: decision. Phase: Evidence, decisions and gates.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Define identities/ownership of Profile, Space, persistent item, live page, window attachment, window-local temporary tab and split layout. Splits reference page identity instead of owning copied navigation state.
- [ ] Choose same-site navigation and active-work protections explicitly, with the fork owner deciding unresolved product choices.
- [ ] Record versioned decisions and synthetic transition fixtures; no provider account, numeric capacity or timeout is inferred.
- [ ] Specify one Profile per Space, Profile-scoped Favorites/settings and cross-Profile move/reopen/deny rules. Profile reassignment/deletion cannot relabel a live authenticated page or leave referenced data orphaned.
- [ ] Use consistent consequence-based labels such as Temporary, Kept in this Space and Profile shortcut. Preserve imported Arc labels as explainable aliases, not another lifecycle.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Close with the named decision/evidence artifact and scenario coverage. Owner decisions are recorded in the issue. Unknown native behavior and unresolved service choices remain visible.

## Prerequisites

No blocking implementation issue. Record any named owner decision before enabling optional behavior.

## Evidence and design intent

REA 6.7.0 bundle/resource observations recorded in artifacts.md; no application execution or native-performance measurement.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Pinned Tabs Tabs you want to stick around](https://resources.arc.net/hc/en-us/articles/19231060187159-Pinned-Tabs-Tabs-you-want-to-stick-around)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)
- [Arc: Profiles Separate Work Personal Browsing](https://resources.arc.net/hc/en-us/articles/19227964556183-Profiles-Separate-Work-Personal-Browsing)
- [Official Arc download](https://releases.arc.net/release/Arc-1.170.0-89123.dmg)
- [Official Dia download](https://releases.diabrowser.com/release/Dia-latest.dmg)

Static artifact lead: official Arc 1.170.0 build 89123 macOS DMG SHA-256 `5cf6d057e8944f847dac83d89b4d30d7a49c8974d02d29b35d2cf5afce9e36c1`; its MainMenu.nib SHA-256 `c05560f6665c062b78dd326c4ce56307154010f981a45f4959671fb460ab1963` has retained menu/action declarations (REA evidence `ev_cfff947c28d386fd3ea781438da8f9bf06f232c17b7f9431381cf4bf35d6306f`). This supports workflow discovery only, not callable/current behavior. Official Dia 1.54.2 build 89247 macOS DMG SHA-256 `7f0cb70555190ee1d36563b09a4ed5d8146e2eeaa4b3430bcb38bbce7e6ad913`; the portable compiled-interface decoder recovered updater dialogs, not its product UI. No runtime, native performance, signature verification or accepted decompilation result is claimed. Full native app inventories and partial Arc resource decoding succeeded; native analysis timed out. No proprietary implementation is copied.

## Open decisions and limits

The fork owner records the named scope/security/service decision before implementation or external access.

Publish as an owner decision, not an automatically accepted implementation.
