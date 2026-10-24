<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Compatibility independent review, round 1

Reviewed 2026-10-24 for [issue 59](https://github.com/Timpan4/helium-arcified/issues/59), at coordinator baseline `a4b6b480d3de07857ab6e939149254a8359e5449`.

Disposition: **Clean bounded documentary review. No actionable findings.** The packet is ready for research aggregation. Issue completion and native acceptance remain **OPEN**. This review does not declare a release candidate, approve a fallback policy or verify native behavior.

## Reviewed scope and identities

I reviewed all nine files in `arcified/native/production/compatibility/`, the complete assigned compatibility issue object, root workstream controls, lifecycle contract v3, native Space integration authority, execution state, the existing compatibility publication and Arc/Dia artifact report. I inspected the cited prepared-source seams and fork flags. The prepared tree was read only.

All nine packet SHA-256 identities matched the review request:

| Packet file | SHA-256 |
| --- | --- |
| `README.md` | `c16f710c90f7d1689b3da286c8cf59ca0855fb8e1049f7ece6641829b72fcc50` |
| `checks.json` | `4e0dbfbfc33125175ddf6630cb664144003e7c4bb51205ca46e2e229d32ac370` |
| `compatibility-matrix.json` | `3d5e9df226bae8dcf0d6ffc78d879744d33cde3840a71f0e761b3fa480bb6e86` |
| `defects.json` | `f43c6f82e90a2e9ea55d21a63db95e17bc9b80d5065ca0c5a46973cbc8337390` |
| `issue-object.json` | `f24c024fc5cd63950aa6f422168bfc8b5060e38ff096e776812c55dfc5a9d277` |
| `ownership.json` | `7af44b77ccca73671a30b092f7fe4fb78ee3d8ac78da2c1640c75e4fc4f89dd3` |
| `prepared-source.sha256` | `70d8e9ee6c264541aac412f75bfe51f2a1322c35e79fa1517681cbd8ede9866d` |
| `result.json` | `ccc6ddc189019f56fd14a05eea93ac9f1b282c848e32a89a1bd236434f0f0203` |
| `source-identities.json` | `86a152d6defc938d28216214be936eb388c6535bb4f3ead8409c2a74a39e5b74` |

The copied issue object equals the authoritative assignment object, including all acceptance criteria, dependencies, reference limitations and command allocation. The current root manifest still assigns this tracking leaf to research, reserves 40728, keeps the prepared tree read only and gives root integration/build ownership.

Provenance limit: the live `non-ai-production-workstreams.json` hash at review was `daf98f1cf993c1a21686289a88489bf001266d7fff4d85089d30055584c8a7b0`, rather than historical A02 `4f9c0d697e2c391ab190f42367d9b87f359578d4dfe8f70a49cd05b2cca3fe64`. I read its current relevant controls and compatibility assignment; they agree with this packet. I did not reconstruct the earlier whole-manifest bytes. The other seven authority/foundation hashes and all fourteen prepared-source hashes matched. This is a bounded historical-identity limitation, not evidence of a changed compatibility requirement or a reason to rewrite shared controls.

## Assessment

The 20 scenarios cover every named workflow in the three acceptance criteria. They distinguish PWA install/launch/uninstall, extension popups, address/password autofill and passkeys, preview/system print dialogs, page/extension downloads, document/worker notification clicks, inbound/outbound protocols, casting, video/document PiP return and absent/optional Widevine. Each has Linux, Windows and macOS cells. All 60 cells remain OPEN with empty runtime evidence and defect lists. The undeclared RC and absent installed artifacts are explicit.

The packet requires exact RC/package/source/configuration identities, requesting and receiving page/Profile/host observations, pending-context changes, cancellation/failure evidence and applicable private/handoff variants. Its separate same-native-host gate cannot be satisfied by competing windows. The README and result retain root's same actual window and actual live saved-page handoff requirements. Pure model success is explicitly insufficient.

The cited primary source supports the documentary findings:

- S02 uses Profile-specific activation-ordered tabbed-browser lookup and app-specific protocol translation. The README correctly limits this evidence; it does not claim the file proves installation, uninstallation or generic inbound selection.
- S03 retains browser/Profile/tab-strip ownership and refuses extension popup display over security dialogs. S04 checks contents/frame focus. S05 derives WebAuthn Profile from the requesting RenderFrameHost and records OTR state. These are seams to exercise, not native passes or a promised credential provider.
- S06 retains weak initiator ownership and handles initiator loss during preview creation. S07 permits extension downloads without associated WebContents, permits missing Linux dialog construction and uses an any-Profile last-active parent fallback only on Linux/Windows. Candidate D1 states this source risk accurately and does not claim a reproduced leak, crash or UI failure.
- S08's missing-document fallback navigates with the supplied Profile. S09 checks notification permission before persistent click dispatch. S10 retains initiating contents, modal-manager checks and the normal external-protocol security path. The packet leaves Arcified fallback host/Space policy with the focus-routing owner.
- S11 checks feature and BrowserContext policy state; it does not prove discovery or a usable receiver. S12 delegates PiP return/focus to its controller. The packet keeps actual recipient and stale-initiator behavior unverified.
- S13 separates Widevine support, library/platform availability, component deployment and bundling. S14 explicitly requires a separate Google license for CDM use/distribution. Fork flags request support while disabling mDNS/service discovery. The packet correctly avoids inferring CDM presence, lawful provisioning, codec/service entitlement or working Cast from those settings.

No new native capability, fake host, parallel Model/store/codec, private/download classifier, legacy command or message is justified for this tracking packet. Null patch/result/test-registration fields are appropriate. Its feature-owner integration requests retain real native regression ownership. It preserves upstream source and license boundaries, prohibits proprietary CDM bundling and retains security prompts.

The owner questions are appropriately unsettled. Q1 requests actual RC identities rather than inventing an RC. Q2 routes unknown host/Space fallback to #44 while retaining known immutable Profile identity. Q3 requests an explicit optional Cast/Widevine claim and lawful authorized test configuration. None is presented as owner approval, and none reopens the already approved lifecycle behavior.

## Independent light verification

These checks ran in the coordinator worktree and passed within their stated scope:

1. `git rev-parse HEAD` returned the assigned baseline above.
2. `sha256sum` over all nine packet files matched the review-input identities. `sha256sum --check arcified/native/production/compatibility/prepared-source.sha256` returned exit 0 with all fourteen prepared files OK.
3. `sha256sum` over all eight A01-A08 inputs matched seven recorded hashes and identified only the live-manifest drift described above.
4. `fast_filesystem.fs_read_many`/`fs_read_file` supplied packet, authority, foundation and scoped prepared-source text. Hosted JavaScript validation independently confirmed nine LF-terminated text files without CR/NUL/trailing whitespace, seven canonical JSON files, complete issue-object equality, unique scenario IDs, valid source/variant references, all three criteria represented, all three platform keys on every scenario and all sixty cells OPEN. Every result evidence path resolves within the packet.
5. Manual comparison of the complete scenario inventory and owner questions against the assigned criteria and lifecycle/native authority found no documentary omission requiring a correction. Source-line checks covered the cited README seams, including additional WebAuthn, print and protocol ranges.

The coordinator's supplied conflict-marker check is reused; no native compile, build, syntax target, test, benchmark, CI, reference-app execution or browser/visual interaction ran. I did not modify the packet, root controls, prepared source, patches or Git history. This review is the only authored file.

## Remaining gates

These are explicit limits of the research result, not findings against this packet:

1. Root must supply a declared RC and exact per-OS installed artifacts. All 60 native result cells and all three acceptance criteria remain OPEN. Source inspection and Linux baseline compilation cannot close them.
2. Site-controls #38 and window-chrome #35 still need native completion/evidence. Root's same-native-window multi-Profile isolation and live saved-page handoff proofs remain prerequisites for affected variants, followed by actual workflow observations on each OS.
3. Coordinator must consolidate Q2 with focus-routing-contract #44. Recommended question: "When an incoming OS link has no originating page, or a notification/PiP initiator no longer exists, which surviving host/Space should receive it?" Retain known originating/registered Profile identity; keep unsettled routes unavailable pending disposition.
4. Coordinator must obtain Q1's RC identities and Q3's optional Cast receiver/media and lawful Widevine test configuration, if any. Unsupported or unavailable combinations require evidence and visible disposition. Package-level no-CDM-bundling and unchanged security-prompt behavior remain unverified.
5. Downloads/window-chrome must assess D1 with the actual extension-without-WebContents route and another Profile most recently active. File a separate reproducible defect if native evidence establishes a failure; do not promote the current candidate to a runtime defect.

No remote issue, PR, account or live-host action was taken. Native acceptance remains **OPEN**.
