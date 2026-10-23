<!-- arcified-backlog:agent-takeover -->

Part of [Track optional intelligence](https://github.com/Timpan4/helium-arcified/issues/11) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Provide an explicit human takeover and resume flow for an accepted browser task.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] Pause before human action and show task state, last action and pending effects.
- [ ] On resume, inspect changed page/source identity and seek fresh approval for changed actions rather than replaying stale input.
- [ ] Cancel, page navigation, account changes and failed resume preserve work and do not duplicate external effects.
- [ ] Before enabling this capability outside the development harness, pass its applicable ai-boundary hostile-context, consent, cancellation and destination-change cases. This is an enablement gate, separate from implementation prerequisites.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Keep an optional agent task in a visibly separate workspace](https://github.com/Timpan4/helium-arcified/issues/66) (`agent-workspace`)

## Evidence and design intent

YAB official manual/product documentation, reviewed 2026-10-23. Its inconsistent version labels and vendor size/performance claims do not establish runtime results.

YAB sources describe macOS/WebKit. Arc/Dia documentation contributes only where explicitly cited. Linux and Windows are independent downstream targets.

- [YAB: manual / agent tabs](https://yetanotherbrowser.com/manual/agent-tabs)
- [YAB: manual / mcp](https://yetanotherbrowser.com/manual/mcp)
- [YAB: privacy](https://yetanotherbrowser.com/privacy)
- [Dia: security](https://www.diabrowser.com/security)
- [Dia: security / bulletins](https://www.diabrowser.com/security/bulletins)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
