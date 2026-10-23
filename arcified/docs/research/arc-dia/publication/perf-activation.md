<!-- arcified-backlog:perf-activation -->

Part of [Track measured performance and recovery](https://github.com/Timpan4/helium-arcified/issues/12) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Keep tab switch and close responsive by moving nonessential cleanup off the foreground activation path.

Type: implementation. Phase: Measured performance.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Trace selection, focus and destination paint separately from destruction/cleanup work before and after the change.
- [ ] Exercise forms, downloads, media, splits and Profile switches without changing required lifecycle protections.
- [ ] Run the same native workload on each OS and show the actual mechanism and any memory tradeoff.
- [ ] First identify a measured local bottleneck. If native Helium already meets the contract, retain the proven baseline and focused regression coverage rather than adding speculative mechanisms.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Define reproducible browser performance fixtures and milestones](https://github.com/Timpan4/helium-arcified/issues/20) (`perf-baseline`)
- [Open and close temporary tabs within a Space](https://github.com/Timpan4/helium-arcified/issues/49) (`temporary-tabs`)

## Evidence and design intent

Dia Mac 1.48.0 faster-close note and Windows 1.1.0 tab switch/close reports.

Dia release evidence is mostly macOS, with explicitly cited Windows entries. No Linux reference or reproducible vendor benchmark.

- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)
- [Dia: changelog](https://www.diabrowser.com/changelog)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
