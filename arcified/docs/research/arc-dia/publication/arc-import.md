<!-- arcified-backlog:arc-import -->

Part of [Track data migration and privacy](https://github.com/Timpan4/helium-arcified/issues/5) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Bring an owner-supplied Arc export into Arcified without flattening its organization.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Convert only export versions/fields proven by arc-import-format, preserving supported names/order and reporting unsupported source data instead of fabricating it.
- [ ] Preview the mapping and unresolved entries; cancellation or a malformed export leaves existing data unchanged.
- [ ] Preserve the original export, report unmappable data, and verify restart and a round-trip Arcified export.
- [ ] Treat imported archives, HTML, URLs, CSS and attachments as untrusted and inert. Enforce path/origin containment, no automatic scripts/remote fetches and transactional failure recovery.
- [ ] Offer category/open-tab choices, progress and additive/idempotent re-import. Preserve source data and report OS credential prompts and unsupported extension/session state.
- [ ] Stage a local preview manifest with source provenance, import tabs asleep until selected and distinguish genuinely new/conflicting records on repeat import. Never periodically reread another browser's credential store implicitly.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Recover the native workspace after an interrupted write](https://github.com/Timpan4/helium-arcified/issues/50) (`session-durability`)
- [Attach Spaces to isolated browser Profiles](https://github.com/Timpan4/helium-arcified/issues/48) (`profiles`)
- [Discover what supported Arc exports actually contain](https://github.com/Timpan4/helium-arcified/issues/27) (`arc-import-format`)

## Evidence and design intent

YAB official manual/product documentation, reviewed 2026-10-23. Its inconsistent version labels and vendor size/performance claims do not establish runtime results.

YAB sources describe macOS/WebKit. Arc/Dia documentation contributes only where explicitly cited. Linux and Windows are independent downstream targets.

- [Arc: Import Bookmarks Logins History Extensions from Your Previous Browser](https://resources.arc.net/hc/en-us/articles/19335089616791-Import-Bookmarks-Logins-History-Extensions-from-Your-Previous-Browser)
- [Arc: Move from Arc to Dia](https://resources.arc.net/hc/en-us/articles/44413129117591-Move-from-Arc-to-Dia)
- [Dia: changelog](https://www.diabrowser.com/changelog)
- [YAB: manual / bring in](https://yetanotherbrowser.com/manual/bring-in)

## Open decisions and limits

Portable export contents remain unknown until arc-import-format. Direct import marketing does not prove export schema.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
