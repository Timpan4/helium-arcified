# Sidebar composition review

Reviewed on 2026-10-23. No actionable findings in the authorized conformance-only scope. The three scenarios exercise real component implementations against a shared lifecycle model and assert observable results. They do not establish a production sidebar integration.

## Reviewed scope

Reviewed the complete uncommitted `arcified/native/integration/sidebar/sidebar_composition_test.cc` and `arcified/docs/features/sidebar-composition.md`. Checked the relevant accessibility, pins, Favorites, temporary-tabs, organization, workspace inspection and handoff API contracts, plus the implementations involved in the label and handoff joins. This was a review of the new composition, not a repeat review of every underlying feature.

Reviewed content identities from `git hash-object`:

| File | Blob hash |
| --- | --- |
| `sidebar_composition_test.cc` | `4f8fd4295c81b9432b68a85e67c7bea5b4577704` |
| `sidebar-composition.md` | `545808da81d865706171a0e9fc329787b226913e` |

## Findings and behavioral evidence

No changes are required for this fallback. The following observations support that conclusion.

1. **Canonical alias labels are a real join.** `Fixture::Preview` at test line 112 passes the actual `pins::DescribeSavedItem` result into `ProjectFolderPreview`, keyed by its returned item identity. The scenario at line 229 checks agreement with both Favorite and workspace alias resolution, an explicit non-ASCII expected label, and the existing live page. It applies a real label edit through the resolved canonical item and observes the replacement label through Favorite presentation and folder search. Keeping and rechecking the earlier preview also tests owned snapshot behavior. These expectations would detect a missing edit, wrong label or wrong live identity. The UTF-16 row names are separately supplied fixture values, so their equality and name-change assertions cover accessibility projection behavior given those values, not automatic label propagation.

2. **The visible preorder and collapsed descendant cases respect separate contracts.** The expanded fixture at line 128 includes the alias, nested saved item, unplaced saved item and temporary row. Its hierarchy, sibling-count, Down and End assertions test the accessibility API's derived semantics and traversal. The selection scenario at line 307 feeds actual projected selection into the saved row and actual selection into the move planner. It checks a concrete destination order and reads back unchanged organization state. The collapse request is asserted before being applied explicitly. After application, a search with no matches still exposes the collapsed folder's active saved descendant, while the separate visible projection excludes both hidden rows and suggests the surviving folder for lost child focus. No test derives visible ordering from preview matches or inspection iteration.

3. **Requesting and destination attachment boundaries are exercised independently.** The scenario at line 375 resolves the alias from a different Space in the destination window, prepares the owning-Space destination, and checks source attachment, focus intent and split effects. At line 414, detaching the requesting attachment causes fresh resolution/preparation to refuse and the old plan to fail revalidation. The test restores that attachment and its selected Space before separately detaching the owning-Space destination at line 437. This isolates the second boundary. Workspace resolution then reports that attachment is required, while preparation still refuses. The fresh plan at line 448 is checked before the explicit synthetic transfer. Subsequent checks observe stale source intent, the new live attachment, retained page identity/navigation/activity/saved destination, split cleanup and inspection references.

4. **Borrowed lifetimes are valid in these scenarios.** Every row vector outlives its projection and all comparisons. The UTF-16 names and command labels reference string literals with static lifetime. The disabled-command projection borrows a separate copied vector that is modified before borrowing; the earlier vector remains unchanged. Owned previews, command requests and handoff plans are not mistaken for live mutable model references. The stale projection deliberately continues to advertise its old command, while a fresh disabled projection refuses it. Actual dispatch authorization remains a caller obligation.

5. **The document describes the fallback honestly.** It identifies the missing native ordering, row binding, localization/encoding, object-validity and focus inputs. It distinguishes fixture-supplied rows from an adapter, model transfer from native transfer, semantic changes from native AX events, and standalone checks from browser/platform acceptance. The OPEN native preparation/build/platform work remains outside this result. No production facade, state owner or native ordering/conversion policy was introduced.

## Independently executed checks

On Linux x86_64 with Clang, clang-format and clang-tidy 23.1.3:

| Check | Result and coverage |
| --- | --- |
| Compile the documented composition source set with C++17, `-Wall -Wextra -Werror -pedantic`, then run | Exit 0; printed `PASS: sidebar composed conformance scenarios`. Executes all three scenarios called by `main`. |
| Compile and run the same source set with `-g -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer` and the same strict flags | Exit 0; same PASS output; no sanitizer diagnostics on these scenarios. |
| `clang-format --style=Chromium --dry-run --Werror` on the new C++ file | Exit 0; no diagnostics. |
| `clang-tidy --checks='-*,clang-analyzer-*' --warnings-as-errors='*'` on the new C++ translation unit, with the documented C++17 flags | Exit 0; no diagnostics. This is not an analyzer run over every linked implementation. |
| `git diff --no-index --check /dev/null` for each reviewed new file | No whitespace diagnostics; each returned 1 for the new-file difference. |

## Coverage limits

The row fixtures are authoritative synthetic inputs. These checks do not validate a native producer's completeness, ordering, ID allocation, row-to-alias binding, UTF-8/UTF-16 conversion, localization, selection publication or capability refresh. The explicit alias command check followed by caller-supplied binding demonstrates compatible contracts, not a working dispatch adapter. Similarly, supplying refreshed names and collapsed rows does not prove automatic synchronization with organization or pins metadata.

Only the concrete scenarios above were executed. No mutation testing, wider primitive-policy reruns, browser build, native UI interaction, keyboard dispatch, native attachment preparation, AX event publication, assistive technology, native failure/cancellation transaction, ABA detection or focus application was checked. Windows and macOS behavior remains unverified. The conformance-only fallback is reviewable and passes its focused checks; production/native acceptance remains open.

Only this review report was added by the reviewer. The implementation and feature document were preserved. No commit, remote write, PR linking or PR watching was performed.
