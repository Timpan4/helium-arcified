#include "recent_switcher.h"

#include <cstdlib>
#include <iostream>
#include <utility>

namespace arcified::recent_switcher {
namespace {

using lifecycle::Attachment;
using lifecycle::ItemId;
using lifecycle::Model;
using lifecycle::PageId;
using lifecycle::ProfileId;
using lifecycle::SpaceId;
using lifecycle::Status;
using lifecycle::TemporaryTabId;
using lifecycle::WindowId;

static_assert(lifecycle::kContractVersion == 3);

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const ProfileId kWork{"work"};
const ProfileId kHome{"home"};
const SpaceId kDesk{"desk"};
const SpaceId kResearch{"research"};
const SpaceId kPersonal{"personal"};
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const Attachment kDeskFirst{kFirst, kDesk};
const Attachment kDeskSecond{kSecond, kDesk};
const Attachment kResearchFirst{kFirst, kResearch};
const Attachment kResearchSecond{kSecond, kResearch};
const Attachment kPersonalFirst{kFirst, kPersonal};
const PageId kA{"a"};
const PageId kB{"b"};
const PageId kC{"c"};

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kHome) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kResearch, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kPersonal, kHome) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  for (const auto& attachment : {kDeskFirst, kDeskSecond, kResearchFirst,
                                 kResearchSecond, kPersonalFirst}) {
    REQUIRE(model.Attach(attachment) == Status::kOk);
  }
  REQUIRE(model.SelectSpace(kFirst, kDesk) == Status::kOk);
  return model;
}

void Temporary(Model& model, PageId page, const Attachment& attachment) {
  const TemporaryTabId tab{page.value + "-temporary"};
  REQUIRE(model.OpenTemporary(page, tab, attachment,
                              "https://synthetic.example.test/same-title") ==
          Status::kOk);
}

void Saved(Model& model,
           PageId page,
           lifecycle::ItemOwner owner,
           const Attachment& attachment) {
  const ItemId item{page.value + "-saved"};
  REQUIRE(model.AddItem(item, std::move(owner),
                        "https://synthetic.example.test/saved") == Status::kOk);
  const auto result = model.OpenItem(item, attachment, page);
  REQUIRE(std::holds_alternative<lifecycle::OpenedPage>(result));
  REQUIRE(std::get<lifecycle::OpenedPage>(result).page == page);
}

void Close(Model& model, const PageId& page) {
  REQUIRE(model.ReportActivity(page, lifecycle::Activity::kQuiescent) ==
          Status::kOk);
  REQUIRE(model.ClosePage(page) == Status::kOk);
}

Snapshot Input(std::vector<HistoryEntry> history,
               std::optional<PageId> focused = kA) {
  return Snapshot{kFirst,
                  1,
                  std::move(focused),
                  {kDesk, kResearch, kPersonal},
                  std::move(history),
                  {}};
}

void Selected(const Session& session, const PageId& page) {
  REQUIRE(session.holding());
  REQUIRE(session.selected());
  REQUIRE(session.selected()->entry.page == page);
}

ActivationIntent Intent(const ReleaseResult& result, const PageId& page) {
  REQUIRE(std::holds_alternative<ActivationIntent>(result));
  const auto intent = std::get<ActivationIntent>(result);
  REQUIRE(intent.entry.page == page);
  return intent;
}

void End(const ReleaseResult& result, EndReason reason) {
  REQUIRE(std::holds_alternative<EndReason>(result));
  REQUIRE(std::get<EndReason>(result) == reason);
}

void HoldCycleReverseRelease() {
  auto model = Workspace();
  for (const auto& page : {kA, kB, kC})
    Temporary(model, page, kDeskFirst);
  const auto input = Input({{kA, kDesk}, {kB, kDesk}, {kC, kDesk}});
  Session session;
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  Selected(session, kB);
  REQUIRE(session.Begin(model, input, Direction::kForward) ==
          Update::kAlreadyHolding);
  Selected(session, kB);
  REQUIRE(session.Cycle(model, input, Direction::kForward) == Update::kHolding);
  Selected(session, kC);
  REQUIRE(session.Cycle(model, input, Direction::kForward) == Update::kHolding);
  Selected(session, kA);
  REQUIRE(session.Cycle(model, input, Direction::kReverse) == Update::kHolding);
  Selected(session, kC);
  const auto intent = Intent(session.Release(model, input), kC);
  REQUIRE(intent.expected_source == kDeskFirst);
  REQUIRE(intent.destination == kDeskFirst);
  REQUIRE(intent.context_generation == input.context_generation);
  REQUIRE(!session.holding());
  REQUIRE(session.candidates().empty());
  REQUIRE(!session.selected());
  End(session.Release(model, input), EndReason::kIdle);

  REQUIRE(session.Begin(model, input, Direction::kReverse) == Update::kHolding);
  Selected(session, kC);
  REQUIRE(session.Cycle(model, input, Direction::kReverse) == Update::kHolding);
  Selected(session, kB);
  session.Cancel();
  End(session.Release(model, input), EndReason::kIdle);
  REQUIRE(session.Cycle(model, input, Direction::kForward) == Update::kIdle);
}

void AnchorEmptyAndSingle() {
  auto model = Workspace();
  Session session;
  auto input = Input({});
  REQUIRE(session.Begin(model, input, Direction::kForward) ==
          Update::kNoCandidates);
  REQUIRE(!session.holding());
  Temporary(model, kA, kDeskFirst);
  input.history = {{kA, kDesk}};
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  Selected(session, kA);
  End(session.Release(model, input), EndReason::kAlreadyFocused);

  Temporary(model, kB, kDeskFirst);
  Temporary(model, kC, kDeskFirst);
  input.history = {{kB, kDesk}, {kA, kDesk}, {kC, kDesk}};
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  Selected(session, kC);  // Anchor is authoritative focus, not history[0].
  session.Cancel();
  input.focused_page.reset();
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  Selected(session, kB);
  session.Cancel();
  REQUIRE(session.Begin(model, input, Direction::kReverse) == Update::kHolding);
  Selected(session, kC);
}

void FrozenHistoryAndMetadata() {
  auto model = Workspace();
  Temporary(model, kA, kDeskFirst);
  Temporary(model, kB, kDeskFirst);
  auto input = Input(
      {{kA, kDesk}, {kB, kDesk}, {kB, kResearch}, {PageId{"unknown"}, kDesk}});
  input.previews.emplace(kA, Preview{"Same title", "Desk", "Playing"});
  input.previews.emplace(kB, Preview{"Same title", "Desk", "Loading"});
  Session session;
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  REQUIRE(session.candidates().size() == 2);
  Selected(session, kB);
  REQUIRE(session.selected()->preview->status == "Loading");
  Temporary(model, kC, kDeskFirst);
  input.history = {{kC, kDesk}, {kB, kResearch}, {kA, kDesk}};
  input.previews.at(kB) = Preview{"Updated title", "Desk", "Protected"};
  REQUIRE(session.Reconcile(model, input) == Update::kHolding);
  REQUIRE(session.candidates().size() == 2);
  Selected(session, kB);
  REQUIRE(session.selected()->entry.requested_space == kDesk);
  REQUIRE(session.selected()->preview->title == "Updated title");
  REQUIRE(session.selected()->preview->space_label == "Desk");
  REQUIRE(session.selected()->preview->status == "Protected");
  input.previews.clear();
  REQUIRE(session.Reconcile(model, input) == Update::kHolding);
  REQUIRE(!session.selected()->preview);
  Intent(session.Release(model, input), kB);
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  REQUIRE(session.candidates().size() == 2);  // kB's first request is invalid.
  Selected(session, kC);
}

void RemovedSelectionReconcilesDirectionally() {
  for (const auto direction : {Direction::kForward, Direction::kReverse}) {
    auto model = Workspace();
    for (const auto& page : {kA, kB, kC})
      Temporary(model, page, kDeskFirst);
    const auto input = Input({{kA, kDesk}, {kB, kDesk}, {kC, kDesk}});
    Session session;
    REQUIRE(session.Begin(model, input, direction) == Update::kHolding);
    const auto removed = session.selected()->entry.page;
    Close(model, removed);
    REQUIRE(session.Reconcile(model, input) == Update::kHolding);
    Selected(session, direction == Direction::kForward ? kC : kB);
    REQUIRE(session.candidates().size() == 2);
    Intent(session.Release(model, input),
           direction == Direction::kForward ? kC : kB);
  }
}

void ReleaseRefusesUnseenReplacement() {
  auto model = Workspace();
  for (const auto& page : {kA, kB, kC})
    Temporary(model, page, kDeskFirst);
  const auto input = Input({{kA, kDesk}, {kB, kDesk}, {kC, kDesk}});
  Session session;
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  Close(model, kB);
  End(session.Release(model, input), EndReason::kSelectionChanged);
  REQUIRE(!session.holding());
  End(session.Release(model, input), EndReason::kIdle);

  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  Selected(session, kC);
  Close(model, kC);
  // A cycle is a fresh user action. Pick kA once, without skipping it.
  REQUIRE(session.Cycle(model, input, Direction::kReverse) == Update::kHolding);
  Selected(session, kA);
  Close(model, kA);
  End(session.Release(model, input), EndReason::kNoCandidates);
  REQUIRE(!session.holding());
}

void ClosingUnselectedAndChangingSplitsKeepsIdentity() {
  auto model = Workspace();
  for (const auto& page : {kA, kB, kC})
    Temporary(model, page, kDeskFirst);
  const lifecycle::SplitId split{"split"};
  REQUIRE(model.SetSplit(split, kDeskFirst, {kA, kB, kC}) == Status::kOk);
  const auto input = Input({{kA, kDesk}, {kB, kDesk}, {kC, kDesk}});
  Session session;
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  Close(model, kA);
  REQUIRE(model.SetSplit(split, kDeskFirst, {kC, kB}) == Status::kOk);
  REQUIRE(session.Reconcile(model, input) == Update::kHolding);
  Selected(session, kB);
  REQUIRE(model.RemoveSplit(split) == Status::kOk);
  REQUIRE(session.Reconcile(model, input) == Update::kHolding);
  Selected(session, kB);
  Intent(session.Release(model, input), kB);
}

void SavedTransferRefreshesSourceWithoutMutatingModel() {
  auto model = Workspace();
  Temporary(model, kA, kDeskFirst);
  Saved(model, kB, kDesk, kDeskSecond);
  REQUIRE(model.RecordNavigation(kB, "https://synthetic.example.test/draft") ==
          Status::kOk);
  REQUIRE(model.ReportActivity(kB, lifecycle::Activity::kProtected) ==
          Status::kOk);
  const lifecycle::SplitId split{"saved-layout"};
  REQUIRE(model.SetSplit(split, kDeskSecond, {kB}) == Status::kOk);
  auto input = Input({{kA, kDesk}, {kB, kResearch}});
  input.previews.emplace(kB, Preview{"Draft", "Desk", "Protected"});
  Session session;
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  Selected(session, kB);
  REQUIRE(session.selected()->source == kDeskSecond);
  REQUIRE(session.selected()->destination == kDeskFirst);
  REQUIRE(session.selected()->profile == kWork);
  // Preview cannot transfer, select the requested Space, prune a split, reset
  // current navigation, change activity or duplicate the page.
  REQUIRE(model.pages().at(kB).attachment() == kDeskSecond);
  REQUIRE(model.splits().at(split).pages == std::vector<PageId>{kB});
  REQUIRE(model.windows().at(kFirst).selected_space == kDesk);
  REQUIRE(model.pages().size() == 2);
  const auto intent = Intent(session.Release(model, input), kB);
  REQUIRE(intent.expected_source == kDeskSecond);
  REQUIRE(intent.destination == kDeskFirst);
  REQUIRE(model.pages().at(kB).attachment() == kDeskSecond);
  REQUIRE(model.pages().at(kB).current_navigation ==
          "https://synthetic.example.test/draft");
  REQUIRE(model.pages().at(kB).activity == lifecycle::Activity::kProtected);
  REQUIRE(model.items().at(ItemId{"b-saved"}).saved_destination ==
          "https://synthetic.example.test/saved");

  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  REQUIRE(model.TransferSavedPage(kB, kDeskFirst) == Status::kOk);
  REQUIRE(session.Reconcile(model, input) == Update::kHolding);
  Selected(session, kB);
  REQUIRE(session.selected()->source == kDeskFirst);
  REQUIRE(model.splits().empty());
  // Even a move first discovered on release returns the same stable page,
  // with a fresh source attachment. The external executor still revalidates.
  REQUIRE(model.TransferSavedPage(kB, kDeskSecond) == Status::kOk);
  REQUIRE(Intent(session.Release(model, input), kB).expected_source ==
          kDeskSecond);
}

void DestinationAndAliasSemantics() {
  auto model = Workspace();
  Saved(model, kB, kDesk, kDeskSecond);
  const lifecycle::ShortcutId alias{"alias"};
  REQUIRE(model.AddShortcut(alias, kWork, ItemId{"b-saved"}) == Status::kOk);
  // Alias is resolved by the caller, so the alias and original dedup as kB.
  auto input = Input({{kB, kResearch}, {kB, kDesk}}, std::nullopt);
  Session session;
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  REQUIRE(session.candidates().size() == 1);
  REQUIRE(session.selected()->destination == kDeskFirst);
  session.Cancel();
  input.eligible_spaces = {kResearch};
  REQUIRE(session.Begin(model, input, Direction::kForward) ==
          Update::kNoCandidates);
  input.eligible_spaces = {kDesk};
  input.history = {{kB, kPersonal}, {kB, kDesk}};
  REQUIRE(session.Begin(model, input, Direction::kForward) ==
          Update::kNoCandidates);  // First request fails Profile check.
  input.history = {{kB, SpaceId{"missing"}}};
  REQUIRE(session.Begin(model, input, Direction::kForward) ==
          Update::kNoCandidates);

  input.history = {{kB, kResearch}};
  REQUIRE(model.Detach(kDeskFirst) == Status::kOk);
  REQUIRE(session.Begin(model, input, Direction::kForward) ==
          Update::kNoCandidates);  // Owning Space must already be attached.
  REQUIRE(model.Attach(kDeskFirst) == Status::kOk);
  REQUIRE(model.Detach(kResearchFirst) == Status::kOk);
  REQUIRE(session.Begin(model, input, Direction::kForward) ==
          Update::kNoCandidates);  // Validate request before resolving owner.
}

void ProfileShortcutExplicitDestinationAndScopeChanges() {
  auto model = Workspace();
  Temporary(model, kA, kDeskFirst);
  Saved(model, kB, kWork, kDeskSecond);
  auto input = Input({{kA, kDesk}, {kB, kResearch}});
  Session session;
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  Selected(session, kB);
  REQUIRE(session.selected()->destination == kResearchFirst);
  REQUIRE(model.TransferSavedPage(kB, kResearchSecond) == Status::kOk);
  // Updating history cannot change this hold's explicit requested Space.
  input.history = {{kB, kDesk}, {kA, kDesk}};
  REQUIRE(session.Reconcile(model, input) == Update::kHolding);
  REQUIRE(session.selected()->source == kResearchSecond);
  REQUIRE(session.selected()->destination == kResearchFirst);
  input.eligible_spaces = {kDesk};
  REQUIRE(session.Reconcile(model, input) == Update::kHolding);
  Selected(session, kA);
  input.eligible_spaces = {kDesk, kResearch};
  REQUIRE(session.Reconcile(model, input) == Update::kHolding);
  REQUIRE(session.candidates().size() == 1);  // Removed rows never reappear.
  session.Cancel();
  input.history = {{kB, kPersonal}};
  REQUIRE(session.Begin(model, input, Direction::kForward) ==
          Update::kNoCandidates);
}

void LocalTemporaryAndMultipleProfiles() {
  auto model = Workspace();
  Temporary(model, kA, kDeskFirst);
  Temporary(model, kB, kDeskSecond);
  Temporary(model, kC, kPersonalFirst);
  auto input = Input({{kA, kDesk}, {kB, kDesk}, {kC, kPersonal}});
  Session session;
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  REQUIRE(session.candidates().size() == 2);
  Selected(session, kC);
  REQUIRE(session.selected()->profile == kHome);
  REQUIRE(Intent(session.Release(model, input), kC).destination ==
          kPersonalFirst);
  REQUIRE(model.windows().at(kFirst).selected_space == kDesk);
  REQUIRE(model.pages().at(kC).profile == kHome);
  input.history = {{kA, kResearch}};
  REQUIRE(session.Begin(model, input, Direction::kForward) ==
          Update::kNoCandidates);  // Same-window temporary rehome also refused.
}

void ContextChangesAndWindowClosureCancel() {
  auto model = Workspace();
  Temporary(model, kA, kDeskFirst);
  Temporary(model, kB, kDeskFirst);
  auto input = Input({{kA, kDesk}, {kB, kDesk}});
  Session session;
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  ++input.context_generation;
  End(session.Release(model, input), EndReason::kContextChanged);
  REQUIRE(!session.holding());
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  ++input.context_generation;
  REQUIRE(session.Cycle(model, input, Direction::kReverse) ==
          Update::kContextChanged);
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  ++input.context_generation;
  REQUIRE(session.Begin(model, input, Direction::kForward) ==
          Update::kContextChanged);
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  input.window = kSecond;
  REQUIRE(session.Reconcile(model, input) == Update::kContextChanged);
  input.window = kFirst;
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  REQUIRE(model.ReportActivity(kA, lifecycle::Activity::kQuiescent) ==
          Status::kOk);
  REQUIRE(model.ReportActivity(kB, lifecycle::Activity::kQuiescent) ==
          Status::kOk);
  REQUIRE(model.CloseWindow(kFirst) == Status::kOk);
  End(session.Release(model, input), EndReason::kContextChanged);
  REQUIRE(session.Begin(model, input, Direction::kForward) ==
          Update::kContextChanged);
}

void KeepRetainsIdentityAndReopenDoesNotReplaceIt() {
  auto model = Workspace();
  Temporary(model, kA, kDeskFirst);
  Temporary(model, kB, kDeskFirst);
  auto input = Input({{kA, kDesk}, {kB, kDesk}});
  Session session;
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  const ItemId item{"kept-during-hold"};
  REQUIRE(model.Keep(kB, item) == Status::kOk);
  REQUIRE(session.Reconcile(model, input) == Update::kHolding);
  Selected(session, kB);
  Intent(session.Release(model, input), kB);

  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  Close(model, kB);
  const auto opened = model.OpenItem(item, kDeskFirst, kC);
  REQUIRE(std::holds_alternative<lifecycle::OpenedPage>(opened));
  REQUIRE(std::get<lifecycle::OpenedPage>(opened).page == kC);
  input.history = {{kA, kDesk}, {kC, kDesk}};
  End(session.Release(model, input), EndReason::kSelectionChanged);
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  Selected(session, kC);  // Only a new hold can acquire the reopened page.
}

void ReleaseChecksDestinationAgain() {
  auto model = Workspace();
  Temporary(model, kA, kDeskFirst);
  Saved(model, kB, kWork, kDeskSecond);
  auto input = Input({{kA, kDesk}, {kB, kResearch}});
  Session session;
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  REQUIRE(model.Detach(kResearchFirst) == Status::kOk);
  End(session.Release(model, input), EndReason::kSelectionChanged);
  REQUIRE(model.pages().at(kB).attachment() == kDeskSecond);
  REQUIRE(model.Attach(kResearchFirst) == Status::kOk);
  REQUIRE(session.Begin(model, input, Direction::kForward) == Update::kHolding);
  input.eligible_spaces = {kDesk};
  End(session.Release(model, input), EndReason::kSelectionChanged);
  REQUIRE(model.pages().at(kB).attachment() == kDeskSecond);
  input.eligible_spaces.clear();
  REQUIRE(session.Begin(model, input, Direction::kForward) ==
          Update::kNoCandidates);
}

}  // namespace
}  // namespace arcified::recent_switcher

int main() {
  using namespace arcified::recent_switcher;
  HoldCycleReverseRelease();
  AnchorEmptyAndSingle();
  FrozenHistoryAndMetadata();
  RemovedSelectionReconcilesDirectionally();
  ReleaseRefusesUnseenReplacement();
  ClosingUnselectedAndChangingSplitsKeepsIdentity();
  SavedTransferRefreshesSourceWithoutMutatingModel();
  DestinationAndAliasSemantics();
  ProfileShortcutExplicitDestinationAndScopeChanges();
  LocalTemporaryAndMultipleProfiles();
  ContextChangesAndWindowClosureCancel();
  KeepRetainsIdentityAndReopenDoesNotReplaceIt();
  ReleaseChecksDestinationAgain();
  std::cout << "recent-switcher: all synthetic contract-v3 fixtures passed\n";
}
