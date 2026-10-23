<!-- arcified-backlog:perf-profiles -->

Part of [Track measured performance and recovery](https://github.com/Timpan4/helium-arcified/issues/12) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Release unused Profile-owned views/resources while preserving durable state and isolation.

Type: implementation. Phase: Measured performance.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Use measured need and an explicit lifecycle policy, and never unload active downloads, capture or unsaved work silently.
- [ ] Reactivate the intended Profile and native window with correct cookies, permissions, selected tab and PiP return.
- [ ] Measure idle memory and reactivation latency and test interruption/restart.
- [ ] First identify a measured local bottleneck. If native Helium already meets the contract, retain the proven baseline and focused regression coverage rather than adding speculative mechanisms.
- [ ] Account for service workers, extensions and Profile-owned background tasks before calling a Profile unused.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Attach Spaces to isolated browser Profiles](https://github.com/Timpan4/helium-arcified/issues/48) (`profiles`)
- [Define reproducible browser performance fixtures and milestones](https://github.com/Timpan4/helium-arcified/issues/20) (`perf-baseline`)

## Evidence and design intent

Dia Mac 0.44.0 unused-Profile unloading; no published trigger/capacity contract.

Dia release evidence is mostly macOS, with explicitly cited Windows entries. No Linux reference or reproducible vendor benchmark.

- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)
- [Dia: changelog](https://www.diabrowser.com/changelog)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
