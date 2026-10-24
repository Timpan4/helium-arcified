# Quote-card-decision independent review, round 1

Result: clean bounded review. No actionable findings.

Reviewed issue 29's decision packet against the full assigned leaf, the root control manifest, applicable lifecycle/native integration authority, cited primary documents and relevant prepared Chromium source. This accepts the packet as research ready for coordinator consideration. It does not close the issue, record an owner decision or accept native behavior.

## Scope and exact inputs

Packet: `arcified/native/production/quote-card-decision/`. Baseline: `a4b6b480d3de07857ab6e939149254a8359e5449`. Review performed on 2026-10-24.

All supplied review-input hashes matched independently computed SHA-256 values:

| File | SHA-256 |
| --- | --- |
| `ownership.json` | `6718488aa38ed5534be2e06019abb6c97dc770bddfb825608c0470b6c3e75ab0` |
| `authoritative-issue.json` | `12fa93cfbead4591d1c20d286d28cff63f3e20ff930e8da128a0d2aac3108b92` |
| `sources.json` | `d3cce7aee3aacdd402eda7d23890bf6715cd0554188bf387aaa2c37a78c2ef5e` |
| `source-identities.json` | `6a2c34ccbabf2f77e5e5fdc5814b7cf5cd3596796242cf304e0076ca0e6443ec` |
| `decision.md` | `8591f4a0e4880ec93e0855a5d7f3f232e628aadab031fca388060fcbcd397fc5` |
| `result.json` | `57b2800ec19f9e866dfdc2a12f854db2f02371e3e9522cd6f1d1caf84ea09221` |
| `verification.md` | `4e03a5aeccb90f9e360ada2de80b39047adb0d215992f9e03f834842fdc3e0e3` |

The full assigned issue object in [authoritative-issue.json](../../../native/production/quote-card-decision/authoritative-issue.json) matches every field of the `quote-card-decision` object in the assigned root checkout's `arcified/docs/production/research/assignment.json`. Added assignment provenance fields do not replace issue requirements. The manifest agrees on decision-leaf scope, issue URL and command allocation 40790.

All 18 recorded file identities in [source-identities.json](../../../native/production/quote-card-decision/source-identities.json) matched on inspection. These include the five root authority files, three foundation files and ten prepared-source files. They identify inspected bytes, not frozen release state. Root controls may subsequently change. The prepared tree remained read-only.

## Acceptance coverage

| Authoritative criterion | Assessment and evidence |
| --- | --- |
| Use 2023 Share Quote evidence and document hyperlink/image behavior and current uncertainty. | Satisfied for documentary research. `decision.md:11-17` and source records A1-A4 identify the historical macOS selection command, custom hyperlink and quote image, the later @ fix and button disable behavior. Clipboard payload, image delivery/hosting, current availability and Windows behavior remain explicitly unknown. |
| Recommend supported local card/text-fragment behavior, privacy preview and a disable route. | Satisfied as a proposal. `decision.md:19-41` distinguishes attributed text, native text-fragment URLs, a local PNG with a text companion and deferred hosted previews. It defines exact-output preview, explicit actions, cancellation, source/document/Profile binding, native policy checks and the proposed Profile-scoped control. Initial disable state and scope await owner acceptance. No implementation or setting is enabled. |
| Record owner acceptance/defer and a portable fallback rather than claiming Markdown reproduces Share Quote. | Portable fallback and decision preparation are delivered. `decision.md:23-28,85-98` and `result.json` preserve the difference between clipboard formats and hyperlink/image behavior. Actual owner acceptance/defer remains PENDING and this criterion remains open. The packet correctly refuses to invent approval. |

This is a decision leaf. No production patch, changed prepared-source result file or new test registration is required before owner scope acceptance. Their explicit null/empty records and stated reason are appropriate. A fake implementation would not improve acceptance evidence.

## Primary-source accuracy and provenance

External checks used documentary web retrieval only on 2026-10-24. No application, browser UI or recipient sharing route ran.

The [2023 Arc macOS release notes](https://resources.arc.net/hc/en-us/articles/20498377604887-Arc-for-macOS-2023-Release-Notes) support the packet's November 30 V1.19.1 link/image description and December 14 V1.21.0 @ fix. The [2024-2026 notes](https://resources.arc.net/hc/en-us/articles/20498293324823-Arc-for-macOS-2024-2026-Release-Notes) support January 11 V1.25.1 button disabling and the named automatic suppression sites, then January 25 V1.27.2 Confluence suppression. The packet correctly limits that evidence to the URL-bar button instead of claiming all commands are disabled.

The [sharing help URL](https://resources.arc.net/hc/en-us/articles/19228534606743-Share-Spaces-Folders-Splits-with-Anyone) again returned 404 directly. Search-indexed first-party content was still available and marked as crawled six months earlier. It corroborates the historical quote workflow. Its collection snapshot, authentication and deletion statements are kept separate from quote-link behavior. The [older release archive](https://start.arc.net/release-notes) directs readers to the Resource Center. A missing archive text match is correctly not treated as proof of retirement.

The [WICG text-directive draft](https://wicg.github.io/scroll-to-text-fragment/) and [Google's text-fragment explanation](https://web.dev/articles/text-fragments) support the stated user involvement, frame/MIME/isolation restrictions, scrolling privacy concerns and site opt-out discussion. Neither source establishes installed fork behavior or Arc card parity. The packet retains that limit and makes no cross-browser guarantee.

## Native source and boundary assessment

Focused prepared-source inspection confirms the cited mechanisms:

1. `render_view_context_menu.cc:2683` checks the text-fragment preference, excludes Glic and requires HTTP(S). The packet correctly rejects the inference that HTTP(S) means public or safe.
2. `link_to_text_menu_observer.cc` checks extensions/PDF-viewer exclusion, generated-selector availability, frame liveness, blocklisted URLs and focused main frame; requests selectors, handles its existing timeout and uses the enterprise clipboard helper. No new numeric constraint or fallback bypass is proposed.
3. The prepared disabled-site list retains downstream-substituted domain bytes. Fragment helper declarations support preserving existing ordinary-fragment semantics.
4. The named interactive tests exist and several mock selector generation. Their file is registered at `BUILD.gn:446` within `interactive_ui_tests`, whose declaration starts at line 441. The named policy test begins at `render_view_context_menu_browsertest.cc:3957`; its file is registered at `BUILD.gn:371` within `browser_tests`, whose declaration starts at line 365. These are source declarations, not executed results.

The existing private-context headers support the exact-native-context and selected-user-output preflight statements. The packet does not equate preflight eligibility with native success or public-share authorization. It preserves native clipboard/file checks, unsupported routes and the prohibition on automatic Library/Recent-files registration.

The proposal preserves the authoritative Model, metadata/codec and immutable page/Profile targets. It does not use `Model::OpenItem` as output evidence or change saved destinations. Same-window multi-Profile behavior and live saved-page handoff remain separate real production gates. Command/resource integration is deferred with the correct 40790 allocation, Library 40701 preservation, declarative ActionItem mapping requirement and unique message prefix.

## Scenario and decision quality

Every scenario in `decision.md:67-83` remains OPEN on Linux, Windows and macOS. Coverage includes successful and failed generation, exclusions/site policy, text fidelity, card/text companion, privacy preview, clipboard/save failures, private/guest/unknown context, stale document/focus/Profile, disable/accessibility and portable fallback. The packet does not turn unavailable runtime evidence into parity, absence or passing results.

Both owner questions are proposals with recommendations and tradeoffs, accurately repeated in `result.json`. Coordinator consolidation remains the next decision path:

1. Accept the proposed optional local PNG card, native text-fragment link and attributed-text fallback, including preview, Profile-scoped disable control and initially disabled state, or defer new card/fragment UI and retain the portable issue 95 fallback. The packet recommends local scope.
2. Accept deferring hosted quote-preview URLs and automatic recipient link images. The packet recommends deferral pending a separate service contract.

No owner answer was found in the inspected assigned issue or root decision records. That is an explicit limit, not a defect in this research submission.

## Review checks and remaining gates

Light checks performed:

1. `sha256sum` over all seven packet files and all 18 recorded source paths exited 0. All values matched the supplied packet identities and recorded source identities.
2. JavaScript checks over fast_filesystem-read bytes passed JSON parsing/canonical two-space formatting, required result fields, exact authoritative issue-field comparison, evidence-path membership, pending owner state, final LF and whitespace checks.
3. Focused fast_filesystem reads/searches confirmed the native mechanisms, named test cases and GN source-set registrations above. Test contents were read, not executed.
4. Primary-source documentary retrieval confirmed historical facts and the sharing-page/index limitations above.

No native setup, build, test execution, benchmark, visual check, remote mutation, account access, patch application, staging or commit occurred. Only this review report was authored.

Owner scope acceptance or deferral remains PENDING. Proposed native implementation and every per-platform scenario remain OPEN. Actual clipboard/DLP/file-save behavior, accessibility, no-network local rendering, current Arc service behavior and Windows reference behavior are unverified. Root lifecycle production gates remain OPEN. These limits prevent release/native acceptance, while leaving the decision packet ready for coordinator consolidation.
