#include "peek.h"

#include <utility>

namespace arcified::peek {

EligibilityResult EvaluateLink(const lifecycle::Model& model,
                               lifecycle::PageId parent,
                               lifecycle::NavigationKind kind,
                               lifecycle::SiteRelation relation,
                               std::string destination,
                               Invocation invocation,
                               const Settings& settings) {
  const auto routing =
      model.PlanSavedPageLink(parent, kind, relation, std::move(destination));
  if (const auto* refusal =
          std::get_if<lifecycle::LinkRoutingRefusal>(&routing)) {
    return *refusal;
  }

  // The lifecycle planner established a live saved parent. Read its existing
  // item and aliases; do not introduce a separate pinned/Favorite registry.
  const auto& page = model.pages().at(parent);
  const auto& saved = std::get<lifecycle::SavedPage>(page.owner);
  const auto& item = model.items().at(saved.item);
  const bool kept = std::holds_alternative<lifecycle::SpaceId>(item.owner);
  bool favorite = std::holds_alternative<lifecycle::ProfileId>(item.owner);
  for (const auto& [id, shortcut] : model.shortcuts()) {
    if (shortcut.profile == page.profile && shortcut.target == saved.item) {
      favorite = true;
      break;
    }
  }

  switch (invocation) {
    case Invocation::kAutomatic:
      if ((kept && settings.automatic_kept_outbound) ||
          (favorite && settings.automatic_favorite_outbound)) {
        return Eligible{};
      }
      return PolicyRefusal::kAutomaticDisabledForSource;
    case Invocation::kExplicitModifier:
      if (settings.explicit_outbound)
        return Eligible{};
      return PolicyRefusal::kExplicitDisabled;
  }
  return PolicyRefusal::kExplicitDisabled;
}

ReferenceStatus CheckReferences(
    const lifecycle::Model& model,
    const PresentationReferences& references,
    const lifecycle::Attachment& expected_attachment) {
  if (references.parent == references.preview)
    return ReferenceStatus::kSamePage;
  if (!model.attachments().count(expected_attachment))
    return ReferenceStatus::kAttachmentMissing;
  const auto parent = model.pages().find(references.parent);
  if (parent == model.pages().end())
    return ReferenceStatus::kParentMissing;
  const auto preview = model.pages().find(references.preview);
  if (preview == model.pages().end())
    return ReferenceStatus::kPreviewMissing;
  if (!std::holds_alternative<lifecycle::SavedPage>(parent->second.owner))
    return ReferenceStatus::kParentNotSaved;
  if (!std::holds_alternative<lifecycle::TemporaryTab>(preview->second.owner))
    return ReferenceStatus::kPreviewNotTemporary;
  if (parent->second.profile != preview->second.profile)
    return ReferenceStatus::kProfileMismatch;
  if (!(parent->second.attachment() == expected_attachment) ||
      !(preview->second.attachment() == expected_attachment)) {
    return ReferenceStatus::kAttachmentMismatch;
  }
  return ReferenceStatus::kCompatible;
}

}  // namespace arcified::peek
