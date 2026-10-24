<!-- arcified-backlog:agent-workspace -->

Part of [Track optional intelligence](https://github.com/Timpan4/helium-arcified/issues/11) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

If browser automation is accepted, isolate its task tabs from the user's foreground browsing while preserving an explicit auth boundary.

Type: implementation. Phase: Optional capability.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] The user chooses task/Profile/page access; agent tabs never silently take focus or acquire unrelated authenticated context.
- [ ] Show ownership, task state, created tabs, retention/cleanup policy and a recoverable result with sources.
- [ ] Keep history/private data policy explicit, and require browser-owned approval before consequential actions; no copied helper counts or close timers.
- [ ] Before enabling this capability outside the development harness, pass its applicable ai-boundary hostile-context, consent, cancellation and destination-change cases. This is an enablement gate, separate from implementation prerequisites.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Choose the browser automation surface and authorization model](https://github.com/Timpan4/helium-arcified/issues/25) (`automation-contract`)
- [Define browser-owned consent and authorization for optional AI](https://github.com/Timpan4/helium-arcified/issues/43) (`assistant-consent-contract`)

## Evidence and design intent

YAB official manual/product documentation, reviewed 2026-10-23. Its inconsistent version labels and vendor size/performance claims do not establish runtime results.

YAB sources describe macOS/WebKit. Arc/Dia documentation contributes only where explicitly cited. Linux and Windows are independent downstream targets.

- [YAB: manual / agent tabs](https://yetanotherbrowser.com/manual/agent-tabs)
- [YAB: manual / mcp](https://yetanotherbrowser.com/manual/mcp)
- [YAB: privacy](https://yetanotherbrowser.com/privacy)
- [Dia: security](https://www.diabrowser.com/security)
- [Dia: security / bulletins](https://www.diabrowser.com/security/bulletins)

## Open decisions and limits

YAB vendor-described macOS behavior only; account/profile, history, side-effect and retention policies need the named contracts.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
