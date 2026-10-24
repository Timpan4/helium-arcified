#ifndef ARCIFIED_NATIVE_FEATURES_PEEK_PEEK_H_
#define ARCIFIED_NATIVE_FEATURES_PEEK_PEEK_H_

#include <string>
#include <variant>

#include "../../model/lifecycle.h"

namespace arcified::peek {

// Caller supplies every setting. No product default or persistence is chosen.
// "Kept" means a Space-owned item. "Favorite" means a Profile-owned item or
// an existing Profile shortcut alias to the parent's saved item.
struct Settings {
  Settings(bool automatic_kept_outbound,
           bool automatic_favorite_outbound,
           bool explicit_outbound)
      : automatic_kept_outbound(automatic_kept_outbound),
        automatic_favorite_outbound(automatic_favorite_outbound),
        explicit_outbound(explicit_outbound) {}

  bool automatic_kept_outbound;
  bool automatic_favorite_outbound;
  bool explicit_outbound;
};

// Native input handling identifies the modifier. No platform key is selected.
enum class Invocation { kAutomatic, kExplicitModifier };
enum class PolicyRefusal { kAutomaticDisabledForSource, kExplicitDisabled };
struct Eligible {};
using EligibilityResult =
    std::variant<Eligible, PolicyRefusal, lifecycle::LinkRoutingRefusal>;

// Pure presentation eligibility within PlanSavedPageLink's approved scope.
// Destination and classification are browser-validated inputs. No navigation
// plan, URL, Profile or attachment snapshot is retained. Eligibility neither
// opens a page nor authorizes a fallback when refused. Native code must
// revalidate the originating document and routing before creating a branch.
EligibilityResult EvaluateLink(const lifecycle::Model& model,
                               lifecycle::PageId parent,
                               lifecycle::NavigationKind kind,
                               lifecycle::SiteRelation relation,
                               std::string destination,
                               Invocation invocation,
                               const Settings& settings);

// Borrowed presentation references only. Lifecycle/native owners retain pages,
// Profile, navigation, activity and lifetime. No focus or selection is stored.
struct PresentationReferences {
  lifecycle::PageId parent;
  lifecycle::PageId preview;
};

enum class ReferenceStatus {
  kCompatible,
  kSamePage,
  kAttachmentMissing,
  kParentMissing,
  kPreviewMissing,
  kParentNotSaved,
  kPreviewNotTemporary,
  kProfileMismatch,
  kAttachmentMismatch,
};

// Checks current graph compatibility, not native readiness or parentage.
// The caller supplies the intended visible attachment and proves that the
// preview was created for this parent. Recheck after model mutations. Native
// focus/security, current document and object-generation checks are separate.
// Even kCompatible grants no dismissal, close, cleanup or promotion authority.
ReferenceStatus CheckReferences(
    const lifecycle::Model& model,
    const PresentationReferences& references,
    const lifecycle::Attachment& expected_attachment);

}  // namespace arcified::peek

#endif  // ARCIFIED_NATIVE_FEATURES_PEEK_PEEK_H_
