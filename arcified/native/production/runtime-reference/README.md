<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Runtime reference evidence for issue 42

Research packet delivered. Actual reference runtime observation is blocked and native acceptance is OPEN. No screenshot, recording, account access, app execution or native implementation is claimed.

The authoritative issue is [snapshotted here](authoritative-issue.json). Its baseline is `a4b6b480d3de07857ab6e939149254a8359e5449`. The full assignment, root control manifest and lifecycle/native integration/execution authorities were read with fast_filesystem. [sources.json](sources.json) records their exact paths and measured SHA-256 identities. The root manifest changed concurrently; its full reread preserved this leaf's scope, ownership and command allocation. Both observed hashes are retained. Root owns lifecycle and ownership/lifetime policy. This packet requests coordination with the separate focus-routing-contract leaf.

## Supported findings

1. Existing macOS identities are Arc 1.170.0 build 89123 and Dia 1.54.2 build 89247. Their recorded DMG hashes, architectures, original acquisition routes and REA evidence IDs are retained in sources.json from [artifacts.md](../../../docs/research/arc-dia/artifacts.md). These hashes were copied from the existing evidence, not recomputed over installers here. Static Arc menu selectors are workflow leads. Dia's decoded interfaces cover updater dialogs only. Neither proves a feature is callable or current.
2. Windows needs separately identified packages from [issue 31](https://github.com/Timpan4/helium-arcified/issues/31). No dependency result existed at the scoped check. This is a snapshot, not a claim that its concurrent primary will not deliver one. No macOS hash, Dia release-note version or Arc menu resource substitutes for a Windows package, signature status or runtime.
3. Arc's publisher says new accounts closed on October 22, 2026 and desktop operation/sign-in ends on December 22, 2026. Any future Arc run therefore needs an existing authorized test account and a supported host. No login was attempted. These are published constraints, not observed shutdown behavior. The assigned help-center sunset URL could not be fetched; the live [publisher farewell](https://arc.net/) supports the dates used.
4. The cached [Pinned Tabs article](https://resources.arc.net/hc/en-us/articles/19231060187159-Pinned-Tabs-Tabs-you-want-to-stick-around) explicitly covers macOS and Windows and describes outbound pinned-site Peek. Its direct URL returned 404 here. It supplies no exact site classifier, redirect contract or focus proof. The cached [Folders article](https://resources.arc.net/hc/en-us/articles/19228419623447-Folders-Stash-Similar-Tabs-Together) describes hover/search preview and a child under a closed folder specifically for macOS. Its general platform heading does not prove Windows preview.
5. Dia documents different changes by platform: Windows v1.2.0 pinned-URL return, Windows v1.3.0 hover reveal with splits, and Mac v1.53.0 sidebar/split transition fixes. Both October 22 platform entries mention external-app-dialog keyboard support. These are leads for the scenario rows, not runtime passes. The [changelog](https://www.diabrowser.com/changelog) labels Mac v1.54.0 while the recorded DMG is 1.54.2. Preserve that distinction.
6. The Mac-oriented [multitasking page](https://start.arc.net/master-multitasking) documents split creation and mini-player/PiP entry. It does not establish cross-platform return targeting or live page preservation. No inspected source settles handoff identity, modal ownership, focus restoration or same-window real Profile isolation.

All external sources were checked on 2026-10-24. sources.json distinguishes live pages, cached first-party search results, failed fetches and historical local references. Remote page bytes were not archived. Original temporary artifact paths could not be inspected because fast_filesystem refused them as outside allowed roots. Their availability is unknown, not absent; no alternate route was attempted.

## Complete scenario inventory

[scenarios.json](scenarios.json) supplies steps, capture requirements, documentary differences, unknowns and downstream constraints for every required family:

| Family | Runtime questions retained |
| --- | --- |
| Peek/pin URL rules | Saved versus current URL, same-site/outbound triggers, redirects, explicit invocation, dismiss/reset/promotion and parent focus |
| Tab handoff | Existing page versus duplicate/reload, source/destination, protected state, real Profile and same-native-window continuity |
| Folder preview | Hover/keyboard access, search, nested hierarchy, expansion state and active descendant |
| Split transitions | Create/reorder/resize, active pane, promotion/dissolution, modal/fullscreen return and motion |
| Sidebar reveal | Pointer/keyboard reveal, page versus workspace selection, active cues and command target |
| Media return | PiP/mini-player source after navigation, Space/window changes or transfer, stale source and competing players |
| Native dialogs | Permission/file/print/download/protocol/credential ownership, keyboard cancel and return across split/Peek/selection changes |

Each family has distinct Arc macOS, Dia macOS, Arc Windows and Dia Windows rows. All 28 product/platform runtime rows are unexecuted. There are also seven Linux rows marked NO_OFFICIAL_REFERENCE_PRODUCT, per the authoritative issue. This inventory covers the requested scenarios; it does not claim runtime coverage passed. No captures exist.

The future capture record requires actual OS/app/package identity, account authorization reference, scenario steps and observed result, platform differences, capture paths and hashes. Use account/Profile aliases. A documentary screenshot or serialized selector cannot substitute for a recording of the interaction. Native object continuity needs native instrumentation as well as visual evidence.

## Acceptance amendments proposed to the coordinator

These are packet-local proposals. No shared issue, acceptance list or root control was changed.

1. Criterion 1 stays OPEN. Require exact package and supported-host identity for each product/OS run plus an authorized test Profile/account reference. Pin macOS to the recorded bytes. Reconcile Windows through issue 31 before any run. Linux has no reference-product parity claim; its own native checks remain OPEN.
2. Criterion 2 stays OPEN. Use the per-product scenario inventory and link actual screenshots/recordings and notes. Where a feature cannot be reached, distinguish login blocked, unavailable host, not found in that build, documented unsupported and documented retired. Do not turn any of these into a passing interaction.
3. Criterion 3 has documentary preparation only and stays OPEN for runtime reconciliation. Mac-specific folder preview does not establish Windows parity. A missing Peek help page does not establish feature retirement. Retained Notes/legacy Boost selectors in the static record do not establish active UI. Route retirement/service dispositions through issue 23; unavailable live-folder/calendar/account actions require a specific inaccessible result, not inferred parity or a replacement service.

No owner rule is adopted for cleanup, destructive actions, source disposition, providers, same-site routing or redirects. The approved lifecycle Model, canonical metadata/codec, immutable Profile/page identity and private/download classifiers remain authoritative.

## Foundation reuse and native boundary

[Peek](../../../docs/features/peek.md), [saved items](../../../docs/features/pins.md), [folder preview](../../../docs/organization-folder-preview.md) and [media/PiP](../../../docs/media-picture-in-picture.md) already document pure component work. Their tests were not rerun because this packet changes no components. Their success is not evidence for native focus, dialogs, live navigation or renderer state. Root's execution-state records separate pure checks from open native ownership/build/interaction gates.

This is a research leaf with no authorized runtime environment and no behavior settled by observation. A production native patch, prepared-source base/result files, registered native tests, ActionItem mapping and IDS_ARCIFIED_RUNTIME_REFERENCE strings are inapplicable to this packet. No command is added; 40799 remains reserved to this leaf and Library 40701 is untouched. No patches/series entry or build input changed. Do not add a facade, second Model/store or fake native test to make this research appear implemented.

Same-window multi-Profile support and exact live saved-page handoff remain production gates. Model::OpenItem success, matching URLs, page titles or before/after screenshots alone cannot pass them. Focus-routing-contract should consume these scenarios without changing root's lifecycle ownership contract.

## Decisions for consolidation

1. Which supported macOS and Windows test hosts and existing authorized Arc test account/Profiles can the coordinator allocate for the recorded-artifact captures? Recommend existing test Profiles with fixture content. This enables evidence without personal browsing data; fresh Arc signup is unavailable under the published policy.
2. If an Arc scenario is inaccessible because of login or sunset, should its acceptance explicitly retain historical-only reference evidence and an OPEN downstream verification gate? Recommend that disposition. It preserves honest coverage but cannot settle the missing runtime behavior.

No owner question was sent directly. Coordinator owns consolidated questions and authorization for any later runtime session.

## Checks and remaining work

[checks.json](checks.json) records exact checks and results. [verify.jq](verify.jq) validates the issue identity, scenario/platform inventory, source links, honest unrun states, empty captures and open native acceptance. JSON uses two-space formatting and final newlines. Markdown was checked for whitespace and local-link validity. No project Markdown formatter/linter was found in the inspected root/extension configuration or executable lookup; no new tool or dependency was installed.

Next coordinator actions are to reconcile issue 31, resolve host/account allocation, authorize the supported-host capture session separately, obtain fresh independent review, and record actual per-platform results or precise inaccessible states. Native acceptance and all actual runtime criteria remain OPEN.
