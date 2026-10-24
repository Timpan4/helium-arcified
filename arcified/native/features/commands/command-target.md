# Shared page command target

Commands owns `command_target.h` and its read-only resolver. Navigation and
transients accepted this interface before implementation. It serves explicit
page commands such as Back, Forward, Reload, Stop and Find, including a page
presented by a transient surface. It does not own keyboard arbitration,
modifiers, overlay lifetime, selection, focus, navigation, privacy policy or
command execution.

`ResolvePageCommandTarget(model, focused_page, expected_attachment)` returns an
owned `PageCommandTarget{page, profile, attachment}` or `TargetError`. The focus
owner supplies both identities. The resolver checks live page identity and its
current attachment, then derives the immutable Profile from the lifecycle
model. It never infers focus from selected Space or a saved item. Resolution does
not change a page, item or window.

`RevalidatePageCommandTarget(model, target)` checks current semantic liveness,
attachment and Profile. Native execution must additionally revalidate the
TabModel/WebContents binding and native operation generation on the serialized
UI sequence immediately before effects. A page transferred away and back passes
semantic revalidation. Native replacement can also retain `PageId`. This API is
therefore not a native freshness token or an authorization decision.

The native owner must reject stale completions, preserve actual focus ownership
and apply action-specific policies. A command target does not authorize a saved
page transfer, a destructive command or cross-Profile migration. Native page
commands and destination activation plans are separate contracts.

The source design is [native Space integration](../../../docs/native-space-integration.md),
especially its command/native-completion boundary, and the pinned
[C2/C3 source references](../../../docs/native-space-source-references.md).
The authoritative [lifecycle v3 contract](../../../docs/lifecycle-contract.md)
and existing model supply the identities and ownership. No lifecycle changes or
native patches are introduced.

Focused synthetic check from the repository root:

```sh
check_output_dir=$(mktemp -d)
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc \
  arcified/native/features/commands/command_target.cc \
  arcified/native/features/commands/command_target_test.cc \
  -o "$check_output_dir/command-target-test"
"$check_output_dir/command-target-test"
```

The fixtures cover explicit focus independent of selected Space, saved and
temporary pages, missing targets, wrong Profile/attachment, transfer and close.
They expose the semantic ABA limitation rather than claiming to solve native
generations. Installed native behavior and Linux/Windows/macOS acceptance remain
open under the deferred host/build prerequisites.
