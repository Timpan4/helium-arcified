<!-- arcified-backlog:sync-enroll -->

Part of [Track optional encrypted sync](https://github.com/Timpan4/helium-arcified/issues/13) and [the desktop roadmap](https://github.com/Timpan4/helium-arcified/issues/2).

## Scope decision

The owner excluded cross-device sync on 2026-10-23. Closed as not planned. No sync implementation, Arcified account, remote backend or native validation of this proposed capability is required. The original idea below is retained as historical research, not accepted work.

## Historical proposed outcome

Let an authorized device join encrypted workspace sync with understandable identity and state.

Type: implementation. Phase: Optional capability.

Disposition: Excluded by owner on 2026-10-23. Cross-device sync and its account/backend are not required.

## Historical acceptance criteria

- [ ] Authenticate/enroll explicitly, disclose which data syncs, and keep excluded cookies/passwords/credentials out of the payload.
- [ ] Store keys through each OS's secure facilities and show pending, syncing, offline and failed states.
- [ ] Verify a new device receives the intended workspace and a declined or untrusted enrollment receives no readable data.

## Closure

Owner decision recorded: out of scope, closed as not planned. Native blocker links are removed. Closure does not claim an implementation.

## Evidence and design intent

The linked reference documents/release histories describe this workflow family. Acceptance below specifies the intended improved Arcified behavior.

Reference platform support is limited to the cited source. Documentary macOS or Windows coverage does not prove native behavior; Linux is a downstream design and verification target.

- [Arc: Arc Sync](https://resources.arc.net/hc/en-us/articles/20272860828823-Arc-Sync)
- [Arc: Recovery Cards](https://resources.arc.net/hc/en-us/articles/22370005382167-Recovery-Cards)
- [Dia: security](https://www.diabrowser.com/security)
- [Dia: security / bulletins](https://www.diabrowser.com/security/bulletins)

## Open decisions and limits

Native integration, failure behavior and applicable platform results must be proven for this slice; unavailable checks stay open.

Proposed work after native/reference prerequisites and explicit decisions in this issue.
