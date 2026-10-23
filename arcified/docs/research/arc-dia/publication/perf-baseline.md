<!-- arcified-backlog:perf-baseline -->

Part of [Track measured performance and recovery](https://github.com/Timpan4/helium-arcified/issues/12) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Define a repeatable benchmark protocol and shared public/synthetic sessions before comparing products or setting targets.

Type: research. Phase: Measured performance.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Document fixtures, process accounting and milestones for startup, input-to-visible activation, navigation, command search, scrolling, memory and idle work.
- [ ] Control hardware/OS/build, engine/settings, ad blocking/extensions, account/network and cache state; include a matched fork/Helium baseline.
- [ ] Separate perceived-response, input-ready, paint, network completion and quiescence, retain raw traces/distributions, and leave numerical targets to evidence and owner needs.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Close with the named decision/evidence artifact and scenario coverage. Owner decisions are recorded in the issue. Unknown native behavior and unresolved service choices remain visible.

## Prerequisites

No blocking implementation issue. Record any named owner decision before enabling optional behavior.

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Dia release evidence is mostly macOS, with explicitly cited Windows entries. No Linux reference or reproducible vendor benchmark.

- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)
- [Dia: changelog](https://www.diabrowser.com/changelog)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
