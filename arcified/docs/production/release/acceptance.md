# Release production acceptance

This coordinator owns the seven release leaves assigned by the root-local `production/release/assignment.json`. It does not own publication, the integrated build tree, final quilt registration or native acceptance. The baseline is `a4b6b480d3de07857ab6e939149254a8359e5449`. Its 51 reviewed local foundations do not establish native release readiness.

| Issue | Packet | Acceptance still requiring root evidence |
| --- | --- | --- |
| #131 release-linux | `arcified/native/production/release-linux/` | Approved package format and fork identity; clean install, upgrade, uninstall data choices; provenance and native smoke checks. |
| #132 release-windows | `arcified/native/production/release-windows/` | Authorized signing and signature verification; install, default browser registration, upgrade, retained Profile data; explicit rollback and security failure behavior. |
| #133 release-macos | `arcified/native/production/release-macos/` | Authorized signing and notarization; installed signature checks; install, protocol registration, upgrade, retained data; architecture and OS support. |
| #41 engine-updates | `arcified/native/production/engine-updates/` | Current authoritative engine/security information; an executed declared update with conflicts, refresh steps and native results on each target OS; release provenance. |
| #165 update-linux | `arcified/native/production/update-linux/` | Approved channel, service and signing ownership; artifact authentication and downgrade handling; interrupted update, restart, approved rollback and retained data. |
| #166 update-windows | `arcified/native/production/update-windows/` | Approved channel, service and signing ownership; artifact authentication and downgrade handling; interrupted update, restart, approved rollback and retained data. |
| #167 update-macos | `arcified/native/production/update-macos/` | Approved channel, service and signing ownership; artifact authentication and downgrade handling; interrupted update, restart, approved rollback and retained data. |

All rows are OPEN. Packet implementation, sparse applicability checks and independent source review are separate evidence from compiled code, installed behavior and approved release artifacts.

The root authorities leave release identity, signing and channel ownership undecided. The coordinator reported these gates to root. No package format, provider, rollback, cleanup or data disposition policy is selected by this document. Source changes must preserve unavailable unsupported paths until their prerequisites are satisfied.

The prepared Linux Chromium `155.0.8059.79` tree is read-only for this coordinator and its children. Root alone owns heavy compilation, native tests, runtime and visual acceptance. Windows and macOS SDK, runtime, signing and installer acceptance remain OPEN. Local work is restricted to disjoint issue packets, light source checks, independent review and reviewed local commits.
