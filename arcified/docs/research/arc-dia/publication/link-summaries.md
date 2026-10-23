<!-- arcified-backlog:link-summaries -->

Part of [Track optional intelligence](https://github.com/Timpan4/helium-arcified/issues/11) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Offer the useful Arc Max link-preview workflow with a clear content-access boundary.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] Invocation clearly identifies that a linked page may be fetched and sent to the selected provider.
- [ ] Cancel, failed fetch, authentication and cross-origin restrictions return to the original page without navigation surprises.
- [ ] Show source attribution and distinguish a generated summary from verified page contents.
- [ ] Support the selected local or remote provider rather than requiring a cloud service; a remote path also consumes explicit remote-provider consent.
- [ ] Check credentials, redirects and private/local-network targets before fetching; generated citations and remote images are not fetched merely to render an answer.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Attribute answers to the exact page context used](https://github.com/Timpan4/helium-arcified/issues/118) (`citations`)
- [Read supported page content outside the browser UI path](https://github.com/Timpan4/helium-arcified/issues/119) (`page-reading`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Arc Max Boost Your Browsing with AI](https://resources.arc.net/hc/en-us/articles/19335160678679-Arc-Max-Boost-Your-Browsing-with-AI)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Arc for Windows 2023 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
