# Published Arcified desktop backlog

[Roadmap](https://github.com/Timpan4/helium-arcified/issues/2). Verified on 2026-10-23: 135 non-AI items before 23 later AI/supporting items, 158 active leaf items. Six sync proposals and their tracker are closed as not planned. The repository contains 178 historical issues, 171 open and 7 excluded, with 177 parent links and 262 native blocker links. No non-AI dependency path reaches AI.

## Active topic trackers

- [Native builds and platform integration](https://github.com/Timpan4/helium-arcified/issues/3)
- [Spaces and tab lifecycle](https://github.com/Timpan4/helium-arcified/issues/4)
- [Data migration and privacy](https://github.com/Timpan4/helium-arcified/issues/5)
- [Navigation and multitasking](https://github.com/Timpan4/helium-arcified/issues/6)
- [Appearance, input and onboarding](https://github.com/Timpan4/helium-arcified/issues/7)
- [Library, files and capture](https://github.com/Timpan4/helium-arcified/issues/8)
- [Media controls](https://github.com/Timpan4/helium-arcified/issues/9)
- [Creation and live integrations](https://github.com/Timpan4/helium-arcified/issues/10)
- [Optional intelligence](https://github.com/Timpan4/helium-arcified/issues/11), later priority
- [Measured performance and recovery](https://github.com/Timpan4/helium-arcified/issues/12)
- [Distribution and engine maintenance](https://github.com/Timpan4/helium-arcified/issues/14)
- [Reference evidence and release coverage](https://github.com/Timpan4/helium-arcified/issues/15)

Excluded: [cross-device sync tracker](https://github.com/Timpan4/helium-arcified/issues/13). Core browsing, local boards, Profiles, restart recovery and portable import/export need no Arc account.

## Work-item index

| Key | Issue | Area | Kind | Priority | State |
| --- | --- | --- | --- | --- | --- |
| `native-linux` | [#16 Boot a downstream native Arcified build on Linux](https://github.com/Timpan4/helium-arcified/issues/16) | platform | implementation | Before AI | Open |
| `native-windows` | [#17 Boot a downstream native Arcified build on Windows](https://github.com/Timpan4/helium-arcified/issues/17) | platform | implementation | Before AI | Open |
| `native-macos` | [#18 Boot a downstream native Arcified build on macOS](https://github.com/Timpan4/helium-arcified/issues/18) | platform | implementation | Before AI | Open |
| `spaces` | [#32 Create and switch live Spaces in the native sidebar](https://github.com/Timpan4/helium-arcified/issues/32) | core | implementation | Before AI | Open |
| `profiles` | [#48 Attach Spaces to isolated browser Profiles](https://github.com/Timpan4/helium-arcified/issues/48) | core | implementation | Before AI | Open |
| `temporary-tabs` | [#49 Open and close temporary tabs within a Space](https://github.com/Timpan4/helium-arcified/issues/49) | core | implementation | Before AI | Open |
| `pins` | [#67 Pin a tab with a durable home URL and reset action](https://github.com/Timpan4/helium-arcified/issues/67) | core | implementation | Before AI | Open |
| `favorites` | [#106 Keep Profile Favorites available across Spaces](https://github.com/Timpan4/helium-arcified/issues/106) | core | implementation | Before AI | Open |
| `folders` | [#107 Organize pinned tabs into nested sidebar folders](https://github.com/Timpan4/helium-arcified/issues/107) | core | implementation | Before AI | Open |
| `multi-select` | [#68 Select and move several sidebar tabs](https://github.com/Timpan4/helium-arcified/issues/68) | core | implementation | Before AI | Open |
| `archive` | [#69 Browse and restore manually archived tabs](https://github.com/Timpan4/helium-arcified/issues/69) | core | implementation | Before AI | Open |
| `auto-archive` | [#108 Clean up inactive temporary tabs without losing work](https://github.com/Timpan4/helium-arcified/issues/108) | core | implementation | Before AI | Open |
| `session-durability` | [#50 Recover the native workspace after an interrupted write](https://github.com/Timpan4/helium-arcified/issues/50) | core | implementation | Before AI | Open |
| `undo-windows` | [#70 Reopen a closed window with its workspace](https://github.com/Timpan4/helium-arcified/issues/70) | core | implementation | Before AI | Open |
| `tab-status` | [#71 Show tab loading, audio, capture and attention state](https://github.com/Timpan4/helium-arcified/issues/71) | core | implementation | Before AI | Open |
| `window-ownership` | [#72 Move tabs and Spaces between native windows](https://github.com/Timpan4/helium-arcified/issues/72) | core | implementation | Before AI | Open |
| `export` | [#73 Export portable Arcified workspace data](https://github.com/Timpan4/helium-arcified/issues/73) | data | implementation | Before AI | Open |
| `arc-import` | [#74 Import Arc Spaces, folders and custom tab names](https://github.com/Timpan4/helium-arcified/issues/74) | data | implementation | Before AI | Open |
| `browser-import` | [#75 Import browser bookmarks, history and selected settings](https://github.com/Timpan4/helium-arcified/issues/75) | data | implementation | Before AI | Open |
| `prototype-migration` | [#76 Migrate Arcified Lab snapshots into native Spaces](https://github.com/Timpan4/helium-arcified/issues/76) | data | implementation | Before AI | Open |
| `command-bar` | [#77 Navigate and run browser actions from one command bar](https://github.com/Timpan4/helium-arcified/issues/77) | navigation | implementation | Before AI | Open |
| `command-ranking` | [#109 Find open and pinned tabs across Spaces](https://github.com/Timpan4/helium-arcified/issues/109) | navigation | implementation | Before AI | Open |
| `site-search` | [#110 Use configurable site-search shortcuts in the command bar](https://github.com/Timpan4/helium-arcified/issues/110) | navigation | implementation | Before AI | Open |
| `tab-switcher` | [#78 Switch recent tabs with a keyboard preview](https://github.com/Timpan4/helium-arcified/issues/78) | navigation | implementation | Before AI | Open |
| `find` | [#33 Find page text without breaking selection or focus](https://github.com/Timpan4/helium-arcified/issues/33) | navigation | implementation | Before AI | Open |
| `peek` | [#79 Preview a link in Peek and promote or dismiss it](https://github.com/Timpan4/helium-arcified/issues/79) | navigation | implementation | Before AI | Open |
| `little-arc` | [#111 Open external links in a lightweight browser window](https://github.com/Timpan4/helium-arcified/issues/111) | navigation | implementation | Before AI | Open |
| `air-traffic` | [#80 Route external links to the intended Space or Profile](https://github.com/Timpan4/helium-arcified/issues/80) | navigation | implementation | Before AI | Open |
| `sidebar-layout` | [#51 Resize, hide and reveal the sidebar without moving focus](https://github.com/Timpan4/helium-arcified/issues/51) | appearance | implementation | Before AI | Open |
| `split-horizontal` | [#81 Browse two tabs side by side with independent navigation](https://github.com/Timpan4/helium-arcified/issues/81) | navigation | implementation | Before AI | Open |
| `split-vertical` | [#112 Switch split panes between horizontal and vertical orientation](https://github.com/Timpan4/helium-arcified/issues/112) | navigation | implementation | Before AI | Open |
| `focus-mode` | [#82 Enter a distraction-free layout with controls on demand](https://github.com/Timpan4/helium-arcified/issues/82) | appearance | implementation | Before AI | Open |
| `space-themes` | [#52 Customize each Space's color, name and icon](https://github.com/Timpan4/helium-arcified/issues/52) | appearance | implementation | Before AI | Open |
| `motion` | [#83 Respect motion and sound preferences in sidebar transitions](https://github.com/Timpan4/helium-arcified/issues/83) | appearance | implementation | Before AI | Open |
| `shortcut-settings` | [#34 Discover and customize browser keyboard shortcuts](https://github.com/Timpan4/helium-arcified/issues/34) | appearance | implementation | Before AI | Open |
| `accessibility` | [#149 Operate Spaces and the sidebar with assistive technology](https://github.com/Timpan4/helium-arcified/issues/149) | platform | implementation | Before AI | Open |
| `ime-localization` | [#113 Edit browser chrome reliably with IME and bidirectional text](https://github.com/Timpan4/helium-arcified/issues/113) | platform | implementation | Before AI | Open |
| `window-chrome` | [#35 Respect native caption controls, displays and window states](https://github.com/Timpan4/helium-arcified/issues/35) | platform | implementation | Before AI | Open |
| `library` | [#19 Open a unified Library for saved browsing material](https://github.com/Timpan4/helium-arcified/issues/19) | library | implementation | Before AI | Open |
| `downloads` | [#36 Manage downloads with clear progress and recovery](https://github.com/Timpan4/helium-arcified/issues/36) | library | implementation | Before AI | Open |
| `recent-files` | [#53 Drag recent downloads and captures from the sidebar](https://github.com/Timpan4/helium-arcified/issues/53) | library | implementation | Before AI | Open |
| `capture` | [#37 Capture a full page and reuse it from Library](https://github.com/Timpan4/helium-arcified/issues/37) | library | implementation | Before AI | Open |
| `pip` | [#114 Keep video available in picture in picture](https://github.com/Timpan4/helium-arcified/issues/114) | media | implementation | Before AI | Open |
| `media-controls` | [#115 Control background audio from the browser shell](https://github.com/Timpan4/helium-arcified/issues/115) | media | implementation | Before AI | Open |
| `site-controls` | [#38 Reach site permissions and extensions from compact chrome](https://github.com/Timpan4/helium-arcified/issues/38) | navigation | implementation | Before AI | Open |
| `developer-mode` | [#39 Inspect the active page and keep developer controls discoverable](https://github.com/Timpan4/helium-arcified/issues/39) | navigation | implementation | Before AI | Open |
| `boost-style` | [#54 Customize a site's appearance with a reversible Boost](https://github.com/Timpan4/helium-arcified/issues/54) | extras | implementation | Before AI | Open |
| `boost-zap` | [#84 Hide selected site elements with a reversible picker](https://github.com/Timpan4/helium-arcified/issues/84) | extras | implementation | Before AI | Open |
| `boost-code` | [#85 Offer explicit local code customization with safety boundaries](https://github.com/Timpan4/helium-arcified/issues/85) | extras | implementation | Before AI | Open |
| `easel` | [#86 Collect links, text and captures on a local visual board](https://github.com/Timpan4/helium-arcified/issues/86) | extras | implementation | Before AI | Open |
| `live-capture` | [#116 Refresh a selected web region on a visual board](https://github.com/Timpan4/helium-arcified/issues/116) | extras | implementation | Before AI | Open |
| `notes` | [#87 Preserve historical Arc Notes through import and export](https://github.com/Timpan4/helium-arcified/issues/87) | extras | implementation | Before AI | Open |
| `share-folders` | [#172 Share a folder or split as an inspectable set of links](https://github.com/Timpan4/helium-arcified/issues/172) | extras | implementation | Before AI | Open |
| `live-folders` | [#150 Keep a user-connected work folder current](https://github.com/Timpan4/helium-arcified/issues/150) | extras | implementation | Before AI | Open |
| `perf-baseline` | [#20 Define reproducible browser performance fixtures and milestones](https://github.com/Timpan4/helium-arcified/issues/20) | performance | research | Before AI | Open |
| `perf-activation` | [#90 Present the next tab before doing close cleanup](https://github.com/Timpan4/helium-arcified/issues/90) | performance | implementation | Before AI | Open |
| `perf-startup` | [#91 Restore a usable window before loading background tabs](https://github.com/Timpan4/helium-arcified/issues/91) | performance | implementation | Before AI | Open |
| `perf-discard` | [#123 Sleep eligible background tabs and restore them safely](https://github.com/Timpan4/helium-arcified/issues/123) | performance | implementation | Before AI | Open |
| `perf-profiles` | [#92 Unload inactive Profile resources without mixing identities](https://github.com/Timpan4/helium-arcified/issues/92) | performance | implementation | Before AI | Open |
| `bfcache` | [#124 Preserve eligible pages through back and forward cache](https://github.com/Timpan4/helium-arcified/issues/124) | performance | implementation | Before AI | Open |
| `perf-hidden` | [#93 Stop redundant work in hidden sidebar and split surfaces](https://github.com/Timpan4/helium-arcified/issues/93) | performance | implementation | Before AI | Open |
| `perf-layout` | [#125 Keep page content anchored during sidebar and split transitions](https://github.com/Timpan4/helium-arcified/issues/125) | performance | implementation | Before AI | Open |
| `perf-leaks` | [#126 Detect browser-owned resource leaks through repeated workflows](https://github.com/Timpan4/helium-arcified/issues/126) | performance | implementation | Before AI | Open |
| `perf-energy` | [#40 Keep idle browsing quiet and allow OS sleep](https://github.com/Timpan4/helium-arcified/issues/40) | performance | implementation | Before AI | Open |
| `crashed-tab` | [#94 Recover a crashed renderer with a clear retry surface](https://github.com/Timpan4/helium-arcified/issues/94) | performance | implementation | Before AI | Open |
| `folder-preview` | [#158 Find a tab inside a collapsed folder](https://github.com/Timpan4/helium-arcified/issues/158) | core | implementation | Before AI | Open |
| `tab-handoff` | [#159 Hand off a persistent tab between native windows](https://github.com/Timpan4/helium-arcified/issues/159) | core | implementation | Before AI | Open |
| `calendar` | [#160 Show an opt-in calendar countdown and join action](https://github.com/Timpan4/helium-arcified/issues/160) | extras | implementation | Before AI | Open |
| `share-quote` | [#95 Copy links as plain URL, Markdown or attributed text](https://github.com/Timpan4/helium-arcified/issues/95) | navigation | implementation | Before AI | Open |
| `easel-drawing` | [#128 Draw and align content on a research board](https://github.com/Timpan4/helium-arcified/issues/128) | extras | implementation | Before AI | Open |
| `shared-board` | [#22 Decide hosting and access for revocable content sharing](https://github.com/Timpan4/helium-arcified/issues/22) | extras | decision | Before AI | Open |
| `boost-sharing` | [#96 Preview, import and export visual Boosts safely](https://github.com/Timpan4/helium-arcified/issues/96) | extras | implementation | Before AI | Open |
| `settings` | [#129 Find and reset settings without losing browsing data](https://github.com/Timpan4/helium-arcified/issues/129) | appearance | implementation | Before AI | Open |
| `onboarding` | [#97 Set up a usable local browser without a cloud account](https://github.com/Timpan4/helium-arcified/issues/97) | appearance | implementation | Before AI | Open |
| `private-browsing` | [#98 Keep private windows out of workspace persistence and AI](https://github.com/Timpan4/helium-arcified/issues/98) | data | implementation | Before AI | Open |
| `automation` | [#130 Expose permissioned browser automation through platform adapters](https://github.com/Timpan4/helium-arcified/issues/130) | platform | implementation | Before AI | Open |
| `compatibility` | [#59 Record the native Chromium compatibility release matrix](https://github.com/Timpan4/helium-arcified/issues/59) | research | tracking | Before AI | Open |
| `release-linux` | [#131 Install a Linux Arcified package safely](https://github.com/Timpan4/helium-arcified/issues/131) | release | implementation | Before AI | Open |
| `release-windows` | [#132 Install a signed Windows Arcified package](https://github.com/Timpan4/helium-arcified/issues/132) | release | implementation | Before AI | Open |
| `release-macos` | [#133 Install a signed macOS Arcified app](https://github.com/Timpan4/helium-arcified/issues/133) | release | implementation | Before AI | Open |
| `engine-updates` | [#41 Prove a repeatable Chromium update procedure](https://github.com/Timpan4/helium-arcified/issues/41) | release | implementation | Before AI | Open |
| `runtime-reference` | [#42 Observe reference UX on supported desktop operating systems](https://github.com/Timpan4/helium-arcified/issues/42) | research | research | Before AI | Open |
| `retirements` | [#23 Record historical feature and service replacement decisions](https://github.com/Timpan4/helium-arcified/issues/23) | research | research | Before AI | Open |
| `release-ux-check` | [#161 Verify complete desktop UX coverage on all target platforms](https://github.com/Timpan4/helium-arcified/issues/161) | research | tracking | Before AI | Open |
| `lifecycle-contract` | [#24 Settle native tab ownership and cleanup semantics](https://github.com/Timpan4/helium-arcified/issues/24) | research | decision | Before AI | Open |
| `pin-section-controls` | [#134 Reset visible pins and collapse the pinned section](https://github.com/Timpan4/helium-arcified/issues/134) | core | implementation | Before AI | Open |
| `bulk-close` | [#135 Close selected tabs with active-work protection and undo](https://github.com/Timpan4/helium-arcified/issues/135) | core | implementation | Before AI | Open |
| `bulk-split` | [#162 Open a selected set of tabs as a split](https://github.com/Timpan4/helium-arcified/issues/162) | navigation | implementation | Before AI | Open |
| `workspace-import` | [#136 Preview and merge a portable Arcified workspace import](https://github.com/Timpan4/helium-arcified/issues/136) | data | implementation | Before AI | Open |
| `saved-search` | [#163 Search folders, archive and history from the command bar](https://github.com/Timpan4/helium-arcified/issues/163) | navigation | implementation | Before AI | Open |
| `region-capture` | [#60 Capture a selected region with a cancellable picker](https://github.com/Timpan4/helium-arcified/issues/60) | library | implementation | Before AI | Open |
| `board-export` | [#164 Export an editable research board and a visual snapshot](https://github.com/Timpan4/helium-arcified/issues/164) | extras | implementation | Before AI | Open |
| `extension-workflow` | [#61 Keep extension toolbar actions usable in compact chrome](https://github.com/Timpan4/helium-arcified/issues/61) | platform | implementation | Before AI | Open |
| `web-apps` | [#62 Preserve installed web app launch and window behavior](https://github.com/Timpan4/helium-arcified/issues/62) | platform | implementation | Before AI | Open |
| `credentials-flow` | [#137 Keep autofill and passkey prompts attached to the right page](https://github.com/Timpan4/helium-arcified/issues/137) | platform | implementation | Before AI | Open |
| `print-flow` | [#138 Print and save a page to PDF from compact chrome](https://github.com/Timpan4/helium-arcified/issues/138) | platform | implementation | Before AI | Open |
| `hosted-sharing` | [#179 Publish and revoke a workspace link snapshot](https://github.com/Timpan4/helium-arcified/issues/179) | extras | implementation | Before AI | Open |
| `automation-contract` | [#25 Choose the browser automation surface and authorization model](https://github.com/Timpan4/helium-arcified/issues/25) | research | decision | Before AI | Open |
| `integration-policy` | [#26 Choose account scopes for live folder and calendar integrations](https://github.com/Timpan4/helium-arcified/issues/26) | research | decision | Before AI | Open |
| `update-linux` | [#165 Deliver authenticated Arcified updates on Linux](https://github.com/Timpan4/helium-arcified/issues/165) | release | implementation | Before AI | Open |
| `update-windows` | [#166 Deliver authenticated Arcified updates on Windows](https://github.com/Timpan4/helium-arcified/issues/166) | release | implementation | Before AI | Open |
| `update-macos` | [#167 Deliver authenticated Arcified updates on macOS](https://github.com/Timpan4/helium-arcified/issues/167) | release | implementation | Before AI | Open |
| `schema-migrations` | [#99 Upgrade stored workspace schemas without losing the original](https://github.com/Timpan4/helium-arcified/issues/99) | data | implementation | Before AI | Open |
| `arc-import-format` | [#27 Discover what supported Arc exports actually contain](https://github.com/Timpan4/helium-arcified/issues/27) | research | research | Before AI | Open |
| `library-file-removal` | [#63 Remove Library entries and delete real files safely](https://github.com/Timpan4/helium-arcified/issues/63) | library | implementation | Before AI | Open |
| `favorite-previews-decision` | [#28 Decide which historical Favorite site previews are useful](https://github.com/Timpan4/helium-arcified/issues/28) | research | decision | Before AI | Open |
| `tab-order-navigation` | [#100 Navigate tabs and Spaces in sidebar order](https://github.com/Timpan4/helium-arcified/issues/100) | navigation | implementation | Before AI | Open |
| `blank-window` | [#140 Open a Blank Window distinct from normal and private windows](https://github.com/Timpan4/helium-arcified/issues/140) | navigation | implementation | Before AI | Open |
| `gesture-navigation` | [#101 Route trackpad and touch gestures to Spaces or history](https://github.com/Timpan4/helium-arcified/issues/101) | platform | implementation | Before AI | Open |
| `manual-cleanup` | [#141 Clear temporary tabs with a preview and undo](https://github.com/Timpan4/helium-arcified/issues/141) | core | implementation | Before AI | Open |
| `automatic-pip` | [#168 Offer automatic Mini Player with global and site opt-out](https://github.com/Timpan4/helium-arcified/issues/168) | media | implementation | Before AI | Open |
| `history-controls` | [#102 Browse and clear local history separately from Archive](https://github.com/Timpan4/helium-arcified/issues/102) | library | implementation | Before AI | Open |
| `quote-card-decision` | [#29 Decide a local replacement for historical Share Quote cards](https://github.com/Timpan4/helium-arcified/issues/29) | research | decision | Before AI | Open |
| `browser-diagnostics` | [#142 Open native task manager and performance recording](https://github.com/Timpan4/helium-arcified/issues/142) | platform | implementation | Before AI | Open |
| `security-surfaces` | [#143 Preserve trustworthy security prompts in compact browser chrome](https://github.com/Timpan4/helium-arcified/issues/143) | platform | implementation | Before AI | Open |
| `easel-collaboration-decision` | [#30 Decide whether live board collaboration belongs in scope](https://github.com/Timpan4/helium-arcified/issues/30) | research | decision | Before AI | Open |
| `reference-windows-artifacts` | [#31 Identify official Windows Arc and Dia reference packages](https://github.com/Timpan4/helium-arcified/issues/31) | research | research | Before AI | Open |
| `space-management` | [#64 Rename, reorder and remove a Space with recovery](https://github.com/Timpan4/helium-arcified/issues/64) | core | implementation | Before AI | Open |
| `profile-safety` | [#103 Reassign or delete a Profile without relabeling authenticated pages](https://github.com/Timpan4/helium-arcified/issues/103) | core | implementation | Before AI | Open |
| `split-persistence` | [#169 Restore a saved split's layout and active pane](https://github.com/Timpan4/helium-arcified/issues/169) | navigation | implementation | Before AI | Open |
| `split-actions` | [#177 Pin, move and maximize a durable split](https://github.com/Timpan4/helium-arcified/issues/177) | navigation | implementation | Before AI | Open |
| `peek-split-promotion` | [#144 Promote Peek into a split without reloading its page](https://github.com/Timpan4/helium-arcified/issues/144) | navigation | implementation | Before AI | Open |
| `tab-layout-choice` | [#104 Present the same tabs in sidebar or top-row layouts](https://github.com/Timpan4/helium-arcified/issues/104) | appearance | implementation | Before AI | Open |
| `layout-inspection` | [#65 Inspect local page spacing with reversible layout tools](https://github.com/Timpan4/helium-arcified/issues/65) | platform | implementation | Before AI | Open |
| `snippets` | [#145 Save and insert local user-authored text snippets](https://github.com/Timpan4/helium-arcified/issues/145) | extras | implementation | Before AI | Open |
| `local-ocr` | [#146 Select text from an image through an explicit local OCR action](https://github.com/Timpan4/helium-arcified/issues/146) | extras | implementation | Before AI | Open |
| `call-controls` | [#170 Keep an active call reachable while switching workspaces](https://github.com/Timpan4/helium-arcified/issues/170) | media | implementation | Before AI | Open |
| `data-flow-controls` | [#171 Explain and control every browser data destination](https://github.com/Timpan4/helium-arcified/issues/171) | data | implementation | Before AI | Open |
| `workspace-organizer` | [#147 Inspect and repair workspace ownership in one organizer](https://github.com/Timpan4/helium-arcified/issues/147) | core | implementation | Before AI | Open |
| `workspace-operation-journal` | [#148 Undo a workspace change as the action the user performed](https://github.com/Timpan4/helium-arcified/issues/148) | core | implementation | Before AI | Open |
| `focus-routing-contract` | [#44 Define which page, pane or overlay receives a browser action](https://github.com/Timpan4/helium-arcified/issues/44) | research | decision | Before AI | Open |
| `board-sharing` | [#178 Publish and revoke a board snapshot independently](https://github.com/Timpan4/helium-arcified/issues/178) | extras | implementation | Before AI | Open |
| `perf-measure-linux` | [#45 Record the native browser performance baseline on Linux](https://github.com/Timpan4/helium-arcified/issues/45) | performance | implementation | Before AI | Open |
| `perf-measure-windows` | [#46 Record the native browser performance baseline on Windows](https://github.com/Timpan4/helium-arcified/issues/46) | performance | implementation | Before AI | Open |
| `perf-measure-macos` | [#47 Record the native browser performance baseline on macOS](https://github.com/Timpan4/helium-arcified/issues/47) | performance | implementation | Before AI | Open |
| `local-assistant` | [#55 Ask an explicitly selected local model from the browser](https://github.com/Timpan4/helium-arcified/issues/55) | assistant | implementation | Later | Open |
| `context-selector` | [#88 Choose and inspect @tab and @space context before sending](https://github.com/Timpan4/helium-arcified/issues/88) | assistant | implementation | Later | Open |
| `remote-provider` | [#117 Connect a remote AI provider through explicit consent](https://github.com/Timpan4/helium-arcified/issues/117) | assistant | implementation | Later | Open |
| `streaming` | [#89 Stream, stop and retry assistant answers without blocking browsing](https://github.com/Timpan4/helium-arcified/issues/89) | assistant | implementation | Later | Open |
| `citations` | [#118 Attribute answers to the exact page context used](https://github.com/Timpan4/helium-arcified/issues/118) | assistant | implementation | Later | Open |
| `page-reading` | [#119 Read supported page content outside the browser UI path](https://github.com/Timpan4/helium-arcified/issues/119) | assistant | implementation | Later | Open |
| `ask-page` | [#151 Ask about a page from an explicit local find action](https://github.com/Timpan4/helium-arcified/issues/151) | assistant | implementation | Later | Open |
| `link-summaries` | [#152 Preview a link summary only after an explicit request](https://github.com/Timpan4/helium-arcified/issues/152) | assistant | implementation | Later | Open |
| `tidy-titles` | [#120 Suggest better tab titles with user control](https://github.com/Timpan4/helium-arcified/issues/120) | assistant | implementation | Later | Open |
| `tidy-downloads` | [#121 Suggest descriptive download names before committing them](https://github.com/Timpan4/helium-arcified/issues/121) | assistant | implementation | Later | Open |
| `tidy-tabs` | [#153 Preview AI-suggested tab groups and apply them with undo](https://github.com/Timpan4/helium-arcified/issues/153) | assistant | implementation | Later | Open |
| `chat-command` | [#122 Send an explicit question from the command bar](https://github.com/Timpan4/helium-arcified/issues/122) | assistant | implementation | Later | Open |
| `skills` | [#56 Decide reusable assistant prompt workflows](https://github.com/Timpan4/helium-arcified/issues/56) | assistant | decision | Later | Open |
| `read-tools` | [#57 Decide additional assistant retrieval capabilities](https://github.com/Timpan4/helium-arcified/issues/57) | assistant | decision | Later | Open |
| `write-tools` | [#58 Decide assistant actions with external effects](https://github.com/Timpan4/helium-arcified/issues/58) | assistant | decision | Later | Open |
| `ai-boundary` | [#154 Validate assistant authorization with hostile page fixtures](https://github.com/Timpan4/helium-arcified/issues/154) | assistant | implementation | Later | Open |
| `chat-history` | [#155 Manage local assistant conversations and generated artifacts](https://github.com/Timpan4/helium-arcified/issues/155) | assistant | implementation | Later | Open |
| `memory-decision` | [#173 Decide optional assistant memory with inspect and delete controls](https://github.com/Timpan4/helium-arcified/issues/173) | assistant | decision | Later | Open |
| `perf-ai` | [#127 Measure assistant startup and rendering without slowing browsing](https://github.com/Timpan4/helium-arcified/issues/127) | performance | implementation | Later | Open |
| `instant-links` | [#175 Evaluate explicit Instant Links from the command bar](https://github.com/Timpan4/helium-arcified/issues/175) | assistant | research | Later | Open |
| `assistant-consent-contract` | [#43 Define browser-owned consent and authorization for optional AI](https://github.com/Timpan4/helium-arcified/issues/43) | research | decision | Later | Open |
| `agent-workspace` | [#66 Keep an optional agent task in a visibly separate workspace](https://github.com/Timpan4/helium-arcified/issues/66) | assistant | implementation | Later | Open |
| `agent-takeover` | [#105 Pause agent browsing and hand control to the user](https://github.com/Timpan4/helium-arcified/issues/105) | assistant | implementation | Later | Open |
| `sync-contract` | [#21 Decide encrypted sync scope and key ownership](https://github.com/Timpan4/helium-arcified/issues/21) | sync | decision | Excluded | Closed, not planned |
| `sync-enroll` | [#156 Enroll another device and show encrypted sync status](https://github.com/Timpan4/helium-arcified/issues/156) | sync | implementation | Excluded | Closed, not planned |
| `sync-conflicts` | [#157 Merge offline workspace edits without losing tabs](https://github.com/Timpan4/helium-arcified/issues/157) | sync | implementation | Excluded | Closed, not planned |
| `sync-recovery` | [#174 Recover encrypted workspace access with trusted recovery material](https://github.com/Timpan4/helium-arcified/issues/174) | sync | implementation | Excluded | Closed, not planned |
| `sync-roundtrip` | [#139 Prove an encrypted workspace round-trip with synthetic data](https://github.com/Timpan4/helium-arcified/issues/139) | sync | implementation | Excluded | Closed, not planned |
| `sync-revoke` | [#176 Revoke a synced device and restrict future key access](https://github.com/Timpan4/helium-arcified/issues/176) | sync | implementation | Excluded | Closed, not planned |

[Coverage and dispositions](coverage-matrix.md), [readback proof](publication/verification.json) and [receipts](publication/receipts.json). Native behavior and comparative performance remain unverified. Each active issue defines its own closure evidence.
