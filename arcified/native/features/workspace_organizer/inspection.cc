#include "inspection.h"

#include <algorithm>
#include <type_traits>

namespace arcified::workspace_organizer {

bool Inspection::Contains(const Identity& identity) const {
  return std::any_of(identities.begin(), identities.end(),
                     [&](const auto& id) { return id == identity; });
}

std::vector<Reference> Inspection::ReferencesTo(
    const Identity& identity) const {
  std::vector<Reference> result;
  for (const auto& reference : references)
    if (reference.target == identity)
      result.push_back(reference);
  return result;
}

std::vector<Reference> Inspection::MissingTargets() const {
  std::vector<Reference> result;
  for (const auto& reference : references)
    if (!Contains(reference.target))
      result.push_back(reference);
  return result;
}

std::vector<lc::SpaceId> Inspection::UnattachedSpaces() const {
  std::vector<lc::SpaceId> result;
  for (const auto& identity : identities) {
    if (const auto* space = std::get_if<lc::SpaceId>(&identity)) {
      const auto incoming = ReferencesTo(*space);
      if (std::none_of(incoming.begin(), incoming.end(), [](const auto& ref) {
            return ref.relation == Relation::kAttachmentSpace;
          }))
        result.push_back(*space);
    }
  }
  return result;
}

std::vector<lc::ProfileId> Inspection::UnreferencedProfiles() const {
  std::vector<lc::ProfileId> result;
  for (const auto& identity : identities)
    if (const auto* profile = std::get_if<lc::ProfileId>(&identity))
      if (ReferencesTo(*profile).empty())
        result.push_back(*profile);
  return result;
}

Inspection Inspect(const lc::Model& model) {
  Inspection result;
  for (const auto& [id, profile] : model.profiles())
    result.identities.push_back(id);
  for (const auto& [id, space] : model.spaces()) {
    result.identities.push_back(id);
    result.references.push_back({id, Relation::kSpaceProfile, space.profile});
  }
  for (const auto& [id, window] : model.windows()) {
    result.identities.push_back(id);
    if (window.selected_space)
      result.references.push_back(
          {id, Relation::kSelectedSpace, *window.selected_space});
  }
  for (const auto& attachment : model.attachments()) {
    result.identities.push_back(attachment);
    result.references.push_back(
        {attachment, Relation::kAttachmentWindow, attachment.window});
    result.references.push_back(
        {attachment, Relation::kAttachmentSpace, attachment.space});
  }
  for (const auto& [id, item] : model.items()) {
    result.identities.push_back(id);
    std::visit(
        [&result, item_id = id](const auto& owner) {
          result.references.push_back({item_id, Relation::kItemOwner, owner});
        },
        item.owner);
  }
  for (const auto& [id, shortcut] : model.shortcuts()) {
    result.identities.push_back(id);
    result.references.push_back(
        {id, Relation::kShortcutProfile, shortcut.profile});
    result.references.push_back(
        {id, Relation::kShortcutTarget, shortcut.target});
  }
  for (const auto& [id, page] : model.pages()) {
    result.identities.push_back(id);
    result.references.push_back({id, Relation::kPageProfile, page.profile});
    result.references.push_back(
        {id, Relation::kPageAttachment, page.attachment()});
    std::visit(
        [&result, page_id = id](const auto& owner) {
          using T = std::decay_t<decltype(owner)>;
          if constexpr (std::is_same_v<T, lc::SavedPage>) {
            result.references.push_back(
                {page_id, Relation::kPageOwner, owner.item});
          } else {
            static_assert(std::is_same_v<T, lc::TemporaryTab>);
            result.identities.push_back(owner.id);
            result.references.push_back(
                {page_id, Relation::kPageOwner, owner.id});
            result.references.push_back(
                {owner.id, Relation::kTemporaryOrigin, owner.attachment});
          }
        },
        page.owner);
  }
  for (const auto& [id, split] : model.splits()) {
    result.identities.push_back(id);
    result.references.push_back(
        {id, Relation::kSplitAttachment, split.attachment});
    for (const auto& page : split.pages)
      result.references.push_back({id, Relation::kSplitPage, page});
  }
  return result;
}

ProfileDeletionPreview PreviewProfileDeletion(const lc::Model& model,
                                              const lc::ProfileId& profile) {
  if (!model.profiles().count(profile))
    return {ReferenceState::kIdentityNotFound, {}};
  auto references = Inspect(model).ReferencesTo(profile);
  const auto state = references.empty() ? ReferenceState::kNoModelReferences
                                        : ReferenceState::kReferenced;
  return {state, std::move(references)};
}

SpaceReassignmentPreview PreviewSpaceReassignment(const lc::Model& model,
                                                  const lc::SpaceId& space,
                                                  const lc::ProfileId& target) {
  auto source = model.spaces().find(space);
  if (source == model.spaces().end() || !model.profiles().count(target))
    return {ReassignmentState::kIdentityNotFound,
            PopulatedMigrationPolicy::kNotApplicable,
            {}};
  auto references = Inspect(model).ReferencesTo(space);
  if (source->second.profile == target)
    return {ReassignmentState::kSameProfile,
            PopulatedMigrationPolicy::kNotApplicable, std::move(references)};
  const bool populated = std::any_of(
      references.begin(), references.end(),
      [](const auto& ref) { return ref.relation == Relation::kItemOwner; });
  const auto state = references.empty()
                         ? ReassignmentState::kEmptyUnattached
                         : ReassignmentState::kBlockedByModelReferences;
  return {state,
          populated ? PopulatedMigrationPolicy::kOpen
                    : PopulatedMigrationPolicy::kNotApplicable,
          std::move(references)};
}

SidebarResult ResolveSidebar(const lc::Model& model,
                             const SidebarReference& reference,
                             const lc::Attachment& requesting) {
  std::optional<lc::ShortcutId> shortcut;
  std::optional<lc::ItemId> item_id;
  std::visit(
      [&](const auto& id) {
        using T = std::decay_t<decltype(id)>;
        if constexpr (std::is_same_v<T, lc::ItemId>) {
          item_id = id;
        } else {
          static_assert(std::is_same_v<T, lc::ShortcutId>);
          auto alias = model.shortcuts().find(id);
          if (alias != model.shortcuts().end()) {
            shortcut = id;
            item_id = alias->second.target;
          }
        }
      },
      reference);
  if (!item_id)
    return lc::Status::kNotFound;
  auto item = model.items().find(*item_id);
  if (item == model.items().end())
    return lc::Status::kNotFound;
  if (!model.attachments().count(requesting))
    return lc::Status::kNotAttached;
  auto location = requesting;
  const auto profile = std::visit(
      [&](const auto& owner) -> lc::ProfileId {
        if constexpr (std::is_same_v<std::decay_t<decltype(owner)>,
                                     lc::SpaceId>) {
          location.space = owner;
          return model.spaces().at(owner).profile;
        } else {
          return owner;
        }
      },
      item->second.owner);
  if (model.spaces().at(requesting.space).profile != profile)
    return lc::Status::kProfileMismatch;
  std::optional<LiveReference> live;
  for (const auto& [id, page] : model.pages())
    if (const auto* saved = std::get_if<lc::SavedPage>(&page.owner))
      if (saved->item == *item_id)
        live = LiveReference{id, saved->attachment};
  return SidebarLocation{profile,
                         *item_id,
                         shortcut,
                         item->second.owner,
                         location,
                         model.attachments().count(location)
                             ? SidebarAttachment::kAttached
                             : SidebarAttachment::kRequiresAttachment,
                         live};
}

}  // namespace arcified::workspace_organizer
