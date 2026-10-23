<!-- arcified-backlog:page-reading -->

Part of [Track optional intelligence](https://github.com/Timpan4/helium-arcified/issues/11) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Extract user-selected page context without freezing the browser or treating page content as instructions.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] Keep extraction cancellable and outside the foreground UI path; show scope and failure for unsupported frames, PDFs and other formats.
- [ ] Respect origins, permissions, private Profiles and user-selected text, and disclose omissions before sending.
- [ ] Use public/synthetic fixtures to verify attribution and that hidden/unselected page content is excluded.
- [ ] Before enabling this capability outside the development harness, pass its applicable ai-boundary hostile-context, consent, cancellation and destination-change cases. This is an enablement gate, separate from implementation prerequisites.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Choose and inspect @tab and @space context before sending](https://github.com/Timpan4/helium-arcified/issues/88) (`context-selector`)
- [Define browser-owned consent and authorization for optional AI](https://github.com/Timpan4/helium-arcified/issues/43) (`assistant-consent-contract`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Dia: changelog](https://www.diabrowser.com/changelog)
- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
