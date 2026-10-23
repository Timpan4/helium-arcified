<!-- arcified-backlog:web-apps -->

Part of [Track native builds and platform integration](https://github.com/Timpan4/helium-arcified/issues/3) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Verify installed web apps remain useful alongside Spaces and lightweight windows.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Install/launch/uninstall a public fixture app with distinct app identity.
- [ ] Protocol/external links, app windows and default-browser entry obey the chosen routing and Profile boundary.
- [ ] Restart and window controls work on each target OS, with unsupported capabilities recorded.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Respect native caption controls, displays and window states](https://github.com/Timpan4/helium-arcified/issues/35) (`window-chrome`)

## Evidence and design intent

The linked fork source is baseline evidence. This issue proposes native integration or a design contract; it does not claim reference-app execution.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
