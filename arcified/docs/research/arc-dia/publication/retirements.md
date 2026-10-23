<!-- arcified-backlog:retirements -->

Part of [Track reference evidence and release coverage](https://github.com/Timpan4/helium-arcified/issues/15) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Give every retired or service-dependent Arc feature an explicit downstream disposition.

Type: research. Phase: Evidence, decisions and gates.

Disposition: Historical/retired reference. Recommended replacement or decision, not current runtime parity.

## Acceptance criteria

- [ ] Cover Notes, Reader Mode beta, Paste & Go, Recent Tab indicator, Command Bar education, GitHub Previews, iCloud sync, Legacy Boost Builder, Ask on Page and Arc sunset/migration. Also cover People Cards and historical Favorite-site previews.
- [ ] For each, record implement/replace/import/defer with the surviving workflow and evidence; separately record Dia Memory's conflicting status.
- [ ] Keep mobile Arc Search features, Dia work connectors/decks and unavailable vendor services explicit scope decisions rather than silently claiming desktop parity.
- [ ] The owner excluded iCloud and new cross-device sync. Keep local import/export and migration without an Arcified account; record that explicit disposition instead of planning a replacement sync backend.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Close with the named decision/evidence artifact and scenario coverage. Owner decisions are recorded in the issue. Unknown native behavior and unresolved service choices remain visible.

## Prerequisites

No blocking implementation issue. Record any named owner decision before enabling optional behavior.

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Arc feature evidence is macOS-heavy or macOS-only as recorded in the dossier. Windows parity is not assumed; Linux is a downstream design target.

- [Arc: Arc Sunset Key Dates and What to Expect](https://resources.arc.net/hc/en-us/articles/44412972248471-Arc-Sunset-Key-Dates-and-What-to-Expect)
- [start.arc.net release-notes](https://start.arc.net/release-notes)
- [Arc: Arc for macOS 2024 2026 Release Notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc: Phasing Out Arc Notes](https://resources.arc.net/hc/en-us/articles/19233788518039-Phasing-Out-Arc-Notes)
- [Dia: changelog](https://www.diabrowser.com/changelog)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
