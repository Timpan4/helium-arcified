# Easel live collaboration decision

Issue [#30](https://github.com/Timpan4/helium-arcified/issues/30). Baseline `a4b6b480d3de07857ab6e939149254a8359e5449`. Research recorded 2026-10-23 22:29:28 UTC.

Recommend deferring live multi-user board editing and presence until a concrete user need, accountable service owner, and approved access and retention model exist. Local boards, exported files and published snapshots do not satisfy historical live collaboration. This is a recommendation. The owner's collaboration choice is **PENDING**, and native acceptance is **OPEN**.

## Authority and evidence

The full assigned issue object is preserved in [authority.json](authority.json). [sources.json](sources.json) records exact local SHA-256 identities, external URLs, retrieval status and limits. The root lifecycle contract and native integration authority continue to govern all browser ownership and security behavior.

The [Arc macOS release notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes) document presence-cursor repair on March 14, 2024, v1.34.0, and repair of re-granting edit permission after removal on February 8, 2024, v1.29.0. These entries establish historical presence and permissioned shared editing. They do not document a transport, merge algorithm, retention policy or current service operation.

A cached first-party search result for [Easels: Capture & Create](https://resources.arc.net/hc/en-us/articles/19231142050071-Easels-Capture-Create) describes sharing, PNG export, viewing in other browsers and editing in Arc on macOS. It excludes Windows and iOS authoring. Direct retrieval returned 404; the search cache was five months old. Treat this as historical documentation. Exact invitation UI and current account/service behavior remain unobserved.

The existing [Arc UX dossier](../../../docs/research/arc-dia/arc-ux.md) already separates Easel presence from collection snapshots. Its format/protocol uncertainties remain open. [Artifact evidence](../../../docs/research/arc-dia/artifacts.md) covers static bundle and menu observations, not application execution. No new download, reference-app run or reverse engineering was needed.

## Workflows and their boundaries

1. **Historical shared editing with presence.** A board owner shares access and grants editing permission. Another permitted editor opens the same shared board through a supported editor. Both can modify that shared board and see presence, including the documented cursor concept. Removing and later restoring edit permission is part of the historical workflow. This is a documentary reconstruction, not an observed invitation sequence or proof of simultaneous-edit correctness.
2. **Local board editing.** A user creates and edits local board content. Multiple windows showing local data do not establish remote participants, access grants, shared edit delivery or presence. The optional [easel leaf](../../../docs/research/arc-dia/publication/easel.md) owns editing, undo, persistence, accessibility and safe embeds. This decision adds none of those implementations.
3. **Read-only snapshot sharing.** A user previews selected board content and explicitly publishes a snapshot through a separately approved service. Recipients view that snapshot. Later board edits remain private until another publish action. The [board-sharing leaf](../../../docs/research/arc-dia/publication/board-sharing.md) owns this proposal. It gives recipients neither edit authority over the original nor presence. Hosted revocation concerns future service access, not recall of downloaded recipient copies.
4. **File export or migration.** A user saves an image or portable editable board and transfers that file deliberately. Each recipient copy has an independent lifetime. File exchange gives no concurrent editing, presence, live updates or remote revocation. Historical PNG export does not establish an editable Arc export format.

## Recommended scope and owner record

The root authority approves local-first browsing without new cross-device sync, mandatory accounts or provider access. It does not specifically approve or reject live collaboration. Broad authorization to work on non-AI issues does not settle this service decision.

Recommended option: defer live editing and presence. Continue independently scoped local authoring/export work and keep any proposed hosted snapshots subject to [shared-board](../../../docs/research/arc-dia/publication/shared-board.md). The tradeoff is explicit loss of real-time teamwork while avoiding an unowned remote data service. Do not label a snapshot route as collaboration.

Alternative for a demonstrated need: authorize a separate design investigation after naming the actual multi-user task and accountable operator. Implementation would still require owner decisions on identities and Profile association, editor/viewer grants, invitation and revocation, exposed content/metadata, retention and deletion, backups and service exit, abuse handling, and offline/conflict/failure behavior. No provider, account model, retention period, cleanup action or synchronization mechanism is selected here.

Owner choice: **PENDING**. No approval artifact was found in the assigned object, root manifest, lifecycle decision ledger or execution-state owner decisions. The third acceptance criterion remains incomplete. The coordinator must consolidate the questions below and record the actual response in root authority and the issue through its authorized workflow.

1. "Do you approve deferring live board editing and presence, while retaining local boards/export and treating any hosted snapshot sharing as a separate decision?" Recommended answer: defer. Tradeoff: no live teamwork.
2. If deferral is rejected: "Which concrete multi-user board task requires live editing or presence, who owns the service, and what access, exposed-data, retention, revocation and service-exit model do you approve?" Recommended answer: supply these before authorizing implementation. This preserves owner control but leaves live collaboration unavailable until settled.

These questions are for coordinator consolidation. This packet does not contact the owner or mutate the issue.

## Migration and local-export fallback

Keep source boards, exports and unsupported original data unchanged. The [board-export leaf](../../../docs/research/arc-dia/publication/board-export.md), issue #164, targets a portable editable format with source attribution/layout plus an image export. It requires round-trip, cancellation, file-error and missing-media checks. That is the proposed fallback, not a completed native feature.

For an existing Arc board, use an owner-authorized export only where actually available. The cached help text supports PNG as a visual preservation route. It does not prove a portable editable format or preservation of permissions, presence, shared identities, live captures or edit history. Without a supported editable export, retain originals and report manual reconstruction or unsupported fields explicitly. Never remove a source after copying, infer a migration codec, transfer credentials, or promise recovery of unavailable hosted content. Current Arc service/export access was not tested.

## Integration and native verification limits

No native patch, prepared-source base/result file, ActionItem mapping, string resource or native test registration is applicable to this decision-only change. Do not add a success stub, fake service/store or clickable collaboration action. The reserved legacy ID is 40735; it is not wired here. Library 40701 remains untouched. Any later implementation needing that legacy route must include its actual ActionItem mapping; other actions need ActionItems or coordinated IDs. Any later strings use `IDS_ARCIFIED_EASEL_COLLABORATION_DECISION`. Newly authored Arcified code must use GPL-3.0-only.

The lifecycle Model, metadata/codec, command targets and authoritative private/download classifiers are unchanged. This packet creates no second ownership model or focus-routing contract. Same-window multi-Profile behavior and exact live saved-page handoff remain root production gates. `Model::OpenItem` success and passing pure fixtures would prove neither gate.

Linux, Windows and macOS native interaction checks are **OPEN**, with no browser execution authorized for this leaf. Reference runtime presence, concurrent editing, reconnect/conflicts, access revocation and export availability are unobserved. Future accepted implementation must prove those behaviors with permitted independent participants and scoped fixtures, preserve actual Profile identity and private-content exclusions, and record failure/offline outcomes. Unsupported routes remain unavailable.

The first acceptance criterion is supported by documentary research; the second is delivered as a recommendation; the third awaits the owner's choice. Native acceptance stays **OPEN** independently of a later decision closure. See [checks.md](checks.md) for the actual light checks and [result.json](result.json) for coordinator handoff.
