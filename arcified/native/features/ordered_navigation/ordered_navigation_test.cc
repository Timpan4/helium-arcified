#include "ordered_navigation.h"

#include <cstdlib>
#include <iostream>
#include <limits>

namespace arcified::ordered_navigation {
namespace {

using namespace lifecycle;

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
const WindowId kWindow{"window"};
const WindowId kOtherWindow{"other-window"};
const Attachment kScope{kWindow, kDesk};
const Attachment kRemote{kOtherWindow, kDesk};
const ItemId kFirst{"z-first"};
const ItemId kSecond{"a-second"};
const ItemId kResearchItem{"research-item"};
const ItemId kProfileItem{"profile-item"};
const ItemId kPrivateItem{"private-item"};
const ShortcutId kAlias{"research-alias"};
const PageId kTemporary{"temporary"};
const PageId kRemoteTemporary{"remote-temporary"};
const PageId kSavedPage{"saved-page"};
const SplitId kSplit{"split"};

struct Fixture {
  Model model;
  SidebarSnapshot snapshot{
      SnapshotRevision{"initial"},
      kScope,
      {{kFirst, {}, std::nullopt},
       {kSecond,
        {FolderReference{"outer"}, FolderReference{"inner"}},
        std::nullopt},
       {kTemporary, {}, std::nullopt}},
      {kDesk, kPersonal, kResearch}};

  Fixture() {
    REQUIRE(model.AddProfile(kWork) == Status::kOk);
    REQUIRE(model.AddProfile(kHome) == Status::kOk);
    REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
    REQUIRE(model.AddSpace(kResearch, kWork) == Status::kOk);
    REQUIRE(model.AddSpace(kPersonal, kHome) == Status::kOk);
    REQUIRE(model.AddWindow(kWindow) == Status::kOk);
    REQUIRE(model.AddWindow(kOtherWindow) == Status::kOk);
    for (const auto& space : {kDesk, kResearch, kPersonal})
      REQUIRE(model.Attach({kWindow, space}) == Status::kOk);
    REQUIRE(model.Attach(kRemote) == Status::kOk);
    REQUIRE(model.SelectSpace(kWindow, kDesk) == Status::kOk);
    REQUIRE(model.AddItem(kFirst, kDesk, "https://first.test/") == Status::kOk);
    REQUIRE(model.AddItem(kSecond, kDesk, "https://second.test/") ==
            Status::kOk);
    REQUIRE(model.AddItem(kResearchItem, kResearch, "https://research.test/") ==
            Status::kOk);
    REQUIRE(model.AddItem(kProfileItem, kWork, "https://profile.test/") ==
            Status::kOk);
    REQUIRE(model.AddItem(kPrivateItem, kPersonal, "https://private.test/") ==
            Status::kOk);
    REQUIRE(model.AddShortcut(kAlias, kWork, kResearchItem) == Status::kOk);
    REQUIRE(model.OpenTemporary(kTemporary, TemporaryTabId{"local"}, kScope,
                                "https://temporary.test/") == Status::kOk);
    REQUIRE(model.OpenTemporary(kRemoteTemporary, TemporaryTabId{"remote"},
                                kRemote,
                                "https://remote.test/") == Status::kOk);
  }
};

DestinationPlan Plan(const SelectionResult& result) {
  REQUIRE(std::holds_alternative<DestinationPlan>(result));
  return std::get<DestinationPlan>(result);
}

TabDestination Tab(const SelectionResult& result) {
  return std::get<TabDestination>(Plan(result).destination);
}

SpaceDestination SpaceTarget(const SelectionResult& result) {
  return std::get<SpaceDestination>(Plan(result).destination);
}

void ExpectRefusal(const SelectionResult& result, Refusal expected) {
  REQUIRE(std::holds_alternative<Refusal>(result));
  REQUIRE(std::get<Refusal>(result) == expected);
}

void SidebarOrderAndReveal() {
  Fixture f;
  const auto next = Tab(
      SelectTab(f.model, f.snapshot,
                RelativeTab{kFirst, Direction::kNext, EdgeBehavior::kStop}));
  REQUIRE(next.item == kSecond);  // Sidebar order differs from map/ID order.
  REQUIRE(next.reveal_row.collapsed_ancestor_ids ==
          (std::vector<FolderReference>{FolderReference{"outer"},
                                        FolderReference{"inner"}}));
  REQUIRE(!next.live_page);
  REQUIRE(next.destination == kScope);
  REQUIRE(Tab(SelectTab(f.model, f.snapshot,
                        RelativeTab{kTemporary, Direction::kPrevious,
                                    EdgeBehavior::kStop}))
              .item == kSecond);
  REQUIRE(Tab(SelectTab(f.model, f.snapshot, DirectIndex{2})).live_page ==
          kTemporary);
  REQUIRE(Tab(SelectTab(f.model, f.snapshot, DirectTab{kSecond})).item ==
          kSecond);
  REQUIRE(f.model.pages().size() == 2);
  REQUIRE(f.model.windows().at(kWindow).selected_space == kDesk);
}

void BoundariesAndEmptyOrder() {
  Fixture f;
  ExpectRefusal(
      SelectTab(f.model, f.snapshot,
                RelativeTab{kFirst, Direction::kPrevious, EdgeBehavior::kStop}),
      Refusal::kAtBoundary);
  REQUIRE(Tab(SelectTab(f.model, f.snapshot,
                        RelativeTab{kFirst, Direction::kPrevious,
                                    EdgeBehavior::kWrap}))
              .live_page == kTemporary);
  REQUIRE(Tab(SelectTab(f.model, f.snapshot,
                        RelativeTab{kTemporary, Direction::kNext,
                                    EdgeBehavior::kWrap}))
              .item == kFirst);
  ExpectRefusal(SelectTab(f.model, f.snapshot,
                          DirectIndex{std::numeric_limits<std::size_t>::max()}),
                Refusal::kIndexOutOfBounds);
  ExpectRefusal(SelectTab(f.model, f.snapshot, DirectTab{kProfileItem}),
                Refusal::kNotVisible);
  f.snapshot.tabs.erase(f.snapshot.tabs.begin() + 1, f.snapshot.tabs.end());
  REQUIRE(
      Tab(SelectTab(f.model, f.snapshot,
                    RelativeTab{kFirst, Direction::kNext, EdgeBehavior::kWrap}))
          .item == kFirst);
  f.snapshot.tabs.clear();
  ExpectRefusal(SelectTab(f.model, f.snapshot, DirectIndex{0}),
                Refusal::kEmpty);
}

void SavedIdentityAndAliases() {
  Fixture f;
  REQUIRE(std::holds_alternative<OpenedPage>(
      f.model.OpenItem(kFirst, kRemote, kSavedPage)));
  REQUIRE(f.model.RecordNavigation(kSavedPage, "https://first.test/draft") ==
          Status::kOk);
  REQUIRE(f.model.ReportActivity(kSavedPage, Activity::kProtected) ==
          Status::kOk);
  const auto saved = Tab(SelectTab(f.model, f.snapshot, DirectTab{kFirst}));
  REQUIRE(saved.item == kFirst);
  REQUIRE(saved.live_page == kSavedPage);
  REQUIRE(saved.live_attachment == kRemote);
  REQUIRE(saved.destination == kScope);
  REQUIRE(f.model.pages().at(kSavedPage).attachment() == kRemote);
  REQUIRE(f.model.pages().at(kSavedPage).current_navigation ==
          "https://first.test/draft");
  REQUIRE(f.model.pages().at(kSavedPage).activity == Activity::kProtected);
  REQUIRE(f.model.pages().size() == 3);
  f.snapshot.tabs.push_back({kAlias, {}, std::nullopt});
  const auto alias = Tab(SelectTab(f.model, f.snapshot, DirectTab{kAlias}));
  REQUIRE(alias.item == kResearchItem);
  REQUIRE(alias.destination == (Attachment{kWindow, kResearch}));
  REQUIRE(alias.profile == kWork);
  REQUIRE(!alias.live_page);
  REQUIRE(std::get<ShortcutId>(alias.reveal_row.target) == kAlias);
  REQUIRE(f.model.Detach({kWindow, kResearch}) == Status::kOk);
  f.snapshot.space_order.pop_back();
  ExpectRefusal(SelectTab(f.model, f.snapshot, DirectTab{kAlias}),
                Refusal::kInvalidProjection);
}

void VisibilityAndProjectionValidation() {
  Fixture f;
  const auto valid = f.snapshot;
  for (const TabReference& hidden :
       {TabReference{kPrivateItem}, TabReference{kResearchItem},
        TabReference{kRemoteTemporary}, TabReference{PageId{"missing"}}}) {
    f.snapshot = valid;
    f.snapshot.tabs.push_back({hidden, {}, std::nullopt});
    ExpectRefusal(SelectTab(f.model, f.snapshot, DirectIndex{0}),
                  Refusal::kInvalidProjection);
  }
  f.snapshot = valid;
  f.snapshot.tabs.push_back({kProfileItem, {}, std::nullopt});
  REQUIRE(
      Tab(SelectTab(f.model, f.snapshot, DirectTab{kProfileItem})).profile ==
      kWork);
  f.snapshot = valid;
  f.snapshot.tabs.push_back(valid.tabs.front());
  ExpectRefusal(SelectTab(f.model, f.snapshot, DirectIndex{0}),
                Refusal::kInvalidProjection);
  f.snapshot = valid;
  f.snapshot.tabs[0].collapsed_ancestor_ids = {FolderReference{"same"},
                                               FolderReference{"same"}};
  ExpectRefusal(SelectTab(f.model, f.snapshot, DirectIndex{0}),
                Refusal::kInvalidProjection);
  f.snapshot = valid;
  f.snapshot.revision = SnapshotRevision{""};
  ExpectRefusal(SelectTab(f.model, f.snapshot, DirectIndex{0}),
                Refusal::kInvalidProjection);
  f.snapshot = valid;
  REQUIRE(f.model.SelectSpace(kWindow, kPersonal) == Status::kOk);
  ExpectRefusal(SelectTab(f.model, f.snapshot, DirectIndex{0}),
                Refusal::kInvalidScope);
}

void ReorderAndCloseFreshness() {
  Fixture f;
  const auto old = Plan(
      SelectTab(f.model, f.snapshot,
                RelativeTab{kFirst, Direction::kNext, EdgeBehavior::kStop}));
  REQUIRE(std::holds_alternative<DestinationPlan>(
      Revalidate(f.model, f.snapshot, old)));
  std::swap(f.snapshot.tabs[1], f.snapshot.tabs[2]);
  f.snapshot.revision = SnapshotRevision{"reordered"};
  ExpectRefusal(Revalidate(f.model, f.snapshot, old), Refusal::kStaleSnapshot);
  REQUIRE(
      Tab(SelectTab(f.model, f.snapshot,
                    RelativeTab{kFirst, Direction::kNext, EdgeBehavior::kStop}))
          .live_page == kTemporary);
  const auto temporary =
      Plan(SelectTab(f.model, f.snapshot, DirectTab{kTemporary}));
  REQUIRE(f.model.ReportActivity(kTemporary, Activity::kQuiescent) ==
          Status::kOk);
  REQUIRE(f.model.ClosePage(kTemporary) == Status::kOk);
  // Even an incorrectly retained revision cannot select a vanished page.
  ExpectRefusal(Revalidate(f.model, f.snapshot, temporary),
                Refusal::kStaleSnapshot);
  f.snapshot.tabs.erase(f.snapshot.tabs.begin() + 1);
  f.snapshot.revision = SnapshotRevision{"closed"};
  ExpectRefusal(
      SelectTab(f.model, f.snapshot,
                RelativeTab{kTemporary, Direction::kNext, EdgeBehavior::kWrap}),
      Refusal::kAnchorMissing);
  REQUIRE(Tab(SelectTab(f.model, f.snapshot, DirectIndex{1})).item == kSecond);
}

void SplitIdentityAndStaleMetadata() {
  Fixture f;
  REQUIRE(std::holds_alternative<OpenedPage>(
      f.model.OpenItem(kFirst, kScope, kSavedPage)));
  REQUIRE(f.model.SetSplit(kSplit, kScope, {kSavedPage, kTemporary}) ==
          Status::kOk);
  f.snapshot.tabs = {{kSavedPage, {}, kSplit}, {kTemporary, {}, kSplit}};
  const auto split = Plan(SelectTab(
      f.model, f.snapshot,
      RelativeTab{kSavedPage, Direction::kNext, EdgeBehavior::kStop}));
  REQUIRE(std::get<TabDestination>(split.destination).reveal_row.split ==
          kSplit);
  REQUIRE(std::get<TabDestination>(split.destination).live_page == kTemporary);
  REQUIRE(f.model.RemoveSplit(kSplit) == Status::kOk);
  ExpectRefusal(Revalidate(f.model, f.snapshot, split),
                Refusal::kStaleSnapshot);
  f.snapshot.tabs = {{kSavedPage, {}, std::nullopt}};
  ExpectRefusal(SelectTab(f.model, f.snapshot, DirectIndex{0}),
                Refusal::kInvalidProjection);
  f.snapshot.tabs = {{kFirst, {}, std::nullopt}};
  const auto saved = Plan(SelectTab(f.model, f.snapshot, DirectIndex{0}));
  REQUIRE(f.model.TransferSavedPage(kSavedPage, kRemote) == Status::kOk);
  ExpectRefusal(Revalidate(f.model, f.snapshot, saved),
                Refusal::kStaleSnapshot);
  const auto transferred = Plan(SelectTab(f.model, f.snapshot, DirectIndex{0}));
  f.snapshot.tabs[0].collapsed_ancestor_ids = {FolderReference{"new-parent"}};
  ExpectRefusal(Revalidate(f.model, f.snapshot, transferred),
                Refusal::kStaleSnapshot);
}

void SpaceOrderAndIsolation() {
  Fixture f;
  auto result =
      SelectSpace(f.model, f.snapshot,
                  RelativeSpace{Direction::kNext, EdgeBehavior::kStop});
  REQUIRE(SpaceTarget(result).destination == (Attachment{kWindow, kPersonal}));
  REQUIRE(SpaceTarget(result).profile == kHome);
  REQUIRE(f.model.windows().at(kWindow).selected_space == kDesk);
  REQUIRE(f.model.pages().at(kTemporary).profile == kWork);
  REQUIRE(SpaceTarget(SelectSpace(f.model, f.snapshot, DirectSpace{kResearch}))
              .profile == kWork);
  REQUIRE(SpaceTarget(SelectSpace(f.model, f.snapshot, DirectIndex{2}))
              .destination.space == kResearch);
  REQUIRE(SpaceTarget(SelectSpace(f.model, f.snapshot,
                                  RelativeSpace{Direction::kPrevious,
                                                EdgeBehavior::kWrap}))
              .destination.space == kResearch);
  ExpectRefusal(
      SelectSpace(f.model, f.snapshot,
                  RelativeSpace{Direction::kPrevious, EdgeBehavior::kStop}),
      Refusal::kAtBoundary);
  const auto plan = Plan(result);
  REQUIRE(std::holds_alternative<DestinationPlan>(
      Revalidate(f.model, f.snapshot, plan)));
  f.snapshot.space_order.push_back(kDesk);
  ExpectRefusal(SelectSpace(f.model, f.snapshot, DirectIndex{0}),
                Refusal::kInvalidProjection);
  f.snapshot.space_order = {kDesk, kResearch};
  ExpectRefusal(SelectSpace(f.model, f.snapshot, DirectSpace{kPersonal}),
                Refusal::kNotVisible);
  ExpectRefusal(Revalidate(f.model, f.snapshot, plan), Refusal::kStaleSnapshot);
  f.snapshot.space_order.clear();
  ExpectRefusal(SelectSpace(f.model, f.snapshot, DirectIndex{0}),
                Refusal::kEmpty);
}

}  // namespace
}  // namespace arcified::ordered_navigation

int main() {
  using namespace arcified::ordered_navigation;
  SidebarOrderAndReveal();
  BoundariesAndEmptyOrder();
  SavedIdentityAndAliases();
  VisibilityAndProjectionValidation();
  ReorderAndCloseFreshness();
  SplitIdentityAndStaleMetadata();
  SpaceOrderAndIsolation();
  std::cout << "ordered navigation synthetic fixtures passed\n";
}
