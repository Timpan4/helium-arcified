# Profile safety impact preview

Preparatory implementation for [issue 103](https://github.com/Timpan4/helium-arcified/issues/103). The [C++ component](../../native/features/profile_safety/impact.h) inventories references in the existing lifecycle graph. It does not implement Profile reassignment, deletion, migration, native activity detection or recovery. Installed native acceptance remains open on Linux, Windows and macOS.

The scope follows the [publication record](../research/arc-dia/publication/profile-safety.md), matching `profile-safety` record in [backlog.json](../research/arc-dia/backlog.json), and [lifecycle contract version 3](../lifecycle-contract.md). The approved one-Profile-per-Space mapping, shared Profiles and Profile-scoped Favorites remain authoritative. Activity, cleanup, destruction and populated migration policy remain OPEN. Earlier version 2 discussion in the [native integration design](../native-space-integration.md) does not override the current contract.

## Interface and meaning

`profile_safety::Preview(const lifecycle::Model&, Request)` returns `PreviewError` for a missing source Space or Profile, or a missing destination Profile. Otherwise it returns an owned `Impact` snapshot containing only typed IDs, ownership/attachment references and reported activity. It does not read or copy saved destinations, current navigation, titles, form values or credentials. IDs follow the lifecycle contract's opaque identity requirement; this component does not sanitize caller-supplied IDs for logging or authorize exposing another Profile's data.

| Request | Direct reference inventory | Context only |
| --- | --- | --- |
| `ReassignSpace{space, target}` | The source Space, its attachments, selected-window references, Space-owned items, aliases targeting those items, pages attached to the Space and their split references. Includes a Profile-owned saved page currently attached there, without labeling its item as Space-owned. | Source Profile first, destination Profile second when different. Each lists standalone Profile items and all Profile shortcut aliases. These are visibility dependencies, not a migration list. |
| `DeleteSpace{space}` | The same source Space reference inventory. | None. This is preparatory inventory for Profile operations, not a new product Space deletion policy. |
| `DeleteProfile{profile}` | Every Space sharing the Profile, their attachments and selections, all Space-owned and standalone Profile items, aliases, live pages and split references. Includes Profile items/aliases even without any Space or live page. | None. Other Profiles' references are excluded. |

An alias into a reassigned Space appears among direct references and also in the source Profile's complete context-only alias inventory. The direct occurrence explains the target dependency. The context occurrence explains current Profile visibility. A context alias to an unrelated Space is context only. Target Profile aliases never enter the direct inventory solely because that Profile is the requested destination. Consumers must retain these distinctions and must not sum the overlapping lists as a unique entity count or imply aliases/Favorites migrate.

Every valid request returns `ExecutionGate::kBlockedPolicyOpen`, including empty Profiles, empty Spaces, same-Profile reassignment and pages reported quiescent. This gate is a refusal by this feature to authorize execution, not a newly approved product denial policy. There is no execute, confirm, transaction, recovery token or model-mutating API. The existing model's conservative empty-object commands do not authorize this feature to run them. Dropping a preview requires no rollback because it changes nothing.

Every preview reports native forms, downloads, authenticated state and Profile data outside the model as unknown. Each included live page retains its actual immutable Profile, typed saved/temporary association, attachment and `Unknown`, `Quiescent` or `Protected` report. Unknown and protected work remain visible. A quiescent report does not prove native forms/downloads safe. Empty modeled pages do not prove a Profile has no native work. The model has no inventory of actual sign-ins, site settings, cookies, native downloads or storage files.

The caller serializes access with the model's single writer. Rebuild the preview after relevant changes and before presenting current impact. An old snapshot stays independently readable but can become stale. It is not a reservation, generation token or actuation authorization. Lists use deterministic model ID order, except split page references retain their existing layout order and Profile contexts use source/target order. They do not define sidebar order. The component holds no cache, allocator, focus, navigation, persistence or parallel lifecycle store. Runtime work traverses the current in-memory graph without fetching native data.

## Focused synthetic checks

[impact_test.cc](../../native/features/profile_safety/impact_test.cc) uses public lifecycle commands and reserved `.test` destinations. It covers exact direct/context separation, aliases into and outside the source Space, source/target Favorites, shared Profiles, local temporaries in multiple windows, selected Space references, protected/unknown/quiescent pages, split membership, unreferenced Profile data, missing targets and rebuilding after transfer/navigation/selection changes. A Profile-owned saved page transferred to a sibling Space, with sibling temporary work and a split, stays outside Space previews while remaining in the Profile deletion inventory. It checks retained source identity/navigation and independently owned old snapshots. It does not simulate migration, native failure recovery or browser activity detection.

Run from the repository root with the existing C++17 compiler. The explicit driver mode follows the [lifecycle check instructions](../lifecycle-contract.md#synthetic-transition-fixtures-and-focused-checks).

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/profile_safety/impact.h \
  arcified/native/features/profile_safety/impact.cc \
  arcified/native/features/profile_safety/impact_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/profile_safety/impact.cc \
  arcified/native/features/profile_safety/impact_test.cc \
  -o /tmp/arcified-profile-safety-test
/tmp/arcified-profile-safety-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/profile_safety/impact.cc \
  arcified/native/features/profile_safety/impact_test.cc \
  -o /tmp/arcified-profile-safety-sanitized
/tmp/arcified-profile-safety-sanitized
```

On 2026-10-23, the Chromium format check and both compile/run commands above passed in the Linux T3 workspace. Compilation used `-Wall -Wextra -Werror -pedantic`; the sanitizer run emitted no diagnostics. Whitespace checks of all four new files passed. These results cover the standalone synthetic preview only, not a native browser or another platform toolchain.

## Native gaps and acceptance

The [native source references](../native-space-source-references.md#c3-live-identity-and-handoff) identify immutable WebContents/BrowserContext ownership, native before-unload/lifetime boundaries and Profile keepalives. The [session/navigation references](../native-space-source-references.md#c4-navigation-and-session-boundaries) describe Profile-local ownership, not live-state migration. These are documentary source observations. This slice adds no native adapter or source patch. Profiles owns context targeting; private owns private boundaries. No original/OTR inference or private policy is introduced here.

1. The first issue acceptance criterion is advanced only by the modeled reference preview. The owner must still approve the actual deny/detach/reopen/migrate choice. No destination creation or source disposition runs.
2. Native forms, downloads, authenticated page protection and model-to-native reference completeness remain unproven. They require an authoritative native inventory and installed interaction evidence before destructive work.
3. Cancel/interruption/failed deletion recovery remains open because there is no deletion transaction or durable identity-map recovery here. Read-only cancellation proves only that discarding this preview leaves the model intact.
4. Same actual OS-window cross-Profile selection still needs the production native host proof described in the integration design. A WebView presentation primitive cannot satisfy that gate.
5. Installed Linux, Windows and macOS behavior, platform builds, failure handling and accessibility/visual checks remain open. Passing standalone C++ fixtures cannot close issue 103.

No heavy preparation, Chromium build/download, native browsing data access, provider call, remote write, global install or shared lifecycle/tooling change is part of this slice.
