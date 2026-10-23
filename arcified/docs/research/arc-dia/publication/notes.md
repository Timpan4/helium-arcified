<!-- arcified-backlog:notes -->

Part of [Track creation and live integrations](https://github.com/Timpan4/helium-arcified/issues/10) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Preserve the useful Notes workflow through read/import/export of owner-supplied legacy material, with an optional editor as a separate product choice.

Type: implementation. Phase: Optional capability.

Disposition: Historical/retired reference. Recommended replacement or decision, not current runtime parity.

## Acceptance criteria

- [ ] Inspect supported Arc Notes HTML ZIP exports from owner-authorized fixtures and preserve text, links and attachments without modifying the original.
- [ ] Search/view/export imported notes locally and allow an explicit external-document destination for New Note.
- [ ] Decide whether a new local editor is needed before implementation; classify Notes as retired Arc scope rather than current binary parity.
- [ ] Treat imported archives, HTML, URLs, CSS and attachments as untrusted and inert. Enforce path/origin containment, no automatic scripts/remote fetches and transactional failure recovery.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Recover the native workspace after an interrupted write](https://github.com/Timpan4/helium-arcified/issues/50) (`session-durability`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Arc feature evidence is macOS-heavy or macOS-only as recorded in the dossier. Windows parity is not assumed; Linux is a downstream design target.

- [Arc: Phasing Out Arc Notes](https://resources.arc.net/hc/en-us/articles/19233788518039-Phasing-Out-Arc-Notes)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
