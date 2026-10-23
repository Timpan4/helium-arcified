<!-- arcified-backlog:local-ocr -->

Part of [Track creation and live integrations](https://github.com/Timpan4/helium-arcified/issues/10) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Extract text from a selected image or capture with a supported local platform adapter.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] Explicit invocation shows image scope and detected text before copy or use.
- [ ] Keep processing local, report unsupported/denied origins and never silently fall back to a cloud provider.
- [ ] Verify Linux, Windows and macOS capabilities independently and label confidence/errors without treating Apple Live Text as a portable mechanism.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Capture a full page and reuse it from Library](https://github.com/Timpan4/helium-arcified/issues/37) (`capture`)
- [Keep private windows out of workspace persistence and AI](https://github.com/Timpan4/helium-arcified/issues/98) (`private-browsing`)

## Evidence and design intent

YAB official manual/product documentation, reviewed 2026-10-23. Its inconsistent version labels and vendor size/performance claims do not establish runtime results.

YAB sources describe macOS/WebKit. Arc/Dia documentation contributes only where explicitly cited. Linux and Windows are independent downstream targets.

- [YAB: manual](https://yetanotherbrowser.com/manual)
- [YAB: changelog](https://yetanotherbrowser.com/changelog)
- [YAB: privacy](https://yetanotherbrowser.com/privacy)

## Open decisions and limits

YAB documents macOS Live Text. No equivalent Linux/Windows adapter or measured accuracy is established.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
