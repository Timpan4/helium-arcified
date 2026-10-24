# Saved-page window handoff

Status: reusable preparation and revalidation for [issue 72](https://github.com/Timpan4/helium-arcified/issues/72), against [lifecycle contract v3](../lifecycle-contract.md). Native host integration and installed Linux, Windows and macOS acceptance remain open. The issue's [publication brief](../research/arc-dia/publication/window-ownership.md) requires actual native interaction, failure and persistence evidence before closure.

The [handoff component](../../native/features/window_ownership/handoff.h) reads the existing lifecycle `Model`. It prepares an advisory plan for bringing the same saved live page to an existing attachment. It never calls a mutating Model command or a native API. Planning, refusing, revalidating or abandoning the plan leaves the Model unchanged. This covers preparation cancellation, not rollback after a native detach.

## Interface and authority

`PrepareSavedPageHandoff(model, page, destination, native_source_active)` follows `Model::TransferSavedPage` preconditions. The source must be a saved live page. The destination must already be attached and use the page's immutable Profile. A Space-owned item must remain in its owning Space. A standalone Profile shortcut can use another attached Space in its Profile. A same-attachment plan has no split effects.

`PrepareOpenItemHandoff(model, item, requesting, native_source_active)` handles only `Model::OpenItem`'s already-live branch. It first checks the requesting attachment and Profile. A Space-owned item resolves to its owning Space in the requesting window, which must also be attached. A Profile-owned item uses the requesting attachment. `kItemNotLive` leaves closed-item opening with the Model and future native creation path. No candidate `PageId` is allocated or registered. Alias resolution remains with existing core identities and `OpenShortcut`; this component has no pins, Favorites, label or presentation dependency.

`HandoffPlan` holds owned lifecycle IDs and references. It records source/requested/resolved attachments, the unchanged Profile, selected-Space snapshots read from the Model, focus reference intent and expected split-reference pruning. It contains no URL, current navigation, renderer/history state or activity snapshot. It is advisory, not a reservation, transaction, native success report or durable record. Callers keep prepared values unchanged.

The Model remains the sole owner of attachments and selected Space. `FocusIntent::source_active` is explicit caller/native input. Preparation checks that a supplied page is live in the source window; it cannot establish that the native UI actually considers that page active. `destination_page` references the page to present after successful handoff/selection. It does not select or focus anything, choose a source fallback, track destination active state or establish a selected-page registry. Native source and destination focus, commands and accessibility must be coordinated and verified by the future host.

`RevalidateHandoff(model, plan, fresh_native_source_active)` checks the source identity, saved association, Profile and attachment, then repeats destination resolution against the current Model. It refuses changed selected Spaces, changed or invalid native active-page references, and changed affected split references. Missing/closed windows and detached or reprofiled requesting/resolved destinations invalidate the plan. Navigation and activity changes remain valid because transfer preserves the live page's latest state.

Split effects identify each affected `SplitId`, its original attachment and ordered page references before/after pruning. They predict the existing Model operation, not a new split policy. Empty layouts disappear; a remaining singleton stays in the Model. No layout moves with the page, no destination layout is invented, and no focus or pane geometry is chosen. Unrelated split edits do not invalidate the preview.

## Future native use

1. Prepare from the authoritative Model and caller-native active-page reference. Handle refusal without detaching the source. If pins/Favorites supplied a canonical `ItemId`, use it directly; label and alias presentation do not create another owner.
2. Serialize conflicting operations on the native UI sequence. Revalidate the plan and actual source/destination native objects, including Profile identity, operation generation, shutdown and modal state, before detachment. `kCurrent` means only that the observed Model references and supplied active input still match. It does not detect an intervening move away and back, native handle replacement, native callbacks or unreported events.
3. Transfer the exact native page through a proven ownership-preserving path. Confirm native insertion and presentation before reporting `TransferSavedPage(plan.page, plan.destination)` or the existing-live `OpenItem(plan.item, plan.requested_destination, candidate)` outcome to the Model. `OpenItem` ignores an unused candidate ID on this branch. Neither planning nor revalidation provides atomicity across native callbacks and the Model report.
4. Coordinate selected Space through existing `Model::SelectSpace` and native presentation separately. Retain/recover the original native object on cancellation, stale completion or insertion failure. A failure after native detachment requires a real recovery protocol; this component supplies no fake host adapter or rollback success. Source closure and last-tab host lifetime are unresolved native gates.

There is no general temporary transfer, live Profile reassignment, close/reopen persistence, cleanup, migration transaction or native navigation controller here. Durability owns serialization and IO. Profile safety work owns mutation/migration transactions. Activity detection, cleanup, destruction and populated cross-Profile migration remain OPEN under v3.

## Pinned native gap

The existing [native integration design](../native-space-integration.md) and [source references](../native-space-source-references.md) record static evidence for Chromium `155.0.8059.79`, commit `a49e2e3d4addfd84e765b52afe230bedb81096db`. Their v2 policy and lifecycle hashes describe that earlier review. Current authority for this component is v3 and the approved task brief, including one Profile per Space with shared Profiles allowed, Profile-scoped sign-in/settings/Favorites, Space-scoped theme and saved outbound temporary branches.

The pinned [desktop window interface, lines 77 onward](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/browser_window/public/browser_window_interface.h#77) requires an immutable window Profile and same-Profile desktop tabs. The recorded [Browser construction evidence](../native-space-source-references.md#c1-desktop-window-and-profile-ownership) ties one Profile, strip and window together. [BrowserView/BrowserWidget and command coupling](../native-space-source-references.md#c2-view-widget-and-command-coupling) prevent a view swap or selected-Space update from proving safe same-window multi-Profile operation. Production host extraction remains unproven.

The pinned [tab strip implementation, lines 446 onward](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/tabs/tab_strip_model.cc#446) distinguishes preserving a detached TabModel from extracting WebContents and destroying its wrapper. The recorded [handoff evidence](../native-space-source-references.md#c3-live-identity-and-handoff) also identifies same-Profile drag restrictions, native observer/delegate effects and `Browser::TabStripEmpty` closing the source window. These are constraints to resolve, not proof of a complete transfer path. Hidden backing windows, a dummy page, reload/clone, a changing legacy Profile getter or removal of Profile checks do not meet the approved same-actual-window behavior.

No Chromium preparation/build, native patch, global installation, CI, browsing data or AI/service access was used. Native page/document/history/form continuity, storage/authentication isolation, last-tab lifetime, failure recovery, focus/accessibility and restart/concurrent-edit durability remain unverified on every target platform. Native host acceptance is blocked by the ownership extraction gap and deferred builds.

## Focused synthetic checks

[handoff_test.cc](../../native/features/window_ownership/handoff_test.cc) uses synthetic IDs and `.test` destinations. It checks cancellation/refusal source preservation; protected and unknown page state retained by separate Model reports; exact requesting/owning Space resolution; Profile-shortcut transfer; no candidate allocation; same-attachment layout retention; empty/singleton split effects; stale source/replaced page, destination/window/Profile, selection, native active reference and split edits; and parity with actual Model transfer/open commands. Navigation/activity changes remain Model-owned and do not stale a plan. These are standalone C++ checks, not native browser tests or evidence of native failure rollback.

Run from the repository root with the existing C++17 toolchain. The Clang driver mode follows the [lifecycle check contract](../lifecycle-contract.md#synthetic-transition-fixtures-and-focused-checks).

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/window_ownership/handoff.h \
  arcified/native/features/window_ownership/handoff.cc \
  arcified/native/features/window_ownership/handoff_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/window_ownership/handoff.cc \
  arcified/native/features/window_ownership/handoff_test.cc \
  -o /tmp/arcified-window-ownership-test
/tmp/arcified-window-ownership-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/window_ownership/handoff.cc \
  arcified/native/features/window_ownership/handoff_test.cc \
  -o /tmp/arcified-window-ownership-sanitized
/tmp/arcified-window-ownership-sanitized
```

The strict compile/run, AddressSanitizer/UndefinedBehaviorSanitizer compile/run and Chromium-format dry check passed on the current Linux host. Windows/macOS compilation and installed native acceptance remain open. No build target or dependency was added.
