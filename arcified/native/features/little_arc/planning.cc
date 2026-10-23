#include "planning.h"

namespace arcified::little_arc {
namespace {

std::optional<Refusal> CheckTarget(const lifecycle::Model& model,
                                   const ResolvedTarget& target) {
  const auto space = model.spaces().find(target.attachment.space);
  if (!model.profiles().count(target.profile) ||
      space == model.spaces().end() ||
      !model.windows().count(target.attachment.window)) {
    return Refusal::kTargetNotFound;
  }
  if (space->second.profile != target.profile)
    return Refusal::kTargetProfileMismatch;
  if (!model.attachments().count(target.attachment))
    return Refusal::kTargetNotAttached;
  return std::nullopt;
}

}  // namespace

EntryResult PlanEntry(const lifecycle::Model& model,
                      ResolvedTarget target,
                      EntryIntent intent,
                      Presentation presentation) {
  if (auto refusal = CheckTarget(model, target))
    return *refusal;
  if (const auto* link = std::get_if<ExternalLink>(&intent)) {
    if (link->destination.empty())
      return Refusal::kEmptyDestination;
  }
  return EntryPlan{std::move(target), std::move(intent), presentation};
}

PromotionResult PlanPromotion(const lifecycle::Model& model,
                              lifecycle::PageId page,
                              ResolvedTarget target) {
  if (auto refusal = CheckTarget(model, target))
    return *refusal;
  const auto live = model.pages().find(page);
  if (live == model.pages().end())
    return Refusal::kPageNotFound;
  if (live->second.profile != target.profile)
    return Refusal::kPageProfileMismatch;

  PageReference source{std::move(page), live->second.profile,
                       live->second.attachment()};
  if (source.attachment == target.attachment)
    return PresentInMainWindow{std::move(source)};

  const auto* saved = std::get_if<lifecycle::SavedPage>(&live->second.owner);
  if (!saved)
    return Refusal::kTemporaryTransferUnavailable;
  const auto& owner = model.items().at(saved->item).owner;
  if (const auto* space = std::get_if<lifecycle::SpaceId>(&owner)) {
    if (*space != target.attachment.space)
      return Refusal::kSavedSpaceMismatch;
  }
  return TransferSavedPage{std::move(source), saved->item,
                           std::move(target.attachment)};
}

}  // namespace arcified::little_arc
