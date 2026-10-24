#include "ordered_navigation.h"

#include <algorithm>
#include <set>
#include <type_traits>

namespace arcified::ordered_navigation {
namespace {

using namespace lifecycle;

template <typename T>
bool Contains(const std::vector<T>& values, const T& value) {
  return std::find(values.begin(), values.end(), value) != values.end();
}

std::optional<TabDestination> ResolveRow(const Model& model,
                                         const SidebarSnapshot& snapshot,
                                         const Row& row) {
  const auto& profile = model.spaces().at(snapshot.scope.space).profile;
  std::optional<ItemId> item;
  std::optional<PageId> page;
  Attachment destination = snapshot.scope;
  if (const auto* id = std::get_if<ItemId>(&row.target)) {
    if (!Contains(*model.VisibleItems(snapshot.scope), *id))
      return std::nullopt;
    item = *id;
  } else if (const auto* alias = std::get_if<ShortcutId>(&row.target)) {
    if (!Contains(*model.VisibleShortcuts(snapshot.scope), *alias))
      return std::nullopt;
    item = model.shortcuts().at(*alias).target;
  } else {
    const auto& id = std::get<PageId>(row.target);
    const auto live = model.pages().find(id);
    if (live == model.pages().end() ||
        !(live->second.attachment() == snapshot.scope))
      return std::nullopt;
    page = id;
    if (const auto* saved = std::get_if<SavedPage>(&live->second.owner)) {
      // Saved rows normally use ItemId/ShortcutId. A PageId can identify a
      // presented split pane, but cannot bypass saved-item visibility.
      if (!row.split ||
          !Contains(*model.VisibleItems(snapshot.scope), saved->item))
        return std::nullopt;
      item = saved->item;
    }
  }
  if (item) {
    if (const auto* space =
            std::get_if<SpaceId>(&model.items().at(*item).owner))
      destination.space = *space;
    if (!model.attachments().count(destination) ||
        model.spaces().at(destination.space).profile != profile)
      return std::nullopt;
    for (const auto& [id, live] : model.pages()) {
      const auto* saved = std::get_if<SavedPage>(&live.owner);
      if (saved && saved->item == *item) {
        page = id;
        break;
      }
    }
  }
  std::optional<Attachment> live_attachment;
  if (page) {
    const auto& live = model.pages().at(*page);
    if (live.profile != profile)
      return std::nullopt;
    live_attachment = live.attachment();
  }
  if (row.split) {
    const auto split = model.splits().find(*row.split);
    if (split == model.splits().end() ||
        !(split->second.attachment == snapshot.scope) || !page ||
        !Contains(split->second.pages, *page))
      return std::nullopt;
  }
  return TabDestination{row, destination, profile, item, page, live_attachment};
}

std::optional<Refusal> Validate(const Model& model,
                                const SidebarSnapshot& snapshot) {
  const auto window = model.windows().find(snapshot.scope.window);
  if (!model.attachments().count(snapshot.scope) ||
      window == model.windows().end() ||
      window->second.selected_space != snapshot.scope.space)
    return Refusal::kInvalidScope;
  if (snapshot.revision.value.empty())
    return Refusal::kInvalidProjection;
  std::set<SpaceId> spaces;
  for (const auto& space : snapshot.space_order) {
    if (!model.attachments().count({snapshot.scope.window, space}) ||
        !spaces.insert(space).second)
      return Refusal::kInvalidProjection;
  }
  std::set<TabReference> targets;
  for (const auto& row : snapshot.tabs) {
    if (!targets.insert(row.target).second || !ResolveRow(model, snapshot, row))
      return Refusal::kInvalidProjection;
    std::set<FolderReference> ancestors;
    for (const auto& folder : row.collapsed_ancestor_ids) {
      if (folder.value.empty() || !ancestors.insert(folder).second)
        return Refusal::kInvalidProjection;
    }
  }
  return std::nullopt;
}

using IndexResult = std::variant<Refusal, std::size_t>;
IndexResult Step(std::size_t index,
                 std::size_t count,
                 Direction direction,
                 EdgeBehavior edge) {
  if (direction == Direction::kNext) {
    if (index != count - 1)
      return index + 1;
    if (edge == EdgeBehavior::kWrap)
      return std::size_t{0};
  } else {
    if (index != 0)
      return index - 1;
    if (edge == EdgeBehavior::kWrap)
      return count - 1;
  }
  return Refusal::kAtBoundary;
}

template <typename T>
IndexResult Locate(const std::vector<T>& order, const T& target) {
  const auto found = std::find(order.begin(), order.end(), target);
  if (found == order.end())
    return Refusal::kAnchorMissing;
  return static_cast<std::size_t>(found - order.begin());
}

bool SameRow(const Row& a, const Row& b) {
  return a.target == b.target &&
         a.collapsed_ancestor_ids == b.collapsed_ancestor_ids &&
         a.split == b.split;
}

bool SameDestination(const TabDestination& a, const TabDestination& b) {
  return SameRow(a.reveal_row, b.reveal_row) &&
         a.destination == b.destination && a.profile == b.profile &&
         a.item == b.item && a.live_page == b.live_page &&
         a.live_attachment == b.live_attachment;
}

}  // namespace

SelectionResult SelectTab(const Model& model,
                          const SidebarSnapshot& snapshot,
                          const TabRequest& request) {
  if (const auto error = Validate(model, snapshot))
    return *error;
  if (snapshot.tabs.empty())
    return Refusal::kEmpty;
  std::vector<TabReference> order;
  for (const auto& row : snapshot.tabs)
    order.push_back(row.target);
  const auto index = std::visit(
      [&](const auto& command) -> IndexResult {
        using T = std::decay_t<decltype(command)>;
        if constexpr (std::is_same_v<T, DirectIndex>) {
          if (command.index >= order.size())
            return Refusal::kIndexOutOfBounds;
          return command.index;
        } else if constexpr (std::is_same_v<T, DirectTab>) {
          auto located = Locate(order, command.target);
          if (std::holds_alternative<Refusal>(located))
            return Refusal::kNotVisible;
          return located;
        } else {
          auto anchor = Locate(order, command.anchor);
          if (std::holds_alternative<Refusal>(anchor))
            return std::get<Refusal>(anchor);
          return Step(std::get<std::size_t>(anchor), order.size(),
                      command.direction, command.edge);
        }
      },
      request);
  if (const auto* error = std::get_if<Refusal>(&index))
    return *error;
  return DestinationPlan{
      snapshot.revision, snapshot.scope,
      *ResolveRow(model, snapshot,
                  snapshot.tabs[std::get<std::size_t>(index)])};
}

SelectionResult SelectSpace(const Model& model,
                            const SidebarSnapshot& snapshot,
                            const SpaceRequest& request) {
  if (const auto error = Validate(model, snapshot))
    return *error;
  const auto& order = snapshot.space_order;
  if (order.empty())
    return Refusal::kEmpty;
  const auto index = std::visit(
      [&](const auto& command) -> IndexResult {
        using T = std::decay_t<decltype(command)>;
        if constexpr (std::is_same_v<T, DirectIndex>) {
          if (command.index >= order.size())
            return Refusal::kIndexOutOfBounds;
          return command.index;
        } else if constexpr (std::is_same_v<T, DirectSpace>) {
          auto located = Locate(order, command.space);
          if (std::holds_alternative<Refusal>(located))
            return Refusal::kNotVisible;
          return located;
        } else {
          auto anchor = Locate(order, snapshot.scope.space);
          if (std::holds_alternative<Refusal>(anchor))
            return std::get<Refusal>(anchor);
          return Step(std::get<std::size_t>(anchor), order.size(),
                      command.direction, command.edge);
        }
      },
      request);
  if (const auto* error = std::get_if<Refusal>(&index))
    return *error;
  const auto& space = order[std::get<std::size_t>(index)];
  return DestinationPlan{snapshot.revision, snapshot.scope,
                         SpaceDestination{{snapshot.scope.window, space},
                                          model.spaces().at(space).profile}};
}

SelectionResult Revalidate(const Model& model,
                           const SidebarSnapshot& current,
                           const DestinationPlan& plan) {
  if (current.revision != plan.revision || !(current.scope == plan.scope))
    return Refusal::kStaleSnapshot;
  if (const auto* tab = std::get_if<TabDestination>(&plan.destination)) {
    auto result = SelectTab(model, current, DirectTab{tab->reveal_row.target});
    const auto* fresh = std::get_if<DestinationPlan>(&result);
    if (!fresh ||
        !SameDestination(std::get<TabDestination>(fresh->destination), *tab))
      return Refusal::kStaleSnapshot;
    return result;
  }
  const auto& space = std::get<SpaceDestination>(plan.destination);
  auto result =
      SelectSpace(model, current, DirectSpace{space.destination.space});
  const auto* fresh = std::get_if<DestinationPlan>(&result);
  if (!fresh)
    return Refusal::kStaleSnapshot;
  const auto& destination = std::get<SpaceDestination>(fresh->destination);
  if (!(destination.destination == space.destination) ||
      destination.profile != space.profile)
    return Refusal::kStaleSnapshot;
  return result;
}

}  // namespace arcified::ordered_navigation
