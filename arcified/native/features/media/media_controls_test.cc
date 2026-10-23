#include "media_controls.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace arcified::media {
namespace {

using namespace lifecycle;
static_assert(!std::is_convertible_v<MediaSessionId, PageId>);
static_assert(kContractVersion == 3);

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
const SpaceId kPersonal{"personal"};
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const Attachment kDeskFirst{kFirst, kDesk};
const Attachment kDeskSecond{kSecond, kDesk};
const Attachment kPersonalFirst{kFirst, kPersonal};
const PageId kMusic{"music"};
const PageId kVideo{"video"};
const PageId kCall{"call"};
const ItemId kSavedMusic{"saved-music"};

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kHome) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kPersonal, kHome) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  REQUIRE(model.Attach(kDeskFirst) == Status::kOk);
  REQUIRE(model.Attach(kDeskSecond) == Status::kOk);
  REQUIRE(model.Attach(kPersonalFirst) == Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kPersonal) == Status::kOk);
  REQUIRE(model.AddItem(kSavedMusic, kDesk, "https://media.example.test/") ==
          Status::kOk);
  REQUIRE(std::holds_alternative<OpenedPage>(
      model.OpenItem(kSavedMusic, kDeskFirst, kMusic)));
  REQUIRE(model.OpenTemporary(kVideo, TemporaryTabId{"video-tab"},
                              kPersonalFirst,
                              "https://media.example.test/") == Status::kOk);
  REQUIRE(model.OpenTemporary(kCall, TemporaryTabId{"call-tab"}, kDeskSecond,
                              "https://call.example.test/") == Status::kOk);
  return model;
}

TabSignals Awake() {
  TabSignals signals;
  signals.loading = Signal::kOff;
  signals.crashed = Signal::kOff;
  signals.sleeping = Signal::kOff;
  signals.audible = Signal::kOff;
  signals.muted = Signal::kOff;
  signals.microphone = Signal::kOff;
  signals.camera = Signal::kOff;
  signals.tab_capture = Signal::kOff;
  signals.display_capture = Signal::kOff;
  signals.attention = Signal::kOff;
  return signals;
}

std::vector<MediaSessionObservation> Sessions() {
  return {{MediaSessionId{"music-session"}, kMusic, kWork, "Music example",
           MediaPlayback::kPlaying, true, true, true},
          {MediaSessionId{"video-session"}, kVideo, kHome, "Video example",
           MediaPlayback::kPaused, true, true, true}};
}

TabStatus Observations(const Model& model) {
  TabStatus status;
  auto music = Awake();
  music.audible = Signal::kOn;
  REQUIRE(status.Observe(model, kMusic, music) == ObservationResult::kChanged);
  auto video = Awake();
  video.muted = Signal::kOn;
  REQUIRE(status.Observe(model, kVideo, video) == ObservationResult::kChanged);
  auto call = Awake();
  call.microphone = Signal::kOn;
  call.camera = Signal::kOn;
  call.tab_capture = Signal::kOn;
  call.display_capture = Signal::kOn;
  REQUIRE(status.Observe(model, kCall, call) == ObservationResult::kChanged);
  return status;
}

MediaRow Row(const Model& model,
             const TabStatus& status,
             const PageId& page,
             const std::vector<MediaSessionObservation>& sessions) {
  auto result = ProjectMediaPage(model, status, page, sessions);
  REQUIRE(std::holds_alternative<MediaRow>(result));
  return std::get<MediaRow>(result);
}

bool HasControl(const MediaRow& row, MediaAction action) {
  return std::any_of(
      row.controls.begin(), row.controls.end(),
      [action](const auto& control) { return control.action == action; });
}

MediaActionPlan Plan(const Model& model,
                     const TabStatus& status,
                     const std::vector<MediaSessionObservation>& sessions,
                     const MediaTarget& target,
                     MediaAction action) {
  const auto result = PlanMediaAction(model, status, sessions, target, action);
  REQUIRE(std::holds_alternative<MediaActionPlan>(result));
  return std::get<MediaActionPlan>(result);
}

void ExpectRefusal(const MediaActionResult& result, MediaRefusal expected) {
  REQUIRE(std::holds_alternative<MediaRefusal>(result));
  REQUIRE(std::get<MediaRefusal>(result) == expected);
}

void MultipleSourcesAndCapabilities() {
  auto model = Workspace();
  const auto status = Observations(model);
  const auto sessions = Sessions();
  const auto rows = ProjectMedia(model, status, sessions);
  REQUIRE(rows.size() == 3);
  const auto music = Row(model, status, kMusic, sessions);
  const auto video = Row(model, status, kVideo, sessions);
  const auto call = Row(model, status, kCall, sessions);
  REQUIRE(music.target.expected.profile == kWork);
  REQUIRE(music.target.expected.attachment == kDeskFirst);
  REQUIRE(video.target.expected.profile == kHome);
  REQUIRE(video.target.expected.attachment == kPersonalFirst);
  REQUIRE(call.target.expected.attachment == kDeskSecond);
  REQUIRE(!call.target.session);
  REQUIRE(call.target.expected.signals.microphone == Signal::kOn);
  REQUIRE(call.target.expected.signals.camera == Signal::kOn);
  REQUIRE(call.target.expected.signals.tab_capture == Signal::kOn);
  REQUIRE(call.target.expected.signals.display_capture == Signal::kOn);
  REQUIRE(HasControl(music, MediaAction::kPause));
  REQUIRE(!HasControl(music, MediaAction::kPlay));
  REQUIRE(HasControl(video, MediaAction::kPlay));
  REQUIRE(HasControl(video, MediaAction::kUnmuteOutput));
  REQUIRE(!HasControl(video, MediaAction::kMuteOutput));
  REQUIRE(!HasControl(call, MediaAction::kPlay));
  REQUIRE(!HasControl(call, MediaAction::kPause));
  REQUIRE(music.controls.front().accessible_name == "Pause Music example");
  REQUIRE(call.controls.back().accessible_name == "Return to media tab");
  const auto mute =
      Plan(model, status, sessions, call.target, MediaAction::kMuteOutput);
  REQUIRE(mute.target.expected.page == kCall);
  const auto back =
      Plan(model, status, sessions, music.target, MediaAction::kReturn);
  REQUIRE(back.target.expected.attachment == kDeskFirst);
  REQUIRE(!RevalidateMediaPlan(model, status, sessions, back));
  REQUIRE(model.windows().at(kFirst).selected_space == kPersonal);
  REQUIRE(model.pages().size() == 3);
  REQUIRE(model.pages().at(kCall).activity == Activity::kUnknown);
  REQUIRE(model.pages().at(kMusic).current_navigation ==
          "https://media.example.test/");
}

void TransferRequiresFreshRow() {
  auto model = Workspace();
  const auto status = Observations(model);
  const auto sessions = Sessions();
  REQUIRE(model.ReportActivity(kMusic, Activity::kProtected) == Status::kOk);
  const auto old = Row(model, status, kMusic, sessions);
  const auto plan =
      Plan(model, status, sessions, old.target, MediaAction::kPause);
  REQUIRE(model.TransferSavedPage(kMusic, kDeskSecond) == Status::kOk);
  REQUIRE(RevalidateMediaPlan(model, status, sessions, plan) ==
          MediaRefusal::kStaleSnapshot);
  const auto fresh = Row(model, status, kMusic, sessions);
  REQUIRE(fresh.target.expected.attachment == kDeskSecond);
  REQUIRE(Plan(model, status, sessions, fresh.target, MediaAction::kReturn)
              .target.expected.attachment == kDeskSecond);
  REQUIRE(model.pages().at(kMusic).activity == Activity::kProtected);
  REQUIRE(model.pages().at(kMusic).profile == kWork);
  REQUIRE(model.pages().size() == 3);
}

void SessionFailures() {
  const auto model = Workspace();
  const auto status = Observations(model);
  auto sessions = Sessions();
  const auto target = Row(model, status, kMusic, sessions).target;
  sessions.front().profile = kHome;
  ExpectRefusal(
      PlanMediaAction(model, status, sessions, target, MediaAction::kPause),
      MediaRefusal::kProfileMismatch);
  sessions = Sessions();
  sessions.front().session = MediaSessionId{""};
  ExpectRefusal(
      PlanMediaAction(model, status, sessions, target, MediaAction::kPause),
      MediaRefusal::kInvalidSession);
  sessions = Sessions();
  sessions.push_back(sessions.front());
  ExpectRefusal(
      PlanMediaAction(model, status, sessions, target, MediaAction::kPause),
      MediaRefusal::kAmbiguousSession);
  sessions = Sessions();
  sessions.back().session = sessions.front().session;
  REQUIRE(ProjectMedia(model, status, sessions).size() == 1);
  ExpectRefusal(
      PlanMediaAction(model, status, sessions, target, MediaAction::kPause),
      MediaRefusal::kAmbiguousSession);
  sessions = Sessions();
  auto another = sessions.front();
  another.session = MediaSessionId{"another-session-same-page"};
  sessions.push_back(another);
  ExpectRefusal(
      PlanMediaAction(model, status, sessions, target, MediaAction::kPause),
      MediaRefusal::kAmbiguousSession);
}

void FreshCapabilitiesAndSessionIdentity() {
  const auto model = Workspace();
  const auto status = Observations(model);
  auto sessions = Sessions();
  const auto target = Row(model, status, kMusic, sessions).target;
  const auto plan = Plan(model, status, sessions, target, MediaAction::kPause);
  sessions.front().can_pause = false;
  REQUIRE(RevalidateMediaPlan(model, status, sessions, plan) ==
          MediaRefusal::kUnsupportedAction);
  sessions = Sessions();
  sessions.front().controllable = false;
  ExpectRefusal(
      PlanMediaAction(model, status, sessions, target, MediaAction::kPause),
      MediaRefusal::kUnsupportedAction);
  sessions = Sessions();
  sessions.front().playback = MediaPlayback::kPaused;
  ExpectRefusal(
      PlanMediaAction(model, status, sessions, target, MediaAction::kPause),
      MediaRefusal::kUnsupportedAction);
  REQUIRE(HasControl(Row(model, status, kMusic, sessions), MediaAction::kPlay));
  sessions.front().playback = MediaPlayback::kUnknown;
  REQUIRE(
      !HasControl(Row(model, status, kMusic, sessions), MediaAction::kPlay));
  sessions = Sessions();
  sessions.front().session = MediaSessionId{"replacement-session"};
  REQUIRE(RevalidateMediaPlan(model, status, sessions, plan) ==
          MediaRefusal::kSessionChanged);
  sessions.erase(sessions.begin());
  REQUIRE(RevalidateMediaPlan(model, status, sessions, plan) ==
          MediaRefusal::kSessionChanged);
}

void SleepingCrashedAndUnknown() {
  const auto model = Workspace();
  auto status = Observations(model);
  const auto sessions = Sessions();
  auto signals = status.Read(model, kMusic)->signals;
  for (Signal state : {Signal::kOn, Signal::kUnknown}) {
    signals.sleeping = state;
    status.Observe(model, kMusic, signals);
    const auto row = Row(model, status, kMusic, sessions);
    REQUIRE(row.playback == MediaPlayback::kUnknown);
    REQUIRE(row.controls.size() == 1);
    REQUIRE(row.controls.front().action == MediaAction::kReturn);
    ExpectRefusal(PlanMediaAction(model, status, sessions, row.target,
                                  MediaAction::kPause),
                  MediaRefusal::kUnsupportedAction);
    REQUIRE(Plan(model, status, sessions, row.target, MediaAction::kReturn)
                .target.expected.page == kMusic);
  }
  signals.sleeping = Signal::kOff;
  for (Signal state : {Signal::kOn, Signal::kUnknown}) {
    signals.crashed = state;
    status.Observe(model, kMusic, signals);
    REQUIRE(Row(model, status, kMusic, sessions).controls.size() == 1);
  }
  signals.crashed = Signal::kOff;
  signals.muted = Signal::kUnknown;
  status.Observe(model, kMusic, signals);
  const auto unknown_mute = Row(model, status, kMusic, sessions);
  REQUIRE(HasControl(unknown_mute, MediaAction::kPause));
  REQUIRE(!HasControl(unknown_mute, MediaAction::kMuteOutput));
  REQUIRE(!HasControl(unknown_mute, MediaAction::kUnmuteOutput));
}

void InvalidationAndSignalChange() {
  auto model = Workspace();
  auto status = Observations(model);
  const auto sessions = Sessions();
  const auto old = Row(model, status, kMusic, sessions);
  auto changed = old.target.expected.signals;
  changed.attention = Signal::kOn;
  status.Observe(model, kMusic, changed);
  ExpectRefusal(PlanMediaAction(model, status, sessions, old.target,
                                MediaAction::kReturn),
                MediaRefusal::kStaleSnapshot);
  const auto before_navigation = Row(model, status, kMusic, sessions);
  REQUIRE(status.Invalidate(kMusic));
  REQUIRE(model.RecordNavigation(kMusic, "https://media.example.test/") ==
          Status::kOk);
  ExpectRefusal(PlanMediaAction(model, status, sessions,
                                before_navigation.target, MediaAction::kPause),
                MediaRefusal::kStaleSnapshot);
  // Correct host invalidation, not URL comparison, makes this same-URL case
  // safe.
  REQUIRE(std::get<MediaRefusal>(ProjectMediaPage(model, status, kMusic, {})) ==
          MediaRefusal::kNotMedia);
  const auto unknown = Row(model, status, kMusic, sessions);
  REQUIRE(unknown.controls.size() == 1);
  REQUIRE(unknown.target.expected.signals.audible == Signal::kUnknown);
  auto wrong_profile = unknown.target;
  wrong_profile.expected.profile = kHome;
  ExpectRefusal(PlanMediaAction(model, status, sessions, wrong_profile,
                                MediaAction::kReturn),
                MediaRefusal::kStaleSnapshot);
}

void ClosedPageAndReopen() {
  auto model = Workspace();
  const auto status = Observations(model);
  const auto sessions = Sessions();
  const auto old = Row(model, status, kMusic, sessions);
  const auto plan =
      Plan(model, status, sessions, old.target, MediaAction::kPause);
  REQUIRE(model.ReportActivity(kMusic, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kMusic) == Status::kOk);
  REQUIRE(RevalidateMediaPlan(model, status, sessions, plan) ==
          MediaRefusal::kPageNotFound);
  REQUIRE(ProjectMedia(model, status, sessions).size() == 2);
  REQUIRE(model.items().count(kSavedMusic) == 1);
  const PageId reopened{"reopened-music"};
  REQUIRE(std::holds_alternative<OpenedPage>(
      model.OpenItem(kSavedMusic, kDeskSecond, reopened)));
  REQUIRE(RevalidateMediaPlan(model, status, sessions, plan) ==
          MediaRefusal::kPageNotFound);
  REQUIRE(ProjectMedia(model, status, sessions).size() == 2);
  REQUIRE(model.pages().at(reopened).activity == Activity::kUnknown);
}

void NoCleanupAuthority() {
  auto model = Workspace();
  auto status = Observations(model);
  const auto sessions = Sessions();
  REQUIRE(model.ReportActivity(kVideo, Activity::kProtected) == Status::kOk);
  const auto video = Row(model, status, kVideo, sessions);
  Plan(model, status, sessions, video.target, MediaAction::kUnmuteOutput);
  const auto music = Row(model, status, kMusic, sessions);
  Plan(model, status, sessions, music.target, MediaAction::kPause);
  // Planning does not optimistically alter playback, mute, capture or activity.
  REQUIRE(status.Read(model, kVideo)->signals.muted == Signal::kOn);
  REQUIRE(status.Read(model, kCall)->signals.microphone == Signal::kOn);
  REQUIRE(model.pages().at(kVideo).activity == Activity::kProtected);
  REQUIRE(model.pages().at(kMusic).activity == Activity::kUnknown);
  REQUIRE(model.ClosePage(kVideo) == Status::kWorkNotQuiescent);
  REQUIRE(model.ClosePage(kMusic) == Status::kWorkNotQuiescent);
  REQUIRE(model.Reset(kMusic) == Status::kWorkNotQuiescent);
  status.Observe(model, kMusic, Awake());
  REQUIRE(std::get<MediaRefusal>(ProjectMediaPage(model, status, kMusic, {})) ==
          MediaRefusal::kNotMedia);
  REQUIRE(model.pages().at(kMusic).activity == Activity::kUnknown);
}

}  // namespace
}  // namespace arcified::media

int main() {
  using namespace arcified::media;
  MultipleSourcesAndCapabilities();
  TransferRequiresFreshRow();
  SessionFailures();
  FreshCapabilitiesAndSessionIdentity();
  SleepingCrashedAndUnknown();
  InvalidationAndSignalChange();
  ClosedPageAndReopen();
  NoCleanupAuthority();
  std::cout << "media-controls synthetic checks passed\n";
}
