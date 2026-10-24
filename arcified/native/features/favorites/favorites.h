#ifndef ARCIFIED_NATIVE_FEATURES_FAVORITES_FAVORITES_H_
#define ARCIFIED_NATIVE_FEATURES_FAVORITES_FAVORITES_H_

#include "../../model/lifecycle.h"
#include "../pins/saved_item.h"

namespace arcified::favorites {

// Preserve the explicit reference identity, even for aliases of the same item.
using FavoriteReference =
    std::variant<lifecycle::ItemId, lifecycle::ShortcutId>;

enum class Refusal {
  kNotAttached,
  kNotFound,
  kNotFavorite,
  kProfileMismatch,
  kDuplicateReference,
  kIncompleteOrder,
};

// Owned read snapshot, not native authorization, selection or a commit token.
// ProfileId describes model scope, never regular/private native capability.
struct FavoriteTarget {
  FavoriteReference reference;
  lifecycle::ItemId item;
  lifecycle::ProfileId profile;
  lifecycle::Attachment destination;
  bool destination_attached;
};
using TargetResult = std::variant<Refusal, FavoriteTarget>;

struct FavoriteList {
  lifecycle::ProfileId profile;
  std::vector<FavoriteTarget> entries;
};
using ListResult = std::variant<Refusal, FavoriteList>;

// Resolve again after any model mutation before preparing a native command.
// A visible alias can require an explicit attach/switch action at destination.
TargetResult ResolveFavorite(const lifecycle::Model& model,
                             const lifecycle::Attachment& requesting,
                             FavoriteReference reference);

// Enumerates identities only. Traversal order is not user presentation order.
// No limit, deduplication by URL, page allocation or model mutation occurs.
ListResult EnumerateFavorites(const lifecycle::Model& model,
                              const lifecycle::Attachment& requesting);

// Caller owns presentation order. Reject missing, extra, duplicate, stale and
// cross-Profile references rather than silently repairing a partial snapshot.
ListResult OrderFavorites(const lifecycle::Model& model,
                          const lifecycle::Attachment& requesting,
                          const std::vector<FavoriteReference>& order);

struct FavoritePresentation {
  FavoriteTarget target;
  pins::SavedItemPresentation saved_item;
};
using PresentationResult =
    std::variant<Refusal, pins::SavedItemError, FavoritePresentation>;

// Validate Favorite scope before describing its canonical saved item. Caller
// looks up metadata by ResolveFavorite(...).item, never by alias identity.
// Label edits use pins::PlanLabelEdit/ApplyLabelEdit on that same ItemId.
PresentationResult DescribeFavorite(const lifecycle::Model& model,
                                    const lifecycle::Attachment& requesting,
                                    FavoriteReference reference,
                                    const pins::SavedItemMetadata& metadata,
                                    std::string default_label);

}  // namespace arcified::favorites

#endif  // ARCIFIED_NATIVE_FEATURES_FAVORITES_FAVORITES_H_
