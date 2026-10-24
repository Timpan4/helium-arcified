#include "window_restore.h"

#include <algorithm>
#include <set>

namespace arcified::window_restore {
namespace {

bool SameOwner(const lifecycle::PageOwner& a, const lifecycle::PageOwner& b) {
  if (a.index() != b.index())
    return false;
  if (const auto* temporary = std::get_if<lifecycle::TemporaryTab>(&a)) {
    const auto& other = std::get<lifecycle::TemporaryTab>(b);
    return temporary->id == other.id &&
           temporary->attachment == other.attachment;
  }
  const auto& saved = std::get<lifecycle::SavedPage>(a);
  const auto& other = std::get<lifecycle::SavedPage>(b);
  return saved.item == other.item && saved.attachment == other.attachment;
}

Error CheckOwners(const lifecycle::Model& model,
                  const std::vector<ClosedSpace>& spaces,
                  const std::vector<ClosedPage>& pages) {
  for (const auto& closed : spaces) {
    const auto space = model.spaces().find(closed.space);
    if (space == model.spaces().end() ||
        space->second.profile != closed.profile)
      return Error::kOwnerChanged;
  }
  for (const auto& page : pages) {
    const auto* saved = std::get_if<lifecycle::SavedPage>(&page.owner);
    if (!saved)
      continue;
    const auto item = model.items().find(saved->item);
    if (item == model.items().end())
      return Error::kOwnerChanged;
    if (const auto* space =
            std::get_if<lifecycle::SpaceId>(&item->second.owner)) {
      if (*space != saved->attachment.space)
        return Error::kOwnerChanged;
    } else if (std::get<lifecycle::ProfileId>(item->second.owner) !=
               page.profile) {
      return Error::kOwnerChanged;
    }
  }
  return Error::kOk;
}

Error CheckAvailable(const lifecycle::Model& model, const ReopenPlan& plan) {
  const auto owners = CheckOwners(model, plan.spaces, plan.pages);
  if (owners != Error::kOk)
    return owners;
  if (model.windows().count(plan.window))
    return Error::kIdentityConflict;
  for (const auto& candidate : plan.pages) {
    if (model.pages().count(candidate.page))
      return Error::kIdentityConflict;
    for (const auto& [id, live] : model.pages()) {
      const auto* temporary =
          std::get_if<lifecycle::TemporaryTab>(&candidate.owner);
      const auto* live_temporary =
          std::get_if<lifecycle::TemporaryTab>(&live.owner);
      if (temporary && live_temporary && temporary->id == live_temporary->id)
        return Error::kIdentityConflict;
      const auto* saved = std::get_if<lifecycle::SavedPage>(&candidate.owner);
      const auto* live_saved = std::get_if<lifecycle::SavedPage>(&live.owner);
      if (saved && live_saved && saved->item == live_saved->item)
        return Error::kSavedPageAlreadyLive;
    }
  }
  return Error::kOk;
}

bool HasSplits(const lifecycle::Model& model,
               const lifecycle::WindowId& window) {
  for (const auto& [id, split] : model.splits())
    if (split.attachment.window == window)
      return true;
  return false;
}

}  // namespace

Error Journal::StageClose(UndoId undo,
                          const lifecycle::Model& model,
                          CloseReason reason,
                          const CaptureEligibility& eligibility,
                          lifecycle::WindowId window,
                          Presentation presentation,
                          std::vector<NativePageJoin> joins) {
  if (reason != CloseReason::kDeliberate ||
      !eligibility.IsWindowVerifiedRegular(window))
    return Error::kExcluded;
  const auto current = model.windows().find(window);
  if (current == model.windows().end())
    return Error::kNotFound;
  // Check every actual page, not only caller-listed pages, before retaining
  // any native handle or constructing a historical record.
  for (const auto& [id, page] : model.pages())
    if (page.attachment().window == window &&
        !eligibility.IsPageVerifiedRegular(window, id, page.profile))
      return Error::kExcluded;
  if (HasSplits(model, window))
    return Error::kSplitIntegrationPending;
  if (undo.value.empty())
    return Error::kIdentityConflict;
  if (entries_.count(undo))
    return Error::kDuplicateId;

  ClosedWindow record{window,
                      {},
                      {},
                      current->second.selected_space,
                      presentation.selected_page};
  std::set<lifecycle::SpaceId> spaces;
  for (const auto& space : presentation.spaces) {
    if (!spaces.insert(space).second ||
        !model.attachments().count({window, space}))
      return Error::kInvalidPresentation;
    record.spaces.push_back({space, model.spaces().at(space).profile});
  }
  for (const auto& attachment : model.attachments())
    if (attachment.window == window && !spaces.count(attachment.space))
      return Error::kInvalidPresentation;

  std::map<lifecycle::PageId, NativeRestoreId> native;
  for (auto& join : joins)
    if (join.native.value.empty() ||
        !native.emplace(join.page, std::move(join.native)).second)
      return Error::kNativeRestoreRequired;
  std::set<lifecycle::PageId> pages;
  std::set<std::pair<lifecycle::ProfileId, NativeRestoreId>> native_keys;
  for (const auto& id : presentation.pages) {
    const auto page = model.pages().find(id);
    if (!pages.insert(id).second || page == model.pages().end() ||
        page->second.attachment().window != window)
      return Error::kInvalidPresentation;
    const auto join = native.find(id);
    if (join == native.end() ||
        !native_keys.emplace(page->second.profile, join->second).second)
      return Error::kNativeRestoreRequired;
    record.pages.push_back(
        {id, page->second.profile, page->second.owner, join->second});
  }
  if (native.size() != pages.size())
    return Error::kNativeRestoreRequired;
  for (const auto& [id, page] : model.pages())
    if (page.attachment().window == window && !pages.count(id))
      return Error::kInvalidPresentation;
  if (record.selected_page) {
    if (!pages.count(*record.selected_page) ||
        model.pages().at(*record.selected_page).attachment().space !=
            record.selected_space)
      return Error::kInvalidPresentation;
  }
  entries_.emplace(std::move(undo), Entry{std::move(record), PendingClose{}});
  return Error::kOk;
}

Error Journal::ConfirmClose(UndoId undo, const lifecycle::Model& model) {
  auto entry = entries_.find(undo);
  if (entry == entries_.end())
    return Error::kNotFound;
  if (!std::holds_alternative<PendingClose>(entry->second.state))
    return Error::kWrongState;
  const auto& record = entry->second.record;
  if (model.windows().count(record.window))
    return Error::kCloseIncomplete;
  for (const auto& page : record.pages)
    if (model.pages().count(page.page))
      return Error::kCloseIncomplete;
  entry->second.state = Closed{};
  history_.push_back(std::move(undo));
  return Error::kOk;
}

Error Journal::CancelClose(UndoId undo) {
  auto entry = entries_.find(undo);
  if (entry == entries_.end())
    return Error::kNotFound;
  if (!std::holds_alternative<PendingClose>(entry->second.state))
    return Error::kWrongState;
  entries_.erase(entry);
  return Error::kOk;
}

ReopenResult Journal::BeginReopen(UndoId undo,
                                  AttemptId attempt,
                                  const lifecycle::Model& model,
                                  lifecycle::WindowId target,
                                  std::vector<NewPageIdentity> identities) {
  auto entry = entries_.find(undo);
  if (entry == entries_.end())
    return Error::kNotFound;
  if (!std::holds_alternative<Closed>(entry->second.state))
    return Error::kWrongState;
  const auto& record = entry->second.record;
  if (attempt.value.empty() || target.value.empty() || target == record.window)
    return Error::kIdentityConflict;
  std::map<lifecycle::PageId, NewPageIdentity> mapping;
  std::set<lifecycle::PageId> new_pages;
  std::set<lifecycle::TemporaryTabId> new_temporaries;
  std::set<lifecycle::PageId> old_pages;
  std::set<lifecycle::TemporaryTabId> old_temporaries;
  for (const auto& page : record.pages) {
    old_pages.insert(page.page);
    if (const auto* temporary =
            std::get_if<lifecycle::TemporaryTab>(&page.owner))
      old_temporaries.insert(temporary->id);
  }
  for (auto& identity : identities) {
    if (identity.new_page.value.empty() || old_pages.count(identity.new_page) ||
        !new_pages.insert(identity.new_page).second ||
        !mapping.emplace(identity.closed_page, identity).second)
      return Error::kIdentityConflict;
    if (identity.new_temporary &&
        (identity.new_temporary->value.empty() ||
         old_temporaries.count(*identity.new_temporary) ||
         !new_temporaries.insert(*identity.new_temporary).second))
      return Error::kIdentityConflict;
  }
  if (mapping.size() != record.pages.size())
    return Error::kIdentityConflict;
  ReopenPlan plan{
      target, record.spaces, {}, record.selected_space, std::nullopt};
  for (const auto& page : record.pages) {
    const auto found = mapping.find(page.page);
    if (found == mapping.end())
      return Error::kIdentityConflict;
    const auto& identity = found->second;
    auto owner = page.owner;
    if (auto* temporary = std::get_if<lifecycle::TemporaryTab>(&owner)) {
      if (!identity.new_temporary)
        return Error::kIdentityConflict;
      temporary->id = *identity.new_temporary;
      temporary->attachment.window = target;
    } else {
      if (identity.new_temporary)
        return Error::kIdentityConflict;
      std::get<lifecycle::SavedPage>(owner).attachment.window = target;
    }
    plan.pages.push_back(
        {identity.new_page, page.profile, std::move(owner), page.native});
    if (record.selected_page == page.page)
      plan.selected_page = identity.new_page;
  }
  const auto available = CheckAvailable(model, plan);
  if (available != Error::kOk)
    return available;
  entry->second.state = Reopening{std::move(attempt), plan};
  return plan;
}

Error Journal::RevalidateReopen(UndoId undo,
                                AttemptId attempt,
                                const lifecycle::Model& model) const {
  const auto entry = entries_.find(undo);
  if (entry == entries_.end())
    return Error::kNotFound;
  const auto* reopening = std::get_if<Reopening>(&entry->second.state);
  if (!reopening || reopening->attempt != attempt)
    return Error::kWrongState;
  return CheckAvailable(model, reopening->plan);
}

Error Journal::CancelReopen(UndoId undo, AttemptId attempt) {
  auto entry = entries_.find(undo);
  if (entry == entries_.end())
    return Error::kNotFound;
  const auto* reopening = std::get_if<Reopening>(&entry->second.state);
  if (!reopening || reopening->attempt != attempt)
    return Error::kWrongState;
  entry->second.state = Closed{};
  return Error::kOk;
}

Error Journal::ConfirmReopen(UndoId undo,
                             AttemptId attempt,
                             const lifecycle::Model& model,
                             Presentation observed) {
  auto entry = entries_.find(undo);
  if (entry == entries_.end())
    return Error::kNotFound;
  const auto* reopening = std::get_if<Reopening>(&entry->second.state);
  if (!reopening || reopening->attempt != attempt)
    return Error::kWrongState;
  const auto& plan = reopening->plan;
  const auto owners = CheckOwners(model, plan.spaces, plan.pages);
  if (owners != Error::kOk)
    return owners;
  const auto window = model.windows().find(plan.window);
  if (window == model.windows().end() ||
      window->second.selected_space != plan.selected_space ||
      observed.selected_page != plan.selected_page)
    return Error::kRestoreIncomplete;
  if (HasSplits(model, plan.window))
    return Error::kSplitIntegrationPending;
  std::vector<lifecycle::SpaceId> spaces;
  for (const auto& closed : plan.spaces)
    spaces.push_back(closed.space);
  std::vector<lifecycle::PageId> pages;
  for (const auto& restored : plan.pages) {
    pages.push_back(restored.page);
    const auto page = model.pages().find(restored.page);
    if (page == model.pages().end() ||
        page->second.profile != restored.profile ||
        !SameOwner(page->second.owner, restored.owner))
      return Error::kRestoreIncomplete;
  }
  if (observed.spaces != spaces || observed.pages != pages)
    return Error::kRestoreIncomplete;
  std::set<lifecycle::SpaceId> expected_spaces(spaces.begin(), spaces.end());
  for (const auto& space : spaces)
    if (!model.attachments().count({plan.window, space}))
      return Error::kRestoreIncomplete;
  for (const auto& attachment : model.attachments())
    if (attachment.window == plan.window &&
        !expected_spaces.count(attachment.space))
      return Error::kRestoreIncomplete;
  std::set<lifecycle::PageId> expected_pages(pages.begin(), pages.end());
  for (const auto& [id, page] : model.pages())
    if (page.attachment().window == plan.window && !expected_pages.count(id))
      return Error::kRestoreIncomplete;
  entries_.erase(entry);
  history_.erase(std::remove(history_.begin(), history_.end(), undo),
                 history_.end());
  return Error::kOk;
}

}  // namespace arcified::window_restore
