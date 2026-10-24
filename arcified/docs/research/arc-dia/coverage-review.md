# Coverage review of the Arc and Dia backlog

Reviewed 2026-10-23. Review target: the complete draft `backlog.json` with 108 records, plus `arc-ux.md`, `dia-ux-performance.md`, `fork-baseline.md`, and `artifacts.md`. I also read this directory's README and the applicable Arcified instructions. This report recommends changes. It does not apply them to the backlog or publish anything.

## Verdict

Do not publish the current catalog as a complete, implementation-ready account of "all useful Arc desktop UX." It is a substantial candidate backlog, with sound cautions about static evidence and vendor performance claims, but it still has ownership contradictions, missing feature dispositions, weak security prerequisites, and tickets that bundle independently deliverable work.

Publishing scoped research and implementation issues is appropriate after the editorial and dependency corrections below. Running Arc/Dia, completing Ghidra analysis, measuring performance, or implementing the browser is not a prerequisite to publishing honestly labeled follow-up issues. Those remain prerequisites to the corresponding runtime, implementation, or parity claims.

I read every draft record, including its acceptance criteria and blocking keys. Programmatic checks on that complete JSON found 108 unique keys, no unknown blocking keys, no dependency cycles, and no unknown source-group keys. All records have the same evidence disclaimer. There are no per-record platform-evidence or disposition fields. A valid dependency graph and a large issue count do not establish semantic coverage.

## Publication blockers and required changes

### 1. Settle ownership before implementing dependent state

Affected keys: `spaces`, `profiles`, `temporary-tabs`, `pins`, `favorites`, `folders`, `session-durability`, `window-ownership`, `tab-handoff`, `split-horizontal`, `split-vertical`, `private-browsing`.

`spaces` says each Space owns live tab identities and selection. `temporary-tabs` gives each Space a Today area. The decision that temporary tabs are window-local appears only in `tab-handoff`, after `window-ownership`, which itself waits on persisted state. `window-ownership` still allows moved, mirrored, or duplicated live tabs, while `tab-handoff` requires no duplicate live instances. A developer can satisfy the early issues and then have to replace their core data model.

1. Add `workspace-ownership-contract` as a small design issue before these implementations. Specify the identities and ownership of Profile, Space, persistent item, live page, window attachment, window-local temporary tab, and split layout. A split references page identities; it must not become a second owner of duplicated navigation state.
2. Make `spaces` and `profiles` depend on that contract, rather than designing Profile attachment after Spaces. Define one Profile per Space, Profile-scoped Favorites and settings, and how a window presents Spaces from different Profiles. Require actual isolation of all named services, not merely separated cookies or a UI label.
3. Define Space reassignment, Profile deletion, cross-Profile moves, and private-mode actions before allowing them. A live authenticated page cannot silently retain one Profile's session while presenting another Profile's identity. Choose an explicit move/reopen/deny flow with unsaved-work handling. Reject deletion while referenced, or define a transactional detach/migrate flow; confirmation alone is insufficient.
4. Keep `window-ownership` as the implementation of the chosen rule. Keep `tab-handoff` as its persistent-tab interaction slice. Do not leave mutually incompatible rules open in both.
5. Split persistence infrastructure from tests for later features. `session-durability` currently promises split recovery before a durable split exists. Define the schema and recovery foundation first, then attach split recovery to the split feature. Simply making it depend on `split-vertical` would create a cycle because that issue already depends on `session-durability`.

This is an architecture blocker to the current implementation order, not a demand to reproduce undocumented Windows behavior. Arc's macOS Tab Handoff is documented in November 2023. Cite that source for `tab-handoff`, rather than only the 2024-2026 and Windows rollups. [Arc macOS 2023 release notes](https://resources.arc.net/hc/en-us/articles/20498377604887-Arc-for-macOS-2023-Release-Notes).

### 2. Move security requirements ahead of the capabilities they constrain

Affected keys: `local-assistant`, `context-selector`, `remote-provider`, `page-reading`, `read-tools`, `write-tools`, `ai-boundary`, `skills`, `link-summaries`, `instant-links`, `boost-code`, `automation`, `shared-board`, `sync-contract`.

`ai-boundary` depends on `read-tools`; `write-tools` depends only on `read-tools`; `remote-provider` depends only on `context-selector`. This permits page upload and tool execution before the trust boundary is specified or tested. The README's general caution cannot repair that order.

1. Add `assistant-consent-contract`, a pre-implementation data-flow and authorization contract. Make page reading, local/remote provider dispatch, and tools depend on it. Cover explicit source approval, Profile binding, redirects and endpoint identity, private/form data, cancellation, logs, and revocation. Local models still receive page data and are still subject to the repository's explicit page-content approval requirement.
2. Retain `ai-boundary` as adversarial integration validation after the implementations exist. Make enabling/distributing those capabilities depend on that validation. Do not create a dependency cycle by making their implementation depend on a test that needs them first.
3. Enforce capabilities outside model output. The criterion that page instructions "cannot" authorize tools needs browser-owned authorization checks, approved source/action identities, and hostile fixtures. A system prompt or a successful test corpus is not proof that prompt injection is solved.
4. In `read-tools`, distinguish retrieval from harmlessness: a read can expose credentials or cause a network request. In `link-summaries` and `instant-links`, cover credentials, redirects, local/private-network destinations and generated URLs, not only URL syntax. Never fetch generated citations or remote images merely to render an answer.
5. In `remote-provider`, document transport, redirect/fallback behavior and secure-storage failure, including a Linux environment without an available key store. No silent provider change, downgrade to unsafe storage, or content retry to a different provider. Mark service/retention choices as unresolved instead of the generic ready-to-refine status.
6. In `boost-code`, require a reviewed permission model, per-origin and per-device enablement, disabled imported/synced scripts, a browser-level disable path, and explicit interaction with CSP and frames. Do not inherit script authorization from `boost-sharing` or sync enrollment. `automation` likewise needs an authenticated, authorized local boundary and no unauthenticated network command endpoint by default.

Dia's published security controls are useful examples, not security proof for this fork. Its policy explicitly acknowledges residual prompt-injection risk. [Dia Security](https://www.diabrowser.com/security).

### 3. Preserve data-loss and import boundaries in the actual tickets

Affected keys: `session-durability`, `export`, `arc-import`, `browser-import`, `prototype-migration`, `notes`, `easel`, `easel-drawing`, `live-capture`, `boost-sharing`, `private-browsing`, `archive`, `downloads`, `recent-files`, `sync-recovery`, and the release issues.

1. Add `schema-migrations` for supported on-disk upgrades, interruption, invalid/unknown future versions, rollback compatibility, and recovery without overwriting the last valid original. "Versioned validated state" and an export round-trip do not cover an installed browser upgrade. Make release upgrade tests use these fixtures.
2. Separate `arc-import` format discovery from its importer. Add `arc-import-format` to inspect authorized exports and document what they actually contain. Dia's direct import claim does not prove a portable Arc export contains Spaces, Favorites, Profiles, splits, or sessions. Keep unsupported fields visible rather than fabricating them or requiring all exports to contain them.
3. Require inert handling of imported HTML, archives, URLs, CSS and embeds. `notes` needs path-containment checks, no automatic scripts or remote fetches, safe attachment handling and transactional import. `easel-drawing` and `live-capture` need trusted/sandboxed embeds and origin/region revalidation. `boost-sharing` must consider network-bearing CSS and imported rule scope, not equate "no JavaScript" with "no data exposure." These are requirements for future implementations, not findings of existing vulnerabilities.
4. Restore the dossier's distinction between clearing history, removing a Library reference and deleting a real file. Add `library-file-removal` for explicit file deletion with OS Trash/recycle behavior where supported, honest fallback behavior, missing files and failed deletion. `downloads`/`recent-files` must not expose partial or quarantined files as completed, safe downloads.
5. In `private-browsing`, distinguish app-owned session records from files the user deliberately saves, downloads, copies or exports. Preserve private-mode isolation without promising that user-saved files vanish at session end. Apply the policy to Archive, AI context/history, captures, integration caches and diagnostics.
6. In `sync-recovery`, scope revocation to future authorized service/key access. It cannot revoke plaintext or keys already copied by another device. Record that limitation and test the chosen key-rotation/enrollment protocol. Separate enrollment, recovery-secret UX and revocation implementation instead of using a generic "future access is denied" claim.

### 4. Resolve product dispositions before presenting optional work as accepted scope

`phase: Optional capability` is scheduling metadata, not a decision that a feature is useful, approved, or rejected. `retirements` only promises to make a table later. That cannot support a present claim that every historical need has been dispositioned.

Fill the disposition table below now, with recommended outcomes identified as recommendations until accepted. Carry the decisions into each affected issue. Keep useful historical Arc workflows eligible for implementation, but do not rebuild every historical service or Dia work feature merely because it appears in a changelog.

The parent brief allows this review to recommend scope; it does not authorize the reviewer to make final product choices. No new permission is needed to publish a clearly labeled decision/research issue within the authorized fork backlog.

### 5. Make evidence and platform scope survive issue publication

All 108 records use the same evidence sentence. It mixes documented behavior with downstream recommendations and does not identify the particular unsupported detail. Examples: `automation` cites an `.sdef` presence lead but no executed automation; `compatibility` cites artifact URLs without explaining which capability those bytes establish; `runtime-reference` asks to use recorded identities even though only macOS images are recorded.

1. Add per-record evidence references, source release/date or section, Arc platform/status, downstream Linux/Windows/macOS target, deliberate deviations, and unresolved questions. Link to a durable repository dossier at the reviewed revision when available. Keep direct first-party links too.
2. Preserve the Arc 1.170.0 build 89123 and Dia 1.54.2 build 89247 artifact hashes in artifact-based issues. `Dia-latest.dmg` is an acquisition route, not a pinned identity. Do not label it Dia 1.54.0 because the changelog uses that label.
3. Add `reference-windows-artifacts`, or split `runtime-reference` by supported OS. Acquire and identify official Windows reference packages before their scenarios; the macOS identities do not cover Windows. Record Linux as a downstream design/verification target, not an official Arc/Dia reference platform.
4. Associate unresolved semantic decisions with the affected keys, especially `pins`, `peek`, `little-arc`, `window-ownership`, `tab-handoff`, and `split-vertical`. Either make the specific reference research a prerequisite or explicitly choose a downstream behavior. A completely disconnected `runtime-reference` issue does not constrain contradictory implementation claims.
5. Keep `release-ux-check` open against the accepted feature matrix. Its current release-package blockers do not enumerate the accepted features or their validation states. Add per-OS implementation, test and release status; feature absence cannot pass because packaging passes. Do not make every feature wait for all OS builds merely to fix this bookkeeping.

Three exact Arc help URLs returned 404 during this review: Notes retirement, Developer Mode, and collection sharing. The sunset page initially failed through a direct open but was accessible through the release-note link. The release-note rollups remained accessible. Preserve dated source locators or a concise retained fact ledger for vanished pages; a dead page is not proof of feature retirement.

## Missing or weakened Arc feature coverage

These are concrete deltas, not a demand to implement everything. Each needs either an implementation slice or an explicit supported/deferred/replaced disposition.

| Existing key(s) | Addition or change | Evidence and acceptance target |
| --- | --- | --- |
| `favorites`, `live-folders`, `calendar`, `retirements` | Add `favorite-previews-decision`. | Historical Favorite-site previews are distinct from AI `link-summaries`, folder search previews and a meeting countdown. Identify supported integrations, their data/action scope, site-level disable and stale-state behavior. Do not claim the generic live-folder ticket covers them. |
| `retirements` | Add a People Cards row. | The historical source also mentions People Cards. Recommend excluding that service-dependent enrichment from the core browser, while preserving its history as a disposition. Current status and value are unverified. |
| `tab-switcher`, `shortcut-settings` | Add `tab-order-navigation`, or an explicit independently testable slice. | Recent-tab switching does not cover next/previous in Sidebar order or direct tab/Space selection. Specify traversal across folders/splits, scope, platform modifiers and focus. |
| `window-ownership`, `private-browsing`, `onboarding` | Add `blank-window`. | The Arc dossier explicitly distinguishes Blank Window, normal window and Incognito. Define organization, movement into a Space, close and restore. Blank must not be mistaken for private. |
| `spaces`, `sidebar-layout`, `ime-localization` | Add `gesture-navigation`. | Space switching, history gestures and pointer/touch alternatives need intentional routing, cancellation and native testing. IME tests and a general pointer criterion do not test trackpad gestures or conflicts with horizontal page scrolling. |
| `temporary-tabs`, `multi-select`, `auto-archive` | Extend acceptance for manual cleanup. | Explicit Clear Today/close-all, close-other/below where retained, and restore/undo are distinct from timed cleanup. Define active selection and protected work. Record whether Today view resets activity and how policy changes apply to existing tabs. |
| `little-arc` | Add invocation, recovery and multiple-window cases. | Cover a keyboard quick lookup independent of an external URL, duplicate-link reuse, multiple lightweight windows, promotion history and Archive provenance. Close and idle cleanup need explicit policy; do not copy a historical timer automatically. |
| `air-traffic`, `peek`, `archive` | Extend routing scope or state a deviation. | The dossier includes Peek routing and destination cleanup when a Space is removed. Define exact/contains matching, precedence, fallback and one-time override. External links alone are incomplete. Decide explicitly whether Archive restore participates. |
| `pip`, `media-controls` | Add `automatic-pip`, or a separate sub-slice. | Manual PiP is not automatic Mini Player behavior when leaving media. Include global/per-site opt-out, source identity, competing players, return-to-source, resize/move and native keyboard media actions. |
| `library`, `archive`, `command-ranking`, `settings` | Add `history-controls`; extend Library navigation. | A history search provider is not a local history browsing/deletion surface. Define history versus Archive and clear-data effects. Explicitly include the Space organizer and media/created-item providers or mark them deferred; "saved workspace material" is too vague. |
| `share-folders` | Include Space and selected-tab bundles. | The source family covers more than folders and splits. Keep portable snapshots and recipient validation, with Profile/sensitive-link preview. |
| `share-quote` | Rename the present ticket to clipboard formats and add `quote-card-decision`. | Plain URL/Markdown/attributed selection is useful but is not the documented Share Quote hyperlink/image workflow. Decide on local quote cards and text-fragment behavior separately from hosted sharing; disclose deliberate replacement. |
| `developer-mode`, `capture` | Record dispositions for remaining developer affordances. | Include automatic localhost activation, JSON readability and portrait capture where useful. Do not silently install an extension or infer that opening DevTools covers them. Add a small `browser-diagnostics` slice for native task manager/performance recording access if retained. |
| `space-themes`, `folders` | Add personalization disposition details. | Folder icons and theme intensity/grain are absent from criteria. Either support them with original assets or explicitly choose simpler controls. They need not each become a new ticket. |
| `site-controls`, `compatibility` | Add `security-surfaces` and identify ad-block/site-exception behavior. | Verify full origin, security interstitials, before-unload prompts, permission/capture indicators, external-protocol prompts and extension dialogs in Peek/splits/hidden chrome. Preserve the fork's protections and normal access to them. Do not claim an ad blocker or PWA implementation from a generic Chromium label. |
| `easel`, `easel-drawing`, `shared-board` | Add `easel-collaboration-decision`. | A local board plus shared snapshots does not cover live collaboration/presence or editable shared boards from the dossier. Recommend deferring collaboration until a use case and service are chosen; record the gap rather than calling snapshots parity. |

The historical Favorite-site previews and People Cards are explicitly present in the December 16, 2021 entry. The same archive documents advanced theme controls and mouse-based Space switching. These are historical evidence, not proof of availability in Arc 1.170.0. [Historical Arc release notes](https://start.arc.net/release-notes).

The 2023 notes independently support Tab Handoff, Share Quote, developer JSON/portrait workflows, folder icons and Little Arc refinements. Current availability still requires the feature's own evidence. [Arc macOS 2023 release notes](https://resources.arc.net/hc/en-us/articles/20498377604887-Arc-for-macOS-2023-Release-Notes).

## Historic, retired and non-goal dispositions

These are recommendations for the parent to record, not final owner decisions. "Replace" must name the surviving workflow, not erase the historical row.

| Family | Exact affected keys | Recommended disposition |
| --- | --- | --- |
| Arc Notes | `notes`, `retirements` | Compatibility import/view/export and explicit external-document routing. No new native editor in core scope. Unknown legacy attachment/export formats remain research. |
| Reader Mode beta | `site-controls`, `compatibility`, `retirements` | Preserve a useful reading path through the available native/extension facility; explicitly decide unsupported cases. Do not claim restoration of the retired Arc beta merely because a reader menu exists. |
| Paste & Go suggestion | `command-bar`, `retirements` | Retain explicit paste-and-navigate; do not rebuild automatic clipboard suggestion scanning without a separate privacy/product decision. |
| Recent Tab indicator | `tab-status`, `tab-switcher`, `retirements` | Replace the historical visual hint with accessible active/recent navigation if desired. Preserve the distinction between active and most recently used. |
| Command Bar action education | `command-bar`, `onboarding`, `retirements` | Replace the old tutorial with discoverable actions and local onboarding. Action retirement and tutorial retirement are different. |
| GitHub Previews | `live-folders`, `favorite-previews-decision`, `retirements` | Treat the GitHub workflow as replaced by an opt-in Live Folder. Other Favorite-site previews still need their own decision. |
| Legacy Boost Builder | `boost-style`, `boost-zap`, `boost-code`, `boost-sharing`, `retirements` | Replace the old editor with safe declarative customization. Scripts remain a separately approved capability. Deprecating the legacy builder did not retire all Boosts. |
| iCloud sync | `sync-contract`, `sync-enroll`, `sync-recovery`, `retirements` | Do not emulate an unavailable proprietary backend. Retain local export; choose a new interoperable sync service/protocol only through the sync decision. |
| Ask on Page | `ask-page`, `find`, `retirements` | Optional deliberate revival/replacement, not current macOS Arc parity. Keep ordinary find local. Windows status stays unknown until supported evidence exists. |
| Arc account/sharing services and sunset | `onboarding`, `arc-import`, `shared-board`, `runtime-reference`, `retirements` | Local-first onboarding and portable migration. Do not depend on Arc login, vendor sharing, or surviving service access for core use. Test-account availability may constrain reference research. |
| Easels and Live Captures | `easel`, `easel-drawing`, `live-capture`, `shared-board` | Optional useful Arc creation family, not a retired-feature exclusion by default. Keep local authoring/export distinct from authenticated refresh and collaboration. Record accepted/deferred scope. |
| Original Arc integrations | `calendar`, `live-folders`, `favorite-previews-decision` | Opt-in integrations with account/site scope and ordinary bookmark/folder fallbacks. They are not an excuse to add arbitrary Dia enterprise connectors. |
| Dia Memory | `memory-decision`, `chat-history`, `retirements` | Defer personal memory collection. Retain the documented source conflict and a separate decision; do not implement passive browsing capture as a default. |
| Dia work product | `skills`, `read-tools`, `write-tools`, `chat-history`, `memory-decision`, `retirements` | Explicitly non-goal for the Arc desktop/performance objective unless the owner separately accepts it. This includes work connectors, decks/reports, Morning Brief, proactive work collection, enterprise identity/admin workflows and collaboration services. Generic tool/prompt infrastructure also needs a reason to exist in the accepted scope. |
| Arc Max-like assistance | `local-assistant`, `context-selector`, `remote-provider`, `streaming`, `citations`, `page-reading`, `ask-page`, `link-summaries`, `tidy-titles`, `tidy-downloads`, `tidy-tabs`, `chat-command`, `instant-links` | Optional Arc workflows may justify a minimal assistant foundation. Do not make them depend on an enterprise work-agent platform. In particular, `link-summaries` should not require `remote-provider` if an explicitly selected local provider can perform the task. |
| Mobile Arc Search | `retirements` | Outside this desktop request. No implicit requirement to add mobile-only browsing/search behavior. |

The macOS release notes explicitly remove Ask on Page in August 2025, deprecate the legacy Boost builder in October 2024, and require device-specific re-enablement for JavaScript Boosts in September 2024. Keep those separate facts, rather than describing all Boosts as retired. [Arc macOS 2024-2026 release notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes).

The vendor's sunset timeline is a real research-access constraint. It is not a reason to bypass login, manufacture a runtime observation, or claim the remaining UX has been verified. [Arc sunset timeline](https://resources.arc.net/hc/en-us/articles/44412972248471-Arc-Sunset-Key-Dates-and-What-to-Expect).

## Performance and evidence corrections

The performance tickets mostly do the right thing: distinguish vendor claims from measurements, use native workloads, preserve lifecycle protections, and avoid invented time/memory/cache limits. Keep that discipline.

1. `perf-baseline`: add a matched fork/Helium baseline. An Arc-versus-Dia comparison alone confounds engine version, hardware, settings, ad blocking, extensions, account/network state and feature set. Separate UI input-ready, active-page paint, full network completion and background quiescence. Name process accounting and repeatable session fixtures. Native platforms can be measured independently before the full matrix is complete.
2. `perf-activation`, `perf-startup`, `perf-profiles`, `perf-hidden`, `perf-layout`: make each change conditional on a measured local bottleneck. A valid outcome is that the existing Helium/Chromium path already meets the chosen behavior and needs only regression coverage. Do not assume a Dia mechanism is missing here. Required lifecycle/security cleanup cannot simply be delayed to produce a faster paint metric.
3. `perf-hidden`: distinguish visual work from state and security updates. Hidden UI may stop painting; it must still retain correct capture/permission/media state and synchronize on reveal. `perf-layout` must distinguish unnecessary churn from the necessary page reflow when content width changes. "Anchored" cannot mean fixed pixels under every resize.
4. `perf-discard`, `perf-profiles`, `bfcache`, `perf-leaks`: keep archive, renderer discard, BFCache retention, Profile unloading and page destruction distinct. Measure wake-up/reload cost and protected-work correctness along with memory. Account for service workers, extension background work and other Profile-owned services before declaring a Profile unused.
5. `perf-energy`: use actual OS diagnostics under stated power/display/media conditions. No inference from a framework's presence, a quiet sidebar, or a vendor sleep fix establishes energy savings. `perf-ai` remains optional and must separate provider latency from browser overhead.
6. Correct two source attributions in `dia-ux-performance.md`: Memory retirement appears under Mac 1.50.0 dated September 24, 2026, not 1.49.0; the distinct crashed-tab landing description appears under 0.45.0, while 0.44.0 documents unused-Profile unloading. The security FAQ still describes Memory. These are documentary corrections, not runtime findings. [Dia changelog](https://www.diabrowser.com/changelog), [Dia Security](https://www.diabrowser.com/security).
7. Keep `artifacts.md`'s limitations intact: inventory/resource declarations, not execution; partial Interface Builder decoding despite exhausted retained pages; Arc's universal binary required a separate slice; Ghidra did not return accepted analysis; no recovered pseudocode; no measured app performance. The timeout does not negate the successful static inventory, and that inventory does not fill the native-analysis gap.
8. If public issues cite REA action families, include the specific evidence ID, artifact hash, resource and observation in a durable readable summary. Evidence exports under `/tmp` may expire. A mutable download URL or opaque evidence ID alone is insufficient for another developer to understand the claim. Do not copy proprietary code or resources to solve this documentation problem.

I found no actual runtime-performance claim masquerading as a measurement in the reviewed dossiers. The risk is future overstatement and the generic per-ticket evidence text, not a demonstrated fabricated benchmark. I did not independently replay the REA exports or verify the downloaded image signatures/hashes in this review.

## Tickets that need smaller slices

Keep existing keys as parent anchors or rename them with explicit mappings, rather than silently changing their meaning after publication. These are separable outcomes, not estimated durations or imposed ticket-size limits.

| Current key | Concrete split recommendation |
| --- | --- |
| `spaces`, `profiles` | Ownership contract first; Space create/switch; Space organization/delete/undo; Profile create/attach; Profile reassignment/deletion safety. |
| `session-durability` | Durable schema/write/recovery foundation; crash-restoration validation for supported objects; `schema-migrations`; feature-specific restore checks live with splits/windows rather than ahead of them. |
| `split-vertical` | `split-orientation`, `split-persistence`, and `split-durable-actions`. Orientation, resizing, persistence, pin/Favorite/move, and pane maximization need not land together. |
| `arc-import`, `browser-import` | Format/source-capability discovery; workspace/bookmark import; history import; credential/extension support decision per source/OS. Do not publish one ticket implicitly promising every browser's passwords and extensions. |
| `command-bar`, `command-ranking` | Core input/URL/search/action dispatch; scoped tab results; archive/history providers and ranking. Keep one coherent action registry shared by menus/shortcuts, with cancellable queries and IME-safe execution. |
| `site-controls`, `compatibility` | Origin/permission controls; extension/toolbar interaction; password/passkey flow; web-app lifecycle; print/notifications/protocol routing; optional media compatibility. A cross-platform catch-all check does not substitute for testable feature slices. |
| `accessibility`, `ime-localization`, `window-chrome` | Keep these as tracking/gate issues and attach focused requirements to each UI slice. Split platform test runs where tools/input/window-system behavior differ. Do not postpone keyboard access until folders and sidebar are complete. |
| `easel`, `easel-drawing` | Local board persistence/editing; drawing/layout; portable content/image export; media embed safety; collaboration decision. |
| `shared-board` | Service/access/retention decision, `workspace-snapshot-sharing`, and `board-snapshot-sharing`. Workspace link sharing should not wait on drawing tools. |
| `sync-contract`, `sync-recovery` | Threat model/data contract before encrypted proof; then enrollment; recovery material; device revocation/key rotation. Protocol decisions must not be concealed in an implementation ticket. |
| `perf-baseline` | Shared fixtures/measurement definitions, then platform/workload evidence. The current ticket covers startup, activation, navigation, scrolling, memory, idle work and several products on every platform. Treat it as an umbrella, not one small benchmark task. |
| `automation`, `runtime-reference` | Automation capability/security contract plus platform adapters; reference acquisition and scenario observation separately by supported OS. |
| `release-linux`, `release-windows`, `release-macos` | Distribution/identity choice; installable development package; authorized signing/trust verification; authenticated updater and data-preserving recovery. Package availability and update safety are different deliverables. |

Also repair missing completion prerequisites without building a monolithic DAG: `peek` promises split promotion without a split dependency; `multi-select` promises split actions before splits; `find` and `site-controls` promise Peek/split behavior before all of those surfaces exist. Put cross-feature integration criteria in their own slices or explicitly attach them when the dependent feature lands. `sync-conflicts` needs the schema/ownership contract as well as a working transport. `release-ux-check` needs the actual accepted coverage inventory.

## Full-record review accounting

The following inventory covers all 108 original keys once. It establishes which records were inspected, not that every record is ready or that every possible Arc workflow has been found. Findings above identify the changes; unaffected criteria should be preserved.

- Native foundations: `native-linux`, `native-windows`, `native-macos`.
- Workspace and lifecycle: `spaces`, `profiles`, `temporary-tabs`, `pins`, `favorites`, `folders`, `multi-select`, `archive`, `auto-archive`, `session-durability`, `undo-windows`, `tab-status`, `window-ownership`, `folder-preview`, `tab-handoff`.
- Data and privacy: `export`, `arc-import`, `browser-import`, `prototype-migration`, `private-browsing`.
- Navigation: `command-bar`, `command-ranking`, `site-search`, `tab-switcher`, `find`, `peek`, `little-arc`, `air-traffic`, `split-horizontal`, `split-vertical`, `site-controls`, `developer-mode`, `share-quote`.
- Appearance and platform interaction: `sidebar-layout`, `focus-mode`, `space-themes`, `motion`, `shortcut-settings`, `accessibility`, `ime-localization`, `window-chrome`, `settings`, `onboarding`, `automation`, `compatibility`.
- Library and media: `library`, `downloads`, `recent-files`, `capture`, `pip`, `media-controls`.
- Optional Arc creation/integrations: `boost-style`, `boost-zap`, `boost-code`, `easel`, `live-capture`, `notes`, `share-folders`, `live-folders`, `calendar`, `easel-drawing`, `shared-board`, `boost-sharing`.
- Assistance: `local-assistant`, `context-selector`, `remote-provider`, `streaming`, `citations`, `page-reading`, `ask-page`, `link-summaries`, `tidy-titles`, `tidy-downloads`, `tidy-tabs`, `chat-command`, `skills`, `read-tools`, `write-tools`, `ai-boundary`, `chat-history`, `memory-decision`, `instant-links`.
- Performance and renderer recovery: `perf-baseline`, `perf-activation`, `perf-startup`, `perf-discard`, `perf-profiles`, `bfcache`, `perf-hidden`, `perf-layout`, `perf-leaks`, `perf-energy`, `perf-ai`, `crashed-tab`.
- Sync: `sync-contract`, `sync-enroll`, `sync-conflicts`, `sync-recovery`.
- Distribution: `release-linux`, `release-windows`, `release-macos`, `engine-updates`.
- Research and release coverage: `runtime-reference`, `retirements`, `release-ux-check`.

## Parent handoff and limitations

1. Revise the ownership, security, data-recovery, evidence and scope records before bulk publication. Retain unknowns in bounded research/decision issues rather than waiting for every question to be solved.
2. Add or explicitly disposition the missing families and split the broad scopes above. Revalidate key references after rekeying. Do not publish a retired/non-goal feature as an accepted implementation merely because its record already exists.
3. Publish only to the authorized fork, with sources, platform scope, acceptance criteria and real blocking relationships. The parent owns publishing and reading back every issue. This review neither contacted a tracker nor linked/watched any issue or PR.
4. Describe the result as a reviewed candidate backlog with explicit remaining research. This review proves a full read of the supplied catalog and identifies concrete defects. It does not prove all useful Arc needs have been found, native behavior matches, code builds, data migration works, or Dia is faster.

No app, runtime scenario, benchmark, browser build, migration, or native accessibility test ran in this review. I did not inspect real user browsing data, execute downloaded binaries, rerun Ghidra, or independently validate all first-party claims. Selected live source checks corroborated the release details noted above and exposed the historical-preview gap and citation corrections. Those checks were targeted, not a new exhaustive feature census. Only this report is modified by the reviewer.
