# Arc, Dia and YAB desktop investigation

[The published roadmap](https://github.com/Timpan4/helium-arcified/issues/2) prioritizes browser UX, Linux/Windows/macOS support and measured core performance. The owner excluded cross-device sync and put AI later. Core browsing requires no Arc, Dia or new Arcified account.

## Current scope and publication

- 135 active non-AI work items precede 23 later AI/supporting items, for 158 active leaf items.
- Six cross-device sync proposals and their tracker are closed as not planned. Local Profiles, multi-window organization, restart recovery and import/export remain in scope.
- There are 178 historical issues: 171 open and 7 closed as not planned. Twelve topic trackers remain active. The native graph has 262 blocking relationships, no missing keys or cycles, and no non-AI dependency path through AI.
- Active kinds are 137 implementation items, 13 decisions, 6 research items and 2 evidence trackers. Optional integrations and hosted sharing remain explicit decisions rather than core account requirements.

All 178 remote titles/bodies, IDs, states/reasons and parent/child/blocker sets were read back from paginated GitHub GraphQL and matched to exact publication inputs. Expected leaf labels match. All 178 local Markdown bodies match those inputs. Publication is proven complete; no items await publication.

See the [published index](published-index.md), [catalog](backlog.json), [readback proof](publication/verification.json), [receipts](publication/receipts.json) and [owner scope-change receipts](publication/scope-change-receipts.json). AI's parent tracker applies later priority to all 21 assistant items; the consent contract and AI resource-cost item also carry explicit later priority. Privacy controls can ship independently, while later AI enablement still requires its consent/security contract.

## Evidence and reviews

- [Arc desktop and historical UX](arc-ux.md), [Dia UX/performance research](dia-ux-performance.md) and [fork baseline](fork-baseline.md).
- [Official artifact identities and REA limits](artifacts.md), [YAB inspiration](yab-inspiration.md) and [UX improvement proposals](ux-improvements.md).
- [Luna slicing audit](slicing-review.md), [Astra coverage audit](coverage-review.md), [final publication review](final-publication-review.md) and [responses](review-responses.md).
- [Full family mapping and historical dispositions](coverage-matrix.md).

Daybreak Blue and Luna supplied primary-source inventories and design/slicing reviews. Astra reviewed every revised catalog record and found no publication blockers. The parent reconciled findings, published and verified GitHub state, then applied the owner's later scope decisions and verified every issue again.

Official Arc and Dia macOS images were downloaded, hashed, extracted and inspected with REA. Complete bundle inventories and partial Arc menu/resource decoding succeeded. Native analysis did not complete within the provider's existing startup deadline. No accepted pseudocode was recovered or proprietary implementation copied.

## Remaining product work

No reference app, native Arcified build, migration scenario or benchmark was executed. Native UX, accessibility, platform behavior and comparative performance remain unverified and explicitly tracked. YAB is documentary evidence. Catalog coverage does not establish exhaustive undocumented parity. Inspect and reuse existing Helium behavior before adding code.

Use each issue's prerequisites, owner decisions and close proof. Implementation, tested behavior and released behavior remain separate states. Closed sync proposals are excluded history, not delivered features. AI is optional and later; retain its consent and security requirements if implemented. Optional GitHub, calendar, remote-provider and hosted-sharing accounts do not create an Arc account requirement.

## Fork permission change

The owner explicitly approved repository-wide agent changes. Root and Arcified AGENTS.md permit them locally, preserving the prohibition on writes to imputnet repositories, licensing and patch-order requirements. These permission edits and research files remain local and uncommitted. No matching open PR was found; no PR was created or merged.
