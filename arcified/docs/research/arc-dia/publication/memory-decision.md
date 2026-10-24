<!-- arcified-backlog:memory-decision -->

Part of [Track optional intelligence](https://github.com/Timpan4/helium-arcified/issues/11) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Treat persistent personal memory as a separate opt-in product decision, given conflicting Dia documentation.

Type: decision. Phase: Evidence, decisions and gates.

Disposition: Recommended optional capability, independently disableable. It does not block basic local browsing.

## Acceptance criteria

- [ ] Document the evidence conflict and choose supported scope, retention and sensitive-site exclusions before implementation.
- [ ] If enabled, users can inspect/edit/delete memories, disable collection and distinguish memory from history.
- [ ] If accepted, verify revocation/deletion and cross-Profile separation with synthetic data; do not treat vendor Memory claims as current runtime proof.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Close with the named decision/evidence artifact and scenario coverage. Owner decisions are recorded in the issue. Unknown native behavior and unresolved service choices remain visible.

## Prerequisites

- [Manage local assistant conversations and generated artifacts](https://github.com/Timpan4/helium-arcified/issues/155) (`chat-history`)
- [Validate assistant authorization with hostile page fixtures](https://github.com/Timpan4/helium-arcified/issues/154) (`ai-boundary`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Dia: changelog](https://www.diabrowser.com/changelog)
- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)

## Open decisions and limits

Dia Mac 1.50.0 (September 24, 2026) says Memory is retiring, while Security FAQ still describes it. Recommend deferring personal collection.

Publish as an owner decision, not an automatically accepted implementation.
