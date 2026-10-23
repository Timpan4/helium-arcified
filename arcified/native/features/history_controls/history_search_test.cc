#include "history_search.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <utility>

namespace arcified::history_controls {
namespace {

using lifecycle::Attachment;
using lifecycle::Model;
using lifecycle::PageId;
using lifecycle::ProfileId;
using lifecycle::SpaceId;
using lifecycle::Status;
using lifecycle::WindowId;

static_assert(lifecycle::kContractVersion == 3);
static_assert(!std::is_convertible_v<HistoryQueryId, ProfileId>);
static_assert(!std::is_convertible_v<HistoryQueryId, PageId>);

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const ProfileId kWork{"work"};
const ProfileId kPersonal{"personal"};
const SpaceId kDesk{"desk"};
const SpaceId kResearch{"research"};
const SpaceId kHome{"home"};
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const HistoryContext kWorkContext{kWork, HistoryContextKind::kRegular};
const HistoryContext kPersonalContext{kPersonal, HistoryContextKind::kRegular};
const HistoryQueryId kQuery{"query"};
const HistoryQueryId kNewQuery{"new-query"};

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kPersonal) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kResearch, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kHome, kPersonal) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  REQUIRE(model.Attach({kFirst, kDesk}) == Status::kOk);
  REQUIRE(model.Attach({kFirst, kResearch}) == Status::kOk);
  REQUIRE(model.Attach({kFirst, kHome}) == Status::kOk);
  REQUIRE(model.Attach({kSecond, kDesk}) == Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kDesk) == Status::kOk);
  REQUIRE(model.SelectSpace(kSecond, kDesk) == Status::kOk);
  return model;
}

HistorySearchPlan Plan(const Model& model,
                       const WindowId& window = kFirst,
                       const HistoryContext& context = kWorkContext,
                       const HistoryQueryId& query = kQuery,
                       std::u16string text = u"example.test") {
  auto result =
      PlanHistorySearch(model, window, context, query, std::move(text));
  REQUIRE(std::holds_alternative<HistorySearchPlan>(result));
  return std::get<HistorySearchPlan>(std::move(result));
}

void ExpectRefusal(HistorySearchResult result, HistorySearchRefusal expected) {
  REQUIRE(std::holds_alternative<HistorySearchRefusal>(result));
  REQUIRE(std::get<HistorySearchRefusal>(result) == expected);
}

void SearchSyntax() {
  struct Case {
    std::u16string input;
    std::u16string text;
    bool host_only;
  };
  const Case cases[] = {
      {u"", u"", false},
      {u"host:", u"", true},
      {u"host:example.test", u"example.test", true},
      {u"host:host:example.test", u"host:example.test", true},
      {u"Host:example.test", u"Host:example.test", false},
      {u" host:example.test", u" host:example.test", false},
      {u"title host:example.test", u"title host:example.test", false},
      {u"https://example.test/host:path", u"https://example.test/host:path",
       false},
      {u"host:例.test", u"例.test", true},
      {u" \"résumé\" 日本語 🐈 ", u" \"résumé\" 日本語 🐈 ", false},
  };
  for (const auto& test : cases) {
    const auto parsed = ParseHistorySearchText(test.input);
    REQUIRE(parsed.text == test.text);
    REQUIRE(parsed.host_only == test.host_only);
  }
  // A query is a string, not a C string or URL to normalize.
  const std::u16string with_nul{u'h', u'o', u's', u't', u':', u'\0', u'x'};
  const auto parsed = ParseHistorySearchText(with_nul);
  REQUIRE(parsed.host_only);
  REQUIRE(parsed.text == std::u16string({u'\0', u'x'}));
}

void ProfileScopeAndOwnedSnapshot() {
  auto model = Workspace();
  auto query = kQuery;
  std::u16string text = u"host:example.test";
  auto result = PlanHistorySearch(model, kFirst, kWorkContext, query, text);
  REQUIRE(std::holds_alternative<HistorySearchPlan>(result));
  const auto& plan = std::get<HistorySearchPlan>(result);
  query.value = "changed";
  text = u"changed";
  REQUIRE(plan.query == kQuery);
  REQUIRE(plan.attachment == (Attachment{kFirst, kDesk}));
  REQUIRE(plan.profile == kWork);
  REQUIRE(plan.search.text == u"example.test");
  REQUIRE(plan.search.host_only);

  const auto other_window = Plan(model, kSecond);
  REQUIRE(other_window.profile == plan.profile);
  REQUIRE(other_window.attachment == (Attachment{kSecond, kDesk}));
  REQUIRE(model.SelectSpace(kFirst, kResearch) == Status::kOk);
  const auto other_space = Plan(model, kFirst, kWorkContext, kNewQuery);
  REQUIRE(other_space.profile == plan.profile);
  REQUIRE(other_space.attachment == (Attachment{kFirst, kResearch}));
  REQUIRE(model.SelectSpace(kFirst, kHome) == Status::kOk);
  const auto personal = Plan(model, kFirst, kPersonalContext, kNewQuery);
  REQUIRE(personal.profile == kPersonal);
  REQUIRE(personal.attachment.window == kFirst);
}

void PreparationRefusals() {
  auto model = Workspace();
  ExpectRefusal(
      PlanHistorySearch(model, kFirst, kWorkContext, HistoryQueryId{""}, u""),
      HistorySearchRefusal::kInvalidQueryId);
  ExpectRefusal(
      PlanHistorySearch(model, WindowId{"missing"}, kWorkContext, kQuery, u""),
      HistorySearchRefusal::kWindowNotFound);
  const WindowId empty{"empty"};
  REQUIRE(model.AddWindow(empty) == Status::kOk);
  ExpectRefusal(PlanHistorySearch(model, empty, kWorkContext, kQuery, u""),
                HistorySearchRefusal::kNoSelectedSpace);
  ExpectRefusal(PlanHistorySearch(model, kFirst, kPersonalContext, kQuery, u""),
                HistorySearchRefusal::kProfileMismatch);
  for (const auto& profile : {kWork, kPersonal}) {
    ExpectRefusal(
        PlanHistorySearch(model, kFirst,
                          {profile, HistoryContextKind::kOffTheRecord}, kQuery,
                          u""),
        HistorySearchRefusal::kPrivateContext);
    ExpectRefusal(PlanHistorySearch(model, kFirst,
                                    {profile, HistoryContextKind::kUnsupported},
                                    kQuery, u""),
                  HistorySearchRefusal::kUnsupportedContext);
  }
}

void CompletionLifetime() {
  auto model = Workspace();
  const auto plan = Plan(model);
  std::optional<HistoryQueryId> active = plan.query;
  REQUIRE(!CheckHistorySearchCompletion(model, active, kWorkContext, plan));
  // Same text in a replacement query never authorizes the earlier completion.
  const auto replacement = Plan(model, kFirst, kWorkContext, kNewQuery);
  active = replacement.query;
  REQUIRE(CheckHistorySearchCompletion(model, active, kWorkContext, plan) ==
          HistorySearchRefusal::kStaleQuery);
  REQUIRE(
      !CheckHistorySearchCompletion(model, active, kWorkContext, replacement));
  active.reset();
  REQUIRE(
      CheckHistorySearchCompletion(model, active, kWorkContext, replacement) ==
      HistorySearchRefusal::kStaleQuery);
  active = plan.query;
  REQUIRE(CheckHistorySearchCompletion(model, active, kPersonalContext, plan) ==
          HistorySearchRefusal::kProfileMismatch);
  REQUIRE(CheckHistorySearchCompletion(
              model, active, {kWork, HistoryContextKind::kOffTheRecord},
              plan) == HistorySearchRefusal::kPrivateContext);
  REQUIRE(CheckHistorySearchCompletion(
              model, active, {kWork, HistoryContextKind::kUnsupported}, plan) ==
          HistorySearchRefusal::kUnsupportedContext);
  // Publication consumes the active ID, so a duplicate delivery is stale.
  REQUIRE(!CheckHistorySearchCompletion(model, active, kWorkContext, plan));
  active.reset();
  REQUIRE(CheckHistorySearchCompletion(model, active, kWorkContext, plan) ==
          HistorySearchRefusal::kStaleQuery);
}

void SelectionAndRemovalRaces() {
  auto model = Workspace();
  const auto plan = Plan(model);
  std::optional<HistoryQueryId> active = plan.query;
  REQUIRE(model.SelectSpace(kFirst, kResearch) == Status::kOk);
  REQUIRE(CheckHistorySearchCompletion(model, active, kWorkContext, plan) ==
          HistorySearchRefusal::kSelectionChanged);
  REQUIRE(model.SelectSpace(kFirst, kHome) == Status::kOk);
  REQUIRE(CheckHistorySearchCompletion(model, active, kPersonalContext, plan) ==
          HistorySearchRefusal::kSelectionChanged);
  REQUIRE(CheckHistorySearchCompletion(model, active, kWorkContext, plan) ==
          HistorySearchRefusal::kProfileMismatch);

  // The caller must invalidate on every switch, including away and back.
  active.reset();
  REQUIRE(model.SelectSpace(kFirst, kDesk) == Status::kOk);
  REQUIRE(CheckHistorySearchCompletion(model, active, kWorkContext, plan) ==
          HistorySearchRefusal::kStaleQuery);
  active = plan.query;
  REQUIRE(model.Detach({kFirst, kDesk}) == Status::kOk);
  REQUIRE(CheckHistorySearchCompletion(model, active, kWorkContext, plan) ==
          HistorySearchRefusal::kNoSelectedSpace);
  REQUIRE(model.CloseWindow(kFirst) == Status::kOk);
  REQUIRE(CheckHistorySearchCompletion(model, active, kWorkContext, plan) ==
          HistorySearchRefusal::kWindowNotFound);
  const auto surviving = Plan(model, kSecond);
  REQUIRE(!CheckHistorySearchCompletion(model, surviving.query, kWorkContext,
                                        surviving));

  // Even the same attachment cannot publish the old Profile's results after
  // an allowed empty-Space reassignment. This guard also covers missed ID
  // invalidation at that boundary.
  REQUIRE(model.Detach({kSecond, kDesk}) == Status::kOk);
  REQUIRE(model.ReassignEmptySpace(kDesk, kPersonal) == Status::kOk);
  REQUIRE(model.Attach({kSecond, kDesk}) == Status::kOk);
  REQUIRE(model.SelectSpace(kSecond, kDesk) == Status::kOk);
  REQUIRE(CheckHistorySearchCompletion(model, surviving.query, kPersonalContext,
                                       surviving) ==
          HistorySearchRefusal::kProfileMismatch);
  const auto reassigned = Plan(model, kSecond, kPersonalContext, kNewQuery);
  REQUIRE(!CheckHistorySearchCompletion(model, reassigned.query,
                                        kPersonalContext, reassigned));
}

void ReadOnlyWorkspace() {
  auto model = Workspace();
  const lifecycle::ItemId item{"saved"};
  const PageId saved{"saved-page"};
  const PageId temporary{"temporary-page"};
  const lifecycle::SplitId split{"split"};
  const Attachment attachment{kFirst, kDesk};
  const std::string destination = "https://example.test/saved";
  const std::string navigation = "https://example.test/draft";
  REQUIRE(model.AddItem(item, kDesk, destination) == Status::kOk);
  REQUIRE(std::holds_alternative<lifecycle::OpenedPage>(
      model.OpenItem(item, attachment, saved)));
  REQUIRE(model.RecordNavigation(saved, navigation) == Status::kOk);
  REQUIRE(model.ReportActivity(saved, lifecycle::Activity::kProtected) ==
          Status::kOk);
  REQUIRE(model.OpenTemporary(temporary, lifecycle::TemporaryTabId{"temporary"},
                              attachment, navigation) == Status::kOk);
  REQUIRE(model.SetSplit(split, attachment, {saved, temporary}) == Status::kOk);
  const auto plan = Plan(model);
  REQUIRE(!CheckHistorySearchCompletion(model, plan.query, kWorkContext, plan));
  ExpectRefusal(
      PlanHistorySearch(model, kFirst, kPersonalContext, kNewQuery, u""),
      HistorySearchRefusal::kProfileMismatch);

  REQUIRE(model.windows().at(kFirst).selected_space == kDesk);
  REQUIRE(model.items().size() == 1);
  REQUIRE(model.items().at(item).saved_destination == destination);
  REQUIRE(model.pages().size() == 2);
  REQUIRE(model.pages().at(saved).profile == kWork);
  REQUIRE(model.pages().at(saved).current_navigation == navigation);
  REQUIRE(model.pages().at(saved).activity == lifecycle::Activity::kProtected);
  REQUIRE(model.pages().at(saved).attachment() == attachment);
  REQUIRE(model.TemporaryPages(attachment) == std::vector<PageId>{temporary});
  REQUIRE(model.splits().at(split).pages ==
          (std::vector<PageId>{saved, temporary}));
}

}  // namespace
}  // namespace arcified::history_controls

int main() {
  using namespace arcified::history_controls;
  SearchSyntax();
  ProfileScopeAndOwnedSnapshot();
  PreparationRefusals();
  CompletionLifetime();
  SelectionAndRemovalRaces();
  ReadOnlyWorkspace();
  std::cout << "history search synthetic checks passed\n";
}
