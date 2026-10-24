<!-- arcified-backlog:air-traffic -->

Part of [Track navigation and multitasking](https://github.com/Timpan4/helium-arcified/issues/6) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Let users define inspectable routing rules for links opened outside the browser.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Create/edit/delete and preview domain/URL match rules with explicit precedence and a fallback.
- [ ] Keep private Profile choices safe; a rule cannot silently expose page content or bypass browser URL validation.
- [ ] Routing works for both direct main-window and lightweight-window entry and supports a one-time override.
- [ ] Support main-window routing first; add a Little Arc destination when that capability exists. Delete rules with deleted targets and show precedence for overlapping rules.
- [ ] Record whether Peek and Archive restore participate; implement the approved scope or label the deliberate deviation. Test exact/contains, overlapping rules, deleted Space targets and one-time override.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Create and switch live Spaces in the native sidebar](https://github.com/Timpan4/helium-arcified/issues/32) (`spaces`)
- [Attach Spaces to isolated browser Profiles](https://github.com/Timpan4/helium-arcified/issues/48) (`profiles`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Arc feature evidence is macOS-heavy or macOS-only as recorded in the dossier. Windows parity is not assumed; Linux is a downstream design target.

- [Arc: Air Traffic Control Automate Your Link Routing](https://resources.arc.net/hc/en-us/articles/22932014625431-Air-Traffic-Control-Automate-Your-Link-Routing)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
