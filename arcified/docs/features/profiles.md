# Profile preparation boundary

Status: source-backed preparation component for [issue 48](https://github.com/Timpan4/helium-arcified/issues/48). This is standalone C++ over lifecycle contract version 3, with synthetic fixtures. Installed native acceptance remains open on Linux, Windows and macOS.

[preparation.h](../../native/features/profiles/preparation.h) resolves existing model identities and validates explicit native bindings. It owns no Profile, Space, page, attachment, selection, navigation, persistence or native registry. It consumes `const lifecycle::Model` and the shared [private context facts](../../native/features/private_browsing/context_boundary.h). Private browsing owns classification and operation eligibility; profile safety owns impact preview and future destructive policy. Adjacent workspace and durability components retain their existing responsibilities.

## Public operations

| Operation | Input and result |
| --- | --- |
| `PrepareRegularProfile` | Registered `ProfileId`, caller's `ProfileBinding` and actual native facts. Returns a regular Profile service target even when the Profile has no Space or its Spaces have no attachment. |
| `PrepareSpace` | Existing `(WindowId, SpaceId)` attachment, binding and actual target context facts. Returns owned attachment, Profile ID, exact native context ID and shared classification. Does not require or change window selection. |
| `PreparePage` | Existing `PageId`, binding and actual page context facts. Resolves the page's current attachment and immutable Profile. Supports saved and local temporary pages without creating, transferring or selecting either. |
| `PrepareRegularProfileServices` | Resolves an attached Space, then reuses `PrepareRegularProfile`. Returns an exact regular Profile lookup target, without accessing a service. |

`ProfileBinding` is `{ lifecycle::ProfileId profile; private_browsing::NativeContextId context; }`. `RegularProfileServiceResult` is `std::variant<PreparationRefusal, RegularProfileServiceTarget>`, whose successful value contains those same typed IDs. Other results carry `SpacePreparation` or `PagePreparation`. The header names individual missing-model, attachment, mapping, unknown-context, unsupported-context and exact-identity failures.

Regular preparation checks membership, binding Profile equality, exact native identity and `facts.original_profile == profile`. Classification comes only from `ClassifyContext`. Missing or empty native context identity and incomplete regular mapping fail closed. Guest, system and other unsupported contexts receive no preparation target. An exact supported OTR identity can receive an identity snapshot from `PrepareSpace` or `PreparePage`, including when original-Profile mapping is unavailable. It never receives a regular service target. Sibling OTR contexts with the same original Profile remain distinct.

An OTR `ContextTarget` is an identity snapshot only. It cannot authorize ordinary lifecycle persistence, regular Space attachment, export, capture, history, AI or any other private operation. Consumers must separately apply the private boundary. An eligible regular snapshot likewise does not authorize remote activity, prove the complete workspace graph is safe to persist, or replace graph/version validation.

The returned values own their IDs and become stale when the model or native objects change. They are neither reservations nor commit tokens. Recompute them at the point of use under the caller's serialization/lifetime discipline. A previously successful result remains an ordinary value after a native context dies; this component cannot detect that event.

## Native caller contract

1. Allocate non-reused live `NativeContextId` values for exact real native Profile objects. Preserve actual OTR identity. Never serialize these IDs or derive them from a profile path, display name, `GetOriginalProfile()`, `IsSameOrParent()`, an OTR ID alone, native pointer text or a session ID. This component implements no allocator or registry.
2. Supply an authoritative association between the requested model Profile and that exact native identity. Maintain consistency across calls and distinguish different model browsing identities. The component checks the supplied pair; it cannot independently prove the caller associated the right native object with the model ID or detect bindings omitted from this call.
3. Capture facts from live native objects on the browser UI sequence. For pages, use `Profile::FromBrowserContext(web_contents->GetBrowserContext())`. For preparation of a browsing context, use that context's actual Profile. Classify using actual `IsOffTheRecord()`, `IsGuestSession()`, `IsRegularProfile()` and system/other native distinctions under the shared facts contract. Supply a regular Profile's model mapping as `original_profile`. Original-Profile metadata never establishes exact OTR equality.
4. Retain native lifetime and invalidate native associations on destruction or replacement. Keep actual TabModel, WebContents, navigation controller and BrowserContext ownership authoritative. No facts enum or boolean demonstrates isolation by itself. In particular, page facts alone do not prove a destination container, command controller or service matches. Validate each actual target against the authoritative binding.
5. Apply private operation eligibility and native security/access rules before acting. Revalidate model identity, actual native identity and current ownership after any asynchronous gap. Native creation, selection, transfer, focus, cancellation and event reconciliation require the separate native integration path. Report actual success through the lifecycle owner's existing APIs only after that path succeeds.

For same-Profile Spaces, `target.profile` and `target.context` remain the same, while `attachment.space` identifies the independent Space theme scope. Favorites, sign-ins and site settings remain Profile-scoped. The component stores none of these payloads. For a Profile-only durability capture, resolve every registered Profile, including unreferenced ones, and validate the complete graph separately. This resolver is an eligibility input, not a persistence implementation.

## Source-backed service targets

The sources below were read at Chromium commit `a49e2e3d4addfd84e765b52afe230bedb81096db`, the repository's documented `155.0.8059.79` pin. Read-only Gitiles requests decoded individual source files. No checkout, download archive, preparation or Chromium build ran. A patch-header search found no local patch to these exact header/factory files. This is source inspection, not proof of a prepared tree or service behavior.

| Source | Consequence for the caller |
| --- | --- |
| [profile.h](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/profiles/profile.h) | A Profile is a BrowserContext. `FromBrowserContext` resolves the real Profile; `GetPrefs` belongs to it. Multiple OTR objects can share one original Profile. Guest and system also have distinct OTR cases. Original equality is insufficient. |
| [profile_selections.h](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/profiles/profile_selections.h) | Keyed-service policies distinguish own-instance, original-only, redirected and OTR-only lookup. Exact page identity and effective service ownership are separate questions. Do not implement a universal original-Profile redirect. |
| [host_content_settings_map_factory.cc](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/content_settings/host_content_settings_map_factory.cc) | `GetForProfile` takes the actual BrowserContext on the UI thread. Regular and OTR use own instances; construction may consult the original's map and extension settings. Pass the exact context to the native factory. |
| [bookmark_model_factory.cc](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/bookmarks/bookmark_model_factory.cc) | `GetForBrowserContext` performs native factory selection. Regular OTR lookup redirects to the original. This component returns only regular lookup targets and does not invent a private bookmark permission or map lifecycle Favorites into another bookmark store. |
| [history_service_factory.cc](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/history/history_service_factory.cc) | `GetForProfile` applies history preferences and `ServiceAccessType`; the factory redirects regular OTR lookup to the original. Preserve native access rules and null results. No history read/write or explicit-access bypass is authorized here. |

`RegularProfileServiceTarget.context` identifies the exact regular Profile to supply to those native APIs. It is not a service pointer, does not guarantee service availability, and does not prescribe creating services during capture. A native caller still chooses the appropriate existing/non-creating API and access type. Cookies, credentials, extension state, storage partitions and session restoration require their native owners and are not serviced or proven by this component.

The [native Space integration design](../native-space-integration.md) and [source references C1 through C4](../native-space-source-references.md) establish why a legacy Browser, BrowserView or WebView adapter cannot prove same-window multi-Profile ownership. That design describes contract version 2 and contains now-stale approval wording. The current [version 3 lifecycle contract](../lifecycle-contract.md) and task decisions govern one Profile per Space, shared Profile scopes and outbound saved-link branching. No shared design artifact is edited by this slice.

## Focused synthetic verification

[preparation_test.cc](../../native/features/profiles/preparation_test.cc) uses synthetic IDs and reserved `.test` destinations. It checks shared Profile targets with independent Space identity, explicit cross-Profile preparation without changing selection, regular-only service eligibility, unreferenced Profiles and unattached Spaces, exact sibling OTR rejection, guest/system/other refusal, missing facts and attachments, protected saved-page owner resolution, retained stale snapshots after an independently simulated saved-page transfer, and local temporary ownership. The private facts header is the only private-boundary dependency. There is no policy linkage or fake native implementation.

Run from the repository root using the existing local toolchain:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/profiles/preparation.h \
  arcified/native/features/profiles/preparation.cc \
  arcified/native/features/profiles/preparation_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/profiles/preparation.cc \
  arcified/native/features/profiles/preparation_test.cc \
  -o /tmp/arcified-profiles-preparation-test
/tmp/arcified-profiles-preparation-test
```

Clang's explicit driver mode follows the existing standalone lifecycle check. A sanitizer run uses the same inputs with `-g -fsanitize=address,undefined -fno-omit-frame-pointer` and a separate output path. On 2026-10-23, the formatter check, strict C++17 fixtures and ASan/UBSan fixtures passed. Git whitespace checks and local documentation link-target checks also passed. These checks prove only standalone preparation behavior, not native integration.

## Open acceptance and native gaps

Issue 48 remains open. Same actual OS-window cross-Profile presentation has no proven production host. No native binding producer, registry, keepalive, page replacement observer, command/service dispatcher or asynchronous reconciliation is implemented here. No cookie/login/extension/permission/history isolation test, visual interaction, restart, multiwindow restore, failure recovery or accessibility acceptance ran in an installed native build on any target platform.

Profile reassignment, deletion, activity, cleanup, destruction and populated migration policy remain OPEN. This slice executes none of them and does not settle them through preparation success. Shared saved transfer, local temporaries and outbound saved-link branching retain their approved lifecycle semantics; preparation adds no navigation or transfer policy. No runtime integration, build-tooling, patch-series, workflow or shared-model changes are included.
