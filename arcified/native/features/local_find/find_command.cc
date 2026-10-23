#include "find_command.h"

namespace arcified::local_find {
namespace {

std::optional<Refusal> CheckBinding(
    const lifecycle::PageId& page,
    const lifecycle::ProfileId& profile,
    bool closing,
    CommandEligibility eligibility,
    const std::optional<ControllerSnapshot>& controller) {
  if (eligibility == CommandEligibility::kDeferToNativeInput)
    return Refusal::kDeferToNativeInput;
  if (!controller)
    return Refusal::kControllerUnavailable;
  if (page != controller->bound_page)
    return Refusal::kPageMismatch;
  if (profile != controller->bound_profile)
    return Refusal::kProfileMismatch;
  if (controller->generation.value.empty())
    return Refusal::kMissingGeneration;
  if (closing && !controller->find_ui_active)
    return Refusal::kNoActiveSession;
  return std::nullopt;
}

}  // namespace

FindPlan::FindPlan(NativeOperation operation,
                   const ControllerSnapshot& controller)
    : operation_(operation),
      page_(controller.bound_page),
      profile_(controller.bound_profile),
      generation_(controller.generation) {}

PlanResult PlanFindCommand(
    const lifecycle::PageId& validated_focused_page,
    const lifecycle::ProfileId& validated_profile,
    Command command,
    CommandEligibility eligibility,
    const std::optional<ControllerSnapshot>& controller) {
  if (auto refusal =
          CheckBinding(validated_focused_page, validated_profile,
                       command == Command::kClose, eligibility, controller)) {
    return *refusal;
  }
  switch (command) {
    case Command::kOpen:
      return FindPlan(OpenFindBar{}, *controller);
    case Command::kNext:
      return FindPlan(AdvanceMatch{Direction::kForward}, *controller);
    case Command::kPrevious:
      return FindPlan(AdvanceMatch{Direction::kBackward}, *controller);
    case Command::kClose:
      return FindPlan(EndSessionKeepingSelectionAndResults{}, *controller);
  }
  return Refusal::kUnsupportedCommand;
}

std::optional<Refusal> RevalidateFindPlan(
    const FindPlan& plan,
    const lifecycle::PageId& validated_focused_page,
    const lifecycle::ProfileId& validated_profile,
    CommandEligibility eligibility,
    const std::optional<ControllerSnapshot>& controller) {
  if (auto refusal = CheckBinding(
          validated_focused_page, validated_profile,
          std::holds_alternative<EndSessionKeepingSelectionAndResults>(
              plan.operation()),
          eligibility, controller)) {
    return refusal;
  }
  if (plan.page() != validated_focused_page)
    return Refusal::kPageMismatch;
  if (plan.profile() != validated_profile)
    return Refusal::kProfileMismatch;
  if (plan.generation().value != controller->generation.value)
    return Refusal::kStaleGeneration;
  return std::nullopt;
}

}  // namespace arcified::local_find
