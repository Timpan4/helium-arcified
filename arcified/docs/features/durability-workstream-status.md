# Durability workstream status

Base: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`. Branch: `feat/durability-workstream`.

All three local components passed focused checks and clean independent reviews. They are ready for root integration. None has native acceptance; all three issues remain open.

| Issue | T3 task ID suffix | Ownership | Stage |
| --- | --- | --- | --- |
| #50 session-durability | `durability-session-durability-primary-v1` | `native/features/durability/`, `docs/features/session-durability.md` | Local component reviewed, native acceptance open |
| #99 schema-migrations | `durability-schema-migrations-primary-v1` | `native/features/migrations/`, `docs/features/schema-migrations.md` | Local component reviewed, native acceptance open |
| #70 undo-windows | `durability-undo-windows-primary-v1` | `native/features/window_restore/`, `docs/features/undo-windows.md` | Local component reviewed, native acceptance open |

All assignments verified provider `codex`, model `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`. Full task IDs have prefix `node:delegated-task:command%3Amcp%3Acebfe883-2446-46e1-a2dc-7633001cec65%3Adelegate-task%3A` followed by the suffix above. Children shared this checkout with disjoint files. The coordinator owns commits after independent review.

## Accepted boundaries

Lifecycle Model v3 remains the ownership and navigation authority. Durability owns schema 1, separate from semantic contract 3. Its snapshot uses lifecycle identities. It excludes renderer memory, history, forms, activity and credentials. Native committed navigation reports are required to stage a restored Model.

Durability owns exact original/candidate byte images and staged write ordering. Recovery never promotes a candidate automatically. Migrations consumes that format and protocol, with only the actual current format admitted. No historical format or filesystem atomicity is claimed.

Undo records references to existing Space/item identities and process-local native restore handles. Actual native close/reopen success must be reported before history changes. It neither authorizes closure nor duplicates shared saved items.

Capture requires explicit native eligibility for every Profile/page, including window eligibility for undo. Root must bind these boundaries to the single profiles area's private-context classifier. No runtime context identity is serialized.

Profiles reports clean independent review and commit `cd5ef303` on `feat/profiles-workstream` for its header-only context classifier and Profile preparation API. Their exact APIs are unchanged. Root integrates that dependency; this worktree has no native binding or isolation proof.

Workspace owns Space display metadata and order. Splits owns orientation/weight metadata and reconciliation against canonical membership. Schema 1 omits those payloads and requires explicit evidence that unsupported feature payloads are absent. No selected-page or focus registry is introduced.

Workspace reports reviewed Space metadata commit `4bea343d1f4b50d2f498567800ba27fa0d3a6231` on `feat/workspace-workstream`, with the agreed DTO unchanged. Root integrates it. This schema still refuses persistence when omitted payloads are present.

## Blockers and next actions

All primaries and independent reviews completed without pending child work. The [review record](durability-review.md) pins all three clean reviews and exact component inputs. Review task suffixes are `durability-migration-review-round1`, `durability-snapshot-review-round1` and `durability-undo-review-round1`, using the same task prefix and verified GPT-6.1 High/default settings. #99 also reran its integrated fixtures and sanitizers against #50's final API without changes.

Local component work is complete and awaiting root integration. The coordinator returns the local commit SHA in its handoff. Root must integrate shared classifier binding and canonical metadata, then continue native acceptance when builds and runtime checks are authorized.

Native acceptance remains open: real native ownership extraction and same-window multi-Profile hosting, ID/native event joins, filesystem flush/replacement/recovery, interrupted writes and crashes, installed upgrades, closed-window restoration, private context enforcement, names/order and split/pane metadata, native UI/accessibility, Windows/macOS execution. Chromium preparation/build, heavy downloads, benchmarks, global installs and remote CI are deferred.

No pushes, PRs, issue updates or merges are authorized.
