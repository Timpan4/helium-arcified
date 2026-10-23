#include "media_controls.h"

#include <algorithm>
#include <utility>

namespace arcified::media {
namespace {

bool HasMediaSignal(const TabSignals& signals) {
  return signals.audible == Signal::kOn || signals.muted == Signal::kOn ||
         signals.microphone == Signal::kOn || signals.camera == Signal::kOn ||
         signals.tab_capture == Signal::kOn ||
         signals.display_capture == Signal::kOn;
}

void AddControl(MediaRow& row, MediaAction action, const std::string& verb) {
  row.controls.push_back({action, verb + " " + row.source_name});
}

}  // namespace

MediaRowResult ProjectMediaPage(
    const lifecycle::Model& model,
    const TabStatus& status,
    const lifecycle::PageId& page,
    const std::vector<MediaSessionObservation>& current_sessions) {
  const auto live = model.pages().find(page);
  if (live == model.pages().end())
    return MediaRefusal::kPageNotFound;
  const auto source = status.Read(model, page);
  if (!source)
    return MediaRefusal::kStatusUnavailable;

  const MediaSessionObservation* session = nullptr;
  for (const auto& candidate : current_sessions) {
    if (candidate.page != page)
      continue;
    if (candidate.session.value.empty())
      return MediaRefusal::kInvalidSession;
    if (candidate.profile != live->second.profile)
      return MediaRefusal::kProfileMismatch;
    // Fail closed for duplicated session IDs even across different pages.
    const auto uses =
        std::count_if(current_sessions.begin(), current_sessions.end(),
                      [&candidate](const auto& other) {
                        return other.session == candidate.session;
                      });
    if (session || uses != 1)
      return MediaRefusal::kAmbiguousSession;
    session = &candidate;
  }
  if (!session && !HasMediaSignal(source->signals))
    return MediaRefusal::kNotMedia;

  MediaRow row{
      {*source, session ? std::optional(session->session) : std::nullopt},
      session && !session->source_name.empty() ? session->source_name
                                               : "media tab",
      MediaPlayback::kUnknown,
      {}};
  // Unknown never means awake or usable. Capture remains independently visible.
  if (source->signals.sleeping == Signal::kOff &&
      source->signals.crashed == Signal::kOff) {
    if (session) {
      row.playback = session->playback;
      if (session->controllable) {
        switch (session->playback) {
          case MediaPlayback::kUnknown:
            break;
          case MediaPlayback::kPaused:
            if (session->can_play)
              AddControl(row, MediaAction::kPlay, "Play");
            break;
          case MediaPlayback::kPlaying:
            if (session->can_pause)
              AddControl(row, MediaAction::kPause, "Pause");
            break;
        }
      }
    }
    switch (source->signals.muted) {
      case Signal::kUnknown:
        break;
      case Signal::kOff:
        AddControl(row, MediaAction::kMuteOutput, "Mute audio from");
        break;
      case Signal::kOn:
        AddControl(row, MediaAction::kUnmuteOutput, "Unmute audio from");
        break;
    }
  }
  AddControl(row, MediaAction::kReturn, "Return to");
  return row;
}

std::vector<MediaRow> ProjectMedia(
    const lifecycle::Model& model,
    const TabStatus& status,
    const std::vector<MediaSessionObservation>& current_sessions) {
  std::vector<MediaRow> rows;
  for (const auto& [page, live] : model.pages()) {
    auto result = ProjectMediaPage(model, status, page, current_sessions);
    if (auto* row = std::get_if<MediaRow>(&result))
      rows.push_back(std::move(*row));
  }
  return rows;
}

MediaActionResult PlanMediaAction(
    const lifecycle::Model& model,
    const TabStatus& status,
    const std::vector<MediaSessionObservation>& current_sessions,
    const MediaTarget& target,
    MediaAction action) {
  // Check the presented snapshot first, including capture-only requests. An
  // observation invalidation must not turn an old UI row into a new target.
  if (!model.pages().count(target.expected.page))
    return MediaRefusal::kPageNotFound;
  const auto source = status.Read(model, target.expected.page);
  if (!source)
    return MediaRefusal::kStatusUnavailable;
  if (!(*source == target.expected))
    return MediaRefusal::kStaleSnapshot;
  auto result =
      ProjectMediaPage(model, status, target.expected.page, current_sessions);
  if (auto* refusal = std::get_if<MediaRefusal>(&result))
    return *refusal;
  const auto& row = std::get<MediaRow>(result);
  if (row.target.session != target.session)
    return MediaRefusal::kSessionChanged;
  const auto control = std::find_if(
      row.controls.begin(), row.controls.end(),
      [action](const auto& control) { return control.action == action; });
  if (control == row.controls.end())
    return MediaRefusal::kUnsupportedAction;
  return MediaActionPlan{row.target, action};
}

std::optional<MediaRefusal> RevalidateMediaPlan(
    const lifecycle::Model& model,
    const TabStatus& status,
    const std::vector<MediaSessionObservation>& current_sessions,
    const MediaActionPlan& plan) {
  const auto result = PlanMediaAction(model, status, current_sessions,
                                      plan.target, plan.action);
  if (const auto* refusal = std::get_if<MediaRefusal>(&result))
    return *refusal;
  return std::nullopt;
}

}  // namespace arcified::media
