<!-- arcified-backlog:read-tools -->

Part of [Track optional intelligence](https://github.com/Timpan4/helium-arcified/issues/11) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Decide whether to support this optional capability. Requirements for an accepted implementation: Let approved read-only tools fetch explicitly authorized context with visible activity and failure states.

Type: decision. Phase: Evidence, decisions and gates.

Disposition: Candidate beyond Arc parity. Evaluate a concrete owner-valued workflow before implementation; no enterprise-agent platform is required for core UX.

## Acceptance criteria

- [ ] Record the fork owner's supported workflow and approve or defer scope before implementing this optional capability.
- [ ] If accepted, show tool identity, source scope and context actually used, and allow access revocation.
- [ ] If accepted, authenticate per service/Profile and prevent arbitrary page instructions from granting tool access.
- [ ] If accepted, record tool errors and cancellation without hidden retries or invented request quotas.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Close with the named decision/evidence artifact and scenario coverage. Owner decisions are recorded in the issue. Unknown native behavior and unresolved service choices remain visible.

## Prerequisites

- [Define browser-owned consent and authorization for optional AI](https://github.com/Timpan4/helium-arcified/issues/43) (`assistant-consent-contract`)

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Dia: changelog](https://www.diabrowser.com/changelog)
- [Fork: tree/main/arcified](https://github.com/Timpan4/helium-arcified/tree/main/arcified)
- [Fork: blob/main/patches/series](https://github.com/Timpan4/helium-arcified/blob/main/patches/series)

## Open decisions and limits

The fork owner records the named scope/security/service decision before implementation or external access.

Publish as an owner decision, not an automatically accepted implementation.
