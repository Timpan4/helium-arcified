<!-- arcified-backlog:sync-conflicts -->

Part of [Track optional encrypted sync](https://github.com/Timpan4/helium-arcified/issues/13) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## Scope decision

The owner excluded cross-device sync on 2026-10-23. Closed as not planned. No sync implementation, Arcified account, remote backend or native validation of this proposed capability is required. The original idea below is retained as historical research, not accepted work.

## Historical proposed outcome

Converge concurrent Space/folder/tab edits with visible conflict handling.

Type: implementation. Phase: Optional capability.

Disposition: Excluded by owner on 2026-10-23. Cross-device sync and its account/backend are not required.

## Historical acceptance criteria

- [ ] Test concurrent moves, renames, edits, deletes/restores and clock skew without cycles, duplication or silent loss.
- [ ] Persist deletion/tombstone semantics and prevent old clients from resurrecting removed work.
- [ ] Show conflicts that require a user choice and preserve a recoverable local export.

## Closure

Owner decision recorded: out of scope, closed as not planned. Native blocker links are removed. Closure does not claim an implementation.

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Arc Sync](https://resources.arc.net/hc/en-us/articles/20272860828823-Arc-Sync)
- [Arc: Recovery Cards](https://resources.arc.net/hc/en-us/articles/22370005382167-Recovery-Cards)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
