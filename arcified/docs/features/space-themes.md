# Space theme palettes and preview

Status: reusable standalone C++ component for [issue 52](https://github.com/Timpan4/helium-arcified/issues/52). Native acceptance remains open. This slice implements palette resolution and an editor-local preview, not an installed native theme editor or browser adapter.

The [header](../../native/features/space_themes/space_themes.h), [implementation](../../native/features/space_themes/space_themes.cc), and [behavior fixtures](../../native/features/space_themes/space_themes_test.cc) consume `lifecycle::SpaceId` from [contract v3](../lifecycle-contract.md). Each Space chooses one Profile; several Spaces may share that Profile while retaining separate themes. The lifecycle model remains the identity authority. This component has no Space registry, Profile lookup, durable store, name/icon record, selected Space, focus state, or navigation state.

## Interface and caller transaction

`Theme` contains the caller's requested light surface, dark surface, accent, and system/light/dark preference. `Rgb` is opaque eight-bit sRGB. These C++ values are not a serialization schema. The caller validates external representations before constructing them and confirms that every supplied Space identity is live and authorized.

`ResolvePalette(theme, environment)` returns surface, foreground, selected surface, selected foreground, and separator roles. The caller supplies the current OS color scheme on each call. Explicit light/dark preferences override that scheme. System preference resolves anew, including while previewing. There is no observer, OS service, cached system mode, or native repaint implementation here.

1. After validating the Space in the authoritative lifecycle owner, create a caller-owned `Preview(space_id)` for its editor. `Set(theme)` replaces only that editor's draft.
2. For presentation, call `Resolve(live_space, current_theme, environment)`. `current_theme` must belong to `live_space`. Only an exact `SpaceId` match uses the draft; a different Space uses its caller-supplied theme, even within the same Profile. A caller may retain an editor across a Space switch without letting its draft color another Space.
3. Read `proposal()` to obtain an owned `ThemeChange` containing the original Space identity and requested colors. Reading or modifying this copy does not consume the draft or acknowledge a save. The caller revalidates live identity, authorization and concurrent changes before attempting its own durable transaction.
4. Keep the draft when saving fails. After the caller has confirmed a successful save and updated its authoritative theme, clear the draft with `Revert()` or destroy the editor. There is no save callback or persistence claim in this component. The caller must serialize pending saves with subsequent edits and cancellation so an old save completion cannot clear or overwrite a newer edit.
5. Cancel with `Revert()`. It discards the draft, leaving subsequent resolution to the caller's current theme and current environment. It never writes back an opening snapshot, so an external theme update or OS mode change is retained. Repeated revert is harmless.

The caller owns the editor's lifetime and whether other presentations show its preview. Two editors are independent; this component supplies no conflict policy or cross-window preview broadcast. Deleting a Space requires its owner to retire its editors. IDs must not be reused as the lifecycle contract requires. Neither matching IDs nor `proposal()` establish that a Space still exists.

## Normal contrast

The normal path preserves the chosen surface. It selects whichever of black and white has greater contrast for its foreground and separator. It retains an accent with at least 3:1 contrast against the surface; otherwise the selected surface uses that contrasting endpoint. Selected text gets its own black/white foreground. The requested accent in `Theme` and in the proposal remains unchanged, allowing a preview to show the effective colors without silently rewriting the user's input.

The thresholds come from [WCAG 2.2 1.4.3](https://www.w3.org/TR/WCAG22/#contrast-minimum), 4.5:1 for ordinary text, and [1.4.11](https://www.w3.org/TR/WCAG22/#non-text-contrast), 3:1 for non-text controls/states. Pinned Chromium [ui/gfx/color_utils.h](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/gfx/color_utils.h) exposes the same `kMinimumReadableContrastRatio` and `kMinimumVisibleContrastRatio` contracts. Its `GetContrastRatio` and `GetColorWithMaxContrast` are native integration candidates, not dependencies linked into this standalone component.

The implementation follows WCAG's [relative luminance](https://www.w3.org/TR/WCAG22/#dfn-relative-luminance) and [contrast ratio](https://www.w3.org/TR/WCAG22/#dfn-contrast-ratio) definitions:

- Normalize each encoded sRGB channel by 255. For a normalized channel `c <= 0.04045`, use `c / 12.92`; otherwise use `((c + 0.055) / 1.055)^2.4`.
- Relative luminance is `0.2126 R + 0.7152 G + 0.0722 B` using those linear channels.
- Contrast is `(max(L1, L2) + 0.05) / (min(L1, L2) + 0.05)`, without rounding before comparison.

For an opaque sRGB surface with luminance `L`, black contrast is `(L + 0.05) / 0.05` and white contrast is `1.05 / (L + 0.05)`. Their product is 21. The greater ratio is therefore at least `sqrt(21)`, which exceeds 4.5. This establishes the endpoint fallback's text contrast for any valid input color; the fixtures exercise representative chromatic colors and every encoded gray. This is a mathematical palette property, not native accessibility or screenshot evidence.

## Enforced native palettes

`Environment::enforced_palette` supplies the caller's authoritative colors when forced contrast, high contrast, or managed/native policy requires them. Its presence takes priority over custom preferences and drafts. Every role passes through unchanged. The normal algorithm does not audit, repair, recolor or certify these colors. A fixture deliberately supplies low-contrast native roles to prove that trust boundary. The caller must obtain suitable opaque colors and preserve platform accessibility behavior.

Pinned Chromium [ui/color/color_provider_key.h](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/ui/color/color_provider_key.h) distinguishes `ColorMode`, `ContrastMode`, and `ForcedColors`. `kSystem` is only one non-default forced-color mode. An adapter must account for the actual host's forced/high-contrast and managed-theme conditions rather than infer them from dark mode or theme luminance. No such adapter or native role mapping is implemented here.

## Native adoption boundaries

Source inspection used Chromium `155.0.8059.79`, pinned to `a49e2e3d4addfd84e765b52afe230bedb81096db`, and the local Helium patches at checkpoint `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. The Chromium headers cited here were read through Gitiles `?format=TEXT`; no source preparation, patch application or Chromium build ran.

| Inspected source | Consequence for this slice |
| --- | --- |
| [ThemeService header](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/themes/theme_service.h) | Theme setters and reinstaller belong to a Profile service. Do not use them as a per-Space preview/save mechanism, since multiple Spaces can share a Profile. The header directs rooted UI to its actual Widget/AppControl ColorProvider and warns that Profile-derived keys can cause consistency and contrast problems. |
| [Helium default-theme patch](../../../patches/helium/ui/default-theme.patch) | Initializes/resets grayscale through Profile preferences. It does not provide a Space theme owner or schema. This component writes none of those preferences. |
| [Helium native-frame-materials patch](../../../patches/helium/ui/native-frame-materials.patch) | The inspected availability and `ShouldUseNativeFrameMaterials` paths are macOS-specific and read a Profile preference. They do not prove a Space-scoped cross-platform material mechanism. |
| [Native host design and evidence](../native-space-integration.md), [source references](../native-space-source-references.md) | Production extraction of the same OS window across real Profiles remains unproven. A palette resolver does not establish that host or repair Profile-bound theme service lifetimes. The earlier design's v2 statements do not override lifecycle v3's approved theme scope. |

The native renderer may consume these palette roles for its opaque Space chrome. It owns painting, invalidation, clipping and native role mapping. The separator contrasts with the Space surface only; it cannot certify a boundary against an arbitrary webpage, texture, translucent material or image. The host must retain its native web-content separation and verify the composed result.

The component owns no caption classification, window-control rendering, opacity, native materials, display metrics, geometry, visibility, reveal, resize, keyboard routing, focus or announcements. Layout supplies post-layout geometry and native metrics. Native caption eligibility/exclusions and the actual window controls retain their platform authority. Accessibility and the native host retain focus and announcements. Generic foreground colors alone do not prove caption-control hover, pressed, disabled or high-contrast behavior.

Opaque colors provide the reusable fallback input on Linux, Windows and macOS. Applying a platform material, selecting its opacity, or compositing it behind these roles would require host implementation and visual verification before relying on this component's contrast guarantee. No material is enabled or required by this slice.

Intensity/grain controls and custom folder icons are absent. This slice uses flat opaque surface/accent colors as its simpler presentation primitive, with no texture assets. A product replacement for those controls and original folder icon assets remains open. Name/icon drafts also remain with their eventual metadata editor; no authoritative native metadata owner was available here, so there is no duplicate name/icon store or persistence contract.

## Focused checks

Run from the repository root:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/space_themes/space_themes.h \
  arcified/native/features/space_themes/space_themes.cc \
  arcified/native/features/space_themes/space_themes_test.cc

clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/space_themes/space_themes.cc \
  arcified/native/features/space_themes/space_themes_test.cc \
  -o /tmp/arcified-space-themes-b0793bf5-test && \
  /tmp/arcified-space-themes-b0793bf5-test

clang-tidy arcified/native/features/space_themes/space_themes.cc \
  arcified/native/features/space_themes/space_themes_test.cc \
  --checks='clang-analyzer-*' --warnings-as-errors='*' -- -std=c++17

clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/space_themes/space_themes.cc \
  arcified/native/features/space_themes/space_themes_test.cc \
  -o /tmp/arcified-space-themes-b0793bf5-sanitized-test && \
  /tmp/arcified-space-themes-b0793bf5-sanitized-test
```

These passed with local Clang/clang-format 23.1.3, including AddressSanitizer and UndefinedBehaviorSanitizer execution. The initial clang-tidy invocation without `--checks` reported `no checks enabled`; explicit analyzer checks then ran successfully. The fixtures cover known contrast relationships, the sRGB foreground crossover, an unrounded accent-threshold boundary, readable text and selected states, accent preservation/fallback, every encoded gray, system and explicit schemes, OS changes during preview, enforced-color priority and exact forwarding, Space isolation, owned proposals, repeated/replaced drafts, cancellation against changed caller settings, and separate editors for Spaces sharing and not sharing a Profile. They link the existing lifecycle model to obtain synthetic valid Space identities. They do not run a native browser.

## Acceptance still open

| Issue criterion | Evidence from this slice | Remaining proof |
| --- | --- | --- |
| Edit colors/name/icon and light/dark with preview/revert | Executable color draft, light/dark resolution and revert component | Native editor interaction, name/icon authority and editing, cancellation/save failures in the actual host |
| Legible text, selection, controls and content boundaries in custom/forced contrast | Normal-contrast palette properties and enforced-color forwarding fixtures | Native palette mapping, window controls, focus/accessibility, composed content boundaries and forced-contrast visuals |
| Restart persistence and OS dark-mode changes | Fresh supplied OS mode affects resolution without changing the proposal | Native OS notifications/repaint, durable settings/schema adoption, restart and crash/failure evidence |
| Native materials with opaque fallback | Opaque palette input independent of platform | Native renderer fallback, available materials and composed legibility on each platform |
| Intensity/grain and original folder icons or deliberate replacement | Flat colors documented as this component's primitive; no assets copied | Product replacement decision and native folder icon implementation/evidence |

Installed native behavior and visual checks on Linux, Windows and macOS remain open. No native browser was built, launched or used for this slice. No real browsing data, provider access, remote write, shared lifecycle/tooling edit, patch-series edit, staging or commit is part of this work.
