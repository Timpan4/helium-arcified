<!-- SPDX-License-Identifier: GPL-3.0-only -->

# Windows reference artifact research

Issue [31](https://github.com/Timpan4/helium-arcified/issues/31), baseline `a4b6b480d3de07857ab6e939149254a8359e5449`. Retrieved October 23, 2026 UTC, October 24 locally.

Research is partially complete. Official acquisition routes, original descriptors and small installer bootstrapper identities are recorded. Complete Windows browser MSIX packages were not acquired. Package hashes, signature verification, archive contents, installation and runtime acceptance remain open.

## Official acquisition evidence

The [Arc download page](https://arc.net/download) links its Windows bootstrapper. Dia's [Windows page](https://www.diabrowser.com/windows) serializes its bootstrapper URL in the site's download configuration. Those publisher links yielded the local files below. Both SHA-256 values were recorded before REA inspection.

| Local bootstrapper | Bytes | Assembly version | SHA-256 |
| --- | ---: | --- | --- |
| ArcInstaller.exe | 2002840 | 1.20.0.0 | `971bc17eae07151311a9ef3fab3c9bb67cdbeb4702ceb6eb0285b4d0b9a10b16` |
| DiaInstaller.exe | 2011032 | 1.30.0.0 | `b868db670fe0c179f63c991602d4e187824bc656af17bc6684bca35df3355e3d` |

REA identifies PE32 machine `0x014c` with IL-only managed AnyCPU classification and .NET Framework 4.8.1 target metadata in each bootstrapper. These are installer versions, not browser versions or browser architecture evidence. Versions were established during static identity inspection, not known before that inspection. Native UTF-16 string inspection separately found file/product version literals and the revision suffixes retained in [acquisition.json](acquisition.json).

The Arc bootstrapper contains the official Windows release base and descriptor filenames. Combining these literals produced candidate URLs that were then fetched successfully. The Dia bootstrapper contains complete descriptor URLs. This establishes static leads and successful retrieval, not execution of either installer's routing logic.

| Browser | Architecture | Official descriptor MainPackage version | MSIX HEAD reported bytes | Package SHA-256 |
| --- | --- | --- | ---: | --- |
| Arc | x64 | 1.129.0.288 | 363558003 | Unknown |
| Arc | ARM64 | 1.129.0.288 | 348172636 | Unknown |
| Dia | x64 | 1.3.1.2 | 894261660 | Unknown |
| Dia | ARM64 | 1.3.1.2 | 865976532 | Unknown |

Each versioned URL returned HTTP 200 to a body-free HEAD request. Full URLs, observed headers, descriptor hashes, package names, publisher claims and dependency descriptors are in [acquisition.json](acquisition.json). All original `.appinstaller` descriptors are retained with CRLF bytes unchanged. Their hashes identify the descriptors alone. They contain no MSIX SHA-256. Content-Length and ETag cannot substitute for it.

The full packages are large, so downloading them would exceed this task's explicit restriction. No byte threshold was invented. No full MSIX content was inspected. Before any later content analysis, acquire the chosen package, record its exact URL, manifest version, architecture, type and whole-file SHA-256, then retain signature and analysis results separately. Re-read mutable descriptors before acquisition; never label future bytes with this snapshot's version by assumption.

Dia's [official changelog](https://www.diabrowser.com/changelog) labels its October 22 Windows release v1.3.0. The fetched Windows descriptor says 1.3.1.2. Neither label is replaced with the Mac v1.54.0 changelog entry or the prior macOS DMG identity.

## Static inspection, signature and extraction limits

[rea-evidence.json](rea-evidence.json) retains the returned REA 6.7.0 records, exact source identities, module MVIDs and evidence IDs. Managed inspection parsed metadata without loading the assemblies, resolving CLR dependencies, decompiling or executing code. PE resource inspection examined 12 directory entries and 4 resources for Arc's bootstrapper, and 38 entries and 17 resources for Dia's. Resource coverage applies to those files only. Icon references do not prove shell appearance, UI loading or browser feature coverage. Opaque payloads remain opaque.

Signature state is **unverified** for every bootstrapper, descriptor and target MSIX. REA's advertised `inspect_signature` route targets Mach-O/macOS, not Authenticode. The available Linux host has no discovered `osslsigncode`; no tool was installed. Managed strong-name absence is not an unsigned-Authenticode conclusion. Descriptor Publisher strings are unverified claims. No chain, timestamp, revocation, MSIX deployment trust or successful installation is proven.

REA supports the archive route `open_binary` then `inspect_artifact` for MSIX/AppX. It was not run because no complete MSIX input exists here. Use its returned root digest and coverage before contained PE/resource analysis. `extract_artifact` chooses a fresh temporary directory outside this packet; a later task must authorize that evidence location before extraction. No native disassembler target was opened and no REA extraction ran. No installer was launched.

## Platform and scenario states

The existing foundation [artifacts.md](../../../docs/research/arc-dia/artifacts.md) records macOS DMGs and static bundle/resource inspection. Its hashes and observations do not identify Windows packages or establish runtime behavior. It was reviewed and not repeated.

| Platform evidence | State |
| --- | --- |
| Windows x64 and ARM64 | Official package descriptors and bootstrapper static evidence only. Complete package acquisition, integrity, signatures and runtime remain open for both browsers. |
| macOS | Prior documentary/static foundation only. No new acquisition or runtime result here. |
| Linux | Research host only. No official Linux Arc/Dia reference artifact is established by the reviewed sources. Arcified Linux acceptance remains a separate native gate. |

Every Windows reference interaction below remains **NOT_RUN** for Arc and Dia. No screenshots, recordings or account operations were authorized.

| Reference scenario | Missing evidence |
| --- | --- |
| Peek and pinned URL rules | Installed reference build and observed navigation/return behavior |
| Tab handoff | Actual page/document identity and window activation |
| Folder preview | Reachability and platform-specific behavior |
| Split transitions | Native interaction and continuity |
| Sidebar reveal | Actual hover, focus and keyboard behavior |
| Media return | Real PiP/media activation destination |
| Native dialogs | Focus, ownership, cancellation and return target |

The [Arc desktop support article](https://resources.arc.net/hc/en-us/articles/19337669022103-Arc-for-Desktop-macOS-and-Windows) says new accounts ended October 22, 2026, and existing accounts can use Arc until December 22, 2026. This is a runtime prerequisite, not account access authorization. No signup, login or credentials were requested.

Both acquired bootstrapper files contain a Windows 11 requirement message. Arc's download page advertises Windows 10 19H1 or higher. Strings may be stale, and no installer was executed, so Windows 10 support remains unresolved. No minimum build or dependency policy is inferred from the message.

## Coordinator decisions and integration requests

1. Consolidate this owner question: "Which supported Windows test host and architecture will be used, and may the coordinator acquire its matching official full MSIX package and declared dependencies?" Recommend matching the designated host first. Acquiring both architectures expands static coverage but requires the additional large downloads. Keep every unacquired tuple open.
2. Consolidate this owner question: "Is an existing Arc account authorized and available for the reference scenarios before the published sunset, or should Arc runtime remain unavailable?" Recommend documentary/static-only Arc evidence if no authorized existing account exists. Dia cannot substitute for Arc runtime proof.
3. Send the four package tuples and their open identity gates to `runtime-reference`. A later authorized Windows task must check actual OS support, signatures and installation before feature observations. Leave the Windows 10 claim unresolved until tested or clarified by the publisher.
4. Keep large original executables and HTML as local ignored inputs. Commit reviewable authored evidence and original descriptors through the coordinator's normal review. This packet performs no staging, commit, publication, issue mutation or PR operation.

No production native patch, prepared-source base/result pair, resource message, legacy command, ActionItem mapping or GN/native test registration applies to package acquisition research. Adding a fake native service or synthetic success path would not satisfy this issue. Assigned command 40792 is unused and Library 40701 is unchanged. Same-window multi-Profile operation and saved-live-page handoff remain open under the root lifecycle/native integration authorities. No prepared Chromium source was modified or built.

[authority.json](authority.json) preserves the full assigned issue and hashes of the read authority files. [checks.json](checks.json) records focused verification. Authored JSON is canonically formatted; Markdown receives local whitespace/link checks. No repository Markdown formatter/linter is declared in the inspected configuration, so no package was added to run one. Original third-party descriptors are syntax-checked without reformatting their bytes.

Native acceptance: **OPEN**.
