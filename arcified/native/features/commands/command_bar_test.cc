#include "command_bar.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <utility>

namespace arcified::commands {
namespace {

static_assert(lifecycle::kContractVersion == 3);
static_assert(!std::is_convertible_v<SessionToken, QueryToken>);
static_assert(!std::is_convertible_v<ResultId, lifecycle::PageId>);

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const RequestKey kRequest{SessionToken{"session-a"}, QueryToken{"query-a"}};
const Context kContext{
    lifecycle::ProfileId{"profile-a"},
    {lifecycle::WindowId{"window-a"}, lifecycle::SpaceId{"space-a"}},
    lifecycle::PageId{"origin-page"},
    lifecycle::ItemId{"origin-item"}};
const Origin kOrigin{SurfaceToken{"initiating-editor"}, kContext};
const SavedItemTarget kSavedTarget{lifecycle::ItemId{"saved-item"},
                                   lifecycle::ProfileId{"saved-profile"},
                                   lifecycle::SpaceId{"owning-space"}};

Result Navigate(const char* id = "navigate") {
  return {ResultId{id}, "Example", "Navigate to https://example.test/",
          NavigateAction{kContext, "https://example.test/"}};
}

Result Search(const char* id = "search") {
  return {ResultId{id}, "Example search", "Search provider for example",
          SearchAction{kContext, "example", SearchProviderKey{"provider"}}};
}

Result Switch() {
  return {ResultId{"switch"}, "Existing tab", "Switch to tab: Existing tab",
          SwitchToTabAction{
              lifecycle::PageId{"target-page"},
              lifecycle::ProfileId{"profile-b"},
              {lifecycle::WindowId{"window-b"}, lifecycle::SpaceId{"space-b"}},
              lifecycle::ItemId{"target-item"}}};
}

Result Execute() {
  return {ResultId{"command"}, "Browser action", "Execute browser action",
          ExecuteCommandAction{kContext, CommandKey{"caller-command"}}};
}

Result OpenSaved() {
  return {ResultId{"open-saved"}, "Saved item", "Open saved item: Saved item",
          OpenSavedItemAction{kSavedTarget}};
}

Result RevealSaved() {
  return {ResultId{"reveal-saved"}, "Saved item",
          "Reveal saved item in workspace",
          RevealInWorkspaceAction{kSavedTarget}};
}

Result RevealTemporary() {
  return {ResultId{"reveal-temporary"}, "Temporary page",
          "Reveal temporary page in workspace",
          RevealInWorkspaceAction{
              TemporaryPageTarget{lifecycle::PageId{"temporary-page"},
                                  lifecycle::ProfileId{"temporary-profile"},
                                  {lifecycle::WindowId{"temporary-window"},
                                   lifecycle::SpaceId{"temporary-space"}}}}};
}

void ExpectSaved(const SavedItemTarget& target) {
  REQUIRE(target.item == lifecycle::ItemId{"saved-item"});
  REQUIRE(target.profile == lifecycle::ProfileId{"saved-profile"});
  REQUIRE(std::get<lifecycle::SpaceId>(target.owner) ==
          lifecycle::SpaceId{"owning-space"});
}

void ExpectContext(const Context& actual, const Context& expected) {
  REQUIRE(actual.profile == expected.profile);
  REQUIRE(actual.attachment == expected.attachment);
  REQUIRE(actual.page == expected.page);
  REQUIRE(actual.item == expected.item);
}

void ExpectOrigin(const Origin& actual, const Origin& expected) {
  REQUIRE(actual.surface == expected.surface);
  ExpectContext(actual.context, expected.context);
}

void ExpectStatus(const ActivationResult& result, BarStatus expected) {
  REQUIRE(std::holds_alternative<BarStatus>(result));
  REQUIRE(std::get<BarStatus>(result) == expected);
}

void InvocationAndEscape() {
  CommandBar bar;
  REQUIRE(!bar.session());
  REQUIRE(!bar.selected_result());
  REQUIRE(bar.Invoke(kRequest, kOrigin, "unfinished query") == BarStatus::kOk);
  REQUIRE(bar.session()->query == "unfinished query");
  REQUIRE(bar.session()->results.empty());
  ExpectStatus(bar.Activate(), BarStatus::kNoSelection);
  REQUIRE(bar.Invoke({SessionToken{"replacement"}, QueryToken{"replacement"}},
                     Origin{SurfaceToken{"other-surface"}, kContext},
                     "replacement") == BarStatus::kAlreadyOpen);
  REQUIRE(bar.PublishResults(kRequest, {Navigate(), Search()}) ==
          BarStatus::kOk);
  REQUIRE(bar.Select(ResultId{"search"}) == BarStatus::kOk);
  auto cancelled = bar.Escape();
  REQUIRE(cancelled.has_value());
  REQUIRE(cancelled->request == kRequest);
  REQUIRE(cancelled->abandoned_query == "unfinished query");
  ExpectOrigin(cancelled->origin, kOrigin);
  REQUIRE(!bar.session());
  REQUIRE(!bar.selected_result());
  REQUIRE(!bar.Escape());
  ExpectStatus(bar.Activate(), BarStatus::kClosed);
  REQUIRE(bar.PublishResults(kRequest, {Navigate()}) == BarStatus::kClosed);
  REQUIRE(bar.SetQuery(QueryToken{"closed"}, "text") == BarStatus::kClosed);
  REQUIRE(bar.Select(ResultId{"navigate"}) == BarStatus::kClosed);
  REQUIRE(bar.MoveSelection(SelectionDirection::kNext) == BarStatus::kClosed);

  // Retention is an explicit caller choice with a new request and surface.
  Origin next{SurfaceToken{"new-editor"}, kContext};
  RequestKey next_request{SessionToken{"session-b"}, QueryToken{"query-b"}};
  REQUIRE(bar.Invoke(next_request, next, cancelled->abandoned_query) ==
          BarStatus::kOk);
  REQUIRE(bar.session()->query == "unfinished query");
  REQUIRE(bar.session()->results.empty());
  auto second = bar.Escape();
  ExpectOrigin(second->origin, next);
  REQUIRE(bar.Invoke({SessionToken{"session-c"}, QueryToken{"query-c"}}, next,
                     "") == BarStatus::kOk);
  REQUIRE(bar.session()->query.empty());
}

void TypedQueryAndStaleRequests() {
  CommandBar bar;
  REQUIRE(bar.Invoke(kRequest, kOrigin, "old") == BarStatus::kOk);
  REQUIRE(bar.PublishResults(kRequest, {Navigate()}) == BarStatus::kOk);
  REQUIRE(bar.SetQuery(QueryToken{"query-b"}, "new query") == BarStatus::kOk);
  const RequestKey second{kRequest.session, QueryToken{"query-b"}};
  REQUIRE(bar.session()->request == second);
  REQUIRE(bar.session()->results.empty());
  REQUIRE(!bar.selected_result());
  ExpectStatus(bar.Activate(), BarStatus::kNoSelection);
  REQUIRE(bar.PublishResults(kRequest, {Navigate()}) == BarStatus::kStaleQuery);
  REQUIRE(bar.PublishResults({SessionToken{"other-session"}, second.query},
                             {Navigate()}) == BarStatus::kStaleQuery);
  REQUIRE(bar.session()->query == "new query");
  REQUIRE(bar.session()->results.empty());
  REQUIRE(bar.PublishResults(second, {Search()}) == BarStatus::kOk);
  REQUIRE(bar.selected_result()->id == ResultId{"search"});
  REQUIRE(bar.SetQuery(second.query, "same token, different text") ==
          BarStatus::kInvalidInput);
  REQUIRE(bar.SetQuery(QueryToken{""}, "invalid") == BarStatus::kInvalidInput);
  REQUIRE(bar.session()->query == "new query");
  REQUIRE(bar.selected_result()->id == ResultId{"search"});

  // Returning to identical text still requires a different opaque token.
  REQUIRE(bar.SetQuery(QueryToken{"query-return"}, "old") == BarStatus::kOk);
  REQUIRE(bar.PublishResults(kRequest, {Navigate()}) == BarStatus::kStaleQuery);
  const RequestKey returned{kRequest.session, QueryToken{"query-return"}};
  REQUIRE(bar.PublishResults(returned, {}) == BarStatus::kOk);
  REQUIRE(!bar.selected_result());
  REQUIRE(bar.PublishResults(returned, {Search()}) == BarStatus::kOk);
  REQUIRE(bar.selected_result()->id == ResultId{"search"});
  REQUIRE(bar.Escape()->abandoned_query == "old");
  const RequestKey reopened{SessionToken{"new-session"},
                            QueryToken{"new-query"}};
  REQUIRE(bar.Invoke(reopened, kOrigin, "old") == BarStatus::kOk);
  REQUIRE(bar.PublishResults(returned, {Search()}) == BarStatus::kStaleQuery);
  REQUIRE(bar.session()->results.empty());
}

void StableSelectionAndRemoval() {
  CommandBar bar;
  REQUIRE(bar.Invoke(kRequest, kOrigin, "query") == BarStatus::kOk);
  REQUIRE(bar.PublishResults(kRequest, {Navigate(), Search(), Execute()}) ==
          BarStatus::kOk);
  REQUIRE(bar.session()->results.front().id == ResultId{"navigate"});
  REQUIRE(bar.Select(ResultId{"search"}) == BarStatus::kOk);
  auto refreshed = Search();
  refreshed.title = "Updated display title";
  REQUIRE(bar.PublishResults(kRequest, {Execute(), refreshed, Navigate()}) ==
          BarStatus::kOk);
  REQUIRE(bar.selected_result()->id == ResultId{"search"});
  REQUIRE(bar.selected_result()->title == "Updated display title");
  REQUIRE(bar.session()->results.front().id == ResultId{"command"});
  REQUIRE(bar.Select(ResultId{"missing"}) == BarStatus::kNotFound);
  REQUIRE(bar.selected_result()->id == ResultId{"search"});
  REQUIRE(bar.PublishResults(kRequest, {Navigate(), Execute()}) ==
          BarStatus::kOk);
  REQUIRE(!bar.selected_result());
  ExpectStatus(bar.Activate(), BarStatus::kNoSelection);
  REQUIRE(bar.PublishResults(kRequest, {Search(), Navigate()}) ==
          BarStatus::kOk);
  REQUIRE(!bar.selected_result());
  REQUIRE(bar.MoveSelection(SelectionDirection::kPrevious) == BarStatus::kOk);
  REQUIRE(bar.selected_result()->id == ResultId{"navigate"});
  REQUIRE(bar.MoveSelection(SelectionDirection::kNext) == BarStatus::kOk);
  REQUIRE(bar.selected_result()->id == ResultId{"navigate"});
  REQUIRE(bar.MoveSelection(SelectionDirection::kPrevious) == BarStatus::kOk);
  REQUIRE(bar.selected_result()->id == ResultId{"search"});
  REQUIRE(bar.MoveSelection(SelectionDirection::kPrevious) == BarStatus::kOk);
  REQUIRE(bar.selected_result()->id == ResultId{"search"});
  REQUIRE(bar.PublishResults(kRequest, {}) == BarStatus::kOk);
  REQUIRE(bar.MoveSelection(SelectionDirection::kNext) ==
          BarStatus::kNoSelection);
  REQUIRE(bar.PublishResults(kRequest, {Execute(), Navigate()}) ==
          BarStatus::kOk);
  REQUIRE(!bar.selected_result());
  REQUIRE(bar.MoveSelection(SelectionDirection::kNext) == BarStatus::kOk);
  REQUIRE(bar.selected_result()->id == ResultId{"command"});
}

void ExplicitTypedActivation() {
  const std::vector<Result> rows{Navigate(),       Search(),    Switch(),
                                 Execute(),        OpenSaved(), RevealSaved(),
                                 RevealTemporary()};
  const std::vector<ActionKind> kinds{
      ActionKind::kNavigate,         ActionKind::kSearch,
      ActionKind::kSwitchToTab,      ActionKind::kExecuteCommand,
      ActionKind::kOpenSavedItem,    ActionKind::kRevealInWorkspace,
      ActionKind::kRevealInWorkspace};
  for (std::size_t i = 0; i < rows.size(); ++i) {
    CommandBar bar;
    REQUIRE(bar.Invoke(kRequest, kOrigin, "query") == BarStatus::kOk);
    REQUIRE(bar.PublishResults(kRequest, rows) == BarStatus::kOk);
    REQUIRE(bar.Select(rows[i].id) == BarStatus::kOk);
    REQUIRE(bar.session().has_value());
    REQUIRE(bar.selected_result()->action_label == rows[i].action_label);
    auto activated = bar.Activate();
    REQUIRE(std::holds_alternative<ActivationIntent>(activated));
    const auto& intent = std::get<ActivationIntent>(activated);
    REQUIRE(intent.request == kRequest);
    ExpectOrigin(intent.origin, kOrigin);
    REQUIRE(intent.result.id == rows[i].id);
    REQUIRE(intent.result.title == rows[i].title);
    REQUIRE(intent.result.action_label == rows[i].action_label);
    REQUIRE(KindOf(intent.result.action) == kinds[i]);
    REQUIRE(!bar.session());
    REQUIRE(!bar.Escape());
    ExpectStatus(bar.Activate(), BarStatus::kClosed);
    switch (kinds[i]) {
      case ActionKind::kNavigate: {
        const auto& action = std::get<NavigateAction>(intent.result.action);
        ExpectContext(action.target, kContext);
        REQUIRE(action.destination == "https://example.test/");
        break;
      }
      case ActionKind::kSearch: {
        const auto& action = std::get<SearchAction>(intent.result.action);
        ExpectContext(action.target, kContext);
        REQUIRE(action.query == "example");
        REQUIRE(action.provider == SearchProviderKey{"provider"});
        break;
      }
      case ActionKind::kSwitchToTab: {
        const auto& action = std::get<SwitchToTabAction>(intent.result.action);
        REQUIRE(action.page == lifecycle::PageId{"target-page"});
        REQUIRE(action.profile == lifecycle::ProfileId{"profile-b"});
        REQUIRE(action.source.window == lifecycle::WindowId{"window-b"});
        REQUIRE(action.source.space == lifecycle::SpaceId{"space-b"});
        REQUIRE(action.item == lifecycle::ItemId{"target-item"});
        REQUIRE(action.profile != intent.origin.context.profile);
        REQUIRE(!(action.source == intent.origin.context.attachment));
        break;
      }
      case ActionKind::kExecuteCommand: {
        const auto& action =
            std::get<ExecuteCommandAction>(intent.result.action);
        ExpectContext(action.target, kContext);
        REQUIRE(action.command == CommandKey{"caller-command"});
        break;
      }
      case ActionKind::kOpenSavedItem: {
        const auto& action =
            std::get<OpenSavedItemAction>(intent.result.action);
        ExpectSaved(action.target);
        break;
      }
      case ActionKind::kRevealInWorkspace: {
        const auto& action =
            std::get<RevealInWorkspaceAction>(intent.result.action);
        if (const auto* saved = std::get_if<SavedItemTarget>(&action.target)) {
          ExpectSaved(*saved);
        } else {
          const auto& target = std::get<TemporaryPageTarget>(action.target);
          REQUIRE(target.page == lifecycle::PageId{"temporary-page"});
          REQUIRE(target.profile == lifecycle::ProfileId{"temporary-profile"});
          REQUIRE(target.attachment.window ==
                  lifecycle::WindowId{"temporary-window"});
          REQUIRE(target.attachment.space ==
                  lifecycle::SpaceId{"temporary-space"});
        }
        break;
      }
    }
  }
}

void InvalidInputsAreAtomic() {
  CommandBar bar;
  REQUIRE(bar.Invoke({SessionToken{""}, kRequest.query}, kOrigin, "") ==
          BarStatus::kInvalidInput);
  REQUIRE(bar.Invoke({kRequest.session, QueryToken{""}}, kOrigin, "") ==
          BarStatus::kInvalidInput);
  const auto check_origin = [&bar](Origin origin) {
    REQUIRE(bar.Invoke(kRequest, std::move(origin), "") ==
            BarStatus::kInvalidInput);
    REQUIRE(!bar.session());
  };
  Origin invalid = kOrigin;
  invalid.surface = SurfaceToken{""};
  check_origin(invalid);
  invalid = kOrigin;
  invalid.context.profile = lifecycle::ProfileId{""};
  check_origin(invalid);
  invalid = kOrigin;
  invalid.context.attachment.window = lifecycle::WindowId{""};
  check_origin(invalid);
  invalid = kOrigin;
  invalid.context.attachment.space = lifecycle::SpaceId{""};
  check_origin(invalid);
  invalid = kOrigin;
  invalid.context.page = lifecycle::PageId{""};
  check_origin(invalid);
  invalid = kOrigin;
  invalid.context.item = lifecycle::ItemId{""};
  check_origin(invalid);

  REQUIRE(bar.Invoke(kRequest, kOrigin, "query") == BarStatus::kOk);
  REQUIRE(bar.PublishResults(kRequest, {Navigate(), Search()}) ==
          BarStatus::kOk);
  REQUIRE(bar.Select(ResultId{"search"}) == BarStatus::kOk);
  const auto rejected = [&bar](Result result) {
    REQUIRE(bar.PublishResults(kRequest, {Execute(), std::move(result)}) ==
            BarStatus::kInvalidInput);
    REQUIRE(bar.session()->results.size() == 2);
    REQUIRE(bar.session()->results.front().id == ResultId{"navigate"});
    REQUIRE(bar.selected_result()->id == ResultId{"search"});
  };
  auto row = Navigate();
  row.id = ResultId{""};
  rejected(row);
  row = Navigate();
  row.title.clear();
  rejected(row);
  row = Navigate();
  row.action_label.clear();
  rejected(row);
  row = Navigate();
  std::get<NavigateAction>(row.action).destination.clear();
  rejected(row);
  row = Search();
  std::get<SearchAction>(row.action).query.clear();
  rejected(row);
  row = Search();
  std::get<SearchAction>(row.action).provider = SearchProviderKey{""};
  rejected(row);
  row = Switch();
  std::get<SwitchToTabAction>(row.action).page = lifecycle::PageId{""};
  rejected(row);
  row = Switch();
  std::get<SwitchToTabAction>(row.action).profile = lifecycle::ProfileId{""};
  rejected(row);
  row = Switch();
  std::get<SwitchToTabAction>(row.action).source.space = lifecycle::SpaceId{""};
  rejected(row);
  row = Execute();
  std::get<ExecuteCommandAction>(row.action).command = CommandKey{""};
  rejected(row);
  row = Execute();
  std::get<ExecuteCommandAction>(row.action).target.page =
      lifecycle::PageId{""};
  rejected(row);
  row = OpenSaved();
  std::get<OpenSavedItemAction>(row.action).target.item = lifecycle::ItemId{""};
  rejected(row);
  row = OpenSaved();
  std::get<OpenSavedItemAction>(row.action).target.owner =
      lifecycle::SpaceId{""};
  rejected(row);
  row = RevealSaved();
  std::get<SavedItemTarget>(
      std::get<RevealInWorkspaceAction>(row.action).target)
      .profile = lifecycle::ProfileId{""};
  rejected(row);
  row = RevealTemporary();
  std::get<TemporaryPageTarget>(
      std::get<RevealInWorkspaceAction>(row.action).target)
      .attachment.window = lifecycle::WindowId{""};
  rejected(row);
  REQUIRE(bar.PublishResults(kRequest, {Navigate(), Navigate()}) ==
          BarStatus::kInvalidInput);
  REQUIRE(bar.selected_result()->id == ResultId{"search"});
}

void BorrowedInputsAndInertText() {
  CommandBar bar;
  // Text is neither classified nor interpreted; native URL policy is required.
  REQUIRE(bar.Invoke(kRequest, kOrigin, "query\n\xE2\x98\x83") ==
          BarStatus::kOk);
  auto row = Navigate();
  std::get<NavigateAction>(row.action).destination = "unclassified:inert input";
  REQUIRE(bar.PublishResults(kRequest, {row, Search()}) == BarStatus::kOk);
  REQUIRE(bar.Select(bar.session()->results.back().id) == BarStatus::kOk);
  REQUIRE(bar.PublishResults(bar.session()->request, bar.session()->results) ==
          BarStatus::kOk);
  REQUIRE(bar.selected_result()->id == ResultId{"search"});
  REQUIRE(bar.SetQuery(QueryToken{"copied-query"}, bar.session()->query) ==
          BarStatus::kOk);
  REQUIRE(bar.session()->query == "query\n\xE2\x98\x83");
  REQUIRE(bar.PublishResults(bar.session()->request, {row}) == BarStatus::kOk);
  auto result = bar.Activate();
  REQUIRE(
      std::get<NavigateAction>(std::get<ActivationIntent>(result).result.action)
          .destination == "unclassified:inert input");

  Context empty_space{kContext.profile, kContext.attachment, std::nullopt,
                      std::nullopt};
  REQUIRE(bar.Invoke({SessionToken{"empty-space"}, QueryToken{"empty-query"}},
                     {SurfaceToken{"empty-surface"}, empty_space},
                     "") == BarStatus::kOk);
  Result default_search{ResultId{"default-search"}, "Default search",
                        "Search for text",
                        SearchAction{empty_space, "text", std::nullopt}};
  REQUIRE(bar.PublishResults(bar.session()->request, {default_search}) ==
          BarStatus::kOk);
  auto activated = bar.Activate();
  const auto& action = std::get<SearchAction>(
      std::get<ActivationIntent>(activated).result.action);
  REQUIRE(!action.provider);
  REQUIRE(!action.target.page);
  REQUIRE(!action.target.item);

  // A standalone Profile shortcut preserves its owning Profile variant.
  for (const bool reveal : {false, true}) {
    SavedItemTarget shortcut{lifecycle::ItemId{"shortcut-item"},
                             lifecycle::ProfileId{"shortcut-profile"},
                             lifecycle::ProfileId{"shortcut-profile"}};
    Result shortcut_row{
        ResultId{"shortcut-result"}, "Profile shortcut",
        reveal ? "Reveal Profile shortcut" : "Open Profile shortcut",
        OpenSavedItemAction{shortcut}};
    if (reveal)
      shortcut_row.action = RevealInWorkspaceAction{shortcut};
    CommandBar shortcut_bar;
    REQUIRE(shortcut_bar.Invoke(kRequest, kOrigin, "shortcut") ==
            BarStatus::kOk);
    REQUIRE(shortcut_bar.PublishResults(kRequest, {shortcut_row}) ==
            BarStatus::kOk);
    auto intent = shortcut_bar.Activate();
    const auto& emitted = std::get<ActivationIntent>(intent).result.action;
    const SavedItemTarget& target =
        reveal ? std::get<SavedItemTarget>(
                     std::get<RevealInWorkspaceAction>(emitted).target)
               : std::get<OpenSavedItemAction>(emitted).target;
    REQUIRE(target.item == lifecycle::ItemId{"shortcut-item"});
    REQUIRE(target.profile == lifecycle::ProfileId{"shortcut-profile"});
    REQUIRE(std::get<lifecycle::ProfileId>(target.owner) ==
            lifecycle::ProfileId{"shortcut-profile"});
  }
}

}  // namespace
}  // namespace arcified::commands

int main() {
  using namespace arcified::commands;
  InvocationAndEscape();
  TypedQueryAndStaleRequests();
  StableSelectionAndRemoval();
  ExplicitTypedActivation();
  InvalidInputsAreAtomic();
  BorrowedInputsAndInertText();
  std::cout << "command-bar synthetic session checks passed\n";
}
