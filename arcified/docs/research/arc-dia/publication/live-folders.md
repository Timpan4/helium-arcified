<!-- arcified-backlog:live-folders -->

Part of [Track creation and live integrations](https://github.com/Timpan4/helium-arcified/issues/10) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Implement the approved GitHub pull-request Live Folder workflow with ordinary-folder fallback.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] Preview account, query/scope and update behavior, and allow disconnect without deleting unrelated local tabs.
- [ ] Show loading, empty, stale, permission-denied and removed-item states with a manual refresh.
- [ ] Keep tokens in the OS credential store and request a provider/service decision before enabling a live connection.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Organize pinned tabs into nested sidebar folders](https://github.com/Timpan4/helium-arcified/issues/107) (`folders`)
- [Attach Spaces to isolated browser Profiles](https://github.com/Timpan4/helium-arcified/issues/48) (`profiles`)
- [Choose account scopes for live folder and calendar integrations](https://github.com/Timpan4/helium-arcified/issues/26) (`integration-policy`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Automatic GitHub Live Folders](https://resources.arc.net/hc/en-us/articles/22731612065815-Automatic-GitHub-Live-Folders)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
