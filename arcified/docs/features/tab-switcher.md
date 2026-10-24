# Recent-tab switcher

Status: reusable standalone C++ hold-session component for [issue 78](https://github.com/Timpan4/helium-arcified/issues/78), not native integration or issue closure. Source baseline is `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. The [issue brief](../research/arc-dia/publication/tab-switcher.md) and `tab-switcher` entry in [backlog.json](../research/arc-dia/backlog.json) require hold/cycle/release, reverse/cancel, title/Space/status previews and safe reconciliation across close, move and split changes. The [implementation](../../native/features/recent_switcher/recent_switcher.h) and [focused fixtures](../../native/features/recent_switcher/recent_switcher_test.cc) implement the semantic interaction without opening or acting on real tabs.

The [lifecycle contract v3](../lifecycle-contract.md) is authoritative. The [native Space design](../native-space-integration.md) and [source references](../native-space-source-references.md) predate v3. Their ownership extraction and native feasibility gaps remain; their older statements that Profile/Space mapping is undecided do not override v3. This feature changes no shared model, command resolver, persistence, focus owner, patch series, tooling or workflow.

## Caller contract

`arcified::recent_switcher::Session` provides `Begin`, `Cycle`, `Reconcile`, `Release` and `Cancel`. `Direction` is semantic forward/reverse; this component binds no keys. `Begin`, `Cycle` and `Reconcile` return an `Update`. `Release` returns an `EndReason` or a plan-specific `ActivationIntent` and ends the session. `Cancel` ends it with no activation intent. Candidate and selected-row getters are presentation views invalidated by the next non-const call.

The caller supplies one serialized `Snapshot` paired with the current read-only `lifecycle::Model`:

| Input | Authority and meaning |
| --- | --- |
| `window` | Requesting logical window. A changed or closed window cancels the hold. |
| `context_generation` | Caller-owned guard changed on external focus, selection or context changes. It must not be reused during a session. This is not a persisted MRU, a lifecycle revision or a native-object generation. |
| `focused_page` | Current authoritative focus, used as the initial cycle anchor and for an already-focused release no-op. The caller must change its context guard when external focus changes. No focus is inferred from selected Space, tab indices or split order. |
| `eligible_spaces` | Explicit caller scope. There is no default global, window-wide or current-Space MRU policy. Empty scope admits no candidates. |
| `history` | Recent-first `HistoryEntry{PageId, requested_space}` values from the existing activation-history owner. The caller resolves aliases to the existing live `PageId` first. The first occurrence of a page wins, even if its destination is invalid. |
| `previews` | Caller-provided title, resolved Space label and status, keyed by live `PageId`. These strings carry no identity or activity permission. Missing metadata remains absent and never supplies an invented title or eligibility rule. |

The coordinator accepted these scope/destination rules before implementation. Every candidate first needs an existing requesting `(window, requested_space)` attachment with the page's immutable Profile, matching `Model::OpenItem`'s check order. Destination eligibility then follows existing lifecycle ownership:

| Live page owner | Destination |
| --- | --- |
| Temporary | Its exact original attachment. A page in another window or a different requested Space is ineligible. |
| Space-kept saved item | Its owning Space in the requesting window. That destination must already be attached and eligible. A caller-resolved alias does not rehome the kept item into the requesting Space. |
| Standalone Profile-owned saved item | The explicit caller `requested_space` in the requesting window, with the same Profile. The component chooses no alternative destination. |

Only already-live pages participate. Closed saved entries are not reopened. Saved items remain shared across windows, while foreign temporary pages remain excluded. A saved page live elsewhere yields a request for the existing `PageId`, never another page or a URL snapshot. Protected/unknown activity does not become a close, reset or discard decision. Cleanup, activity detection, destructive actions and cross-Profile migration remain open.

## Hold behavior

1. `Begin` freezes eligible, deduplicated history entries for this hold. It advances once from the focused page in the requested direction. If focus is absent from the candidate list, forward starts at the first candidate and reverse at the last. Empty history/scope returns `kNoCandidates`. A repeated begin refreshes the existing hold and returns `kAlreadyHolding` without advancing.
2. `Cycle` wraps in either direction. History reordering, newly opened pages and changed history destinations do not reorder or append candidates during the hold. The next hold consumes fresh history. Only ephemeral hold order is stored; this component never records or persists MRU history.
3. `Reconcile` refreshes liveness, scope, source attachment, immutable Profile and caller preview metadata. It retains selection by `PageId` when preceding rows close, a saved page transfers or splits change. It never uses tab, row or pane positions as identity. If selection disappears, it previews the next surviving frozen candidate in the last cycle direction. A cycle encountering that removal consumes the cycle once, without skipping the survivor. Removed candidates do not reappear until a new hold.
4. `Release` reconciles again. If the selected page first disappears or becomes ineligible on release, it returns `kSelectionChanged`, or `kNoCandidates` when nothing survives, instead of activating a replacement that the caller has not had a chance to present. An already-focused local page returns `kAlreadyFocused`. An unchanged eligible selection yields `ActivationIntent{entry, expected_source, destination, context_generation}`. The source attachment is freshly read, including a transfer discovered on release. Repeated release returns `kIdle`.
5. A changed caller context guard or requesting window, or a closed requesting window, ends the hold with `kContextChanged`. The native owner maps Escape, explicit dismissal, mouse interaction, widget deactivation/closing, shutdown and invalidated input ownership to `Cancel` as appropriate. Cancel and preview do not restore focus because they never change it.

Views must reconcile before rendering. The metadata owner must supply a Space label for the resolved destination, especially when an alias request resolves to an owning Space. A native layout can draw the returned rows and selection without activating a WebContents, selecting a Space, transferring a tab or requesting content focus. This component has no renderer, screenshot capture or accessibility widget; those UI behaviors remain unverified.

## Command and native adapter seam

The accepted shared resolver lives in the commands workstream as `arcified/native/features/commands/command_target.h/.cc`. Its header was inspected read-only in the peer checkout. No copy or dependency is added here before root integration.

After integration, the external coordinator consumes the plan-specific intent. It resolves `intent.entry.page` with `commands::ResolvePageCommandTarget(model, page, intent.expected_source)`, then uses `commands::RevalidatePageCommandTarget(model, target)` at the actuation boundary. The shared `PageCommandTarget` owns the resolved immutable Profile/current attachment snapshot; this feature does not define a competing generic target type or focus registry. The coordinator must separately revalidate the caller context guard, requested/resolved destination, owning item and attachment eligibility against the newest policy/model snapshot.

The native executor must also check the real TabModel/WebContents binding, Profile/BrowserContext, host lifetime and native operation generation immediately before effects on the serialized UI sequence. The semantic resolver cannot detect native replacement or a page moved away and back, and a release intent is neither a reservation nor a commit token. A changed binding/generation cancels stale work. A valid semantic intent is not native transfer permission or proof of native success.

Same-attachment activation must preserve layouts. Cross-window saved-page selection must transfer the existing live page preserving current navigation, history, document state and Profile, then coordinate successful native presentation with lifecycle reporting. It must not call `OpenItem`, `TransferSavedPage` or `SelectSpace` speculatively and claim a native effect completed. Failure/cancellation must retain the original page and prior selection. The external coordinator, not this session, owns serialization and recovery. Shared saved-page transfer and same-window multi-Profile presentation still require the ownership extraction described in the native Space design.

## Pinned source seams

The repository engine pin is Chromium `155.0.8059.79`, resolved in the existing source-reference ledger to commit `a49e2e3d4addfd84e765b52afe230bedb81096db`. The Chromium files below were read at that exact commit using Gitiles text responses. The web reader failed to open Gitiles; direct read-only HTTP succeeded. An initial guessed `ui/views/accelerator_table.cc` path returned 404; the correct `ui/accelerator_table.cc` below supplies the evidence. No source archive, prepared Chromium tree or native build was created.

| Read source | Observation and consequence |
| --- | --- |
| [accelerator_table.cc, line 62 onward](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/accelerator_table.cc#62), especially lines 95 to 99 and 303 to 308 | The non-macOS table maps Ctrl+Tab and Ctrl+Shift+Tab to cycle commands and lists those commands as repeatable. The eventual Linux/Windows adapter should translate its configured cycle command to begin/cycle and its initiating modifier release to release. This is source mapping, not proof of key event delivery. |
| [global_keyboard_shortcuts_mac.mm, line 112 onward](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/global_keyboard_shortcuts_mac.mm#112) | macOS has its own command mappings, including Cmd+Shift+brackets, Ctrl+PageUp/PageDown and Cmd+Option+arrows for tab selection. These are adjacent selection mappings, not evidence that every shortcut starts a hold. The macOS adapter still needs a verified hold/modifier-release path and must preserve ordinary ordered-navigation shortcuts. |
| [tab_strip_model.cc, line 1100 onward](https://chromium.googlesource.com/chromium/src/+/a49e2e3d4addfd84e765b52afe230bedb81096db/chrome/browser/ui/tabs/tab_strip_model.cc#1100) | `ActivateTabAt` resolves an index immediately, and `ActivateTab` checks actual owning model and changes selection. It belongs after release/native validation, never in preview cycling. Resolve the current native tab from the stable registry immediately before activation, rather than retaining an index. |
| [tab-cycling-mru-impl.patch](../../../patches/brave/tab-cycling-mru-impl.patch), especially lines 46 to 93 and 149 to 216 | Existing Helium-adapted MRU sorts last-active times into indices, rotates and activates on each cycle. Its BrowserView handler observes Control release, mouse press, widget activation and closing. Reuse the event-lifetime integration seam while replacing immediate activation/index storage for preview mode. Do not run both cycling paths for one gesture. Blob `724fe13a66020ab5c5a7340534a1d2675d8e31ae`. |
| [tab-cycling-mru.patch](../../../patches/helium/core/tab-cycling-mru.patch), especially lines 30 to 55 | The native cycling path consults its Profile preference. Preserve existing preference and ordered-navigation behavior; this component does not force MRU mode or replace the sibling's ordered navigation. Blob `74d874fde4fa904a54c802adf534d45c4d7585ad`. |
| [custom-keyboard-shortcuts-wiring.patch](../../../patches/helium/ui/custom-keyboard-shortcuts-wiring.patch), especially lines 24 to 34, 64 to 90 and 115 to 134 | BrowserView caches a Profile-specific shortcut service, installs customized accelerators and updates active macOS menu accelerators. Bind to the configured native command and initiating modifier instead of installing a global fixed shortcut. Profile service ownership remains part of the host extraction gap. Blob `69754829331fd7f8b296d93b6153575f72c9a2aa`. |

The [native source ledger](../native-space-source-references.md#c1-desktop-window-and-profile-ownership) documents Chromium's immutable single-Profile desktop Browser contract. Displaying a candidate for another Profile does not establish safe same-window activation. A hidden backing window, a changed Profile getter, another navigation controller or a reloaded copy of a saved page cannot satisfy the approved behavior.

## Focused verification and remaining acceptance

Run from the repository root with the installed C++17 toolchain:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/features/recent_switcher/recent_switcher.h \
  arcified/native/features/recent_switcher/recent_switcher.cc \
  arcified/native/features/recent_switcher/recent_switcher_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/recent_switcher/recent_switcher.cc \
  arcified/native/features/recent_switcher/recent_switcher_test.cc \
  -o /tmp/arcified-recent-switcher-test
/tmp/arcified-recent-switcher-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/recent_switcher/recent_switcher.cc \
  arcified/native/features/recent_switcher/recent_switcher_test.cc \
  -o /tmp/arcified-recent-switcher-sanitized
/tmp/arcified-recent-switcher-sanitized
```

The fixtures use synthetic IDs and `.test` destinations only. They cover forward/reverse wrap, focus anchoring, repeated begin/release, cancellation, empty/single lists, deduplication, frozen history/destinations, refreshed/missing metadata, selected/unselected close, directional reconciliation, release-time liveness/destination/scope rejection, split reorder/removal, preserved saved identity and source refresh on transfer, read-only navigation/activity/layout preservation, kept/alias owner visibility, explicit Profile-shortcut destinations, temporary locality, logical multiple-Profile candidates, guard/window cancellation, Keep identity retention and closed-item reopen with a different page identity. They do not call a native tab or focus API.

Checks on 2026-10-23: the strict Clang C++17 build and executable passed all 13 fixture functions. The separate AddressSanitizer/UndefinedBehaviorSanitizer build and executable passed the same fixtures without diagnostics. The Chromium-style formatter check passed for the header, implementation and test file. New-file `git diff --no-index --check /dev/null <path>` checks emitted no whitespace diagnostics; exit 1 records the new-file differences. These are standalone Linux-host checks, not native Chromium build tests or installed-browser checks.

Installed native acceptance remains open on Linux, Windows and macOS separately. This standalone component does not prove keyboard repeat/modifier delivery, visual previews, accessibility announcements, focus retention in a rendered UI, actual activation, same-window multi-Profile ownership, state-preserving native transfer or recovery from native failure. It adds no persistence/restore or cleanup policy. The external resolver/executor and native identity registry must be integrated and tested with synthetic pages before any of those claims can be made. No AI, account or sync integration is introduced.
