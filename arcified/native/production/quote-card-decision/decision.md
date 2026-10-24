# Quote-card decision for issue 29

Status: recommendation ready for coordinator review. Owner acceptance or deferral is **PENDING**. Native acceptance is **OPEN**.

Recommend an optional local quote card plus Chromium-generated text-fragment links, with an attributed plain-text fallback. Defer hosted quote previews and snapshot services. This packet records a deliberate replacement proposal, not Share Quote parity or production support.

The authoritative assignment is retained in [authoritative-issue.json](authoritative-issue.json). [sources.json](sources.json) records external retrieval and limitations; [source-identities.json](source-identities.json) identifies inspected bytes. Retrieval occurred on 2026-10-23 UTC. The workspace baseline is `a4b6b480d3de07857ab6e939149254a8359e5449`.

## Supported reference facts

Arc's November 30, 2023 macOS V1.19.1 notes describe selecting webpage text, invoking Share Quote from the URL bar or context menu, and copying a custom page hyperlink with an image of the selection. December 14 V1.21.0 records an @-character fix. This establishes a link/image workflow, rather than a Markdown format. [A1, 2023 release notes](https://resources.arc.net/hc/en-us/articles/20498377604887-Arc-for-macOS-2023-Release-Notes).

January 11, 2024 V1.25.1 documents disabling the URL-bar button in Arc > Settings > Advanced and automatic suppression on Gmail, Slack, Notion and YouTube. January 25 V1.27.2 adds Confluence. These entries do not prove that disabling the button disables the context-menu command. [A2, 2024 release notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes).

The supplied sharing article returned 404 on direct retrieval. Search-indexed content at that first-party URL still describes the quote link/image workflow. Its separate collection-sharing section describes snapshot links and their access/deletion constraints. Those constraints cannot establish quote-link retention or revocation. [A3, indexed sharing article](https://resources.arc.net/hc/en-us/articles/19228534606743-Share-Spaces-Folders-Splits-with-Anyone). The older archive directs readers to the Resource Center; its missing Share Quote text match is not proof of removal. [A4, archive](https://start.arc.net/release-notes).

Unknown: the historical clipboard MIME payload, whether the image was embedded or fetched as link metadata, any quote-hosting storage or retention contract, precise click/highlight behavior, recipient app previews, current availability, and Windows support. No Arc application or recipient app ran. Linux has no official reference product in this assignment. A local PNG and a text-fragment URL must each describe their own behavior without claiming those unknowns.

## Keep the outputs distinct

| Output | Recommended downstream behavior | Boundary |
| --- | --- | --- |
| Attributed text | Copy the chosen quotation and separately reviewed source URL as plain text. Optional Markdown belongs to issue 95. | A portable text representation. It does not supply an image, hosted preview or reliable highlighting. |
| Text-fragment link | Reuse Chromium's selector generation for eligible HTTP(S) selections, with an explicit link preview and copy action. | A reference to current page text, not a frozen snapshot. Text changes, recipient support, policy or generation failure can prevent highlighting. |
| Local quote card | Render selected text locally into an original static PNG, with separately chosen title/source attribution. Offer a text companion. | The PNG is not a clickable hyperlink. No automatic social-link preview, website capture, authentication transfer or live update is promised. |
| Hosted quote preview or collection snapshot | Defer. Keep the command unavailable unless the owner later chooses an actual service and its separate contract. | No backend, public URL, deletion promise, account or provider is supplied by this packet. |

The WICG draft defines text directives and security restrictions for their activation. Keep browser checks for user involvement, frame/MIME eligibility and browsing-context isolation. [W1, URL Fragment Text Directives](https://wicg.github.io/scroll-to-text-fragment/). Google's implementation guidance describes privacy risks from observable scrolling and site opt-out. A generated URL can reveal the selected words and additional disambiguating context to whoever receives it. Treat it as content to review. [W2, text fragments](https://web.dev/articles/text-fragments).

## Proposed interaction and privacy boundary

These are implementation requirements to accept or defer, not implemented UI or approved settings policy.

1. Invoke a deliberate command for the current selection. Capture its originating page, document/frame and exact native Profile context. Selection alone must not publish, copy, save or contact any service. Generate/render locally without fetching page images, remote fonts, metadata or favicons for the card.
2. Preview the exact quote, optional title, displayed source and serialized link, including selector context. Show whether the output is text, URL or PNG and whether it contains page content. Let the user omit title/source and cancel. Do not claim URL cleaning or secret detection is complete. Do not silently rewrite a destination using an invented tracker/provider rule.
3. Give each output an explicit action. Keep native clipboard/DLP and file-save cancellation checks. Say that clipboard managers, paste targets and exported files may retain copies. No automatic Library/Recent-files entry, workspace persistence or upload follows from exporting selected content.
4. Revalidate document/frame/context and command liveness before output. Navigation, page destruction, Profile shutdown or a stale selection must not retarget the action to a newly selected page. Failure/cancellation must leave the page, saved destination, clipboard and intended file unchanged as applicable, with success reported only after the real output path confirms it.
5. Propose a Profile-scoped Settings > Sharing > Local quote sharing control, disabled initially until enabled deliberately. Turning it off should remove/disable every new card and quote-fragment entry point, including menus, command actions and any future shortcut. Ordinary Copy remains available. This is broader than Arc's documented button-only control. The owner must accept its scope and initial state. Keep existing managed policy authoritative; do not use this control to override it.
6. Use the existing exact-context private/guest classifier and selected-user-output preflight. `CheckUserFile` requires deliberate content/context intent and the persistence notice. It does not approve public sharing, general persistence or automatic registration. Do not infer privacy from a lifecycle ProfileId or normal-profile alias. Unsupported/unknown routes remain unavailable; native eligibility remains an integration gate.

Card layout must expose the full selected text through an accessible text alternative/companion, with keyboard preview/cancel/output and visible focus. Do not silently truncate selection or introduce a character/size cap. Native rendering limits, if encountered, require an explicit unsupported result or a source-backed implementation decision.

## Existing source and foundation evidence

Prepared tree: `/home/timpan4/.t3/builds/arcified-linux-155-native-v1/src`, version `155.0.8059.79`. Paths below are relative to it. Hashes in [source-identities.json](source-identities.json) cover complete inspected files even where only relevant ranges were read.

| Source | Observed mechanism | Consequence |
| --- | --- | --- |
| `chrome/browser/renderer_context_menu/render_view_context_menu.cc:2683` | Checks `kScrollToTextFragmentEnabled`, excludes Glic, requires HTTP(S), then creates the observer. | Preserve policy and eligibility. HTTP(S) alone does not prove public or non-sensitive content despite the source comment. |
| `chrome/browser/renderer_context_menu/link_to_text_menu_observer.cc:115` | Selection initializes generation; command enabling depends on a generated link. | Reuse native selection/generation rather than concatenate selected text into a guessed fragment. |
| Same file, lines 190, 244, 267 and 310 | Rejects absent selectors/stale frames, checks disabled sites and focused main frame, requests a renderer selector, cancels on the existing implementation deadline. | Preserve native failure/cancellation. This packet adds no timeout, retry or selection limit. |
| Same file, lines 287 and 436 | Uses the generated link and enterprise clipboard helper. | Keep data-protection handling on the actual output route. Source inspection is not a completed copy. |
| `components/shared_highlighting/core/common/disabled_sites.cc:30` | Contains a finite host/path blocklist, including downstream-substituted domain bytes. | Preserve actual prepared bytes. It is not a comprehensive sensitive-page detector or permission to normalize the list. |
| `components/shared_highlighting/core/common/fragment_directives_utils.h` | Declares parse/remove/append helpers for escaped selectors. | Preserve ordinary fragment semantics through the native helpers. |

Existing fork foundations reviewed: `arcified/docs/research/arc-dia/coverage-review.md` explicitly separates issue 95 clipboard formats from this decision. `arcified/native/features/private_browsing/operation_boundary.h` and `context_boundary.h` supply pure preflight/context facts. They do not prove native dispatch, clipboard policy or file-save behavior. No pure component was reimplemented or retested.

Root authority remains `lifecycle-contract.md`, `native-space-integration.md` and `execution-state.json` in the assigned root checkout, identified by hash. Keep their Model, metadata/codec and page/command targets. Quote export must not open, transfer, reset, close or duplicate a saved page. Same-window multi-Profile and live saved-page handoff remain root production gates. In particular, `Model::OpenItem` success is irrelevant to quote output acceptance.

No native patch, changed prepared-source result file or test registration is appropriate for this decision leaf before owner scope acceptance. Existing native test sources were inspected, not executed:

- `LinkToTextMenuObserverTest.CopiesLinkToText`, `CopiesLinkForEmptySelector`, `InvalidSelectorForIframe`, `HiddenForExtensions` and `Blocklist` are in `link_to_text_menu_observer_interactive_uitest.cc`. `chrome/browser/renderer_context_menu/BUILD.gn:441` registers the file in `interactive_ui_tests`. Several use mocked generation; they do not prove the proposed card UI.
- `ContextMenuBrowserTest.CopyLinkToTextDisabledWithScrollToTextPolicyDisabled` is in `render_view_context_menu_browsertest.cc:3957`, registered in that directory's `browser_tests` source set at `BUILD.gn:365`.

The coordinator should extend these real registrations and add native card/preview interaction coverage only if the owner accepts implementation. No GN generation, compilation, sparse apply or fake native test was run.

## Native scenarios still required

Every row is **OPEN on Linux, Windows and macOS**. No browser/visual execution was authorized. Existing test declarations do not change these states.

| Scenario | Required later observation |
| --- | --- |
| Eligible text fragment | Actual selection generates/copies a reviewed selector; recipient follows it through normal browser policy. Distinguish generator mocks from live matching. |
| Empty/failed/blocked selection | Action is unavailable or reports failure; no fake link or success. Preserve plain-text fallback. |
| Non-HTTP(S), PDF/extension, iframe and site opt-out | Retain native exclusions and explicitly report unsupported cases. Do not bypass security or inject a polyfill. |
| Selection fidelity | @, punctuation, Unicode/RTL, multiline and repeated text preserve visible quote and correct escaped selector context. |
| Card and companion | PNG contains only approved fields, can be saved/pasted using native routes, and has a usable accessible text companion. No clickable-image promise. |
| Privacy preview | Sensitive quote/title/query/fragment are visible before output; omission and cancellation produce the reviewed result. Verify local generation adds no network request. |
| Clipboard/save failure | Managed clipboard block, save cancellation, existing-file prompt and output failure preserve native behavior and truthful status. |
| Exact context | Private/guest/unknown decisions use the existing classifier; deliberate private output shows persistence consequences without workspace/cache registration. |
| Stale document and focus | Navigation, selection change, split/pane switch, Space/Profile switch, page shutdown and Profile shutdown cannot redirect output to another document/context. |
| Disable and accessibility | All new entry points honor the chosen setting; native policy wins. Keyboard, screen reader, focus return, localized text and contrast work. |
| Portable fallback | Recipient without matching/highlight support can read quotation and use the reviewed ordinary URL. No card-host dependency. |

## Owner questions for coordinator consolidation

1. Accept the optional local PNG card, native text-fragment link and attributed-text fallback with the preview contract, proposed Profile-scoped disable route and initially disabled state, or defer the new card/fragment UI and retain only the portable issue 95 fallback? Recommend accepting the local scope. It requires native UI and platform verification but no hosted service.
2. Accept deferring hosted quote-preview URLs and automatic recipient link images? Recommend deferral. A local image and companion link preserve useful sharing; hosted preview parity would require a separately accepted service, access and revocation contract.

Neither question has an owner answer in the supplied authority. Record the eventual acceptance or deferral in root/issue controls through the coordinator. Do not treat this research task as that approval.

## Acceptance trace

| Assigned criterion | Evidence state |
| --- | --- |
| 2023 Share Quote evidence, hyperlink/image behavior and uncertainty | Documentary research delivered above, with direct primary source and indexed-source limitation. Current runtime remains unverified. |
| Supported local card/text-fragment behavior, privacy preview and disable route | Source-backed recommendation delivered. Implementation, owner scope approval and native verification remain open. |
| Owner acceptance/defer and portable fallback | Fallback and exact questions delivered. Owner acceptance/defer is **PENDING**; this criterion cannot be closed by the agent. |
