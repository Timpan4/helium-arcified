<!-- arcified-backlog:folder-preview -->

Part of [Track spaces and tab lifecycle](https://github.com/Timpan4/helium-arcified/issues/4) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Preview/search folder contents and reveal an active descendant without forcing the folder open.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Pointer and keyboard invocation list matching child tabs and expose hierarchy to assistive technology.
- [ ] Show the active child of a collapsed folder as a projection rather than changing folder expansion.
- [ ] Background opens, renamed tabs, nested folders and drag-out retain identity and placement.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Organize pinned tabs into nested sidebar folders](https://github.com/Timpan4/helium-arcified/issues/107) (`folders`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Folders Stash Similar Tabs Together](https://resources.arc.net/hc/en-us/articles/19228419623447-Folders-Stash-Similar-Tabs-Together)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
