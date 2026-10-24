# Profile Favorites

Preparatory implementation slice for [issue 106](https://github.com/Timpan4/helium-arcified/issues/106). Native acceptance remains open on Linux, Windows and macOS. The [publication brief](../research/arc-dia/publication/favorites.md) and `favorites` entry in [backlog.json](../research/arc-dia/backlog.json) define the acceptance criteria.

The [lifecycle v3 contract](../lifecycle-contract.md) remains authoritative. A standalone Favorite is a Profile-owned `ItemId`; a Favorite for a kept Space item is a Profile-owned `ShortcutId` referencing its canonical `ItemId`. Neither representation creates another live page or navigation owner. Distinct items saving the same destination retain distinct identities. No capacity is imposed; choosing capacity remains an explicit owner decision.

## Reusable component

[`favorites.h`](../../native/features/favorites/favorites.h) and [`favorites.cc`](../../native/features/favorites/favorites.cc) implement a stateless C++17 projection over `const lifecycle::Model`.

| API | Result and boundary |
| --- | --- |
| `ResolveFavorite(model, requesting, reference)` | Validates the requesting attachment and Favorite's Profile membership. Returns owned reference identity, canonical `ItemId`, model `ProfileId`, explicit destination attachment and `destination_attached`. A direct Space-owned item is not a Favorite; its Profile alias is. |
| `EnumerateFavorites(model, requesting)` | Returns every Profile-owned item and alias in the requesting attachment's Profile. Does not hide aliases when the owning Space is unattached. Traversal order is not user order. |
| `OrderFavorites(model, requesting, order)` | Validates an exact caller-supplied order of the complete Favorite reference set. Rejects duplicate, missing, stale, non-Favorite and cross-Profile references. Owns no order store and never silently drops or appends references. |
| `DescribeFavorite(model, requesting, reference, metadata, default_label)` | Resolves scope, then calls the real pins `DescribeSavedItem` with the canonical item. Returns that shared label/live-page presentation together with the Favorite target. |

`FavoriteReference` is `variant<ItemId, ShortcutId>`, so an item and alias with identical ID text remain distinct. Multiple explicit aliases of one item also remain distinct references. They share the target's saved destination and live page, not a copied lifecycle. The model graph is trusted after its public commands validate it. These functions do not parse a disk record or accept an unvalidated external ownership graph.

The caller keeps custom labels in the common [`pins::SavedItemMetadata`](../../native/features/pins/saved_item.h), keyed by canonical `ItemId`. To rename or restore the default, resolve the Favorite, look up its item's metadata and use pins `PlanLabelEdit`/`ApplyLabelEdit`. There is no Favorite-specific label store or edit history. Default labels come from the caller's native presentation. No icon token design was approved; icon editing stays open.

All outputs are snapshots. Recompute after model, metadata or title changes. The caller serializes commands and revalidates before native actuation; these values are neither reservations nor native authorization. A model `ProfileId` never establishes regular/private context access. This slice has no native Profile handles, active-page selection, focus, persistence, ID allocation or navigation payload.

## Native integration boundary

Opening a standalone Favorite uses `Model::OpenItem`; opening an alias uses `Model::OpenShortcut`. These are synthetic transitions, not native executors. Native actuation must follow the [native Space integration plan](../native-space-integration.md), preserving actual Profile and page identity before reporting success. In particular, an alias can be visible from a same-Profile Space while its owning Space is not attached to the requesting window. The UI must offer an explicit attachment/switch action; it cannot silently rehome the target.

The source constraints in [C1 and C2](../native-space-source-references.md#c1-desktop-window-and-profile-ownership) prevent treating a changing sidebar as proof of same-window Profile isolation. [C3](../native-space-source-references.md#c3-live-identity-and-handoff) identifies the native handoff and object-lifetime requirements. [C4](../native-space-source-references.md#c4-navigation-and-session-boundaries) requires Profile-local session ownership. A Favorite projection supplies none of these native mechanisms.

## Acceptance coverage and gaps

| Requirement | Local evidence | Remaining acceptance |
| --- | --- | --- |
| Add/remove/reorder and custom labels/icons with keyboard equivalents | Model commands supply identities and conservative removal semantics. Exact caller order is validated without storage. Favorites reuse pins label editing/presentation by canonical item. | Native commands, keyboard/accessibility, durable ordering/metadata and icon editing remain open. |
| Close/reset preserves the saved destination | Synthetic fixtures exercise reset refusal for protected activity, reset after reported quiescence, close and reopen at the saved destination. | Native close cancellation/reset, approved activity/destructive policies and installed interaction remain open. |
| Profile separation and restore across windows/restart | Synthetic projection fixtures cover Profile membership, typed identity, duplicate destinations, missing attachments and stale ordering. Lifecycle fixtures exercise same-Profile handoff of the same protected live page and alias attachment requirements. | Real Profile storage isolation, crash/restart consistency and native session joins remain open. |
| Explicit capacity decision | No cap is introduced. | Owner decision remains open. |

These fixtures use synthetic identities and reserved `.test` destinations. They access no browsing data or providers and launch no browser. Passing standalone checks cannot establish native focus, renderer continuity, storage isolation, persistence, accessibility or any installed-build result. Chromium preparation/build, downloads, benchmarks and remote CI remain deferred.

## Focused checks

Run from the repository root with the existing C++17 toolchain. The explicit Clang driver mode follows the standalone model check documented in the lifecycle contract.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/favorites/favorites.h \
  arcified/native/features/favorites/favorites.cc \
  arcified/native/features/favorites/favorites_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/pins/saved_item.cc \
  arcified/native/features/favorites/favorites.cc \
  arcified/native/features/favorites/favorites_test.cc \
  -o /tmp/arcified-favorites-sanitized
/tmp/arcified-favorites-sanitized
clang-tidy --checks='clang-analyzer-*' --warnings-as-errors='*' \
  arcified/native/features/favorites/favorites.cc \
  arcified/native/features/favorites/favorites_test.cc -- -std=c++17
```

The eight named fixtures in [`favorites_test.cc`](../../native/features/favorites/favorites_test.cc) cover Profile/reference projection, explicit destination attachment, exact caller order, absent attachments, typed reference identity, shared pins metadata, Favorite lifecycle and alias owning-attachment requirements. They check the component's public results and existing Model transitions. They do not prove runtime browser behavior or restart restoration.

Local checks on 2026-10-23 passed Chromium-style formatting, strict C++17 compilation with ASan/UBSan, all eight fixtures and the explicit `clang-analyzer-*` clang-tidy checks without diagnostics. The host's default clang-tidy invocation enabled no checks, so the recorded command explicitly selects the analyzer checks. New-file `git diff --no-index --check` checks emitted no whitespace diagnostics; exit 1 reports new-file differences. No installed native interaction or platform acceptance ran.
