#ifndef ARCIFIED_NATIVE_FEATURES_MEDIA_MEDIA_CONTROLS_H_
#define ARCIFIED_NATIVE_FEATURES_MEDIA_MEDIA_CONTROLS_H_

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "tab_status.h"

namespace arcified::media {

using MediaSessionId = lifecycle::Id<struct MediaSessionTag>;
enum class MediaPlayback { kUnknown, kPaused, kPlaying };
enum class MediaAction { kPlay, kPause, kMuteOutput, kUnmuteOutput, kReturn };

// Caller-owned current native facts, never retained in another status store.
// The host must bind the session to the current registered document/WebContents
// and exact Profile BEFORE supplying it. Session IDs are nonempty and never
// reused for another native session. They are not document/generation tokens.
// Chromium has one MediaSession per WebContents; multiple source pages may
// each have a session. This models neither cast devices nor remote sessions.
struct MediaSessionObservation {
  MediaSessionId session;
  lifecycle::PageId page;
  lifecycle::ProfileId profile;
  std::string source_name;  // Host-filtered display text, never an identity.
  MediaPlayback playback = MediaPlayback::kUnknown;
  bool controllable = false;
  bool can_play = false;
  bool can_pause = false;
};

// Owned UI snapshot, not ownership or native actuation authority. The native
// host must also match the originating UI request to its current document.
struct MediaTarget {
  PageStatus expected;
  std::optional<MediaSessionId> session;
};

struct MediaControl {
  MediaAction action;
  // English reference text; localize in native UI.
  std::string accessible_name;
};

struct MediaRow {
  MediaTarget target;
  std::string source_name;
  MediaPlayback playback;
  std::vector<MediaControl> controls;
};

enum class MediaRefusal {
  kPageNotFound,
  kStatusUnavailable,
  kNotMedia,
  kInvalidSession,
  kAmbiguousSession,
  kProfileMismatch,
  kStaleSnapshot,
  kSessionChanged,
  kUnsupportedAction,
};
using MediaRowResult = std::variant<MediaRefusal, MediaRow>;

// Reads current lifecycle ownership and TabStatus, without mutating either.
// Audio, output mute, capture, and current native sessions qualify as media.
// Unknown crash/sleep state withholds output/playback controls. Return remains
// a route to the current source, never a request to wake/reload/transfer it.
MediaRowResult ProjectMediaPage(
    const lifecycle::Model& model,
    const TabStatus& status,
    const lifecycle::PageId& page,
    const std::vector<MediaSessionObservation>& current_sessions);

// All current model pages are considered, independent of selected Space/window.
// Unresolvable/closed sources are omitted; ProjectMediaPage exposes refusals.
// Ordering is model identity order, not a persisted user order or priority.
std::vector<MediaRow> ProjectMedia(
    const lifecycle::Model& model,
    const TabStatus& status,
    const std::vector<MediaSessionObservation>& current_sessions);

struct MediaActionPlan {
  MediaTarget target;
  MediaAction action;
};
using MediaActionResult = std::variant<MediaRefusal, MediaActionPlan>;

// Rereads ownership/status and current session capabilities. No toggles, native
// dispatch, lifecycle mutation, cleanup permission or optimistic success.
MediaActionResult PlanMediaAction(
    const lifecycle::Model& model,
    const TabStatus& status,
    const std::vector<MediaSessionObservation>& current_sessions,
    const MediaTarget& target,
    MediaAction action);

// Repeat immediately before native actuation with fresh host observations.
// nullopt means local preconditions still match, never native dispatch success.
// The host's registry/document check remains mandatory even if this passes.
std::optional<MediaRefusal> RevalidateMediaPlan(
    const lifecycle::Model& model,
    const TabStatus& status,
    const std::vector<MediaSessionObservation>& current_sessions,
    const MediaActionPlan& plan);

}  // namespace arcified::media

#endif  // ARCIFIED_NATIVE_FEATURES_MEDIA_MEDIA_CONTROLS_H_
