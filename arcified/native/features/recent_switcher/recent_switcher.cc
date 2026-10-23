#include "recent_switcher.h"

#include <algorithm>
#include <set>
#include <utility>

namespace arcified::recent_switcher {
namespace {

std::size_t Step(std::size_t index, std::size_t size, Direction direction) {
  if (direction == Direction::kForward)
    return index == size - 1 ? 0 : index + 1;
  return index == 0 ? size - 1 : index - 1;
}

// Switcher eligibility only, not the shared command-target resolver. Match
// OpenItem's requesting-attachment/Profile check before resolving a kept owner.
std::optional<Candidate> Project(const lifecycle::Model& model,
                                 const Snapshot& snapshot,
                                 const HistoryEntry& entry) {
  const auto live = model.pages().find(entry.page);
  const lifecycle::Attachment requested{snapshot.window, entry.requested_space};
  if (live == model.pages().end() || !model.attachments().count(requested))
    return std::nullopt;
  const auto& page = live->second;
  if (model.spaces().at(requested.space).profile != page.profile)
    return std::nullopt;

  lifecycle::Attachment destination = requested;
  if (const auto* saved = std::get_if<lifecycle::SavedPage>(&page.owner)) {
    const auto& owner = model.items().at(saved->item).owner;
    if (const auto* space = std::get_if<lifecycle::SpaceId>(&owner))
      destination.space = *space;
  } else if (!(page.attachment() == requested)) {
    // Temporary pages never leave their original window/Space.
    return std::nullopt;
  }

  if (!model.attachments().count(destination) ||
      std::find(snapshot.eligible_spaces.begin(),
                snapshot.eligible_spaces.end(),
                destination.space) == snapshot.eligible_spaces.end()) {
    return std::nullopt;
  }
  std::optional<Preview> preview;
  const auto metadata = snapshot.previews.find(entry.page);
  if (metadata != snapshot.previews.end())
    preview = metadata->second;
  return Candidate{entry, page.profile, page.attachment(),
                   std::move(destination), std::move(preview)};
}

}  // namespace

Update Session::Begin(const lifecycle::Model& model,
                      const Snapshot& snapshot,
                      Direction direction) {
  if (held_) {
    const auto update = Reconcile(model, snapshot);
    return update == Update::kHolding ? Update::kAlreadyHolding : update;
  }
  if (!model.windows().count(snapshot.window))
    return Update::kContextChanged;

  std::vector<Candidate> candidates;
  std::set<lifecycle::PageId> seen;
  for (const auto& entry : snapshot.history) {
    if (!seen.insert(entry.page).second)
      continue;
    if (auto candidate = Project(model, snapshot, entry))
      candidates.push_back(std::move(*candidate));
  }
  if (candidates.empty())
    return Update::kNoCandidates;

  std::size_t selection =
      direction == Direction::kForward ? 0 : candidates.size() - 1;
  for (std::size_t index = 0; index < candidates.size(); ++index) {
    if (snapshot.focused_page == candidates[index].entry.page) {
      selection = Step(index, candidates.size(), direction);
      break;
    }
  }
  held_ = Held{snapshot.window, snapshot.context_generation, direction,
               std::move(candidates), selection};
  return Update::kHolding;
}

Update Session::Reconcile(const lifecycle::Model& model,
                          const Snapshot& snapshot) {
  if (!held_)
    return Update::kIdle;
  if (held_->window != snapshot.window ||
      held_->context_generation != snapshot.context_generation ||
      !model.windows().count(snapshot.window)) {
    Cancel();
    return Update::kContextChanged;
  }

  // Freeze history and requested destinations for this hold. Refresh only
  // liveness, permitted scope, attachment/Profile and caller display metadata.
  std::vector<Candidate> candidates;
  std::map<lifecycle::PageId, std::size_t> surviving;
  for (const auto& candidate : held_->candidates) {
    if (auto current = Project(model, snapshot, candidate.entry)) {
      surviving.emplace(current->entry.page, candidates.size());
      candidates.push_back(std::move(*current));
    }
  }
  if (candidates.empty()) {
    Cancel();
    return Update::kNoCandidates;
  }

  // Search from the old selected identity in the last cycle direction. If it
  // survives, keep it even when preceding rows, splits or attachments changed.
  auto index = held_->selection;
  while (!surviving.count(held_->candidates[index].entry.page))
    index = Step(index, held_->candidates.size(), held_->direction);
  held_->selection = surviving.at(held_->candidates[index].entry.page);
  held_->candidates = std::move(candidates);
  return Update::kHolding;
}

Update Session::Cycle(const lifecycle::Model& model,
                      const Snapshot& snapshot,
                      Direction direction) {
  if (!held_)
    return Update::kIdle;
  const auto previous = selected()->entry.page;
  held_->direction = direction;
  const auto update = Reconcile(model, snapshot);
  if (update != Update::kHolding)
    return update;
  // A lost selection already advances to a survivor in this direction.
  if (selected()->entry.page == previous)
    held_->selection =
        Step(held_->selection, held_->candidates.size(), direction);
  return Update::kHolding;
}

ReleaseResult Session::Release(const lifecycle::Model& model,
                               const Snapshot& snapshot) {
  if (!held_)
    return EndReason::kIdle;
  const auto previous = selected()->entry.page;
  const auto update = Reconcile(model, snapshot);
  if (update == Update::kContextChanged)
    return EndReason::kContextChanged;
  if (update == Update::kNoCandidates)
    return EndReason::kNoCandidates;

  const auto candidate = *selected();
  const auto generation = held_->context_generation;
  Cancel();
  if (candidate.entry.page != previous)
    return EndReason::kSelectionChanged;
  if (snapshot.focused_page == candidate.entry.page &&
      candidate.source == candidate.destination) {
    return EndReason::kAlreadyFocused;
  }
  return ActivationIntent{candidate.entry, candidate.source,
                          candidate.destination, generation};
}

void Session::Cancel() {
  held_.reset();
}

const std::vector<Candidate>& Session::candidates() const {
  static const std::vector<Candidate> empty;
  return held_ ? held_->candidates : empty;
}

const Candidate* Session::selected() const {
  return held_ ? &held_->candidates[held_->selection] : nullptr;
}

}  // namespace arcified::recent_switcher
