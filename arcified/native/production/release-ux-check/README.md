# Release UX verification packet

Issue [#161](https://github.com/Timpan4/helium-arcified/issues/161), `release-ux-check`. Baseline `a4b6b480d3de07857ab6e939149254a8359e5449`. Recorded 2026-10-24.

Native acceptance is OPEN. This packet reconciles the catalog and defines the remaining installed-candidate checks. It contains no executed native scenario, production implementation, release approval or unsupported-platform certification.

## Coverage proven here

[The index](matrix-index.json) enumerates every matrix file. The complete root manifest reconciles to 135 active non-AI issue keys. The published backlog has 164 leaf records: those active items, 23 later AI/supporting items and six owner-excluded sync items. All 558 acceptance criteria are retained verbatim with stable criterion IDs and explicit Linux, Windows and macOS result slots. [Source-family coverage](source-family-coverage.json) carries every one of the 23 workflow-family rows, 43 source-family keys and 19 historical/optional disposition rows, joined to published issue URLs and criterion IDs.

These are complete counts for the named local authorities, not exhaustive discovery of undocumented reference-product behavior. No criterion has a native PASS. Implementation, test and release states remain separate. Platform-specific native build, release, update and measurement issues retain their declared OS; their other-OS slots are explicit N/A rather than passes. Later AI work is deferred by priority. Sync exclusion is an owner decision. An optional recommendation remains unresolved until its owning issue records approval or deferral.

The tables preserve the source criteria rather than replacing them with packaging checks. [Cross-cutting scenarios](cross-cutting-scenarios.json) add concrete steps and observations for affected surfaces: accessibility, keyboard, IME/localization, touch/pointer, focus targeting, restart, interrupted writes, protected work, asynchronous failure, private/download boundaries and optional states. They are test obligations, not registered native tests or policy approvals. Each later run must join every affected feature to its applicable cases. Unknown applicability stays OPEN.

## Why native execution is blocked

The authoritative [root execution snapshot](/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/arcified/docs/execution-state.json) says Linux baseline compilation is active and preparation is PREPARED_NOT_BUILT. Its [Linux run report](/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/arcified/docs/native-linux-run-20261023.md) leaves compilation completion, launch, restart and native UX open. This is reported snapshot evidence, not a live build/process check.

The release dependencies are [Linux #131](https://github.com/Timpan4/helium-arcified/issues/131), [Windows #132](https://github.com/Timpan4/helium-arcified/issues/132) and [macOS #133](https://github.com/Timpan4/helium-arcified/issues/133). Their authoritative assignment requires declared support, package identity and actual installation evidence. No installed candidate package/executable digest, feature-state manifest, native interaction result or release proof was supplied to this leaf. All applicable results remain OPEN/NOT_RUN.

This assignment forbids build/setup/test execution, remote CI, reference-app execution and browser/visual work. It permits only source inspection and light packet checks. No missing host, touch device, IME or assistive-technology combination is declared unsupported merely because it was unavailable for this task. Minimum OS versions, architectures, Linux display/compositor environments and input/AT combinations are UNDECLARED until release owners provide the support matrix.

## Native ownership and focus gates

The [lifecycle authority](/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/arcified/docs/lifecycle-contract.md) and [native integration design](/home/timpan4/.t3/worktrees/helium-arcified/t3-a4238261/arcified/docs/native-space-integration.md) require these observed production results.

1. Keep the same actual native top-level window across differently profiled Space switches. Observe real Profile services, BrowserContext isolation, command ownership and empty-context behavior. A logical selection, content-only view or hidden backing browser window cannot pass.
2. Bring an already-live saved page to the requesting host with the same actual TabModel/WebContents/controller/document and immutable Profile. Preserve form/history/scroll/current-navigation state, saved destination and protected activity. Cover last-tab source, stale target, failure/cancellation and closing owners. `Model::OpenItem` success is synthetic evidence only.
3. Keep Profile settings/sign-ins/Favorites scoped to the actual Profile, Space themes scoped to Space, saved identities shared and temporary pages local. Preserve canonical lifecycle, metadata/codec and native session joins.
4. Branch only the approved browser-classified outbound saved-page link. The actual native classifier and URL/security/opener rules own classification and navigation. Unknown/same-site/non-link routes gain no new policy or fallback here.
5. Resolve address/reload/find/zoom/security/credential commands against the active page surface and move/keep/archive against the selected workspace object. Test split, Peek, chrome reveal, modal, fullscreen and close transitions with native focus and accessible cues. [Focus contract #44](https://github.com/Timpan4/helium-arcified/issues/44) owns unsettled focus behavior. This packet tests that contract when accepted and does not decide return-focus rules.

The prepared Chromium 155.0.8059.79 interface explicitly documents immutable per-window Profile ownership in `chrome/browser/ui/browser_window/public/browser_window_interface.h`; `browser.h` stores a const Profile pointer. [Source identities](source-identities.json) pin these exact inspected bytes. Those declarations support the ownership proof gate, not a claim that the extraction is implemented or impossible.

Activity/cleanup/destructive policy, populated cross-Profile migration/source disposition, other navigation triggers and unspecified restore/private/guest choices remain with root and the owning issues. Do not change those policies to make a scenario pass. Preserve the shared private/download classifiers, native before-unload/discard checks and protected originals.

## Optional and historical coverage

The matrices identify every active optional capability from the backlog's phase/disposition and retain source-family decisions. Check unavailable/disabled behavior without an account and without silently advertising absent functionality. Enabled, offline, stale, denied and revoked tests require accepted scope, actual implementation and the appropriate authorization. A disabled-state pass cannot certify an unimplemented enabled state.

Keep Notes compatibility separate from a new editor; historical People Cards and service-dependent Favorite previews require visible dispositions. Preserve the owner exclusion of iCloud/new cross-device sync alongside active local import/export. Keep Dia Memory and mobile/work/enterprise references as their recorded later/outside-core or unresolved recommendations. No current external service behavior, scope, polling interval, retention, provider or account policy is inferred.

## Reuse and evidence boundaries

[The authority snapshot](authority-snapshot.json) retains the root's foundation/integration evidence and references the lifecycle v3, sidebar, layout/split/motion and persistence reports inspected here. Their successful pure fixtures are not rerun. They establish only the semantic/component scope those reports name; this packet does not assert that all underlying source bytes or native inputs remain unchanged. No existing installed native UX PASS was supplied to reuse.

For future reuse, retain the original run and compare source/patch order, package/executable digests, feature configuration, OS/architecture, hardware/input/AT, fixture data, scenario/oracle and relevant policy versions. Reuse only when those relevant inputs are unchanged and the original evidence actually covers the result. Otherwise reopen the affected result. A package rebuild does not itself require unrelated unchanged semantic fixtures to rerun, and it does not prove installed UX.

The root's macOS Arc/Dia artifact report supports workflow discovery and static resource observations only. No app runtime, current Windows reference behavior, signature verification, proprietary implementation or performance result is claimed. External URLs are preserved from the root's 2026-10-23 evidence; no new external retrieval was needed or performed.

## Support and runtime evidence required

Every scenario result must record the exact installed candidate and source/patch identity, OS/version/architecture, actual window system and hardware, relevant AT/IME/layout/locale, optional state, steps, expected/observed behavior and durable artifact hashes. Keep data before/after and native object/ownership traces for failure, handoff and restart scenarios. Screenshots alone cannot prove object continuity or exclude temporary hidden windows.

For Linux, Windows and macOS independently, record unsupported combinations only from the approved support declaration or observed technical evidence. Missing hardware/OS execution is UNAVAILABLE with its reason and an open result. No supported OS/architecture, AT, IME or touch combination is fabricated in this packet.

## Integration requests

1. Release coordinator: attach each candidate's installed package/executable hashes, source revision, ordered patch identities, supported OS/architecture/input matrix, install result and optional-feature configuration. Keep package/build/install/UX/release states separate.
2. Feature coordinators: supply accepted scope or visible deferral for every optional/retired family and join every active feature criterion to actual registered native tests and installed interaction artifacts. Reconcile changed evidence against these issue/criterion IDs without replacing source authority.
3. Root: preserve same-window multi-Profile and exact-object handoff as gates. Authorize and run the installed/native/visual scenarios only after suitable candidates and hosts exist. Root owns build/test execution and integrated patch/test registration.
4. Research coordinator: integrate the separately owned focus-routing-contract decision/evidence into these focus scenarios. The probed packet `result.json` and `README.md` paths were absent when inspected; this is not a claim that another primary has done no work.

## Owner questions for coordinator consolidation

1. Which OS versions, architectures, Linux display/compositor environments, assistive technologies, IMEs and touch/input devices define the first release support matrix? Recommend an explicit declared matrix with unavailable combinations kept open. This avoids unsupported-platform guesses while leaving the release support choice with the owner.
2. Which optional-feature candidates are accepted for the first release, and which are visibly deferred in their owning issues? Recommend retaining local core behavior without services and requiring each accepted optional feature's disabled and authorized-enabled/failure evidence. The tradeoff is explicit deferred functionality rather than an unverified parity claim.

Existing lifecycle and provider/service questions stay in their owning issues. This packet sends those references to the coordinator and does not ask the owner or root again.

## Patch and verification disposition

This is a tracking leaf, so a production patch, prepared base/result modification and native test registration are inapplicable. There is no product logic to implement or meaningful browser regression to register here. `source-identities.json` records exact inspected prepared-source bytes and empty change/patch/test arrays. Scenario tables do not masquerade as Chromium tests. No command, ActionItem or message was added. Reserved legacy ID 40796 and prefix `IDS_ARCIFIED_RELEASE_UX_CHECK` remain unused; Library 40701 is preserved.

[Checks](checks.json) record the exact light commands and hosted-JavaScript validation performed. No Markdown/JSON formatter or linter is declared in the inspected repository configuration. JSON uses canonical two-space formatting and the packet receives focused structural, coverage, reference, whitespace and identity checks. No dependencies were installed. No native setup/build/test, benchmark, remote CI, browser/visual execution, shared-control write, patch/series modification, staging or commit occurred.

The source-identity recheck observed a concurrent root-manifest change. A complete fast_filesystem reread confirmed the same 135 active issue keys/URLs, scope, baseline and command allocations. All other inspected source identities, including every prepared-source file, stayed unchanged. Both manifest hash observations are retained; no claim of a frozen live control file is made.

All four acceptance criteria in issue #161 remain open for actual native observation and accepted scope. This packet is ready for independent coordinator review as documentary coverage evidence.
