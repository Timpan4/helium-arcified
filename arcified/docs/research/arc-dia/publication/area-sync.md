<!-- arcified-backlog:area-sync -->

Part of [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2). This tracker groups 6 scoped work items for optional encrypted sync.

## Scope decision

The owner excluded cross-device sync. All six child proposals and this tracker are closed as not planned. No Arcified account or sync backend is required. Local import/export, restart recovery and organization across windows on the same machine remain in scope. Closed items do not represent implemented sync.

## Historical work items

- [x] [#21 Decide encrypted sync scope and key ownership](https://github.com/Timpan4/helium-arcified/issues/21), `sync-contract`, decision, Evidence, decisions and gates
- [x] [#156 Enroll another device and show encrypted sync status](https://github.com/Timpan4/helium-arcified/issues/156), `sync-enroll`, implementation, Optional capability
- [x] [#157 Merge offline workspace edits without losing tabs](https://github.com/Timpan4/helium-arcified/issues/157), `sync-conflicts`, implementation, Optional capability
- [x] [#174 Recover encrypted workspace access with trusted recovery material](https://github.com/Timpan4/helium-arcified/issues/174), `sync-recovery`, implementation, Optional capability
- [x] [#139 Prove an encrypted workspace round-trip with synthetic data](https://github.com/Timpan4/helium-arcified/issues/139), `sync-roundtrip`, implementation, Optional capability
- [x] [#176 Revoke a synced device and restrict future key access](https://github.com/Timpan4/helium-arcified/issues/176), `sync-revoke`, implementation, Optional capability

## Closure

Closed as not planned after removing native sync blocker links. Historical source research remains available in the children.
