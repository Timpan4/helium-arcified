#include "workspace_results.h"

#include <cstdlib>
#include <iostream>

#include "command_target.h"

namespace arcified::commands {
namespace {

using namespace lifecycle;

void Require(bool condition, const char* expression, int line) {
  if (!condition) {
    std::cerr << "line " << line << ": " << expression << '\n';
    std::exit(EXIT_FAILURE);
  }
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

void RankingBarAndTargetFlow() {
  const ProfileId profile{"work"};
  const SpaceId space{"desk"};
  const WindowId first{"first"};
  const WindowId second{"second"};
  const Attachment here{first, space};
  const Attachment there{second, space};
  const ItemId item{"saved"};
  const PageId page{"live"};
  Model model;
  REQUIRE(model.AddProfile(profile) == Status::kOk);
  REQUIRE(model.AddSpace(space, profile) == Status::kOk);
  REQUIRE(model.AddWindow(first) == Status::kOk);
  REQUIRE(model.AddWindow(second) == Status::kOk);
  REQUIRE(model.Attach(here) == Status::kOk);
  REQUIRE(model.Attach(there) == Status::kOk);
  REQUIRE(model.AddItem(item, space, "https://saved.example.test/") ==
          Status::kOk);
  RankingDisplayLabels labels;
  labels.items.emplace(item, "Example");
  const RankingScope scope{profile, here, RankingPrivacy::kRegular};
  const auto ranked_closed = RankCommands(model, scope, labels, "Example");
  REQUIRE(std::holds_alternative<std::vector<RankedCommand>>(ranked_closed));
  const auto& closed = std::get<std::vector<RankedCommand>>(ranked_closed);
  REQUIRE(closed.size() == 1);
  auto open =
      MakeWorkspaceResult(closed.front(), WorkspaceResultMode::kActivate,
                          "Open saved destination: Example, Work, Desk");
  REQUIRE(KindOf(open.action) == ActionKind::kOpenSavedItem);
  REQUIRE(std::get<OpenSavedItemAction>(open.action).target.item == item);
  auto reveal =
      MakeWorkspaceResult(closed.front(), WorkspaceResultMode::kReveal,
                          "Reveal in Workspace: Example, Work, Desk");
  REQUIRE(KindOf(reveal.action) == ActionKind::kRevealInWorkspace);
  REQUIRE(reveal.id != open.id);

  CommandBar bar;
  const RequestKey request{SessionToken{"session"}, QueryToken{"query"}};
  const Origin origin{SurfaceToken{"toolbar"},
                      {profile, here, std::nullopt, std::nullopt}};
  REQUIRE(bar.Invoke(request, origin, "Example") == BarStatus::kOk);
  REQUIRE(bar.PublishResults(request, {open}) == BarStatus::kOk);
  const auto selected = open.id;
  // Label edits preserve semantic action identity and selection.
  auto renamed = closed.front();
  renamed.title = "Renamed";
  auto renamed_open =
      MakeWorkspaceResult(renamed, WorkspaceResultMode::kActivate,
                          "Open saved destination: Renamed");
  REQUIRE(renamed_open.id == selected);
  REQUIRE(bar.PublishResults(request, {renamed_open}) == BarStatus::kOk);
  REQUIRE(bar.selected_result()->id == selected);
  REQUIRE(model.pages().empty());

  // A real observed open changes the consequence to Switch. Publishing that
  // row clears selection instead of making Enter silently change its action.
  REQUIRE(
      std::holds_alternative<OpenedPage>(model.OpenItem(item, there, page)));
  auto ranked_live = RankCommands(model, scope, labels, "Example");
  const auto& live = std::get<std::vector<RankedCommand>>(ranked_live).front();
  auto switch_row = MakeWorkspaceResult(live, WorkspaceResultMode::kActivate,
                                        "Switch: Example, Work, Desk, Second");
  REQUIRE(switch_row.id != selected);
  REQUIRE(bar.PublishResults(request, {switch_row}) == BarStatus::kOk);
  REQUIRE(!bar.selected_result());
  REQUIRE(std::get<BarStatus>(bar.Activate()) == BarStatus::kNoSelection);
  REQUIRE(bar.Select(switch_row.id) == BarStatus::kOk);
  const auto intent = std::get<ActivationIntent>(bar.Activate());
  REQUIRE(intent.origin.surface == origin.surface);
  const auto& action = std::get<SwitchToTabAction>(intent.result.action);
  REQUIRE(action.page == page);
  REQUIRE(action.source == there);
  REQUIRE(action.item == item);
  REQUIRE(model.pages().at(page).attachment() == there);
  const auto target = std::get<PageCommandTarget>(
      ResolvePageCommandTarget(model, action.page, action.source));
  REQUIRE(target.profile == action.profile);
  REQUIRE(model.TransferSavedPage(page, here) == Status::kOk);
  REQUIRE(RevalidatePageCommandTarget(model, target) ==
          TargetError::kAttachmentMismatch);
  const auto moved_rank = RankCommands(model, scope, labels, "Example");
  const auto moved_row = MakeWorkspaceResult(
      std::get<std::vector<RankedCommand>>(moved_rank).front(),
      WorkspaceResultMode::kActivate, "Switch: Example, Work, Desk, First");
  REQUIRE(moved_row.id != switch_row.id);
}

void TypedOwnersTemporaryAndFramedIdentity() {
  const ProfileId profile{"work"};
  const SpaceId space{"desk"};
  const Attachment here{WindowId{"window"}, space};
  const RankingProvenanceLabels provenance{"Work", "Desk", "Desk", "Window"};
  const RankedCommand shortcut{profile,
                               RevealSavedItem{ItemId{"shortcut"}, profile, {}},
                               "Example", provenance, RankingMatch::kExact};
  const auto open =
      MakeWorkspaceResult(shortcut, WorkspaceResultMode::kActivate,
                          "Open saved destination: Example");
  REQUIRE(std::get<ProfileId>(
              std::get<OpenSavedItemAction>(open.action).target.owner) ==
          profile);
  const RankedCommand temporary{
      profile,
      RevealTemporaryPage{TemporaryTabId{"tab"}, {PageId{"page"}, here}},
      "Draft", provenance, RankingMatch::kExact};
  const auto activate = MakeWorkspaceResult(
      temporary, WorkspaceResultMode::kActivate, "Switch: Draft");
  REQUIRE(!std::get<SwitchToTabAction>(activate.action).item);
  const auto reveal = MakeWorkspaceResult(
      temporary, WorkspaceResultMode::kReveal, "Reveal in Workspace: Draft");
  const auto& target = std::get<TemporaryPageTarget>(
      std::get<RevealInWorkspaceAction>(reveal.action).target);
  REQUIRE(target.page == PageId{"page"});
  REQUIRE(target.attachment == here);
  auto a = temporary;
  auto b = temporary;
  a.profile = ProfileId{"b:c"};
  b.profile = ProfileId{"c"};
  std::get<RevealTemporaryPage>(a.reveal).live.page = PageId{"a"};
  std::get<RevealTemporaryPage>(b.reveal).live.page = PageId{"a:b"};
  REQUIRE(MakeWorkspaceResult(a, WorkspaceResultMode::kActivate, "Switch").id !=
          MakeWorkspaceResult(b, WorkspaceResultMode::kActivate, "Switch").id);
}

}  // namespace
}  // namespace arcified::commands

int main() {
  arcified::commands::RankingBarAndTargetFlow();
  arcified::commands::TypedOwnersTemporaryAndFramedIdentity();
  std::cout << "workspace result integration fixtures passed\n";
}
