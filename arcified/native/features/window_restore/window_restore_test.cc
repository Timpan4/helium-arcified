#include "window_restore.h"

#include <cstdlib>
#include <iostream>
#include <set>
#include <tuple>

namespace arcified::window_restore {
namespace {

namespace lc = lifecycle;

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
const lc::SpaceId kHome{"home"};
const lc::WindowId kSource{"source"};
const lc::WindowId kOther{"other"};
const lc::WindowId kTarget{"reopened"};
const lc::PageId kSavedPage{"saved-page"};
const lc::PageId kTemporary{"temporary"};
const lc::PageId kFavoritePage{"favorite-page"};
const lc::ItemId kItem{"shared-item"};
const lc::ItemId kFavorite{"favorite"};
const UndoId kUndo{"undo"};
const AttemptId kAttempt{"attempt"};
const AttemptId kLater{"later-attempt"};

lc::Model Workspace() {
  lc::Model model;
  REQUIRE(model.AddProfile(kWork) == lc::Status::kOk);
  REQUIRE(model.AddProfile(kPersonal) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kHome, kPersonal) == lc::Status::kOk);
  REQUIRE(model.AddWindow(kSource) == lc::Status::kOk);
  REQUIRE(model.AddWindow(kOther) == lc::Status::kOk);
  REQUIRE(model.Attach({kSource, kDesk}) == lc::Status::kOk);
  REQUIRE(model.Attach({kSource, kHome}) == lc::Status::kOk);
  REQUIRE(model.Attach({kOther, kDesk}) == lc::Status::kOk);
  REQUIRE(model.SelectSpace(kSource, kDesk) == lc::Status::kOk);
  REQUIRE(model.AddItem(kItem, kDesk, "https://saved.example.test/") ==
          lc::Status::kOk);
  REQUIRE(model.AddItem(kFavorite, kWork, "https://favorite.example.test/") ==
          lc::Status::kOk);
  REQUIRE(std::holds_alternative<lc::OpenedPage>(
      model.OpenItem(kItem, {kSource, kDesk}, kSavedPage)));
  REQUIRE(std::holds_alternative<lc::OpenedPage>(
      model.OpenItem(kFavorite, {kSource, kDesk}, kFavoritePage)));
  REQUIRE(model.OpenTemporary(
              kTemporary, lc::TemporaryTabId{"old-temporary"}, {kSource, kHome},
              "https://temporary.example.test/") == lc::Status::kOk);
  REQUIRE(model.OpenTemporary(lc::PageId{"other-page"},
                              lc::TemporaryTabId{"other-temporary"},
                              {kOther, kDesk}, "https://other.example.test/") ==
          lc::Status::kOk);
  return model;
}

// Synthetic identity allowlist, NOT a native privacy classifier. These checks
// prove protocol exclusion only; no native OTR/guest/Profile mapping is tested.
class SyntheticEligibility : public CaptureEligibility {
 public:
  explicit SyntheticEligibility(const lc::Model& model) {
    for (const auto& [id, window] : model.windows())
      windows.insert(id);
    for (const auto& [id, page] : model.pages())
      pages.emplace(page.attachment().window, id, page.profile);
  }
  bool IsWindowVerifiedRegular(const lc::WindowId& window) const override {
    return windows.count(window);
  }
  bool IsPageVerifiedRegular(const lc::WindowId& window,
                             const lc::PageId& page,
                             const lc::ProfileId& profile) const override {
    return pages.count({window, page, profile});
  }
  std::set<lc::WindowId> windows;
  std::set<std::tuple<lc::WindowId, lc::PageId, lc::ProfileId>> pages;
};

Presentation OriginalPresentation() {
  return {{kHome, kDesk}, {kTemporary, kSavedPage, kFavoritePage}, kSavedPage};
}

std::vector<NativePageJoin> Joins() {
  // Identical native lookup text in distinct Profiles is legal. Within the
  // same Profile each page must reference a different native restore record.
  return {{kSavedPage, NativeRestoreId{"one"}},
          {kFavoritePage, NativeRestoreId{"two"}},
          {kTemporary, NativeRestoreId{"one"}}};
}

Error Stage(Journal& journal,
            const lc::Model& model,
            const CaptureEligibility& eligibility) {
  return journal.StageClose(kUndo, model, CloseReason::kDeliberate, eligibility,
                            kSource, OriginalPresentation(), Joins());
}

void Close(lc::Model& model) {
  // This explicitly supplied synthetic report uses the existing model's safe
  // subset. The journal never authorizes close or changes activity itself.
  for (const auto& [id, page] : model.pages())
    if (page.attachment().window == kSource)
      REQUIRE(model.ReportActivity(id, lc::Activity::kQuiescent) ==
              lc::Status::kOk);
  REQUIRE(model.CloseWindow(kSource) == lc::Status::kOk);
}

std::vector<NewPageIdentity> NewIdentities() {
  return {{kFavoritePage, lc::PageId{"new-favorite"}, std::nullopt},
          {kSavedPage, lc::PageId{"new-saved"}, std::nullopt},
          {kTemporary, lc::PageId{"new-temporary"},
           lc::TemporaryTabId{"new-temporary-tab"}}};
}

ReopenPlan Plan(Journal& journal, const lc::Model& model) {
  auto result =
      journal.BeginReopen(kUndo, kAttempt, model, kTarget, NewIdentities());
  REQUIRE(std::holds_alternative<ReopenPlan>(result));
  return std::get<ReopenPlan>(result);
}

Presentation RestoredPresentation(const ReopenPlan& plan) {
  Presentation observed{{}, {}, plan.selected_page};
  for (const auto& space : plan.spaces)
    observed.spaces.push_back(space.space);
  for (const auto& page : plan.pages)
    observed.pages.push_back(page.page);
  return observed;
}

void ReportSyntheticRestore(lc::Model& model, const ReopenPlan& plan) {
  REQUIRE(model.AddWindow(plan.window) == lc::Status::kOk);
  for (const auto& space : plan.spaces)
    REQUIRE(model.Attach({plan.window, space.space}) == lc::Status::kOk);
  for (const auto& page : plan.pages) {
    if (const auto* temporary = std::get_if<lc::TemporaryTab>(&page.owner)) {
      REQUIRE(model.OpenTemporary(
                  page.page, temporary->id, temporary->attachment,
                  "https://native-restored.example.test/") == lc::Status::kOk);
    } else {
      const auto& saved = std::get<lc::SavedPage>(page.owner);
      REQUIRE(std::holds_alternative<lc::OpenedPage>(
          model.OpenItem(saved.item, saved.attachment, page.page)));
      REQUIRE(model.RecordNavigation(page.page,
                                     "https://native-restored.example.test/") ==
              lc::Status::kOk);
    }
  }
  if (plan.selected_space)
    REQUIRE(model.SelectSpace(plan.window, *plan.selected_space) ==
            lc::Status::kOk);
}

void ConfirmedCloseAndReopen() {
  auto model = Workspace();
  SyntheticEligibility eligibility(model);
  Journal journal;
  REQUIRE(Stage(journal, model, eligibility) == Error::kOk);
  REQUIRE(journal.history().empty());
  REQUIRE(journal.ConfirmClose(kUndo, model) == Error::kCloseIncomplete);
  REQUIRE(model.CloseWindow(kSource) == lc::Status::kWorkNotQuiescent);
  REQUIRE(model.pages().at(kTemporary).activity == lc::Activity::kUnknown);
  Close(model);
  REQUIRE(journal.ConfirmClose(kUndo, model) == Error::kOk);
  REQUIRE(journal.history() == std::vector<UndoId>{kUndo});
  REQUIRE(journal.ConfirmClose(kUndo, model) == Error::kWrongState);
  auto plan = Plan(journal, model);
  REQUIRE(plan.spaces[0].space == kHome);
  REQUIRE(plan.spaces[0].profile == kPersonal);
  REQUIRE(plan.spaces[1].space == kDesk);
  REQUIRE(plan.pages[0].page == lc::PageId{"new-temporary"});
  REQUIRE(plan.pages[0].profile == kPersonal);
  REQUIRE(plan.pages[0].native == NativeRestoreId{"one"});
  REQUIRE(plan.pages[1].page == lc::PageId{"new-saved"});
  REQUIRE(std::get<lc::SavedPage>(plan.pages[1].owner).item == kItem);
  REQUIRE(plan.selected_page == lc::PageId{"new-saved"});
  REQUIRE(plan.window == kTarget);
  REQUIRE(journal.RevalidateReopen(kUndo, kAttempt, model) == Error::kOk);
  REQUIRE(journal.ConfirmReopen(kUndo, kAttempt, model,
                                RestoredPresentation(plan)) ==
          Error::kRestoreIncomplete);
  ReportSyntheticRestore(model, plan);
  auto wrong_order = RestoredPresentation(plan);
  std::swap(wrong_order.pages[0], wrong_order.pages[1]);
  REQUIRE(journal.ConfirmReopen(kUndo, kAttempt, model, wrong_order) ==
          Error::kRestoreIncomplete);
  auto wrong_selection = RestoredPresentation(plan);
  wrong_selection.selected_page.reset();
  REQUIRE(journal.ConfirmReopen(kUndo, kAttempt, model, wrong_selection) ==
          Error::kRestoreIncomplete);
  REQUIRE(!journal.history().empty());
  REQUIRE(journal.ConfirmReopen(kUndo, kAttempt, model,
                                RestoredPresentation(plan)) == Error::kOk);
  REQUIRE(journal.history().empty());
  REQUIRE(journal.CancelReopen(kUndo, kAttempt) == Error::kNotFound);
  REQUIRE(model.items().size() == 2);
  REQUIRE(model.items().at(kItem).saved_destination ==
          "https://saved.example.test/");
  REQUIRE(std::get<lc::SpaceId>(model.items().at(kItem).owner) == kDesk);
  REQUIRE(model.pages().size() == 4);
  REQUIRE(model.pages().at(lc::PageId{"other-page"}).current_navigation ==
          "https://other.example.test/");
  REQUIRE(model.pages().at(lc::PageId{"new-saved"}).current_navigation ==
          "https://native-restored.example.test/");
  REQUIRE(model.pages().at(lc::PageId{"new-saved"}).activity ==
          lc::Activity::kUnknown);
  std::cout
      << "PASS confirmed close/reopen, shared ownership, order, selection\n";
}

void ExclusionAndCancellation() {
  auto model = Workspace();
  SyntheticEligibility eligibility(model);
  Journal journal;
  eligibility.windows.erase(kSource);
  REQUIRE(Stage(journal, model, eligibility) == Error::kExcluded);
  eligibility.windows.insert(kSource);
  eligibility.pages.erase({kSource, kTemporary, kPersonal});
  // Omitting the unverified page from presentation cannot bypass the gate.
  REQUIRE(journal.StageClose(
              kUndo, model, CloseReason::kDeliberate, eligibility, kSource,
              {{kHome, kDesk}, {}, std::nullopt}, {}) == Error::kExcluded);
  eligibility.pages.emplace(kSource, kTemporary, kWork);
  REQUIRE(Stage(journal, model, eligibility) == Error::kExcluded);
  eligibility.pages.emplace(kSource, kTemporary, kPersonal);
  for (const auto reason : {CloseReason::kCrash, CloseReason::kSessionShutdown})
    REQUIRE(journal.StageClose(kUndo, model, reason, eligibility, kSource,
                               OriginalPresentation(),
                               Joins()) == Error::kExcluded);
  REQUIRE(journal.history().empty());
  REQUIRE(Stage(journal, model, eligibility) == Error::kOk);
  REQUIRE(journal.CancelClose(kUndo) == Error::kOk);
  REQUIRE(journal.history().empty());
  REQUIRE(journal.ConfirmClose(kUndo, model) == Error::kNotFound);
  REQUIRE(model.windows().count(kSource));
  REQUIRE(model.pages().size() == 4);

  const lc::WindowId empty{"empty"};
  REQUIRE(model.AddWindow(empty) == lc::Status::kOk);
  REQUIRE(journal.StageClose(UndoId{"empty-undo"}, model,
                             CloseReason::kDeliberate, eligibility, empty,
                             {{}, {}, std::nullopt}, {}) == Error::kExcluded);
  eligibility.windows.insert(empty);
  REQUIRE(journal.StageClose(UndoId{"empty-undo"}, model,
                             CloseReason::kDeliberate, eligibility, empty,
                             {{}, {}, std::nullopt}, {}) == Error::kOk);
  REQUIRE(model.CloseWindow(empty) == lc::Status::kOk);
  REQUIRE(journal.ConfirmClose(UndoId{"empty-undo"}, model) == Error::kOk);
  const auto result =
      journal.BeginReopen(UndoId{"empty-undo"}, AttemptId{"empty-attempt"},
                          model, lc::WindowId{"new-empty"}, {});
  REQUIRE(std::holds_alternative<ReopenPlan>(result));
  const auto plan = std::get<ReopenPlan>(result);
  ReportSyntheticRestore(model, plan);
  REQUIRE(journal.ConfirmReopen(UndoId{"empty-undo"},
                                AttemptId{"empty-attempt"}, model,
                                RestoredPresentation(plan)) == Error::kOk);
  std::cout
      << "PASS synthetic eligibility, non-deliberate close, cancellation\n";
}

void InputAndSplitBoundaries() {
  auto model = Workspace();
  SyntheticEligibility eligibility(model);
  Journal journal;
  auto missing_space = OriginalPresentation();
  missing_space.spaces.pop_back();
  REQUIRE(journal.StageClose(kUndo, model, CloseReason::kDeliberate,
                             eligibility, kSource, missing_space,
                             Joins()) == Error::kInvalidPresentation);
  auto missing_page = OriginalPresentation();
  missing_page.pages.pop_back();
  auto missing_join = Joins();
  missing_join.erase(missing_join.begin() + 1);
  REQUIRE(journal.StageClose(kUndo, model, CloseReason::kDeliberate,
                             eligibility, kSource, missing_page,
                             missing_join) == Error::kInvalidPresentation);
  auto duplicate_page = OriginalPresentation();
  duplicate_page.pages.push_back(kSavedPage);
  REQUIRE(journal.StageClose(kUndo, model, CloseReason::kDeliberate,
                             eligibility, kSource, duplicate_page,
                             Joins()) == Error::kInvalidPresentation);
  auto wrong_selection = OriginalPresentation();
  wrong_selection.selected_page = kTemporary;
  REQUIRE(journal.StageClose(kUndo, model, CloseReason::kDeliberate,
                             eligibility, kSource, wrong_selection,
                             Joins()) == Error::kInvalidPresentation);
  REQUIRE(journal.StageClose(kUndo, model, CloseReason::kDeliberate,
                             eligibility, kSource, OriginalPresentation(),
                             {}) == Error::kNativeRestoreRequired);
  auto duplicate_native = Joins();
  duplicate_native[1].native = duplicate_native[0].native;
  REQUIRE(journal.StageClose(kUndo, model, CloseReason::kDeliberate,
                             eligibility, kSource, OriginalPresentation(),
                             duplicate_native) ==
          Error::kNativeRestoreRequired);
  REQUIRE(model.SetSplit(lc::SplitId{"split"}, {kSource, kDesk},
                         {kSavedPage, kFavoritePage}) == lc::Status::kOk);
  REQUIRE(Stage(journal, model, eligibility) ==
          Error::kSplitIntegrationPending);
  REQUIRE(journal.history().empty());
  REQUIRE(model.splits().size() == 1);
  REQUIRE(model.RemoveSplit(lc::SplitId{"split"}) == lc::Status::kOk);
  REQUIRE(Stage(journal, model, eligibility) == Error::kOk);
  REQUIRE(Stage(journal, model, eligibility) == Error::kDuplicateId);
  std::cout << "PASS complete membership, joins, selection and split gate\n";
}

void StaleRestoreAndSharedItemConflict() {
  auto model = Workspace();
  SyntheticEligibility eligibility(model);
  Journal journal;
  REQUIRE(Stage(journal, model, eligibility) == Error::kOk);
  Close(model);
  REQUIRE(journal.ConfirmClose(kUndo, model) == Error::kOk);
  auto reused_identity = NewIdentities();
  reused_identity[0].new_page = kTemporary;
  REQUIRE(std::get<Error>(journal.BeginReopen(kUndo, kAttempt, model, kTarget,
                                              reused_identity)) ==
          Error::kIdentityConflict);
  auto missing_temporary = NewIdentities();
  missing_temporary[2].new_temporary.reset();
  REQUIRE(std::get<Error>(journal.BeginReopen(kUndo, kAttempt, model, kTarget,
                                              missing_temporary)) ==
          Error::kIdentityConflict);
  auto plan = Plan(journal, model);
  REQUIRE(journal.CancelReopen(kUndo, kLater) == Error::kWrongState);
  REQUIRE(std::get<Error>(journal.BeginReopen(kUndo, kLater, model, kTarget,
                                              NewIdentities())) ==
          Error::kWrongState);
  REQUIRE(std::holds_alternative<lc::OpenedPage>(
      model.OpenItem(kItem, {kOther, kDesk}, lc::PageId{"elsewhere"})));
  REQUIRE(journal.RevalidateReopen(kUndo, kAttempt, model) ==
          Error::kSavedPageAlreadyLive);
  REQUIRE(model.pages().at(lc::PageId{"elsewhere"}).attachment().window ==
          kOther);
  REQUIRE(journal.CancelReopen(kUndo, kAttempt) == Error::kOk);
  REQUIRE(std::get<Error>(journal.BeginReopen(kUndo, kLater, model, kTarget,
                                              NewIdentities())) ==
          Error::kSavedPageAlreadyLive);
  REQUIRE(model.ReportActivity(lc::PageId{"elsewhere"},
                               lc::Activity::kQuiescent) == lc::Status::kOk);
  REQUIRE(model.ClosePage(lc::PageId{"elsewhere"}) == lc::Status::kOk);
  REQUIRE(model.RemoveItem(kItem) == lc::Status::kOk);
  REQUIRE(std::get<Error>(journal.BeginReopen(kUndo, kLater, model, kTarget,
                                              NewIdentities())) ==
          Error::kOwnerChanged);
  REQUIRE(!model.windows().count(kTarget));
  REQUIRE(journal.history() == std::vector<UndoId>{kUndo});
  std::cout
      << "PASS stale attempts, fresh identities and no saved-page stealing\n";
}

void PartialRestoreRetainsHistory() {
  auto model = Workspace();
  SyntheticEligibility eligibility(model);
  Journal journal;
  REQUIRE(Stage(journal, model, eligibility) == Error::kOk);
  Close(model);
  REQUIRE(journal.ConfirmClose(kUndo, model) == Error::kOk);
  auto plan = Plan(journal, model);
  ReportSyntheticRestore(model, plan);
  REQUIRE(model.SelectSpace(kTarget, kHome) == lc::Status::kOk);
  REQUIRE(journal.ConfirmReopen(kUndo, kAttempt, model,
                                RestoredPresentation(plan)) ==
          Error::kRestoreIncomplete);
  REQUIRE(model.SelectSpace(kTarget, kDesk) == lc::Status::kOk);
  REQUIRE(model.SetSplit(lc::SplitId{"unexpected-split"}, {kTarget, kDesk},
                         {lc::PageId{"new-saved"},
                          lc::PageId{"new-favorite"}}) == lc::Status::kOk);
  REQUIRE(journal.ConfirmReopen(kUndo, kAttempt, model,
                                RestoredPresentation(plan)) ==
          Error::kSplitIntegrationPending);
  REQUIRE(model.RemoveSplit(lc::SplitId{"unexpected-split"}) ==
          lc::Status::kOk);
  REQUIRE(model.TransferSavedPage(lc::PageId{"new-saved"}, {kOther, kDesk}) ==
          lc::Status::kOk);
  REQUIRE(journal.ConfirmReopen(kUndo, kAttempt, model,
                                RestoredPresentation(plan)) ==
          Error::kRestoreIncomplete);
  REQUIRE(!journal.history().empty());
  REQUIRE(model.TransferSavedPage(lc::PageId{"new-saved"}, {kTarget, kDesk}) ==
          lc::Status::kOk);
  REQUIRE(model.OpenTemporary(
              lc::PageId{"unexpected"}, lc::TemporaryTabId{"unexpected-tab"},
              {kTarget, kDesk},
              "https://unexpected.example.test/") == lc::Status::kOk);
  REQUIRE(journal.ConfirmReopen(kUndo, kAttempt, model,
                                RestoredPresentation(plan)) ==
          Error::kRestoreIncomplete);
  REQUIRE(journal.CancelReopen(kUndo, kAttempt) == Error::kOk);
  REQUIRE(model.windows().count(kTarget));  // No fabricated native rollback.
  REQUIRE(journal.history() == std::vector<UndoId>{kUndo});
  std::cout << "PASS partial/extra restore retains history without rollback\n";
}

void HistoryAndProfileChange() {
  auto model = Workspace();
  SyntheticEligibility eligibility(model);
  Journal journal;
  REQUIRE(Stage(journal, model, eligibility) == Error::kOk);
  Close(model);
  REQUIRE(journal.ConfirmClose(kUndo, model) == Error::kOk);
  const UndoId other_undo{"other-undo"};
  const lc::PageId other_page{"other-page"};
  REQUIRE(journal.StageClose(
              other_undo, model, CloseReason::kDeliberate, eligibility, kOther,
              {{kDesk}, {other_page}, std::nullopt},
              {{other_page, NativeRestoreId{"other-native"}}}) == Error::kOk);
  REQUIRE(model.ReportActivity(other_page, lc::Activity::kQuiescent) ==
          lc::Status::kOk);
  REQUIRE(model.CloseWindow(kOther) == lc::Status::kOk);
  REQUIRE(journal.ConfirmClose(other_undo, model) == Error::kOk);
  const std::vector<UndoId> both{kUndo, other_undo};
  REQUIRE(journal.history() == both);
  auto plan = Plan(journal, model);
  REQUIRE(model.ReassignEmptySpace(kHome, kWork) == lc::Status::kOk);
  REQUIRE(journal.RevalidateReopen(kUndo, kAttempt, model) ==
          Error::kOwnerChanged);
  REQUIRE(journal.CancelReopen(kUndo, kAttempt) == Error::kOk);
  REQUIRE(model.ReassignEmptySpace(kHome, kPersonal) == lc::Status::kOk);
  auto result =
      journal.BeginReopen(kUndo, kLater, model, kTarget, NewIdentities());
  REQUIRE(std::holds_alternative<ReopenPlan>(result));
  plan = std::get<ReopenPlan>(result);
  REQUIRE(journal.ConfirmReopen(kUndo, kAttempt, model,
                                RestoredPresentation(plan)) ==
          Error::kWrongState);
  ReportSyntheticRestore(model, plan);
  REQUIRE(journal.ConfirmReopen(kUndo, kLater, model,
                                RestoredPresentation(plan)) == Error::kOk);
  REQUIRE(journal.history() == std::vector<UndoId>{other_undo});
  std::cout
      << "PASS Profile revalidation, stale completion and targeted history\n";
}

void ChangedCloseMembership() {
  auto model = Workspace();
  SyntheticEligibility eligibility(model);
  Journal journal;
  REQUIRE(Stage(journal, model, eligibility) == Error::kOk);
  REQUIRE(model.TransferSavedPage(kSavedPage, {kOther, kDesk}) ==
          lc::Status::kOk);
  Close(model);
  // The actual window closed, but the captured saved page survived in another
  // window. The coordinator must cancel the stale capture, not invent a close.
  REQUIRE(journal.ConfirmClose(kUndo, model) == Error::kCloseIncomplete);
  REQUIRE(journal.history().empty());
  REQUIRE(journal.CancelClose(kUndo) == Error::kOk);
  REQUIRE(model.pages().at(kSavedPage).attachment().window == kOther);
  REQUIRE(model.pages().at(kSavedPage).activity == lc::Activity::kUnknown);
  std::cout << "PASS changed close membership cannot create stale history\n";
}

}  // namespace
}  // namespace arcified::window_restore

int main() {
  using namespace arcified::window_restore;
  ConfirmedCloseAndReopen();
  ExclusionAndCancellation();
  InputAndSplitBoundaries();
  StaleRestoreAndSharedItemConflict();
  PartialRestoreRetainsHistory();
  HistoryAndProfileChange();
  ChangedCloseMembership();
}
