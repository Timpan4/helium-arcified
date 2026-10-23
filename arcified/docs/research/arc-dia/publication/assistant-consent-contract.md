<!-- arcified-backlog:assistant-consent-contract -->

Part of [Track reference evidence and release coverage](https://github.com/Timpan4/helium-arcified/issues/15) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## Priority

Later, after non-AI browser UX, platform support and measured core performance. This work must not block ordinary browser functionality. Retain its security and consent requirements if implemented later.

## User outcome

Specify data access and provider/tool authorization before implementing assistant dispatch.

Type: decision. Phase: Evidence, decisions and gates.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Bind approved sources and actions to user, Profile, origin, endpoint and exact operation; distinguish public research fixtures from real page-content approval.
- [ ] Define explicit page/selection approval for local and remote providers, private/form exclusions, transport/redirect/fallback behavior, cancellation, logs and revocation.
- [ ] Enforce capabilities in browser-owned code outside model output. The fork owner records unresolved data/service choices before enabling them; no prompt is treated as authorization.
- [ ] Use browser-owned capability receipts for approved endpoint, Profile, source identities, payload class, effects and revocation. Navigation/account/destination/payload changes invalidate affected approval.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Close with the named decision/evidence artifact and scenario coverage. Owner decisions are recorded in the issue. Unknown native behavior and unresolved service choices remain visible.

## Prerequisites

- [Settle native tab ownership and cleanup semantics](https://github.com/Timpan4/helium-arcified/issues/24) (`lifecycle-contract`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)
- [Dia: security](https://www.diabrowser.com/security)
- [Dia: security / bulletins](https://www.diabrowser.com/security/bulletins)

## Open decisions and limits

The fork owner records the named scope/security/service decision before implementation or external access.

Publish as an owner decision, not an automatically accepted implementation.
