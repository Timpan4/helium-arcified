# Native Space design review

Result: the preparatory design is supported by the pinned ownership evidence. No blocking source-interpretation or architecture defect was found within this investigation scope. One authority/version update is needed before the plan guides model binding. Native feasibility and issue 32 acceptance remain unproven.

Reviewed independently on 2026-10-23. This is the first independent plan review. The later owner approvals supplied with the review task take precedence over the document's older decision ledger. Only this report was written. No implementation, Git history, GitHub issue or PR was changed.

## Reviewed revision and evidence

Repository HEAD is `09670c97b1c308d0fb31869b3a9a9bc6969bc47f`. The reviewed documents are untracked, so their Git blob hashes identify the reviewed bytes:

| Artifact | Git blob |
| --- | --- |
| [native-space-integration.md](../native-space-integration.md) | `7ba592e1c5521e5881300bca2738c8268b08fcc3` |
| [native-space-source-references.md](../native-space-source-references.md) | `8bde6614650d3e9be675d0d9f6c3b6b48efe8410` |

Both artifacts were read in full. The live [issue 32](https://github.com/Timpan4/helium-arcified/issues/32) was fetched read-only. Its native sidebar, selected-page preservation, durable identity/order/selection and installed Linux/Windows/macOS acceptance requirements agree with the plan's stated scope.

Independently resolved Chromium tag `155.0.8059.79` to `a49e2e3d4addfd84e765b52afe230bedb81096db` through Gitiles JSON and checked the local engine pin. Fetched individual files through Gitiles TEXT, decoded them in memory, and inspected ownership passages in all 22 Chromium files cited by C1 through C4 and H1. These were small source requests, without a checkout, archive, preparation or build. The web reader could not access Gitiles; direct HTTP succeeded.

All 14 Helium/Brave patch blob hashes in H1 match the local files. Inspected the relevant presentation, shortcut/MRU, restore, discard, Profile-menu and BrowserContext-token hunks, plus the close-window-with-last-tab patch. Repeated the targeted ownership-header search: 347 patch files searched, no matching patch header for `browser.h`, `browser_window_interface.h` or `create_browser_window.h`. This confirms that scoped observation, not the absence of every possible ownership change elsewhere. The series positions checked agree with the document. The patch series was not applied or exhaustively reviewed.

The [round-2 lifecycle review](lifecycle-review-round-2.md) supplies the explicitly pinned v2 semantic scope. Its header, implementation and contract hashes agree with the reference table. This review does not rerun that model review or interpret later policy-worker changes as defects. The supplied task establishes the author's verified Astra advisory provenance; this review did not repeat the advisory assignment lookup or use advisory agreement as evidence of native correctness.

## Finding

### P2. Update the authority and version boundary before model binding

Locations: [Authority and evidence](../native-space-integration.md#authority-and-evidence), [Proposed ownership seam](../native-space-integration.md#proposed-ownership-seam), [Implementation slices and proof gates](../native-space-integration.md#implementation-slices-and-proof-gates), and [Source identity](../native-space-source-references.md#source-identity).

The plan still says one Profile per Space and settings scope are unapproved. Its final owner-choice paragraph also leaves navigation routing unresolved. Those statements predate the new approvals supplied to this review. Leaving them as current authority could lead an implementer to omit required sharing, theme or navigation behavior, or ask the owner to settle decisions already made. This is a documentation synchronization finding, not an original error by the author.

Required update:

1. Record the later authority explicitly: each Space has one Profile; multiple Spaces may share that Profile; sign-ins, site settings and Favorites are Profile-scoped; themes are Space-scoped; outbound links from saved pages open local temporary branches. Remove those approved choices from the unresolved list. Keep safety signals, cleanup, destructive actions and migration/source-disposition choices open. Do not infer approval of private/guest persistence or other unspecified policies.
2. Label the v2 hashes as the historical semantic baseline evaluated by this source investigation. Replace claims about the "current model" with that revision scope where appropriate. Preserve the old review's identity rather than silently changing its hashes to a newer model. Link the subsequent contract/model revision when available and state which integration assumptions were rechecked. Expected current hash drift alone is not a model bug.
3. Add the approved behavior to the implementation checks. Same-Profile Spaces must share the real Profile services and Favorites while retaining distinct temporary-page presentation and Space themes. Switching themes must not mutate a Profile preference that changes every Space using it. Outbound-link routing must create a temporary branch in the requesting attachment under the originating page's real Profile, preserve the saved page, and retain native URL/origin/opener/security handling. The routing approval does not settle destructive reset or cleanup behavior.

The theme requirement needs an explicit seam because the pinned [BrowserWidget theme path](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/frame/browser_widget.cc#340) currently resolves themes through the browser's Profile. Retaining a Profile-specific chrome tree by itself cannot supply distinct themes for Spaces sharing that Profile. This does not refute design A; it adds a concrete presentation responsibility following the later approval.

## Ownership and risk assessment

The main source conclusions are sound:

- **Production construction and lifetime.** The [desktop interface](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/browser_window/public/browser_window_interface.h#83) promises immutable Profile ownership and homogeneous tabs. Browser stores a const Profile pointer and constructs its strip with that Profile. Its custom-window branch actually enforces `CHECK_IS_TEST`. BrowserView captures its owner and builds Profile-bound chrome; BrowserWidget uses that view as delegate and synchronously destroys the Browser after native-widget destruction. A production ownership refactor is therefore a supported conclusion. Source inspection does not prove that extracting it will succeed.
- **Commands and security.** Toolbar/location-bar construction, command-controller observers, native-window properties and Helium's cached shortcut service have real Profile/browser dependencies. Replacing a getter or attaching different contents cannot repair those captured owners. Navigation derives its initiating Profile from the source browser, and cross-Profile navigation clears source/opener information. The plan correctly preserves that boundary rather than treating a navigation API as live-state migration.
- **Same live page.** The [detach/insert paths](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/tabs/tab_strip_model.cc#446) support the proposed candidate mechanism: one path transfers a TabModel; the WebContents-returning path destroys its wrapper. TabModel removal clears opener/blocked bookkeeping, and insertion invokes delegates and can activate an empty strip. The plan exposes these effects, last-tab closure, modal/capture ownership and exclusive ownership during failure. It does not claim those APIs supply an atomic transaction or a proven rollback route.
- **Profile and session ownership.** WebContents/controller retain their BrowserContext; its lifetime must cover the contents. SessionService tracks its own Profile, tab-to-window association and selected index. Workspace host grouping and durable IDs therefore need a separate join/recovery design. The plan correctly distinguishes that metadata from native navigation history and distinguishes restart reconstruction from uninterrupted live state.

Design A is a reasonable preferred investigation because it retains homogeneous Profile services and page ownership. Design B is honestly presented as a broader alternative, not ruled out by assertion. Design C is correctly conditional on extracting ownership and dispatch. The document does not present any of them as an existing API. It expressly rejects replacement/hidden OS windows, Profile relabeling, cloned navigation, dummy tabs and fake adapters as acceptance evidence.

The slices expose the essential engineering risks rather than deferring them behind model success: production creation/destruction, Profile/process keepalives, empty contexts, callback/accelerator deactivation, command ownership, exact-object transfer, source/destination closure, failure recovery, split pruning, session recovery and platform interaction. Native-window lookup and browser-wide service users still need an exhaustive inventory in the extraction slice. The current cited examples are sufficient to justify that work, not to claim the inventory is complete.

## Next coherent implementation gate

After the authority note is synchronized, complete slice 1's production ownership extraction design. Identify the changed construction/destruction edges and interface contracts for BrowserManagerService, BrowserWindowFeatures, BrowserView/BrowserWidget, Profile/process keepalives, native-window lookup, commands and Profile-local session registration. Select how Profile-bound chrome is hosted and how Space theme presentation is applied. Explicitly resolve behavior when a context empties while the host or other contexts remain live.

That design should then gate a focused native proof in a separately authorized pinned prepared tree. It must retain the same real top-level OS window while presenting distinct real Profiles, create no backing top-level windows, route actual navigation and commands through the presented Profile, and demonstrate same-origin synthetic storage separation. A WebView-only demo or test-only custom BrowserWindow cannot pass. Failure keeps the architecture question open and requires evaluating the competing design without weakening the approved UX.

Only after that proof should the same-object saved-page transfer proof and semantic prepare/commit boundary be connected. Preserve TabModel, WebContents, controller and document identity, including last-tab transfer and failure with a closing destination/source. No feature should ship on the assumption that the detached unique pointer alone proves recoverability.

## Verification limits

This review accepts a source-grounded preparatory artifact, subject to the authority update. It does not accept a production API, prepared patch set, browser implementation or closure of issue 32. No model fixture, browser test, full native build, installation, visual interaction or platform acceptance scenario ran. Full native builds are not required to accept this qualified design review. The already-listed native proof, persistence, failure, accessibility and Linux/Windows/macOS acceptance gates remain open.

The report received a focused whitespace check. No Markdown formatter/linter is declared in the inspected repository configuration. No unrelated test suite was run for this report-only change.
