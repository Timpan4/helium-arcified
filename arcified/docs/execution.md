# Implementation execution

Non-AI browser work comes first. AI has 23 later items and six sync proposals are excluded. Full browser builds, heavy source preparation and remote CI dispatch remain deferred.

## Current evidence

Of 135 non-AI items, 0 are fully proven complete, 1 research item is locally complete awaiting publication, 56 are in progress and 78 are not started. All 51 additional primary assignments are confirmed as GPT-6.1 High. Assignment and reviewed local components are distinct from native acceptance.

The local baseline is checkpoint `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. It contains the authorized fork-wide permission change, published backlog, native preparation foundations, lifecycle v3 candidate and performance fixtures. No implementation push or PR has run.

| Work item | State | Evidence and remaining work |
| --- | --- | --- |
| [Performance protocol #20](https://github.com/Timpan4/helium-arcified/issues/20) | Locally complete, awaiting publication | Six focused tests, syntax checks, clean independent review and tiny browser form/ready/echo/downloaded-JSON checks. No native benchmark result. |
| [Ownership contract #24](https://github.com/Timpan4/helium-arcified/issues/24) | v3 independently reviewed; native acceptance open | 16 fixtures and supplemental sanitizer groups passed. Earlier borrowed-ID P1 cleared. Native state/auth transfer remains unverified. |
| [Linux #16](https://github.com/Timpan4/helium-arcified/issues/16), [Windows #17](https://github.com/Timpan4/helium-arcified/issues/17), [macOS #18](https://github.com/Timpan4/helium-arcified/issues/18) | Partial preparation foundations reviewed | Shared path/provenance and macOS filesystem findings cleared. Native preparation, SDKs, GN, build and launch remain OPEN. |
| [Spaces #32](https://github.com/Timpan4/helium-arcified/issues/32) | Preparatory source design reviewed; refinement running | Same-window multi-Profile host extraction remains a production proof gate. No view-only or replacement-window acceptance claim. |
| Lightweight native checks | Manual-only candidate; P2 fixed and independently cleared | The overflow probe exits 1 with the fix, versus 0 without. No CI dispatch. Windows/macOS runner execution remains unverified. |

## Parallel workstreams

Each isolated worktree has a GPT-6.1 High coordinator instructed to delegate one GPT-6.1 High primary per listed item and independently review the results. Luna helpers are allowed. Coordinators own separate paths and coordinate shared interfaces before edits. Native acceptance stays open where builds or required owner policies block proof.

| Area thread | Item keys |
| --- | --- |
| [tabs](t3-thread://v1/mcp:087e06d6-7397-442f-aa44-0d521b8f705a) | temporary-tabs, pins, favorites |
| [organization](t3-thread://v1/mcp:09a422d8-5f11-4ffc-868b-1f651c2b6690) | folders, folder-preview, multi-select |
| [workspace](t3-thread://v1/mcp:dff658dd-1e55-4c88-a8be-52e0060beccf) | space-management, workspace-organizer, window-ownership |
| [profiles](t3-thread://v1/mcp:21d7ebe2-965d-4d7f-b4f3-3dca19c97599) | profiles, profile-safety, private-browsing |
| [durability](t3-thread://v1/mcp:2e55284e-3f1f-4675-b463-730c26d78321) | session-durability, schema-migrations, undo-windows |
| [portability](t3-thread://v1/mcp:479fbda1-3a40-467e-974d-530eba9b2b19) | export, workspace-import, prototype-migration |
| [commands](t3-thread://v1/mcp:789c2e5d-e313-424d-9632-88eeebcca9a9) | command-bar, command-ranking, site-search |
| [navigation](t3-thread://v1/mcp:585bbe99-dbb0-4250-821a-4128dfc86d96) | tab-switcher, tab-order-navigation, find |
| [transients](t3-thread://v1/mcp:145b2d72-e3d4-43d9-b6e8-5b91cc83ab05) | peek, little-arc, air-traffic |
| [splits](t3-thread://v1/mcp:34db67d9-f29c-411b-979e-6ed66a07f26b) | split-horizontal, split-vertical, split-persistence |
| [layout](t3-thread://v1/mcp:7db91a56-ed4a-411d-ba5b-0aaddb0a1955) | sidebar-layout, tab-layout-choice, focus-mode |
| [appearance](t3-thread://v1/mcp:c80846e0-bd66-49a4-a9c5-c6d84bfa8c40) | space-themes, motion, shortcut-settings |
| [library](t3-thread://v1/mcp:fc95390f-0e9a-4e5e-8940-c0e5690f7d16) | library, downloads, history-controls |
| [capture](t3-thread://v1/mcp:77b4a091-1bbd-40db-a7cf-57a02cdf0d3e) | capture, region-capture, recent-files |
| [media](t3-thread://v1/mcp:67104e7f-06f8-4791-baa8-c62117d9b463) | pip, media-controls, tab-status |
| [site-tools](t3-thread://v1/mcp:3929680e-69d2-476b-ab24-251509e618fa) | site-controls, developer-mode, browser-diagnostics |
| [accessibility](t3-thread://v1/mcp:316a7ad0-e14e-4b98-82ff-f673ef9feb54) | accessibility, ime-localization, window-chrome |

The [machine-readable manifest](parallel-workstreams.json) records actual thread IDs, worktree paths, model selections and primary task receipts.

## Owner decisions and open work

Approved: saved Space tabs shared across windows; temporaries window-local; live saved pages transfer here preserving state; cross-Profile Spaces use the same actual native window; one Profile per Space with shared Profiles allowed; Profile-scoped sign-ins, site settings and Favorites; Space-scoped themes; saved-page outbound links branch temporarily. Tiny synthetic performance-fixture browser checks are permitted.

Activity protection, automatic cleanup, destructive actions and populated cross-Profile migration policies remain open. Teams must not assume approval. Cross-platform native UI, authentication isolation, real persistence and performance remain unverified.

The [execution state](execution-state.json) and [fixture browser report](reviews/performance-browser-check.md) distinguish local checks from native evidence. Parent owns integration and GitHub state. Next: collect assignment receipts, integrate reviewed components and keep blocked native acceptance open.

## Game pause request

The owner chose cs2 detection every 30 seconds and manual resume only. T3 rejected this interval with `Expected a value greater than or equal to 60000`. No guard is armed. A structured question is pending to accept one-minute checks or leave automatic detection off. [State](game-pause-state.json) preserves that decision gate.
