# Portability workstream status

Base checkpoint: `c9748ec8e6d64524c1de9d943dd6f2cfacb319fe`.
Branch: `feat/portability-workstream`.

## Current state

Three primaries completed through T3 with verified provider `codex`,
model `gpt-6.1-sol`, `reasoningEffort=high`, `serviceTier=default`.
All three local components are independently reviewed and ready for root
integration. Native acceptance remains open for all three issues. Ordinary
linked suites pass; import and migration sanitizer suites pass. The composed
converter/codec/planner repeat-and-conflict check, Chromium formatter, Bash
syntax, staged whitespace and real extension fixture-shape check pass.

| Issue | Task ID | Exclusive code ownership | Stage |
| --- | --- | --- | --- |
| #73 export | `node:delegated-task:command%3Amcp%3Abbd47251-8a3b-4733-a985-4c7196ad668e%3Adelegate-task%3Aportability-export-primary-v1` | `native/features/portability/` | Reviewed local component |
| #136 workspace-import | `node:delegated-task:command%3Amcp%3Abbd47251-8a3b-4733-a985-4c7196ad668e%3Adelegate-task%3Aportability-workspace-import-primary-v1` | `native/features/workspace_import/` | Reviewed local component |
| #76 prototype-migration | `node:delegated-task:command%3Amcp%3Abbd47251-8a3b-4733-a985-4c7196ad668e%3Adelegate-task%3Aportability-prototype-migration-primary-v1` | `native/features/prototype_migration/` | Reviewed local component |

Paths above are relative to `arcified/`. Each primary owns its matching
`docs/features/` document. The coordinator owns this status artifact.

## Accepted interfaces

- Lifecycle Model remains the ownership and navigation authority.
- Export owns one C++17 versioned DTO, strict length-prefixed UTF-8 codec,
  validator and Model projection. It returns validated bytes without file IO.
- Import consumes that DTO and a const Model. Explicit Profile bindings and
  create versus match-existing mappings produce a plan, never mutate state.
- Prototype migration alone parses explicitly supplied extension-shaped JSON.
  Its unnamed portable Profile groups source records and requires an explicit
  destination Profile binding. It conveys no authentication identity.
- Durability owns persistence/recovery. Its accepted session schema and staged
  StoreImage model do not perform filesystem writes or flushes. Atomic portable
  destination replacement, cancellation and future import transactions remain
  native integration gaps. Library has no shared provider or store dependency.
- Native export source admission must consume reviewed regular-context
  classification and model binding, failing closed for unknown/private/guest/
  system contexts. Model projection is synthetic metadata only and cannot
  establish that eligibility. Never serialize native context identity.

## Blockers and next actions

Native session durability, native import transaction/application, native URL
policy and installed Linux/Windows/macOS interaction evidence remain open.
Names, user order and timestamps absent from Model cannot be invented.
Populated cross-Profile migration and other open owner policies remain open.
No full Chromium preparation/build, heavy download or remote CI is authorized.

Round 1 specification review found no actionable defects. Standards review
found no documented violations and one nonblocking UTF-8 duplication concern.
The coordinator consolidated scalar decoding in `portability/utf8.h`, retaining
format NUL rules and JSON diagnostics at their input boundaries. All affected
focused checks pass, including converter and composed planner ASan/UBSan.
Follow-up standards and specification reviews both completed without actionable
findings. The specification reviewer independently reran the format suite and
both affected sanitizer suites successfully.
Assignments use verified `codex/gpt-6.1-sol`, `reasoningEffort=high`,
`serviceTier=default`. Their task suffixes are
`portability-standards-review-r2` and `portability-spec-review-r2`, using the
same task-ID prefix as the primaries.

Next: root integrates the reviewed local commit. Future native owners must wire
regular source admission, authoritative metadata acquisition, explicit real
Profile choice, durable never-reused IDs and source-target associations, atomic
destination file replacement and recoverable import application. Verify lazy
opening, cancellation/interruption, source retention and native
Linux/Windows/macOS UI/accessibility before issue acceptance. No shared lifecycle,
tooling, patch series, workflows or root catalog files changed here.
