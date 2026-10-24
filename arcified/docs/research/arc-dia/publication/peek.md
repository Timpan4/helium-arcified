<!-- arcified-backlog:peek -->

Part of [Track navigation and multitasking](https://github.com/Timpan4/helium-arcified/issues/6) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Open eligible links as an overlay preview while preserving the parent tab and scroll position.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Specify automatic pinned/Favorite link rules plus explicit modifier invocation and a user setting.
- [ ] Escape/click-outside/close dismiss safely; promotion to an ordinary tab preserves destination and Profile. Split promotion is verified by peek-split-promotion.
- [ ] Back/forward, downloads, permissions, forms and nested links do not strand content or silently lose edits.
- [ ] Verify origin identity, permission bubbles and browser-owned dialogs remain bound to the intended page/pane through site-controls and security-surfaces.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Open and close temporary tabs within a Space](https://github.com/Timpan4/helium-arcified/issues/49) (`temporary-tabs`)
- [Settle native tab ownership and cleanup semantics](https://github.com/Timpan4/helium-arcified/issues/24) (`lifecycle-contract`)
- [Define which page, pane or overlay receives a browser action](https://github.com/Timpan4/helium-arcified/issues/44) (`focus-routing-contract`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)

## Open decisions and limits

Parent/focus/history/download and restore semantics are deliberately specified by lifecycle/focus contracts; native reference observation remains open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
