#ifndef ARCIFIED_NATIVE_FEATURES_PROFILE_SAFETY_IMPACT_H_
#define ARCIFIED_NATIVE_FEATURES_PROFILE_SAFETY_IMPACT_H_

#include <array>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::profile_safety {

struct ReassignSpace {
  lifecycle::SpaceId space;
  lifecycle::ProfileId target;
};
// Preparatory reference inventory for Profile operations, not deletion policy.
struct DeleteSpace {
  lifecycle::SpaceId space;
};
struct DeleteProfile {
  lifecycle::ProfileId profile;
};
using Request = std::variant<ReassignSpace, DeleteSpace, DeleteProfile>;

enum class PreviewError { kSpaceNotFound, kProfileNotFound };
enum class ExecutionGate { kBlockedPolicyOpen };
enum class NativeUnknown {
  kUnsavedForms,
  kDownloads,
  kAuthenticatedState,
  kProfileDataOutsideModel,
};

struct SpaceReference {
  lifecycle::SpaceId id;
  lifecycle::ProfileId profile;
};
struct ItemReference {
  lifecycle::ItemId id;
  lifecycle::ItemOwner owner;
};
struct AliasReference {
  lifecycle::ShortcutId id;
  lifecycle::ProfileId profile;
  lifecycle::ItemId target;
};
struct SelectionReference {
  lifecycle::WindowId window;
  lifecycle::SpaceId space;
};
struct PageReference {
  lifecycle::PageId id;
  lifecycle::ProfileId profile;
  // Reuses the authoritative typed owner, which contains only IDs/attachment.
  lifecycle::PageOwner owner;
  lifecycle::Activity reported_activity;
};
struct SplitReference {
  lifecycle::SplitId id;
  lifecycle::Attachment attachment;
  std::vector<lifecycle::PageId> pages;
};

// Visibility dependencies only. These items/aliases are not reassigned,
// removed, copied or promised available in the destination by this preview.
struct ProfileContext {
  lifecycle::ProfileId profile;
  std::vector<lifecycle::ItemId> standalone_items;
  std::vector<AliasReference> aliases;
};

// Ephemeral ID-only snapshot, not an ownership store or execution token.
// Container order is deterministic ID order, not sidebar or user order.
struct Impact {
  Request request;
  std::vector<SpaceReference> spaces;
  std::vector<lifecycle::Attachment> attachments;
  std::vector<SelectionReference> selections;
  std::vector<ItemReference> items;
  std::vector<AliasReference> aliases;
  std::vector<PageReference> pages;
  std::vector<SplitReference> splits;
  // Reassignment only: source first, then target when different.
  std::vector<ProfileContext> context_only;
  // Even an empty or same-Profile request cannot authorize execution.
  ExecutionGate execution = ExecutionGate::kBlockedPolicyOpen;
  // Always unknown, including when there are no modeled pages or all report
  // quiescence. Profile-level native work need not have a modeled page.
  std::array<NativeUnknown, 4> native_unknowns{
      NativeUnknown::kUnsavedForms, NativeUnknown::kDownloads,
      NativeUnknown::kAuthenticatedState,
      NativeUnknown::kProfileDataOutsideModel};
};
using PreviewResult = std::variant<PreviewError, Impact>;

// Reads a valid single-writer lifecycle graph. Caller serializes access with
// model mutations and rebuilds the preview after any relevant change. No
// destination/navigation strings or native handles are read or copied.
PreviewResult Preview(const lifecycle::Model& model, Request request);

}  // namespace arcified::profile_safety

#endif  // ARCIFIED_NATIVE_FEATURES_PROFILE_SAFETY_IMPACT_H_
