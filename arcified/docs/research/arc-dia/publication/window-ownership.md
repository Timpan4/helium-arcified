<!-- arcified-backlog:window-ownership -->

Part of [Track spaces and tab lifecycle](https://github.com/Timpan4/helium-arcified/issues/4) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Support tear-off, window-to-window moves and shared Space visibility with one coherent ownership rule.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Implement the single ownership and handoff rule approved in lifecycle-contract; do not leave move/mirror/duplicate as interchangeable runtime choices.
- [ ] Cross-window moves preserve Profile isolation, page state and selected tab; failed drops leave the source intact.
- [ ] Window close/reopen and simultaneous edits do not create orphaned tabs or divergent workspace state.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Attach Spaces to isolated browser Profiles](https://github.com/Timpan4/helium-arcified/issues/48) (`profiles`)
- [Settle native tab ownership and cleanup semantics](https://github.com/Timpan4/helium-arcified/issues/24) (`lifecycle-contract`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)

## Open decisions and limits

Windows ownership semantics remain incomplete in public docs. Implement the single chosen downstream contract.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
