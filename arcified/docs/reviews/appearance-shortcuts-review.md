# Shortcut preview independent reviews

Result: round 1 found one P2 in an authored native fixture. The coordinator corrected it. Round 2 reported no new actionable findings. Native acceptance remains open.

Both completed T3 review assignments were verified as `codex`, `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`, with no pending child runs.

1. Round 1: `node:delegated-task:command%3Amcp%3A61ace16c-e235-4508-9b0c-b56fadb689cd%3Adelegate-task%3Aappearance-shortcuts-independent-review-v1`.
2. Round 2: `node:delegated-task:command%3Amcp%3A61ace16c-e235-4508-9b0c-b56fadb689cd%3Adelegate-task%3Aappearance-shortcuts-independent-review-v2`.

## Finding and response

The reset fixture assumed New Tab's defaults were modifiable. The pinned Helium macOS platform fallback marks Cocoa defaults unmodifiable when no live main menu supplies them. The test established no menu and could fail before exercising reset preview.

The coordinator removed that command assumption. The fixture selects actual service-reported nonempty, modifiable defaults, excludes its destination command, and copies the command ID/defaults before mutation rebuilds the cache. The embedded patch test and blob header match the owned test. No native menu or global state is changed.

Round 2 confirmed the assumption was removed and the assertions still test reset displacement and preview immutability. It could not establish from local source fragments that every platform supplies a qualifying candidate. This remains a native execution requirement, not a demonstrated remaining defect.

Neither review found a preview implementation defect. Validation order, native equality, lock authority, macOS reservations and reset displacement match the existing service. The collector borrows authoritative state and adds no store.

## Frozen round 2 files

Paths are relative to `arcified/`. Both reviewer and coordinator matched the final identities.

| File | Git blob |
| --- | --- |
| `native/features/shortcut_settings/conflict_preview.h` | `1106ccd78074891ac6e918959d220bd596f329d1` |
| `native/features/shortcut_settings/conflict_preview_test.cc` | `ad399a41a046da412d1ce112b39b772242d9fea9` |
| `native/features/shortcut_settings/browser_shortcut_service_preview_unittest.cc` | `a672e0a11fbef88026c3e00c069a6c19b9f74918` |
| `native/features/shortcut_settings/native-conflict-preview.patch` | `58ce7ac96f27218316ce03f9d2a91037c7ebcedc` |
| `native/features/shortcut_settings/validate_patch.mjs` | `dba086a82b38a60b51daf48f4450e4bc85822a6a` |
| `docs/features/shortcut-settings.md` | `7891a27879bb05ab79d655aa491c8883db5aa1d8` |

## Checks and limits

Both rounds passed Node syntax, the pinned-source validator, Chromium formatting and strict C++17 portable collector compilation/execution. The validator checks forward/reverse patch application, embedded-source identity and unchanged existing service mutations. Round 2 also checked source/doc whitespace. Patch context contains four blank lines with the required context prefix; checking the patch as ordinary text flags them. Applying it with `--whitespace=error` passes.

The coordinator additionally passed `clang-tidy --checks='clang-analyzer-*' --warnings-as-errors='*' arcified/native/features/shortcut_settings/conflict_preview_test.cc -- -std=c++17 -Wall -Wextra -Werror -pedantic` without diagnostics. This checks the portable collector, not Chromium service/gtest compilation. Other exact commands are in [shortcut-settings.md](../features/shortcut-settings.md#checks-and-remaining-acceptance).

Native gtests remain uncompiled and GN-unregistered. The artifact remains unapplied to a prepared browser tree. Full-series application, platform service execution, UI/Mojo integration, persistence/restart, dialog/split/text-field precedence, accessibility routes, OS/webpage conflict behavior and installed Linux/Windows/macOS acceptance remain open. No browser was launched or visually verified.
