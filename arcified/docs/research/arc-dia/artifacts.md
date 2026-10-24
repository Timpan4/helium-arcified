# Shipped artifact evidence

Acquired on 2026-10-23 from the publishers' release domains. These files remain local investigation inputs. No proprietary source, executable or asset is included in the product or this research directory.

| Artifact | Download | Observed size | SHA-256 |
| --- | --- | --- | --- |
| Arc 1.170.0, build 89123 macOS DMG | `https://releases.arc.net/release/Arc-latest.dmg`, redirected to `Arc-1.170.0-89123.dmg` | 458434157 bytes | `5cf6d057e8944f847dac83d89b4d30d7a49c8974d02d29b35d2cf5afce9e36c1` |
| Dia 1.54.2, build 89247 macOS DMG | `https://releases.diabrowser.com/release/Dia-latest.dmg` | 861800582 bytes | `7f0cb70555190ee1d36563b09a4ed5d8146e2eeaa4b3430bcb38bbce7e6ad913` |

The version/build labels come from the shipped disk image metadata reported by 7-Zip. The Dia changelog's latest displayed version was 1.54.0; the actual downloaded image says 1.54.2. Do not substitute the changelog label for the observed artifact identity.

## Versioned Dia route

A later HEAD request to `https://releases.diabrowser.com/release/Dia-1.54.2-89247.dmg` returned HTTP 200 and Content-Length 861800582. This verifies the versioned route and reported size only. The SHA-256 above comes from the acquired latest-route download, not a second full download of that versioned URL.

## REA observations

REA 6.7.0 inspected both DMGs. Its Linux DMG route establishes root identity only. Installed 7-Zip 26.05 extracted the application bundles without executing them, after which REA inspected each complete filesystem-backed bundle inventory.

| Inspection | Retained evidence | Coverage |
| --- | --- | --- |
| Arc DMG summary | `ev_3300ed8b56418e9d039d138c74fab56076f7eb3ae63bd1031bba3df82355f437` | Root hash only; no native macOS mount |
| Dia DMG summary | `ev_4dbb2aa3a1f3259080ec420fe62b98dd6ae2face2ddb9de74fca0f95e9bdc7e0` | Root hash only; no native macOS mount |
| Arc bundle inspection | `ev_dcabd9b70e94bbfdb90a30177e627ee31aa05b603e050aaff9b5933b6661a394` | 1483 content nodes, 1955 occurrences, 1103 regular files, no inventory integrity contradictions |
| Arc bundle extraction | `ev_2fe3566c0850e35eb71ac6cc83885f659be418b534ce2cd7208ea398b14ee2f0` | Regular files materialized, containment verified; nested archives remain separate |
| Dia bundle inspection | `ev_60e9f8ba91705be3543db67e4159ce4b087128b8410cb6a492dca60e83026e2e` | 2652 content nodes, 3144 occurrences, 2292 regular files, no inventory integrity contradictions |

Arc's principal executable is universal x86_64/arm64. Its original hash is `25af5f4eaaae5aca25021f2737e90cc4d41fdf195c4a5c4012ea31a1bba4b86a`. LLVM lipo produced a separate arm64 analysis slice with hash `77be289e16a139056dd9d0bf145ad4707d4e217289bb279a333949dcfd39c52d`. Dia's principal executable is arm64, hash `2ab81313bd192e4c0e6e27f5aafffe1c275cd2dbb93b49bf320d8d7d9e94bdd7`.

Both inventories contain an ArcCore framework and native browser helper applications. This is a packaged-component observation, not evidence that their current implementations, behavior, or performance are equivalent. Arc also ships an `Arc.sdef` scripting dictionary and native resource/asset bundles. Presence establishes a research lead, not execution of an automation or UI feature.

## Shipped menu resource observations

REA decoded the bundles' compiled Interface Builder resources without executing them. Arc's retained graph is `ev_cfff947c28d386fd3ea781438da8f9bf06f232c17b7f9431381cf4bf35d6306f`. Six documents include the product MainMenu resource, whose archive hash is `c05560f6665c062b78dd326c4ce56307154010f981a45f4959671fb460ab1963`. That document contains 678 recognized archive objects and 358 recognized connections. All 1591 nodes in the retained graph were paged; filtering the product menu gives 151 distinct target-action selector names, including standard native text/window actions.

The product action families corroborate leads for Space/Profile switching, pin/reset/reset-visible and section collapse, folder expansion, command input, ordered/recent tab traversal, Little Arc, split pane creation/maximization/separation, capture variants, URL/Markdown/quote copying, Library/archive/downloads, Boost/Easel/Notes actions, meeting join, DevTools/task manager/tracing, import/recovery and updates. These are serialized action declarations. They do not prove the associated implementation exists, is reachable, is enabled or works. Legacy Boost and Notes actions remain present despite documented retirement, illustrating that distinction.

Dia's retained Interface Builder graph is `ev_44bd1b36489984ff70c53536c98091ed1a324bed4893267cb8ccd129c299127d`. Its five decoded documents all belong to the Sparkle updater; this route did not expose product UI. That does not establish that Dia lacks a native menu or any product feature.

Both graphs are explicitly partial at the decoder level because private/unrecognized objects and some ownerless connectors remain unresolved. Exhausting the retained pages does not remove that limitation. Pixel geometry, visibility, constraints at runtime and dynamically constructed UI were not inferred. No proprietary implementation or menu resource is copied into Arcified.

## Tool limits and unresolved runtime work

REA's Ghidra provider refused the universal Arc executable because it cannot enforce slice selection. The separate arm64 slice imported, but default analysis did not finish within REA's configured 330000 ms startup deadline. That value is the provider's existing setting, not an investigation-imposed limit. No partial analysis was accepted and no pseudocode was recovered. REA's GNU objdump adapter also reported that it could not recognize these Mach-O files. Installed LLVM tools provide a format-aware fallback for header inspection without changing persistent tool configuration.

The host is Linux. Native macOS UI observation, Apple asset catalog decoding, signing verification and macOS plist utility routes are unavailable. No Windows application was executed. No app login was attempted, no real user browsing data was read, and no page content was sent to a model. The application bundles were not run. Inventory completeness describes the inventory, not feature or runtime completeness.

Runtime interaction details, performance distributions, source-schema compatibility and unpublished service behavior remain explicit follow-up research. The backlog must retain them as verification work rather than claim static evidence proves the UX.

## Local retention

Original downloads, extracted bundles and portable REA evidence exports are under `/tmp/arc-dia-ux.rZ9n65/`. The Arc regular-file extraction is under `/tmp/rea-extracted-f07865ad-474c-4e2f-a255-f2307819f2ed/`. These temporary paths are local and can expire. Durable artifact identities and evidence IDs are recorded above so the inspection can be repeated from the official releases. Public issues must not depend on another developer having these local paths.
