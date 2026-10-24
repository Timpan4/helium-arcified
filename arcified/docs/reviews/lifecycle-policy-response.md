# Lifecycle policy response

Recorded: 2026-10-23. Scope: issue 24's standalone contract/model. This implements the new policy brief after the clean [version 2 review](lifecycle-review-round-2.md); it is not a fresh independent review or native-browser acceptance.

The [contract](../lifecycle-contract.md) now records these owner approvals precisely:

1. Each Space chooses exactly one Profile; multiple Spaces can share it. Sign-ins, site settings and Favorites are Profile-scoped. Space themes are Space-scoped. Settings/theme payloads are not implemented by this identity model.
2. Outbound links from a saved page open a temporary branch in that page's current window, Space and Profile. The saved page stays where it was, preserving its saved destination, current navigation, identity and activity.
3. Existing saved-page handoff and same-window Space switching across Profiles remain approved. Their transitions and regression fixtures remain intact.

The owner explicitly kept activity/cleanup/destructive-action and populated cross-Profile migration/source-disposition policy open. Existing refusals remain a bounded safety subset, not approved product defaults. No cleanup or source removal was added. Same-site link routing, redirects, scripts and typed/other navigation do not gain outbound-link approval.

`Model::PlanSavedPageLink` is a const decision returning either an explicit refusal or an owned `TemporaryBranchPlan`. Only browser-supplied `kLink` and `kOutbound` qualify. Unknown/same-site classification, non-link triggers, missing/non-saved source and empty destination are refused. The model never derives sites from URL text. The caller supplies authoritative classification relative to the source's current committed document; GURL/native classification and native actuation remain separate work.

Planning performs no allocation or mutation. Native failure/cancellation records nothing. The caller must revalidate source/document/Profile/attachment/classification, coordinate model preconditions and actual native success, then report successful local temporary creation with fresh IDs through existing `OpenTemporary`. Plans are snapshots, not reservations or commit tokens. Native races, redirect handling and failure reconciliation are not implemented or proven. The original's protected/unknown activity is preserved, so branching does not depend on approving a destructive-action policy.

Semantic version is now **3**. The routing API and new approvals are traceable to this revision; prior transition signatures remain unchanged, and the borrowed-ID fix remains. `CheckContractVersion` rejects version 2. The native design plan was based on version 2 and needs explicit version 3 adoption at its future adapter boundary. Native-space documents were left untouched during their separate design review. Previous reviews were preserved.

Focused checks passed on Linux with existing Clang 23.1.3:

```sh
clang-format --style=Chromium --dry-run --Werror \
  arcified/native/model/lifecycle.h \
  arcified/native/model/lifecycle.cc \
  arcified/native/model/lifecycle_test.cc
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  arcified/native/model/lifecycle.cc arcified/native/model/lifecycle_test.cc \
  -o /tmp/arcified-lifecycle-policy-v3-test
/tmp/arcified-lifecycle-policy-v3-test
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  arcified/native/model/lifecycle.cc arcified/native/model/lifecycle_test.cc \
  -o /tmp/arcified-lifecycle-policy-v3-sanitized
/tmp/arcified-lifecycle-policy-v3-sanitized
clang++ --driver-mode=g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I arcified/native/model arcified/native/model/lifecycle.cc \
  /tmp/arcified-lifecycle-round-2-adversarial.cc \
  -o /tmp/arcified-lifecycle-policy-v3-adversarial
/tmp/arcified-lifecycle-policy-v3-adversarial
```

All 16 supplied fixtures passed both strict unsanitized and ASan/UBSan runs. They include all 13 previous getter/transfer/selection regressions and three routing fixtures. New coverage checks protected/unknown originals, separate simulated native-success reporting, preserved saved/current navigation and split membership, fresh local temporary ownership, explicit refusal outcomes, Profile-owned saved items and replanning after transfer. The unchanged supplemental round 2 source passed all three adversarial groups against version 3 with ASan/UBSan. No sanitizer diagnostic occurred. These are synthetic model checks, not native success/cancellation or browser isolation tests.

The executables above and the previous review/source/test artifacts are retained. No full build, source download, global installation, remote CI or cleanup deletion was performed. No tooling, CI, performance, execution-ledger or GitHub changes were made by this task. Fresh independent review and remaining owner policy questions belong to the parent. Native classification/actuation, browser interaction and Windows/macOS behavior remain unverified.
