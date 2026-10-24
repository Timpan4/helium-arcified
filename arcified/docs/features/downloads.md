# Download row presentation

Local reusable presentation logic for [issue 36](https://github.com/Timpan4/helium-arcified/issues/36), recorded 2026-10-23. This is an unconnected C++ component with synthetic checks. It does not implement a native downloads surface or satisfy the issue's native acceptance criteria.

## Implemented slice

[`DeriveRowPresentation`](../../native/features/downloads/row_presentation.h) projects current native row signals into main-button routing and a hidden, indeterminate or determinate progress bar. It preserves a disabled native button, routes warning activation to the native security subpage, preserves pause presentation and gives native looping progress precedence over a reported percentage. Unknown or invalid percentages stay indeterminate instead of being clamped to apparent completion.

The function has no download queue, identities, observers, persistence, lifecycle transitions, filesystem access or command execution. Inputs and outputs are transient render values. There is no completed/safe verdict, retry decision, byte-based progress estimate or automatic action. In particular, 100 percent does not enable the main button or replace a security route. `kNativeOpenOrSchedule` names Chromium's existing row route, whose `OpenDownload()` can schedule opening a still-active download. It is not a promise of completion, file presence, safety or successful opening.

No shared interface changes are required. Library should continue treating downloads as the existing native provider. This helper is an optional local rendering utility, not a Library provider or a new service contract. Nothing is wired into Chromium, GN, quilt patches, the extension or the lifecycle model.

## Pinned source identity

Work started at fork baseline `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. [`chromium_version.txt`](../../../chromium_version.txt) pins `155.0.8059.79`. Direct read-only Gitiles HTTP confirmed that [tag metadata](https://chromium.googlesource.com/chromium/src/+/refs/tags/155.0.8059.79?format=JSON) resolves to commit `a49e2e3d4addfd84e765b52afe230bedb81096db`. Source files were fetched individually with `?format=TEXT` and base64 decoded. No checkout, archive, preparation or Chromium build ran. The web reader failed to open Gitiles; direct HTTP succeeded. Initial guessed `ui/download/bubble/download_bubble_row_info.*` paths returned 404 and supply no evidence. The resolved paths below were inspected.

| Source at the pinned commit | Observation |
| --- | --- |
| [DownloadItem declaration](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/download/public/common/download_item.h#66) | The actual states are `IN_PROGRESS`, `COMPLETE`, `CANCELLED`, `INTERRUPTED`, plus a sentinel. Pause and resumability are separate facts. `PercentComplete()` documents unknown as `-1` and complete as `100`. `AllDataSaved()` can precede completion. The helper does not reproduce these states. |
| [DownloadUIModel declarations](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/download/download_ui_model.h) | Existing status, warning, filename, domain, accessibility, interruption and external-removal presentation remains authoritative. The domain display can represent the initiating site rather than the URL serving file bytes. |
| [DownloadItemModel](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/download/download_item_model.cc#316) | `PercentComplete()` can include rename/upload progress. Deriving a separate received/total percentage would lose native semantics. `IsCommandEnabled()` additionally excludes save-package pause and extension opening. |
| [DownloadBubbleRowViewInfo accessors](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/download/download_bubble_row_view_info.h) and [population](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/download/download_bubble_row_view_info.cc#78) | Native row information already determines main-button enablement, security subpages, primary commands and progress visibility/looping. Updates reset presentation before repopulation. Destruction is reported to observers. |
| [Native quick actions and progress](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/download/download_bubble_info_utils.cc#266) | Warning, insecure and enterprise paths affect quick actions and progress. Async scanning can loop even after all bytes arrive. Failure-specific resume/retry selection lives in row population, not in this helper. |
| [Native main-button handler](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/download/bubble/download_bubble_row_view.cc#656) and [progress rendering](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/views/download/bubble/download_bubble_row_view.cc#733) | The handler checks live controller/navigation/model availability, button enablement and unintended input. It records warning actions and opens the security dialog, or calls native `OpenDownload()`. Progress uses native visibility/looping, `PercentComplete()/100` and `IsPaused()`. The helper projects render choices only. |
| [DownloadCommands](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/download/download_commands.cc#169) and [base model commands](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/download/download_ui_model.cc#641) | Weak model loss disables commands. Pause/resume/cancel availability is native. Base `RETRY` reports enabled but its execution is a no-op. `DISCARD` invokes `Remove()`. Command enablement alone is insufficient to select a safe set of UI actions. |
| [Bubble command controller](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/download/bubble/download_bubble_ui_controller.cc#308) | Retry has a controller-owned request path. Keep, bypass, review and discard have warning/security behavior. Do not replace this dispatch with a bare command call. |
| [DownloadItemImpl opening](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/components/download/internal/common/download_item_impl.cc#1224) | Open/reveal eligibility includes native completion, temporary-file and external-removal conditions. Absence of a dangerous flag alone is not proof of file accessibility or safety. |

SHA-256 of the main inspected presentation inputs, after decoding Gitiles content:

| File | SHA-256 |
| --- | --- |
| `download_bubble_row_view_info.h` | `247c037c97b616d6dc4d91aab95a8abcf5069623baf81125c2a0c1aea418a1f5` |
| `download_bubble_row_view_info.cc` | `57a6f5cf0d1836571c009a243cb7ccb7f025e4f5d0d40d67c410594ceca3019f` |
| `download_bubble_info_utils.cc` | `792ad92d2a5e7dbf576ea614f6bb86975697bc7e71b292a0e022268fd939efa0` |
| `download_bubble_row_view.cc` | `7227a2f6b22eafd96ee698843a4fc670d4fefe02bb6045cc538d177a331a55b7` |
| `download_item.h` | `331654852c822268159c948c1b0b00d603cb8906e815c4f572acfad26ac85aa2` |

Local source differences were also inspected. [`add-specific-error-for-disabled-extension-downloads.patch`](../../../patches/helium/ui/add-specific-error-for-disabled-extension-downloads.patch), Git blob `4bae98d1090d95e22412f577de0efc11577a3bf6`, adds `OPEN_HELIUM_SERVICES` and specific blocked-extension text. [`fix-building-without-safebrowsing.patch`](../../../patches/inox-patchset/fix-building-without-safebrowsing.patch), blob `4ec6d2189de8ef3f3dfb6be51dc8092989c16acd`, changes Safe Browsing build paths and download-removal handling. These observations do not establish what protections are enabled in a built Helium binary. Preserve the actual build's native warning, enterprise and OS trust paths. Neither patch was applied or changed here.

## Native binding boundary

The future renderer reads these existing accessors together on the native UI sequence while the row/model remain alive:

| `NativeRowSignals` field | Authoritative native accessor |
| --- | --- |
| `main_button_enabled` | `DownloadBubbleRowViewInfo::main_button_enabled()` |
| `has_security_subpage` | `DownloadBubbleRowViewInfo::has_subpage()` |
| `progress_visible` | `DownloadBubbleRowViewInfo::has_progress_bar()` |
| `progress_looping` | `DownloadBubbleRowViewInfo::is_progress_bar_looping()` |
| `paused` | That row's `DownloadUIModel::IsPaused()` |
| `percent_complete` | That row's `DownloadUIModel::PercentComplete()` |

On a native update, derive a fresh presentation. On model/row loss, clear it; default signals disable the control and hide progress. Do not combine signals from different rows, Profiles or observation moments. The helper does not install an observer or guarantee a coherent native snapshot. The binding table is a source-checked integration specification, not a compiled or installed adapter.

For activation, revalidate current native identity and retain the existing native event handler, input protection, warning telemetry, security subpage and command controller. The returned route is a render hint, not a permission or a durable command token. Never act on the cached route directly. Primary commands and quick actions remain native, including current enablement, command IDs, text, icons and failure-specific retry routing. There is no custom retry implementation here.

Use existing native status/warning text, `GetFileNameToReportUser()` with the existing UI text/elision path, `GetDownloadDomainForDisplay()` and native accessibility presentation. Do not parse raw file paths or URLs into a new safety label, expose credentials from URLs, or call a renderer-computed percentage a completion verdict. Filename spoofing, origin formatting and accessibility are not tested by this component.

`DownloadItem::Remove()` can delete an intermediate file for an in-progress or interrupted download. It must not be labeled harmless remove-reference. `DeleteFile()` has asynchronous success/failure semantics. This component offers neither operation. Destructive/cleanup policy remains open; no deletion, quarantine bypass, dangerous-file validation or trust override is introduced.

The [lifecycle contract v3](../lifecycle-contract.md), [native design](../native-space-integration.md) and [pinned ownership references](../native-space-source-references.md) were inspected. No Space/page ownership is added here. Downloads remain owned by their existing Profile-native service, independent of this transient row projection. Cross-Profile Space switching must target the correct existing service without moving downloads or introducing a second queue. Same-OS-window production feasibility remains subject to the native ownership extraction gate. This helper proves no Space-switch continuation or Profile isolation.

## Focused checks

From the repository root, using the existing C++17 compiler and Chromium formatter style:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/downloads/row_presentation.h \
  arcified/native/features/downloads/row_presentation.cc \
  arcified/native/features/downloads/row_presentation_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/features/downloads/row_presentation.cc \
  arcified/native/features/downloads/row_presentation_test.cc \
  -o /tmp/arcified-downloads-row-presentation-test
/tmp/arcified-downloads-row-presentation-test
git diff --check -- arcified/native/features/downloads arcified/docs/features/downloads.md
```

The implementation run compiled and executed `/tmp/arcified-downloads-source.Jom476/row-presentation-test` with the flags above. It printed `Download row presentation fixtures passed` and exited successfully. Synthetic fixtures cover all enablement/security-route combinations, hidden progress at 100 percent, unknown/out-of-contract percentages including integer extremes, every integral native percentage from 0 through 100, native looping/paused combinations, and independent projections without retained controls. The percentage scale comes from the pinned `PercentComplete()` contract, not an added capacity or policy. Checks use explicit process failure through `Check()`, independent of `NDEBUG`.

The formatter dry run passed. `git diff --check` emitted no diagnostics. Each of the four new files also passed `git diff --no-index --check /dev/null <path>` with no whitespace diagnostics; exit status 1 records the new-file difference. All local Markdown link targets were checked with filesystem metadata and exist. No Markdown formatter/linter was found in the inspected Arcified configuration.

These checks exercise only `DeriveRowPresentation` with synthetic signals. They do not instantiate Chromium classes, run native download code, establish scanning classification or prove command execution. New-file whitespace checks include untracked files explicitly because ordinary `git diff --check` omits them.

Coordinator verification passed after making the optional-value test guards explicit: the formatter, the same compile flags and fixtures, and `clang-tidy --checks='clang-analyzer-*,bugprone-*' --warnings-as-errors='*' arcified/native/features/downloads/row_presentation.cc arcified/native/features/downloads/row_presentation_test.cc -- -std=c++17`. The installed clang-tidy has no enabled default checks, so the analyzer and bugprone families were selected explicitly. It reported no user-code diagnostics. The coordinator also read the pinned main-button and progress handlers directly and confirmed the documented routing and progress mapping.

## Open native acceptance

Issue 36 remains open. No native build or visual interaction ran on Linux, Windows or macOS. There is no installed interface to visually verify for this unconnected component. Integration still needs actual Profile/service binding, native update/destruction handling, native command dispatch, safe filename/origin rendering, warnings and OS trust prompts, keyboard/focus/accessibility evidence, and build identity.

Run public/synthetic downloads through pause/resume support, cancellation, failure-specific retry/refusal, missing files and permission failure. Check partial and scanning states, blocked/insecure/quarantined downloads, restart restoration, failure text and correct Profile scope through approved Space switches. Verify continuation in the same actual OS window only after the ownership extraction gate is resolved. File removal/deletion needs the open policy decisions and separate real-file failure/recovery evidence. Standalone success cannot close any of those native/platform requirements.
