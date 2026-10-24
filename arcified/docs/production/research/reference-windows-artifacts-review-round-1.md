<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Windows reference artifacts review, round 1

No actionable findings in the bounded partial research packet. This verdict accepts the evidence and its stated limits. It does not close [issue 31](https://github.com/Timpan4/helium-arcified/issues/31), complete package acquisition, or accept native behavior. Native acceptance remains **OPEN**.

Reviewed on October 24, 2026, against baseline `a4b6b480d3de07857ab6e939149254a8359e5449`. Packet paths below are relative to `arcified/native/production/reference-windows-artifacts/`.

## Scope and identity

Read the full assigned issue object and root control manifest from the coordinator-specified `t3-a4238261` checkout using fast_filesystem. Compared the packet's copied issue object with that assignment. They match exactly. Read the applicable lifecycle contract, native integration authority, execution state, root/subtree instructions, and existing macOS foundation artifact report. This acquisition leaf does not require a native patch, prepared-source base/result pair, message, ActionItem, legacy command, or native test registration. The packet correctly leaves those inapplicable and preserves the root's same-window multi-Profile and same-live-page handoff gates.

Recomputed SHA-256 for every one of the 17 packet files supplied in the review brief. All matched the coordinator's exact source-byte identities. In particular, the reviewed `result.json` is `0fb5fa41e39cae88fdda52d739f82a9edbdd65566cda7fbc09439244a0256ce8`, `acquisition.json` is `09aff5eb357bb1d329c74c982f9aaac99de09420edf9e48e703285e63d3ccebb`, and `rea-evidence.json` is `5d8864ef2a1611f8924d162709aac1d4e8be34d00aec9eda2875189b736d0510`. This review applies to those bytes.

## Acceptance mapping

| Assignment criterion | Review result and remaining gate |
| --- | --- |
| Record official URL, exact version/build, type, architecture and SHA-256 before inspection | Correctly OPEN in `result.json:67`. Both bootstrappers have independent whole-file hashes and explicitly scoped assembly versions. All four descriptors agree with the recorded MainPackage name, version, architecture, publisher claim and MSIX URL. Arc is `1.129.0.288`, Dia is `1.3.1.2`, each for x64 and ARM64. Full MSIX acquisition and hashes remain absent. Descriptor hashes, bootstrapper versions and HEAD metadata do not establish browser identity. The packet explicitly admits bootstrapper versions were learned during identity inspection. No complete-before-inspection claim is accepted. |
| Use supported REA archive/PE/resource routes and separate signature, extraction and runtime limits | Correctly PARTIAL in `result.json:72`. Independent static managed and PE-resource inspection reproduced all four saved evidence IDs and their stated coverage. No full archive exists here. Archive inventory/extraction, Authenticode/MSIX trust, installation and runtime remain open. |
| Do not reuse macOS hashes or run installers without an authorized supported environment | Satisfied within the reviewed packet, as scoped in `result.json:77`. Windows input hashes are independent of the macOS foundation identities. The evidence contains static parsing, not installer or app execution. This review ran no target code and does not independently prove historical process activity. |

The missing full acquisition and runtime evidence are explicit limits, not defects in this partial delivery. The result must remain `RESEARCH_PARTIAL` with native acceptance OPEN.

## Provenance and static evidence

The retained Arc HTML prefix contains the exact official bootstrapper URL. The complete retained Dia Windows HTML contains its exact bootstrapper URL in serialized configuration. Independent primary-source retrieval on the review date confirmed the [Arc download page](https://arc.net/download), [Dia Windows page](https://www.diabrowser.com/windows), and [Dia changelog](https://www.diabrowser.com/changelog). The changelog's Windows v1.3.0 label remains distinct from descriptor version 1.3.1.2 and the Mac release label. Arc HTML reads were truncated, so this is a specific-link observation, not complete HTML coverage.

Independent UTF-16 string inspection corroborated the Arc release base and descriptor filenames, Dia's complete descriptor URLs, both product-version revision literals, and both Windows 11 requirement messages. The packet correctly identifies Arc URL assembly as an analyst inference followed by retrieval. Strings do not prove installer routing or OS support. Arc's download page still advertises Windows 10 19H1 or higher, so the packet's unresolved Windows 10 status is appropriate.

Independent `inspect_managed_artifact` calls with `detail: "summary"` reproduced `ev_710f7267bb0cd7a0a615bc1821345586589758e7f0994dc1a05044b197bc0476` for Arc and `ev_ea254f31ef15cb74f299f3e8d14889d0fab29f3837029c45135a6da6d5dcc4ef` for Dia. Hash, size, assembly version, MVID, PE32 machine, AnyCPU classification and .NET Framework target agree with the packet. These summaries are not complete managed parent collections.

Independent `inspect_pe_resources` calls reproduced `ev_b26257a1ff395a7ab546c12bdb6861ff524f49c8817732b53fca1a0cf41cf254` and `ev_eadfcf0efed68dae58198157d0d4efa91d63240886911c5cf3dba8cc46ecf38a`. Arc coverage is 12 directory entries and 4 resources. Dia coverage is 38 entries and 17 resources. Opaque payloads and static icon relationships do not establish displayed UI or browser features.

The advertised signature tool targets Mach-O, and the extraction tool chooses a fresh temporary directory. Their documented limits agree with `README.md:39` and `README.md:41`. Publisher strings and absent managed strong names are correctly kept separate from Windows signature validity. No archive, extraction or signature route was run during this review.

Independent retrieval of the account/sunset article cited at `README.md:65` failed. The web tool reported the URL inaccessible, and curl returned HTTP 403, exit 22. The related official sunset URL was also inaccessible through the web tool. The exact dates and account restrictions therefore remain the primary's recorded publisher observation, not independently corroborated facts in this review. No contrary account availability or access authorization is inferred. Recorded MSIX HEAD results were inspected, not repeated; they prove only their historical route/size observations.

## Retention, licensing and verification

The four original descriptors retain CRLF bytes and publisher ownership. `acquisition.json:250` explicitly restricts GPL-3.0-only to newly authored evidence rather than relicensing publisher payloads. The reviewed authored evidence contains metadata and limited literals, not proprietary implementation or full published articles. Git ignore checks confirmed both EXEs and both HTML inputs remain ignored local originals. The scoped status lists the authored evidence and descriptors as untracked. Reproducibility after publication requires reacquisition and hash comparison for ignored inputs; the packet does not promise those original bytes travel with Git.

Independent light checks passed:

1. In the packet directory, `sha256sum --check source-bytes.sha256` passed for all eight retained acquisition inputs. A separate `sha256sum` invocation over all 17 named packet files matched the review brief.
2. `xmllint --nonet --noout Arc.x64.appinstaller Arc.arm64.appinstaller Dia.x64.appinstaller Dia.arm64.appinstaller` exited 0. This checks XML syntax, not schema or trust.
3. Complete fast_filesystem readback followed by JSON parse, canonical `JSON.stringify(value, null, 2) + "\n"` comparison, and trailing-whitespace checks passed for all six packet JSON files. The full copied issue object matches the authoritative assignment.
4. `git check-ignore` passed for both EXEs and both HTML inputs. `git status --short --untracked-files=all --` scoped to the packet and this report confirmed the retention boundary before this report was added.
5. `strings --encoding=l ArcInstaller.exe DiaInstaller.exe | rg '1\.20\.0|1\.30\.0|https://releases\.(arc.net|diabrowser.com)|Arc\.appinstaller|Arc\.arm64\.appinstaller|Windows 11'` exited 0 and corroborated the bounded literals described above.
6. The four explicit-path REA calls described above reproduced the saved evidence identities without executing the assemblies or opening a native disassembler target.

No full package download, runtime/visual interaction, account action, prepared-source operation, build, native test, benchmark, CI, remote write, staging or commit ran. Only this review report was written. Report readback and Git's new-file whitespace check cover the authored review text.

## Remaining gates and coordinator questions

All four browser/architecture tuples remain open for whole-package/dependency acquisition, hashes, archive content and signing/deployment trust. A supported authorized Windows host must establish actual OS compatibility and installability before runtime-reference can observe Peek/pinned URL rules, tab handoff, folder preview, split transitions, sidebar reveal, media return and native dialogs. Every listed Windows interaction remains NOT_RUN for both products. Neither macOS evidence nor the Linux research host closes these gates.

The packet's owner questions are complete for this bounded acquisition handoff. Send them through the coordinator without requesting credentials or assuming an account exists:

1. "Which supported Windows test host and architecture will be used, and may the coordinator acquire its matching official full MSIX package and declared dependencies?" Recommend matching the designated host first. Additional architectures require additional large downloads and remain OPEN until scoped. The follow-up also needs an allowed extraction evidence location.
2. "Is an existing Arc account authorized and available for reference scenarios before the published sunset, or should Arc runtime remain unavailable?" Recommend documentary/static-only evidence when no authorized existing account is available. Reconfirm the publisher's current access dates before scheduling runtime work. Dia cannot supply Arc runtime proof.

No fixes requested for this round. Keep issue 31 and native acceptance OPEN while these gates remain unmet.
