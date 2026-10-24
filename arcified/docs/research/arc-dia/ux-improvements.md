# UX improvements over Arc

Research date: 2026-10-23
Target: Timpan4/helium-arcified on Linux, Windows and macOS
Purpose: focused product-design deltas for the reviewed backlog, not a replacement catalog

## Position

Arcified should preserve Arc's useful spatial browsing model without preserving its ambiguity. A user should be able to answer four questions at any time:

1. Where is this page: which Profile, Space, window, folder and split?
2. What will this action change: the live page, its durable reference, its location, or its data boundary?
3. Can I reverse it: immediately, after restart, or only from an export?
4. What work is local, what crosses a network boundary and what has actually been measured?

Yab is useful evidence that several Arc-shaped interactions can be made more explicit. Its manual documents switching the same tabs between a sidebar and top row, creating no tab until the user chooses a destination, restoring a batch of closed tabs together, keeping Peek attached to its parent tab, giving each split pane its own address bar, resolving shortcut conflicts, importing one data family at a time and asking before consequential agent actions. These are vendor-documented macOS behaviors, not proof of implementation quality or cross-platform suitability. Arcified should borrow the mechanisms only where they strengthen its own local-first, cross-platform model.

No numerical capacity, cleanup interval, undo lifetime, latency target or pane limit is proposed here. Those values require an existing project policy, an owner decision or measurements from the target builds.

## Material backlog deltas

### 1. Use one tab model with interchangeable presentations

**Proposed issue:** `tab-surface-mode`, "Show the same tabs in a sidebar or native top strip."

**Maps to:** `sidebar-layout`, `spaces`, `temporary-tabs`, `tab-status`, `settings`, `onboarding`, `accessibility`, `window-chrome`.

**Mechanism.** Keep tab, Space, Profile, folder and split identity independent of presentation. The user may choose a left sidebar, right sidebar or platform-appropriate top strip. Changing presentation rebinds the existing model; it does not migrate, clone, reload or reorder pages. A hidden sidebar temporarily overlays the page instead of silently shrinking it, while keyboard access remains available without hover.

This is a deliberate improvement over treating Arc's left sidebar as the product identity. It lets compact laptops, wide displays, pointer users, keyboard users and platform conventions choose an appropriate surface without fragmenting the state model. Yab demonstrates the same-tab presentation switch and non-destructive Spaces toggle on macOS; Arcified must design and verify its own adapters on all three target systems.

**Observable acceptance flow.**

1. Open a workspace containing temporary tabs, durable tabs, a folder, a sleeping tab and a split. Record the selected page, focused control, tab identities, scroll positions and order.
2. Switch from sidebar to top strip, return to the sidebar, then move it from left to right and hide it. The same objects remain selected and ordered, no renderer reload is caused by the presentation change, and status indicators remain available.
3. Reveal hidden chrome by keyboard, locate the active tab with a screen reader, then return focus to the prior page control. Pointer reveal and fullscreen use the same state without trapping focus.
4. Restart and confirm the chosen presentation and workspace are restored independently. Moving the workspace export to another OS preserves the workspace even when that OS chooses a different default presentation.

**Tradeoffs and deviation.** Two presentations increase UI and accessibility verification work. The payoff is platform adaptation without a second data model. Platform-specific materials, title-bar integration and default placement may vary; tab meaning, action names and portable state must not. Do not copy Yab's macOS-only details or infer a minimum window width from its manual.

### 2. Make lifecycle names describe consequences

**Change existing issues:** `lifecycle-contract`, `temporary-tabs`, `pins`, `favorites`, `folders`, `onboarding`, `command-bar`.

**Mechanism.** Present three user-facing lifecycles even if imported Arc labels remain visible during migration:

- **Temporary:** part of the current browsing session and eligible for user-approved cleanup.
- **Kept in this Space:** durable at a stable position and excluded from automatic cleanup.
- **Profile shortcut:** a durable Profile-scoped entry available from every Space attached to that Profile.

Folder membership is organization, not a fourth lifecycle. Opening a kept item creates or activates a live page; its saved destination remains distinct from current navigation. Reset, close, archive, remove from folder and remove the durable reference are different verbs. Context menus, drag targets, command results and accessibility names must use the same terms.

Arc's Favorite, Pinned and Today categories are useful but easy to misread as location alone. Yab's separate pins and kept tabs show another viable vocabulary, but Arcified should derive labels from the consequences rather than copy either product.

**Observable acceptance flow.**

1. Open a temporary page, keep it in the current Space, place it in a folder and add a Profile shortcut.
2. Navigate the live page away from its saved destination. The UI identifies the current page, the kept destination and the Profile shortcut without implying they are separate authenticated pages.
3. Remove the item from its folder. Its kept status is unchanged. Remove the Profile shortcut. The kept Space item remains. Choose Reset and preview the navigation back to the saved destination without silently discarding protected work.
4. Search for the item from another Space. The result says whether it will switch to a live page, open the Space-kept destination or open a Profile shortcut.

**Tradeoffs and deviation.** Clearer nouns add migration copy and may differ from familiar Arc labels. Preserve imported category provenance and explain the mapping once. Do not preserve confusing terminology merely for visual parity.

### 3. Add a workspace organizer for ownership changes

**Proposed issue:** `workspace-organizer`, "Inspect and change Space, Profile and window ownership safely."

**Maps to:** `lifecycle-contract`, `spaces`, `profiles`, `favorites`, `window-ownership`, `tab-handoff`, `library`, `settings`, `private-browsing`.

**Mechanism.** Provide one local organizer that shows Profiles, their Spaces, Profile shortcuts, durable Space items and current window attachments. It exposes ownership before destructive actions and makes cross-Profile movement an explicit reopen operation. A page authenticated in Profile A must never appear to become Profile B merely because its row was dragged there.

The organizer is not a second bookmark manager. It is the place to understand and repair ownership: find unattached Spaces, reassign a Space, detach a window, remove a Profile after its references are resolved and inspect what is excluded from sync/export. Ordinary browsing stays in the sidebar and command bar.

**Observable acceptance flow.**

1. Create Personal and Work Profiles, attach Spaces to each and sign into a synthetic site differently in each Profile.
2. Open the organizer and select the Work Space. It shows its Profile, durable items and attached windows without exposing cookies or secrets.
3. Request reassignment to Personal. Preview lists affected live pages and offers the owner-approved behavior from `lifecycle-contract`, such as reopen saved destinations under Personal while retaining the source until success. Cancel leaves every identity unchanged.
4. Complete reassignment. The synthetic site uses Personal state, the original Work page is not relabeled or leaked, and undo restores the organization without claiming to restore submitted or unsaved renderer state.
5. Attempt to delete a referenced Profile. The organizer blocks or stages the required detach/migration instead of relying on a generic confirmation dialog.

**Tradeoffs and deviation.** Arc's lightweight drag interactions feel faster when ownership is implicit. Arcified should accept a review step at identity boundaries. Same-Profile moves may remain direct and undoable; cross-Profile moves require an explicit reopen or denial path.

### 4. Make every search result state its verb and provenance

**Change existing issues:** `command-bar`, `command-ranking`, `saved-search`, `folder-preview`, `archive`, `settings`, `tab-switcher`.

**Mechanism.** Treat the command bar as a typed result router. Every result exposes a source label and primary verb:

- Live tab: **Switch**, with Space, Profile and window.
- Durable item with no live page: **Open saved destination**, with folder/Space.
- Archive entry: **Restore**, with original Space and close reason.
- History entry: **Open history result**, never mislabeled as Archive.
- Browser command: **Run command**, with side-effect state.
- Setting: **Open setting**, with browser/device/Profile scope.

Opening the command bar creates no empty tab. Escape returns to the exact prior surface. Selecting an already-open page switches to it by default rather than opening a duplicate, while row actions may explicitly move, copy or open another instance when the lifecycle contract permits. A Reveal in Workspace action temporarily exposes the containing Space/folder and returns focus predictably.

Yab documents deferred tab creation, switching to an existing open tab and one field for tabs, history, commands and settings. Arcified should add stronger provenance because it has explicit Profile and cross-window boundaries.

**Observable acceptance flow.**

1. Prepare duplicate titles across Profiles, a collapsed-folder item, an archived tab, a history-only URL and an unavailable setting on one platform.
2. Search by title and URL. Results retain selection while loading and announce result kind, source Profile/Space and action.
3. Activate the live result and observe a switch with no duplicate. Use Reveal in Workspace on the folder result and observe only the necessary ancestors being exposed, with screen-reader focus on the item.
4. Restore the archive result to its original Space, then repeat the search. The archive result disappears or changes state without stale duplication.
5. Open and cancel a blank command bar. No temporary tab, history entry or workspace mutation is created.

**Tradeoffs and deviation.** Provenance consumes row space, so use concise labels and progressive detail without hiding the primary verb. Ranking may still learn locally from explicit selection, but it must not erase type boundaries or cross Profile/private scope.

### 5. Record reversible workspace operations as operations

**Proposed issue:** `workspace-operation-journal`, "Undo a workspace change as the action the user performed."

**Maps to:** `bulk-close`, `archive`, `auto-archive`, `spaces`, `folders`, `multi-select`, `pin-section-controls`, `undo-windows`, `session-durability`.

**Mechanism.** Store app-owned workspace changes as atomic operation records with affected object identities, prior organization and a human-readable reason. A batch close restores as a batch; automatic cleanup identifies the policy that selected the tabs; moving a folder restores its prior parent; deleting a Space restores its organization. Page history and browser history remain separate from this journal.

Yab's manual documents grouped restore for tabs closed together and a cleanup notice with Undo. Arcified should generalize that idea to its own workspace objects and keep the journal local. The journal does not promise recovery of server-side effects, form submissions, live media or renderer memory.

**Observable acceptance flow.**

1. Select several eligible temporary tabs from different positions and close them once. One notification names the operation and affected tabs; protected work remains untouched.
2. Undo. The tabs return to their prior Space, order and selection without resubmitting forms. Redo or a repeated close is not confused with the original operation.
3. Move a nested folder, delete a populated Space with an explicit disposition and trigger cleanup using an owner-approved policy. Each action is distinguishable and undo restores only its own objects.
4. Interrupt a journal write using the session-durability fixture. Restart chooses the last valid state and reports any operation that could not be recovered rather than duplicating objects.

**Tradeoffs and deviation.** A durable journal costs local storage and schema work. Its retention policy must come from the owner or existing project policy. Immediate operation identity is still valuable even if only the current session is retained initially. Permanent deletion remains a separate, explicit action and may be irreversible after its warning.

### 6. Resolve shortcuts and focus through one visible contract

**Change existing issues:** `shortcut-settings`, `accessibility`, `ime-localization`, `command-bar`, `find`, `sidebar-layout`, `focus-mode`, `tab-switcher`.

**Mechanism.** Use a single action registry for menus, shortcuts, command-bar commands and assistive actions. Shortcut settings show the resolved action by context, including page content, browser chrome, text editing, dialog, Peek and active split pane. The UI explains OS-reserved bindings, extension conflicts, keyboard-layout interpretation and which surface handled the last command.

Pages should keep normal editing/application shortcuts unless the browser documents a necessary reserved command. When a page consumes a configurable browser binding, expose a discoverable second invocation or explicit browser-command route rather than silently failing. Do not assume macOS modifiers on Windows or Linux.

Yab's manual documents searchable bindings, immediate conflict disclosure, keyboard-layout awareness, page-first handling and a second press for some browser commands. Arcified should make the resolver inspectable and accessible instead of adopting a hidden precedence table.

**Observable acceptance flow.**

1. Search shortcuts by action name and by the physical/logical key expression used on the current OS. Assign a binding already used by a page-facing action.
2. Settings names the conflict and contexts before saving. Choose a different binding or explicitly accept a supported precedence; cancel changes nothing.
3. On a synthetic editor page, invoke the binding during text entry, IME composition, browser chrome focus and a modal dialog. The documented target handles it, and assistive output identifies the result without corrupting composition.
4. Change keyboard layout and open a split. Invoke Find, Reload and pane navigation. Each action targets the focused pane and the menu displays the effective platform binding.

**Tradeoffs and deviation.** Deterministic conflict handling is more work than a flat shortcut editor. It prevents browser chrome from breaking web applications and gives keyboard users a repair path. A second-press rule can be surprising, so it must be visible, configurable where safe and never replace an accessible menu/command path.

### 7. Define focus routing across page, pane, Peek and chrome

**Proposed issue:** `focus-routing-contract`, "Route browser actions to the intended surface."

**Maps to:** `split-horizontal`, `split-vertical`, `peek`, `find`, `sidebar-layout`, `focus-mode`, `site-controls`, `credentials-flow`, `accessibility`.

**Mechanism.** Model active surface separately from selected workspace item. The focus path is explicit: window, durable item, pane or Peek, then browser chrome or web content. Commands such as address, reload, find, zoom, site controls and credential prompts target the active page surface. Workspace commands such as move, keep and archive target the selected item. A visible, non-color-only cue and accessible announcement identify the active pane or overlay.

Yab documents per-pane address bars and last-click routing, and it keeps a Peek attached to its parent tab across tab switches. Arcified should retain those useful ideas while supporting more than a hard-coded two-pane layout if the approved split model allows it.

**Observable acceptance flow.**

1. Open a split with two synthetic pages, each containing a text field and different permission state. Focus the second pane and invoke address, reload, find, zoom and site controls. Only the second pane changes.
2. Open Peek from the second pane, move focus through its browser controls and web content, then dismiss it. Focus returns to the initiating control or a documented fallback in the second pane.
3. Switch tabs while Peek is open, then return. The chosen lifecycle contract determines whether the same Peek is restored; it never appears over the wrong parent.
4. Maximize a pane, hide chrome, enter fullscreen, show a credential prompt and close the active pane. At each transition the active surface is visible to sighted users and announced to assistive technology. No keyboard route becomes trapped.

**Tradeoffs and deviation.** Per-pane affordances use space. Compact variants may share visual chrome, but target ownership must remain explicit before execution. Do not copy Yab's documented two-pane product limit or Arc's unresolved edge cases as an Arcified limit.

### 8. Make import a staged, repeatable local transaction

**Change existing issues:** `arc-import`, `browser-import`, `workspace-import`, `export`, `session-durability`, `profiles`, `onboarding`.

**Mechanism.** Every import produces a local preview manifest before mutation. The manifest names the source browser/profile, data families, proposed Profile/Space mapping, duplicate keys, unsupported fields and credential handling. The importer records source provenance locally so repeating the same import can identify already-imported objects and present genuine conflicts instead of duplicating everything.

Import data families independently. Tabs/workspaces may be imported asleep and loaded only on selection. Credentials, cookies, extensions and sign-ins are separate security decisions with platform-specific authorization; a successful workspace import must not imply they were copied. The original browser and source export remain unchanged. Export stays available without sync or an account.

Yab documents direct local import, per-family later imports, background completion, sleeping imported tabs and add-only repeat import. Arcified should improve on the trust boundary by avoiding periodic rereads of another browser's credential store unless the owner separately approves such synchronization.

**Observable acceptance flow.**

1. Select an owner-authorized synthetic fixture containing multiple Profiles, Spaces, nested folders, duplicate URLs and an unsupported field.
2. Preview the mapping, exclude a data family, change a destination and cancel. No browser data or source file changes.
3. Import and restart. Organization and source labels remain, tabs are not eagerly loaded, and unsupported data is reported rather than fabricated.
4. Import the same fixture again after both source and destination changed. The preview distinguishes unchanged, new and conflicting objects. Choose a conflict action, then export the resulting Arcified workspace and validate the documented round trip.

**Tradeoffs and deviation.** Provenance metadata and staged review add storage and interaction cost. They make migration repeatable, debuggable and portable. Never promise that cookies, passkeys, passwords, extensions or sign-ins can move uniformly across Linux, Windows and macOS.

### 9. Put optional AI behind browser-owned capability receipts

**Proposed issue:** `assistant-consent-contract`, "Authorize assistant context and effects outside model output."

**Maps to:** `local-assistant`, `context-selector`, `remote-provider`, `page-reading`, `read-tools`, `write-tools`, `ai-boundary`, `streaming`, `citations`, `private-browsing`.

**Mechanism.** Keep AI absent from ordinary browsing unless invoked. Each request gets a browser-owned capability receipt containing:

- provider/endpoint and Profile identity;
- selected tabs, selection ranges or files, with visible origin and stale state;
- allowed reads and the exact payload class;
- proposed external effects, each bound to destination, account and reviewed payload;
- cancellation and revocation state.

The model cannot expand this receipt. Navigation, redirect, account change or payload change invalidates the relevant approval. Drafting into a page and sending from the page are separate effects. Credentials are filled by browser facilities and never exposed as assistant context. Citations identify the content actually read, not merely a URL the model mentioned.

Yab's manual documents visible context pills and confirmations for send, pay, delete and upload. Dia documents selected context, citations and tool activity while acknowledging prompt-injection risk. Arcified should reject blanket bypass as a default and treat any persistent site permission as a later owner decision with visible revocation.

**Observable acceptance flow.**

1. With AI unconfigured and unused, inspect the synthetic network/log fixture and confirm no page text, history or selection is sent to any model endpoint.
2. Configure only the test/local harness approved for development. Select public synthetic text, invoke the assistant and inspect the receipt. Remove one source before send; the serialized fixture excludes it.
3. A hostile synthetic page asks the model to attach another Profile and submit a form. The receipt does not expand. A proposed draft appears without submission.
4. Review the send action, then change the destination or payload. Approval becomes stale and requires a new review. Rejecting sends no mutation; ambiguous cancellation reports what is known rather than retrying silently.
5. Repeat in private browsing. Context access is unavailable unless the separate private-mode policy explicitly permits a one-time action, and no assistant history is persisted.

**Tradeoffs and deviation.** Receipts add friction to multi-step automation. They make the boundary testable and let future policy relax prompts by capability without trusting page or model text. This research does not authorize a provider connection, credential use or real page-content test.

### 10. Design perceived responsiveness and measured performance together, but report them separately

**Change existing issues:** `perf-baseline`, `perf-activation`, `perf-startup`, `perf-hidden`, `perf-layout`, `perf-discard`, `perf-profiles`, `perf-energy`, `streaming`.

**Mechanism.** Define user-visible milestones for each interaction and record the actual work behind them. For tab switch and close, distinguish input received, selection committed, destination first presented, destination input-ready and cleanup quiescent. For launch, distinguish first window, chrome usable, selected page ready and background restoration. For command search, distinguish query accepted, stable first results and provider completion.

The UI may acknowledge input before all work completes, but it must not show a false final state. Foreground focus, title, status and selected item change atomically. Cleanup, cache work and hidden-row maintenance may move off the foreground path only after required lifecycle/security work is preserved. Cancellation must stop app-owned optional work and leave a coherent state.

Dia's changelog reports foreground-first activation, background cleanup, fewer hidden updates, lazy restore and BFCache work, but publishes no reproducible benchmark protocol. Yab's no-empty-tab and sleeping-import mechanisms are useful workload reductions, not performance measurements. Arcified must measure its Chromium-based implementation rather than infer WebKit, Arc or Dia results.

**Observable acceptance flow.**

1. Use the same public/synthetic session fixture on a recorded native build. Exercise switch, close, launch/restore, sidebar hide/reveal, split resize and command search.
2. Capture the named milestones, raw traces, process ownership, renderer state and visual evidence. Record distributions and failure states, not a single best run.
3. Verify that early acknowledgment still names loading, sleeping, crashed or stale states accurately and that the destination accepts input only when reported ready.
4. Compare before/after traces for the actual mechanism. If Helium already avoids the suspected work, close the optimization hypothesis with evidence and retain the regression check rather than adding another scheduler or cache.
5. Run platform-specific checks independently. Do not withhold valid Linux evidence until Windows and macOS are available, and do not generalize one platform's mechanism to the others.

**Tradeoffs and deviation.** Perceived responsiveness can improve without reducing completion time; backgrounding can increase peak memory or prolong cleanup. Report both interaction milestones and resource costs. Targets must come from measurements and owner needs, never Arc/Dia marketing or copied implementation constants.

### 11. Verify one semantic product through platform adapters

**Change existing issues:** `native-linux`, `native-windows`, `native-macos`, `window-chrome`, `shortcut-settings`, `ime-localization`, `accessibility`, `compatibility`, `release-ux-check`.

**Mechanism.** Maintain one semantic action/state matrix and separate presentation/input adapters. Platform conventions may change menu placement, modifier labels, title-bar material, window controls, system credential prompts, file/trash integration, gestures and notification behavior. They may not change ownership, undo semantics, Profile isolation, export format or AI consent.

The release UX check should link each accepted scenario to native evidence per OS and distinguish four states: implemented and verified, implemented but unverified, intentionally adapted, or unsupported with a recorded reason. A package launching is not evidence that the interaction works.

**Observable acceptance flow.**

1. Load the same portable synthetic workspace on Linux, Windows and macOS.
2. Run the ownership, command-result, cleanup, shortcut, split-focus and export flows above using native keyboard, pointer and assistive-technology routes.
3. Record platform-specific presentation and the invariant outcome. An opaque Linux fallback may replace a macOS material; a Windows caption control may occupy different chrome; both preserve readable states and the same workspace data.
4. Move the exported workspace between systems. Unsupported appearance metadata falls back without dropping organization, and reopening on the source OS preserves the original supported value where the schema permits.

**Tradeoffs and deviation.** A strict pixel-parity target would be cheaper to specify and worse to use. Semantic parity with explicit native adaptation costs more design documentation but avoids making Linux and Windows imitate macOS defects or making macOS ignore its conventions.

## Recommended new narrow issues

The proposed additions are distinct from the current records and can remain small:

| Key | Deliverable | Primary blockers |
| --- | --- | --- |
| `tab-surface-mode` | Bind one existing tab/workspace model to sidebar and top-strip presentations without reload or migration. | `temporary-tabs`, `sidebar-layout`, `tab-status` |
| `workspace-organizer` | Inspect and safely change Profile, Space and window ownership using the approved lifecycle contract. | `lifecycle-contract`, `profiles`, `window-ownership` |
| `workspace-operation-journal` | Represent one workspace mutation and its inverse, then prove grouped close/move/delete undo with interruption recovery. | `session-durability`, `archive` |
| `focus-routing-contract` | Record and fixture the active-surface rules used by split, Peek, browser chrome and page commands. | `lifecycle-contract` |
| `assistant-consent-contract` | Define browser-owned context/effect receipts before any provider, page-read or tool implementation is enabled. | `profiles`, `private-browsing` policy inputs |

These should not absorb the implementation tickets they constrain. The contract issues close with owner-approved scenario tables and synthetic fixtures. The UI issues close with the affected native interaction. Do not create a catch-all "Arc but better" issue.

## Explicit non-goals from this review

- Do not make AI necessary for tab organization, search, cleanup, import, split view or accessibility.
- Do not copy Yab's product limits, timers, memory heuristics, platform requirements, service choices or WebKit performance claims.
- Do not add a remote sync, sharing, integration or model provider without its existing owner decision and authorization path.
- Do not test with real browsing content, credentials or signed-in services. Use public/synthetic fixtures until separately authorized.
- Do not treat static artifact actions, vendor changelog language or a visual match as proof of native behavior or performance.

## First-party sources used for these deltas

### Existing Arc and Dia dossier sources

- [Arc Spaces](https://resources.arc.net/hc/en-us/articles/19228064149143-Spaces-Distinct-Browsing-Areas)
- [Arc Profiles](https://resources.arc.net/hc/en-us/articles/19227964556183-Profiles-Separate-Work-Personal-Browsing)
- [Arc Pinned Tabs](https://resources.arc.net/hc/en-us/articles/19231060187159-Pinned-Tabs-Tabs-you-want-to-stick-around)
- [Arc Auto Archive](https://resources.arc.net/hc/en-us/articles/19228855311127-Auto-Archive-Clean-as-you-go)
- [Arc Keyboard Shortcuts](https://resources.arc.net/hc/en-us/articles/20595231349911-Keyboard-Shortcuts)
- [Arc macOS 2024-2026 release notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc Windows 2023-2026 release notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)
- [Dia changelog](https://www.diabrowser.com/changelog)
- [Dia Security](https://www.diabrowser.com/security)

### Yab inspiration checked for this review

- [Yab product site](https://yetanotherbrowser.com/)
- [The sidebar](https://yetanotherbrowser.com/manual/sidebar)
- [Working with tabs](https://yetanotherbrowser.com/manual/tabs)
- [Tidying and sleeping tabs](https://yetanotherbrowser.com/manual/tidy-and-sleep)
- [Spaces](https://yetanotherbrowser.com/manual/spaces)
- [Split View](https://yetanotherbrowser.com/manual/split-view)
- [Peek](https://yetanotherbrowser.com/manual/peek)
- [The field](https://yetanotherbrowser.com/manual/the-field)
- [Keyboard shortcuts](https://yetanotherbrowser.com/manual/keyboard-shortcuts)
- [Bringing in from another browser](https://yetanotherbrowser.com/manual/bring-in)
- [What agents ask first](https://yetanotherbrowser.com/manual/permissions)
- [Privacy](https://yetanotherbrowser.com/manual/privacy)
- [Sync](https://yetanotherbrowser.com/manual/sync)

Yab's official material currently describes a macOS product. It is a design reference, not Linux/Windows evidence, an independent security audit or a benchmark source.
