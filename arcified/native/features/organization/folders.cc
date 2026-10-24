#include "folders.h"

#include <algorithm>
#include <utility>

namespace arcified::organization {
namespace {

FolderStatus CheckItem(const lifecycle::Model& model,
                       const lifecycle::ItemId& item,
                       const lifecycle::SpaceId& space) {
  const auto found = model.items().find(item);
  if (found == model.items().end())
    return FolderStatus::kItemNotFound;
  const auto* owner = std::get_if<lifecycle::SpaceId>(&found->second.owner);
  if (!owner || *owner != space)
    return FolderStatus::kItemOwnerMismatch;
  return FolderStatus::kOk;
}

}  // namespace

std::optional<Location> Hierarchy::Locate(const Entry& entry) const {
  auto found = std::find(roots.begin(), roots.end(), entry);
  if (found != roots.end())
    return Location{std::nullopt,
                    static_cast<std::size_t>(found - roots.begin())};
  for (const auto& [id, folder] : folders) {
    found = std::find(folder.children.begin(), folder.children.end(), entry);
    if (found != folder.children.end()) {
      return Location{
          id, static_cast<std::size_t>(found - folder.children.begin())};
    }
  }
  return std::nullopt;
}

std::optional<std::vector<FolderId>> Hierarchy::Ancestors(
    const Entry& entry) const {
  auto location = Locate(entry);
  if (!location)
    return std::nullopt;
  std::vector<FolderId> result;
  while (location->parent) {
    result.push_back(*location->parent);
    location = Locate(*location->parent);
    if (!location)
      return std::nullopt;
  }
  std::reverse(result.begin(), result.end());
  return result;
}

Folders::Trees::iterator Folders::FindFolder(const FolderId& id) {
  return std::find_if(trees_.begin(), trees_.end(), [&](const auto& tree) {
    return tree.second.folders.find(id) != tree.second.folders.end();
  });
}

Folders::Trees::const_iterator Folders::FindFolder(const FolderId& id) const {
  return std::find_if(trees_.begin(), trees_.end(), [&](const auto& tree) {
    return tree.second.folders.find(id) != tree.second.folders.end();
  });
}

FolderStatus Folders::Validate(const lifecycle::Model& model,
                               const lifecycle::SpaceId& space) const {
  if (model.spaces().find(space) == model.spaces().end())
    return FolderStatus::kSpaceNotFound;
  const auto tree = trees_.find(space);
  if (tree == trees_.end())
    return FolderStatus::kOk;
  const auto check = [&](const std::vector<Entry>& entries) {
    for (const auto& entry : entries) {
      if (const auto* item = std::get_if<lifecycle::ItemId>(&entry)) {
        const auto status = CheckItem(model, *item, space);
        if (status != FolderStatus::kOk)
          return status;
      }
    }
    return FolderStatus::kOk;
  };
  auto status = check(tree->second.roots);
  if (status != FolderStatus::kOk)
    return status;
  for (const auto& [id, folder] : tree->second.folders) {
    status = check(folder.children);
    if (status != FolderStatus::kOk)
      return status;
  }
  return FolderStatus::kOk;
}

FolderStatus Folders::ValidatePlacement(const Placement& placement) const {
  const auto tree = trees_.find(placement.container.space);
  const std::vector<Entry>* children = nullptr;
  if (placement.container.folder) {
    const auto folder_tree = FindFolder(*placement.container.folder);
    if (folder_tree == trees_.end())
      return FolderStatus::kNotFound;
    if (folder_tree->first != placement.container.space)
      return FolderStatus::kCrossSpaceUnsupported;
    children =
        &folder_tree->second.folders.at(*placement.container.folder).children;
  } else if (tree != trees_.end()) {
    children = &tree->second.roots;
  }
  if (placement.before &&
      (!children || std::find(children->begin(), children->end(),
                              *placement.before) == children->end())) {
    return FolderStatus::kNotFound;
  }
  return FolderStatus::kOk;
}

Hierarchy& Folders::EnsureTree(const lifecycle::SpaceId& space) {
  return trees_.try_emplace(space, Hierarchy{space, {}, {}}).first->second;
}

std::vector<Entry>& Folders::Children(Hierarchy& tree,
                                      const std::optional<FolderId>& folder) {
  return folder ? tree.folders.at(*folder).children : tree.roots;
}

void Folders::Relocate(Hierarchy& tree,
                       Entry entry,
                       const Placement& placement) {
  const auto old = tree.Locate(entry);
  if (old) {
    if (placement.before == std::optional<Entry>(entry))
      return;
    auto& source = Children(tree, old->parent);
    source.erase(source.begin() + old->index);
  }
  auto& destination = Children(tree, placement.container.folder);
  const auto position =
      placement.before
          ? std::find(destination.begin(), destination.end(), *placement.before)
          : destination.end();
  destination.insert(position, std::move(entry));
}

FolderStatus Folders::CreateFolder(const lifecycle::Model& model,
                                   FolderId id,
                                   std::string label,
                                   Placement placement) {
  if (id.value.empty() || label.empty())
    return FolderStatus::kInvalidInput;
  if (FindFolder(id) != trees_.end())
    return FolderStatus::kDuplicateId;
  auto status = Validate(model, placement.container.space);
  if (status != FolderStatus::kOk)
    return status;
  status = ValidatePlacement(placement);
  if (status != FolderStatus::kOk)
    return status;
  auto& tree = EnsureTree(placement.container.space);
  tree.folders.emplace(id, Folder{std::move(label), true, {}});
  Relocate(tree, std::move(id), placement);
  return FolderStatus::kOk;
}

FolderStatus Folders::Rename(const lifecycle::Model& model,
                             FolderId id,
                             std::string label) {
  if (label.empty())
    return FolderStatus::kInvalidInput;
  const auto tree = FindFolder(id);
  if (tree == trees_.end())
    return FolderStatus::kNotFound;
  const auto status = Validate(model, tree->first);
  if (status != FolderStatus::kOk)
    return status;
  tree->second.folders.at(id).label = std::move(label);
  return FolderStatus::kOk;
}

FolderStatus Folders::SetExpanded(const lifecycle::Model& model,
                                  FolderId id,
                                  bool expanded) {
  const auto tree = FindFolder(id);
  if (tree == trees_.end())
    return FolderStatus::kNotFound;
  const auto status = Validate(model, tree->first);
  if (status != FolderStatus::kOk)
    return status;
  tree->second.folders.at(id).expanded = expanded;
  return FolderStatus::kOk;
}

FolderStatus Folders::SetAllExpanded(const lifecycle::Model& model,
                                     lifecycle::SpaceId space,
                                     bool expanded) {
  const auto status = Validate(model, space);
  if (status != FolderStatus::kOk)
    return status;
  const auto tree = trees_.find(space);
  if (tree != trees_.end()) {
    for (auto& [id, folder] : tree->second.folders)
      folder.expanded = expanded;
  }
  return FolderStatus::kOk;
}

FolderStatus Folders::PlaceItem(const lifecycle::Model& model,
                                lifecycle::ItemId item,
                                Placement placement) {
  auto status = Validate(model, placement.container.space);
  if (status != FolderStatus::kOk)
    return status;
  status = CheckItem(model, item, placement.container.space);
  if (status != FolderStatus::kOk)
    return status;
  for (const auto& [space, tree] : trees_) {
    if (space != placement.container.space && tree.Locate(item))
      return FolderStatus::kCrossSpaceUnsupported;
  }
  status = ValidatePlacement(placement);
  if (status != FolderStatus::kOk)
    return status;
  Relocate(EnsureTree(placement.container.space), std::move(item), placement);
  return FolderStatus::kOk;
}

FolderStatus Folders::MoveFolder(const lifecycle::Model& model,
                                 FolderId id,
                                 Placement placement) {
  const auto tree = FindFolder(id);
  if (tree == trees_.end())
    return FolderStatus::kNotFound;
  if (tree->first != placement.container.space)
    return FolderStatus::kCrossSpaceUnsupported;
  auto status = Validate(model, tree->first);
  if (status != FolderStatus::kOk)
    return status;
  status = ValidatePlacement(placement);
  if (status != FolderStatus::kOk)
    return status;
  if (placement.container.folder) {
    const auto& target = *placement.container.folder;
    const auto ancestors = tree->second.Ancestors(target);
    if (target == id || std::find(ancestors->begin(), ancestors->end(), id) !=
                            ancestors->end()) {
      return FolderStatus::kCycle;
    }
  }
  Relocate(tree->second, std::move(id), placement);
  return FolderStatus::kOk;
}

SnapshotResult Folders::Snapshot(const lifecycle::Model& model,
                                 lifecycle::SpaceId space) const {
  const auto status = Validate(model, space);
  if (status != FolderStatus::kOk)
    return status;
  const auto tree = trees_.find(space);
  if (tree == trees_.end())
    return Hierarchy{std::move(space), {}, {}};
  return tree->second;
}

SnapshotResult Folders::ReadFolder(const lifecycle::Model& model,
                                   FolderId id) const {
  const auto tree = FindFolder(id);
  if (tree == trees_.end())
    return FolderStatus::kNotFound;
  return Snapshot(model, tree->first);
}

}  // namespace arcified::organization
