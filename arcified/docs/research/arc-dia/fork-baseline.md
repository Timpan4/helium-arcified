# Arc and Dia fork baseline

Reviewed 2026-10-23 from the checked-in `Timpan4/helium-arcified` repository. This is a source and build-configuration review. No browser build was run or installed, and this review does not establish that any patch applies or that any UI works at runtime.

## Current state

The fork has a Manifest V3 browser-extension prototype named Arcified Lab. It can save the current window's ordinary web URLs as named snapshots, reopen a snapshot in a separate window, list and activate tabs in the current window, and explicitly capture the active page for a local LM Studio request. The app stores snapshots in extension-local storage. It does not implement live Arc Spaces, a native sidebar, Dia's multi-tab assistant, or native Chromium integration.

The extension is useful for trying small pieces of state and UX. It is not a native browser feature and its tests do not exercise a browser. The prototype and its CI cover none of the native browser build, input, accessibility, data lifecycle, package, update, or performance requirements for Linux, Windows, and macOS.

## Evidence by capability

| Area | Present in the fork | State and evidence |
| --- | --- | --- |
| Workspaces and tabs | Extension can save up to 120 eligible tab URLs per snapshot, keep up to 40 snapshots, rename by saving over a same-name entry, delete a snapshot, reopen its URLs in a new window, and activate a tab in the current window. | Prototype only. `arcified/extension/src/panel.ts` and `state.ts`. It stores URLs, not tab/session objects; snapshots are separate from live windows and do not track navigation or tab lifecycle. The limits are implementation constants, not a product spec. |
| Extension UI | Toolbar action opens a Chromium side panel. It presents saved snapshots, current-window tabs, explicit page capture, a question box, and a text answer. | Prototype only. `arcified/extension/static/manifest.json`, `panel.html`, `panel.css`, `src/background.ts`, and `src/panel.ts`. This is the browser extension side panel, not a native Helium sidebar. |
| Dia-style assistant | User clicks Read current page, grants optional host permission for that origin, then clicks Ask. It sends the question and captured title, URL, selection, and page text to `http://127.0.0.1:1234/v1` (LM Studio). The page text is marked as untrusted in the system prompt. | Local, single-page, non-streaming prototype only. `arcified/extension/src/ai.ts`, `src/panel.ts`, and `static/manifest.json`. No `@tab`/`@space` selection, citations, provider abstraction, API-provider authentication, streaming, tool execution, or explicit consent flow for any remote provider. The extension sends context to the local endpoint only after Ask, but that alone is not a complete assistant threat model. |
| Persisted data | Extension storage contains a version-keyed JSON-like snapshot shape. The loader filters malformed entries and unsafe/non-web URLs. | Local prototype only. `arcified/extension/src/state.ts` and `src/panel.ts`. There is no schema migration, backup, export/import, recovery, cross-device sync, encryption, or conflict resolution. |
| Tests | Three Bun unit tests cover URL filtering, normalization of malformed persisted values, and snapshot upsert/name normalization. | These test pure extension state helpers only. `arcified/extension/tests/state.test.ts`. They do not test extension UI flows, browser APIs, native browser behavior, platform input, or persistence failure. |
| Arcified native patches | No `patches/arcified/` directory or Arcified entries in `patches/series` exist in this checkout. | Native Arcified integration is not started. The series has Helium patches that provide adjacent browser capabilities: vertical tabs and layout (`patches/helium/ui/layout/core.patch`, `vertical.patch`), tab styling (`patches/helium/ui/tabs.patch`), split view (`patches/helium/ui/split-view.patch`, `multi-contents-view.patch`), side panel (`patches/helium/ui/side-panel.patch`), keyboard shortcuts (`patches/helium/core/keyboard-shortcuts.patch`, `patches/helium/ui/custom-keyboard-shortcuts-wiring.patch`), lazy session restore (`patches/helium/core/lazy-session-restore.patch`), frame materials (`patches/helium/ui/native-frame-materials.patch`), and memory-saving defaults (`patches/helium/core/memory-saving-by-default.patch`). They are Helium baseline patches, not Arcified implementations or evidence of Arc semantics. |
| Roadmap | M0 records the fork, extension sandbox, and three-OS extension checks as done. Browser installation observations remain unchecked. M1 live Spaces, tab lifecycle, and native address-bar/sidebar integration; M2 native interface and cross-platform browser builds; M3 Dia features and sync; and M4 releases remain unchecked. | Planned only. `arcified/docs/roadmap.md`. |

The extension's `normalizeStored` function accepts the current snapshot shape but contains no migration path between schema versions. `STORAGE_KEY` is `arcified-lab-v1`. Saving data updates the local extension storage; no user-visible export or recovery mechanism exists. `readPage` captures only the active tab after the user clicks and obtains origin permission. The manifest grants the extension access to the loopback LM Studio endpoint and declares HTTP/HTTPS page access optional.

## Native and platform build evidence

This repository is a Chromium patch/configuration repository, not a checked-in Chromium source tree. `arcified/docs/native-integration.md` says native work must be implemented and tested in a Chromium source tree before refreshing quilt patches. The checked-in `patches/series` contains the Helium baseline in order and no Arcified patch entries. `devutils/check_patch_files.py` checks series references and patch readability; `devutils/validate_patches.py` checks patch applicability against a retrieved source tree. These are patch validation steps, not browser compilation or runtime tests.

`.github/workflows/arcified-extension.yml` runs Bun install and `bun run check` on Ubuntu, Windows, and macOS runners and uploads extension `dist` artifacts. It provides cross-OS checks for extension typecheck/build/unit tests only. The repository's `.github/workflows/ci.yml` runs its patch/configuration validation on Ubuntu, including retrieving Chromium source for patch validation; it does not build the browser. No checked-in workflow builds or exercises a native Arcified browser on any OS.

The root README identifies separate Helium packaging/build repositories for Linux, Windows, and macOS. The current fork does not contain Arcified release packaging, application identities, signing, update channels, or app-binary CI. `arcified/docs/native-integration.md` links to the upstream platform build projects and explicitly notes that Bun cannot compile Chromium.

## Proposed end-to-end issues

These are candidate issue scopes for the fork's own tracker. They are not filed. Keep native changes in `patches/arcified/` and its `patches/series` entries, as permitted by `arcified/AGENTS.md`; preserve the existing Helium patch order and upstream contribution boundary.

### 1. Define and implement live Space and tab lifecycle in native Chromium

**Scope:** Establish a browser-owned model for Spaces, their windows, pinned tabs, active tab, and per-Space session state; connect create, switch, close, restore, and tab navigation to that model. Resolve the documented close behavior for pinned tabs, audio, downloads, and unsaved forms before freezing the contract.

**Acceptance criteria:** A native browser session can create, rename, switch, close, and restore multiple Spaces. Each Space retains its own live tab identities and pinned-tab state across restart. Switching Spaces does not silently close tabs or lose downloads, audio state, or unsaved-form warnings. A focused native test covers create/switch/restart/restore and the decided close behaviors. The implementation is carried as a downstream patch group and applies to the pinned Chromium source revision.

**Dependencies:** Product decisions for lifecycle semantics; Chromium source checkout and test/build environment; native session-restore and profile ownership design. Existing `lazy-session-restore.patch` is an adjacent primitive, not a substitute for a Space data model.

### 2. Connect the native Space switcher to Helium's sidebar and navigation

**Scope:** Add the Arc-style Space selector and live tab list to Helium's native browser chrome, then connect selection, pinning, temporary tabs, Peek, and split view to the native tab/window model. Keep omnibox navigation and browser commands attached to the selected live Space.

**Acceptance criteria:** A user can switch Spaces and select/open/close/pin tabs entirely in native browser chrome; the omnibox opens pages in the selected Space according to the agreed temporary-tab rule; Peek and split view act on the correct live tabs; state remains correct after switching windows and restoring a session. Native UI/controller tests cover those flows. No extension side-panel API is required for the core flow.

**Dependencies:** Issue 1's state and lifecycle contract; Chromium source tree. Reuse or adapt Helium's `layout/vertical.patch`, `side-panel.patch`, `split-view.patch`, and `multi-contents-view.patch` only after verifying behavior against the new model.

### 3. Verify native interaction and accessibility across Linux, Windows, and macOS

**Scope:** Make the core Space and tab flows reliable with mouse, keyboard, assistive technology, IME composition, display scaling, and multi-window/multi-display use. Account for native caption controls, window dragging/resizing, and platform-specific focus behavior.

**Acceptance criteria:** On each supported OS, documented manual and automated checks cover keyboard-only Space/tab navigation, accessible names/roles/states and focus announcements, IME text entry in editable browser chrome, drag/drop and context-menu actions, high-DPI scaling, mixed-scale displays, maximize/fullscreen/restore, and caption-button/window hit testing. No clipped or unreachable controls occur in the tested configurations; failures are recorded with OS and display settings. Existing Helium vertical-tab/frame patches are not considered verified by their presence in the series.

**Dependencies:** Native Space UI from Issues 1–2; access to all three native build/run environments and representative scaling/IME configurations; platform accessibility inspection tools.

### 4. Add recoverable local data migration and export/import

**Scope:** Define a versioned, validated on-disk format for Space state, migrate the extension's snapshot data where technically feasible, and provide user-controlled export/import plus safe failure recovery.

**Acceptance criteria:** Migration preserves supported names, URLs, ordering, and timestamps, reports data it cannot map, and never silently replaces the original profile data on failure. Export produces a documented portable file; import validates it before mutation and handles duplicates/conflicts predictably. Tests cover each supported schema migration, malformed/corrupt files, interrupted writes, duplicate names, and round-trip export/import. User data remains local unless a separate sync flow is enabled.

**Dependencies:** Issue 1's native data model; a migration policy for extension-local snapshots; a documented format and ownership boundary for profile data.

### 5. Build opt-in Dia context selection with privacy and source attribution

**Scope:** Replace the single captured page with explicit `@tab` and `@space` context selection and a provider boundary for local models first. Add streaming answers and source citations while keeping page access and provider disclosure explicit. Defer remote API providers until authentication, consent, retention, and threat-model decisions are approved.

**Acceptance criteria:** The user sees and confirms exactly which tabs/Space are included before a request. Requests are sent only to the selected provider; no background page collection occurs. Answers stream and cite the source tab/title/URL for claims grounded in supplied pages. Page content is treated as untrusted input; provider failures, cancellation, permission denial, and empty or inaccessible pages have recoverable UI states. Tests verify request payload scoping and that no unselected page content is sent.

**Dependencies:** Native tab/Space model (Issue 1); a documented consent and data-flow threat model; a provider interface and separately reviewed credentials plan before remote providers. The current local LM Studio request in `arcified/extension/src/ai.ts` is not Dia feature coverage.

### 6. Add encrypted cross-device Space sync with conflict and recovery behavior

**Scope:** Synchronize the agreed Space data across devices with end-to-end encryption, device enrollment/revocation, conflict handling, and restore/export paths.

**Acceptance criteria:** The threat model identifies metadata and content exposed to the service, key creation/storage/recovery and device revocation are documented, and sync cannot upload plaintext Space contents. Tests cover concurrent edits, offline changes, duplicate/conflicting names, revoked devices, corrupt ciphertext, key loss, and restore from export. The UI shows sync state and actionable errors without discarding local data.

**Dependencies:** Stable native data schema and migration/export (Issues 1 and 4); an approved sync service and key-management design; security review. Do not infer sync support from Helium's existing sync-related patches.

### 7. Establish native performance budgets and regression coverage

**Scope:** Measure the user-visible costs of browser startup/session restore, Space switching, tab strip rendering, memory use, and assistant context capture on reference Linux, Windows, and macOS systems. Set targets from measured baseline and product decisions rather than inventing thresholds.

**Acceptance criteria:** A reproducible benchmark plan records hardware, OS, build revision, profile size, and workload. It reports startup/restore and Space-switch latency distributions, idle and loaded memory, tab-count scaling, and UI responsiveness during context capture/assistant streaming. Results are compared with the Helium baseline and regressions have an agreed triage gate. Long-running capture/refinement yields to the UI and supports cancellation. `memory-saving-by-default.patch` and `lazy-session-restore.patch` are not accepted as performance verification by themselves.

**Dependencies:** Native feature flows (Issues 1–2); reproducible native builds on all three platforms; reference hardware and an agreed measurement method. Any numeric budgets require measured evidence and owner approval.

### 8. Package, sign, update, and security-review native releases

**Scope:** Integrate the fork's native patches into each platform's packaging pipeline; define product identity, code signing/notarization where required, update metadata/channels, rollback, and release security checks.

**Acceptance criteria:** Linux, Windows, and macOS produce installable, correctly branded artifacts from the same declared source revision. Each platform documents signing identity and verification, update source/channel, signature validation, rollback/failure recovery, and release provenance. Clean install, upgrade, downgrade rejection or supported rollback, and data-preserving update are verified on each OS. Release CI reports native package and security checks; extension artifacts are not presented as browser releases. Proprietary Widevine binaries are not bundled.

**Dependencies:** Native features and stable profile/data migration; access to the fork's platform packaging repositories and authorized signing/release infrastructure; a decision on distribution channels and update ownership. Root `README.md` and `arcified/docs/native-integration.md` point to separate upstream platform repositories, so changes there require fork-owned integration or explicit downstream coordination.

## Boundaries and verification not performed

This report uses repository source, documentation, patch inventory, and workflow definitions only. It does not inspect official app binaries or compare runtime behavior; that artifact REA belongs to the parent task owner. It does not create issues, contact upstream, or claim that existing Helium patches build or run on any target OS. The current evidence supports extension prototype behavior at source level and CI configuration coverage, not installed-browser behavior or native feature parity.
