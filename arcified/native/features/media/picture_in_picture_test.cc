#include "picture_in_picture.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace arcified::media {
namespace {

using lifecycle::Activity;
using lifecycle::Attachment;
using lifecycle::ItemId;
using lifecycle::Model;
using lifecycle::PageId;
using lifecycle::ProfileId;
using lifecycle::SpaceId;
using lifecycle::SplitId;
using lifecycle::Status;
using lifecycle::TemporaryTabId;
using lifecycle::WindowId;
using Action = PictureInPictureAction;
using Refusal = PictureInPictureRefusal;

static_assert(lifecycle::kContractVersion == 3);
static_assert(!std::is_convertible_v<PictureInPictureSessionId, PageId>);

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const ProfileId kWork{"work"};
const ProfileId kHome{"home"};
const SpaceId kVideoSpace{"video-space"};
const SpaceId kOtherSpace{"other-work-space"};
const SpaceId kHomeSpace{"home-space"};
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const Attachment kOriginal{kFirst, kVideoSpace};
const Attachment kTransferred{kSecond, kOtherSpace};
const ItemId kVideoItem{"video-shortcut"};
const PageId kVideo{"video-page"};
const PageId kUnrelated{"unrelated-page"};
const SplitId kLayout{"layout"};
const PictureInPictureSessionId kSession{"native-session"};
const std::string kDestination = "https://video.example.test/watch";

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kHome) == Status::kOk);
  REQUIRE(model.AddSpace(kVideoSpace, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kOtherSpace, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kHomeSpace, kHome) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  REQUIRE(model.Attach(kOriginal) == Status::kOk);
  REQUIRE(model.Attach({kFirst, kOtherSpace}) == Status::kOk);
  REQUIRE(model.Attach({kFirst, kHomeSpace}) == Status::kOk);
  REQUIRE(model.Attach(kTransferred) == Status::kOk);
  REQUIRE(model.Attach({kSecond, kHomeSpace}) == Status::kOk);
  // A Profile-owned shortcut can retain its page across same-Profile Spaces.
  REQUIRE(model.AddItem(kVideoItem, kWork, kDestination) == Status::kOk);
  auto opened = model.OpenItem(kVideoItem, kOriginal, kVideo);
  REQUIRE(std::holds_alternative<lifecycle::OpenedPage>(opened));
  REQUIRE(std::get<lifecycle::OpenedPage>(opened).page == kVideo);
  REQUIRE(model.OpenTemporary(kUnrelated, TemporaryTabId{"unrelated-tab"},
                              {kSecond, kHomeSpace},
                              kDestination) == Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kVideoSpace) == Status::kOk);
  return model;
}

NativePictureInPictureSession NativeSession() {
  return {{kSession, kVideo, kWork}, PictureInPictureKind::kVideo, true};
}

PictureInPictureSource Bind(const Model& model,
                            const NativePictureInPictureSession& native) {
  auto result = BindPictureInPictureSource(model, native);
  REQUIRE(std::holds_alternative<PictureInPictureSource>(result));
  return std::get<PictureInPictureSource>(result);
}

template <typename Result>
void ExpectRefusal(const Result& result, Refusal refusal) {
  REQUIRE(std::holds_alternative<Refusal>(result));
  REQUIRE(std::get<Refusal>(result) == refusal);
}

PictureInPictureActionPlan ExpectPlan(
    const PictureInPictureActionResult& result,
    const Attachment& attachment,
    Action action) {
  REQUIRE(std::holds_alternative<PictureInPictureActionPlan>(result));
  auto plan = std::get<PictureInPictureActionPlan>(result);
  REQUIRE(plan.source.page == kVideo);
  REQUIRE(plan.source.profile == kWork);
  REQUIRE(plan.source.session == kSession);
  REQUIRE(plan.attachment == attachment);
  REQUIRE(plan.action == action);
  return plan;
}

void BindingBoundaries() {
  auto model = Workspace();
  auto native = NativeSession();
  const auto valid = Bind(model, native);
  REQUIRE(valid.page == kVideo);
  REQUIRE(valid.profile == kWork);
  REQUIRE(valid.session == kSession);

  native.source.session = PictureInPictureSessionId{""};
  ExpectRefusal(BindPictureInPictureSource(model, native),
                Refusal::kInvalidSource);
  native = NativeSession();
  native.source.page = PageId{""};
  ExpectRefusal(BindPictureInPictureSource(model, native),
                Refusal::kInvalidSource);
  native.source.page = PageId{"missing"};
  ExpectRefusal(BindPictureInPictureSource(model, native),
                Refusal::kPageNotFound);
  native = NativeSession();
  native.source.profile = ProfileId{""};
  ExpectRefusal(BindPictureInPictureSource(model, native),
                Refusal::kInvalidSource);
  native.source.profile = kHome;
  ExpectRefusal(BindPictureInPictureSource(model, native),
                Refusal::kProfileMismatch);
  native = NativeSession();
  native.kind = PictureInPictureKind::kDocument;
  ExpectRefusal(BindPictureInPictureSource(model, native),
                Refusal::kNotVideoPictureInPicture);
  REQUIRE(model.pages().size() == 2);
  REQUIRE(model.pages().at(kVideo).activity == Activity::kUnknown);
}

void SelectionDoesNotRetargetActions() {
  auto model = Workspace();
  const auto native = NativeSession();
  const auto binding = Bind(model, native);
  const auto old_plan =
      ExpectPlan(PlanPictureInPictureAction(model, binding, native,
                                            Action::kReturnToSource),
                 kOriginal, Action::kReturnToSource);
  REQUIRE(model.SelectSpace(kFirst, kOtherSpace) == Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kHomeSpace) == Status::kOk);
  REQUIRE(model.SelectSpace(kSecond, kHomeSpace) == Status::kOk);
  for (auto action : {Action::kTogglePlayPause, Action::kCloseSurfaceOnly,
                      Action::kCloseSurfaceAndPause, Action::kReturnToSource}) {
    auto plan =
        ExpectPlan(PlanPictureInPictureAction(model, binding, native, action),
                   kOriginal, action);
    REQUIRE(!RevalidatePictureInPictureAction(model, plan, native));
  }
  REQUIRE(!RevalidatePictureInPictureAction(model, old_plan, native));
  REQUIRE(model.windows().at(kFirst).selected_space == kHomeSpace);
  REQUIRE(model.pages().at(kUnrelated).profile == kHome);
  REQUIRE(model.pages().at(kVideo).current_navigation == kDestination);
  REQUIRE(model.pages().size() == 2);
}

void SamePageHandoffAndStaleAttachment() {
  auto model = Workspace();
  const auto native = NativeSession();
  const auto binding = Bind(model, native);
  REQUIRE(model.RecordNavigation(
              kVideo, "https://video.example.test/continued") == Status::kOk);
  REQUIRE(model.ReportActivity(kVideo, Activity::kProtected) == Status::kOk);
  const auto old_plan =
      ExpectPlan(PlanPictureInPictureAction(model, binding, native,
                                            Action::kReturnToSource),
                 kOriginal, Action::kReturnToSource);

  REQUIRE(model.TransferSavedPage(kVideo, kTransferred) == Status::kOk);
  REQUIRE(model.SelectSpace(kSecond, kHomeSpace) == Status::kOk);
  REQUIRE(RevalidatePictureInPictureAction(model, old_plan, native) ==
          Refusal::kAttachmentChanged);
  for (auto action : {Action::kTogglePlayPause, Action::kCloseSurfaceOnly,
                      Action::kCloseSurfaceAndPause, Action::kReturnToSource}) {
    auto current =
        ExpectPlan(PlanPictureInPictureAction(model, binding, native, action),
                   kTransferred, action);
    REQUIRE(!RevalidatePictureInPictureAction(model, current, native));
  }
  REQUIRE(old_plan.attachment == kOriginal);
  REQUIRE(model.pages().at(kVideo).profile == kWork);
  REQUIRE(model.pages().at(kVideo).activity == Activity::kProtected);
  REQUIRE(model.pages().at(kVideo).current_navigation ==
          "https://video.example.test/continued");
  REQUIRE(model.items().at(kVideoItem).saved_destination == kDestination);
  REQUIRE(model.pages().size() == 2);
}

void EndedAndReplacedSessions() {
  auto model = Workspace();
  auto native = NativeSession();
  const auto binding = Bind(model, native);
  const auto old_plan =
      ExpectPlan(PlanPictureInPictureAction(model, binding, native,
                                            Action::kTogglePlayPause),
                 kOriginal, Action::kTogglePlayPause);
  ExpectRefusal(PlanPictureInPictureAction(model, binding, std::nullopt,
                                           Action::kReturnToSource),
                Refusal::kNoNativeSession);
  REQUIRE(RevalidatePictureInPictureAction(model, old_plan, std::nullopt) ==
          Refusal::kNoNativeSession);

  // Same logical page/controller can acquire a new player, document or
  // contents. The native adapter must publish a new token in each case.
  for (const char* replacement :
       {"new-player", "new-document", "new-contents"}) {
    native.source.session = PictureInPictureSessionId{replacement};
    ExpectRefusal(PlanPictureInPictureAction(model, binding, native,
                                             Action::kCloseSurfaceOnly),
                  Refusal::kSessionReplaced);
    REQUIRE(RevalidatePictureInPictureAction(model, old_plan, native) ==
            Refusal::kSessionReplaced);
    const auto rebound = Bind(model, native);
    auto result = PlanPictureInPictureAction(model, rebound, native,
                                             Action::kTogglePlayPause);
    REQUIRE(std::holds_alternative<PictureInPictureActionPlan>(result));
    const auto plan = std::get<PictureInPictureActionPlan>(result);
    REQUIRE(plan.source.session == PictureInPictureSessionId{replacement});
    REQUIRE(plan.source.page == kVideo);
    REQUIRE(!RevalidatePictureInPictureAction(model, plan, native));
  }

  native.source = {PictureInPictureSessionId{"other-page-session"}, kUnrelated,
                   kHome};
  ExpectRefusal(PlanPictureInPictureAction(model, binding, native,
                                           Action::kReturnToSource),
                Refusal::kSessionReplaced);
  REQUIRE(model.pages().size() == 2);
}

void NativeMismatchAndCapabilityFailures() {
  auto model = Workspace();
  auto native = NativeSession();
  const auto binding = Bind(model, native);
  const auto plan =
      ExpectPlan(PlanPictureInPictureAction(model, binding, native,
                                            Action::kTogglePlayPause),
                 kOriginal, Action::kTogglePlayPause);
  native.source.page = kUnrelated;
  ExpectRefusal(PlanPictureInPictureAction(model, binding, native,
                                           Action::kReturnToSource),
                Refusal::kSourceMismatch);
  native = NativeSession();
  native.source.profile = kHome;
  REQUIRE(RevalidatePictureInPictureAction(model, plan, native) ==
          Refusal::kProfileMismatch);
  native = NativeSession();
  native.kind = PictureInPictureKind::kDocument;
  REQUIRE(RevalidatePictureInPictureAction(model, plan, native) ==
          Refusal::kNotVideoPictureInPicture);
  native = NativeSession();
  native.can_toggle_play_pause = false;
  REQUIRE(RevalidatePictureInPictureAction(model, plan, native) ==
          Refusal::kPlayPauseUnavailable);
  for (auto action : {Action::kCloseSurfaceOnly, Action::kCloseSurfaceAndPause,
                      Action::kReturnToSource}) {
    ExpectPlan(PlanPictureInPictureAction(model, binding, native, action),
               kOriginal, action);
  }
  auto bad_binding = binding;
  bad_binding.profile = kHome;
  ExpectRefusal(PlanPictureInPictureAction(model, bad_binding, NativeSession(),
                                           Action::kReturnToSource),
                Refusal::kProfileMismatch);
  bad_binding.session = PictureInPictureSessionId{""};
  ExpectRefusal(PlanPictureInPictureAction(model, bad_binding, NativeSession(),
                                           Action::kReturnToSource),
                Refusal::kInvalidSource);
  ExpectRefusal(PlanPictureInPictureAction(model, binding, NativeSession(),
                                           static_cast<Action>(-1)),
                Refusal::kUnsupportedAction);
}

void SourceClosureNeverReopensReplacement() {
  auto model = Workspace();
  const auto native = NativeSession();
  const auto binding = Bind(model, native);
  const auto plan =
      ExpectPlan(PlanPictureInPictureAction(model, binding, native,
                                            Action::kReturnToSource),
                 kOriginal, Action::kReturnToSource);
  // Explicit synthetic quiescence is fixture setup, never inferred from PiP.
  REQUIRE(model.ReportActivity(kVideo, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kVideo) == Status::kOk);
  REQUIRE(RevalidatePictureInPictureAction(model, plan, native) ==
          Refusal::kPageNotFound);
  const PageId reopened{"reopened-page"};
  REQUIRE(std::holds_alternative<lifecycle::OpenedPage>(
      model.OpenItem(kVideoItem, kOriginal, reopened)));
  ExpectRefusal(PlanPictureInPictureAction(model, binding, native,
                                           Action::kReturnToSource),
                Refusal::kPageNotFound);
  ExpectRefusal(BindPictureInPictureSource(model, native),
                Refusal::kPageNotFound);
  REQUIRE(model.pages().at(reopened).profile == kWork);
  REQUIRE(model.pages().size() == 2);
}

void TemporaryKeepAndNonDestructiveCloseIntent() {
  auto model = Workspace();
  const PageId temporary{"temporary-video"};
  REQUIRE(model.OpenTemporary(temporary, TemporaryTabId{"temporary-video-tab"},
                              kOriginal, kDestination) == Status::kOk);
  auto native = NativeSession();
  native.source.page = temporary;
  native.source.session = PictureInPictureSessionId{"temporary-session"};
  const auto binding = Bind(model, native);
  REQUIRE(model.Keep(temporary, ItemId{"kept-video"}) == Status::kOk);
  REQUIRE(model.SetSplit(kLayout, kOriginal, {kVideo, temporary}) ==
          Status::kOk);
  for (auto activity : {Activity::kUnknown, Activity::kProtected}) {
    REQUIRE(model.ReportActivity(temporary, activity) == Status::kOk);
    for (auto action :
         {Action::kTogglePlayPause, Action::kCloseSurfaceOnly,
          Action::kCloseSurfaceAndPause, Action::kReturnToSource}) {
      auto result = PlanPictureInPictureAction(model, binding, native, action);
      REQUIRE(std::holds_alternative<PictureInPictureActionPlan>(result));
      const auto plan = std::get<PictureInPictureActionPlan>(result);
      REQUIRE(plan.source.page == temporary);
      REQUIRE(plan.source.profile == kWork);
      REQUIRE(plan.attachment == kOriginal);
      REQUIRE(plan.action == action);
      REQUIRE(!RevalidatePictureInPictureAction(model, plan, native));
    }
    REQUIRE(model.ClosePage(temporary) == Status::kWorkNotQuiescent);
    REQUIRE(model.pages().at(temporary).activity == activity);
    REQUIRE(model.pages().at(temporary).current_navigation == kDestination);
    REQUIRE(model.splits().at(kLayout).pages ==
            std::vector<PageId>({kVideo, temporary}));
    REQUIRE(model.pages().size() == 3);
    REQUIRE(model.items().size() == 2);
  }
}

}  // namespace
}  // namespace arcified::media

int main() {
  using namespace arcified::media;
  BindingBoundaries();
  SelectionDoesNotRetargetActions();
  SamePageHandoffAndStaleAttachment();
  EndedAndReplacedSessions();
  NativeMismatchAndCapabilityFailures();
  SourceClosureNeverReopensReplacement();
  TemporaryKeepAndNonDestructiveCloseIntent();
  std::cout << "PiP source binding: 7 synthetic fixtures passed\n";
}
