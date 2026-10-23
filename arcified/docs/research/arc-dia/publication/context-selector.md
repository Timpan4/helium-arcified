<!-- arcified-backlog:context-selector -->

Part of [Track optional intelligence](https://github.com/Timpan4/helium-arcified/issues/11) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Let users attach selected pages, text or a Space to a question with an inspectable context list.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] Preview each source, origin and content scope; allow removal and show inaccessible or stale content.
- [ ] Read only authorized sources and exclude private/sensitive contexts by default unless explicitly selected and allowed.
- [ ] Switching tabs/Profiles between selection and send does not silently substitute content or mix identities.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Ask an explicitly selected local model from the browser](https://github.com/Timpan4/helium-arcified/issues/55) (`local-assistant`)
- [Attach Spaces to isolated browser Profiles](https://github.com/Timpan4/helium-arcified/issues/48) (`profiles`)
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
