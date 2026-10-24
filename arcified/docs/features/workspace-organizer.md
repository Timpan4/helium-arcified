# Workspace organizer

Status: read-only C++ component and synthetic checks for [issue 147](https://github.com/Timpan4/helium-arcified/issues/147). Native host integration and acceptance remain blocked. No installed browser interaction is implemented or proven.

## Authority and interface

[inspection.h](../../native/features/workspace_organizer/inspection.h) and [inspection.cc](../../native/features/workspace_organizer/inspection.cc) derive disposable identity projections directly from the [lifecycle v3 Model](../../native/model/lifecycle.h). The Model remains the sole ownership, navigation, attachment and selected-Space authority. There is no second bookmark model, navigation record, selected-page registry, persistence layer or mutation transaction.

Call these APIs on the Model's single-writer sequence. Every result owns its identity values and becomes advisory evidence about an earlier state once the Model changes. Reinspect before acting through an authoritative component. A result is neither a reservation nor native success evidence. The component has no callbacks or native side effects. Dismissing a preview drops values without beginning or cancelling a transaction.

| API | Result and meaning |
| --- | --- |
| `Inspect(model)` | All registered Profile, Space, window, attachment, item, alias, page, temporary-tab and split identities, with typed reference edges. |
| `Inspection::ReferencesTo(identity)` | Direct incoming Model references. Relation kinds explain ownership, Profile membership, alias target, selected Space, attachment and split membership. Transitive ownership can be followed through these edges. |
| `Inspection::UnattachedSpaces()` | Spaces with no current window attachment. They may own closed saved items. This does not mean lost, orphaned or safe to delete. |
| `Inspection::UnreferencedProfiles()` | Profiles with no incoming Model references. External users and Profile data remain uninspected. |
| `Inspection::MissingTargets()` | References whose target identity is absent from this projection. This diagnoses identity target presence only. It is not a complete graph validator or a native registry audit. |
| `PreviewProfileDeletion(model, profile)` | Missing identity, known references, or no Model references, plus the explicit `kUninspected` external-reference state. It never authorizes deletion. |
| `PreviewSpaceReassignment(model, space, target)` | Missing identities, same Profile, known reference blockers, or empty/unattached evidence. Populated cross-Profile migration is explicitly `kOpen`. It never creates a move/reopen plan. |
| `ResolveSidebar(model, item-or-alias, requesting)` | Profile, item, optional alias, organizational owner, owning sidebar attachment and optional current live-page identity/attachment. It never opens, transfers, selects or focuses anything. |

Snapshot iteration follows Model container order. It is not a durable or user-selected sidebar order. Split reference edges retain the Model's page iteration order without creating a layout, pane capacity or focus policy.

## Identity and provenance

The output types contain opaque Model IDs, attachments, relation/state enums and optional identity references. They contain no saved destination, current navigation, activity report, title, display label, browser object, credential or account payload. The implementation does not read destination or navigation fields. ID text must satisfy the lifecycle contract's opaque-identity requirement. The component does not sanitize a caller's misuse of an ID as a URL or secret container.

Provenance here means the relationships actually present in the Model. A Space resolves to its chosen Profile. A kept item resolves to its owning Space and then Profile. A standalone Profile shortcut resolves to its Profile. An alias resolves to its target item without duplicating the saved destination or live page. A page exposes immutable Profile membership and saved-item or original temporary-tab ownership. Windows expose the selected Space reference, and attachments and splits expose their referenced identities.

Imported labels, folder membership, external routing references, actual Profile availability and native registry joins are not modeled here. They are unknown, not proven absent. Valid public Model commands preserve reference integrity, so the synthetic checks do not claim to discover corrupt persisted data. A closed saved item and a Space without an attachment are valid states. `MissingTargets()` has a separate synthetic projection check for its narrow diagnostic behavior; there is no import or repair path that accepts that projection as authoritative state.

## Sidebar resolution

An ordinary kept item resolves to its owning Space in the requesting window. An alias retains its own `ShortcutId` and resolves the same target `ItemId` and owning Space. An alias visible from a sibling Space never changes the item's owner. If the owning Space is not attached in that window, the result explicitly says `kRequiresAttachment` and retains the intended Space/window IDs. A future UI may offer an explicit attachment/switch action through its authoritative owner.

A standalone Profile-owned item resolves in the requesting Space when that Space uses the same Profile. Cross-Profile requests return `kProfileMismatch`. An unattached requesting context returns `kNotAttached`; unknown item/alias identities return `kNotFound`. This is identity resolution, not a promise that an arbitrary item is currently rendered in the requesting sidebar.

For a live saved item, the result distinguishes its current page attachment from the requested sidebar location. Resolution does not transfer the page. An eventual selection action must use the Model/native handoff path and preserve the existing page identity, state and Profile. There is no selected-page or focus record here. Removing an alias makes that alias unresolvable while its target remains independently resolvable.

## Conservative previews and open policy

Deletion evidence lists direct incoming Profile references, including Spaces, standalone items, aliases and pages. References through Space-owned items remain traceable through the Space edge. The current Model refuses `RemoveProfile` while its owned/referencing entities remain. A Profile with no Model references still reports external references as `kUninspected`. No filesystem, credential or Profile directory deletion is implemented.

Reassignment evidence uses incoming Space references. Owned items and attachments block the existing `ReassignEmptySpace` transition. Selected-Space references are also shown; valid Model selection already requires an attachment. Same-Profile requests are identified separately, matching the Model's no-op semantics even for a populated Space. A different-Profile request with owned items explicitly reports that populated migration policy is OPEN. An attached empty Space remains blocked by current Model semantics without inventing a migration transaction.

The Profile safety slice owns reassignment preparation, transaction state and any approved saved-destination reopening. This component does not clone the Model to simulate a mutation, relabel pages, copy destinations, allocate IDs or decide source disposition. Empty/unattached evidence is not an instruction to execute reassignment. All external-reference coverage remains unknown. Activity, cleanup, destructive-action review and populated migration choices remain OPEN under the [lifecycle contract](../lifecycle-contract.md).

## Focused verification

[inspection_test.cc](../../native/features/workspace_organizer/inspection_test.cc) uses synthetic identities and reserved `.test` destination strings. It performs no requests and reads no personal browsing or AI data.

The checks cover complete entity projection and all relation kinds, shared saved identity, window-local temporary provenance, closed items in unattached Spaces, reference presence, payload independence, stale owned snapshots after page transfer/Space selection, alias and ordinary-item resolution, missing owning attachments, Profile mismatch, removed aliases, protected-work preservation, referenced deletion/reassignment refusal, same-Profile no-op evidence, empty/unattached evidence and explicit OPEN populated migration policy. Preview tests compare advisory evidence with public Model command outcomes without implementing a second transaction.

Run from the repository root using the existing C++ toolchain. Chromium style and the explicit Clang driver mode follow the existing lifecycle check commands.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/workspace_organizer/inspection.h \
  arcified/native/features/workspace_organizer/inspection.cc \
  arcified/native/features/workspace_organizer/inspection_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/workspace_organizer/inspection.cc \
  arcified/native/features/workspace_organizer/inspection_test.cc \
  -o /tmp/arcified-workspace-organizer-test
/tmp/arcified-workspace-organizer-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/workspace_organizer/inspection.cc \
  arcified/native/features/workspace_organizer/inspection_test.cc \
  -o /tmp/arcified-workspace-organizer-sanitized
/tmp/arcified-workspace-organizer-sanitized
```

These commands passed on the Linux task host. The strict compiler flags serve as the focused C++ warning check. Scoped `git diff --no-index --check /dev/null <new-file>` checks emitted no whitespace diagnostics for each added file; their exit status 1 denotes new-file differences. Local documentation link targets were checked for existence. These results do not establish Windows/macOS compiler compatibility or any native browser behavior.

## Native acceptance blockers

The [native integration design](../native-space-integration.md) and [pinned source references](../native-space-source-references.md) describe Browser/Profile/window ownership assumptions that still require a proven production extraction path. Same-window multi-Profile presentation, command isolation and saved-page native handoff are unproven. This component does not resolve that engineering gate. The native design predates v3; the v3 lifecycle contract governs this component where approved mapping and routing decisions supersede its earlier open-policy wording.

No Chromium source preparation/build, heavy download, global install, remote write or CI run is included. Linux native organizer UI, Windows/macOS compilation and runtime, persistence/crash recovery, accessibility, visual interaction, authenticated Profile isolation and native failure/cancellation remain unverified. There is no affected installed UI to visually verify in this component. Synthetic C++ checks prove identity projection and reference semantics only. Issue 147 remains open pending its native and Profile safety prerequisites and platform acceptance evidence.
