# Capture preparation and preview

Status: reusable standalone C++ component for [capture #37](https://github.com/Timpan4/helium-arcified/issues/37), 2026-10-23. Native capture, preview UI, clipboard, file dialogs and Library integration are unavailable. No installed browser behavior is claimed.

[capture.h](../../native/features/capture/capture.h) and [capture.cc](../../native/features/capture/capture.cc) implement checked raster planning, actual RGBA region cropping and transient capture/preview/export transitions. They reuse [lifecycle v3](../lifecycle-contract.md) identities and const model references. They neither mutate page ownership/navigation nor introduce a current-page registry, file store, Library record or host facade.

## Target and policy boundary

The native caller supplies a specific `PageId`, immutable `ProfileId`, attachment, optional `SplitId`, committed-document token and `visual_revision`. Preparation checks the live model page, attachment, selected Space and split membership. Native selection within the Space/split remains a host fact. Tokens must be opaque and nonempty. A new document gets a new document token; scrolling, layout, zoom, surface/WebContents replacement or a changed picker mapping gets a new visual revision, even when the dimensions stay the same. Never substitute URL/title matching or global current-page lookup.

`HostFacts` must come from the actual target's native owner and security context. The caller attests that the target is selected/visible, the provider isolates its content, capture policy allows the operation, and the result excludes hidden/private/sensitive content. Unknown permission and missing content attestation fail closed. This is a conservative supported subset, not a product decision about exporting private sessions. The host still owns Profile/OTR semantics, permission evaluation, enterprise/content restrictions, native object lifetime and fresh-frame evidence. No pixels may be acquired before successful preparation and policy attestation. A returned plan is preparation only, not proof that a native capture occurred.

Provider capability defaults to `kUnavailable`. The fixtures explicitly opt into synthetic visible-pane/full-page capability. An embedding must not mark a capability available until its concrete native provider is implemented and checked. Unknown `Scope`, `Provider`, `ExportAction` and native result values are refused at the public boundary.

## Scale and region seam

Visible-pane capture uses the host's viewport CSS extent. Full-page capture uses the host's complete scrollable document CSS extent. Both require an explicit, finite positive `pixels_per_css_pixel`; each raster dimension is `ceil(css_extent * scale)`. There is no implicit device-scale/page-zoom assumption, automatic downscale or viewport fallback. The provider must honor that scale and return exactly the planned raster dimensions. Full-page requests against a visible-only provider return `kFullPageUnavailable`.

Dimension and byte arithmetic checks reject nonfinite, zero, overflowing or unrepresentable images. Bounds come from `uint32_t` pixel representation, `size_t` arithmetic and the standard vector's `max_size()`, not a product image quota. RGBA8 has four channels. There is no imposed long-page cap. Native renderer/backend limits, memory exhaustion, lazy content, fixed/sticky elements, nested scrolling and full-page acquisition/stitching remain unimplemented. A native provider must report failure when it cannot fulfill the plan. This slice does not promise to render a long page merely because its geometry is representable.

The region picker owns native-view DIP geometry and inward rounding. View DIPs are not CSS pixels. The host translates its confirmed selection to capture-local `VisibleRegion`, preserving the complete target/document/visual revision, exact visible raster dimensions and half-open integer crop. The host must validate signed edges before converting them to unsigned coordinates. No shared region header or shared geometry owner is introduced. A region is accepted only for visible-pane scope, matching target and raster dimensions, and a nonempty crop wholly inside that raster. Changed scale, stale target and overflowing/empty rectangles are refused.

`CompleteCapture` first validates fresh target/policy and the exact full RGBA8 raster, then copies only the bounded crop into the preview. The crop is never passed to a native capture API as a CSS/DIP rectangle. [The composed fixture](../../native/features/capture/capture_region_test.cc) exercises the real picker and capture component with different CSS, DIP and raster scales, preserving exact selected pixels. Its conversion is a synthetic host example, not a production adapter.

## Async preview and export

1. Call `Session::Begin` with fresh facts. Only its successful ticket permits the host to start the concrete native capture. Keep the session on its UI sequence. Marshal callbacks to it, and associate tokens with that exact session instance.
2. Call `CompleteCapture` with the ticket token, fresh facts and either actual pixels or native failure/cancellation. A cancelled/replaced operation's token cannot publish pixels. Wrong dimensions/byte lengths, changed source, revoked capture permission and stale geometry cannot create a preview. `Revalidate` also invalidates pending capture immediately on host change events.
3. Before showing an existing preview and on every source/policy event, call `Revalidate`. Source invalidation drops the session's preview. A new capture requires new preparation. Call `BeginExport` with copy/save and fresh facts; it additionally checks the corresponding host permission. Its ticket carries immutable preview bytes for that native operation only.
4. Native filename selection, codec/format choice, clipboard/file writing and recoverable OS errors remain the caller's responsibility. Revalidate again after a file dialog and immediately before the actual native write. During export, revalidation also checks that action's permission. On invalidation, stop using the ticket, release external image references and request native cancellation. Local invalidation cannot revoke bytes already held by a caller or undo a completed external write.
5. Report the actual native result through `CompleteExport`. Failure/cancellation retains a still-valid preview for retry. During pending native export, local `Cancel` returns busy until the native outcome is reported; it cannot pretend that a dialog/write was cancelled. An invalidated export preserves only its pending result tracking, then returns to empty state. Success means caller-reported clipboard/file success only. It does not mean Library insertion succeeded.

Tokens increase within the noncopyable session and fail at the integer representation boundary rather than wrap. They are not a persistent identity, timeout, retry limit or native object handle. Sessions do not coordinate focus, transfers, activity protection, cleanup or destructive actions. Those lifecycle policies remain open.

## Pinned native evidence

The ownership constraints remain those in [native Space integration](../native-space-integration.md) and [source references](../native-space-source-references.md), pinned to Chromium `155.0.8059.79`, commit `a49e2e3d4addfd84e765b52afe230bedb81096db`. Neither a new native host nor same-window multi-Profile feasibility is established here.

Direct read-only Gitiles HTTP inspection on 2026-10-23 confirmed these adjacent capture contracts. No Chromium tree was prepared or built.

- [web_contents_capture_client.cc, lines 69 to 90](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/extensions/browser/api/web_contents_capture_client.cc#69) scales an optional source rectangle with `ScaleToEnclosingRect`. The source warns that overflow can produce an empty rectangle and then capture the whole visible surface. It does not provide full-page acquisition or make an invalid region safe. This component instead validates full returned pixels before bounded cropping.
- [render_widget_host_view.h, lines 223 to 248](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/content/public/browser/render_widget_host_view.h#223) distinguishes source view coordinates, screen bounds, output bitmap dimensions and device scaling. It also permits callbacks on another sequence and warns of old/failed output for occluded renderers. Matching dimensions alone is not fresh-frame proof. A future adapter must observe source/surface freshness and report errors, rather than accept any returned bitmap.

## Focused checks and open acceptance

Run from the repository root using the existing C++17 toolchain. No downloads, native browser launches or services are required.

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/capture/capture.h \
  arcified/native/features/capture/capture.cc \
  arcified/native/features/capture/capture_test.cc \
  arcified/native/features/capture/capture_region_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/capture/capture.cc \
  arcified/native/features/capture/capture_test.cc \
  -o /tmp/arcified-capture-sanitized
/tmp/arcified-capture-sanitized
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/region_capture/region_capture.cc \
  arcified/native/features/capture/capture.cc \
  arcified/native/features/capture/capture_region_test.cc \
  -o /tmp/arcified-capture-region-sanitized
/tmp/arcified-capture-region-sanitized
```

The capture fixtures cover explicit scale, full-page geometry and synthetic long raster preview, unavailable providers, lifecycle/split targeting, unknown/denied policy, region bounds and exact pixels, stale/cancelled callbacks, malformed images, source invalidation, export retries, export-policy revocation and unknown boundary enums. The composed fixture covers the actual picker-to-capture conversion, exact cropped RGBA bytes, stale picker/source and changed scale. These are standalone synthetic checks, not native browser/OS tests.

| Acceptance from the publication/backlog | Local evidence | Still open |
| --- | --- | --- |
| Selected page/split pane at intended scale without hidden/private content | Identity/policy/scale validation, exact raster validation and bounded crop fixtures | Concrete native selection/provider, content isolation, actual pixels, visual/accessibility interaction |
| Cancel, copy, save and filename selection through native flows with recoverable errors | Local cancellation, stale-token refusal and reported native outcome/retry fixtures | Preview UI, clipboard, native dialogs, filename handling, encoding and OS failure evidence |
| Library references and permission/sensitive restrictions | Fail-closed host policy and export-action checks | Library dependency #19, native policy provenance and Library reference handoff. No Library store or success is simulated |
| Full scrollable page at documented scale and honest limitations | Explicit scale, long raster preparation/preview, visible-only refusal | Native full-page rendering/stitching, backend limits and documented runtime behavior |

Installed native behavior and visual evidence on Linux, Windows and macOS all remain **open**. Full Chromium preparation/build, downloads/Library integration, heavy downloads, benchmarks and remote CI remain deferred. Real browsing data and AI providers were not accessed.
