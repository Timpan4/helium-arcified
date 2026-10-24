#include "saved_source.h"

#include <algorithm>
#include <map>
#include <optional>
#include <utility>

namespace arcified::library {
namespace {

using VisibleSources = std::map<lifecycle::ItemId, SavedSource>;

std::optional<VisibleSources> ProjectSources(
    const lifecycle::Model& model,
    const lifecycle::Attachment& attachment) {
  const auto items = model.VisibleItems(attachment);
  const auto shortcuts = model.VisibleShortcuts(attachment);
  if (!items || !shortcuts)
    return std::nullopt;

  VisibleSources sources;
  for (const auto& id : *items) {
    sources.emplace(id, std::holds_alternative<lifecycle::SpaceId>(
                            model.items().at(id).owner)
                            ? SavedSource::kKeptInSpace
                            : SavedSource::kProfileShortcut);
  }
  for (const auto& id : *shortcuts) {
    // Alias identity never creates another saved destination or live page.
    // Prefer direct visibility when the target already has a row.
    sources.try_emplace(model.shortcuts().at(id).target,
                        SavedSource::kShortcutAlias);
  }
  return sources;
}

unsigned char FoldAscii(unsigned char character) {
  return character >= 'A' && character <= 'Z' ? character + ('a' - 'A')
                                              : character;
}

bool Matches(std::string_view destination, std::string_view query) {
  return query.empty() ||
         std::search(destination.begin(), destination.end(), query.begin(),
                     query.end(), [](unsigned char a, unsigned char b) {
                       return FoldAscii(a) == FoldAscii(b);
                     }) != destination.end();
}

}  // namespace

SavedSearchResult SearchSaved(const lifecycle::Model& model,
                              const lifecycle::Attachment& attachment,
                              std::string_view query) {
  const auto sources = ProjectSources(model, attachment);
  if (!sources)
    return lifecycle::Status::kNotAttached;

  std::vector<SavedEntry> entries;
  for (const auto& [item, source] : *sources) {
    const auto& destination = model.items().at(item).saved_destination;
    if (Matches(destination, query))
      entries.push_back({item, source, destination});
  }
  return entries;
}

lifecycle::OpenResult OpenSavedResult(lifecycle::Model& model,
                                      lifecycle::Attachment attachment,
                                      lifecycle::ItemId item,
                                      lifecycle::PageId new_page) {
  const auto sources = ProjectSources(model, attachment);
  if (!sources)
    return lifecycle::Status::kNotAttached;
  if (!sources->count(item))
    return lifecycle::Status::kNotFound;
  return model.OpenItem(std::move(item), std::move(attachment),
                        std::move(new_page));
}

}  // namespace arcified::library
