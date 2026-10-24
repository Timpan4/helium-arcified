#include "command_bar.h"

#include <algorithm>
#include <iterator>
#include <set>
#include <type_traits>
#include <utility>

namespace arcified::commands {
namespace {

bool ValidAttachment(const lifecycle::Attachment& attachment) {
  return !attachment.window.value.empty() && !attachment.space.value.empty();
}

bool ValidContext(const Context& context) {
  return !context.profile.value.empty() &&
         ValidAttachment(context.attachment) &&
         (!context.page || !context.page->value.empty()) &&
         (!context.item || !context.item->value.empty());
}

bool ValidSavedItem(const SavedItemTarget& target) {
  return !target.item.value.empty() && !target.profile.value.empty() &&
         std::visit([](const auto& owner) { return !owner.value.empty(); },
                    target.owner);
}

bool ValidAction(const Action& action) {
  return std::visit(
      [](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, SwitchToTabAction>) {
          return !value.page.value.empty() && !value.profile.value.empty() &&
                 ValidAttachment(value.source) &&
                 (!value.item || !value.item->value.empty());
        } else if constexpr (std::is_same_v<T, OpenSavedItemAction>) {
          return ValidSavedItem(value.target);
        } else if constexpr (std::is_same_v<T, RevealInWorkspaceAction>) {
          return std::visit(
              [](const auto& target) {
                using Target = std::decay_t<decltype(target)>;
                if constexpr (std::is_same_v<Target, SavedItemTarget>) {
                  return ValidSavedItem(target);
                } else {
                  return !target.page.value.empty() &&
                         !target.profile.value.empty() &&
                         ValidAttachment(target.attachment);
                }
              },
              value.target);
        } else {
          if (!ValidContext(value.target))
            return false;
          if constexpr (std::is_same_v<T, NavigateAction>) {
            return !value.destination.empty();
          } else if constexpr (std::is_same_v<T, SearchAction>) {
            return !value.query.empty() &&
                   (!value.provider || !value.provider->value.empty());
          } else {
            return !value.command.value.empty();
          }
        }
      },
      action);
}

}  // namespace

ActionKind KindOf(const Action& action) {
  return std::visit(
      [](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, NavigateAction>) {
          return ActionKind::kNavigate;
        } else if constexpr (std::is_same_v<T, SearchAction>) {
          return ActionKind::kSearch;
        } else if constexpr (std::is_same_v<T, SwitchToTabAction>) {
          return ActionKind::kSwitchToTab;
        } else if constexpr (std::is_same_v<T, ExecuteCommandAction>) {
          return ActionKind::kExecuteCommand;
        } else if constexpr (std::is_same_v<T, OpenSavedItemAction>) {
          return ActionKind::kOpenSavedItem;
        } else {
          return ActionKind::kRevealInWorkspace;
        }
      },
      action);
}

BarStatus CommandBar::Invoke(RequestKey request,
                             Origin origin,
                             std::string query) {
  if (session_)
    return BarStatus::kAlreadyOpen;
  if (request.session.value.empty() || request.query.value.empty() ||
      origin.surface.value.empty() || !ValidContext(origin.context)) {
    return BarStatus::kInvalidInput;
  }
  session_ = Session{std::move(request),
                     std::move(origin),
                     std::move(query),
                     {},
                     std::nullopt};
  choose_first_ = true;
  return BarStatus::kOk;
}

BarStatus CommandBar::SetQuery(QueryToken token, std::string query) {
  if (!session_)
    return BarStatus::kClosed;
  if (token.value.empty() || token == session_->request.query)
    return BarStatus::kInvalidInput;
  session_->request.query = std::move(token);
  session_->query = std::move(query);
  session_->results.clear();
  session_->selected.reset();
  choose_first_ = true;
  return BarStatus::kOk;
}

BarStatus CommandBar::PublishResults(RequestKey request,
                                     std::vector<Result> results) {
  if (!session_)
    return BarStatus::kClosed;
  if (!(request == session_->request))
    return BarStatus::kStaleQuery;
  std::set<ResultId> ids;
  for (const auto& result : results) {
    if (result.id.value.empty() || result.title.empty() ||
        result.action_label.empty() || !ValidAction(result.action) ||
        !ids.insert(result.id).second) {
      return BarStatus::kInvalidInput;
    }
  }
  if (session_->selected && ids.find(*session_->selected) == ids.end()) {
    session_->selected.reset();
    choose_first_ = false;
  }
  if (choose_first_ && !results.empty()) {
    session_->selected = results.front().id;
    choose_first_ = false;
  }
  session_->results = std::move(results);
  return BarStatus::kOk;
}

BarStatus CommandBar::Select(ResultId result) {
  if (!session_)
    return BarStatus::kClosed;
  const auto found =
      std::find_if(session_->results.begin(), session_->results.end(),
                   [&result](const Result& row) { return row.id == result; });
  if (found == session_->results.end())
    return BarStatus::kNotFound;
  session_->selected = std::move(result);
  choose_first_ = false;
  return BarStatus::kOk;
}

BarStatus CommandBar::MoveSelection(SelectionDirection direction) {
  if (!session_)
    return BarStatus::kClosed;
  const auto& rows = session_->results;
  if (rows.empty())
    return BarStatus::kNoSelection;
  auto found =
      std::find_if(rows.begin(), rows.end(), [this](const Result& row) {
        return session_->selected && row.id == *session_->selected;
      });
  if (found == rows.end()) {
    found = direction == SelectionDirection::kNext ? rows.begin()
                                                   : std::prev(rows.end());
  } else if (direction == SelectionDirection::kNext) {
    if (std::next(found) != rows.end())
      ++found;
  } else if (found != rows.begin()) {
    --found;
  }
  session_->selected = found->id;
  choose_first_ = false;
  return BarStatus::kOk;
}

const Result* CommandBar::selected_result() const {
  if (!session_ || !session_->selected)
    return nullptr;
  const auto found = std::find_if(
      session_->results.begin(), session_->results.end(),
      [this](const Result& row) { return row.id == *session_->selected; });
  return found == session_->results.end() ? nullptr : &*found;
}

ActivationResult CommandBar::Activate() {
  if (!session_)
    return BarStatus::kClosed;
  const Result* result = selected_result();
  if (!result)
    return BarStatus::kNoSelection;
  ActivationIntent intent{session_->request, session_->origin, *result};
  session_.reset();
  choose_first_ = false;
  return intent;
}

std::optional<CancellationIntent> CommandBar::Escape() {
  if (!session_)
    return std::nullopt;
  CancellationIntent intent{session_->request, session_->origin,
                            session_->query};
  session_.reset();
  choose_first_ = false;
  return intent;
}

}  // namespace arcified::commands
