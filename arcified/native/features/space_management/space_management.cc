#include "space_management.h"

#include <map>
#include <set>
#include <utility>

namespace arcified::space_management {
namespace {

std::optional<MetadataError> ValidateOrder(
    const lifecycle::Model& model,
    const std::vector<lifecycle::SpaceId>& order) {
  std::set<lifecycle::SpaceId> seen;
  for (const auto& space : order) {
    if (!seen.insert(space).second)
      return MetadataError{MetadataErrorCode::kDuplicateSpace, space};
    if (!model.spaces().count(space))
      return MetadataError{MetadataErrorCode::kUnknownSpace, space};
  }
  for (const auto& [space, value] : model.spaces()) {
    if (!seen.count(space))
      return MetadataError{MetadataErrorCode::kMissingSpace, space};
  }
  return std::nullopt;
}

}  // namespace

std::optional<MetadataError> ValidateMetadata(const lifecycle::Model& model,
                                              const Metadata& metadata) {
  std::set<lifecycle::SpaceId> seen;
  for (const auto& entry : metadata) {
    if (!seen.insert(entry.space).second)
      return MetadataError{MetadataErrorCode::kDuplicateSpace, entry.space};
    if (!model.spaces().count(entry.space))
      return MetadataError{MetadataErrorCode::kUnknownSpace, entry.space};
  }
  for (const auto& [space, value] : model.spaces()) {
    if (!seen.count(space))
      return MetadataError{MetadataErrorCode::kMissingSpace, space};
  }
  return std::nullopt;
}

ReconciliationResult ReconcileMetadata(const lifecycle::Model& model,
                                       Metadata metadata) {
  std::set<lifecycle::SpaceId> seen;
  for (const auto& entry : metadata) {
    if (!seen.insert(entry.space).second)
      return MetadataError{MetadataErrorCode::kDuplicateSpace, entry.space};
  }

  Reconciliation result;
  for (auto& entry : metadata) {
    if (model.spaces().count(entry.space))
      result.metadata.push_back(std::move(entry));
    else
      result.removed.push_back(std::move(entry));
  }
  for (const auto& [space, value] : model.spaces()) {
    if (!seen.count(space)) {
      result.metadata.push_back({space, {}});
      result.added.push_back(space);
    }
  }
  return result;
}

MetadataResult ReplaceIdentity(const lifecycle::Model& model,
                               Metadata metadata,
                               lifecycle::SpaceId space,
                               DisplayIdentity identity) {
  if (auto error = ValidateMetadata(model, metadata))
    return *error;
  for (auto& entry : metadata) {
    if (entry.space == space) {
      entry.identity = std::move(identity);
      return metadata;
    }
  }
  return MetadataError{MetadataErrorCode::kUnknownSpace, std::move(space)};
}

MetadataResult ReorderSpaces(const lifecycle::Model& model,
                             Metadata metadata,
                             std::vector<lifecycle::SpaceId> order) {
  if (auto error = ValidateMetadata(model, metadata))
    return *error;
  if (auto error = ValidateOrder(model, order))
    return *error;
  std::map<lifecycle::SpaceId, DisplayIdentity> identities;
  for (auto& entry : metadata)
    identities.emplace(std::move(entry.space), std::move(entry.identity));
  Metadata result;
  for (auto& space : order) {
    auto identity = std::move(identities.at(space));
    result.push_back({std::move(space), std::move(identity)});
  }
  return result;
}

std::optional<SpaceReferences> InspectReferences(const lifecycle::Model& model,
                                                 lifecycle::SpaceId space) {
  if (!model.spaces().count(space))
    return std::nullopt;
  SpaceReferences result{std::move(space), {}, {}, {}, {}, {}, {}};
  std::set<lifecycle::ItemId> kept;
  for (const auto& [id, item] : model.items()) {
    const auto* owner = std::get_if<lifecycle::SpaceId>(&item.owner);
    if (owner && *owner == result.space) {
      result.kept_items.push_back(id);
      kept.insert(id);
    }
  }
  for (const auto& [id, page] : model.pages()) {
    if (page.attachment().space == result.space)
      result.attached_pages.push_back(id);
  }
  for (const auto& [id, shortcut] : model.shortcuts()) {
    if (kept.count(shortcut.target))
      result.aliases_of_kept_items.push_back(id);
  }
  for (const auto& attachment : model.attachments()) {
    if (attachment.space == result.space)
      result.attachments.push_back(attachment);
  }
  for (const auto& [id, split] : model.splits()) {
    if (split.attachment.space == result.space)
      result.splits.push_back(id);
  }
  for (const auto& [id, window] : model.windows()) {
    if (window.selected_space == result.space)
      result.selected_by.push_back(id);
  }
  return result;
}

}  // namespace arcified::space_management
