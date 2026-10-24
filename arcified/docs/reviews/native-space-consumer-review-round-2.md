# Native Space consumer inventory review, round 2

Recorded 2026-10-23. Result: all three findings from the [original independent review](native-space-consumer-review.md) are closed. No new actionable finding was found in the correction scope. The inventory can guide the first bounded BrowserWidget destruction-authority extraction. This closes the source-catalog corrections only. Native feasibility, exhaustive consumer coverage and enabled-feature/internal/reverse-caller closure remain OPEN.

Only this report was written. No product or source-evidence edit, retrieval, preparation, download, build, native/browser work, commit, remote write or PR operation was performed.

## Reviewed revisions and method

Read the original review and [response](native-space-consumer-response.md). Compared the complete current inventory and both feature catalogs with the exact Git blobs recorded by the original review. Directly checked the corrected interpretations against retained pinned source passages. Parsed the JSON, compared every declaration identity with the previous catalog, and checked the complete Markdown member sequence against JSON.

Current working-tree Git blob identities, calculated with `git hash-object` without writing objects:

| Artifact | Git blob |
| --- | --- |
| [Inventory](../native-space-consumer-inventory.md) | `67a9f31539949901ffc276fa78f7dc94bb747037` |
| [feature-members.json](../research/native-space-consumers/feature-members.json) | `a3831827b099413cdd658b8b5dae7155d3d5153c` |
| [feature-members.md](../research/native-space-consumers/feature-members.md) | `2386fbb23ccf73a1c306dc8488d3e1eaf0f383b8` |
| Response | `f2490e30480b39baf4dcb0ff941922ce85ea6adb` |
| Original review | `81c803dbe7a320666c022f89bf8a65ce2666b79f` |

Corresponding SHA-256 identities, calculated with `sha256sum`:

| Artifact | SHA-256 |
| --- | --- |
| Inventory | `83b9dd1a2ee8aeea8fd85f79e1aff8e63fd2c110a41e2ab549f36f4c0cea2029` |
| feature-members.json | `f2da358a85f181fd778ead2c2cb613cf95f9a88c5839896a3796d6de240e8bf8` |
| feature-members.md | `c0f884fa66f6616e6208349d0bbf6732d2b181167904a889ad6a869e2e5ac6b5` |
| Response | `eb126d01d18ac88889133baa4a3f7d86923db66cf0f13640b10a12e548b73a9f` |
| Original review | `bdab4a9a025c83a74a2c986fd226654efbd7cbb2008b08379716f8ac9a85e74b` |

The comparison baselines were inventory `d9800d510dbb3b4a5690a473d80fa524b515d776`, JSON `85805bb9a3ad8f7338f0185c2f29084106001b34` and Markdown `399845262266eb950fcebbaf9e011507d421b650`. All were available as local Git objects. The comparison used their complete contents, not selected hunks.

## Finding closure

1. **Feature lifecycle accounting, closed.** Retained [browser_window_features.cc](../research/native-space-consumers/chromium/chrome/browser/ui/browser_window/internal/browser_window_features.cc), lines 1253 to 1254, conditionally obtains a Views provider through `browser_elements_->AsA<BrowserElementsViews>()` and invokes `provider->TearDown()`. The JSON now records cleanup at 1254 with the condition at 1253; the Markdown records both coordinates and the conditional Views call. Both preserve its position before browser animation reset at 1256 and actor-border reset at 1257. This is provider cleanup, not a reset of `browser_elements_` or an unconditional WebUI teardown. The borrowed `pinned_toolbar_actions_` acquisition remains at 1041. Its null assignment at 1153 remains cleanup and has been removed from initialization in both catalogs. Lazy `find_bar_controller_` construction at 1281 and its cleanup at 1249 are unchanged in both representations and agree with the retained implementation.
2. **E16 dialog and parent, closed.** Retained [devtools_window.cc](../research/native-space-consumers/chromium/chrome/browser/devtools/devtools_window.cc), lines 1971 to 2006, implements `ShowCertificateViewer`. At 1991 to 1992 it chooses the inspected page when docked and `main_web_contents_`, the DevTools frontend, when undocked. The subsequent tab-owner lookup supplies that selected contents' native parent. E16 now states this distinction, proposes the appropriate inspected-page or frontend binding, and names pending certificate UI in its checks. It preserves the separate actual host for undocked tools. The neighboring DevtoolsUIController and CreateDevToolsBrowser claims are unchanged.
3. **E17 insertion and replacement, closed.** Retained [session_service_browser_helper.cc](../research/native-space-consumers/chromium/chrome/browser/ui/sessions/session_service_browser_helper.cc) assigns `SessionTabHelper` window identity in the `kInserted` path at 92 to 101 and separately in the `kReplaced` path at 142 to 147. The former uses inserted contents; the latter uses `replace->new_contents` after the old contents' closing notification. E17 now cites the paths separately. Its construction citation stops at 85, before insertion handling. The neighboring Profile-local service lookup, Browser record lifecycle and platform persistence claims are unchanged.

The response accurately describes these corrections. Its statement that fresh closure is pending is resolved by this report; the response itself was left intact.

## Change containment and counts

The complete inventory comparison changes only E16's two paragraphs and E17's source paragraph. The JSON changes only `browser_elements_` cleanup metadata and removal of the false line-1153 initializer from `pinned_toolbar_actions_`. The Markdown changes only those two member rows. No other inventory prose, feature row, classification, guard or lifecycle coordinate changed.

The JSON still has exactly 122 distinct member entries, comprising 119 `unique_ptr` owners and three borrowed pointers. Every member name, declaration coordinate, ownership kind, proposed owner and declaration guard matches the prior catalog. Markdown has exactly 122 distinct member rows in the same order as JSON. E01 through E25 remain consecutive and unique. These checks preserve the original review's complete captured-declaration accounting. They do not establish an enabled 124-member Helium build from the two downstream additions.

As requested, this closure reuses the original independent review's retained raw-source identity and wider source evidence. That review checked 75 source files, 12 shallow listings and one tag record, with 2,436,067 source bytes, against the manifest and retrieval records at Chromium `155.0.8059.79`, commit `a49e2e3d4addfd84e765b52afe230bedb81096db`. Its direct-parent listing cross-check covered 53 source files; the remaining 22 had retained retrieval-hash consistency only. Those are retrieval and consistency counts, not full-file review or exhaustive consumer counts. This round did not repeat the raw-response hashing, local-input recertification, patch/search replay or wider consumer audit, and makes no new upstream identity claim.

## First bounded patch and remaining limits

The recommendation still targets a real production lifetime dependency. Retained [browser_widget.cc](../research/native-space-consumers/chromium/chrome/browser/ui/views/frame/browser_widget.cc), lines 113 to 142, closes owned widgets and then reaches through `browser_view_->browser()` for feature pre-window teardown while widget/view state is available. Lines 394 to 406 again derive the Browser from BrowserView, force its close path, invoke the native base callback and request synchronous manager destruction. Moving that authority into the production lifetime owner removes an actual dependency if the widget no longer derives the destruction target from the presented view. A forwarding wrapper that retains that dependency would not satisfy the recommendation.

The first-patch text is unchanged. It requires ordinary-window ordering, invalidation and exactly-once completion, including already-posted deletion. The original review's construction-time registration, owner-initiated shutdown/destruction, reentry, subscription-lifetime and keepalive qualifications still apply. Preserve the entire feature pre-window hook, including the newly cataloged conditional provider cleanup. No implementation or passing lifecycle test is claimed here.

The first patch remains independently useful without claiming a windowless Browser, a replacement widget delegate or same-window multi-Profile feasibility. Exhaustive affected-consumer, enabled-feature/internal/factory/reverse-caller closure remains OPEN. Actual host/context construction, continuous native top-level identity without hidden/replacement backing windows, command/Profile/storage isolation, shared-Profile services and Space themes, empty/failure behavior, exact-object/document transfer and recovery, session joins, and installed Linux/Windows/macOS acceptance remain OPEN. The unresolved policy choices remain unchanged.

Verification was limited to the complete artifact comparison, catalog parsing/count/identity checks, targeted retained-source interpretation, hashes, and this report's local-link and whitespace checks. Wider source conclusions are inherited with the original review's limits. No tests, native checks or runtime checks ran. The prior review recorded no configured Markdown formatter or linter; none was installed or run for this report.
