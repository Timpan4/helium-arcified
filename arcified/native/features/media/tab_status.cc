#include "tab_status.h"

#include <tuple>
#include <utility>

namespace arcified::media {
namespace {

StatusIndicator Describe(IndicatorKind kind,
                         Signal state,
                         const char* on,
                         const char* off,
                         const char* unknown) {
  switch (state) {
    case Signal::kOn:
      return {kind, state, on};
    case Signal::kOff:
      return {kind, state, off};
    case Signal::kUnknown:
      return {kind, state, unknown};
  }
  return {kind, Signal::kUnknown, unknown};
}

}  // namespace

bool operator==(const TabSignals& a, const TabSignals& b) {
  return std::tie(a.loading, a.crashed, a.sleeping, a.audible, a.muted,
                  a.microphone, a.camera, a.tab_capture, a.display_capture,
                  a.attention) == std::tie(b.loading, b.crashed, b.sleeping,
                                           b.audible, b.muted, b.microphone,
                                           b.camera, b.tab_capture,
                                           b.display_capture, b.attention);
}

bool operator==(const PageStatus& a, const PageStatus& b) {
  return a.page == b.page && a.profile == b.profile &&
         a.attachment == b.attachment && a.signals == b.signals;
}

std::vector<StatusIndicator> DescribeStatus(const TabSignals& signals,
                                            Signal active) {
  return {
      Describe(IndicatorKind::kActive, active, "Active tab", "Inactive tab",
               "Active tab status unavailable"),
      Describe(IndicatorKind::kLoading, signals.loading, "Loading",
               "Not loading", "Loading status unavailable"),
      Describe(IndicatorKind::kCrashed, signals.crashed, "Crashed",
               "Not crashed", "Crash status unavailable"),
      Describe(IndicatorKind::kSleeping, signals.sleeping, "Sleeping", "Awake",
               "Sleep status unavailable"),
      Describe(IndicatorKind::kAudible, signals.audible, "Audio playing",
               "No audio playing", "Audio status unavailable"),
      Describe(IndicatorKind::kMuted, signals.muted, "Tab audio muted",
               "Tab audio unmuted", "Mute status unavailable"),
      Describe(IndicatorKind::kMicrophone, signals.microphone,
               "Microphone in use", "Microphone not in use",
               "Microphone status unavailable"),
      Describe(IndicatorKind::kCamera, signals.camera, "Camera in use",
               "Camera not in use", "Camera status unavailable"),
      Describe(IndicatorKind::kTabCapture, signals.tab_capture,
               "Tab being captured", "Tab not being captured",
               "Tab capture status unavailable"),
      Describe(IndicatorKind::kDisplayCapture, signals.display_capture,
               "Screen or window capture active",
               "Screen or window capture inactive",
               "Screen capture status unavailable"),
      Describe(IndicatorKind::kAttention, signals.attention, "Needs attention",
               "No attention requested", "Attention status unavailable"),
  };
}

std::string_view ActionAccessibleName(PageActionKind kind) {
  switch (kind) {
    case PageActionKind::kMute:
      return "Mute tab audio";
    case PageActionKind::kUnmute:
      return "Unmute tab audio";
    case PageActionKind::kStopLoading:
      return "Stop loading tab";
    case PageActionKind::kRetryCrash:
      return "Retry crashed tab";
    case PageActionKind::kLocate:
      return "Locate tab";
  }
  return {};
}

ObservationResult TabStatus::Observe(const lifecycle::Model& model,
                                     lifecycle::PageId page,
                                     TabSignals signals) {
  if (!model.pages().count(page))
    return ObservationResult::kPageNotFound;
  auto existing = signals_.find(page);
  if (existing != signals_.end() && existing->second == signals)
    return ObservationResult::kUnchanged;
  signals_.insert_or_assign(std::move(page), std::move(signals));
  return ObservationResult::kChanged;
}

bool TabStatus::Invalidate(const lifecycle::PageId& page) {
  return signals_.erase(page) != 0;
}

std::size_t TabStatus::PruneClosed(const lifecycle::Model& model) {
  std::size_t removed = 0;
  for (auto it = signals_.begin(); it != signals_.end();) {
    if (!model.pages().count(it->first)) {
      it = signals_.erase(it);
      ++removed;
    } else {
      ++it;
    }
  }
  return removed;
}

std::optional<PageStatus> TabStatus::Read(const lifecycle::Model& model,
                                          const lifecycle::PageId& page) const {
  auto live = model.pages().find(page);
  if (live == model.pages().end())
    return std::nullopt;
  auto observation = signals_.find(page);
  return PageStatus{
      page, live->second.profile, live->second.attachment(),
      observation == signals_.end() ? TabSignals{} : observation->second};
}

PageActionResult TabStatus::ResolveAction(const lifecycle::Model& model,
                                          const PageAction& action) const {
  auto current = Read(model, action.expected.page);
  if (!current)
    return ActionRefusal::kPageNotFound;
  if (!(action.expected == *current))
    return ActionRefusal::kStaleSnapshot;

  // Locate supplies a route even for unknown/crashed/sleeping pages. The host
  // owns revealing/focusing and any separate wake/reload policy.
  if (action.kind != PageActionKind::kLocate) {
    if (current->signals.sleeping == Signal::kUnknown)
      return ActionRefusal::kSignalUnavailable;
    if (current->signals.sleeping == Signal::kOn)
      return ActionRefusal::kSleeping;

    Signal required;
    Signal observed;
    switch (action.kind) {
      case PageActionKind::kMute:
        required = Signal::kOff;
        observed = current->signals.muted;
        break;
      case PageActionKind::kUnmute:
        required = Signal::kOn;
        observed = current->signals.muted;
        break;
      case PageActionKind::kStopLoading:
        required = Signal::kOn;
        observed = current->signals.loading;
        break;
      case PageActionKind::kRetryCrash:
        required = Signal::kOn;
        observed = current->signals.crashed;
        break;
      case PageActionKind::kLocate:
        return ActionRefusal::kNotApplicable;
    }
    if (observed == Signal::kUnknown)
      return ActionRefusal::kSignalUnavailable;
    if (observed != required)
      return ActionRefusal::kNotApplicable;
    if (action.kind != PageActionKind::kRetryCrash) {
      if (current->signals.crashed == Signal::kUnknown)
        return ActionRefusal::kSignalUnavailable;
      if (current->signals.crashed == Signal::kOn)
        return ActionRefusal::kNotApplicable;
    }
  }

  return ResolvedPageAction{current->page, current->profile,
                            current->attachment, action.kind};
}

}  // namespace arcified::media
