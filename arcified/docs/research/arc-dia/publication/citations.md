<!-- arcified-backlog:citations -->

Part of [Track optional intelligence](https://github.com/Timpan4/helium-arcified/issues/11) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Show source citations and page-reading status tied to the request's actual context.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] A citation opens the correct source/tab and distinguishes quoted/extracted evidence from model inference.
- [ ] Changing or closing a source after send preserves its recorded identity and shows stale/unavailable status.
- [ ] No-source, conflicting-source and unreadable-page cases avoid implying that unsupported claims were verified.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Choose and inspect @tab and @space context before sending](https://github.com/Timpan4/helium-arcified/issues/88) (`context-selector`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Dia: changelog](https://www.diabrowser.com/changelog)
- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
