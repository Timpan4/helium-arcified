<!-- arcified-backlog:area-assistant -->

Part of [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2). This tracker groups 21 scoped work items for optional intelligence.

## Priority

Later. All 21 assistant work items are below non-AI browser UX, platform parity and measured core performance. Supporting consent and AI resource-cost work are also later, for 23 AI/supporting items total. Core browsing must not depend on this group. Preserve consent/security gates whenever a capability is eventually enabled.

## Work items

- [ ] [#55 Ask an explicitly selected local model from the browser](https://github.com/Timpan4/helium-arcified/issues/55), `local-assistant`, implementation, Optional capability, priority: Later
- [ ] [#88 Choose and inspect @tab and @space context before sending](https://github.com/Timpan4/helium-arcified/issues/88), `context-selector`, implementation, Optional capability, priority: Later
- [ ] [#117 Connect a remote AI provider through explicit consent](https://github.com/Timpan4/helium-arcified/issues/117), `remote-provider`, implementation, Optional capability, priority: Later
- [ ] [#89 Stream, stop and retry assistant answers without blocking browsing](https://github.com/Timpan4/helium-arcified/issues/89), `streaming`, implementation, Optional capability, priority: Later
- [ ] [#118 Attribute answers to the exact page context used](https://github.com/Timpan4/helium-arcified/issues/118), `citations`, implementation, Optional capability, priority: Later
- [ ] [#119 Read supported page content outside the browser UI path](https://github.com/Timpan4/helium-arcified/issues/119), `page-reading`, implementation, Optional capability, priority: Later
- [ ] [#151 Ask about a page from an explicit local find action](https://github.com/Timpan4/helium-arcified/issues/151), `ask-page`, implementation, Optional capability, priority: Later
- [ ] [#152 Preview a link summary only after an explicit request](https://github.com/Timpan4/helium-arcified/issues/152), `link-summaries`, implementation, Optional capability, priority: Later
- [ ] [#120 Suggest better tab titles with user control](https://github.com/Timpan4/helium-arcified/issues/120), `tidy-titles`, implementation, Optional capability, priority: Later
- [ ] [#121 Suggest descriptive download names before committing them](https://github.com/Timpan4/helium-arcified/issues/121), `tidy-downloads`, implementation, Optional capability, priority: Later
- [ ] [#153 Preview AI-suggested tab groups and apply them with undo](https://github.com/Timpan4/helium-arcified/issues/153), `tidy-tabs`, implementation, Optional capability, priority: Later
- [ ] [#122 Send an explicit question from the command bar](https://github.com/Timpan4/helium-arcified/issues/122), `chat-command`, implementation, Optional capability, priority: Later
- [ ] [#56 Decide reusable assistant prompt workflows](https://github.com/Timpan4/helium-arcified/issues/56), `skills`, decision, Evidence, decisions and gates, priority: Later
- [ ] [#57 Decide additional assistant retrieval capabilities](https://github.com/Timpan4/helium-arcified/issues/57), `read-tools`, decision, Evidence, decisions and gates, priority: Later
- [ ] [#58 Decide assistant actions with external effects](https://github.com/Timpan4/helium-arcified/issues/58), `write-tools`, decision, Evidence, decisions and gates, priority: Later
- [ ] [#154 Validate assistant authorization with hostile page fixtures](https://github.com/Timpan4/helium-arcified/issues/154), `ai-boundary`, implementation, Optional capability, priority: Later
- [ ] [#155 Manage local assistant conversations and generated artifacts](https://github.com/Timpan4/helium-arcified/issues/155), `chat-history`, implementation, Optional capability, priority: Later
- [ ] [#173 Decide optional assistant memory with inspect and delete controls](https://github.com/Timpan4/helium-arcified/issues/173), `memory-decision`, decision, Evidence, decisions and gates, priority: Later
- [ ] [#175 Evaluate explicit Instant Links from the command bar](https://github.com/Timpan4/helium-arcified/issues/175), `instant-links`, research, Evidence, decisions and gates, priority: Later
- [ ] [#66 Keep an optional agent task in a visibly separate workspace](https://github.com/Timpan4/helium-arcified/issues/66), `agent-workspace`, implementation, Optional capability, priority: Later
- [ ] [#105 Pause agent browsing and hand control to the user](https://github.com/Timpan4/helium-arcified/issues/105), `agent-takeover`, implementation, Optional capability, priority: Later

## Tracker closure

Keep native sub-issue progress and blocking relationships authoritative. Close accepted implementation work with its named native evidence; close decisions with an explicit approved scope or deferral. Research/tracking closure proves only its named result. Record implemented, tested and released states separately. Optional candidates do not require an account or service for core browsing. Preserve unknown platform behavior and failure/accessibility/data-safety criteria in the owning work item.

Every child carries its own sources, baseline, prerequisites and close proof. The roadmap records pinned artifact identities, historical dispositions and full family coverage. No product code, app execution or measured performance is implied by publication.
