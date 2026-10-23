#include "folder_preview.h"

#include <utility>

namespace arcified::organization {

PreviewResult ProjectFolderPreview(
    const lifecycle::Model& model,
    const Folders& folders,
    FolderId folder,
    lifecycle::Attachment context,
    const std::map<lifecycle::ItemId, std::string>& item_labels,
    std::string_view query,
    std::optional<lifecycle::PageId> active_page) {
  if (!model.attachments().count(context))
    return PreviewError::kNotAttached;
  auto snapshot = folders.Snapshot(model, context.space);
  if (const auto* error = std::get_if<FolderStatus>(&snapshot))
    return *error;
  const auto& hierarchy = std::get<Hierarchy>(snapshot);
  const auto root = hierarchy.folders.find(folder);
  if (root == hierarchy.folders.end())
    return PreviewError::kFolderNotFound;

  if (active_page) {
    const auto page = model.pages().find(*active_page);
    if (page == model.pages().end())
      return PreviewError::kActivePageNotFound;
    if (!(page->second.attachment() == context))
      return PreviewError::kActivePageOutsideAttachment;
  }

  // Join existing live identities without retaining navigation or ownership.
  std::map<lifecycle::ItemId, lifecycle::PageId> live_pages;
  for (const auto& [page_id, page] : model.pages()) {
    if (const auto* saved = std::get_if<lifecycle::SavedPage>(&page.owner))
      live_pages.emplace(saved->item, page_id);
  }

  std::vector<FolderBreadcrumb> ancestry;
  auto ancestors = *hierarchy.Ancestors(Entry{folder});
  ancestors.push_back(folder);
  for (const auto& ancestor : ancestors) {
    const auto& value = hierarchy.folders.at(ancestor);
    ancestry.push_back({ancestor, value.label, value.expanded});
  }

  // Iterative traversal has no call-stack depth limit for nested folders.
  struct Frame {
    FolderId folder;
    std::size_t next = 0;
  };
  std::vector<Frame> stack{{folder}};
  FolderPreview result;
  while (!stack.empty()) {
    auto& frame = stack.back();
    const auto& children = hierarchy.folders.at(frame.folder).children;
    if (frame.next == children.size()) {
      stack.pop_back();
      ancestry.pop_back();
      continue;
    }
    const auto index = frame.next++;
    const auto& entry = children[index];
    if (const auto* child = std::get_if<FolderId>(&entry)) {
      const auto& value = hierarchy.folders.at(*child);
      ancestry.push_back({*child, value.label, value.expanded});
      stack.push_back({*child});
      continue;
    }

    const auto& item = std::get<lifecycle::ItemId>(entry);
    const auto label = item_labels.find(item);
    if (label == item_labels.end())
      return PreviewError::kMissingItemLabel;
    const auto live = live_pages.find(item);
    std::optional<lifecycle::PageId> page;
    if (live != live_pages.end())
      page = live->second;
    PreviewRow row{item,
                   page,
                   label->second,
                   ancestry,
                   {frame.folder, index},
                   page && page == active_page};
    if (!root->second.expanded && row.active)
      result.active_descendant = row;
    if (row.label.find(query) != std::string::npos)
      result.matches.push_back(std::move(row));
  }
  return result;
}

}  // namespace arcified::organization
