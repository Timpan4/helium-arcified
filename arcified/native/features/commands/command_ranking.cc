#include "command_ranking.h"

#include <algorithm>
#include <utility>

namespace arcified::commands {
namespace {

using namespace lifecycle;

template <typename Id>
std::string Label(const std::map<Id, std::string>& labels,
                  const Id& id,
                  const std::string& fallback) {
  const auto found = labels.find(id);
  return found == labels.end() || found->second.empty() ? fallback
                                                        : found->second;
}

std::string FoldAscii(std::string_view text) {
  std::string folded(text);
  for (char& character : folded)
    if (character >= 'A' && character <= 'Z')
      character = static_cast<char>(character - 'A' + 'a');
  return folded;
}

void MatchField(std::string_view text,
                const std::string& query,
                std::optional<RankingMatch>& best) {
  const auto folded = FoldAscii(text);
  const auto position = folded.find(query);
  if (position == std::string::npos)
    return;
  const auto match = folded == query ? RankingMatch::kExact
                     : position == 0 ? RankingMatch::kPrefix
                                     : RankingMatch::kSubstring;
  if (!best || match < *best)
    best = match;
}

void SetLiveLabels(RankedCommand& command,
                   const RankedLivePage& live,
                   const RankingDisplayLabels& labels) {
  command.provenance.live_space =
      Label(labels.spaces, live.attachment.space, live.attachment.space.value);
  command.provenance.live_window = Label(labels.windows, live.attachment.window,
                                         live.attachment.window.value);
}

const RankedLivePage* Live(const RankingRevealTarget& target) {
  if (const auto* saved = std::get_if<RevealSavedItem>(&target))
    return saved->live ? &*saved->live : nullptr;
  return &std::get<RevealTemporaryPage>(target).live;
}

}  // namespace

std::string_view RankedVerbLabel(RankedVerb verb) {
  switch (verb) {
    case RankedVerb::kSwitch:
      return "Switch";
    case RankedVerb::kOpenSavedDestination:
      return "Open saved destination";
  }
  return {};
}

RankingIdentity RankedCommand::identity() const {
  if (const auto* saved = std::get_if<RevealSavedItem>(&reveal))
    return saved->item;
  return std::get<RevealTemporaryPage>(reveal).tab;
}

RankedVerb RankedCommand::verb() const {
  return Live(reveal) ? RankedVerb::kSwitch : RankedVerb::kOpenSavedDestination;
}

RankingResult RankCommands(const lifecycle::Model& model,
                           const RankingScope& scope,
                           const RankingDisplayLabels& labels,
                           std::string_view query) {
  // Validate scope before reading display data or constructing any row.
  if (scope.privacy != RankingPrivacy::kRegular)
    return RankingError::kPrivacyExcluded;
  if (!model.profiles().count(scope.profile))
    return RankingError::kProfileNotFound;
  if (!model.attachments().count(scope.caller))
    return RankingError::kCallerNotAttached;
  if (model.spaces().at(scope.caller.space).profile != scope.profile)
    return RankingError::kProfileMismatch;

  std::vector<RankedCommand> candidates;
  for (const auto& [id, item] : model.items()) {
    const auto* owning_space = std::get_if<SpaceId>(&item.owner);
    const auto& profile = owning_space
                              ? model.spaces().at(*owning_space).profile
                              : std::get<ProfileId>(item.owner);
    if (profile != scope.profile)
      continue;
    RankingProvenanceLabels provenance{
        Label(labels.profiles, profile, profile.value), std::nullopt,
        std::nullopt, std::nullopt};
    if (owning_space)
      provenance.owning_space =
          Label(labels.spaces, *owning_space, owning_space->value);
    candidates.push_back({profile,
                          RevealSavedItem{id, item.owner, std::nullopt},
                          Label(labels.items, id, item.saved_destination),
                          std::move(provenance), RankingMatch::kEmptyQuery});
  }

  // The output projection is still ItemId-ordered here. Join existing pages
  // directly onto it, without retaining a second item/page index or store.
  for (const auto& [id, page] : model.pages()) {
    if (page.profile != scope.profile)
      continue;
    const auto* saved = std::get_if<SavedPage>(&page.owner);
    if (!saved)
      continue;
    auto row = std::lower_bound(
        candidates.begin(), candidates.end(), saved->item,
        [](const RankedCommand& command, const ItemId& item) {
          return std::get<RevealSavedItem>(command.reveal).item < item;
        });
    // Every saved page references an item in this Profile by model invariant.
    auto& target = std::get<RevealSavedItem>(row->reveal);
    target.live = RankedLivePage{id, page.attachment()};
    const auto& item = model.items().at(target.item);
    row->title = Label(labels.items, target.item,
                       Label(labels.pages, id, item.saved_destination));
    SetLiveLabels(*row, *target.live, labels);
  }

  for (const auto& [id, page] : model.pages()) {
    if (page.profile != scope.profile ||
        page.attachment().window != scope.caller.window)
      continue;
    const auto* temporary = std::get_if<TemporaryTab>(&page.owner);
    if (!temporary)
      continue;
    RankedLivePage live{id, page.attachment()};
    RankedCommand command{
        page.profile,
        RevealTemporaryPage{temporary->id, live},
        Label(labels.pages, id, page.current_navigation),
        {Label(labels.profiles, page.profile, page.profile.value), std::nullopt,
         std::nullopt, std::nullopt},
        RankingMatch::kEmptyQuery};
    SetLiveLabels(command, live, labels);
    candidates.push_back(std::move(command));
  }

  const auto folded_query = FoldAscii(query);
  std::vector<RankedCommand> matches;
  for (auto& command : candidates) {
    std::optional<RankingMatch> best;
    if (folded_query.empty()) {
      best = RankingMatch::kEmptyQuery;
    } else {
      // Provenance labels/IDs do not become search fields. In particular,
      // caller metadata for excluded Profiles/pages never enters matching.
      if (const auto* saved = std::get_if<RevealSavedItem>(&command.reveal)) {
        const auto& item = model.items().at(saved->item);
        MatchField(Label(labels.items, saved->item, item.saved_destination),
                   folded_query, best);
        MatchField(item.saved_destination, folded_query, best);
      }
      if (const auto* live = Live(command.reveal)) {
        const auto& page = model.pages().at(live->page);
        MatchField(Label(labels.pages, live->page, page.current_navigation),
                   folded_query, best);
        MatchField(page.current_navigation, folded_query, best);
      }
    }
    if (best) {
      command.match = *best;
      matches.push_back(std::move(command));
    }
  }
  std::sort(matches.begin(), matches.end(), [](const auto& a, const auto& b) {
    if (a.match != b.match)
      return a.match < b.match;
    return a.identity() < b.identity();
  });
  return matches;
}

}  // namespace arcified::commands
