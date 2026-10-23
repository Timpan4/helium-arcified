#include "persistence.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace arcified::splits {
namespace {

using lifecycle::Activity;
using lifecycle::Attachment;
using lifecycle::Model;
using lifecycle::PageId;
using lifecycle::ProfileId;
using lifecycle::SpaceId;
using lifecycle::SplitId;
using lifecycle::Status;
using lifecycle::TemporaryTabId;
using lifecycle::WindowId;

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const Attachment kAttachment{WindowId{"window"}, SpaceId{"space"}};
const Attachment kElsewhere{WindowId{"elsewhere"}, SpaceId{"space"}};
const Attachment kOtherProfile{WindowId{"window"}, SpaceId{"other-space"}};
const SplitId kSplit{"restored-split"};
const PageId kFirst{"restored-first"};
const PageId kSecond{"restored-second"};
const PageId kThird{"restored-third"};
const PageId kOutside{"outside-split"};
const PageId kForeign{"foreign-window"};
const PageId kOther{"other-profile-page"};
const PageId kOldFirst{"saved-first"};
const PageId kOldSecond{"saved-second"};

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(ProfileId{"profile"}) == Status::kOk);
  REQUIRE(model.AddProfile(ProfileId{"other-profile"}) == Status::kOk);
  REQUIRE(model.AddSpace(kAttachment.space, ProfileId{"profile"}) ==
          Status::kOk);
  REQUIRE(model.AddSpace(kOtherProfile.space, ProfileId{"other-profile"}) ==
          Status::kOk);
  REQUIRE(model.AddWindow(kAttachment.window) == Status::kOk);
  REQUIRE(model.AddWindow(kElsewhere.window) == Status::kOk);
  REQUIRE(model.Attach(kAttachment) == Status::kOk);
  REQUIRE(model.Attach(kElsewhere) == Status::kOk);
  REQUIRE(model.Attach(kOtherProfile) == Status::kOk);
  for (const auto& page : {kFirst, kSecond, kThird, kOutside}) {
    REQUIRE(model.OpenTemporary(page, TemporaryTabId{page.value}, kAttachment,
                                "https://example.test/" + page.value) ==
            Status::kOk);
  }
  REQUIRE(model.OpenTemporary(kForeign, TemporaryTabId{"foreign-tab"},
                              kElsewhere,
                              "https://elsewhere.example.test") == Status::kOk);
  REQUIRE(model.OpenTemporary(kOther, TemporaryTabId{"other-tab"},
                              kOtherProfile,
                              "https://other.example.test") == Status::kOk);
  REQUIRE(model.ReportActivity(kFirst, Activity::kProtected) == Status::kOk);
  REQUIRE(model.SetSplit(kSplit, kAttachment, {kThird, kFirst, kSecond}) ==
          Status::kOk);
  REQUIRE(model.SelectSpace(kAttachment.window, kAttachment.space) ==
          Status::kOk);
  return model;
}

RestoredLayout Reconcile(const Model& model,
                         const LayoutMetadata& saved,
                         const RestoredPageMap& mapping) {
  auto result = ReconcileRestoredLayout(model, kSplit, saved, mapping);
  REQUIRE(std::holds_alternative<RestoredLayout>(result));
  return std::get<RestoredLayout>(std::move(result));
}

bool HasIssue(const RestoredLayout& result,
              const PageId& source,
              std::optional<PageId> target,
              RestoreIssueReason reason) {
  for (const auto& issue : result.issues)
    if (issue.source == source && issue.target == target &&
        issue.reason == reason)
      return true;
  return false;
}

void RestoredWeightsFollowCanonicalIdentityAndOrder() {
  const auto model = Workspace();
  for (const auto orientation :
       {Orientation::kHorizontal, Orientation::kVertical}) {
    const auto recovered =
        Reconcile(model, {orientation, {{kOldFirst, 8.0}, {kOldSecond, 2.0}}},
                  {{kOldFirst, kFirst}, {kOldSecond, kSecond}});
    REQUIRE(recovered.issues.empty());
    REQUIRE(recovered.metadata.orientation == orientation);
    REQUIRE(recovered.metadata.weights.size() == 3);
    REQUIRE(recovered.metadata.weights.at(kFirst) == 8.0);
    REQUIRE(recovered.metadata.weights.at(kSecond) == 2.0);
    REQUIRE(recovered.metadata.weights.at(kThird) == 1.0);
    const auto projection = ResolveLayout(model, kSplit, recovered.metadata);
    REQUIRE(std::holds_alternative<LayoutView>(projection));
    const auto& view = std::get<LayoutView>(projection);
    REQUIRE(view.panes.size() == 3);
    REQUIRE(view.panes[0].page == kThird);
    REQUIRE(view.panes[1].page == kFirst);
    REQUIRE(view.panes[2].page == kSecond);
    REQUIRE(!view.focus_target);
  }
}

void MissingAndForeignReferencesLeaveSurvivorsIntact() {
  const auto model = Workspace();
  const PageId missing{"missing-mapping"};
  const PageId absent{"not-restored"};
  const PageId old_absent{"saved-absent"};
  const auto result = Reconcile(model,
                                {Orientation::kVertical,
                                 {{kOldFirst, 7.0},
                                  {missing, 9.0},
                                  {old_absent, 3.0},
                                  {kOutside, 5.0},
                                  {kForeign, 6.0},
                                  {kOther, 8.0}}},
                                {{kOldFirst, kFirst},
                                 {old_absent, absent},
                                 {kOutside, kOutside},
                                 {kForeign, kForeign},
                                 {kOther, kOther}});
  REQUIRE(result.issues.size() == 5);
  REQUIRE(HasIssue(result, missing, std::nullopt,
                   RestoreIssueReason::kMissingMapping));
  REQUIRE(HasIssue(result, old_absent, absent,
                   RestoreIssueReason::kPageNotRestored));
  REQUIRE(HasIssue(result, kOutside, kOutside,
                   RestoreIssueReason::kPageNotInSplit));
  REQUIRE(HasIssue(result, kForeign, kForeign,
                   RestoreIssueReason::kAttachmentMismatch));
  REQUIRE(HasIssue(result, kOther, kOther,
                   RestoreIssueReason::kAttachmentMismatch));
  REQUIRE(result.metadata.weights.size() == 3);
  REQUIRE(result.metadata.weights.at(kFirst) == 7.0);
  REQUIRE(result.metadata.weights.at(kSecond) == 1.0);
  REQUIRE(result.metadata.weights.at(kThird) == 1.0);
  REQUIRE(model.pages().size() == 6);
  REQUIRE(model.pages().at(kFirst).profile == ProfileId{"profile"});
  REQUIRE(model.pages().at(kFirst).activity == Activity::kProtected);
  REQUIRE(model.pages().at(kFirst).current_navigation ==
          "https://example.test/restored-first");
  REQUIRE(model.pages().at(kForeign).attachment() == kElsewhere);
  REQUIRE(model.pages().at(kOther).attachment() == kOtherProfile);
  REQUIRE(model.splits().at(kSplit).pages ==
          (std::vector<PageId>{kThird, kFirst, kSecond}));
  REQUIRE(model.windows().at(kAttachment.window).selected_space ==
          kAttachment.space);
}

void AmbiguousMappingDefaultsOnlyItsTarget() {
  const auto model = Workspace();
  const PageId alias{"saved-alias"};
  const auto result =
      Reconcile(model,
                {Orientation::kHorizontal,
                 {{kOldFirst, 4.0}, {alias, 9.0}, {kOldSecond, 6.0}}},
                {{kOldFirst, kFirst}, {alias, kFirst}, {kOldSecond, kSecond}});
  REQUIRE(result.issues.size() == 2);
  REQUIRE(HasIssue(result, kOldFirst, kFirst,
                   RestoreIssueReason::kAmbiguousMapping));
  REQUIRE(
      HasIssue(result, alias, kFirst, RestoreIssueReason::kAmbiguousMapping));
  REQUIRE(result.metadata.weights.at(kFirst) == 1.0);
  REQUIRE(result.metadata.weights.at(kSecond) == 6.0);
  REQUIRE(result.metadata.weights.at(kThird) == 1.0);
}

void AmbiguityPrecedesWeightValidity() {
  const auto model = Workspace();
  const PageId alias{"saved-invalid-alias"};
  const auto result =
      Reconcile(model,
                {Orientation::kHorizontal,
                 {{kOldFirst, 4.0},
                  {alias, std::numeric_limits<double>::quiet_NaN()},
                  {kOldSecond, 6.0}}},
                {{kOldFirst, kFirst}, {alias, kFirst}, {kOldSecond, kSecond}});
  REQUIRE(result.issues.size() == 2);
  REQUIRE(HasIssue(result, kOldFirst, kFirst,
                   RestoreIssueReason::kAmbiguousMapping));
  REQUIRE(
      HasIssue(result, alias, kFirst, RestoreIssueReason::kAmbiguousMapping));
  REQUIRE(result.metadata.weights.size() == 3);
  REQUIRE(result.metadata.weights.at(kFirst) == 1.0);
  REQUIRE(result.metadata.weights.at(kSecond) == 6.0);
  REQUIRE(result.metadata.weights.at(kThird) == 1.0);
}

void InvalidWeightsRecoverWithoutNormalization() {
  const auto model = Workspace();
  for (const auto invalid : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                             -std::numeric_limits<double>::infinity(),
                             std::numeric_limits<double>::quiet_NaN()}) {
    const auto result = Reconcile(
        model,
        {Orientation::kHorizontal, {{kOldFirst, invalid}, {kOldSecond, 11.0}}},
        {{kOldFirst, kFirst}, {kOldSecond, kSecond}});
    REQUIRE(result.issues.size() == 1);
    REQUIRE(HasIssue(result, kOldFirst, kFirst,
                     RestoreIssueReason::kInvalidWeight));
    REQUIRE(result.metadata.weights.at(kFirst) == 1.0);
    REQUIRE(result.metadata.weights.at(kSecond) == 11.0);
  }
  const auto result =
      Reconcile(model,
                {Orientation::kVertical,
                 {{kOldFirst, std::numeric_limits<double>::max()},
                  {kOldSecond, std::numeric_limits<double>::denorm_min()}}},
                {{kOldFirst, kFirst}, {kOldSecond, kSecond}});
  REQUIRE(result.issues.empty());
  REQUIRE(result.metadata.weights.at(kFirst) ==
          std::numeric_limits<double>::max());
  REQUIRE(result.metadata.weights.at(kSecond) ==
          std::numeric_limits<double>::denorm_min());
}

void EmptyIdsAndAbsentMappingsNeverInferIdentity() {
  const auto model = Workspace();
  const PageId empty{""};
  const auto result =
      Reconcile(model,
                {Orientation::kHorizontal,
                 {{empty, 3.0}, {kFirst, 5.0}, {kOldSecond, 8.0}}},
                {{empty, kFirst}, {kOldSecond, empty}});
  REQUIRE(result.issues.size() == 3);
  REQUIRE(
      HasIssue(result, empty, kFirst, RestoreIssueReason::kInvalidReference));
  REQUIRE(HasIssue(result, kOldSecond, empty,
                   RestoreIssueReason::kInvalidReference));
  REQUIRE(HasIssue(result, kFirst, std::nullopt,
                   RestoreIssueReason::kMissingMapping));
  for (const auto& [page, weight] : result.metadata.weights)
    REQUIRE(weight == 1.0);
}

void MissingSplitAndInvalidOrientationFailWithoutMutation() {
  const auto model = Workspace();
  REQUIRE(std::get<LayoutError>(
              ReconcileRestoredLayout(model, SplitId{"missing"}, {}, {})) ==
          LayoutError::kSplitNotFound);
  REQUIRE(std::get<LayoutError>(ReconcileRestoredLayout(
              model, kSplit, {static_cast<Orientation>(-1), {}}, {})) ==
          LayoutError::kInvalidOrientation);
  REQUIRE(model.splits().size() == 1);
  REQUIRE(model.pages().size() == 6);
}

void ClosureAndTransferOnlyReconcileRemainingMembers() {
  auto model = Workspace();
  const LayoutMetadata saved{Orientation::kVertical,
                             {{kOldFirst, 4.0}, {kOldSecond, 8.0}}};
  const RestoredPageMap mapping{{kOldFirst, kFirst}, {kOldSecond, kSecond}};
  REQUIRE(model.ReportActivity(kSecond, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kSecond) == Status::kOk);
  REQUIRE(model.Keep(kFirst, lifecycle::ItemId{"kept"}) == Status::kOk);
  REQUIRE(model.TransferSavedPage(kFirst, kElsewhere) == Status::kOk);
  const auto result = Reconcile(model, saved, mapping);
  REQUIRE(result.metadata.weights.size() == 1);
  REQUIRE(result.metadata.weights.at(kThird) == 1.0);
  REQUIRE(HasIssue(result, kOldFirst, kFirst,
                   RestoreIssueReason::kAttachmentMismatch));
  REQUIRE(HasIssue(result, kOldSecond, kSecond,
                   RestoreIssueReason::kPageNotRestored));
  REQUIRE(model.pages().at(kFirst).attachment() == kElsewhere);
  REQUIRE(model.pages().at(kFirst).activity == Activity::kProtected);
  REQUIRE(!model.pages().count(kSecond));
}

}  // namespace
}  // namespace arcified::splits

int main() {
  using namespace arcified::splits;
  RestoredWeightsFollowCanonicalIdentityAndOrder();
  MissingAndForeignReferencesLeaveSurvivorsIntact();
  AmbiguousMappingDefaultsOnlyItsTarget();
  AmbiguityPrecedesWeightValidity();
  InvalidWeightsRecoverWithoutNormalization();
  EmptyIdsAndAbsentMappingsNeverInferIdentity();
  MissingSplitAndInvalidOrientationFailWithoutMutation();
  ClosureAndTransferOnlyReconcileRemainingMembers();
  std::cout << "split persistence reconciliation tests passed\n";
}
