# Coverage and deliberate improvements

This matrix accounts for known documented desktop workflow families and the user-requested YAB inspiration. It is a roadmap coverage claim, not proof of exhaustive discovery, implementation, runtime parity or performance. Every target OS needs its own native result. The source dossiers and artifact report retain evidence limits.

The user prefers improved behavior over literal Arc replication. Lifecycle labels explain consequences; tab presentation is interchangeable; cleanup is optional and reversible; ownership changes are staged; sharing is revocable; ordinary browsing works locally without AI or an account. No numeric capacity, timeout, quota or performance target is copied from another product.

## Owner scope and priority

Cross-device sync is excluded. Its six proposals and tracker are closed as not planned. Keep local Profiles, session recovery, multi-window organization and portable import/export. Core browsing requires no Arc or Arcified account. There are 135 active non-AI items ahead of 23 later AI/supporting items. AI consent and security still apply if those later features are implemented; ordinary privacy controls do not depend on AI.

## Family-to-issue mapping

| Family | Issue keys |
| --- | --- |
| Native build foundations | native-linux, native-windows, native-macos |
| Identity, Spaces and ownership | lifecycle-contract, spaces, profiles, space-management, profile-safety, workspace-organizer |
| Persistent/temporary lifecycle | temporary-tabs, pins, pin-section-controls, favorites, folders, folder-preview, tab-status |
| Organization and reversal | multi-select, bulk-close, manual-cleanup, archive, auto-archive, workspace-operation-journal |
| Windows and sessions | window-ownership, tab-handoff, blank-window, undo-windows, session-durability, schema-migrations |
| Command/navigation/find | command-bar, command-ranking, saved-search, site-search, tab-switcher, tab-order-navigation, find |
| Peek and lightweight browsing | peek, peek-split-promotion, little-arc, air-traffic |
| Splits and focus | split-horizontal, split-vertical, split-persistence, split-actions, bulk-split, focus-routing-contract, focus-mode |
| Presentation and native input | sidebar-layout, tab-layout-choice, space-themes, motion, shortcut-settings, accessibility, ime-localization, gesture-navigation, window-chrome |
| Library, files and history | library, downloads, recent-files, library-file-removal, history-controls, capture, region-capture |
| Media and calls | pip, automatic-pip, media-controls, call-controls, calendar |
| Browser and developer integration | site-controls, security-surfaces, developer-mode, browser-diagnostics, layout-inspection, extension-workflow, web-apps, credentials-flow, print-flow, compatibility |
| Creation and customization | boost-style, boost-zap, boost-code, boost-sharing, easel, easel-drawing, board-export, live-capture, notes, snippets, local-ocr |
| Portable/shared content | export, workspace-import, arc-import-format, arc-import, browser-import, prototype-migration, share-folders, share-quote, shared-board, hosted-sharing, board-sharing |
| Assistant and context | assistant-consent-contract, local-assistant, context-selector, remote-provider, page-reading, streaming, citations, ask-page, chat-command |
| Optional Max workflows | link-summaries, tidy-titles, tidy-downloads, tidy-tabs, instant-links |
| Optional advanced assistance | skills, read-tools, write-tools, ai-boundary, chat-history, memory-decision, automation-contract, automation, agent-workspace, agent-takeover |
| Privacy and integrations | private-browsing, data-flow-controls, integration-policy, live-folders, favorite-previews-decision |
| Measured native performance | perf-baseline, perf-measure-linux, perf-measure-windows, perf-measure-macos, perf-activation, perf-startup, perf-discard, perf-profiles, bfcache, perf-hidden, perf-layout, perf-leaks, perf-energy, perf-ai, crashed-tab |
| Cross-device sync, excluded | sync-contract, sync-roundtrip, sync-enroll, sync-conflicts, sync-recovery, sync-revoke |
| Distribution and maintenance | release-linux, release-windows, release-macos, update-linux, update-windows, update-macos, engine-updates |
| Reference/evidence/release gates | reference-windows-artifacts, runtime-reference, retirements, quote-card-decision, easel-collaboration-decision, release-ux-check |
| Local-first onboarding/settings | onboarding, settings |

## Historical, optional and non-goal dispositions

These are design recommendations carried into decision or implementation issues. Publishing an optional candidate does not authorize a provider connection, production service or real page-content use.

| Reference family | Recommended disposition | Issue keys |
| --- | --- | --- |
| Arc Notes | Compatibility import/view/export and external-document routing. A new native editor is outside core scope. | notes |
| Reader Mode beta | Retain a useful native/extension reading path; explicitly decide unsupported cases. | site-controls, compatibility, retirements |
| Paste & Go | Keep explicit paste/navigation; avoid passive clipboard scanning. | command-bar, retirements |
| Recent Tab indicator | Replace with accessible active/recent navigation and ordered selection. | tab-status, tab-switcher, tab-order-navigation |
| Command Bar education | Replace old tutorials with discoverable verbs, provenance and local onboarding. | command-bar, onboarding |
| GitHub Previews | Use an opt-in Live Folder; other Favorite previews need a separate value/scope decision. | live-folders, favorite-previews-decision |
| People Cards | Recommend deferring service-dependent enrichment; preserve the historical row. | retirements |
| Legacy Boost Builder | Replace with reversible declarative styling. Scripts require a separate reviewed per-device/origin capability. | boost-style, boost-zap, boost-code, boost-sharing |
| iCloud sync | Excluded by owner. Keep local export/import and migration without a replacement sync backend or Arcified account. | export, sync-contract |
| Ask on Page | Optional deliberate revival, separate from ordinary local find. Current Windows status remains uncertain. | ask-page, find, retirements |
| Arc accounts and hosted services | Local-first core with portable migration, and independent revocable sharing decisions. | onboarding, arc-import, shared-board |
| Easels and Live Captures | Optional local authoring, drawing, export and scoped refresh. Collaboration is a separate decision. | easel, easel-drawing, board-export, live-capture, easel-collaboration-decision |
| Share Quote | Plain/Markdown/attributed clipboard formats are supported proposals; quote-card/text-fragment behavior has its own decision. | share-quote, quote-card-decision |
| Original live integrations | Opt-in named providers with ordinary bookmark/folder fallback; no silent account discovery. | calendar, live-folders, integration-policy |
| Dia Memory | Recommend deferral of personal collection; retain conflicting documentation and inspect/delete requirements for any future opt-in. | memory-decision |
| Dia work connectors, decks, reports, Morning Brief and enterprise administration | Outside the core Arc/performance objective. Generic prompt/tool workflows remain candidates requiring a concrete need. | skills, read-tools, write-tools, retirements |
| Mobile Arc Search | Outside this desktop deliverable; no implied mobile-only implementation obligation. | retirements |
| YAB engine and platform mechanisms | Borrow portable interactions. Do not switch the engine or assume WebKit size/performance, Live Text, Touch ID or macOS keychain behavior transfers. | tab-layout-choice, local-ocr, perf-baseline |
| YAB agent workspace and takeover | Optional, gated by browser-owned consent, supported provider/auth and automation decisions. No copied caps, schedules or idle timers. | agent-workspace, agent-takeover |

## Evidence status

Arc/Dia official macOS images were downloaded and inspected with REA. No app, native Arcified build, benchmark, migration or accessibility scenario was executed. YAB is a first-party documentary reference only. Windows reference acquisition and native UX scenarios remain open research. Retained menu declarations are partial static evidence, not callable behavior. All implementation/test/release states start as not implemented or unverified until attached evidence proves otherwise.
