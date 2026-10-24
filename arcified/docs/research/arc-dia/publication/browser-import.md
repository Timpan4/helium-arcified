<!-- arcified-backlog:browser-import -->

Part of [Track data migration and privacy](https://github.com/Timpan4/helium-arcified/issues/5) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## User outcome

Offer a first-run or Settings import from installed browsers with explicit data-category choices.

Type: implementation. Phase: Core desktop UX.

Disposition: Recommended core integration or verification, with improvements preferred over literal Arc replication.

## Acceptance criteria

- [ ] Preview source Profile and categories; browser/OS credential-store access requires normal user consent and supports denial.
- [ ] Preserve bookmark hierarchy and avoid silent replacement; explain unsupported password/extension transfer.
- [ ] First-run and later imports are cancellable and recover from a locked source without corrupting either Profile.
- [ ] Offer category/open-tab choices, progress and additive/idempotent re-import. Preserve source data and report OS credential prompts and unsupported extension/session state.
- [ ] Stage a local preview manifest with source provenance, import tabs asleep until selected and distinguish genuinely new/conflicting records on repeat import. Never periodically reread another browser's credential store implicitly.

## Starting point and close proof

The fork currently has an extension prototype. Inspect and reuse Helium's adjacent native features, then integrate or implement the missing contract. Extension tests alone never prove native behavior.

Target platform(s): Linux, Windows and macOS.

Verify the affected interaction in an installed native build on Linux, Windows and macOS. Attach focused behavior and visual evidence, source/build identity, and the named persistence/failure/accessibility cases. Record unavailable-platform checks as open. Extension-only checks cannot prove native behavior. Use public/synthetic fixtures for AI/service checks unless separately authorized.

## Prerequisites

- [Attach Spaces to isolated browser Profiles](https://github.com/Timpan4/helium-arcified/issues/48) (`profiles`)

## Evidence and design intent

YAB official manual/product documentation, reviewed 2026-10-23. Its inconsistent version labels and vendor size/performance claims do not establish runtime results.

YAB sources describe macOS/WebKit. Arc/Dia documentation contributes only where explicitly cited. Linux and Windows are independent downstream targets.

- [Arc: Import Bookmarks Logins History Extensions from Your Previous Browser](https://resources.arc.net/hc/en-us/articles/19335089616791-Import-Bookmarks-Logins-History-Extensions-from-Your-Previous-Browser)
- [Arc: Move from Arc to Dia](https://resources.arc.net/hc/en-us/articles/44413129117591-Move-from-Arc-to-Dia)
- [Dia: changelog](https://www.diabrowser.com/changelog)
- [YAB: manual / bring in](https://yetanotherbrowser.com/manual/bring-in)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
