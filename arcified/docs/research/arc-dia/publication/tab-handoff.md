<!-- arcified-backlog:tab-handoff -->

Part of [Track spaces and tab lifecycle](https://github.com/Timpan4/helium-arcified/issues/4) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Bring the existing live pinned/Favorite page into the requesting window using an explicit shared-tab ownership rule.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Apply the approved persistent-item handoff rule to pins and Favorites while keeping temporary tabs window-local.
- [ ] Preserve the original page's navigation/scroll/media state when handing it off and mark where it moved.
- [ ] Test splits, simultaneous selection, closing a source window and cross-display focus without duplicate live instances.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Move tabs and Spaces between native windows](https://github.com/Timpan4/helium-arcified/issues/72) (`window-ownership`)
- [Keep Profile Favorites available across Spaces](https://github.com/Timpan4/helium-arcified/issues/106) (`favorites`)

## Evidence and design intent

Arc macOS 2023 notes, November 2023 Tab Handoff and shared durable/window-local temporary tab history.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)
- [Arc: Arc for macOS 2023 Release Notes](https://resources.arc.net/hc/en-us/articles/20498377604887-Arc-for-macOS-2023-Release-Notes)

## Open decisions and limits

Arc macOS November 2023 notes document Tab Handoff; Windows details remain unverified.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
