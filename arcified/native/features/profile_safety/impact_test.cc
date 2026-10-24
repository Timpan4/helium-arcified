#include "impact.h"

#include <cstdlib>
#include <iostream>

namespace arcified::profile_safety {
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
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const Attachment kDeskFirst{kFirst, kDesk};
const Attachment kDeskSecond{kSecond, kDesk};
const Attachment kResearchFirst{kFirst, kResearch};
const Attachment kPersonalFirst{kFirst, kPersonal};
const ItemId kKept{"kept"};
const ItemId kOtherKept{"other-kept"};
const ItemId kFavorite{"favorite"};
const ItemId kHomeFavorite{"home-favorite"};
const PageId kSaved{"saved"};
const PageId kTemporary{"temporary"};
const PageId kFavoritePage{"favorite-page"};
const PageId kOtherPage{"other-page"};
const ShortcutId kAlias{"alias"};
const ShortcutId kOtherAlias{"other-alias"};
const ShortcutId kHomeAlias{"home-alias"};
const SplitId kSplit{"split"};
const std::string kDestination = "https://synthetic.test/saved";
const std::string kNavigation = "https://synthetic.test/current";

void Opened(const OpenResult& result) {
  REQUIRE(std::holds_alternative<OpenedPage>(result));
}

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kHome) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kResearch, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kPersonal, kHome) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  REQUIRE(model.Attach(kDeskFirst) == Status::kOk);
  REQUIRE(model.Attach(kDeskSecond) == Status::kOk);
  REQUIRE(model.Attach(kResearchFirst) == Status::kOk);
  REQUIRE(model.Attach(kPersonalFirst) == Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kDesk) == Status::kOk);
  REQUIRE(model.SelectSpace(kSecond, kDesk) == Status::kOk);
  REQUIRE(model.AddItem(kKept, kDesk, kDestination) == Status::kOk);
  REQUIRE(model.AddItem(kOtherKept, kResearch, kDestination) == Status::kOk);
  REQUIRE(model.AddItem(kFavorite, kWork, kDestination) == Status::kOk);
  REQUIRE(model.AddItem(kHomeFavorite, kHome, kDestination) == Status::kOk);
  REQUIRE(model.AddShortcut(kAlias, kWork, kKept) == Status::kOk);
  REQUIRE(model.AddShortcut(kOtherAlias, kWork, kOtherKept) == Status::kOk);
  REQUIRE(model.AddShortcut(kHomeAlias, kHome, kHomeFavorite) == Status::kOk);
  Opened(model.OpenItem(kKept, kDeskFirst, kSaved));
  REQUIRE(model.RecordNavigation(kSaved, kNavigation) == Status::kOk);
  REQUIRE(model.ReportActivity(kSaved, Activity::kProtected) == Status::kOk);
  REQUIRE(model.OpenTemporary(kTemporary, TemporaryTabId{"temporary-tab"},
                              kDeskSecond, kNavigation) == Status::kOk);
  Opened(model.OpenItem(kFavorite, kDeskFirst, kFavoritePage));
  REQUIRE(model.ReportActivity(kFavoritePage, Activity::kQuiescent) ==
          Status::kOk);
  Opened(model.OpenItem(kHomeFavorite, kPersonalFirst, kOtherPage));
  REQUIRE(model.SetSplit(kSplit, kDeskFirst, {kSaved, kFavoritePage}) ==
          Status::kOk);
  return model;
}

Impact Inspect(const Model& model, Request request) {
  auto result = Preview(model, std::move(request));
  REQUIRE(std::holds_alternative<Impact>(result));
  auto impact = std::get<Impact>(std::move(result));
  REQUIRE(impact.execution == ExecutionGate::kBlockedPolicyOpen);
  const std::array<NativeUnknown, 4> unknowns{
      NativeUnknown::kUnsavedForms, NativeUnknown::kDownloads,
      NativeUnknown::kAuthenticatedState,
      NativeUnknown::kProfileDataOutsideModel};
  REQUIRE(impact.native_unknowns == unknowns);
  return impact;
}

void ReassignmentSeparatesContextAndAffectedReferences() {
  auto model = Workspace();
  auto impact = Inspect(model, ReassignSpace{kDesk, kHome});
  REQUIRE(std::get<ReassignSpace>(impact.request).target == kHome);
  REQUIRE(impact.spaces.size() == 1);
  REQUIRE(impact.spaces.front().id == kDesk);
  REQUIRE(impact.spaces.front().profile == kWork);
  REQUIRE(impact.attachments ==
          std::vector<Attachment>({kDeskFirst, kDeskSecond}));
  REQUIRE(impact.selections.size() == 2);
  REQUIRE(impact.selections[0].window == kFirst);
  REQUIRE(impact.selections[0].space == kDesk);
  REQUIRE(impact.selections[1].window == kSecond);
  REQUIRE(impact.selections[1].space == kDesk);
  REQUIRE(impact.items.size() == 1);
  REQUIRE(impact.items[0].id == kKept);
  REQUIRE(std::get<SpaceId>(impact.items[0].owner) == kDesk);
  REQUIRE(impact.aliases.size() == 1);
  REQUIRE(impact.aliases[0].id == kAlias);
  REQUIRE(impact.aliases[0].profile == kWork);
  REQUIRE(impact.aliases[0].target == kKept);
  REQUIRE(impact.context_only.size() == 2);
  const auto& source = impact.context_only[0];
  REQUIRE(source.profile == kWork);
  REQUIRE(source.standalone_items == std::vector<ItemId>({kFavorite}));
  REQUIRE(source.aliases.size() == 2);
  REQUIRE(source.aliases[0].id == kAlias);
  REQUIRE(source.aliases[1].id == kOtherAlias);
  REQUIRE(source.aliases[1].target == kOtherKept);
  const auto& target = impact.context_only[1];
  REQUIRE(target.profile == kHome);
  REQUIRE(target.standalone_items == std::vector<ItemId>({kHomeFavorite}));
  REQUIRE(target.aliases.size() == 1);
  REQUIRE(target.aliases[0].id == kHomeAlias);
  REQUIRE(target.aliases[0].target == kHomeFavorite);
  REQUIRE(impact.pages.size() == 3);
  REQUIRE(impact.pages[0].id == kFavoritePage);
  REQUIRE(impact.pages[0].reported_activity == Activity::kQuiescent);
  REQUIRE(std::get<SavedPage>(impact.pages[0].owner).item == kFavorite);
  REQUIRE(impact.pages[1].id == kSaved);
  REQUIRE(impact.pages[1].profile == kWork);
  REQUIRE(impact.pages[1].reported_activity == Activity::kProtected);
  REQUIRE(impact.pages[2].id == kTemporary);
  REQUIRE(impact.pages[2].reported_activity == Activity::kUnknown);
  const auto& temporary = std::get<TemporaryTab>(impact.pages[2].owner);
  REQUIRE(temporary.id == TemporaryTabId{"temporary-tab"});
  REQUIRE(temporary.attachment == kDeskSecond);
  REQUIRE(impact.splits.size() == 1);
  REQUIRE(impact.splits[0].id == kSplit);
  REQUIRE(impact.splits[0].attachment == kDeskFirst);
  REQUIRE(impact.splits[0].pages ==
          std::vector<PageId>({kSaved, kFavoritePage}));

  // Preview retained all source state, native outcomes are not simulated.
  REQUIRE(model.spaces().at(kDesk).profile == kWork);
  REQUIRE(model.pages().size() == 4);
  REQUIRE(model.items().size() == 4);
  REQUIRE(model.shortcuts().size() == 3);
  REQUIRE(model.attachments().size() == 4);
  REQUIRE(model.windows().at(kFirst).selected_space == kDesk);
  REQUIRE(model.pages().at(kSaved).current_navigation == kNavigation);
  REQUIRE(model.items().at(kKept).saved_destination == kDestination);
  REQUIRE(model.pages().at(kSaved).activity == Activity::kProtected);
  REQUIRE(model.splits().at(kSplit).pages == impact.splits[0].pages);
}

void ProfileDeletionIncludesAllSharedSpacesAndProfileItems() {
  auto model = Workspace();
  auto impact = Inspect(model, DeleteProfile{kWork});
  REQUIRE(impact.spaces.size() == 2);
  REQUIRE(impact.spaces[0].id == kDesk);
  REQUIRE(impact.spaces[1].id == kResearch);
  REQUIRE(impact.attachments ==
          std::vector<Attachment>({kDeskFirst, kResearchFirst, kDeskSecond}));
  REQUIRE(impact.items.size() == 3);
  REQUIRE(impact.items[0].id == kFavorite);
  REQUIRE(std::get<ProfileId>(impact.items[0].owner) == kWork);
  REQUIRE(impact.items[1].id == kKept);
  REQUIRE(impact.items[2].id == kOtherKept);
  REQUIRE(impact.aliases.size() == 2);
  REQUIRE(impact.aliases[0].id == kAlias);
  REQUIRE(impact.aliases[1].id == kOtherAlias);
  REQUIRE(impact.pages.size() == 3);
  for (const auto& page : impact.pages)
    REQUIRE(page.profile == kWork);
  REQUIRE(impact.context_only.empty());
  REQUIRE(model.profiles().size() == 2);
  REQUIRE(model.pages().at(kOtherPage).profile == kHome);
}

void SpaceDeletionIsOnlyPreparatoryInventory() {
  auto model = Workspace();
  auto impact = Inspect(model, DeleteSpace{kDesk});
  REQUIRE(impact.spaces.size() == 1);
  REQUIRE(impact.items.size() == 1);
  REQUIRE(impact.items[0].id == kKept);
  REQUIRE(impact.aliases.size() == 1);
  REQUIRE(impact.pages.size() == 3);
  REQUIRE(impact.context_only.empty());
  REQUIRE(model.spaces().size() == 3);
}

void EmptyAndUnchangedRequestsStayBlocked() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kHome) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  auto same = Inspect(model, ReassignSpace{kDesk, kWork});
  REQUIRE(same.context_only.size() == 1);
  REQUIRE(same.pages.empty());
  REQUIRE(same.items.empty());
  Inspect(model, ReassignSpace{kDesk, kHome});
  Inspect(model, DeleteSpace{kDesk});
  auto empty = Inspect(model, DeleteProfile{kHome});
  REQUIRE(empty.spaces.empty());
  REQUIRE(empty.pages.empty());
  // A Profile can own saved data with no Spaces or live pages.
  REQUIRE(model.AddItem(kFavorite, kHome, kDestination) == Status::kOk);
  REQUIRE(model.AddShortcut(kAlias, kHome, kFavorite) == Status::kOk);
  auto data = Inspect(model, DeleteProfile{kHome});
  REQUIRE(data.spaces.empty());
  REQUIRE(data.items.size() == 1);
  REQUIRE(data.items[0].id == kFavorite);
  REQUIRE(data.aliases.size() == 1);
  REQUIRE(data.aliases[0].target == kFavorite);
}

void MissingTargetsAreExplicitErrors() {
  auto model = Workspace();
  REQUIRE(
      std::get<PreviewError>(Preview(model, DeleteProfile{ProfileId{""}})) ==
      PreviewError::kProfileNotFound);
  REQUIRE(
      std::get<PreviewError>(Preview(model, DeleteSpace{SpaceId{"missing"}})) ==
      PreviewError::kSpaceNotFound);
  REQUIRE(std::get<PreviewError>(
              Preview(model, ReassignSpace{SpaceId{"missing"}, kHome})) ==
          PreviewError::kSpaceNotFound);
  REQUIRE(std::get<PreviewError>(
              Preview(model, ReassignSpace{kDesk, ProfileId{"missing"}})) ==
          PreviewError::kProfileNotFound);
  REQUIRE(model.spaces().at(kDesk).profile == kWork);
}

void SpacePreviewExcludesSiblingSpaceLiveWork() {
  auto model = Workspace();
  const PageId sibling_temporary{"sibling-temporary"};
  const SplitId sibling_split{"sibling-split"};
  REQUIRE(model.TransferSavedPage(kFavoritePage, kResearchFirst) ==
          Status::kOk);
  REQUIRE(model.OpenTemporary(sibling_temporary, TemporaryTabId{"sibling-tab"},
                              kResearchFirst, kNavigation) == Status::kOk);
  REQUIRE(model.SetSplit(sibling_split, kResearchFirst,
                         {kFavoritePage, sibling_temporary}) == Status::kOk);

  const auto reassignment = Inspect(model, ReassignSpace{kDesk, kHome});
  const auto deletion = Inspect(model, DeleteSpace{kDesk});
  for (const auto* impact : {&reassignment, &deletion}) {
    REQUIRE(impact->pages.size() == 2);
    REQUIRE(impact->pages[0].id == kSaved);
    REQUIRE(impact->pages[1].id == kTemporary);
    REQUIRE(impact->splits.size() == 1);
    REQUIRE(impact->splits[0].id == kSplit);
    REQUIRE(impact->splits[0].pages == std::vector<PageId>({kSaved}));
  }
  REQUIRE(reassignment.context_only[0].standalone_items ==
          std::vector<ItemId>({kFavorite}));
  REQUIRE(deletion.context_only.empty());

  const auto profile = Inspect(model, DeleteProfile{kWork});
  REQUIRE(profile.pages.size() == 4);
  REQUIRE(profile.pages[0].id == kFavoritePage);
  REQUIRE(std::get<SavedPage>(profile.pages[0].owner).attachment ==
          kResearchFirst);
  REQUIRE(profile.pages[1].id == kSaved);
  REQUIRE(profile.pages[2].id == sibling_temporary);
  REQUIRE(profile.pages[3].id == kTemporary);
  REQUIRE(profile.splits.size() == 2);
  REQUIRE(profile.splits[0].id == sibling_split);
  REQUIRE(profile.splits[0].pages ==
          std::vector<PageId>({kFavoritePage, sibling_temporary}));
  REQUIRE(profile.splits[1].id == kSplit);
}

void RepreviewUsesCurrentGraphWithoutChangingOldSnapshot() {
  auto model = Workspace();
  const auto before = Inspect(model, ReassignSpace{kDesk, kHome});
  REQUIRE(model.TransferSavedPage(kSaved, kDeskSecond) == Status::kOk);
  REQUIRE(model.RecordNavigation(kSaved, kDestination) == Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kPersonal) == Status::kOk);
  // Borrowing IDs from model getters is safe: the request owns its IDs.
  const auto after =
      Inspect(model, ReassignSpace{model.spaces().find(kDesk)->first,
                                   model.spaces().at(kPersonal).profile});
  REQUIRE(before.selections.size() == 2);
  REQUIRE(after.selections.size() == 1);
  REQUIRE(after.selections[0].window == kSecond);
  REQUIRE(std::get<SavedPage>(before.pages[1].owner).attachment == kDeskFirst);
  REQUIRE(before.pages[1].reported_activity == Activity::kProtected);
  REQUIRE(std::get<SavedPage>(after.pages[1].owner).attachment == kDeskSecond);
  REQUIRE(after.pages[1].reported_activity == Activity::kUnknown);
  REQUIRE(before.splits[0].pages ==
          std::vector<PageId>({kSaved, kFavoritePage}));
  REQUIRE(after.splits[0].pages == std::vector<PageId>({kFavoritePage}));
}

}  // namespace
}  // namespace arcified::profile_safety

int main() {
  using namespace arcified::profile_safety;
  ReassignmentSeparatesContextAndAffectedReferences();
  ProfileDeletionIncludesAllSharedSpacesAndProfileItems();
  SpaceDeletionIsOnlyPreparatoryInventory();
  EmptyAndUnchangedRequestsStayBlocked();
  MissingTargetsAreExplicitErrors();
  SpacePreviewExcludesSiblingSpaceLiveWork();
  RepreviewUsesCurrentGraphWithoutChangingOldSnapshot();
  std::cout << "profile-safety synthetic previews passed\n";
}
