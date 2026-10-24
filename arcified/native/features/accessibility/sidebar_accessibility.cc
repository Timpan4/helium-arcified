#include "sidebar_accessibility.h"

#include <map>
#include <set>
#include <utility>

namespace arcified::accessibility {
namespace {

bool SameCommands(const std::vector<Command>& a,
                  const std::vector<Command>& b) {
  if (a.size() != b.size())
    return false;
  for (size_t i = 0; i < a.size(); ++i) {
    if (!(a[i].id == b[i].id) || a[i].name != b[i].name ||
        a[i].enabled != b[i].enabled) {
      return false;
    }
  }
  return true;
}

}  // namespace

Projection::Projection(const std::vector<Row>& rows,
                       std::vector<Position> positions,
                       std::map<RowId, size_t> indices)
    : rows_(&rows),
      positions_(std::move(positions)),
      indices_(std::move(indices)) {}

std::optional<Projection> Projection::Borrow(const std::vector<Row>& rows) {
  std::map<RowId, size_t> indices;
  std::map<std::optional<RowId>, size_t> siblings;
  std::vector<size_t> ancestors;
  std::vector<Position> positions;
  for (size_t i = 0; i < rows.size(); ++i) {
    const Row& row = rows[i];
    if (row.id.value.empty() || row.name.empty() ||
        !indices.emplace(row.id, i).second ||
        (row.kind == RowKind::kTab && row.expanded.has_value())) {
      return std::nullopt;
    }
    std::set<CommandId> commands;
    for (const Command& command : row.commands) {
      if (command.id.value.empty() || command.name.empty() ||
          !commands.insert(command.id).second) {
        return std::nullopt;
      }
    }
    std::optional<size_t> parent;
    if (row.parent) {
      while (!ancestors.empty() &&
             !(rows[ancestors.back()].id == *row.parent)) {
        ancestors.pop_back();
      }
      if (ancestors.empty() ||
          !rows[ancestors.back()].expanded.value_or(false)) {
        return std::nullopt;
      }
      parent = ancestors.back();
    } else {
      ancestors.clear();
    }
    positions.push_back(
        {parent, ancestors.size() + 1, ++siblings[row.parent], 0});
    ancestors.push_back(i);
  }
  for (size_t i = 0; i < rows.size(); ++i)
    positions[i].set_size = siblings.at(rows[i].parent);
  return Projection(rows, std::move(positions), std::move(indices));
}

std::optional<size_t> Projection::Find(const RowId& row) const {
  const auto found = indices_.find(row);
  return found == indices_.end() ? std::nullopt
                                 : std::optional<size_t>(found->second);
}

std::optional<RowSemantics> Projection::Describe(const RowId& row) const {
  const auto index = Find(row);
  if (!index)
    return std::nullopt;
  const Position& position = positions_[*index];
  return RowSemantics{&(*rows_)[*index], position.level, position.position,
                      position.set_size};
}

KeyResult Projection::OnKey(const RowId& focused,
                            TreeKey key,
                            TextDirection direction) const {
  const auto index = Find(focused);
  if (!index)
    return std::monostate{};
  const Row& row = (*rows_)[*index];
  const auto focus = [this](size_t target) -> KeyResult {
    return FocusSuggestion{(*rows_)[target].id};
  };
  switch (key) {
    case TreeKey::kUp:
      return *index > 0 ? focus(*index - 1) : KeyResult{};
    case TreeKey::kDown:
      return *index + 1 < rows_->size() ? focus(*index + 1) : KeyResult{};
    case TreeKey::kHome:
      return focus(0);
    case TreeKey::kEnd:
      return focus(rows_->size() - 1);
    case TreeKey::kLeft:
    case TreeKey::kRight:
      break;
  }
  const bool expand =
      (key == TreeKey::kRight) == (direction == TextDirection::kLeftToRight);
  if (expand) {
    if (row.expanded == false)
      return ExpansionRequest{row.id, true};
    if (*index + 1 < rows_->size() && positions_[*index + 1].parent == index) {
      return focus(*index + 1);
    }
  } else {
    if (row.expanded == true)
      return ExpansionRequest{row.id, false};
    if (positions_[*index].parent)
      return focus(*positions_[*index].parent);
  }
  return std::monostate{};
}

std::optional<CommandRequest> Projection::RequestCommand(
    const RowId& row,
    const CommandId& command) const {
  const auto index = Find(row);
  if (!index)
    return std::nullopt;
  for (const Command& candidate : (*rows_)[*index].commands) {
    if (candidate.id == command && candidate.enabled)
      return CommandRequest{(*rows_)[*index].id, candidate.id};
  }
  return std::nullopt;
}

std::optional<FocusSuggestion> Projection::SuggestFocusAfterChange(
    const Projection& before,
    const std::optional<RowId>& focused) const {
  if (!focused)
    return std::nullopt;
  if (const auto surviving = Find(*focused))
    return FocusSuggestion{(*rows_)[*surviving].id};
  auto old = before.Find(*focused);
  while (old) {
    const auto parent = before.positions_[*old].parent;
    for (size_t i = *old + 1; i < before.rows_->size(); ++i) {
      if (before.positions_[i].parent == parent &&
          Find((*before.rows_)[i].id)) {
        return FocusSuggestion{(*before.rows_)[i].id};
      }
    }
    for (size_t i = *old; i > 0; --i) {
      if (before.positions_[i - 1].parent == parent &&
          Find((*before.rows_)[i - 1].id)) {
        return FocusSuggestion{(*before.rows_)[i - 1].id};
      }
    }
    if (parent && Find((*before.rows_)[*parent].id))
      return FocusSuggestion{(*before.rows_)[*parent].id};
    old = parent;
  }
  return std::nullopt;
}

std::vector<SemanticChange> Projection::ChangesSince(
    const Projection& before) const {
  std::vector<SemanticChange> changes;
  for (const Row& row : *before.rows_) {
    if (!Find(row.id))
      changes.push_back({row.id, ChangeKind::kRemoved});
  }
  for (const Row& row : *rows_) {
    const auto old = before.Describe(row.id);
    if (!old) {
      changes.push_back({row.id, ChangeKind::kAdded});
      continue;
    }
    const auto now = *Describe(row.id);
    if (row.name != old->row->name)
      changes.push_back({row.id, ChangeKind::kName});
    if (row.kind != old->row->kind)
      changes.push_back({row.id, ChangeKind::kKind});
    if (row.parent != old->row->parent || now.level != old->level ||
        now.position != old->position || now.set_size != old->set_size) {
      changes.push_back({row.id, ChangeKind::kHierarchy});
    }
    if (row.selected != old->row->selected)
      changes.push_back({row.id, ChangeKind::kSelection});
    if (row.expanded != old->row->expanded)
      changes.push_back({row.id, ChangeKind::kExpansion});
    if (!SameCommands(row.commands, old->row->commands))
      changes.push_back({row.id, ChangeKind::kCommands});
  }
  return changes;
}

}  // namespace arcified::accessibility
