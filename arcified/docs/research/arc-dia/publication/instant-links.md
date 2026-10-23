<!-- arcified-backlog:instant-links -->

Part of [Track optional intelligence](https://github.com/Timpan4/helium-arcified/issues/11) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Cover Arc Max's historical workflow of opening search-selected destinations directly.

Type: research. Phase: Evidence, decisions and gates.

Disposition: Historical/retired reference. Recommended replacement or decision, not current runtime parity.

## Acceptance criteria

- [ ] First verify current/historical scope and compare a synthetic search task against ordinary result navigation.
- [ ] Preview destination/source and require an explicit action; generated or redirected URLs pass normal browser validation.
- [ ] Keep provider consent, cancellation and fallback to ordinary search; no arbitrary number of links or feature trigger threshold.
- [ ] Check credentials, redirects and private/local-network targets before fetching; generated citations and remote images are not fetched merely to render an answer.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Close with the named decision/evidence artifact and scenario coverage. Owner decisions are recorded in the issue. Unknown native behavior and unresolved service choices remain visible.

## Prerequisites

- [Send an explicit question from the command bar](https://github.com/Timpan4/helium-arcified/issues/122) (`chat-command`)
- [Validate assistant authorization with hostile page fixtures](https://github.com/Timpan4/helium-arcified/issues/154) (`ai-boundary`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Arc Max Boost Your Browsing with AI](https://resources.arc.net/hc/en-us/articles/19335160678679-Arc-Max-Boost-Your-Browsing-with-AI)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
