<!-- arcified-backlog:snippets -->

Part of [Track creation and live integrations](https://github.com/Timpan4/helium-arcified/issues/10) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Create reusable text snippets and insert one into the intended editable field.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] Create/edit/search/delete locally with explicit trigger/action and no default upload.
- [ ] Preview an expansion when replacement could overwrite text, retain caret/selection and support ordinary undo.
- [ ] Respect sensitive fields, Profile/private-mode policy and clipboard boundaries; external-tool import remains a separate format adapter.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Navigate and run browser actions from one command bar](https://github.com/Timpan4/helium-arcified/issues/77) (`command-bar`)
- [Keep private windows out of workspace persistence and AI](https://github.com/Timpan4/helium-arcified/issues/98) (`private-browsing`)

## Evidence and design intent

YAB official manual/product documentation, reviewed 2026-10-23. Its inconsistent version labels and vendor size/performance claims do not establish runtime results.

YAB sources describe macOS/WebKit. Arc/Dia documentation contributes only where explicitly cited. Linux and Windows are independent downstream targets.

- [YAB: manual / mail snippets](https://yetanotherbrowser.com/manual/mail-snippets)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
