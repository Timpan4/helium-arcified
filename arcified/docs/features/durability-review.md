# Durability component review

## Schema admission, round 1

Independent read-only T3 task: `durability-migration-review-round1`, using the task prefix in [workstream status](durability-workstream-status.md). Assignment verified as `codex/gpt-6.1-sol`, reasoning effort `high`, service tier `default`.

Result: clean review, no actionable findings in the accepted local #99 slice. The reviewer read all migration files against the issue brief and backlog, including shared codec/write protocol and lifecycle dependencies. It independently passed the documented strict C++17 compile and fixtures, ASan/UBSan execution and Chromium formatting.

The review found exact-byte preservation, unsupported-version refusal in either slot, explicit recovery choices, stale-source rejection and admission forwarding consistent with the accepted boundary. This is component evidence. Historical migration, native privacy binding, filesystem durability, recovery UI/accessibility and installed upgrades remain open.

Reviewed Git blob hashes, unchanged before and after review:

| Path | Blob |
| --- | --- |
| `native/features/migrations/schema_gate.h` | `c2bf17e3bdd13f5a341bc4ea34b2e4a4ba08477e` |
| `native/features/migrations/schema_gate.cc` | `2de10d80b0feb4e3d8f090a4ce013fd115c99ada` |
| `native/features/migrations/schema_gate_test.cc` | `e6e5f7b962e3acc70808fb47016f931996484c37` |
| `docs/features/schema-migrations.md` | `12ad52e1a932ab97429365219161b6d650077f28` |
| `native/features/durability/session_snapshot.h` | `86e66a9fe9b9fe5b7ca7a9cfc8680f970ae8d43c` |
| `native/features/durability/session_snapshot.cc` | `fbddd53fce54a40fbed1ebc39c93afa2bee6fc3b` |
| `native/model/lifecycle.h` | `d3a7aed6f1caddd40f5422bc6e5c1f5019636338` |
| `native/model/lifecycle.cc` | `7ffe87eb49d0cf4a92f83058b7c348ed3882012f` |
| `docs/research/arc-dia/publication/schema-migrations.md` | `6e08db63e881545f642d06504fff679381dc0c86` |
| `docs/research/arc-dia/backlog.json` | `16ee4ef346cb48be580510fe37b74c65baefcbe2` |

Paths in this table are relative to `arcified/`.

## Snapshot and recovery, round 1

Independent read-only T3 task `durability-snapshot-review-round1` completed clean with no actionable findings. Assignment verified as GPT-6.1 High/default. The reviewer checked admission, lifecycle graph validation, parser bounds/tags/EOF, native navigation evidence, exact-byte write transitions and honest recovery claims.

It independently passed strict C++17 fixtures, ASan/UBSan and Chromium formatting. Additional review-only checksum-valid malformed inputs covered body truncation, trailing data, oversized string length, invalid tag and invalid graph. No repository edits occurred.

| Path relative to `arcified/` | Reviewed blob |
| --- | --- |
| `native/features/durability/session_snapshot.h` | `225a8b9a72459049424e0a9b5d323fde4150cdca` |
| `native/features/durability/session_snapshot.cc` | `80fb6abc8e259866f82f17069a265d1bc159889d` |
| `native/features/durability/session_snapshot_test.cc` | `ede8b26193d6f8a409d080d2036362f7ccc55a84` |
| `docs/features/session-durability.md` | `a2c6c99235c3abe47badfac91f5a334884cd1466` |

The final snapshot review pinned different dependency hashes from the earlier migration review. Migration's own reviewed files are unchanged. Its primary reran the integrated strict fixtures and sanitizers against the final snapshot API without changes; the final snapshot itself has the separate clean review above.

## Window undo, round 1

Independent read-only T3 task `durability-undo-review-round1` completed clean with no actionable findings. Assignment verified as GPT-6.1 High/default. It covered exact capture membership/order/selection, privacy admission before retaining lookup keys, close confirmation/cancellation, fresh reopen IDs, owner and saved-live conflicts, stale attempts, exact restoration confirmation, targeted consumption and split refusal. No alias-lifetime defect was found under the documented single-writer and non-reused-ID contracts.

It independently passed all seven strict C++17 fixture groups, ASan/UBSan without diagnostics, Chromium formatting and whitespace checks for all component files. Source assertions were compared with the local reference ledger; this review did not reinspect remote Chromium source.

| Path relative to `arcified/` | Reviewed blob |
| --- | --- |
| `native/features/window_restore/window_restore.h` | `ed8eef282ac4aaef23ee2ec7b3a0374e9e87a836` |
| `native/features/window_restore/window_restore.cc` | `da989488e19cef82cf271eb3407bbee980f5a0a6` |
| `native/features/window_restore/window_restore_test.cc` | `d0663a1a6d63a81ac3be3d8a105cb8f069f734c9` |
| `docs/features/undo-windows.md` | `a238d17873e5aff6ba04120bf62e62792c05c262` |

All review inputs were unchanged before and after their respective reviews. Shared lifecycle code, acceptance briefs, backlog and pinned design/source ledgers were also read and hash-pinned by the reviewers. All three reviews are local component evidence. None proves native privacy binding, filesystem durability, installed browser behavior or issue closure.
