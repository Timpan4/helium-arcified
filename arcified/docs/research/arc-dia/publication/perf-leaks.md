<!-- arcified-backlog:perf-leaks -->

Part of [Track measured performance and recovery](https://github.com/Timpan4/helium-arcified/issues/12) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Exercise repeated tab, Peek, split, Profile and assistant lifecycles and identify retained resources.

Type: implementation. Phase: Measured performance.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Record process/resource ownership and memory trends across repeatable create/close cycles.
- [ ] Distinguish bounded caches, renderer/site behavior and app-owned leaks before attributing a regression.
- [ ] Closing pages, tools and profiles releases owned resources without breaking history, BFCache or downloads.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Define reproducible browser performance fixtures and milestones](https://github.com/Timpan4/helium-arcified/issues/20) (`perf-baseline`)
- [Preview a link in Peek and promote or dismiss it](https://github.com/Timpan4/helium-arcified/issues/79) (`peek`)
- [Browse two tabs side by side with independent navigation](https://github.com/Timpan4/helium-arcified/issues/81) (`split-horizontal`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Dia release evidence is mostly macOS, with explicitly cited Windows entries. No Linux reference or reproducible vendor benchmark.

- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)
- [Dia: changelog](https://www.diabrowser.com/changelog)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
