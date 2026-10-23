#include "layout.h"

#include <cstdlib>
#include <iostream>
#include <limits>

namespace arcified::splits {
namespace {

using namespace lifecycle;

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const ProfileId kProfile{"synthetic-profile"};
const SpaceId kSpace{"synthetic-space"};
const WindowId kWindow{"first-window"};
const WindowId kOtherWindow{"second-window"};
const Attachment kAttachment{kWindow, kSpace};
const Attachment kOtherAttachment{kOtherWindow, kSpace};
const PageId kFirst{"first-page"};
const PageId kSecond{"second-page"};
const PageId kThird{"third-page"};
const PageId kForeign{"foreign-page"};
const SplitId kSplit{"split"};
const SplitId kMissing{"missing-split"};

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kProfile) == Status::kOk);
  REQUIRE(model.AddSpace(kSpace, kProfile) == Status::kOk);
  REQUIRE(model.AddWindow(kWindow) == Status::kOk);
  REQUIRE(model.AddWindow(kOtherWindow) == Status::kOk);
  REQUIRE(model.Attach(kAttachment) == Status::kOk);
  REQUIRE(model.Attach(kOtherAttachment) == Status::kOk);
  for (const auto& page : {kFirst, kSecond, kThird}) {
    REQUIRE(model.OpenTemporary(
                page, TemporaryTabId{page.value}, kAttachment,
                "https://synthetic.example.test/" + page.value) == Status::kOk);
  }
  REQUIRE(model.OpenTemporary(
              kForeign, TemporaryTabId{"foreign-tab"}, kOtherAttachment,
              "https://synthetic.example.test/foreign") == Status::kOk);
  REQUIRE(model.SetSplit(kSplit, kAttachment, {kFirst, kSecond, kThird}) ==
          Status::kOk);
  return model;
}

template <typename T>
void ExpectError(const std::variant<LayoutError, T>& result,
                 LayoutError error) {
  REQUIRE(std::holds_alternative<LayoutError>(result));
  REQUIRE(std::get<LayoutError>(result) == error);
}

void ProjectionUsesCanonicalOrderAndExternalFocus() {
  const auto model = Workspace();
  LayoutMetadata metadata;
  metadata.orientation = Orientation::kVertical;
  metadata.weights = {{kFirst, 3.0}, {kThird, 2.0}, {kForeign, -1.0}};
  const auto result = ResolveLayout(model, kSplit, metadata, kSecond);
  REQUIRE(std::holds_alternative<LayoutView>(result));
  const auto& view = std::get<LayoutView>(result);
  REQUIRE(view.split == kSplit);
  REQUIRE(view.attachment == kAttachment);
  REQUIRE(view.orientation == Orientation::kVertical);
  REQUIRE(view.panes.size() == 3);
  REQUIRE(view.panes[0].page == kFirst);
  REQUIRE(view.panes[0].weight == 3.0);
  REQUIRE(view.panes[1].page == kSecond);
  REQUIRE(view.panes[1].weight == 1.0);
  REQUIRE(view.panes[2].page == kThird);
  REQUIRE(view.panes[2].weight == 2.0);
  REQUIRE(view.focus_target == kSecond);
  REQUIRE(!std::get<LayoutView>(ResolveLayout(model, kSplit, {})).focus_target);
  ExpectError(ResolveLayout(model, kSplit, {}, kForeign),
              LayoutError::kPageNotInSplit);
  const auto reconciled =
      std::get<LayoutMetadata>(ReconcileLayout(model, kSplit, metadata));
  REQUIRE(reconciled.weights.size() == 3);
  REQUIRE(reconciled.weights.count(kForeign) == 0);
  REQUIRE(metadata.weights.at(kForeign) == -1.0);
  REQUIRE(model.pages().size() == 4);
  REQUIRE(model.splits().at(kSplit).pages.front() == kFirst);
}

void RelativeWeightsHaveNoNormalizationOrSizeLimit() {
  const auto model = Workspace();
  LayoutMetadata metadata;
  metadata.weights = {{kFirst, std::numeric_limits<double>::max()},
                      {kSecond, std::numeric_limits<double>::denorm_min()}};
  const auto view =
      std::get<LayoutView>(ResolveLayout(model, kSplit, metadata));
  REQUIRE(view.panes[0].weight == std::numeric_limits<double>::max());
  REQUIRE(view.panes[1].weight == std::numeric_limits<double>::denorm_min());
  REQUIRE(view.panes[2].weight == 1.0);

  for (const double invalid :
       {0.0, -1.0, std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN()}) {
    metadata.weights[kFirst] = invalid;
    ExpectError(ReconcileLayout(model, kSplit, metadata),
                LayoutError::kInvalidWeight);
  }
  metadata.weights.clear();
  metadata.orientation = static_cast<Orientation>(-1);
  ExpectError(ReconcileLayout(model, kSplit, metadata),
              LayoutError::kInvalidOrientation);
}

void ReorderRetainsPageIdentityAndRelativeSizes() {
  auto model = Workspace();
  REQUIRE(model.RecordNavigation(
              kFirst, "https://synthetic.example.test/edited") == Status::kOk);
  REQUIRE(model.ReportActivity(kFirst, Activity::kProtected) == Status::kOk);
  const auto original_order = model.splits().at(kSplit).pages;
  const auto navigation = model.pages().at(kFirst).current_navigation;
  LayoutMetadata metadata{Orientation::kHorizontal,
                          {{kFirst, 3.0}, {kSecond, 2.0}, {kThird, 1.0}}};
  const auto plan = std::get<SplitReorder>(
      PlanReorder(model, kSplit, {kThird, kFirst, kSecond}, kFirst));
  REQUIRE(model.splits().at(kSplit).pages == original_order);
  REQUIRE(plan.split == kSplit);
  REQUIRE(plan.attachment == kAttachment);
  REQUIRE(plan.focus_target == kFirst);
  REQUIRE(model.SetSplit(plan.split, plan.attachment, plan.pages) ==
          Status::kOk);
  const auto view = std::get<LayoutView>(
      ResolveLayout(model, kSplit, metadata, plan.focus_target));
  REQUIRE(view.panes[0].page == kThird);
  REQUIRE(view.panes[0].weight == 1.0);
  REQUIRE(view.panes[1].page == kFirst);
  REQUIRE(view.panes[1].weight == 3.0);
  REQUIRE(view.panes[2].page == kSecond);
  REQUIRE(view.panes[2].weight == 2.0);
  REQUIRE(view.focus_target == kFirst);
  REQUIRE(model.pages().at(kFirst).current_navigation == navigation);
  REQUIRE(model.pages().at(kFirst).profile == kProfile);
  REQUIRE(model.pages().at(kFirst).activity == Activity::kProtected);

  for (const std::vector<PageId>& invalid :
       {std::vector<PageId>{kFirst, kSecond},
        std::vector<PageId>{kFirst, kSecond, kSecond},
        std::vector<PageId>{kFirst, kSecond, kForeign},
        std::vector<PageId>{kFirst, kSecond, kThird, kForeign}}) {
    ExpectError(PlanReorder(model, kSplit, invalid),
                LayoutError::kInvalidOrder);
  }
  ExpectError(PlanReorder(model, kSplit, original_order, kForeign),
              LayoutError::kPageNotInSplit);
  REQUIRE(model.splits().at(kSplit).pages == plan.pages);
}

void CloseAndTransferReconcileOnlyObservedMembership() {
  auto model = Workspace();
  LayoutMetadata metadata{Orientation::kVertical,
                          {{kFirst, 3.0}, {kSecond, 2.0}, {kThird, 4.0}}};
  REQUIRE(model.ClosePage(kFirst) == Status::kWorkNotQuiescent);
  REQUIRE(std::get<LayoutView>(ResolveLayout(model, kSplit, metadata))
              .panes.size() == 3);
  REQUIRE(model.ReportActivity(kFirst, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kFirst) == Status::kOk);
  ExpectError(ResolveLayout(model, kSplit, metadata, kFirst),
              LayoutError::kPageNotInSplit);
  // The external focus owner explicitly supplies the next target.
  auto view =
      std::get<LayoutView>(ResolveLayout(model, kSplit, metadata, kSecond));
  REQUIRE(view.panes.size() == 2);
  REQUIRE(view.panes[0].page == kSecond);
  REQUIRE(view.panes[0].weight == 2.0);

  REQUIRE(model.Keep(kSecond, ItemId{"saved-second"}) == Status::kOk);
  const auto navigation = model.pages().at(kSecond).current_navigation;
  REQUIRE(model.TransferSavedPage(kSecond, kOtherAttachment) == Status::kOk);
  const auto reconciled =
      std::get<LayoutMetadata>(ReconcileLayout(model, kSplit, metadata));
  REQUIRE(reconciled.orientation == Orientation::kVertical);
  REQUIRE(reconciled.weights.size() == 1);
  REQUIRE(reconciled.weights.at(kThird) == 4.0);
  view = std::get<LayoutView>(ResolveLayout(model, kSplit, reconciled, kThird));
  REQUIRE(view.panes.size() == 1);
  REQUIRE(view.panes[0].page == kThird);
  REQUIRE(model.pages().at(kSecond).attachment() == kOtherAttachment);
  REQUIRE(model.pages().at(kSecond).current_navigation == navigation);
  REQUIRE(model.pages().at(kSecond).profile == kProfile);

  REQUIRE(model.ReportActivity(kThird, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kThird) == Status::kOk);
  ExpectError(ReconcileLayout(model, kSplit, metadata),
              LayoutError::kSplitNotFound);
}

void UnsplitProposalPreservesEveryPage() {
  auto model = Workspace();
  const auto pages_before = model.pages();
  ExpectError(PlanUnsplit(model, kSplit, kForeign),
              LayoutError::kPageNotInSplit);
  const auto plan = std::get<UnsplitPlan>(PlanUnsplit(model, kSplit, kSecond));
  REQUIRE(plan.split == kSplit);
  REQUIRE(plan.focus_target == kSecond);
  REQUIRE(model.splits().count(kSplit) == 1);
  REQUIRE(model.RemoveSplit(plan.split) == Status::kOk);
  REQUIRE(model.pages().size() == pages_before.size());
  for (const auto& [page, before] : pages_before) {
    const auto& after = model.pages().at(page);
    REQUIRE(after.profile == before.profile);
    REQUIRE(after.attachment() == before.attachment());
    REQUIRE(after.current_navigation == before.current_navigation);
    REQUIRE(after.activity == before.activity);
  }
  ExpectError(ResolveLayout(model, kSplit, {}), LayoutError::kSplitNotFound);
  ExpectError(PlanReorder(model, kMissing, {}), LayoutError::kSplitNotFound);
  ExpectError(PlanUnsplit(model, kMissing, kSecond),
              LayoutError::kSplitNotFound);
}

}  // namespace
}  // namespace arcified::splits

int main() {
  using namespace arcified::splits;
  ProjectionUsesCanonicalOrderAndExternalFocus();
  RelativeWeightsHaveNoNormalizationOrSizeLimit();
  ReorderRetainsPageIdentityAndRelativeSizes();
  CloseAndTransferReconcileOnlyObservedMembership();
  UnsplitProposalPreservesEveryPage();
  std::cout << "split layout checks passed\n";
}
