# Native Space design response

Recorded: 2026-10-23. Scope: P2 and the next source-design gate in the [independent design review](native-space-design-review.md). Only [native-space-integration.md](../native-space-integration.md), [native-space-source-references.md](../native-space-source-references.md) and this response are owned by this task. This is an implementation response for the preparatory documents, not a fresh independent review, native implementation or issue 32 acceptance.

## P2 authority and version response

The plan now records the later approved mapping as authority: each Space chooses exactly one Profile, multiple Spaces may share it, sign-ins/site settings/Favorites are Profile-scoped, themes are Space-scoped, and outbound links from saved pages create local temporary branches under the originating page's actual Profile and current attachment. The unresolved list no longer reopens those choices. Safety signals, cleanup, destructive actions and populated cross-Profile migration/source disposition remain **OPEN**, along with other unspecified policies. Outbound-link approval does not settle same-site or other navigation triggers, private/guest persistence or destructive behavior.

The historical v2 header/implementation/contract hashes and [clean v2 lifecycle review](lifecycle-review-round-2.md) remain intact. The [source identity section](../native-space-source-references.md#source-identity) labels those hashes as the original investigation baseline and records separate v3 hashes. It links the [v3 contract](../lifecycle-contract.md) and [policy response](lifecycle-policy-response.md). The integration plan explicitly adopts v3 at the future native interface and identifies rechecked assumptions. V3 hash changes do not imply a model defect or extend the v2 independent review to the new revision.

The original verified Astra assignment, task ID, completion and qualified advisory result are unchanged. Neither advisory agreement nor the original review proves native feasibility. The review found no blocking source-interpretation or architecture defect within its preparatory scope; this response preserves that finding and its limits.

The new [approval conformance checks](../native-space-integration.md#approval-conformance-checks) require actual shared Profile services/Favorites, distinct local temporaries, independent Space themes without Profile-wide theme mutation, and native outbound-link actuation preserving the saved source and native navigation/security handling. All remain open for native execution.

## Production source-design response

The [ownership extraction design](../native-space-integration.md#production-ownership-extraction-design) chooses design A with retained Profile-bound child chrome for the focused proof. It selects a homogeneous tab model per host/actual Profile with an attachment projection. Proposed production interfaces specify:

1. A process-owned physical host/widget, separate from Profile-keyed context ownership and immutable Profile/tab/service identity.
2. Production host/context creation and ready/failure publication, distinct host versus context collection/activation semantics, and Profile/process keepalive ordering.
3. Separation of `BrowserWindowFeatures` context/host state from presentation attachment, retained child chrome, host-owned widget delegate/destruction, native lookup and explicit command/callback targets.
4. Space theme presentation through host/attached-chrome theme resolution, independently of Profile ThemeService preference changes.
5. Empty attachment/context behavior without dummy pages or host closure, coordinated teardown, and Profile-local session registration joined to the actual host and durable workspace identities.

New [C5 source references](../native-space-source-references.md#c5-production-extraction-edges) ground the contracts in production Browser creation, BrowserManagerService/factory, feature lifecycle, desktop creation, session helpers and global collection forwarding. Existing C1 through C4 and H1 evidence remains in place. The additional investigation fetched individual files at the pinned commit, not a checkout/archive or prepared source tree. The web reader could not access Gitiles; direct read-only HTTP succeeded. An incorrect guessed feature-header path returned 404 and was replaced using the directory listing.

The [source-backed native proof sequence](../native-space-integration.md#source-backed-native-proof-sequence) first requires the changed-owner/consumer inventory, then actual production construction and the same platform-native top-level window through real Profile presentation, navigation/commands, storage isolation, shared-Profile/theme checks and empty-context/failure cases. Continuous native-window creation/destruction evidence must rule out hidden or temporary replacement backing windows. A logical window ID, a WebView-only demonstration, test-only BrowserWindow injection or superficial Profile getter/property changes cannot pass.

Only after that proof may exact-object saved-page transfer and the native prepare/commit interface be connected. TabModel, WebContents, controller and document identity, last-tab source behavior, closing source/destination, insertion failure and retained ownership/recovery remain required evidence. A detached unique pointer alone proves no recovery protocol. V3 link planning remains a pure snapshot decision with separate native classification, actuation, success reporting and race reconciliation.

## Verification and open work

Source/document verification covered the review's P2 requirements, the v3 authority ledger, relevant model identity/routing declarations and decision path, and the additional pinned production ownership passages in C5. It did not rerun the lifecycle fixtures or perform a fresh independent model review. The historical reports and Astra paragraph were preserved.

Documentation checks resolved all local link targets and heading anchors across the three owned documents. Consistency checks confirmed explicit v3 adoption, retained historical source hashes, open policy boundaries, independent theme presentation and actual-window proof requirements. Direct content comparison confirmed the original design review, v2 lifecycle review and Astra paragraph were unchanged; rehashed v3 inputs still match the new reference table. Files retain LF line endings. Focused `git diff --check` for the tracked documents and `git diff --no-index --check /dev/null` for this new response emitted no whitespace diagnostics. The latter's status 1 denotes new-file differences. No Markdown formatter/linter is declared in the inspected repository configuration; no dependency or formatter was installed for this task.

The interface design is documented. The exhaustive enabled-feature and consumer inventory remains open, followed by a separately authorized pinned prepared-tree proof. No implementation facade, code change, full source preparation/build, native/browser interaction, installation, remote write or commit was performed. Native feasibility, Linux/Windows/macOS installed acceptance, persistence/recovery and issue 32 remain unproven. These native gates do not reopen the approved behavior or block scoped acceptance of the qualified documentation response.
