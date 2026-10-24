# Arc desktop UX inventory and proposed backlog

Research date: 2026-10-23

Scope: Arc for macOS and Arc for Windows, including documented historical and retired behavior

Target: Timpan4/helium-arcified, with Linux, Windows, and macOS as downstream platforms

This is a documentary source review. It does not claim Arc binary, network, gesture, rendering, sync-protocol, or native Helium verification. The proposed acceptance criteria describe recommended Arcified behavior. They are not claims about Arc unless explicitly marked as documented.

## Evidence conventions and product status

- **D** means a surviving first-party help page or release note documents the behavior.
- **H** means first-party historical material documents behavior whose current status is unclear.
- **X** means a first-party source explicitly records retirement, removal, or replacement.
- **R** means a downstream recommendation for Arcified.
- **U** means the reviewed official sources do not settle the point.

Arc is scheduled to stop working on December 22, 2026. The official sunset material says new account creation ended on October 22, 2026 and that the product receives basic fixes rather than new features until shutdown. This makes the release-note archive important: current help pages have already been pruned, and several exact feature URLs returned 404 during this research. A missing page is not evidence that a feature disappeared before the global sunset.

Arc did not ship a documented Linux desktop product in the reviewed sources. Every Linux behavior below is an Arcified design decision, not an Arc parity claim.

## Primary-source register

### Product status and chronology

- [Sunset key dates and what to expect](https://resources.arc.net/hc/en-us/articles/44412972248471-Arc-Sunset-Key-Dates-and-What-to-Expect)
- [Move from Arc to Dia](https://resources.arc.net/hc/en-us/articles/44413129117591-Move-from-Arc-to-Dia)
- [Arc for macOS 2021 release notes](https://resources.arc.net/hc/en-us/articles/20498463803799-Arc-for-macOS-2021-Release-Notes)
- [Arc for macOS 2022 release notes](https://resources.arc.net/hc/en-us/articles/20498417809815-Arc-for-macOS-2022-Release-Notes)
- [Arc for macOS 2023 release notes](https://resources.arc.net/hc/en-us/articles/20498377604887-Arc-for-macOS-2023-Release-Notes)
- [Arc for macOS 2024-2026 release notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes)
- [Arc for Windows 2023-2026 release notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes)
- [Historical Arc release-note rollup](https://start.arc.net/release-notes)
- [Historical Command Bar actions](https://start.arc.net/command-bar-actions)
- [Historical multitasking overview](https://start.arc.net/master-multitasking)
- [Historical creation overview](https://start.arc.net/paint-the-internet)

### Organization and persistence

- [Spaces](https://resources.arc.net/hc/en-us/articles/19228064149143-Spaces-Distinct-Browsing-Areas)
- [Profiles](https://resources.arc.net/hc/en-us/articles/19227964556183-Profiles-Separate-Work-Personal-Browsing)
- [Favorites](https://resources.arc.net/hc/en-us/articles/19230755904151-Favorites-Top-Tabs-Across-Every-Space)
- [Pinned Tabs](https://resources.arc.net/hc/en-us/articles/19231060187159-Pinned-Tabs-Tabs-you-want-to-stick-around)
- [Folders](https://resources.arc.net/hc/en-us/articles/19228419623447-Folders-Stash-Similar-Tabs-Together)
- [Auto Archive](https://resources.arc.net/hc/en-us/articles/19228855311127-Auto-Archive-Clean-as-you-go)
- [Library](https://resources.arc.net/hc/en-us/articles/19230634389911-Library-A-home-for-your-downloads-archived-tabs-easels-and-more)
- [Arc Sync](https://resources.arc.net/hc/en-us/articles/20272860828823-Arc-Sync)
- [Recovery Cards](https://resources.arc.net/hc/en-us/articles/22370005382167-Recovery-Cards)
- [Air Traffic Control](https://resources.arc.net/hc/en-us/articles/22932014625431-Air-Traffic-Control-Automate-Your-Link-Routing)
- [Automatic GitHub Live Folders](https://resources.arc.net/hc/en-us/articles/22731612065815-Automatic-GitHub-Live-Folders)
- [Live Calendars](https://resources.arc.net/hc/en-us/articles/24158102740631-Live-Calendars)

### Navigation, creation, and sharing

- [Site Search](https://resources.arc.net/hc/en-us/articles/20855018192791-Site-Search-Directly-Search-any-Website)
- [Fast desktop tab switching](https://resources.arc.net/hc/en-us/articles/25619402657303-How-Do-You-Switch-Between-Tabs-Quickly-on-Arc-Desktop)
- [Default search engine](https://resources.arc.net/hc/en-us/articles/25614032197783-How-Do-You-Switch-Your-Default-Search-Engine-on-Arc-for-Desktop)
- [Keyboard Shortcuts](https://resources.arc.net/hc/en-us/articles/20595231349911-Keyboard-Shortcuts)
- [Import from another browser](https://resources.arc.net/hc/en-us/articles/19335089616791-Import-Bookmarks-Logins-History-Extensions-from-Your-Previous-Browser)
- [Download location](https://resources.arc.net/hc/en-us/articles/25614213458711-How-Do-You-Change-Your-Download-Location-in-Arc-for-Desktop)
- [Full-page capture](https://resources.arc.net/hc/en-us/articles/25481392111895-How-To-Take-Full-Page-Screen-Captures-in-Arc)
- [Easels](https://resources.arc.net/hc/en-us/articles/19231142050071-Easels-Capture-Create)
- [Phasing out Arc Notes](https://resources.arc.net/hc/en-us/articles/19233788518039-Phasing-Out-Arc-Notes)
- [Share Spaces, Folders, and Splits](https://resources.arc.net/hc/en-us/articles/19228534606743-Share-Spaces-Folders-Splits-with-Anyone)
- [Arc Max](https://resources.arc.net/hc/en-us/articles/19335160678679-Arc-Max-Boost-Your-Browsing-with-AI)
- [Developer Mode](https://resources.arc.net/hc/en-us/articles/20468488031511-Developer-Mode-Instant-Dev-Tools)

### Retired exact help URLs preserved for reference

- [Little Arc](https://resources.arc.net/hc/en-us/articles/19235387524503-Little-Arc-Quick-Lookups-Instant-Triaging)
- [Peek](https://resources.arc.net/hc/en-us/articles/19335302900887-Peek-Preview-Sites-From-Pinned-Tabs)
- [Mini Player](https://resources.arc.net/hc/en-us/articles/19234766331799-Mini-Player-Watch-or-Listen-as-you-Browse)

## Coverage matrix

| Area | Officially documented behavior and edge cases | Platform evidence | Recommended Arcified disposition | Sources | Unknowns |
|---|---|---|---|---|---|
| Sidebar shell | **D/H:** Replaces the horizontal tab strip. Resizable, hideable, keyboard-focusable, and used for tabs, Spaces, media, downloads, and status. macOS can collapse it by dragging fully left; hidden-sidebar tab creation produces feedback. | Core on Mac and Windows. Detailed resize, focus, hover, and collapse history is Mac-heavy. | **R:** One native shell with platform bindings. Preserve web-content width and keyboard focus when showing or hiding it. | 2021-2024 Mac notes; Windows notes | Exact Windows resize bounds, hover reveal, and focus sequence are not fully documented. |
| Spaces | **D:** Each Space has its own pinned and unpinned tabs, icon, and theme. Create from Sidebar; switch by icon, gesture, numbered shortcut, or Command Bar. Returning to a Space restores its last active tab. Incognito historically has no Spaces. | Mac and Windows. Gesture and shortcut names differ. Sharing and Air Traffic Control are Mac-only. | **R:** Treat Space as a live view over tabs, not a window snapshot. | Spaces; 2021 and 2023 Mac notes; Windows notes | Official sources do not define cross-window ownership uniformly on Windows. |
| Profiles | **D:** Profile is the storage/privacy boundary for logins, cookies, cache, history, extensions, Favorites, default search, download location, archive timing, and Chromium settings. One Profile can back multiple Spaces. New Profiles are empty; deletion requires detaching all Spaces. Profiles do not sync. | Mac and Windows, with different settings entry points. | **R:** Use a first-class profile identifier on every Space and profile-scoped service. Do not infer a Profile from a window. | Profiles; Arc Sync; download and search settings | Password/autofill import parity varies by source browser and platform. |
| Favorites | **D:** Icon-only durable tabs above Space titles, available across Spaces attached to the Profile. Maximum is 12. Drag, context menu, or move command promotes/demotes. | Mac and Windows. | **R:** Model as profile-scoped pinned references, not a fourth independent tab lifecycle. | Favorites; Profiles; Arc Sync | The help wording “every Space” conflicts unless read through Profile scope. Runtime resolution is unverified. |
| Pinned tabs | **D:** Space-scoped and never Auto Archive. Keep an original pinned URL. Same-site navigation stays in the tab; outbound navigation opens Peek. Reset returns to the original URL. macOS supports collapsing the Pinned section and Option-drag duplication. | Core on both. Collapsible section is documented Mac-only. | **R:** Store current navigation separately from immutable pinned target; resetting must not lose the prior page when the platform provides the documented open-new-tab behavior. | Pinned Tabs; 2022-2024 Mac notes; Windows notes | “Same site” origin/eTLD policy is not specified. |
| Today or unpinned tabs | **D/H:** Temporary section below pinned tabs. Historical name changed from Explore to Unpinned to Today. Idle entries Auto Archive. Tidy Tabs operates only here. Mac notes say Today Tabs are unique per window. | Both have the section. Window ownership is explicit only on Mac. | **R:** Keep one semantic type with display-label aliases. Make window ownership a deliberate cross-platform choice. | Auto Archive; 2023-2024 Mac notes; Windows notes; Arc Max | Current Windows multi-window semantics are unclear. |
| Folders | **D:** Nested durable organization for pinned tabs; contents do not Auto Archive. Mac supports collapsed-folder search/preview and showing the active child below a closed folder. | Basic folders on Mac and Windows. Preview and sharing are Mac-only in reviewed docs. | **R:** Build a nested tree with cycle prevention and an active-descendant projection that does not mutate folder expansion. | Folders; 2022-2024 Mac notes; Windows notes | Maximum nesting depth and drag conflict rules are undocumented. Do not invent a cap. |
| Archive | **D:** Auto Archive uses a Profile-specific cadence, documented default 12 hours, resets on view, and cannot be disabled. Pinned/folder tabs are exempt; Mac release notes exempt active media/recording. Restore differs by platform. Little Arc entries can be filtered on Mac. | Both. Mac Library integrates Archive; Windows uses a dedicated route. | **R:** Separate automatic archive, explicit close, restore target, and history. Preserve Space provenance and allow undo. | Auto Archive; Library; 2022-2024 Mac notes; Windows notes | Current selectable cadence sets differ. Windows notes once documented a 30-day option. |
| Command Bar | **D/H:** Unified URL/search, tab switcher, and action launcher via Command/Control-T. Actions create and move objects, manage splits, invoke extensions, and open settings/history/archive. Immediate Tab filters Mac actions. Suggestions update when switching Spaces. The action tutorial was removed, not actions. | Both, with richer historical Mac action documentation. | **R:** Use typed result providers and one action registry. Keep ranking, execution, and display separate. | Command Bar actions; 2021-2024 Mac notes; Windows notes | Exact provider ordering, ranking, deduplication, and Windows action-filter gesture are undocumented. |
| Search and navigation | **D:** Default engine is Profile-scoped. Site Search uses name, keyword, and a `%s` URL template, then keyword + Tab + query. Recent tab switcher covers the five most recent tabs; ordered traversal follows Sidebar order. | Site Search is Mac-only. Core search and tab switching are both platforms. | **R:** Implement URL detection, default search, tab matching, and action search first. Put Site Search behind a portable provider contract. | Site Search; default search; tab switching | Search suggestion privacy, retention, and network provider policy are not specified by UX docs. |
| Peek | **D/H:** Temporary overlay from links in Pinned Tabs/Favorites or Shift-click. Dismiss, expand to tab, or convert to Split. Windows documents outside-click, X, Esc, Control-O, and settings toggles. Mac routes destinations through Air Traffic Control. | Mac and Windows. | **R:** Make Peek an explicit transient surface with parent-tab ownership and promotion paths. It must not silently become a durable tab on dismissal. | Peek URL; Pinned Tabs; 2023-2024 Mac notes; Windows notes | Focus trapping, history retention, download behavior, and crash restoration are not fully specified. |
| Little Arc | **D/H:** macOS quick-lookup window outside Spaces. External links originally defaulted there. Command-Option-N creates one; Open In moves it to a Space. It can be disabled or replaced by Air Traffic Control routing. Closed pages later appear in Archive. | Mac evidence only. No official Windows rollout found. | **R:** Treat as optional cross-platform external-link triage, distinct from Peek and normal windows. | Little Arc URL; Air Traffic Control; 2021-2024 Mac notes | Windows and Linux product choice; exact Profile-selection heuristic beyond remembered domain behavior. |
| External-link routing | **D:** Air Traffic Control can route by URL contains/equality to Little Arc, most recent Space, or fixed Space. Deleting a Space deletes its rules. Peek honors routing. | Mac-only. | **R:** Add after Spaces/Profiles/Little Arc. Show the resolved destination before saving a rule and clean up dangling targets. | Air Traffic Control; 2024 Mac notes | Rule priority and overlap resolution are not documented. |
| Split View | **D/H:** Several sites form one Sidebar item that can be pinned, favorited, renamed, moved, reordered, and resized. Create by drag, Command Bar, tab-on-tab, or Peek promotion. Horizontal and vertical layouts exist. | Mac and Windows. Windows reached features later. | **R:** Model a split as a durable ordered tree or layout object whose panes point to tabs. Do not duplicate tab lifecycle state. | Multitasking overview; 2021-2024 Mac notes; Windows notes | A Mac fix mentions opening more than four panes, but no authoritative UX contract states a hard limit. Do not infer one. |
| Tabs and windows | **D/H:** Control-Tab uses recency; Sidebar shortcuts use order. Mac later used one live instance per durable tab across windows and restored position with Tab Handoff. Each Mac window owns Today Tabs while Pinned Tabs are shared. Blank Window and Incognito differ from a normal window. | Mac semantics are detailed. Windows documents drag between windows and session restore but not the complete ownership rule. | **R:** Specify tab identity, view attachment, Today ownership, and durable-tab handoff before implementation. | Tab switching; 2021-2024 Mac notes; Windows notes | Windows parity, multi-monitor restoration, and split handoff semantics need runtime/design work. |
| Themes and materials | **D:** Per-Space theme. Mac supports presets, custom color, intensity, grain, dark variants, plus app Light/Dark/Automatic. Theme colors propagate into controls. Windows documents Space colors and Mica/Acrylic. | Platform-specific materials. | **R:** Share semantic theme tokens; implement native material adapters with an opaque fallback. | Spaces; 2022-2024 Mac notes; Windows notes | Exact color transforms and contrast adjustments are undocumented. |
| Motion, sound, and haptics | **D/H:** Arc uses animations for tab movement, sidebar transitions, Peek, loading, media, and privacy indicators. Mac has haptics and a setting to disable Arc-generated sounds. | Primarily Mac; Windows has its own native motion/material behavior. | **R:** Centralize motion and feedback policies. Honor OS reduced-motion and accessibility settings even though Arc docs do not state this. | 2022-2024 Mac notes; Windows notes | **U:** No official reduced-motion contract or formal animation-free mode was found. |
| Library | **D:** Mac Library has Media, Downloads, Easels & Notes, Spaces, and Archived Tabs. Hover exposes configurable recent files. Deletion later moved files to OS Trash. | Help article is Mac-only. Windows has downloads/archive surfaces, not the same five-section Library contract. | **R:** Start with an activity/library shell whose data providers can be enabled independently. Use recoverable file deletion. | Library; 2022-2024 Mac notes; Windows notes | Linux file-manager integration and whether to reproduce the exact five sections are downstream choices. |
| Downloads | **D:** Active progress, cancellation, history, configurable Profile destination, drag-out, and Library/downloads view. Tidy Downloads renames files with undo. | Both, with distinct surfaces and shortcuts. | **R:** Reuse Chromium download state. Never let tab cleanup cancel or hide an active transfer. Keep rename undoable. | Library; download location; Arc Max; Mac and Windows notes | Collision naming, failed rename recovery, and Tidy data handling need product decisions. |
| Media and Mini Player | **D/H:** Leaving an active media tab can show video PiP or sidebar audio controls. Controls include play/pause, return to source, movement/resizing, and per-site disable. Meet gained automatic PiP. Media and recording tabs were protected from archive/suspension. | Both have media features, but controls evolved separately. | **R:** Build protection from cleanup first, then player UI. Make automatic PiP opt-out and expose source tab. | Mini Player URL; 2021-2024 Mac notes; Windows notes | Site compatibility, DRM, multi-player priority, and OS media-session integration require platform verification. |
| Previews and live integrations | **D/H:** Five-second Max previews summarize links. Historical site previews and GitHub Previews existed. GitHub Previews were replaced by automatic GitHub Live Folders. Live Calendars animate a favorited Google Calendar with countdown and Join. | Live Calendar and GitHub Live Folders are documented on Mac and Windows. Max five-second previews are Mac-only. | **R:** Treat each as an optional integration provider, not Sidebar core. Require explicit account/site permission and a disable path. | Arc Max; Live Calendars; GitHub Live Folders; 2024 Mac and Windows notes | Auth scopes, refresh cadence, offline behavior, and non-GitHub.com support are incomplete; GitHub Enterprise is explicitly unsupported by Arc. |
| Boosts | **D/H/X:** Original builder allowed CSS and JavaScript. V2 added no-code styling, element removal, Shuffle, sharing, and gallery. Legacy builder retired in October 2024. In September 2024 all JS Boosts were disabled and require per-device re-enable; a global disable setting was added. | Official rollout evidence is Mac-only. | **R:** If retained, ship declarative CSS/element rules first. Treat script execution as a separate security project with per-origin, per-device consent and a global kill switch. | 2022-2024 Mac notes; historical release notes | Windows/Linux support, sync format, CSP interaction, and script threat model are unresolved. |
| Easels and captures | **D/H:** Mac-only editable whiteboard with text, drawing, media, captures, video, alignment, presence, Live Captures, sharing, PNG export, and Library storage. Viewable in any browser; only Mac Arc edits. Full-page capture is Mac-only. | Mac authoring. Browser viewing is cross-platform. | **R:** Defer authoring unless it serves a validated Arcified goal. Preserve import/export compatibility before collaboration. | Easels; full-page capture; 2022-2024 Mac notes | Storage format, collaboration protocol, and sunset data portability are not documented sufficiently. |
| Arc Notes | **X:** Native notes were phased out: export warning March 14, 2024; view-only April 4; mandatory HTML ZIP export April 18. New Note could route to a configured external document service. | Mac-only historical feature. | **R:** Do not rebuild the retired editor by default. If migration data exists, provide read/import/export and external-provider routing. | Notes retirement; Library; 2024 Mac notes | Exact legacy note schema and shared-note migration are unknown. |
| Sharing | **D:** Mac snapshot links for Spaces, folders, Splits, or selected tabs. Arc recipients can add them; other browsers see a web view. Snapshots do not live-update. Authentication is not transferred. Arc said links could not be self-revoked. Share Quote creates a link/image and can be disabled. | Collection sharing is explicitly not Windows. Easel viewing works in browsers. | **R:** Preserve snapshot semantics but require owner-visible deletion/revocation for any Arcified service. Strip secrets and never imply private-page access transfers. | Sharing; Easels; Notes retirement; 2024 Mac notes | Service hosting, expiration, abuse handling, and encryption are outside the UX docs. |
| Arc Max | **D/H/X:** Per-feature toggles. Mac features: five-second previews, Tidy Tab Titles, Tidy Downloads, ChatGPT in Command Bar, Instant Links. Tidy Tabs is cross-platform and appears only above six Today Tabs. Windows shipped Tidy Tabs and Ask on Page. Ask on Page was removed from Mac Max in August 2025; current Max docs omit it. Features send data to AI partners. | Most Max features Mac-only; Tidy Tabs documented cross-platform. Windows Ask on Page history differs. | **R:** Use an opt-in provider boundary with visible context selection, source disclosure, cancellation, and no silent page upload. Keep deterministic tab/download transforms separate from generative features. | Arc Max; 2024-2026 Mac notes; Windows notes | Current Windows Ask on Page status, model/provider details, retention, and exact prompt context are not settled. |
| Onboarding and import | **D/H:** Import supports bookmarks, logins, history, and extensions depending on source browser. Imported bookmarks become Pinned Tabs. Mac later supported multi-Profile import; Windows added an Arc-style unboxing flow and Profile creation. | Both; source-browser support varies. Account-dependent Arc onboarding is becoming historical because new accounts are disabled. | **R:** Make local use possible without an Arc service. Preview import mapping and report unsupported fields without dropping source data. | Import help; 2023 Mac notes; Windows notes; sunset | Import coverage by browser/version and conflict policy need focused tests. |
| Settings and shortcuts | **D:** Command/Control-Comma opens settings. Profile, links, previews, Max, appearance, account/sync, privacy, and advanced settings evolved over time. Mac supports shortcut remapping, function keys, and Prefer Arc/Prefer Websites conflict handling. Custom shortcuts do not sync. | Both have settings; deep shortcut remapping is documented most clearly on Mac. | **R:** Use one settings schema with platform-specific bindings. Detect conflicts, support reset, and keep a searchable action catalog. | Keyboard Shortcuts; 2022-2024 Mac notes; Windows notes; Arc Sync | Complete current shortcut inventory and Windows remapping parity are not available from surviving docs. |
| Accessibility and input | **D/H:** Keyboard sidebar focus, ordered tab traversal, shortcut tooltips, page zoom, light/dark modes, contrast fixes, privacy indicators, Windows touch support, native context menus, and richer Accessibility Insights/Inspect.exe metadata are documented. | Windows has explicit AT-inspection improvements; Mac sources do not provide a formal VoiceOver statement. | **R:** Require semantic tree coverage, full keyboard operation, visible focus, screen-reader names/states, OS high-contrast and reduced-motion support, and touch targets where applicable. | 2021-2024 Mac notes; Windows notes; Keyboard Shortcuts | **U:** No official WCAG claim, VoiceOver/Narrator test matrix, landmark contract, or reduced-motion behavior was found. |
| Sync and recovery | **D:** End-to-end encrypted Arc Sync carries Spaces, folders, and tabs. It excludes history, passwords, extensions, Favorites, Profiles, shortcuts, Air Traffic Control, and mobile-created tabs. Recovery Card is needed after password change without an already trusted device. iCloud sync retired in June 2024. | Mac and Windows. | **R:** Specify local-first behavior and a threat model before remote sync. Sync stable identifiers and operations, not window snapshots. Never sync excluded secrets accidentally. | Arc Sync; Recovery Cards; 2024 Mac notes | Arc server protocol, conflict-resolution rules, deletion tombstones, and shutdown export format are not public UX contracts. |
| Developer and Chromium affordances | **D:** Developer Mode exposes full URL, dev shortcuts, visual focus treatment, JSON formatting, auto-localhost behavior, and portrait captures. Extensions, DevTools, toolbar, site controls, translations, passwords, and Chromium settings remain part of desktop UX. | Developer Mode help is Mac-only; Chromium affordances exist on both. | **R:** Preserve Chromium capabilities and expose Arcified additions through native extension points. Do not hide security or permission state behind theme chrome. | Developer Mode; Mac and Windows notes | PWA policy and exact toolbar parity are adjacent gaps, not settled by this inventory. |

## Proposed GitHub tickets

These are publication-ready briefs, not created issues. Acceptance criteria are downstream recommendations.

### ARC-UX-001: Define the native Sidebar shell

**Behavior:** Implement the persistent vertical shell, resize/hide/reveal behavior, keyboard focus entry, scrolling, active-tab reveal, bottom controls, and web-content resizing.

**Acceptance criteria:**

- Hiding or showing the Sidebar preserves the active web page and focus destination.
- Keyboard navigation reaches every visible Sidebar control in visual order and returns focus to content predictably.
- Resizing never places controls over web content; full collapse and restoration are reversible.
- Active privacy, media, and download states remain perceivable when the Sidebar is hidden.

**Edges:** full screen, narrow windows, many tabs, command bar open, active tab outside the scroll viewport, RTL text, screen readers.

**Dependencies:** native browser view integration and semantic tab model.

**Platforms:** shared behavior; native window chrome and shortcuts per OS.

**Unknowns:** Windows hover/reveal details and platform-specific minimum layout remain design work.

**Sources:** [2021 Mac notes](https://resources.arc.net/hc/en-us/articles/20498463803799-Arc-for-macOS-2021-Release-Notes); [2024-2026 Mac notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes); [Keyboard Shortcuts](https://resources.arc.net/hc/en-us/articles/20595231349911-Keyboard-Shortcuts).

### ARC-UX-002: Model Spaces as live browsing areas

**Behavior:** Create, rename, icon/theme, reorder, switch, and delete Spaces while retaining each Space's active tab.

**Acceptance criteria:**

- Switching changes the Space-scoped pinned and Today views without recreating tabs.
- Every Space references exactly one Profile; multiple Spaces may reference the same Profile.
- Deleting a Space shows the tabs and routing rules that will be affected and provides an undo path while data is recoverable.
- Incognito behavior is explicitly separate and does not expose normal Spaces.

**Edges:** empty Spaces, same Profile across Spaces, deletion with open splits, cross-window switch, unsynced changes.

**Dependencies:** ARC-UX-003, ARC-UX-004.

**Platforms:** gestures and numbered shortcuts use native bindings; Linux needs a documented alternative to trackpad switching.

**Unknowns:** whether downstream Spaces are shared identically across all windows.
**Sources:** [Spaces](https://resources.arc.net/hc/en-us/articles/19228064149143-Spaces-Distinct-Browsing-Areas); [2023 Mac notes](https://resources.arc.net/hc/en-us/articles/20498377604887-Arc-for-macOS-2023-Release-Notes); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes).

### ARC-UX-003: Enforce Profile isolation and Space assignment

**Behavior:** Scope cookies, logins, history, cache, extensions, Favorites, download/search preferences, and archive timing to a Profile.

**Acceptance criteria:**

- Opening the same site in different Profiles uses separate Chromium storage partitions.
- Assigning a Space to another Profile clearly states which live tabs reload or close.
- A Profile cannot be deleted while any Space references it.
- Profile data never crosses into Command Bar suggestions or Little Arc selection without an explicit routing decision.

**Edges:** default Profile rename, last Profile, extension availability, active downloads, shared URLs.

**Dependencies:** Chromium profile/storage integration.

**Platforms:** same isolation contract on Linux, Windows, and macOS.

**Unknowns:** migration semantics for changing an existing Space's Profile.
**Sources:** [Profiles](https://resources.arc.net/hc/en-us/articles/19227964556183-Profiles-Separate-Work-Personal-Browsing); [Arc Sync](https://resources.arc.net/hc/en-us/articles/20272860828823-Arc-Sync); [download location](https://resources.arc.net/hc/en-us/articles/25614213458711-How-Do-You-Change-Your-Download-Location-in-Arc-for-Desktop).

### ARC-UX-004: Implement Favorite, pinned, and Today tab lifecycles

**Behavior:** Represent Favorites as Profile-scoped durable references, Pinned Tabs as Space-scoped durable tabs with an original URL, and Today tabs as disposable window content.

**Acceptance criteria:**

- Promotion and demotion preserve history and tab identity.
- Resetting a Pinned Tab restores its original URL without confusing reset with Back.
- Pinned and Favorite tabs never Auto Archive; Today tabs do.
- External navigation from a durable tab follows the configured Peek rule.
- The documented Favorite maximum of 12 is enforced with a clear explanation because it comes from Arc's contract.

**Edges:** same-site definition, renamed tabs, duplicate URLs, collapsed Pinned section, active media, tab moved across Profiles.

**Dependencies:** ARC-UX-002, ARC-UX-003, ARC-UX-009.

**Platforms:** Mac-only duplication gesture gets native alternatives elsewhere.

**Unknowns:** resolve the official “every Space” wording through Profile tests before claiming exact parity.
**Sources:** [Favorites](https://resources.arc.net/hc/en-us/articles/19230755904151-Favorites-Top-Tabs-Across-Every-Space); [Pinned Tabs](https://resources.arc.net/hc/en-us/articles/19231060187159-Pinned-Tabs-Tabs-you-want-to-stick-around); [Auto Archive](https://resources.arc.net/hc/en-us/articles/19228855311127-Auto-Archive-Clean-as-you-go).

### ARC-UX-005: Add nested folders and active-descendant projection

**Behavior:** Create, rename, move, nest, collapse, expand, and delete folders; search/select a child without requiring expansion.

**Acceptance criteria:**

- Folder moves cannot create cycles or lose tabs.
- A child selected from a collapsed folder is shown as active without mutating saved expansion state.
- Folder contents are exempt from Auto Archive.
- Deleting a non-empty folder states whether children will be moved, archived, or deleted and supports undo.

**Edges:** deep nesting, drag between Spaces, mixed splits/tabs, active child during delete, sync conflict.

**Dependencies:** ARC-UX-004 and the persistence layer.

**Platforms:** preview UI may adapt, but the tree semantics stay shared.

**Unknowns:** no official nesting cap or conflict rule; do not invent either.
**Sources:** [Folders](https://resources.arc.net/hc/en-us/articles/19228419623447-Folders-Stash-Similar-Tabs-Together); [2023 Mac notes](https://resources.arc.net/hc/en-us/articles/20498377604887-Arc-for-macOS-2023-Release-Notes); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes).

### ARC-UX-006: Build Archive, cleanup, restore, and undo

**Behavior:** Auto Archive eligible Today tabs using a Profile setting; archive explicit closes with provenance; restore to the original Space when possible.

**Acceptance criteria:**

- Viewing a Today tab resets its idle time as documented.
- Active downloads, media, recording, and unsaved-form risk are protected by explicit policy.
- Restore handles a deleted Space by asking for a destination instead of losing the entry.
- Clear Archive requires confirmation and reports whether underlying downloaded files are untouched.
- Archive records can distinguish Little Arc, explicit close, and automatic cleanup where known.

**Edges:** device clock changes, sleeping devices, cross-device cadence mismatch, private windows, browser crash.

**Dependencies:** ARC-UX-002 through ARC-UX-005.

**Platforms:** one model, platform-native restore surface.

**Unknowns:** choose supported cadence values through product policy; Arc's current lists differ by platform.
**Sources:** [Auto Archive](https://resources.arc.net/hc/en-us/articles/19228855311127-Auto-Archive-Clean-as-you-go); [Library](https://resources.arc.net/hc/en-us/articles/19230634389911-Library-A-home-for-your-downloads-archived-tabs-easels-and-more); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes).

### ARC-UX-007: Create a typed Command Bar provider and action system

**Behavior:** Combine URL navigation, web search, open-tab switching, object lookup, and browser actions without conflating their state.

**Acceptance criteria:**

- Results display type, destination Profile/Space when relevant, and shortcut.
- Keyboard selection and execution work without pointer input.
- Providers can cancel stale queries and cannot execute while the user is still composing.
- Space switches refresh scoped suggestions without leaking results from another Profile.
- The action registry is shared with menus and shortcut settings.

**Edges:** invalid URLs, IME/non-Latin input, duplicate suggestions, deleted targets, command bar open during Sidebar click.

**Dependencies:** tab, Space, Profile, history, and settings query APIs.

**Platforms:** Command/Control bindings; behavior shared.

**Unknowns:** exact Arc ranking is undocumented and should not be claimed.
**Sources:** [Command Bar actions](https://start.arc.net/command-bar-actions); [2024-2026 Mac notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes).

### ARC-UX-008: Implement search, Site Search, and tab navigation

**Behavior:** Add default engine selection, URL/search disambiguation, recent-tab switching, Sidebar-order traversal, and optional keyword Site Search.

**Acceptance criteria:**

- Search uses the active Profile's engine and does not send text until the user commits.
- Site Search validates that its template contains `%s` and previews the final URL.
- Recent switching and ordered traversal remain distinct commands.
- Deleted suggestion data is removed from the correct local history store.

**Edges:** search keywords that resemble URLs, spaces in templates, private mode, dead tabs, shortcut conflicts.

**Dependencies:** ARC-UX-003 and ARC-UX-007.

**Platforms:** Site Search is a downstream portability decision because Arc documented it only on Mac.

**Unknowns:** provider ranking and remote suggestion retention.
**Sources:** [Site Search](https://resources.arc.net/hc/en-us/articles/20855018192791-Site-Search-Directly-Search-any-Website); [default search engine](https://resources.arc.net/hc/en-us/articles/25614032197783-How-Do-You-Switch-Your-Default-Search-Engine-on-Arc-for-Desktop); [tab switching](https://resources.arc.net/hc/en-us/articles/25619402657303-How-Do-You-Switch-Between-Tabs-Quickly-on-Arc-Desktop).

### ARC-UX-009: Add Peek as a transient child surface

**Behavior:** Open selected links over the parent page; dismiss, expand, or promote to Split without changing parent history.

**Acceptance criteria:**

- Focus enters Peek, stays within its controls/content while open, and returns to the invoking element on dismissal.
- Outside-click, close, Escape, expand, and Split actions produce one deterministic outcome.
- Downloads, permissions, authentication popups, and external protocols are handled visibly.
- Promotion preserves Peek history and Profile while removing transient ownership.

**Edges:** nested popup attempts, full screen, parent closes, routed links, unsaved input, multiple windows.

**Dependencies:** ARC-UX-003, ARC-UX-004, ARC-UX-011.

**Platforms:** consistent semantics with native close/expand shortcuts.

**Unknowns:** exact Arc crash-restoration and focus behavior need runtime evidence.
**Sources:** [retired Peek URL](https://resources.arc.net/hc/en-us/articles/19335302900887-Peek-Preview-Sites-From-Pinned-Tabs); [Pinned Tabs](https://resources.arc.net/hc/en-us/articles/19231060187159-Pinned-Tabs-Tabs-you-want-to-stick-around); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes).

### ARC-UX-010: Add Little Arc and external-link routing

**Behavior:** Provide an optional quick-lookup window for external links plus rule-based routing to a Profile/Space.

**Acceptance criteria:**

- The user can choose Little Arc, most recent Space, or a fixed Space as the default destination.
- Contains and exact-match rules show ordering and the matched rule before navigation.
- “Open In” promotes the page without losing history or choosing a different Profile silently.
- Deleting a destination Space removes or repairs its rules and reports the change.
- Closed Little Arc pages can be recovered from Archive when history policy permits.

**Edges:** overlapping rules, authentication callbacks, external app links, multiple matching Profiles, private mode.

**Dependencies:** ARC-UX-002, ARC-UX-003, ARC-UX-006.

**Platforms:** Arc evidence is Mac-only; Windows/Linux support is an explicit downstream choice.

**Unknowns:** remembered domain-to-Profile heuristic and rule precedence.
**Sources:** [retired Little Arc URL](https://resources.arc.net/hc/en-us/articles/19235387524503-Little-Arc-Quick-Lookups-Instant-Triaging); [Air Traffic Control](https://resources.arc.net/hc/en-us/articles/22932014625431-Air-Traffic-Control-Automate-Your-Link-Routing); [2021 Mac notes](https://resources.arc.net/hc/en-us/articles/20498463803799-Arc-for-macOS-2021-Release-Notes).

### ARC-UX-011: Define Split View as a durable layout object

**Behavior:** Create, reorder, resize, orient, pin, favorite, rename, move, and dissolve splits.

**Acceptance criteria:**

- Each pane references one tab identity; moving a Split does not clone browsing state.
- Drag indicators state the resulting order and orientation before drop.
- Removing the last pane yields a normal tab without losing history.
- Peek-to-Split preserves Profile and navigation state.
- Keyboard users can select, resize, reorder, and close panes.

**Edges:** mixed Profiles, fullscreen media, pane crash, active download, moving split across Spaces.

**Dependencies:** ARC-UX-004 and native multi-web-view support.

**Platforms:** support orientations appropriate to the window while sharing persistence.

**Unknowns:** official hard pane limit is not established; choose one only from technical/product evidence.
**Sources:** [multitasking overview](https://start.arc.net/master-multitasking); [2022 Mac notes](https://resources.arc.net/hc/en-us/articles/20498417809815-Arc-for-macOS-2022-Release-Notes); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes).

### ARC-UX-012: Specify tab ownership across windows and session restore

**Behavior:** Decide how one durable tab appears in several windows, how Today tabs are owned, and how sessions restore.

**Acceptance criteria:**

- The data model distinguishes tab identity from a window attachment.
- Activating a durable tab in another window follows one documented handoff policy.
- Today tabs restore to their owning window and never leak into unrelated Profiles.
- Blank and Incognito windows have distinct, tested persistence rules.
- Crash restore handles missing displays and invalid previous bounds.

**Edges:** split tabs, multi-monitor changes, OS restart, simultaneous close, multiple active views.

**Dependencies:** ARC-UX-002 through ARC-UX-004 and session storage.

**Platforms:** Mac Tab Handoff is evidence, not an automatic cross-platform requirement.

**Unknowns:** official Windows ownership semantics.
**Sources:** [tab switching](https://resources.arc.net/hc/en-us/articles/25619402657303-How-Do-You-Switch-Between-Tabs-Quickly-on-Arc-Desktop); [2023 Mac notes](https://resources.arc.net/hc/en-us/articles/20498377604887-Arc-for-macOS-2023-Release-Notes); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes).

### ARC-UX-013: Implement semantic themes, native materials, and motion policy

**Behavior:** Per-Space visual tokens, application appearance, OS material adapters, motion, sound, and optional haptics.

**Acceptance criteria:**

- Theme changes update Sidebar, Command Bar, Split controls, toasts, and overlays through semantic tokens.
- Text and focus indicators remain legible in every supported appearance.
- OS reduced-motion and high-contrast preferences override decorative effects.
- Sound and haptic feedback can be disabled without suppressing visual state.
- Unsupported materials fall back to an opaque, accessible surface.

**Edges:** transparent windows, HDR, remote desktop, battery saver, theme sync, custom accent extremes.

**Dependencies:** ARC-UX-001.

**Platforms:** Mica/Acrylic on Windows, native vibrancy where supported on macOS, defined Linux fallback.

**Unknowns:** Arc's exact color transformation and reduced-motion behavior.
**Sources:** [Spaces](https://resources.arc.net/hc/en-us/articles/19228064149143-Spaces-Distinct-Browsing-Areas); [2022 Mac notes](https://resources.arc.net/hc/en-us/articles/20498417809815-Arc-for-macOS-2022-Release-Notes); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes).

### ARC-UX-014: Build Library and download/media activity surfaces

**Behavior:** Provide modular Media, Downloads, created items, Space organizer, and Archive views without making the Mac-only five-section layout mandatory.

**Acceptance criteria:**

- Active downloads show progress, cancel/retry where Chromium supports it, and remain protected from tab cleanup.
- Dragging a downloaded item uses a real completed file; partial files cannot be exposed as complete.
- Deleting a local file uses the OS recoverable deletion facility when available and states the effect.
- Recent-file hover/flyout content follows user-selected categories.
- Archive and download history remain distinct from filesystem contents.

**Edges:** renamed/moved files, missing files, huge histories, active video download, private browsing.

**Dependencies:** Chromium download manager, ARC-UX-006, platform file services.

**Platforms:** native file reveal/trash behavior; Windows need not mimic Mac Library chrome.

**Unknowns:** final provider set and Linux desktop integration.
**Sources:** [Library](https://resources.arc.net/hc/en-us/articles/19230634389911-Library-A-home-for-your-downloads-archived-tabs-easels-and-more); [download location](https://resources.arc.net/hc/en-us/articles/25614213458711-How-Do-You-Change-Your-Download-Location-in-Arc-for-Desktop); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes).

### ARC-UX-015: Protect active media and add an optional Mini Player

**Behavior:** Prevent cleanup/suspension of active media/recording tabs, then add video PiP and sidebar media controls.

**Acceptance criteria:**

- Media, capture, microphone, and screen-sharing state blocks destructive automatic cleanup.
- Mini Player identifies and returns to the source tab.
- Automatic PiP and per-site behavior have visible settings.
- Competing players use a deterministic selection policy derived from Chromium media sessions.
- Dismissing video can continue audio only when the source and user setting allow it.

**Edges:** DRM, multiple audible tabs, Meet calls, full screen, tab move, crashed renderer.

**Dependencies:** ARC-UX-001, ARC-UX-004, Chromium media APIs.

**Platforms:** use native PiP/media-session capabilities and report unsupported paths.

**Unknowns:** site compatibility and licensed DRM behavior require real builds.
**Sources:** [retired Mini Player URL](https://resources.arc.net/hc/en-us/articles/19234766331799-Mini-Player-Watch-or-Listen-as-you-Browse); [2022 Mac notes](https://resources.arc.net/hc/en-us/articles/20498417809815-Arc-for-macOS-2022-Release-Notes); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes).

### ARC-UX-016: Define a safe Boost format and builder

**Behavior:** Decide whether Arcified supports declarative styling only or also script execution.

**Acceptance criteria:**

- Declarative Boosts are origin-scoped, reviewable, individually disableable, and globally disableable.
- Imported legacy JavaScript Boosts start disabled and never auto-enable on another device.
- Any script-capable design has an approved threat model, permission UI, CSP interaction, and per-device consent before implementation.
- Removed elements are reversible from site controls even if the editor cannot load.
- Builder retirement/import behavior is documented for legacy files.

**Edges:** origin changes, iframes, page CSP, extensions, sync, compromised shared Boost.

**Dependencies:** site controls, settings, optional sync.

**Platforms:** no Windows Arc parity claim; a shared safe format is recommended.

**Unknowns:** legacy serialization and gallery/share service.
**Sources:** [historical release notes](https://start.arc.net/release-notes); [2022 Mac notes](https://resources.arc.net/hc/en-us/articles/20498417809815-Arc-for-macOS-2022-Release-Notes); [2024-2026 Mac notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes).

### ARC-UX-017: Decide Easel and capture compatibility scope

**Behavior:** Make a conscious keep, compatibility-only, or reject decision for Easels and Live Captures.

**Acceptance criteria:**

- The decision records user value, storage/export requirements, and why it is in or out of Arcified scope.
- If compatibility is chosen, existing artifacts can be opened or exported without requiring Arc services.
- Full-page and region capture disclose destination and never upload by default.
- Remote embeds and Live Captures are inert until trusted and can be frozen to a static export.

**Edges:** untrusted video embeds, missing source pages, collaboration links, huge canvases, offline use.

**Dependencies:** ARC-UX-014 and a portable artifact format.

**Platforms:** Arc authoring was Mac-only; downstream portability is a new design.

**Unknowns:** artifact schema and collaboration protocol.
**Sources:** [Easels](https://resources.arc.net/hc/en-us/articles/19231142050071-Easels-Capture-Create); [full-page capture](https://resources.arc.net/hc/en-us/articles/25481392111895-How-To-Take-Full-Page-Screen-Captures-in-Arc); [historical creation overview](https://start.arc.net/paint-the-internet).

### ARC-UX-018: Preserve Notes migration without rebuilding retired Notes

**Behavior:** Recognize exported Arc Notes and route new-note actions to a configured provider or local format.

**Acceptance criteria:**

- Arc's HTML ZIP export can be inspected and imported without network access when technically feasible.
- Import never overwrites existing documents without confirmation.
- New Note clearly names the destination provider before sending content.
- No UI implies that retired Arc sharing links remain live or editable.

**Edges:** malformed archives, duplicate notes, attachments, unsupported HTML, external-provider failure.

**Dependencies:** import framework and document-provider settings.

**Platforms:** shared compatibility path; no Mac-only editor reconstruction.

**Unknowns:** original schema and attachments beyond exported HTML.
**Sources:** [Notes retirement](https://resources.arc.net/hc/en-us/articles/19233788518039-Phasing-Out-Arc-Notes); [Library](https://resources.arc.net/hc/en-us/articles/19230634389911-Library-A-home-for-your-downloads-archived-tabs-easels-and-more); [2024-2026 Mac notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes).

### ARC-UX-019: Design revocable snapshot sharing

**Behavior:** Share a snapshot of selected tabs, folder, Space, or Split while improving on Arc's non-revocable links.

**Acceptance criteria:**

- Preview lists every URL/title that will leave the device and excludes cookies, page content, and credentials by default.
- Snapshot links do not grant authenticated access and do not silently update.
- Owners can list and revoke shared artifacts; revocation is effective at the service boundary.
- Importing a snapshot creates independent local objects and reports unsupported items.
- Abuse, deletion, and retention policies are visible before publish.

**Edges:** private URLs, file URLs, intranet hosts, deleted Spaces, mixed Profiles, stale snapshots.

**Dependencies:** a separately approved sharing service and ARC-UX-002/005/011.

**Platforms:** Arc only documented authoring on Mac; downstream can choose portable authoring.

**Unknowns:** hosting, identity, encryption, and service ownership.
**Sources:** [sharing](https://resources.arc.net/hc/en-us/articles/19228534606743-Share-Spaces-Folders-Splits-with-Anyone); [Easels](https://resources.arc.net/hc/en-us/articles/19231142050071-Easels-Capture-Create); [Notes retirement](https://resources.arc.net/hc/en-us/articles/19233788518039-Phasing-Out-Arc-Notes).

### ARC-UX-020: Build an opt-in intelligence boundary for Max-like features

**Behavior:** Separate deterministic helpers from networked AI features and require explicit context selection before page data leaves the browser.

**Acceptance criteria:**

- Tidy Tab Titles, Tidy Downloads, and deterministic grouping can operate locally where feasible and are independently toggleable.
- Networked features show provider, selected tabs/page context, and send action before transmission.
- Every request supports cancellation and failure without altering browsing state.
- Enterprise and user settings can disable the entire capability and each feature.
- Removed Arc features such as Ask on Page remain an explicit reject/defer decision, not a hidden backlog assumption.

**Edges:** private/incognito pages, form data, paywalls, authenticated pages, provider outage, renamed download collisions.

**Dependencies:** provider abstraction, settings, privacy review, ARC-UX-007.

**Platforms:** shared consent model; shortcuts and local model availability vary.

**Unknowns:** provider/model, retention, cost, offline support, and current Windows Ask on Page status.
**Sources:** [Arc Max](https://resources.arc.net/hc/en-us/articles/19335160678679-Arc-Max-Boost-Your-Browsing-with-AI); [2024-2026 Mac notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes).

### ARC-UX-021: Create local-first onboarding, import, settings, and shortcuts

**Behavior:** Onboard without an Arc account, import supported browser data, configure Profiles/Spaces, and expose searchable settings and remappable actions.

**Acceptance criteria:**

- First run can create a local Profile and Space with no cloud account.
- Import previews source, destination Profile, supported data types, and failures; bookmarks map to a user-confirmed durable location.
- Shortcut conflicts show both claimants and allow Arcified, website, or remap choices.
- Reset restores project defaults without erasing unrelated user data.
- Settings are keyboard and screen-reader operable and deep-linkable from Command Bar actions.

**Edges:** repeated import, multiple source Profiles, unavailable password store, partially supported extensions, invalid shortcut chords.

**Dependencies:** ARC-UX-003, ARC-UX-007, platform import adapters.

**Platforms:** source-browser and credential-store capabilities must be reported per OS.

**Unknowns:** exact supported import matrix needs implementation-time evidence.
**Sources:** [import help](https://resources.arc.net/hc/en-us/articles/19335089616791-Import-Bookmarks-Logins-History-Extensions-from-Your-Previous-Browser); [Keyboard Shortcuts](https://resources.arc.net/hc/en-us/articles/20595231349911-Keyboard-Shortcuts); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes).

### ARC-UX-022: Establish an accessibility and input acceptance suite

**Behavior:** Make accessibility a release gate across Sidebar, Command Bar, Peek, Split, Library, settings, and onboarding.

**Acceptance criteria:**

- Every interactive element exposes a stable accessible name, role, state, and relationship in platform inspection tools.
- All operations have keyboard paths with visible focus and no traps, including drag alternatives.
- Zoom, text scaling, high contrast, reduced motion, and screen-reader navigation preserve information and control.
- Privacy, media, download, error, and selection states are not conveyed by color or motion alone.
- Touch input on supported hardware can operate primary controls without disabling precise pointer/keyboard paths.

**Edges:** hidden Sidebar, collapsed folders, dynamic Command Bar results, live regions, multiple split panes, theme extremes.

**Dependencies:** all user-facing tickets; begin with ARC-UX-001 and ARC-UX-007.

**Platforms:** verify with native accessibility inspectors on Linux, Windows, and macOS.

**Unknowns:** Arc offers no formal conformance baseline; this is a downstream quality requirement.
**Sources:** [Keyboard Shortcuts](https://resources.arc.net/hc/en-us/articles/20595231349911-Keyboard-Shortcuts); [2022 Mac notes](https://resources.arc.net/hc/en-us/articles/20498417809815-Arc-for-macOS-2022-Release-Notes); [Windows notes](https://resources.arc.net/hc/en-us/articles/22513842649623-Arc-for-Windows-2023-2026-Release-Notes).

### ARC-UX-023: Specify local-first sync and recovery before implementation

**Behavior:** Define what syncs, what remains Profile/device-local, how conflicts resolve, and how recovery works.

**Acceptance criteria:**

- A written threat model and data inventory precede remote service work.
- Spaces, folders, and tabs use stable identifiers and preserve operations under offline edits.
- Passwords, cookies, browsing history, extensions, and other excluded data cannot enter the payload accidentally.
- Deletions and restores converge without resurrecting deleted folders or losing moved tabs.
- The browser remains usable, exportable, and recoverable when the sync service is unavailable.

**Edges:** concurrent moves, clock skew, lost recovery secret, password change, old client, service shutdown.

**Dependencies:** stable models from ARC-UX-002 through ARC-UX-006.

**Platforms:** shared protocol with OS credential-store adapters.

**Unknowns:** service owner, protocol, conflict algorithm, identity, and retention.
**Sources:** [Arc Sync](https://resources.arc.net/hc/en-us/articles/20272860828823-Arc-Sync); [Recovery Cards](https://resources.arc.net/hc/en-us/articles/22370005382167-Recovery-Cards); [2024-2026 Mac notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes).

### ARC-UX-024: Add opt-in Sidebar integration providers

**Behavior:** Support Live Calendar and GitHub Live Folder-style enhancements through a constrained provider API.

**Acceptance criteria:**

- Providers cannot read unrelated tabs or Profiles and are disabled by default until their required site/account is used and permission accepted.
- Live state is visually distinct from normal bookmarks and can be disabled from the item or settings.
- Provider failure leaves a usable normal Favorite/folder and reports stale state.
- GitHub filters and calendar notification choices persist per Profile.
- Self-hosted or unsupported domains report the limitation rather than silently failing.

**Edges:** revoked sessions, declined events, multiple accounts, offline state, rate limits, enterprise domains.

**Dependencies:** ARC-UX-003 through ARC-UX-005 and a permissions API.

**Platforms:** shared provider contract; platform notifications and meeting launch differ.

**Unknowns:** auth scopes, refresh strategy, and broader provider roadmap.
**Sources:** [Live Calendars](https://resources.arc.net/hc/en-us/articles/24158102740631-Live-Calendars); [Automatic GitHub Live Folders](https://resources.arc.net/hc/en-us/articles/22731612065815-Automatic-GitHub-Live-Folders); [2024-2026 Mac notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes).

### ARC-UX-025: Record retired-feature and migration decisions

**Behavior:** Publish a feature tombstone table so historical Arc scope is consciously implemented, replaced, imported, or rejected.

**Acceptance criteria:**

- Decisions cover Arc Notes, Reader Mode beta, Paste & Go, Recent Tab indicator, Command Bar action education, GitHub Previews, iCloud sync, Legacy Boost Builder, Ask on Page, and the Arc service sunset.
- Every reject/replace decision names the surviving user workflow and migration path, if any.
- UI and import code recognize legacy data without advertising removed features as current.
- Documentation separates Arc historical behavior from Arcified support and verified platform behavior.

**Edges:** stale settings, old synced records, imported Boosts/Notes, archived shared links.

**Dependencies:** relevant feature owners; no code dependency.

**Platforms:** common documentation with platform-specific migration notes.

**Unknowns:** formats not published in official sources require artifact or runtime research owned outside this dossier.
**Sources:** [sunset timeline](https://resources.arc.net/hc/en-us/articles/44412972248471-Arc-Sunset-Key-Dates-and-What-to-Expect); [Notes retirement](https://resources.arc.net/hc/en-us/articles/19233788518039-Phasing-Out-Arc-Notes); [2024-2026 Mac notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes).

## Recommended dependency order

1. Establish tab identity, Profile isolation, and Space/Sidebar ownership: ARC-UX-001 through ARC-UX-004 and ARC-UX-012.
2. Build durable organization and cleanup: ARC-UX-005 and ARC-UX-006.
3. Add command/navigation surfaces: ARC-UX-007 through ARC-UX-010.
4. Add compound views and platform presentation: ARC-UX-011, ARC-UX-013, ARC-UX-014, and ARC-UX-015.
5. Gate optional services and historical creation features behind explicit decisions: ARC-UX-016 through ARC-UX-020 and ARC-UX-024.
6. Keep onboarding, accessibility, sync, and retirements cross-cutting: ARC-UX-021 through ARC-UX-023 and ARC-UX-025.

## Coverage gaps for follow-up research

1. **Runtime-only interaction details:** focus restoration, animation timing, drag thresholds, haptics, exact materials, Peek/Command Bar layering, and multi-window Tab Handoff require binary observation. This dossier makes no claim about them.
2. **Current help-center loss:** dedicated help pages for Peek, Little Arc, Mini Player, Split View, shortcuts, Boosts, and several core organization features were inaccessible or removed during the sunset transition. Release notes preserve behavior but not always the final interaction contract.
3. **Windows ambiguity:** Today-tab ownership across windows, Little Arc availability, Boost support, deep shortcut remapping, Library parity, and the post-2025 status of Ask on Page are not settled by official Windows notes.
4. **Linux absence:** there is no official Arc Linux baseline for window chrome, gestures, materials, credential storage, accessibility, media controls, notifications, or file integration.
5. **Accessibility:** no authoritative VoiceOver/Narrator test matrix, WCAG statement, landmark/focus contract, high-contrast contract, or reduced-motion behavior was found.
6. **Data formats and protocols:** Arc did not publish UX-level schemas for Spaces, splits, Easels, Notes internals, Boosts, sharing, sync operations, or conflict resolution. Artifact and protocol research must stay separate from documentary claims.
7. **Security and privacy:** official UX docs disclose that Max sends data to AI partners and that shared links expose snapshots, but do not fully specify provider prompts, retention, redaction, abuse handling, or service threat models.
8. **Exact boundaries:** “same site” for pinned navigation, Air Traffic Control rule priority, split-pane capacity, folder nesting, restore after deleted Space, and player selection are not authoritative contracts. Do not turn incidental release-note wording into invented limits.
9. **Sunset migration:** official Dia migration lists broad data categories, not behavior equivalence. It does not prove that Dia reproduces any Arc interaction in this matrix.

## Source-to-scope check

The inventory covers the requested Sidebar, Spaces, Profiles, Favorites/Pinned/Today tabs, folders, Archive, Command Bar, search/navigation, Peek/Little Arc, Split View, tabs/windows, themes/motion, Library/downloads/media, Boosts, Easels/Notes, sharing, Max, onboarding/settings/shortcuts, and accessibility areas. It also records Air Traffic Control, sync/recovery, live integrations, developer/Chromium affordances, and explicit retirement handling because they affect the requested desktop UX model.
