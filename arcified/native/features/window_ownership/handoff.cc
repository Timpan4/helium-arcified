#include "handoff.h"

#include <algorithm>
#include <utility>

namespace arcified::window_ownership {
namespace {

using namespace lifecycle;

std::vector<SplitEffect> SplitEffects(const Model& model,
                                      const PageId& page,
                                      const Attachment& source,
                                      const Attachment& destination) {
  std::vector<SplitEffect> effects;
  if (source == destination)
    return effects;
  for (const auto& [id, split] : model.splits()) {
    auto remaining = split.pages;
    remaining.erase(std::remove(remaining.begin(), remaining.end(), page),
                    remaining.end());
    if (remaining != split.pages)
      effects.push_back(
          {id, split.attachment, split.pages, std::move(remaining)});
  }
  return effects;
}

HandoffResult Prepare(const Model& model,
                      HandoffCommand command,
                      PageId page_id,
                      Attachment requested,
                      Attachment destination,
                      std::optional<PageId> source_active) {
  const auto page = model.pages().find(page_id);
  if (page == model.pages().end())
    return HandoffRefusal::kPageNotFound;
  const auto* saved = std::get_if<SavedPage>(&page->second.owner);
  if (!saved)
    return HandoffRefusal::kPageNotSaved;
  // Same order and conditions as Model::TransferSavedPage. The Model remains
  // the only mutator; synthetic parity checks cover these advisory decisions.
  if (!model.attachments().count(destination))
    return HandoffRefusal::kNotAttached;
  if (page->second.profile != model.spaces().at(destination.space).profile)
    return HandoffRefusal::kProfileMismatch;
  const auto& owner = model.items().at(saved->item).owner;
  if (const auto* space = std::get_if<SpaceId>(&owner))
    if (*space != destination.space)
      return HandoffRefusal::kWrongOwningSpace;

  const auto source = saved->attachment;
  if (source_active) {
    const auto active = model.pages().find(*source_active);
    if (active == model.pages().end() ||
        active->second.attachment().window != source.window)
      return HandoffRefusal::kInvalidActivePage;
  }
  return HandoffPlan{command,
                     page_id,
                     saved->item,
                     page->second.profile,
                     source,
                     requested,
                     destination,
                     model.windows().at(source.window).selected_space,
                     model.windows().at(destination.window).selected_space,
                     {std::move(source_active), page_id},
                     SplitEffects(model, page_id, source, destination)};
}

}  // namespace

bool operator==(const SplitEffect& a, const SplitEffect& b) {
  return a.split == b.split && a.attachment == b.attachment &&
         a.before_pages == b.before_pages &&
         a.remaining_pages == b.remaining_pages;
}

HandoffResult PrepareSavedPageHandoff(
    const Model& model,
    PageId page,
    Attachment destination,
    std::optional<PageId> native_source_active) {
  return Prepare(model, HandoffCommand::kTransferSavedPage, std::move(page),
                 destination, destination, std::move(native_source_active));
}

HandoffResult PrepareOpenItemHandoff(
    const Model& model,
    ItemId item_id,
    Attachment requesting,
    std::optional<PageId> native_source_active) {
  const auto item = model.items().find(item_id);
  if (item == model.items().end())
    return HandoffRefusal::kItemNotFound;
  if (!model.attachments().count(requesting))
    return HandoffRefusal::kNotAttached;
  const auto* owning_space = std::get_if<SpaceId>(&item->second.owner);
  const auto profile = owning_space ? model.spaces().at(*owning_space).profile
                                    : std::get<ProfileId>(item->second.owner);
  if (profile != model.spaces().at(requesting.space).profile)
    return HandoffRefusal::kProfileMismatch;
  Attachment destination = requesting;
  if (owning_space) {
    destination.space = *owning_space;
    if (!model.attachments().count(destination))
      return HandoffRefusal::kNotAttached;
  }
  for (const auto& [id, page] : model.pages()) {
    const auto* saved = std::get_if<SavedPage>(&page.owner);
    if (saved && saved->item == item_id)
      return Prepare(model, HandoffCommand::kOpenItem, id, requesting,
                     destination, std::move(native_source_active));
  }
  return HandoffRefusal::kItemNotLive;
}

Revalidation RevalidateHandoff(const Model& model,
                               const HandoffPlan& plan,
                               std::optional<PageId> native_source_active) {
  const auto source = model.pages().find(plan.page);
  if (source == model.pages().end())
    return Revalidation::kSourceChanged;
  const auto* saved = std::get_if<SavedPage>(&source->second.owner);
  if (!saved || saved->item != plan.item ||
      source->second.profile != plan.profile ||
      !(saved->attachment == plan.source))
    return Revalidation::kSourceChanged;
  if (native_source_active != plan.focus.source_active)
    return Revalidation::kFocusChanged;

  HandoffResult current =
      plan.command == HandoffCommand::kTransferSavedPage
          ? PrepareSavedPageHandoff(model, plan.page,
                                    plan.requested_destination,
                                    std::move(native_source_active))
          : PrepareOpenItemHandoff(model, plan.item, plan.requested_destination,
                                   std::move(native_source_active));
  if (const auto* refusal = std::get_if<HandoffRefusal>(&current)) {
    if (*refusal == HandoffRefusal::kInvalidActivePage)
      return Revalidation::kFocusChanged;
    return Revalidation::kDestinationChanged;
  }
  const auto& fresh = std::get<HandoffPlan>(current);
  if (!(fresh.destination == plan.destination))
    return Revalidation::kDestinationChanged;
  if (fresh.source_selected_space != plan.source_selected_space ||
      fresh.destination_selected_space != plan.destination_selected_space)
    return Revalidation::kSelectionChanged;
  if (fresh.split_effects != plan.split_effects)
    return Revalidation::kSplitEffectsChanged;
  return Revalidation::kCurrent;
}

}  // namespace arcified::window_ownership
