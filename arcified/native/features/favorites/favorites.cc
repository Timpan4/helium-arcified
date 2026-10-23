#include "favorites.h"

#include <set>
#include <type_traits>

namespace arcified::favorites {

TargetResult ResolveFavorite(const lifecycle::Model& model,
                             const lifecycle::Attachment& requesting,
                             FavoriteReference reference) {
  if (!model.attachments().count(requesting))
    return Refusal::kNotAttached;
  const auto& profile = model.spaces().at(requesting.space).profile;
  const auto resolved = std::visit(
      [&](const auto& id) -> std::variant<Refusal, lifecycle::ItemId> {
        using T = std::decay_t<decltype(id)>;
        if constexpr (std::is_same_v<T, lifecycle::ItemId>) {
          const auto saved = model.items().find(id);
          if (saved == model.items().end())
            return Refusal::kNotFound;
          const auto* owner =
              std::get_if<lifecycle::ProfileId>(&saved->second.owner);
          if (!owner)
            return Refusal::kNotFavorite;
          if (*owner != profile)
            return Refusal::kProfileMismatch;
          return id;
        } else {
          const auto alias = model.shortcuts().find(id);
          if (alias == model.shortcuts().end())
            return Refusal::kNotFound;
          if (alias->second.profile != profile)
            return Refusal::kProfileMismatch;
          return alias->second.target;
        }
      },
      reference);
  if (const auto* refusal = std::get_if<Refusal>(&resolved))
    return *refusal;
  const auto& item = std::get<lifecycle::ItemId>(resolved);
  lifecycle::Attachment destination = requesting;
  if (const auto* owner =
          std::get_if<lifecycle::SpaceId>(&model.items().at(item).owner)) {
    destination.space = *owner;
  }
  return FavoriteTarget{std::move(reference), item, profile, destination,
                        model.attachments().count(destination) != 0};
}

ListResult EnumerateFavorites(const lifecycle::Model& model,
                              const lifecycle::Attachment& requesting) {
  if (!model.attachments().count(requesting))
    return Refusal::kNotAttached;
  FavoriteList result{model.spaces().at(requesting.space).profile, {}};
  for (const auto& [id, item] : model.items()) {
    const auto* owner = std::get_if<lifecycle::ProfileId>(&item.owner);
    if (owner && *owner == result.profile)
      result.entries.push_back(
          std::get<FavoriteTarget>(ResolveFavorite(model, requesting, id)));
  }
  for (const auto& [id, alias] : model.shortcuts()) {
    if (alias.profile == result.profile)
      result.entries.push_back(
          std::get<FavoriteTarget>(ResolveFavorite(model, requesting, id)));
  }
  return result;
}

ListResult OrderFavorites(const lifecycle::Model& model,
                          const lifecycle::Attachment& requesting,
                          const std::vector<FavoriteReference>& order) {
  auto all = EnumerateFavorites(model, requesting);
  if (const auto* refusal = std::get_if<Refusal>(&all))
    return *refusal;
  FavoriteList result{std::get<FavoriteList>(all).profile, {}};
  std::set<FavoriteReference> distinct;
  for (const auto& reference : order) {
    if (!distinct.insert(reference).second)
      return Refusal::kDuplicateReference;
    auto target = ResolveFavorite(model, requesting, reference);
    if (const auto* refusal = std::get_if<Refusal>(&target))
      return *refusal;
    result.entries.push_back(std::get<FavoriteTarget>(std::move(target)));
  }
  if (result.entries.size() != std::get<FavoriteList>(all).entries.size())
    return Refusal::kIncompleteOrder;
  return result;
}

PresentationResult DescribeFavorite(const lifecycle::Model& model,
                                    const lifecycle::Attachment& requesting,
                                    FavoriteReference reference,
                                    const pins::SavedItemMetadata& metadata,
                                    std::string default_label) {
  auto target = ResolveFavorite(model, requesting, std::move(reference));
  if (const auto* refusal = std::get_if<Refusal>(&target))
    return *refusal;
  auto resolved = std::get<FavoriteTarget>(std::move(target));
  auto saved = pins::DescribeSavedItem(model, resolved.item, metadata,
                                       std::move(default_label));
  if (const auto* error = std::get_if<pins::SavedItemError>(&saved))
    return *error;
  return FavoritePresentation{
      std::move(resolved),
      std::get<pins::SavedItemPresentation>(std::move(saved))};
}

}  // namespace arcified::favorites
