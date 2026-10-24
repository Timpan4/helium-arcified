#include "multi_select.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <utility>

namespace arcified::organization {
namespace {

std::optional<std::set<SelectionRowId>> RowSet(
    const std::vector<SelectionRowId>& rows) {
  std::set<SelectionRowId> ids;
  for (const auto& row : rows) {
    if (std::visit([](const auto& id) { return id.value.empty(); }, row) ||
        !ids.insert(row).second) {
      return std::nullopt;
    }
  }
  return ids;
}

bool IsCurrent(const std::set<SelectionRowId>& rows,
               const Selection& selection) {
  return (!selection.anchor || rows.count(*selection.anchor)) &&
         std::all_of(selection.selected.begin(), selection.selected.end(),
                     [&](const auto& id) { return rows.count(id); });
}

SelectionRowId RowId(const Entry& entry) {
  return std::visit([](const auto& id) -> SelectionRowId { return id; }, entry);
}

// Iterative preorder avoids introducing a nesting limit or recursion policy.
std::map<SelectionRowId, std::size_t> CanonicalPositions(
    const Hierarchy& tree) {
  std::vector<Entry> pending(tree.roots.rbegin(), tree.roots.rend());
  std::map<SelectionRowId, std::size_t> positions;
  while (!pending.empty()) {
    Entry entry = std::move(pending.back());
    pending.pop_back();
    positions.emplace(RowId(entry), positions.size());
    if (const auto* folder = std::get_if<FolderId>(&entry)) {
      const auto& children = tree.folders.at(*folder).children;
      pending.insert(pending.end(), children.rbegin(), children.rend());
    }
  }
  return positions;
}

bool SelectedAncestor(const Hierarchy& tree,
                      const Entry& entry,
                      const Selection& selection) {
  const auto ancestors = tree.Ancestors(entry);
  return ancestors &&
         std::any_of(ancestors->begin(), ancestors->end(), [&](const auto& id) {
           return selection.selected.count(id);
         });
}

bool ContainsEntry(const std::vector<Entry>& entries, const Entry& entry) {
  return std::find(entries.begin(), entries.end(), entry) != entries.end();
}

}  // namespace

SelectionResult ApplySelection(const std::vector<SelectionRowId>& rows,
                               const Selection& selection,
                               SelectionRowId target,
                               SelectionGesture gesture) {
  const auto ids = RowSet(rows);
  if (!ids)
    return SelectionError::kInvalidRows;
  if (!IsCurrent(*ids, selection))
    return SelectionError::kStaleSelection;
  if (!ids->count(target))
    return SelectionError::kTargetNotFound;

  Selection result = selection;
  switch (gesture) {
    case SelectionGesture::kReplace:
      result.selected = {target};
      result.anchor = target;
      break;
    case SelectionGesture::kToggle:
      if (!result.selected.erase(target))
        result.selected.insert(target);
      result.anchor = target;
      break;
    case SelectionGesture::kRange:
    case SelectionGesture::kAddRange: {
      if (gesture == SelectionGesture::kRange)
        result.selected.clear();
      if (!result.anchor)
        result.anchor = target;
      auto first = std::find(rows.begin(), rows.end(), *result.anchor);
      auto last = std::find(rows.begin(), rows.end(), target);
      if (last < first)
        std::swap(first, last);
      result.selected.insert(first, last + 1);
      break;
    }
  }
  return result;
}

SelectionResult ReconcileSelection(const std::vector<SelectionRowId>& rows,
                                   const Selection& selection) {
  const auto ids = RowSet(rows);
  if (!ids)
    return SelectionError::kInvalidRows;
  Selection result;
  std::set_intersection(ids->begin(), ids->end(), selection.selected.begin(),
                        selection.selected.end(),
                        std::inserter(result.selected, result.selected.end()));
  if (selection.anchor && ids->count(*selection.anchor))
    result.anchor = selection.anchor;
  return result;
}

SelectedRowsResult ProjectSelectedRows(const std::vector<SelectionRowId>& rows,
                                       const Selection& selection) {
  const auto ids = RowSet(rows);
  if (!ids)
    return SelectionError::kInvalidRows;
  if (!IsCurrent(*ids, selection))
    return SelectionError::kStaleSelection;
  std::vector<SelectedRow> result;
  for (const auto& row : rows)
    result.push_back({row, selection.selected.count(row) != 0});
  return result;
}

SelectionMoveResult PlanSelectionMove(const lifecycle::Model& model,
                                      const Folders& folders,
                                      lifecycle::SpaceId source_space,
                                      const std::vector<SelectionRowId>& rows,
                                      const Selection& selection,
                                      Placement destination) {
  const auto ids = RowSet(rows);
  if (!ids)
    return SelectionError::kInvalidRows;
  if (!IsCurrent(*ids, selection))
    return SelectionError::kStaleSelection;
  if (selection.selected.empty())
    return SelectionError::kEmptySelection;
  for (const auto& row : selection.selected) {
    if (std::holds_alternative<lifecycle::TemporaryTabId>(row))
      return SelectionError::kTemporaryMoveUnsupported;
  }

  const auto snapshot = folders.Snapshot(model, source_space);
  if (const auto* error = std::get_if<FolderStatus>(&snapshot))
    return *error;
  const auto& tree = std::get<Hierarchy>(snapshot);
  const auto positions = CanonicalPositions(tree);
  std::optional<std::size_t> previous;
  bool saw_unplaced = false;
  std::vector<Entry> moving;
  for (const auto& row : rows) {
    if (std::holds_alternative<lifecycle::TemporaryTabId>(row))
      continue;
    Entry entry = std::holds_alternative<FolderId>(row)
                      ? Entry{std::get<FolderId>(row)}
                      : Entry{std::get<lifecycle::ItemId>(row)};
    const auto position = positions.find(row);
    if (position != positions.end()) {
      if (saw_unplaced || (previous && position->second <= *previous))
        return SelectionError::kOrderMismatch;
      previous = position->second;
    } else {
      const auto* item = std::get_if<lifecycle::ItemId>(&entry);
      if (!item)
        return SelectionError::kSourceMismatch;
      const auto found = model.items().find(*item);
      if (found == model.items().end() ||
          found->second.owner != lifecycle::ItemOwner{source_space}) {
        return SelectionError::kSourceMismatch;
      }
      saw_unplaced = true;
    }
    if (selection.selected.count(row) &&
        !SelectedAncestor(tree, entry, selection)) {
      moving.push_back(entry);
    }
  }

  const auto source = model.spaces().find(source_space);
  const auto target = model.spaces().find(destination.container.space);
  if (target == model.spaces().end())
    return SelectionError::kInvalidDestination;
  if (source->second.profile != target->second.profile)
    return SelectionError::kCrossProfileMigrationUnsupported;
  if (source_space != destination.container.space) {
    for (const auto& [page_id, page] : model.pages()) {
      const auto* saved = std::get_if<lifecycle::SavedPage>(&page.owner);
      if (saved && (ContainsEntry(moving, saved->item) ||
                    SelectedAncestor(tree, saved->item, selection))) {
        return SelectionError::kLiveItemOwnershipMoveUnsupported;
      }
    }
    return SelectionError::kCrossSpaceUnsupported;
  }

  const std::vector<Entry>* children = &tree.roots;
  if (destination.container.folder) {
    const auto folder = tree.folders.find(*destination.container.folder);
    if (folder == tree.folders.end())
      return SelectionError::kInvalidDestination;
    const Entry entry{folder->first};
    if (selection.selected.count(folder->first) ||
        SelectedAncestor(tree, entry, selection)) {
      return SelectionError::kDestinationInsideSelection;
    }
    children = &folder->second.children;
  }
  auto before = children->end();
  if (destination.before) {
    before = std::find(children->begin(), children->end(), *destination.before);
    if (before == children->end())
      return SelectionError::kInvalidDestination;
  }
  // Resolve the anchor against the original sibling sequence before removing
  // moving roots. This keeps self/adjacent drops stable without index guesses.
  while (before != children->end() && ContainsEntry(moving, *before))
    ++before;
  destination.before =
      before == children->end() ? std::nullopt : std::optional<Entry>{*before};
  std::vector<Entry> preview;
  for (const auto& child : *children) {
    if (!ContainsEntry(moving, child))
      preview.push_back(child);
  }
  const auto insertion =
      destination.before
          ? std::find(preview.begin(), preview.end(), *destination.before)
          : preview.end();
  preview.insert(insertion, moving.begin(), moving.end());
  return SelectionMovePlan{std::move(source_space), std::move(moving),
                           std::move(destination), std::move(preview)};
}

}  // namespace arcified::organization
