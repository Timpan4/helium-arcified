# Peek policy and presentation references

Status: local C++ component for [issue 79](https://github.com/Timpan4/helium-arcified/issues/79), 2026-10-23. Based on repository commit `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. No native implementation or issue acceptance is claimed.

[peek.h](../../native/features/peek/peek.h) and [peek.cc](../../native/features/peek/peek.cc) implement pure link eligibility and presentation-reference checks over the authoritative [lifecycle model](../../native/model/lifecycle.h). They create no page, window, navigation controller, Profile, selection or persistence state. The only presentation data is a parent `PageId` and preview `PageId`. The caller owns these references; the lifecycle/native owners retain the pages and their actual state.

## Requirements and authority

The [publication brief](../research/arc-dia/publication/peek.md) and `peek` entry in [backlog.json](../research/arc-dia/backlog.json) require automatic pinned/Favorite rules, explicit invocation, a setting, safe dismissal/promotion and correctly targeted browser surfaces. This slice covers local eligibility and reference compatibility only. The brief's Linux, Windows and macOS installed-build acceptance remains open.

The [version 3 lifecycle contract](../lifecycle-contract.md) approves temporary outbound branches from saved pages while retaining the original. `Model::PlanSavedPageLink` is the routing authority. Peek calls it and propagates every refusal. An explicit modifier cannot override missing source, temporary source, unknown/same-site classification, empty destination or redirect/script/typed/other navigation refusals. A refusal withholds this component's eligibility; it does not authorize in-place navigation or any other fallback. Same-site and non-link routing remain open.

The [native integration design](../native-space-integration.md) and [source references](../native-space-source-references.md) establish the ownership constraints. Their earlier version 2 policy descriptions are historical; approved mapping/settings/outbound decisions come from the current version 3 contract. In particular, same-window cross-Profile support requires ownership extraction and remains unproven. A WebView overlay cannot establish that support.

## Eligibility with explicit settings

`EvaluateLink` takes the model, parent ID, browser-classified navigation kind/site relation, browser-validated destination, invocation and all three settings. The component neither classifies sites from URL text nor selects a platform modifier key. The destination passes only through the existing lifecycle planner. The result retains no destination or snapshot of Profile/attachment/current navigation.

| Input setting | Eligibility within approved saved-page outbound-link scope |
| --- | --- |
| `automatic_kept_outbound` | Automatic invocation may use a saved parent whose item is Space-owned, the authoritative equivalent of a kept/pinned tab. |
| `automatic_favorite_outbound` | Automatic invocation may use a Profile-owned saved item or a saved item targeted by an existing shortcut alias in that Profile. |
| `explicit_outbound` | Explicit modifier invocation may use any eligible saved parent, independently of the automatic settings. |

For automatic invocation, enabling either applicable category suffices. Favorite classification uses existing item/alias membership, not the sidebar entry used to open the page. An alias to another item does not qualify the parent. Removing the final matching alias removes Favorite eligibility for a Space-owned item; a standalone Profile-owned item remains a Favorite. No second Favorite registry or live instance is created.

Callers supply every setting. Disabled settings are exercised as inputs, not adopted as product defaults. This is a configurable local policy surface, not a shipped preference UI or a decision about preference scope/storage. Profile-scoped Favorites remain owned by lifecycle. Explicit Peek for temporary parents, same-site links and nested previews is outside this slice. The component does not silently extend the approved navigation policy to those cases.

`Eligible` means only that these inputs permit considering Peek presentation. It is not a native creation plan, reservation, document-generation token or promise that the destination will load. The native owner must validate URL/security policy, originating document, link classification, Profile and attachment again before actuation. It must coordinate actual branch creation/cancellation with lifecycle reporting and fresh identities. Redirects require their own browser handling; eligibility must not be reused as redirect authorization.

## Presentation-reference checks

`PresentationReferences{parent, preview}` stores existing IDs only. `CheckReferences(model, references, expected_attachment)` requires distinct live pages, a saved parent, a temporary preview, equal immutable Profiles and both pages attached to the caller's expected `(WindowId, SpaceId)`. That attachment must exist in the model. It returns distinct missing-page, owner-kind, Profile and attachment failures so the caller can retain content and reconcile the actual change.

These checks do not establish that the preview was created for this parent. The caller must supply that relationship from native creation. They do not infer selected Space, focus, active pane or origin identity. A parent transfer makes the old pair incompatible; the temporary preview stays local. Keeping the preview changes its lifecycle association and makes this temporary-only presentation incompatible. Neither case causes a close, transfer, reset, replacement or layout mutation here. The checks remain independent of activity reports and never certify a page safe to destroy.

The accepted commands seam is `arcified::commands::PageCommandTarget{PageId, ProfileId, Attachment}`, resolved by `ResolvePageCommandTarget(model, explicit_focused_page, expected_attachment)` and checked again by `Revalidate(model, target)`. When native focus is in Peek, page commands must pass the explicit preview `PageId`, including back/forward, reload, find and zoom. Page-owned download, site-control, credential and permission/dialog actions require the same explicit identity. This does not classify every browser command, such as a Profile-level downloads view, as a page command. Parent commands must pass the parent ID explicitly. Peek does not include peer command headers, infer focus from Space/window selection or keep a duplicate focus record. The native owner must separately verify the actual TabModel/WebContents binding and operation generation. Graph checks cannot detect native replacement or an ABA sequence.

Before presenting either page, the native/commands owners must prove parent/overlay security and focus routing, origin display, accessibility, permission bubbles and browser-owned dialogs. `ReferenceStatus::kCompatible` acknowledges only the current model graph. It remains compatible across observed navigation when IDs/owners/attachments do not change. It cannot attest to the same document, native object generation, visible attachment or successful browser interaction.

## Dismissal, promotion and native gaps

Escape, click-outside and close are dismissal intents requiring the unresolved lifecycle/activity/destructive policy and native before-unload/permission/modal handling. This component exposes no dismiss/close/cleanup command and grants no permission to erase either page, even after a quiescent report. Hiding an overlay must not strand protected or unknown work. How dismissal retains, closes or later restores that page is still open. No timeout or cleanup schedule is introduced.

Ordinary-tab promotion must retain the existing preview `PageId`, immutable Profile and actual native navigation/form state while removing only the presentation relationship after native success. This component performs no promotion and supplies no transaction or rollback mechanism. Split promotion belongs to `peek-split-promotion`. Failure/cancellation must retain the original pages; changing references alone cannot prove promotion success.

Back/forward, nested links, forms, downloads, permissions, focus return after dismissal, stale callbacks, renderer failure, Profile shutdown and persistence/restore require native integration and policy evidence. No history, form, download or restore state is copied into Peek. No extra live saved instance, populated cross-Profile migration, account, sync or AI behavior is added. Native acceptance on Linux, Windows and macOS, including visual/accessibility/security checks, remains blocked on those owners and the window-ownership proof. Full Chromium preparation/build and remote CI were deferred by the implementation brief.

## Focused checks

[peek_test.cc](../../native/features/peek/peek_test.cc) uses synthetic IDs and reserved `.test` destinations. It drives the public lifecycle commands as fixtures, then calls the public Peek functions. Fixture close/keep/transfer commands are external lifecycle stimuli, not implemented Peek actions.

| Fixture | Local behavior checked |
| --- | --- |
| `SettingsAndExistingFavoriteAliases` | Independent automatic/explicit settings, Space-owned/standalone Profile-owned categories, relevant and unrelated aliases, alias removal, retained parent activity/navigation and no duplicate pages. |
| `RoutingScopeCannotBeOverridden` | Every lifecycle routing refusal propagates under both invocations, including enabled explicit invocation; browser classification is authoritative even with identical destination text. |
| `ReferenceBoundaries` | Compatible pair, same ID, missing attachment/pages, invalid owner kinds, cross-Profile/cross-window/other-Space references and preview keep invalidate as specified. |
| `TransferCloseAndNavigationRequireRechecks` | Graph checks preserve unknown/protected/quiescent pages and existing layout references; parent transfer leaves preview local, external close invalidates references, navigation illustrates the document-generation limitation. |

Run from the repository root with the existing host C++17 toolchain. The explicit Clang driver mode follows the lifecycle checks in this host.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/peek/peek.h \
  arcified/native/features/peek/peek.cc \
  arcified/native/features/peek/peek_test.cc
clang-tidy --checks='clang-analyzer-*' --warnings-as-errors='clang-analyzer-*' \
  arcified/native/features/peek/peek.cc \
  arcified/native/features/peek/peek_test.cc -- -std=c++17
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/peek/peek.cc \
  arcified/native/features/peek/peek_test.cc -o /tmp/arcified-peek-test
/tmp/arcified-peek-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/peek/peek.cc \
  arcified/native/features/peek/peek_test.cc -o /tmp/arcified-peek-sanitized
/tmp/arcified-peek-sanitized
```

On 2026-10-23, the formatting and explicit clang-tidy analyzer checks above exited 0 with no diagnostics. Both compiled binaries exited 0 and printed `PASS: Peek eligibility and reference fixtures`; ASan/UBSan reported no findings. The initial clang-tidy invocation without explicit checks reported `Error: no checks enabled.` The explicit invocation above ran the analyzer checks successfully. New-file whitespace checks with `git diff --no-index --check /dev/null <path>` emitted no diagnostics for all four owned files; exit 1 denotes new-file differences. Local document link targets were checked separately.

These checks exercise the standalone component and synthetic model on the local Linux host. They do not prepare/build Chromium, run Helium, execute native UI interactions, prove cross-Profile isolation or validate Windows/macOS behavior. No GN target, shared tooling, native patch, execution catalog or peer feature module is changed.
