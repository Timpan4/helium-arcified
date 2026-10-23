#ifndef ARCIFIED_NATIVE_FEATURES_PROFILES_PREPARATION_H_
#define ARCIFIED_NATIVE_FEATURES_PROFILES_PREPARATION_H_

#include <variant>

#include "../../model/lifecycle.h"
#include "../private_browsing/context_boundary.h"

namespace arcified::profiles {

// Borrowed from the caller's authoritative live native binding. This component
// neither allocates native IDs nor owns a registry. Distinct actual Profile/OTR
// objects require distinct, non-reused native context IDs.
struct ProfileBinding {
  lifecycle::ProfileId profile;
  private_browsing::NativeContextId context;
};

enum class PreparationRefusal {
  kProfileNotFound,
  kWindowNotFound,
  kSpaceNotFound,
  kNotAttached,
  kPageNotFound,
  kBindingProfileMismatch,
  kUnknownNativeContext,
  kNativeContextMismatch,
  kUnsupportedNativeContext,
  kOriginalProfileMismatch,
  kRegularProfileRequired,
};

// Owned identity snapshots only. They retain no native object, reserve no
// operation and grant no permission to navigate, select, transfer or persist.
// An OTR target never authorizes ordinary lifecycle persistence or attachment
// as a regular Space. Consumers must also apply the private-browsing boundary.
struct ContextTarget {
  lifecycle::ProfileId profile;
  private_browsing::NativeContextId context;
  private_browsing::ContextClass context_class;
};
struct SpacePreparation {
  lifecycle::Attachment attachment;
  ContextTarget target;
};
struct PagePreparation {
  lifecycle::PageId page;
  SpacePreparation space;
};
struct RegularProfileServiceTarget {
  lifecycle::ProfileId profile;
  private_browsing::NativeContextId context;
};

using SpacePreparationResult =
    std::variant<PreparationRefusal, SpacePreparation>;
using PagePreparationResult = std::variant<PreparationRefusal, PagePreparation>;
using RegularProfileServiceResult =
    std::variant<PreparationRefusal, RegularProfileServiceTarget>;

// Resolves any registered Profile, including an unreferenced Profile or one
// with unattached Spaces. No attachment, window or selected Space is required.
RegularProfileServiceResult PrepareRegularProfile(
    const lifecycle::Model& model,
    const lifecycle::ProfileId& profile,
    const ProfileBinding& binding,
    const private_browsing::NativeContextFacts& facts);

// facts must come from the actual target browsing context, not the selected
// legacy Browser's Profile or its GetOriginalProfile(). The caller keeps native
// objects alive and serializes model/native observation on the browser UI
// sequence. No result establishes a same-OS-window multi-Profile host.
SpacePreparationResult PrepareSpace(
    const lifecycle::Model& model,
    const lifecycle::Attachment& attachment,
    const ProfileBinding& binding,
    const private_browsing::NativeContextFacts& facts);

// Checks the page's CURRENT attachment and immutable Profile, irrespective of
// the window's selected Space. This is not a destination/transfer planner.
PagePreparationResult PreparePage(
    const lifecycle::Model& model,
    const lifecycle::PageId& page,
    const ProfileBinding& binding,
    const private_browsing::NativeContextFacts& facts);

// Only a validated regular Profile receives this lookup target. Pass its exact
// live Profile to native GetPrefs / HostContentSettingsMapFactory /
// BookmarkModelFactory / HistoryServiceFactory, preserving each native API's
// access rules and null results. This does not obtain or authorize a service.
// Favorites use target.profile; themes use the resolved attachment.space.
RegularProfileServiceResult PrepareRegularProfileServices(
    const lifecycle::Model& model,
    const lifecycle::Attachment& attachment,
    const ProfileBinding& binding,
    const private_browsing::NativeContextFacts& facts);

}  // namespace arcified::profiles

#endif  // ARCIFIED_NATIVE_FEATURES_PROFILES_PREPARATION_H_
