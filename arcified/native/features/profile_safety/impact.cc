#include "impact.h"

#include <set>
#include <utility>

namespace arcified::profile_safety {
namespace {

ProfileContext Context(const lifecycle::Model& model,
                       const lifecycle::ProfileId& profile) {
  ProfileContext context{profile, {}, {}};
  for (const auto& [id, item] : model.items()) {
    const auto* owner = std::get_if<lifecycle::ProfileId>(&item.owner);
    if (owner && *owner == profile)
      context.standalone_items.push_back(id);
  }
  for (const auto& [id, alias] : model.shortcuts()) {
    if (alias.profile == profile)
      context.aliases.push_back({id, alias.profile, alias.target});
  }
  return context;
}

}  // namespace

PreviewResult Preview(const lifecycle::Model& model, Request request) {
  // Only requests cross this module's boundary. Model invariants are owned by
  // lifecycle::Model; no shadow graph validation or ownership is introduced.
  std::set<lifecycle::SpaceId> spaces;
  std::optional<lifecycle::ProfileId> deleting_profile;
  if (const auto* deletion = std::get_if<DeleteProfile>(&request)) {
    if (!model.profiles().count(deletion->profile))
      return PreviewError::kProfileNotFound;
    deleting_profile = deletion->profile;
    for (const auto& [id, space] : model.spaces()) {
      if (space.profile == *deleting_profile)
        spaces.insert(id);
    }
  } else {
    const auto& space = std::holds_alternative<ReassignSpace>(request)
                            ? std::get<ReassignSpace>(request).space
                            : std::get<DeleteSpace>(request).space;
    if (!model.spaces().count(space))
      return PreviewError::kSpaceNotFound;
    if (const auto* reassignment = std::get_if<ReassignSpace>(&request)) {
      if (!model.profiles().count(reassignment->target))
        return PreviewError::kProfileNotFound;
    }
    spaces.insert(space);
  }

  Impact impact{std::move(request), {}, {}, {}, {}, {}, {}, {}, {}};
  for (const auto& id : spaces)
    impact.spaces.push_back({id, model.spaces().at(id).profile});
  for (const auto& attachment : model.attachments()) {
    if (spaces.count(attachment.space))
      impact.attachments.push_back(attachment);
  }
  for (const auto& [id, window] : model.windows()) {
    if (window.selected_space && spaces.count(*window.selected_space))
      impact.selections.push_back({id, *window.selected_space});
  }

  std::set<lifecycle::ItemId> items;
  for (const auto& [id, item] : model.items()) {
    const auto* space = std::get_if<lifecycle::SpaceId>(&item.owner);
    const auto* profile = std::get_if<lifecycle::ProfileId>(&item.owner);
    if ((space && spaces.count(*space)) ||
        (profile && deleting_profile == *profile)) {
      items.insert(id);
      impact.items.push_back({id, item.owner});
    }
  }
  for (const auto& [id, alias] : model.shortcuts()) {
    if (items.count(alias.target) || deleting_profile == alias.profile)
      impact.aliases.push_back({id, alias.profile, alias.target});
  }

  std::set<lifecycle::PageId> pages;
  for (const auto& [id, page] : model.pages()) {
    const auto* saved = std::get_if<lifecycle::SavedPage>(&page.owner);
    if (spaces.count(page.attachment().space) ||
        deleting_profile == page.profile ||
        (saved && items.count(saved->item))) {
      pages.insert(id);
      impact.pages.push_back({id, page.profile, page.owner, page.activity});
    }
  }
  for (const auto& [id, split] : model.splits()) {
    bool referenced = spaces.count(split.attachment.space) != 0;
    for (const auto& page : split.pages)
      referenced = referenced || pages.count(page) != 0;
    if (referenced)
      impact.splits.push_back({id, split.attachment, split.pages});
  }

  if (const auto* reassignment = std::get_if<ReassignSpace>(&impact.request)) {
    const auto& source = model.spaces().at(reassignment->space).profile;
    impact.context_only.push_back(Context(model, source));
    if (source != reassignment->target)
      impact.context_only.push_back(Context(model, reassignment->target));
  }
  return impact;
}

}  // namespace arcified::profile_safety
