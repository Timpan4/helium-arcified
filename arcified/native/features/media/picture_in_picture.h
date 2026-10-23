#ifndef ARCIFIED_NATIVE_FEATURES_MEDIA_PICTURE_IN_PICTURE_H_
#define ARCIFIED_NATIVE_FEATURES_MEDIA_PICTURE_IN_PICTURE_H_

#include <optional>
#include <variant>

#include "../../model/lifecycle.h"

namespace arcified::media {

// Allocated by the native adapter, never reused. Rotate on player, document or
// WebContents replacement, but retain through a handoff of the same live page
// and native session. A controller pointer alone cannot identify a session.
using PictureInPictureSessionId =
    lifecycle::Id<struct PictureInPictureSessionTag>;

// References to existing native state, not ownership of a player or page.
// No attachment is cached here: actions resolve it from the lifecycle model.
struct PictureInPictureSource {
  PictureInPictureSessionId session;
  lifecycle::PageId page;
  lifecycle::ProfileId profile;
};

enum class PictureInPictureKind { kVideo, kDocument };

// Fresh report from the native adapter on the serialized browser UI sequence.
// Present only while the native session/surface is live and usable. The native
// controller supplies capability and permission decisions, not this component.
struct NativePictureInPictureSession {
  PictureInPictureSource source;
  PictureInPictureKind kind;
  bool can_toggle_play_pause;
};

enum class PictureInPictureAction {
  kTogglePlayPause,
  kCloseSurfaceOnly,
  kCloseSurfaceAndPause,
  // Native CloseAndFocusInitiator: close PiP without pausing, focus its source.
  kReturnToSource,
};

enum class PictureInPictureRefusal {
  kInvalidSource,
  kPageNotFound,
  kProfileMismatch,
  kNoNativeSession,
  kSessionReplaced,
  kSourceMismatch,
  kNotVideoPictureInPicture,
  kPlayPauseUnavailable,
  kUnsupportedAction,
  kAttachmentChanged,
};

using PictureInPictureBindingResult =
    std::variant<PictureInPictureRefusal, PictureInPictureSource>;

// Bind only after actual native PiP success, never to request/create a session.
// Document PiP is explicitly outside this video control slice.
PictureInPictureBindingResult BindPictureInPictureSource(
    const lifecycle::Model& model,
    const NativePictureInPictureSession& native_session);

// Owned target snapshot, not a reservation or a report of native success.
struct PictureInPictureActionPlan {
  PictureInPictureSource source;
  lifecycle::Attachment attachment;
  PictureInPictureAction action;
};
using PictureInPictureActionResult =
    std::variant<PictureInPictureRefusal, PictureInPictureActionPlan>;

// Every action carries its original source binding. A stale UI action must
// never silently target the replacement PiP session or currently selected tab.
// nullopt native_session means ended/destroyed/unavailable, not "use active
// tab".
PictureInPictureActionResult PlanPictureInPictureAction(
    const lifecycle::Model& model,
    const PictureInPictureSource& binding,
    const std::optional<NativePictureInPictureSession>& native_session,
    PictureInPictureAction action);

// Run immediately before native actuation, with a new native observation.
// nullopt result means this snapshot still matches; it does not prove native
// execution. Attachment changes require an explicit replan, not stale focus.
// The adapter must also validate actual native handles/Profile/host and
// serialize revalidation with dispatch. No asynchronous transaction is provided
// here.
std::optional<PictureInPictureRefusal> RevalidatePictureInPictureAction(
    const lifecycle::Model& model,
    const PictureInPictureActionPlan& plan,
    const std::optional<NativePictureInPictureSession>& native_session);

}  // namespace arcified::media

#endif  // ARCIFIED_NATIVE_FEATURES_MEDIA_PICTURE_IN_PICTURE_H_
