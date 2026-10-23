<!-- arcified-backlog:auto-archive -->

Part of [Track spaces and tab lifecycle](https://github.com/Timpan4/helium-arcified/issues/4) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Offer configurable automatic archiving with a visible explanation of which tabs will be cleaned up.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Use a documented owner-approved interval or existing project policy; do not invent a fixed timeout.
- [ ] Pinned/Favorite tabs and ineligible active work are protected; forms, audible/capture tabs and downloads have tested handling.
- [ ] Archived tabs appear in the archive with a reason and can be restored or excluded; clock changes and restart do not cause surprise loss.
- [ ] Specify whether viewing Today resets activity and how policy edits apply to existing tabs; do not infer a timer from a historical release.
- [ ] Allow pause/disable and explain an upcoming cleanup before it removes work; choose optional user-controlled cleanup over Arc's historical mandatory policy.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Browse and restore manually archived tabs](https://github.com/Timpan4/helium-arcified/issues/69) (`archive`)
- [Pin a tab with a durable home URL and reset action](https://github.com/Timpan4/helium-arcified/issues/67) (`pins`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Auto Archive Clean as you go](https://resources.arc.net/hc/en-us/articles/19228855311127-Auto-Archive-Clean-as-you-go)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
