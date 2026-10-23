<!-- arcified-backlog:area-performance -->

Part of [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2). This tracker groups 15 scoped work items for measured performance and recovery.

## Work items

- [ ] [#20 Define reproducible browser performance fixtures and milestones](https://github.com/Timpan4/helium-arcified/issues/20), `perf-baseline`, research, Measured performance
- [ ] [#90 Present the next tab before doing close cleanup](https://github.com/Timpan4/helium-arcified/issues/90), `perf-activation`, implementation, Measured performance
- [ ] [#91 Restore a usable window before loading background tabs](https://github.com/Timpan4/helium-arcified/issues/91), `perf-startup`, implementation, Measured performance
- [ ] [#123 Sleep eligible background tabs and restore them safely](https://github.com/Timpan4/helium-arcified/issues/123), `perf-discard`, implementation, Measured performance
- [ ] [#92 Unload inactive Profile resources without mixing identities](https://github.com/Timpan4/helium-arcified/issues/92), `perf-profiles`, implementation, Measured performance
- [ ] [#124 Preserve eligible pages through back and forward cache](https://github.com/Timpan4/helium-arcified/issues/124), `bfcache`, implementation, Measured performance
- [ ] [#93 Stop redundant work in hidden sidebar and split surfaces](https://github.com/Timpan4/helium-arcified/issues/93), `perf-hidden`, implementation, Measured performance
- [ ] [#125 Keep page content anchored during sidebar and split transitions](https://github.com/Timpan4/helium-arcified/issues/125), `perf-layout`, implementation, Measured performance
- [ ] [#126 Detect browser-owned resource leaks through repeated workflows](https://github.com/Timpan4/helium-arcified/issues/126), `perf-leaks`, implementation, Measured performance
- [ ] [#40 Keep idle browsing quiet and allow OS sleep](https://github.com/Timpan4/helium-arcified/issues/40), `perf-energy`, implementation, Measured performance
- [ ] [#127 Measure assistant startup and rendering without slowing browsing](https://github.com/Timpan4/helium-arcified/issues/127), `perf-ai`, implementation, Measured performance, priority: Later
- [ ] [#94 Recover a crashed renderer with a clear retry surface](https://github.com/Timpan4/helium-arcified/issues/94), `crashed-tab`, implementation, Measured performance
- [ ] [#45 Record the native browser performance baseline on Linux](https://github.com/Timpan4/helium-arcified/issues/45), `perf-measure-linux`, implementation, Measured performance
- [ ] [#46 Record the native browser performance baseline on Windows](https://github.com/Timpan4/helium-arcified/issues/46), `perf-measure-windows`, implementation, Measured performance
- [ ] [#47 Record the native browser performance baseline on macOS](https://github.com/Timpan4/helium-arcified/issues/47), `perf-measure-macos`, implementation, Measured performance

## Tracker closure

Keep native sub-issue progress and blocking relationships authoritative. Close accepted implementation work with its named native evidence; close decisions with an explicit approved scope or deferral. Research/tracking closure proves only its named result. Record implemented, tested and released states separately. Optional candidates do not require an account or service for core browsing. Preserve unknown platform behavior and failure/accessibility/data-safety criteria in the owning work item.

Every child carries its own sources, baseline, prerequisites and close proof. The roadmap records pinned artifact identities, historical dispositions and full family coverage. No product code, app execution or measured performance is implied by publication.
