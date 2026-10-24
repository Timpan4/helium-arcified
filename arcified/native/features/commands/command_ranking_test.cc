#include "command_ranking.h"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <utility>

namespace arcified::commands {
namespace {

using namespace lifecycle;
static_assert(kContractVersion == 3);

[[noreturn]] void Fail(const char* expression, int line) {
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression)        \
  do {                             \
    if (!(expression))             \
      Fail(#expression, __LINE__); \
  } while (false)

const ProfileId kWork{"work"};
const ProfileId kOtherProfile{"other-profile"};
const ProfileId kPrivate{"distinct-private-context"};
const SpaceId kDesk{"desk"};
const SpaceId kResearch{"research"};
const SpaceId kUnattached{"unattached"};
const SpaceId kOtherSpace{"other-space"};
const SpaceId kPrivateSpace{"private-space"};
const WindowId kCaller{"caller"};
const WindowId kOtherWindow{"other-window"};
const Attachment kDeskCaller{kCaller, kDesk};
const Attachment kDeskOther{kOtherWindow, kDesk};
const Attachment kResearchCaller{kCaller, kResearch};
const Attachment kResearchOther{kOtherWindow, kResearch};
const std::string kSaved = "https://mail.example.test/inbox";
const std::string kCurrent = "https://mail.example.test/unsaved-compose";
const ItemId kMail{"mail"};
const PageId kMailPage{"mail-page"};

RankingScope Scope() {
  return {kWork, kDeskCaller, RankingPrivacy::kRegular};
}

Model Workspace() {
  Model model;
  for (const auto& profile : {kWork, kOtherProfile, kPrivate})
    REQUIRE(model.AddProfile(profile) == Status::kOk);
  for (const auto& space : {kDesk, kResearch, kUnattached})
    REQUIRE(model.AddSpace(space, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kOtherSpace, kOtherProfile) == Status::kOk);
  REQUIRE(model.AddSpace(kPrivateSpace, kPrivate) == Status::kOk);
  for (const auto& window : {kCaller, kOtherWindow}) {
    REQUIRE(model.AddWindow(window) == Status::kOk);
    for (const auto& space : {kDesk, kResearch, kOtherSpace, kPrivateSpace})
      REQUIRE(model.Attach({window, space}) == Status::kOk);
  }
  REQUIRE(model.SelectSpace(kCaller, kDesk) == Status::kOk);
  return model;
}

std::vector<RankedCommand> Rows(const Model& model,
                                const RankingDisplayLabels& labels = {},
                                std::string_view query = "") {
  auto result = RankCommands(model, Scope(), labels, query);
  REQUIRE(std::holds_alternative<std::vector<RankedCommand>>(result));
  return std::get<std::vector<RankedCommand>>(std::move(result));
}

std::vector<RankingIdentity> Identities(
    const std::vector<RankedCommand>& rows) {
  std::vector<RankingIdentity> identities;
  identities.reserve(rows.size());
  for (const auto& row : rows)
    identities.push_back(row.identity());
  return identities;
}

const RankedCommand& Find(const std::vector<RankedCommand>& rows,
                          const RankingIdentity& identity) {
  for (const auto& row : rows)
    if (row.identity() == identity)
      return row;
  REQUIRE(false);
  std::abort();
}

void ScopeRefusalsAndIsolation() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kMail, kDesk, kSaved) == Status::kOk);
  REQUIRE(model.AddItem(ItemId{"isolated"}, kOtherSpace,
                        "https://excluded.example.test/isolated") ==
          Status::kOk);
  REQUIRE(model.AddItem(ItemId{"private"}, kPrivateSpace,
                        "https://excluded.example.test/private") ==
          Status::kOk);
  REQUIRE(model.OpenTemporary(
              PageId{"private-page"}, TemporaryTabId{"private-tab"},
              {kCaller, kPrivateSpace},
              "https://excluded.example.test/private-live") == Status::kOk);
  REQUIRE(model.OpenTemporary(
              PageId{"isolated-page"}, TemporaryTabId{"isolated-tab"},
              {kCaller, kOtherSpace},
              "https://excluded.example.test/isolated-live") == Status::kOk);
  RankingDisplayLabels labels;
  labels.items.emplace(ItemId{"isolated"}, "needle excluded");
  labels.items.emplace(ItemId{"private"}, "needle excluded");
  labels.pages.emplace(PageId{"private-page"}, "needle excluded");
  labels.pages.emplace(PageId{"isolated-page"}, "needle excluded");
  REQUIRE(Identities(Rows(model, labels)) ==
          std::vector<RankingIdentity>{kMail});
  REQUIRE(Rows(model, labels, "needle").empty());
  REQUIRE(Rows(model, labels, "excluded.example.test").empty());
  for (const auto privacy :
       {RankingPrivacy::kUnknown, RankingPrivacy::kPrivate}) {
    auto scope = Scope();
    scope.privacy = privacy;
    REQUIRE(std::get<RankingError>(RankCommands(model, scope, labels, "")) ==
            RankingError::kPrivacyExcluded);
  }
  const RankingScope private_scope{
      kPrivate, {kCaller, kPrivateSpace}, RankingPrivacy::kPrivate};
  REQUIRE(
      std::get<RankingError>(RankCommands(model, private_scope, labels, "")) ==
      RankingError::kPrivacyExcluded);
  auto scope = Scope();
  scope.profile = ProfileId{"missing"};
  REQUIRE(std::get<RankingError>(RankCommands(model, scope, labels, "")) ==
          RankingError::kProfileNotFound);
  scope = Scope();
  scope.caller = {kCaller, kOtherSpace};
  REQUIRE(std::get<RankingError>(RankCommands(model, scope, labels, "")) ==
          RankingError::kProfileMismatch);
  scope = Scope();
  scope.caller = {kCaller, kUnattached};
  REQUIRE(std::get<RankingError>(RankCommands(model, scope, labels, "")) ==
          RankingError::kCallerNotAttached);
  scope = Scope();
  scope.caller.window = WindowId{"missing"};
  REQUIRE(std::get<RankingError>(RankCommands(model, scope, labels, "")) ==
          RankingError::kCallerNotAttached);
}

void ClosedDiscoveryAndProfileShortcutProvenance() {
  auto model = Workspace();
  const ItemId detached{"detached-item"};
  const ItemId shortcut{"profile-shortcut"};
  REQUIRE(model.AddItem(detached, kUnattached, kSaved) == Status::kOk);
  REQUIRE(model.AddItem(shortcut, kWork, kSaved) == Status::kOk);
  REQUIRE(model.AddShortcut(ShortcutId{"alias"}, kWork, detached) ==
          Status::kOk);
  RankingDisplayLabels labels;
  labels.profiles.emplace(kWork, "Work");
  labels.spaces.emplace(kUnattached, "Offline research");
  labels.items.emplace(detached, "Renamed pin");
  auto rows = Rows(model, labels);
  REQUIRE(rows.size() == 2);
  const auto& row = Find(rows, detached);
  const auto& target = std::get<RevealSavedItem>(row.reveal);
  REQUIRE(target.item == detached);
  REQUIRE(std::get<SpaceId>(target.owner) == kUnattached);
  REQUIRE(!target.live);
  REQUIRE(row.profile == kWork);
  REQUIRE(row.provenance.profile == "Work");
  REQUIRE(row.provenance.owning_space == "Offline research");
  REQUIRE(!row.provenance.live_space);
  REQUIRE(!row.provenance.live_window);
  REQUIRE(row.title == "Renamed pin");
  REQUIRE(RankedVerbLabel(row.verb()) == "Open saved destination");
  const auto& profile_row = Find(rows, shortcut);
  REQUIRE(std::get<ProfileId>(
              std::get<RevealSavedItem>(profile_row.reveal).owner) == kWork);
  REQUIRE(!profile_row.provenance.owning_space);
  REQUIRE(!profile_row.provenance.live_window);
  REQUIRE(profile_row.provenance.profile == "Work");
  REQUIRE(kRankingRevealLabel == "Reveal in Workspace");
  REQUIRE(!model.attachments().count({kCaller, kUnattached}));
  REQUIRE(model.pages().empty());
  REQUIRE(Rows(model, labels, "Renamed pin").size() == 1);
  REQUIRE(Rows(model, labels, "mail.example.test").size() == 2);

  // Identical visible titles retain distinct identities, provenance and verbs.
  labels.items.emplace(shortcut, "Renamed pin");
  REQUIRE(std::holds_alternative<OpenedPage>(model.OpenItem(
      shortcut, kResearchCaller, PageId{"duplicate-title-page"})));
  const auto duplicates = Rows(model, labels, "Renamed pin");
  REQUIRE(Identities(duplicates) ==
          (std::vector<RankingIdentity>{detached, shortcut}));
  REQUIRE(duplicates.front().title == duplicates.back().title);
  REQUIRE(duplicates.front().verb() == RankedVerb::kOpenSavedDestination);
  REQUIRE(duplicates.back().verb() == RankedVerb::kSwitch);
  REQUIRE(duplicates.front().provenance.owning_space == "Offline research");
  REQUIRE(!duplicates.back().provenance.owning_space);
  REQUIRE(duplicates.back().provenance.live_space == kResearch.value);
}

void TemporaryPagesStayInCallerWindowAndProfile() {
  auto model = Workspace();
  const TemporaryTabId local{"local"};
  const TemporaryTabId other_space{"other-space-local"};
  REQUIRE(model.OpenTemporary(PageId{"local-page"}, local, kDeskCaller,
                              kSaved) == Status::kOk);
  REQUIRE(model.OpenTemporary(PageId{"research-page"}, other_space,
                              kResearchCaller, kSaved) == Status::kOk);
  REQUIRE(model.OpenTemporary(PageId{"remote-page"}, TemporaryTabId{"remote"},
                              kDeskOther, kSaved) == Status::kOk);
  auto rows = Rows(model);
  REQUIRE(Identities(rows) ==
          (std::vector<RankingIdentity>{local, other_space}));
  const auto& row = Find(rows, other_space);
  const auto& reveal = std::get<RevealTemporaryPage>(row.reveal);
  REQUIRE(reveal.tab == other_space);
  REQUIRE(reveal.live.page == PageId{"research-page"});
  REQUIRE(reveal.live.attachment == kResearchCaller);
  REQUIRE(row.profile == kWork);
  REQUIRE(row.provenance.profile == kWork.value);
  REQUIRE(row.provenance.live_space == kResearch.value);
  REQUIRE(row.provenance.live_window == kCaller.value);
  REQUIRE(!row.provenance.owning_space);
  REQUIRE(RankedVerbLabel(row.verb()) == "Switch");
  REQUIRE(model.windows().at(kCaller).selected_space == kDesk);
}

void SavedSwitchPreservesExistingLifecycleIdentity() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kMail, kDesk, kSaved) == Status::kOk);
  auto opened = model.OpenItem(kMail, kDeskOther, kMailPage);
  REQUIRE(std::get<OpenedPage>(opened).disposition ==
          OpenDisposition::kCreated);
  REQUIRE(model.RecordNavigation(kMailPage, kCurrent) == Status::kOk);
  REQUIRE(model.ReportActivity(kMailPage, Activity::kProtected) == Status::kOk);
  REQUIRE(model.SetSplit(SplitId{"source-split"}, kDeskOther, {kMailPage}) ==
          Status::kOk);
  RankingDisplayLabels labels;
  labels.items.emplace(kMail, "Renamed mail pin");
  labels.pages.emplace(kMailPage, "Compose draft");
  labels.spaces.emplace(kDesk, "Desk");
  labels.windows.emplace(kOtherWindow, "Other window");
  const auto rows = Rows(model, labels, "Compose");
  REQUIRE(rows.size() == 1);
  const auto& row = rows.front();
  const auto& reveal = std::get<RevealSavedItem>(row.reveal);
  REQUIRE(row.identity() == RankingIdentity{kMail});
  REQUIRE(row.verb() == RankedVerb::kSwitch);
  REQUIRE(row.title == "Renamed mail pin");
  REQUIRE(reveal.live.has_value());
  REQUIRE(reveal.live.value().page == kMailPage);
  REQUIRE(reveal.live.value().attachment == kDeskOther);
  REQUIRE(row.provenance.owning_space == "Desk");
  REQUIRE(row.provenance.live_space == "Desk");
  REQUIRE(row.provenance.live_window == "Other window");
  REQUIRE(Rows(model, labels, "unsaved-compose").size() == 1);
  REQUIRE(Rows(model, labels, "inbox").size() == 1);
  // Ranking did not transfer, reset, close, change activity or prune layouts.
  REQUIRE(model.pages().at(kMailPage).attachment() == kDeskOther);
  REQUIRE(model.pages().at(kMailPage).current_navigation == kCurrent);
  REQUIRE(model.pages().at(kMailPage).activity == Activity::kProtected);
  REQUIRE(model.items().at(kMail).saved_destination == kSaved);
  REQUIRE(model.splits().at(SplitId{"source-split"}).pages ==
          std::vector<PageId>{kMailPage});
  REQUIRE(model.windows().at(kCaller).selected_space == kDesk);

  // Synthetic executor demonstration only. Native preparation/revalidation
  // belongs to command_target, not this ranking function.
  const PageId unused{"unused-candidate"};
  opened = model.OpenItem(reveal.item, kDeskCaller, unused);
  REQUIRE(std::get<OpenedPage>(opened).page == kMailPage);
  REQUIRE(std::get<OpenedPage>(opened).disposition ==
          OpenDisposition::kExisting);
  REQUIRE(model.pages().size() == 1);
  REQUIRE(!model.pages().count(unused));
  REQUIRE(model.pages().at(kMailPage).profile == kWork);
  REQUIRE(model.pages().at(kMailPage).attachment() == kDeskCaller);
  REQUIRE(model.pages().at(kMailPage).current_navigation == kCurrent);
  REQUIRE(model.pages().at(kMailPage).activity == Activity::kProtected);
  REQUIRE(model.splits().empty());
  const auto transferred = Rows(model, labels);
  REQUIRE(transferred.size() == 1);
  REQUIRE(transferred.front().identity() == row.identity());
  const auto& transferred_target =
      std::get<RevealSavedItem>(transferred.front().reveal);
  REQUIRE(transferred_target.live.has_value());
  REQUIRE(transferred_target.live.value().attachment == kDeskCaller);
  // Owned snapshots remain snapshots. The executor must never trust stale rows.
  REQUIRE(reveal.live.value().attachment == kDeskOther);
}

void ProfileShortcutHasLiveLocationButNoOwningSpace() {
  auto model = Workspace();
  const ItemId shortcut{"shortcut"};
  REQUIRE(model.AddItem(shortcut, kWork, kSaved) == Status::kOk);
  REQUIRE(std::holds_alternative<OpenedPage>(
      model.OpenItem(shortcut, kResearchOther, PageId{"shortcut-page"})));
  auto rows = Rows(model);
  REQUIRE(rows.size() == 1);
  const auto& row = rows.front();
  REQUIRE(!row.provenance.owning_space);
  REQUIRE(row.provenance.live_space == kResearch.value);
  REQUIRE(row.provenance.live_window == kOtherWindow.value);
  REQUIRE(std::get<ProfileId>(std::get<RevealSavedItem>(row.reveal).owner) ==
          kWork);
  REQUIRE(row.verb() == RankedVerb::kSwitch);
  REQUIRE(model.ReportActivity(PageId{"shortcut-page"}, Activity::kQuiescent) ==
          Status::kOk);
  REQUIRE(model.ClosePage(PageId{"shortcut-page"}) == Status::kOk);
  const auto closed = Rows(model);
  REQUIRE(closed.size() == 1);
  REQUIRE(closed.front().identity() == row.identity());
  REQUIRE(closed.front().verb() == RankedVerb::kOpenSavedDestination);
  REQUIRE(!std::get<RevealSavedItem>(closed.front().reveal).live);
}

struct CorpusEntry {
  const char* id;
  const char* title;
  const char* destination;
};

// Public synthetic corpus. The vector size describes this fixture, not a
// product limit. No network requests, benchmark or timing assertion occurs.
const std::vector<CorpusEntry> kCorpus{
    {"a-substring", "My alpha notes", "https://notes.example.test/substring"},
    {"b-prefix", "Alpha notes", "https://notes.example.test/prefix"},
    {"c-exact", "ALPHA", "https://notes.example.test/exact"},
    {"d-duplicate", "Alpha", "https://notes.example.test/duplicate"},
    {"e-long-url", "",
     "https://long.example.test/path/to/a/synthetic/document/"
     "whose/destination/is/longer/than/the/visible/row/and/"
     "continues/through/more/path/segments/to/a/searchable/"
     "tail-marker?query=synthetic&source=ranking-fixture"},
    {"f-unicode", "Räksmörgås", "https://unicode.example.test/document"},
};

Model Corpus(bool reverse) {
  auto model = Workspace();
  auto entries = kCorpus;
  if (reverse)
    std::reverse(entries.begin(), entries.end());
  for (const auto& entry : entries)
    REQUIRE(model.AddItem(ItemId{entry.id}, kDesk, entry.destination) ==
            Status::kOk);
  return model;
}

void RepeatableCorpusOrderingAndFreshProjection() {
  auto model = Corpus(false);
  auto reversed = Corpus(true);
  RankingDisplayLabels labels;
  for (const auto& entry : kCorpus)
    labels.items.emplace(ItemId{entry.id}, entry.title);
  const auto rows = Rows(model, labels, "aLpHa");
  const std::vector<RankingIdentity> expected{
      ItemId{"c-exact"}, ItemId{"d-duplicate"}, ItemId{"b-prefix"},
      ItemId{"a-substring"}};
  REQUIRE(Identities(rows) == expected);
  REQUIRE(rows.front().match == RankingMatch::kExact);
  REQUIRE(rows.back().match == RankingMatch::kSubstring);
  REQUIRE(rows.front().verb() == RankedVerb::kOpenSavedDestination);
  const std::vector<std::pair<std::string, std::vector<RankingIdentity>>>
      queries{
          {"",
           {ItemId{"a-substring"}, ItemId{"b-prefix"}, ItemId{"c-exact"},
            ItemId{"d-duplicate"}, ItemId{"e-long-url"}, ItemId{"f-unicode"}}},
          {"aLpHa", expected},
          {"tail-marker", {ItemId{"e-long-url"}}},
          {"Räks", {ItemId{"f-unicode"}}},
          {"absent", {}},
  };
  for (const auto& [query, expected_ids] : queries) {
    REQUIRE(Identities(Rows(model, labels, query)) == expected_ids);
    REQUIRE(Identities(Rows(reversed, labels, query)) == expected_ids);
  }
  REQUIRE(Rows(model, labels).size() == kCorpus.size());
  const auto long_rows = Rows(model, labels, "tail-marker");
  REQUIRE(long_rows.size() == 1);
  REQUIRE(long_rows.front().identity() ==
          RankingIdentity{ItemId{"e-long-url"}});
  REQUIRE(long_rows.front().title == kCorpus.at(4).destination);
  REQUIRE(Rows(model, labels, "Räks").size() == 1);
  REQUIRE(Rows(model, labels, "RÄKS").empty());  // ASCII folding only.
  REQUIRE(Rows(model, labels, "absent").empty());

  const PageId loading_page{"loading-page"};
  REQUIRE(std::holds_alternative<OpenedPage>(
      model.OpenItem(ItemId{"b-prefix"}, kDeskCaller, loading_page)));
  labels.pages.emplace(loading_page, "");
  auto loading = Rows(model, labels, "alpha");
  REQUIRE(Identities(loading) == expected);
  REQUIRE(Find(loading, ItemId{"b-prefix"}).verb() == RankedVerb::kSwitch);
  labels.pages.at(loading_page) = "Loaded document title";
  REQUIRE(Identities(Rows(model, labels, "alpha")) == expected);
  labels.items.at(ItemId{"b-prefix"}) = "Renamed pin";
  auto renamed = Rows(model, labels, "Renamed pin");
  REQUIRE(renamed.size() == 1);
  REQUIRE(renamed.front().identity() == RankingIdentity{ItemId{"b-prefix"}});
  REQUIRE(Rows(model, labels, "Loaded document title").size() == 1);
  REQUIRE(Rows(model, labels, "alpha").size() == 3);
  REQUIRE(Rows(model, labels, "prefix").size() == 1);
  REQUIRE(model.AddItem(ItemId{"unrelated-growth"}, kResearch,
                        "https://growth.example.test/unrelated") ==
          Status::kOk);
  REQUIRE(Identities(Rows(model, labels, "Renamed pin")) ==
          Identities(renamed));
  REQUIRE(model.RemoveItem(ItemId{"c-exact"}) == Status::kOk);
  REQUIRE(Rows(model, labels, "alpha").size() == 2);
}

void EmptyWorkspaceAndEmptyLoadingLabel() {
  auto model = Workspace();
  REQUIRE(Rows(model).empty());
  REQUIRE(Rows(model, {}, "anything").empty());
  const PageId page{"loading-page"};
  const TemporaryTabId tab{"loading-tab"};
  REQUIRE(model.OpenTemporary(page, tab, kDeskCaller, kSaved) == Status::kOk);
  RankingDisplayLabels labels;
  labels.pages.emplace(page, "");
  auto rows = Rows(model, labels);
  REQUIRE(rows.size() == 1);
  REQUIRE(rows.front().title == kSaved);
  labels.pages.at(page) = "Mail loaded";
  REQUIRE(Identities(Rows(model, labels)) == Identities(rows));
  REQUIRE(Rows(model, labels, "Mail loaded").size() == 1);
  REQUIRE(model.RecordNavigation(page, kCurrent) == Status::kOk);
  REQUIRE(Rows(model, labels, "unsaved-compose").size() == 1);
  REQUIRE(Rows(model, labels, "inbox").empty());
}

}  // namespace
}  // namespace arcified::commands

int main() try {
  using namespace arcified::commands;
  ScopeRefusalsAndIsolation();
  ClosedDiscoveryAndProfileShortcutProvenance();
  TemporaryPagesStayInCallerWindowAndProfile();
  SavedSwitchPreservesExistingLifecycleIdentity();
  ProfileShortcutHasLiveLocationButNoOwningSpace();
  RepeatableCorpusOrderingAndFreshProjection();
  EmptyWorkspaceAndEmptyLoadingLabel();
  std::cout << "command-ranking synthetic checks passed\n";
} catch (const std::exception& exception) {
  std::cerr << "command-ranking fixture exception: " << exception.what()
            << '\n';
  return EXIT_FAILURE;
}
