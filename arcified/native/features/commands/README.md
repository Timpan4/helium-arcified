# Commands components

These standalone C++ components implement local decision and interaction logic
for [command-bar #77](../../../docs/research/arc-dia/publication/command-bar.md),
[command-ranking #109](../../../docs/research/arc-dia/publication/command-ranking.md)
and [site-search #110](../../../docs/research/arc-dia/publication/site-search.md).
They are not installed native browser features. See
[workstream status](workstream-status.md) for task identities, review and native
blockers. No issue can close from these synthetic checks.

| Component | Responsibility | Native caller retains |
| --- | --- | --- |
| [Command bar](command-bar.md) | Query/session freshness, caller-ordered rows, stable selection, explicit activation/cancellation intents | Keyboard/IME arbitration, actual focus restoration, URL classification, permissions and execution |
| [Workspace ranking](command-ranking.md) | Deterministic per-call projection within one regular Profile, provenance and Switch/Open/Reveal identities | Authoritative privacy scope, display metadata, native latency measurement, execution |
| [Site search](site-search.md) | Labeled provider discovery, explicit provider/default preview, literal UTF-16 and stale submit refusal | TemplateURLService CRUD/defaults/policy, native query classification and TemplateURLRef expansion |
| [Page target](command-target.md) | Resolve/revalidate explicit lifecycle PageId and Attachment, derive immutable Profile | Real focus owner, native binding/generation freshness and action-specific security policy |
| `workspace_results` | Map ranking output into typed bar actions and stable presentation IDs | Localized consequence/provenance labels, secondary-action presentation and final native revalidation |

## Ranking and command bar integration

`MakeWorkspaceResult(command, mode, action_label)` maps one scoped ranking row
into a bar `Result`. `kActivate` produces a Switch action for a live saved or
temporary page and an OpenSavedItem action for a closed saved destination.
`kReveal` produces an explicit RevealInWorkspace action. A native UI can present
Reveal as a secondary choice; this core supplies no view or button arrangement.
The caller provides a localized consequence label, including visible provenance.

Saved-open actions carry ItemId, immutable Profile and existing ItemOwner. They
carry no copied URL. Switch actions preserve the current PageId/Profile/source
Attachment, even when that page lives in another window. The initiating surface
and requested destination remain separate. Returning an intent never transfers,
selects or opens a page. Source attachment is not a transfer destination.

Presentation IDs include action kind and semantic target with length-framed
opaque fields. Titles and current navigation do not enter row identity. Renames
therefore preserve selection; a closed item becoming live, a page transfer or a
temporary page being kept changes the action identity. The bar clears selection
when the old identity disappears, so Enter cannot silently acquire a new target
or consequence. These keys have no durable ownership or parsing role.

Site search remains a separate explicit-provider mode. Its native `SearchIntent`
must retain service revision and actual provider metadata through native final
revalidation. Do not flatten it into a generic bar SearchAction and lose the
revision, or expand a URL locally. The general bar SearchAction is inert text
for an independently validated native path. The deferred native composition of
provider UI and bar activation remains open.

## Adjacent ownership contracts

Navigation, transients and site-tools consume the same page-target resolver.
No component infers focus from selected Space, overlay parent or saved item.
The native host owns real focus, native command dispatch and announcements.
Input #113 owns arbitration after native IME handling; session methods consume
semantic actions after that gate. Model revalidation cannot detect native
replacement or a page moved away and back. Native binding and operation
generation checks are mandatory immediately before effects on the serialized
UI sequence. No second ownership, navigation, focus or persistence registry is
introduced here.

## Focused checks

Existing project checks use C++17, strict warnings and Chromium formatting. From
the repository root, each fixture links only the needed local components:

```sh
check_output_dir=$(mktemp -d)
commands_dir=arcified/native/features/commands
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  "$commands_dir/command_bar.cc" "$commands_dir/command_bar_test.cc" \
  -o "$check_output_dir/bar-test"
"$check_output_dir/bar-test"
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  "$commands_dir/command_ranking.cc" "$commands_dir/command_ranking_test.cc" \
  -o "$check_output_dir/ranking-test"
"$check_output_dir/ranking-test"
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  "$commands_dir/site_search.cc" "$commands_dir/site_search_test.cc" \
  -o "$check_output_dir/search-test"
"$check_output_dir/search-test"
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  "$commands_dir/command_target.cc" "$commands_dir/command_target_test.cc" \
  -o "$check_output_dir/target-test"
"$check_output_dir/target-test"
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  "$commands_dir/command_target.cc" "$commands_dir/command_bar.cc" \
  "$commands_dir/command_ranking.cc" "$commands_dir/workspace_results.cc" \
  "$commands_dir/workspace_results_test.cc" \
  -o "$check_output_dir/workspace-results-test"
"$check_output_dir/workspace-results-test"
clang-format --style=Chromium --dry-run --Werror "$commands_dir/"*.h "$commands_dir/"*.cc
clang-tidy "$commands_dir/"*.cc \
  --checks='clang-analyzer-core.*,clang-analyzer-cplusplus.*,bugprone-use-after-move' \
  --warnings-as-errors='*' -- -std=c++17 -Wall -Wextra -Werror -pedantic
```

The default local clang-tidy invocation enables no checks; the explicit checks
above cover core/C++ analyzer defects and use after move. The optional
`clang-analyzer-optin.core.EnumCastOutOfRange` checker flags the deliberate
invalid-enum boundary fixture in `site_search_test.cc`; that negative test is
retained. ASan/UBSan executions are
recorded in the feature docs and review report. No native build, browser,
networked provider, real browsing data, benchmark or remote CI is required by
these focused checks. Installed native Linux/Windows/macOS interaction,
security, persistence, accessibility and visual acceptance remain open.
