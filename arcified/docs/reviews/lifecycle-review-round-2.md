# Issue 24 ownership core review, round 2

Result: clean within the standalone version 2 contract/model scope. The accepted P1 is resolved in the reviewed revision. No additional actionable implementation defect was found. This result accepts neither an owner-unapproved product proposal nor a native browser implementation, and does not close issue 24.

Reviewed independently on 2026-10-23 against the [issue body artifact](../research/arc-dia/publication/lifecycle-contract.md), the [first review](lifecycle-review.md), the [fix response](lifecycle-fix-response.md), and the explicit owner decisions supplied with this review task. Production code, supplied fixtures and prior reports were not changed. This report is the only repository write from this review; supplemental test source and executables remain in `/tmp`.

## Reviewed revision and coverage

Repository HEAD: `09670c97b1c308d0fb31869b3a9a9bc6969bc47f`. The reviewed files are untracked, so HEAD alone does not identify their contents. Git blob hashes below identify the exact reviewed bytes, without adding them to Git.

| Artifact | Coverage | Git blob hash |
| --- | --- | --- |
| [lifecycle.h](../../native/model/lifecycle.h) | Entire file, public commands, getter views and private helpers | `f36597f957449716939fa9e9a3bfd7ef8092fef8` |
| [lifecycle.cc](../../native/model/lifecycle.cc) | Entire file, including mutation/refusal paths and reference cleanup | `8d698fd7ed22fc9b3e0e5ce55d39dc5ecb3b062d` |
| [lifecycle_test.cc](../../native/model/lifecycle_test.cc) | Entire file, all 13 fixtures and their assertions | `c327fe9c0c2265ed62034ef613752a8b7c8de31c` |
| [lifecycle-contract.md](../lifecycle-contract.md) | Entire file, decision authority, transition descriptions and closure limits | `1bd201d404603fd73c53fb9ea4d977a563c9c459` |

The issue artifact, prior review and fix response were read in full. Other working-tree changes, PR state and unrelated platform/build tooling were outside this review. No GitHub write, commit, PR registration or monitoring was performed.

## P1 resolution and equivalent borrowed-input hazards

All public mutating commands now take IDs, owner variants, attachments, strings and vectors by value. The owned parameters exist before validation or mutation. Read-only query references do not mutate the model. Both private helpers, `ErasePage` and `RemovePageReferences`, also take owned `PageId` values.

For `ClosePage`, split-vector compaction can no longer change the validated target before `pages_.erase`. Removing an empty layout cannot free the command ID. The supplied getter-sourced fixture verifies the requested quiescent page disappears, its protected sibling survives with its navigation intact, remaining split references resolve to the same attachment, and single-reference layouts disappear. The supplemental test exercises the requested page at the beginning, middle and end of a split, plus a single-reference source split, while protecting the siblings.

For `CloseWindow`, the window ID survives deletion of the page, split, attachment-set entry or window-map entry that supplied it. The preflight checks every local page before any cleanup. Independently tested borrowed IDs from a saved-page attachment, temporary-tab attachment, split attachment, attachment-set element and window-map key. Unknown and protected activity both refused each command with every stored field unchanged. Quiescent closure removed local pages, layouts, attachments and the window, retained the other window's temporary page, and retained the shared saved item. Sanitizers reported no invalid access.

The related mutation paths were inspected for the same mechanism. `SetSplit` owns the replacement ID, attachment and vector before replacing a layout. Item/shortcut/Space/Profile deletion and attachment removal own their selectors before erasure. Keep owns its inputs and copies the temporary attachment before replacing the owner variant. Payload assignment and owner reassignment receive owned values. Existing fixtures exercise getter-sourced replacement/removal, movement, detachment and reassignment. No equivalent lifetime defect was found.

## New transitions and test strength

`TransferSavedPage` validates the page kind, destination attachment, Profile and owning Space before mutation. Reference pruning changes only `splits_`, so its saved-page pointer in `pages_` remains valid. The pruning helper owns the page ID. Transfer preserves page identity, current navigation, activity, Profile and saved-item ownership; it removes old references and empty layouts without moving layout state. Same-attachment transfer preserves layouts. Its atomicity is the synchronous, single-writer semantic transition, not exception recovery or a transaction spanning native callbacks.

`OpenItem` and `OpenShortcut` route an existing saved item through that transfer and return its existing `PageId`. They do not allocate or overwrite a page using the unused candidate ID. A Space-kept item resolves to its owning Space in the requesting window; an alias still requires that attachment. A Profile-owned item can use another attached Space within the same Profile. The supplied fixtures cover protected saved-page transfer, unknown activity on a Profile-owned page, alias attachment refusal, wrong-Space/Profile/missing-destination refusal, and temporary transfer refusal. They test surviving split references and retained saved destinations, beyond status codes alone.

Supplemental tests pass a page ID directly from a split that is compacted and an attachment directly from another live page into transfer. They also pass the saved item ID directly from the live owner, a split attachment and an already-used candidate page ID into `OpenItem`. Successful transfer preserves protected navigation and the protected sibling. Rejected transfers and temporary migration preserve a snapshot of all stored fields. A graph checker verifies attachment targets, selected-Space attachments, item owners, shortcut Profile compatibility, unique saved-page and temporary-tab associations, immutable Profile compatibility, Space-kept placement, and distinct, nonempty split references that resolve in the correct attachment.

`SelectSpace` updates only the named window's selected attached Space. It permits switching between Profiles without changing any page Profile or attachment. The supplied fixture checks both Profile identities, local temporary visibility, saved-item scoping, wrong-Profile opening refusal, missing/unattached selection refusal and selection clearing on detach. Supplemental checks exercise borrowed selection inputs, selection sourced from the same selected value, and detachment of that selected attachment. These are logical ownership checks. They do not test cookies, authentication, renderers or native focus.

## Decision-artifact criteria and closure limits

| Issue criterion | Review result |
| --- | --- |
| Define identities/ownership; splits reference pages | Defined and represented by the typed model. The one-Profile-per-Space architecture remains explicitly proposed. |
| Explicit same-site navigation and active-work choices | Recommendations and conservative evaluation behavior are explicit, but owner decisions remain unresolved. Recording observed navigation does not choose routing, and an activity enum does not establish native safety signals or manual-close policy. |
| Versioned decisions and meaningful synthetic transitions | Version 2, rejection of other semantic versions, approval/proposal ledger and executable transitions are present. No account, capacity or timer is inferred. |
| Profile/Space mapping, shortcut/settings scope, move/reopen/deny and safe deletion | The proposed mapping and shortcut scope are evaluated. Refusals and explicit URL-only copy/reopen preserve modeled Profile identity and references. Settings scope and populated migration/source disposition still need owner decisions; no native migration is claimed. |
| Consequence-based labels and imported aliases | Temporary, Kept in this Space and Profile shortcut are distinguished. Imported labels are described as provenance aliases without adding another lifecycle. Import execution is outside scope. |

The task authorizes these ownership decisions: shared saved tabs, window-local temporary tabs, bringing the existing saved live page into the requesting window with state preserved, and switching Spaces across Profiles in the same native window with isolation. The contract records those decisions separately from its proposals. No additional proposal was treated as owner approved in this review.

The genuine remaining owner choices are the Profile/Space mapping and settings scope, same-site routing, precise active-work signals and destructive-action policy, additional live instances, populated cross-Profile migration and source disposition, closing shared state and undo/restoration, and split/transient presentation. The approved saved-page transfer and same-window switching must not be reopened as undecided choices merely because native implementation is absent.

This standalone revision can be accepted as the versioned local ownership core and synthetic decision evidence requested. Issue 24 is not yet a fully owner-adopted contract while its required same-site/activity choices and other declared ownership proposals remain unresolved. Native feature evidence is a separate future verification track. A full Chromium build is explicitly deferred and is not a prerequisite for this scoped acceptance. This review does not impose a full build or native feature implementation as a new acceptance criterion for the decision artifact.

## Focused verification

Independently ran with the existing Clang 23.1.3 on Linux:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/model/lifecycle.h \
  arcified/native/model/lifecycle.cc \
  arcified/native/model/lifecycle_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc arcified/native/model/lifecycle_test.cc \
  -o /tmp/arcified-lifecycle-round-2-sanitized
/tmp/arcified-lifecycle-round-2-sanitized
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I arcified/native/model arcified/native/model/lifecycle.cc \
  /tmp/arcified-lifecycle-round-2-adversarial.cc \
  -o /tmp/arcified-lifecycle-round-2-adversarial
/tmp/arcified-lifecycle-round-2-adversarial
```

Formatting passed. All 13 supplied fixtures and all three supplemental test groups passed with warnings treated as errors, AddressSanitizer and UndefinedBehaviorSanitizer. No sanitizer diagnostic occurred. The supplemental source and both executables are retained at the paths above. These checks establish behavior for the exercised synthetic cases, supported by inspection of all model mutation paths; they are not exhaustive execution of every possible command sequence. No separate unsanitized run or GCC check was performed in this round.

## Direct native gaps

No real Profile/BrowserContext, native tab/WebContents or window identity is bound to this model. Preserving strings and IDs does not prove transfer of a running renderer, navigation history, forms or authentication state. Same-native-window cross-Profile presentation still needs an implementation and direct cookie/storage/authentication isolation checks. Native failure, cancellation, before-unload, discard eligibility and actual activity reporting must be coordinated before semantic success is committed.

Persistence/restore, stale asynchronous events, non-reused ID allocation, URL validation, migration transactions, journal recovery, focus, accessibility and performance remain outside the model. No browser interaction or Linux/Windows/macOS native scenario execution was performed. Those limits remain visible without relabeling future native checks as unresolved owner approvals or requiring the deferred full builds. No global installation, process shutdown or cleanup deletion was performed; the prior review and test artifacts were preserved.
