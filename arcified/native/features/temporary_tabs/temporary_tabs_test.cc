#include "temporary_tabs.h"

#include <cstdlib>
#include <iostream>

namespace arcified::temporary_tabs {
namespace {

namespace lc = lifecycle;
static_assert(lc::kContractVersion == 3);

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const lc::ProfileId kWork{"work"};
const lc::ProfileId kPersonal{"personal"};
const lc::SpaceId kDesk{"desk"};
const lc::SpaceId kOther{"other"};
const lc::SpaceId kHome{"home"};
const lc::WindowId kFirst{"first"};
const lc::WindowId kSecond{"second"};
const lc::Attachment kDeskFirst{kFirst, kDesk};
const lc::Attachment kDeskSecond{kSecond, kDesk};
const lc::Attachment kOtherFirst{kFirst, kOther};
const lc::Attachment kHomeFirst{kFirst, kHome};
const lc::PageId kDraft{"draft"};
const lc::PageId kSibling{"sibling"};
const lc::TemporaryTabId kDraftTab{"draft-tab"};
const lc::TemporaryTabId kSiblingTab{"sibling-tab"};
const std::string kDestination = "https://example.test/draft";

lc::Model Workspace() {
  lc::Model model;
  REQUIRE(model.AddProfile(kWork) == lc::Status::kOk);
  REQUIRE(model.AddProfile(kPersonal) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kOther, kWork) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kHome, kPersonal) == lc::Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == lc::Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == lc::Status::kOk);
  REQUIRE(model.Attach(kDeskFirst) == lc::Status::kOk);
  REQUIRE(model.Attach(kDeskSecond) == lc::Status::kOk);
  REQUIRE(model.Attach(kOtherFirst) == lc::Status::kOk);
  REQUIRE(model.Attach(kHomeFirst) == lc::Status::kOk);
  return model;
}

void LocalIdentitySurvivesSpaceSelection() {
  auto model = Workspace();
  REQUIRE(model.OpenTemporary(kDraft, kDraftTab, kDeskFirst, kDestination) ==
          lc::Status::kOk);
  // Equal destinations never merge pages across windows.
  REQUIRE(model.OpenTemporary(kSibling, kSiblingTab, kDeskSecond,
                              kDestination) == lc::Status::kOk);
  REQUIRE(model.ReportActivity(kDraft, lc::Activity::kProtected) ==
          lc::Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kHome) == lc::Status::kOk);
  REQUIRE(model.TemporaryPages(kDeskFirst) == std::vector<lc::PageId>{kDraft});
  REQUIRE(model.TemporaryPages(kDeskSecond) ==
          std::vector<lc::PageId>{kSibling});
  REQUIRE(model.TemporaryPages(kHomeFirst).empty());
  REQUIRE(model.pages().at(kDraft).profile == kWork);
  REQUIRE(model.pages().at(kDraft).attachment() == kDeskFirst);
  REQUIRE(model.pages().at(kDraft).activity == lc::Activity::kProtected);
  REQUIRE(model.TransferSavedPage(kDraft, kDeskSecond) ==
          lc::Status::kInvalidInput);
  REQUIRE(model.RecordNavigation(kDraft, "https://example.test/edited") ==
          lc::Status::kOk);
  REQUIRE(model.pages().at(kSibling).current_navigation == kDestination);
  REQUIRE(model.pages().at(kDraft).activity == lc::Activity::kUnknown);
  REQUIRE(model.ClosePage(kDraft) == lc::Status::kWorkNotQuiescent);
  REQUIRE(model.CloseWindow(kFirst) == lc::Status::kWorkNotQuiescent);
  REQUIRE(model.pages().size() == 2);
  REQUIRE(model.windows().at(kFirst).selected_space == kHome);
}

void FailedCreationAndCloseKeepOtherIdentities() {
  auto model = Workspace();
  REQUIRE(model.OpenTemporary(kDraft, kDraftTab, kDeskFirst, kDestination) ==
          lc::Status::kOk);
  REQUIRE(model.OpenTemporary(kSibling, kDraftTab, kDeskSecond, kDestination) ==
          lc::Status::kDuplicateId);
  REQUIRE(!model.pages().count(kSibling));
  REQUIRE(model.OpenTemporary(kSibling, kSiblingTab, kDeskSecond,
                              kDestination) == lc::Status::kOk);
  REQUIRE(model.ReportActivity(kDraft, lc::Activity::kQuiescent) ==
          lc::Status::kOk);
  REQUIRE(model.ClosePage(kDraft) == lc::Status::kOk);
  REQUIRE(model.TemporaryPages(kDeskFirst).empty());
  REQUIRE(model.TemporaryPages(kDeskSecond) ==
          std::vector<lc::PageId>{kSibling});
  REQUIRE(model.pages().at(kSibling).current_navigation == kDestination);
  REQUIRE(model.pages().at(kSibling).activity == lc::Activity::kUnknown);
  // No closed-tab record, selection or reopened page is invented.
  REQUIRE(model.items().empty());
  REQUIRE(!model.windows().at(kFirst).selected_space.has_value());
}

void TemporaryIntentExpiresOnKeepOrClose() {
  auto model = Workspace();
  REQUIRE(ResolveTemporaryIdentity(model, kDraft) ==
          IdentityResult{Refusal::kPageNotFound});
  REQUIRE(model.OpenTemporary(kDraft, kDraftTab, kDeskFirst, kDestination) ==
          lc::Status::kOk);
  const auto resolved = ResolveTemporaryIdentity(model, kDraft);
  REQUIRE(std::get<lc::TemporaryTabId>(resolved) == kDraftTab);
  REQUIRE(!ValidateTemporaryIdentity(model, kDraft, kDraftTab));
  REQUIRE(ValidateTemporaryIdentity(model, kDraft, kSiblingTab) ==
          Refusal::kIdentityChanged);
  REQUIRE(model.RecordNavigation(kDraft, "https://example.test/edited") ==
          lc::Status::kOk);
  REQUIRE(model.ReportActivity(kDraft, lc::Activity::kProtected) ==
          lc::Status::kOk);
  REQUIRE(!ValidateTemporaryIdentity(model, kDraft, kDraftTab));
  REQUIRE(model.Keep(kDraft, lc::ItemId{"kept"}) == lc::Status::kOk);
  REQUIRE(ResolveTemporaryIdentity(model, kDraft) ==
          IdentityResult{Refusal::kNotTemporary});
  REQUIRE(ValidateTemporaryIdentity(model, kDraft, kDraftTab) ==
          Refusal::kNotTemporary);
  REQUIRE(model.pages().at(kDraft).activity == lc::Activity::kProtected);
  // The snapshot owns its identity; model mutations cannot change it.
  REQUIRE(std::get<lc::TemporaryTabId>(resolved) == kDraftTab);

  REQUIRE(model.OpenTemporary(kSibling, kSiblingTab, kDeskFirst,
                              kDestination) == lc::Status::kOk);
  const auto closing = ResolveTemporaryIdentity(model, kSibling);
  REQUIRE(model.ReportActivity(kSibling, lc::Activity::kQuiescent) ==
          lc::Status::kOk);
  REQUIRE(model.ClosePage(kSibling) == lc::Status::kOk);
  REQUIRE(ValidateTemporaryIdentity(model, kSibling,
                                    std::get<lc::TemporaryTabId>(closing)) ==
          Refusal::kPageNotFound);
  REQUIRE(model.OpenTemporary(lc::PageId{"new-page"}, lc::TemporaryTabId{"new"},
                              kDeskFirst, kDestination) == lc::Status::kOk);
  REQUIRE(ValidateTemporaryIdentity(model, kSibling, kSiblingTab) ==
          Refusal::kPageNotFound);
}

void CompleteNativeOrderProjection() {
  auto model = Workspace();
  REQUIRE(model.OpenTemporary(kDraft, kDraftTab, kDeskFirst, kDestination) ==
          lc::Status::kOk);
  REQUIRE(model.OpenTemporary(kSibling, kSiblingTab, kDeskFirst,
                              kDestination) == lc::Status::kOk);
  const lc::PageId foreign{"foreign"};
  const lc::TemporaryTabId foreign_tab{"foreign-tab"};
  REQUIRE(model.OpenTemporary(foreign, foreign_tab, kDeskSecond,
                              kDestination) == lc::Status::kOk);
  REQUIRE(model.ReportActivity(kDraft, lc::Activity::kProtected) ==
          lc::Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kHome) == lc::Status::kOk);
  const std::vector<lc::PageId> native_order{kSibling, kDraft};
  const auto result =
      ProjectTemporaryIdentities(model, kDeskFirst, native_order);
  const auto& rows = std::get<std::vector<TemporaryIdentity>>(result);
  REQUIRE(rows.size() == 2);
  REQUIRE(rows[0].page == kSibling);
  REQUIRE(rows[0].tab == kSiblingTab);
  REQUIRE(rows[1].page == kDraft);
  REQUIRE(rows[1].tab == kDraftTab);
  REQUIRE(model.pages().size() == 3);
  REQUIRE(model.pages().at(kDraft).activity == lc::Activity::kProtected);
  REQUIRE(model.windows().at(kFirst).selected_space == kHome);
  REQUIRE(native_order == (std::vector<lc::PageId>{kSibling, kDraft}));

  const auto other = ProjectTemporaryIdentities(model, kDeskSecond, {foreign});
  REQUIRE(std::get<std::vector<TemporaryIdentity>>(other)[0].tab ==
          foreign_tab);
  REQUIRE(std::get<std::vector<TemporaryIdentity>>(
              ProjectTemporaryIdentities(model, kOtherFirst, {}))
              .empty());
  REQUIRE(model.Keep(kDraft, lc::ItemId{"kept"}) == lc::Status::kOk);
  // A prior projection is an owned snapshot, never an authoritative order.
  REQUIRE(rows[1].page == kDraft);
  REQUIRE(rows[1].tab == kDraftTab);
  REQUIRE(ValidateTemporaryIdentity(model, rows[1].page, rows[1].tab) ==
          Refusal::kNotTemporary);
  const auto after_keep =
      ProjectTemporaryIdentities(model, kDeskFirst, {kSibling});
  REQUIRE(std::get<std::vector<TemporaryIdentity>>(after_keep).size() == 1);
}

void ProjectionRejectsIncompleteOrInvalidNativeSnapshots() {
  auto model = Workspace();
  REQUIRE(model.OpenTemporary(kDraft, kDraftTab, kDeskFirst, kDestination) ==
          lc::Status::kOk);
  REQUIRE(model.OpenTemporary(kSibling, kSiblingTab, kDeskFirst,
                              kDestination) == lc::Status::kOk);
  const lc::PageId foreign{"foreign"};
  REQUIRE(model.OpenTemporary(foreign, lc::TemporaryTabId{"foreign-tab"},
                              kDeskSecond, kDestination) == lc::Status::kOk);
  REQUIRE(std::get<Refusal>(ProjectTemporaryIdentities(
              model, kDeskFirst, {})) == Refusal::kIncompleteOrder);
  REQUIRE(std::get<Refusal>(ProjectTemporaryIdentities(
              model, kDeskFirst, {kDraft})) == Refusal::kIncompleteOrder);
  REQUIRE(std::get<Refusal>(ProjectTemporaryIdentities(
              model, kDeskFirst, {kDraft, kSibling, kDraft})) ==
          Refusal::kDuplicatePage);
  REQUIRE(std::get<Refusal>(ProjectTemporaryIdentities(
              model, kDeskFirst, {kDraft, lc::PageId{"stale"}})) ==
          Refusal::kPageNotFound);
  REQUIRE(std::get<Refusal>(ProjectTemporaryIdentities(model, kDeskFirst,
                                                       {kDraft, foreign})) ==
          Refusal::kWrongAttachment);
  const lc::PageId other_space{"other-space"};
  REQUIRE(model.OpenTemporary(other_space,
                              lc::TemporaryTabId{"other-space-tab"},
                              kOtherFirst, kDestination) == lc::Status::kOk);
  REQUIRE(std::get<Refusal>(ProjectTemporaryIdentities(
              model, kDeskFirst, {kDraft, other_space})) ==
          Refusal::kWrongAttachment);
  const lc::PageId other_profile{"other-profile"};
  REQUIRE(model.OpenTemporary(other_profile,
                              lc::TemporaryTabId{"other-profile-tab"},
                              kHomeFirst, kDestination) == lc::Status::kOk);
  REQUIRE(std::get<Refusal>(ProjectTemporaryIdentities(
              model, kDeskFirst, {kDraft, other_profile})) ==
          Refusal::kWrongAttachment);
  REQUIRE(std::get<Refusal>(ProjectTemporaryIdentities(
              model, {kSecond, kOther}, {})) == Refusal::kAttachmentNotFound);
  REQUIRE(model.Keep(kSibling, lc::ItemId{"kept"}) == lc::Status::kOk);
  REQUIRE(std::get<Refusal>(ProjectTemporaryIdentities(model, kDeskFirst,
                                                       {kDraft, kSibling})) ==
          Refusal::kNotTemporary);
  REQUIRE(model.pages().size() == 5);
  REQUIRE(model.TemporaryPages(kDeskFirst) == std::vector<lc::PageId>{kDraft});
  REQUIRE(model.pages().at(kDraft).current_navigation == kDestination);
  REQUIRE(model.pages().at(kDraft).activity == lc::Activity::kUnknown);
}

}  // namespace
}  // namespace arcified::temporary_tabs

int main() {
  arcified::temporary_tabs::LocalIdentitySurvivesSpaceSelection();
  arcified::temporary_tabs::FailedCreationAndCloseKeepOtherIdentities();
  arcified::temporary_tabs::TemporaryIntentExpiresOnKeepOrClose();
  arcified::temporary_tabs::CompleteNativeOrderProjection();
  arcified::temporary_tabs::
      ProjectionRejectsIncompleteOrInvalidNativeSnapshots();
  std::cout << "temporary tab fixtures passed\n";
}
