#include "tab_status.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <set>
#include <utility>

namespace arcified::media {
namespace {

using lifecycle::Activity;
using lifecycle::Attachment;
using lifecycle::ItemId;
using lifecycle::Model;
using lifecycle::PageId;
using lifecycle::ProfileId;
using lifecycle::SpaceId;
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

const ProfileId kWork{"work"};
const ProfileId kPersonal{"personal"};
const SpaceId kDesk{"desk"};
const SpaceId kHome{"home"};
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const Attachment kDeskFirst{kFirst, kDesk};
const Attachment kDeskSecond{kSecond, kDesk};
const Attachment kHomeFirst{kFirst, kHome};
const ItemId kItem{"saved"};
const PageId kPage{"saved-page"};
const PageId kOther{"other-page"};
const std::string kDestination = "https://media.example.test/fixture";

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kPersonal) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kHome, kPersonal) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  REQUIRE(model.Attach(kDeskFirst) == Status::kOk);
  REQUIRE(model.Attach(kDeskSecond) == Status::kOk);
  REQUIRE(model.Attach(kHomeFirst) == Status::kOk);
  REQUIRE(model.AddItem(kItem, kDesk, kDestination) == Status::kOk);
  REQUIRE(std::holds_alternative<lifecycle::OpenedPage>(
      model.OpenItem(kItem, kDeskFirst, kPage)));
  REQUIRE(model.OpenTemporary(kOther, TemporaryTabId{"other-tab"}, kHomeFirst,
                              kDestination) == Status::kOk);
  return model;
}

// Enumerates the public signal contract to exercise each field independently.
constexpr auto kFields =
    std::array{&TabSignals::loading,         &TabSignals::crashed,
               &TabSignals::sleeping,        &TabSignals::audible,
               &TabSignals::muted,           &TabSignals::microphone,
               &TabSignals::camera,          &TabSignals::tab_capture,
               &TabSignals::display_capture, &TabSignals::attention};

TabSignals All(Signal value) {
  TabSignals signals;
  for (auto field : kFields)
    signals.*field = value;
  return signals;
}

PageStatus Read(const TabStatus& status,
                const Model& model,
                const PageId& page) {
  auto result = status.Read(model, page);
  REQUIRE(result.has_value());
  return *result;
}

void ExpectRefusal(const PageActionResult& result, ActionRefusal refusal) {
  REQUIRE(std::holds_alternative<ActionRefusal>(result));
  REQUIRE(std::get<ActionRefusal>(result) == refusal);
}

void ExpectRoute(const PageActionResult& result,
                 const PageStatus& expected,
                 PageActionKind kind) {
  REQUIRE(std::holds_alternative<ResolvedPageAction>(result));
  const auto& route = std::get<ResolvedPageAction>(result);
  REQUIRE(route.page == expected.page);
  REQUIRE(route.profile == expected.profile);
  REQUIRE(route.attachment == expected.attachment);
  REQUIRE(route.kind == kind);
}

void UnknownIsNotFalse() {
  auto model = Workspace();
  TabStatus status;
  REQUIRE(!status.Read(model, PageId{"missing"}));
  auto page = Read(status, model, kPage);
  REQUIRE(page.profile == kWork);
  REQUIRE(page.attachment == kDeskFirst);
  REQUIRE(page.signals == TabSignals{});
  const auto unknown = DescribeStatus(page.signals, Signal::kUnknown);
  std::set<IndicatorKind> kinds;
  for (const auto& indicator : unknown) {
    REQUIRE(indicator.state == Signal::kUnknown);
    REQUIRE(indicator.accessible_name.find("unavailable") != std::string::npos);
    REQUIRE(kinds.insert(indicator.kind).second);
  }
  REQUIRE(kinds.count(IndicatorKind::kActive));
  REQUIRE(kinds.count(IndicatorKind::kLoading));
  REQUIRE(kinds.count(IndicatorKind::kCrashed));
  REQUIRE(kinds.count(IndicatorKind::kSleeping));
  REQUIRE(kinds.count(IndicatorKind::kAudible));
  REQUIRE(kinds.count(IndicatorKind::kMuted));
  REQUIRE(kinds.count(IndicatorKind::kMicrophone));
  REQUIRE(kinds.count(IndicatorKind::kCamera));
  REQUIRE(kinds.count(IndicatorKind::kTabCapture));
  REQUIRE(kinds.count(IndicatorKind::kDisplayCapture));
  REQUIRE(kinds.count(IndicatorKind::kAttention));
  ExpectRefusal(status.ResolveAction(model, {page, PageActionKind::kMute}),
                ActionRefusal::kSignalUnavailable);
  ExpectRoute(status.ResolveAction(model, {page, PageActionKind::kLocate}),
              page, PageActionKind::kLocate);
}

void IndependentSignalsAndText() {
  auto model = Workspace();
  TabStatus status;
  const auto all_on = All(Signal::kOn);
  REQUIRE(status.Observe(model, kPage, all_on) == ObservationResult::kChanged);
  REQUIRE(Read(status, model, kPage).signals == all_on);
  const auto on = DescribeStatus(all_on, Signal::kOn);
  const auto off = DescribeStatus(All(Signal::kOff), Signal::kOff);
  const std::array on_names{"Active tab",
                            "Loading",
                            "Crashed",
                            "Sleeping",
                            "Audio playing",
                            "Tab audio muted",
                            "Microphone in use",
                            "Camera in use",
                            "Tab being captured",
                            "Screen or window capture active",
                            "Needs attention"};
  const std::array off_names{"Inactive tab",
                             "Not loading",
                             "Not crashed",
                             "Awake",
                             "No audio playing",
                             "Tab audio unmuted",
                             "Microphone not in use",
                             "Camera not in use",
                             "Tab not being captured",
                             "Screen or window capture inactive",
                             "No attention requested"};
  REQUIRE(on.size() == on_names.size());
  REQUIRE(off.size() == off_names.size());
  for (std::size_t i = 0; i < on.size(); ++i) {
    REQUIRE(on[i].state == Signal::kOn);
    REQUIRE(on[i].accessible_name == on_names[i]);
    REQUIRE(off[i].state == Signal::kOff);
    REQUIRE(off[i].accessible_name == off_names[i]);
    REQUIRE(on[i].kind == off[i].kind);
  }
  for (auto field : kFields) {
    auto changed = all_on;
    changed.*field = Signal::kOff;
    REQUIRE(status.Observe(model, kPage, changed) ==
            ObservationResult::kChanged);
    REQUIRE(Read(status, model, kPage).signals == changed);
    REQUIRE(status.Observe(model, kPage, changed) ==
            ObservationResult::kUnchanged);
    REQUIRE(status.Observe(model, kPage, all_on) ==
            ObservationResult::kChanged);
  }
}

void HiddenUpdatesAndInvalidation() {
  auto model = Workspace();
  TabStatus status;
  // The source Space is hidden by another Profile's selected Space. No view,
  // layout callback or mounted row participates in observing or reading it.
  REQUIRE(model.SelectSpace(kFirst, kHome) == Status::kOk);
  auto signals = All(Signal::kOff);
  REQUIRE(status.Observe(model, kPage, signals) == ObservationResult::kChanged);
  const auto old = Read(status, model, kPage);
  signals.loading = Signal::kOn;
  signals.microphone = Signal::kOn;
  signals.muted = Signal::kOn;
  REQUIRE(status.Observe(model, kPage, signals) == ObservationResult::kChanged);
  REQUIRE(status.Observe(model, kPage, signals) ==
          ObservationResult::kUnchanged);
  REQUIRE(Read(status, model, kPage).signals == signals);
  REQUIRE(model.windows().at(kFirst).selected_space == kHome);
  ExpectRefusal(status.ResolveAction(model, {old, PageActionKind::kMute}),
                ActionRefusal::kStaleSnapshot);

  const auto before_navigation = Read(status, model, kPage);
  REQUIRE(status.Invalidate(kPage));
  REQUIRE(model.RecordNavigation(kPage, "https://next.example.test/") ==
          Status::kOk);
  REQUIRE(Read(status, model, kPage).signals == TabSignals{});
  ExpectRefusal(status.ResolveAction(
                    model, {before_navigation, PageActionKind::kStopLoading}),
                ActionRefusal::kStaleSnapshot);
  REQUIRE(!status.Invalidate(kPage));
  // The host must reject old-document callbacks, including same-URL reloads.
  // This component receives only the host's accepted new-document snapshot.
  REQUIRE(status.Observe(model, kPage, All(Signal::kOff)) ==
          ObservationResult::kChanged);
  REQUIRE(Read(status, model, kPage).signals.loading == Signal::kOff);
}

void OwnershipAndStaleRows() {
  auto model = Workspace();
  TabStatus status;
  REQUIRE(status.Observe(model, kPage, All(Signal::kOff)) ==
          ObservationResult::kChanged);
  const auto original = Read(status, model, kPage);
  auto wrong_profile = original;
  wrong_profile.profile = kPersonal;
  ExpectRefusal(
      status.ResolveAction(model, {wrong_profile, PageActionKind::kMute}),
      ActionRefusal::kStaleSnapshot);
  REQUIRE(model.SelectSpace(kFirst, kHome) == Status::kOk);
  ExpectRoute(status.ResolveAction(model, {original, PageActionKind::kMute}),
              original, PageActionKind::kMute);
  REQUIRE(model.TransferSavedPage(kPage, kDeskSecond) == Status::kOk);
  const auto transferred = Read(status, model, kPage);
  REQUIRE(transferred.page == original.page);
  REQUIRE(transferred.profile == kWork);
  REQUIRE(transferred.attachment == kDeskSecond);
  REQUIRE(transferred.signals == original.signals);
  ExpectRefusal(status.ResolveAction(model, {original, PageActionKind::kMute}),
                ActionRefusal::kStaleSnapshot);
  ExpectRefusal(
      status.ResolveAction(model, {original, PageActionKind::kLocate}),
      ActionRefusal::kStaleSnapshot);
  ExpectRoute(
      status.ResolveAction(model, {transferred, PageActionKind::kLocate}),
      transferred, PageActionKind::kLocate);
  // Identical destinations in another Profile never merge signal identities.
  REQUIRE(Read(status, model, kOther).profile == kPersonal);
  REQUIRE(Read(status, model, kOther).signals == TabSignals{});
}

void ActionGatingAndExplicitIntents() {
  auto model = Workspace();
  TabStatus status;
  auto signals = All(Signal::kOff);
  REQUIRE(status.Observe(model, kPage, signals) == ObservationResult::kChanged);
  auto page = Read(status, model, kPage);
  ExpectRoute(status.ResolveAction(model, {page, PageActionKind::kMute}), page,
              PageActionKind::kMute);
  ExpectRefusal(status.ResolveAction(model, {page, PageActionKind::kUnmute}),
                ActionRefusal::kNotApplicable);
  ExpectRefusal(
      status.ResolveAction(model, {page, PageActionKind::kStopLoading}),
      ActionRefusal::kNotApplicable);
  ExpectRefusal(
      status.ResolveAction(model, {page, PageActionKind::kRetryCrash}),
      ActionRefusal::kNotApplicable);

  signals.muted = Signal::kOn;
  signals.loading = Signal::kOn;
  REQUIRE(status.Observe(model, kPage, signals) == ObservationResult::kChanged);
  // An old Mute intent never turns into Unmute after the signal changes.
  ExpectRefusal(status.ResolveAction(model, {page, PageActionKind::kMute}),
                ActionRefusal::kStaleSnapshot);
  page = Read(status, model, kPage);
  ExpectRoute(status.ResolveAction(model, {page, PageActionKind::kUnmute}),
              page, PageActionKind::kUnmute);
  ExpectRoute(status.ResolveAction(model, {page, PageActionKind::kStopLoading}),
              page, PageActionKind::kStopLoading);

  signals.crashed = Signal::kOn;
  REQUIRE(status.Observe(model, kPage, signals) == ObservationResult::kChanged);
  page = Read(status, model, kPage);
  ExpectRoute(status.ResolveAction(model, {page, PageActionKind::kRetryCrash}),
              page, PageActionKind::kRetryCrash);
  ExpectRefusal(status.ResolveAction(model, {page, PageActionKind::kUnmute}),
                ActionRefusal::kNotApplicable);
  signals.sleeping = Signal::kOn;
  REQUIRE(status.Observe(model, kPage, signals) == ObservationResult::kChanged);
  page = Read(status, model, kPage);
  ExpectRefusal(
      status.ResolveAction(model, {page, PageActionKind::kRetryCrash}),
      ActionRefusal::kSleeping);
  ExpectRoute(status.ResolveAction(model, {page, PageActionKind::kLocate}),
              page, PageActionKind::kLocate);

  signals = All(Signal::kOff);
  signals.muted = Signal::kUnknown;
  REQUIRE(status.Observe(model, kPage, signals) == ObservationResult::kChanged);
  page = Read(status, model, kPage);
  ExpectRefusal(status.ResolveAction(model, {page, PageActionKind::kMute}),
                ActionRefusal::kSignalUnavailable);
  // Every mutating route requires known current facts. Missing one required
  // fact never becomes false through a default or another signal's priority.
  for (const auto kind :
       {PageActionKind::kMute, PageActionKind::kUnmute,
        PageActionKind::kStopLoading, PageActionKind::kRetryCrash}) {
    auto applicable = All(Signal::kOff);
    if (kind == PageActionKind::kUnmute)
      applicable.muted = Signal::kOn;
    if (kind == PageActionKind::kStopLoading)
      applicable.loading = Signal::kOn;
    if (kind == PageActionKind::kRetryCrash)
      applicable.crashed = Signal::kOn;
    for (const auto field : {&TabSignals::sleeping, &TabSignals::crashed}) {
      auto unavailable = applicable;
      unavailable.*field = Signal::kUnknown;
      status.Observe(model, kPage, unavailable);
      const auto current = Read(status, model, kPage);
      ExpectRefusal(status.ResolveAction(model, {current, kind}),
                    ActionRefusal::kSignalUnavailable);
    }
    auto unavailable = applicable;
    switch (kind) {
      case PageActionKind::kMute:
      case PageActionKind::kUnmute:
        unavailable.muted = Signal::kUnknown;
        break;
      case PageActionKind::kStopLoading:
        unavailable.loading = Signal::kUnknown;
        break;
      case PageActionKind::kRetryCrash:
        unavailable.crashed = Signal::kUnknown;
        break;
      case PageActionKind::kLocate:
        REQUIRE(false);
    }
    status.Observe(model, kPage, unavailable);
    ExpectRefusal(
        status.ResolveAction(model, {Read(status, model, kPage), kind}),
        ActionRefusal::kSignalUnavailable);
  }
  for (const auto kind : {PageActionKind::kMute, PageActionKind::kUnmute,
                          PageActionKind::kStopLoading,
                          PageActionKind::kRetryCrash, PageActionKind::kLocate})
    REQUIRE(!ActionAccessibleName(kind).empty());
}

void ClosedPagesAndReopen() {
  auto model = Workspace();
  TabStatus status;
  REQUIRE(status.Observe(model, kPage, All(Signal::kOn)) ==
          ObservationResult::kChanged);
  REQUIRE(status.Observe(model, kOther, All(Signal::kOff)) ==
          ObservationResult::kChanged);
  const auto before_close = Read(status, model, kPage);
  // Explicit fixture report only. Status observations do not authorize close.
  REQUIRE(model.ReportActivity(kPage, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kPage) == Status::kOk);
  REQUIRE(!status.Read(model, kPage));
  REQUIRE(status.Observe(model, kPage, All(Signal::kOff)) ==
          ObservationResult::kPageNotFound);
  ExpectRefusal(
      status.ResolveAction(model, {before_close, PageActionKind::kLocate}),
      ActionRefusal::kPageNotFound);
  REQUIRE(status.PruneClosed(model) == 1);
  REQUIRE(status.PruneClosed(model) == 0);
  REQUIRE(Read(status, model, kOther).signals == All(Signal::kOff));
  const PageId reopened{"reopened-with-fresh-id"};
  REQUIRE(std::holds_alternative<lifecycle::OpenedPage>(
      model.OpenItem(kItem, kDeskSecond, reopened)));
  REQUIRE(Read(status, model, reopened).signals == TabSignals{});
}

void SignalsDoNotApproveActivityOrMutateOwners() {
  auto model = Workspace();
  TabStatus status;
  REQUIRE(model.ReportActivity(kPage, Activity::kProtected) == Status::kOk);
  const auto attachment = model.pages().at(kPage).attachment();
  REQUIRE(status.Observe(model, kPage, All(Signal::kOff)) ==
          ObservationResult::kChanged);
  const auto page = Read(status, model, kPage);
  ExpectRoute(status.ResolveAction(model, {page, PageActionKind::kMute}), page,
              PageActionKind::kMute);
  ExpectRoute(status.ResolveAction(model, {page, PageActionKind::kLocate}),
              page, PageActionKind::kLocate);
  REQUIRE(model.pages().at(kPage).activity == Activity::kProtected);
  REQUIRE(model.pages().at(kPage).attachment() == attachment);
  REQUIRE(model.pages().at(kPage).current_navigation == kDestination);
  REQUIRE(model.items().at(kItem).saved_destination == kDestination);
  REQUIRE(!model.windows().at(kFirst).selected_space);
  REQUIRE(model.Reset(kPage) == Status::kWorkNotQuiescent);
  REQUIRE(model.ClosePage(kPage) == Status::kWorkNotQuiescent);
  REQUIRE(model.pages().at(kOther).activity == Activity::kUnknown);
}

}  // namespace
}  // namespace arcified::media

int main() {
  using namespace arcified::media;
  UnknownIsNotFalse();
  IndependentSignalsAndText();
  HiddenUpdatesAndInvalidation();
  OwnershipAndStaleRows();
  ActionGatingAndExplicitIntents();
  ClosedPagesAndReopen();
  SignalsDoNotApproveActivityOrMutateOwners();
  std::cout << "tab-status: 7 synthetic fixtures passed\n";
}
