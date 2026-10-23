#include "picture_in_picture.h"

namespace arcified::media {
namespace {

std::optional<PictureInPictureRefusal> ValidateSource(
    const lifecycle::Model& model,
    const PictureInPictureSource& source) {
  if (source.session.value.empty() || source.page.value.empty() ||
      source.profile.value.empty()) {
    return PictureInPictureRefusal::kInvalidSource;
  }
  auto page = model.pages().find(source.page);
  if (page == model.pages().end())
    return PictureInPictureRefusal::kPageNotFound;
  if (page->second.profile != source.profile)
    return PictureInPictureRefusal::kProfileMismatch;
  return std::nullopt;
}

}  // namespace

PictureInPictureBindingResult BindPictureInPictureSource(
    const lifecycle::Model& model,
    const NativePictureInPictureSession& native_session) {
  if (auto refusal = ValidateSource(model, native_session.source))
    return *refusal;
  if (native_session.kind != PictureInPictureKind::kVideo)
    return PictureInPictureRefusal::kNotVideoPictureInPicture;
  return native_session.source;
}

PictureInPictureActionResult PlanPictureInPictureAction(
    const lifecycle::Model& model,
    const PictureInPictureSource& binding,
    const std::optional<NativePictureInPictureSession>& native_session,
    PictureInPictureAction action) {
  if (auto refusal = ValidateSource(model, binding))
    return *refusal;
  if (!native_session)
    return PictureInPictureRefusal::kNoNativeSession;
  if (native_session->source.session != binding.session)
    return PictureInPictureRefusal::kSessionReplaced;
  if (native_session->source.page != binding.page)
    return PictureInPictureRefusal::kSourceMismatch;
  if (native_session->source.profile != binding.profile)
    return PictureInPictureRefusal::kProfileMismatch;
  if (native_session->kind != PictureInPictureKind::kVideo)
    return PictureInPictureRefusal::kNotVideoPictureInPicture;

  switch (action) {
    case PictureInPictureAction::kTogglePlayPause:
      if (!native_session->can_toggle_play_pause)
        return PictureInPictureRefusal::kPlayPauseUnavailable;
      [[fallthrough]];
    case PictureInPictureAction::kCloseSurfaceOnly:
    case PictureInPictureAction::kCloseSurfaceAndPause:
    case PictureInPictureAction::kReturnToSource:
      return PictureInPictureActionPlan{
          binding, model.pages().at(binding.page).attachment(), action};
  }
  return PictureInPictureRefusal::kUnsupportedAction;
}

std::optional<PictureInPictureRefusal> RevalidatePictureInPictureAction(
    const lifecycle::Model& model,
    const PictureInPictureActionPlan& plan,
    const std::optional<NativePictureInPictureSession>& native_session) {
  auto current = PlanPictureInPictureAction(model, plan.source, native_session,
                                            plan.action);
  if (const auto* refusal = std::get_if<PictureInPictureRefusal>(&current))
    return *refusal;
  if (!(std::get<PictureInPictureActionPlan>(current).attachment ==
        plan.attachment)) {
    return PictureInPictureRefusal::kAttachmentChanged;
  }
  return std::nullopt;
}

}  // namespace arcified::media
