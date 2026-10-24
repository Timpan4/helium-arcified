#include "saved_source.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <utility>

namespace arcified::library {
namespace {

using namespace lifecycle;

static_assert(kContractVersion == 3);

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const ProfileId kWork{"work"};
const ProfileId kOtherProfile{"other-profile"};
const SpaceId kDesk{"desk"};
const SpaceId kResearch{"research"};
const SpaceId kOtherSpace{"other-space"};
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const Attachment kDeskFirst{kFirst, kDesk};
const Attachment kDeskSecond{kSecond, kDesk};
const Attachment kResearchFirst{kFirst, kResearch};
const Attachment kOtherFirst{kFirst, kOtherSpace};
const ItemId kSaved{"saved"};
const ItemId kFavorite{"favorite"};
const ItemId kAliased{"aliased"};
const PageId kPage{"live"};
const std::string kDestination = "https://library.test/Saved";

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kOtherProfile) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kResearch, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kOtherSpace, kOtherProfile) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  REQUIRE(model.Attach(kDeskFirst) == Status::kOk);
  REQUIRE(model.Attach(kDeskSecond) == Status::kOk);
  REQUIRE(model.Attach(kOtherFirst) == Status::kOk);
  return model;
}

std::vector<SavedEntry> Entries(const Model& model,
                                const Attachment& attachment,
                                std::string_view query = "") {
  auto result = SearchSaved(model, attachment, query);
  REQUIRE(std::holds_alternative<std::vector<SavedEntry>>(result));
  return std::get<std::vector<SavedEntry>>(std::move(result));
}

const SavedEntry& Entry(const std::vector<SavedEntry>& entries,
                        const ItemId& item) {
  auto entry =
      std::find_if(entries.begin(), entries.end(),
                   [&item](const auto& row) { return row.item == item; });
  REQUIRE(entry != entries.end());
  return *entry;
}

void ExpectOpen(const OpenResult& result,
                const PageId& page,
                OpenDisposition disposition) {
  REQUIRE(std::holds_alternative<OpenedPage>(result));
  REQUIRE(std::get<OpenedPage>(result).page == page);
  REQUIRE(std::get<OpenedPage>(result).disposition == disposition);
}

void ExpectError(const OpenResult& result, Status status) {
  REQUIRE(std::holds_alternative<Status>(result));
  REQUIRE(std::get<Status>(result) == status);
}

void EmptyAndUnavailable() {
  auto model = Workspace();
  REQUIRE(Entries(model, kDeskFirst).empty());
  auto result = SearchSaved(model, kResearchFirst, "");
  REQUIRE(std::holds_alternative<Status>(result));
  REQUIRE(std::get<Status>(result) == Status::kNotAttached);
  result = SearchSaved(model, {WindowId{"missing"}, kDesk}, "");
  REQUIRE(std::get<Status>(result) == Status::kNotAttached);
  ExpectError(OpenSavedResult(model, kResearchFirst, kSaved, kPage),
              Status::kNotAttached);
  ExpectError(OpenSavedResult(model, kDeskFirst, kSaved, kPage),
              Status::kNotFound);
  REQUIRE(model.pages().empty());
}

void VisibilityAndIdentity() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kSaved, kDesk, kDestination) == Status::kOk);
  REQUIRE(model.AddItem(kFavorite, kWork, kDestination) == Status::kOk);
  REQUIRE(model.AddItem(kAliased, kResearch, kDestination) == Status::kOk);
  REQUIRE(model.AddItem(ItemId{"hidden"}, kResearch,
                        "https://hidden.test/secret") == Status::kOk);
  REQUIRE(model.AddItem(ItemId{"foreign"}, kOtherSpace,
                        "https://foreign.test/secret") == Status::kOk);
  REQUIRE(model.AddItem(ItemId{"foreign-favorite"}, kOtherProfile,
                        "https://foreign.test/favorite") == Status::kOk);
  REQUIRE(model.AddShortcut(ShortcutId{"local-alias"}, kWork, kSaved) ==
          Status::kOk);
  REQUIRE(model.AddShortcut(ShortcutId{"favorite-alias"}, kWork, kFavorite) ==
          Status::kOk);
  REQUIRE(model.AddShortcut(ShortcutId{"first-alias"}, kWork, kAliased) ==
          Status::kOk);
  REQUIRE(model.AddShortcut(ShortcutId{"second-alias"}, kWork, kAliased) ==
          Status::kOk);
  REQUIRE(model.AddShortcut(ShortcutId{"foreign-alias"}, kOtherProfile,
                            ItemId{"foreign"}) == Status::kOk);
  REQUIRE(model.OpenTemporary(PageId{"temporary"}, TemporaryTabId{"tab"},
                              kDeskFirst,
                              "https://temporary.test/secret") == Status::kOk);

  const auto rows = Entries(model, kDeskFirst);
  REQUIRE(rows.size() == 3);
  REQUIRE(Entry(rows, kSaved).source == SavedSource::kKeptInSpace);
  REQUIRE(Entry(rows, kFavorite).source == SavedSource::kProfileShortcut);
  REQUIRE(Entry(rows, kAliased).source == SavedSource::kShortcutAlias);
  for (const auto& row : rows)
    REQUIRE(row.saved_destination == kDestination);
  REQUIRE(Entries(model, kDeskSecond).size() == 3);
  REQUIRE(Entries(model, kDeskFirst, "secret").empty());
  REQUIRE(Entries(model, kOtherFirst).size() == 2);
  ExpectError(OpenSavedResult(model, kDeskFirst, ItemId{"hidden"}, kPage),
              Status::kNotFound);
  ExpectError(OpenSavedResult(model, kDeskFirst, ItemId{"foreign"}, kPage),
              Status::kNotFound);
  ExpectError(OpenSavedResult(model, kOtherFirst, kFavorite, kPage),
              Status::kNotFound);
  REQUIRE(model.pages().size() == 1);
}

void SearchSavedDestinationsOnly() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kSaved, kDesk, kDestination) == Status::kOk);
  REQUIRE(model.AddItem(ItemId{"utf8"}, kDesk,
                        "https://library.test/\xc3\xa5") == Status::kOk);
  ExpectOpen(model.OpenItem(kSaved, kDeskFirst, kPage), kPage,
             OpenDisposition::kCreated);
  REQUIRE(model.RecordNavigation(kPage, "https://live.test/unsaved") ==
          Status::kOk);
  const auto matches = Entries(model, kDeskFirst, "LIBRARY.TEST/sAvEd");
  REQUIRE(matches.size() == 1);
  REQUIRE(matches.front().item == kSaved);
  REQUIRE(matches.front().saved_destination == kDestination);
  REQUIRE(Entries(model, kDeskFirst, "live.test").empty());
  REQUIRE(Entries(model, kDeskFirst, "saved ").empty());
  REQUIRE(Entries(model, kDeskFirst,
                  "a longer query than any destination in this fixture")
              .empty());
  REQUIRE(Entries(model, kDeskFirst, "\xc3\xa5").size() == 1);
  REQUIRE(Entries(model, kDeskFirst, "\xc3\x85").empty());
  REQUIRE(model.pages().at(kPage).current_navigation ==
          "https://live.test/unsaved");
  REQUIRE(model.items().at(kSaved).saved_destination == kDestination);
}

void SelectLiveIdentity() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kSaved, kDesk, kDestination) == Status::kOk);
  ExpectOpen(model.OpenItem(kSaved, kDeskSecond, kPage), kPage,
             OpenDisposition::kCreated);
  REQUIRE(model.RecordNavigation(kPage, "https://live.test/form") ==
          Status::kOk);
  REQUIRE(model.ReportActivity(kPage, Activity::kProtected) == Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kOtherSpace) == Status::kOk);
  const auto rows = Entries(model, kDeskFirst, "saved");
  REQUIRE(rows.size() == 1);
  ExpectOpen(OpenSavedResult(model, kDeskFirst, rows.front().item,
                             PageId{"unused-candidate"}),
             kPage, OpenDisposition::kExisting);
  REQUIRE(model.pages().size() == 1);
  REQUIRE(model.pages().at(kPage).attachment() == kDeskFirst);
  REQUIRE(model.pages().at(kPage).profile == kWork);
  REQUIRE(model.pages().at(kPage).activity == Activity::kProtected);
  REQUIRE(model.pages().at(kPage).current_navigation ==
          "https://live.test/form");
  REQUIRE(model.items().at(kSaved).saved_destination == kDestination);
  REQUIRE(model.windows().at(kFirst).selected_space == kOtherSpace);
}

void AliasSelectionAndStaleness() {
  auto model = Workspace();
  const ShortcutId first{"first-alias"};
  const ShortcutId second{"second-alias"};
  REQUIRE(model.AddItem(kAliased, kResearch, kDestination) == Status::kOk);
  REQUIRE(model.AddShortcut(first, kWork, kAliased) == Status::kOk);
  REQUIRE(model.AddShortcut(second, kWork, kAliased) == Status::kOk);
  const auto rows = Entries(model, kDeskFirst);
  REQUIRE(rows.size() == 1);
  ExpectError(OpenSavedResult(model, kDeskFirst, rows.front().item, kPage),
              Status::kNotAttached);
  REQUIRE(model.pages().empty());
  REQUIRE(model.Attach(kResearchFirst) == Status::kOk);
  REQUIRE(model.RemoveShortcut(first) == Status::kOk);
  ExpectOpen(OpenSavedResult(model, kDeskFirst, rows.front().item, kPage),
             kPage, OpenDisposition::kCreated);
  REQUIRE(model.pages().at(kPage).attachment() == kResearchFirst);
  REQUIRE(model.pages().at(kPage).current_navigation == kDestination);
  REQUIRE(model.RemoveShortcut(second) == Status::kOk);
  ExpectError(OpenSavedResult(model, kDeskFirst, rows.front().item,
                              PageId{"must-not-create"}),
              Status::kNotFound);
  REQUIRE(Entries(model, kDeskFirst).empty());
  REQUIRE(model.pages().size() == 1);
  REQUIRE(model.pages().at(kPage).attachment() == kResearchFirst);

  // Current owner and destination, rather than the stale row, remain authority.
  REQUIRE(model.ReportActivity(kPage, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kPage) == Status::kOk);
  REQUIRE(model.MoveItem(kAliased, kDesk) == Status::kOk);
  const PageId reopened{"reopened"};
  ExpectOpen(OpenSavedResult(model, kDeskFirst, rows.front().item, reopened),
             reopened, OpenDisposition::kCreated);
  REQUIRE(model.pages().at(reopened).attachment() == kDeskFirst);
  REQUIRE(Entry(Entries(model, kDeskFirst), kAliased).source ==
          SavedSource::kKeptInSpace);
}

void RemovedMovedAndDetachedResults() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kSaved, kDesk, kDestination) == Status::kOk);
  auto rows = Entries(model, kDeskFirst);
  REQUIRE(model.MoveItem(kSaved, kResearch) == Status::kOk);
  ExpectError(OpenSavedResult(model, kDeskFirst, rows.front().item, kPage),
              Status::kNotFound);
  REQUIRE(model.RemoveItem(kSaved) == Status::kOk);
  ExpectError(OpenSavedResult(model, kDeskFirst, rows.front().item, kPage),
              Status::kNotFound);
  REQUIRE(model.AddItem(kFavorite, kWork, kDestination) == Status::kOk);
  rows = Entries(model, kDeskFirst);
  REQUIRE(model.Detach(kDeskFirst) == Status::kOk);
  ExpectError(OpenSavedResult(model, kDeskFirst, rows.front().item, kPage),
              Status::kNotAttached);
  REQUIRE(model.pages().empty());
  REQUIRE(model.Detach(kDeskSecond) == Status::kOk);
  REQUIRE(model.ReassignEmptySpace(kDesk, kOtherProfile) == Status::kOk);
  REQUIRE(model.Attach(kDeskFirst) == Status::kOk);
  ExpectError(OpenSavedResult(model, kDeskFirst, rows.front().item, kPage),
              Status::kNotFound);
  REQUIRE(model.pages().empty());
  REQUIRE(model.Detach(kDeskFirst) == Status::kOk);
  REQUIRE(model.ReassignEmptySpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.Attach(kDeskFirst) == Status::kOk);
  ExpectOpen(OpenSavedResult(model, kDeskFirst, rows.front().item, kPage),
             kPage, OpenDisposition::kCreated);
  REQUIRE(model.pages().at(kPage).profile == kWork);
  REQUIRE(model.pages().at(kPage).current_navigation == kDestination);
}

}  // namespace
}  // namespace arcified::library

int main() {
  using namespace arcified::library;
  EmptyAndUnavailable();
  VisibilityAndIdentity();
  SearchSavedDestinationsOnly();
  SelectLiveIdentity();
  AliasSelectionAndStaleness();
  RemovedMovedAndDetachedResults();
  std::cout << "Library saved-source fixtures passed (lifecycle v3)\n";
}
