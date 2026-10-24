#ifndef ARCIFIED_NATIVE_FEATURES_COMMANDS_COMMAND_RANKING_H_
#define ARCIFIED_NATIVE_FEATURES_COMMANDS_COMMAND_RANKING_H_

#include "../../model/lifecycle.h"

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace arcified::commands {

// Supplied by the native BrowserContext boundary, never inferred from labels,
// URLs, original-Profile pointers or a selected window's display name.
enum class RankingPrivacy { kUnknown, kRegular, kPrivate };
struct RankingScope {
  lifecycle::ProfileId profile;
  lifecycle::Attachment caller;
  RankingPrivacy privacy = RankingPrivacy::kUnknown;
};

// Ephemeral display metadata. Missing/empty labels fall back to lifecycle
// destination text or opaque provenance IDs. Labels never grant visibility.
struct RankingDisplayLabels {
  std::map<lifecycle::ProfileId, std::string> profiles;
  std::map<lifecycle::SpaceId, std::string> spaces;
  std::map<lifecycle::WindowId, std::string> windows;
  std::map<lifecycle::ItemId, std::string> items;
  std::map<lifecycle::PageId, std::string> pages;
};

struct RankedLivePage {
  lifecycle::PageId page;
  lifecycle::Attachment attachment;
};

struct RevealSavedItem {
  lifecycle::ItemId item;
  // Profile-owned shortcuts have no owning Space. A live page's attachment
  // describes its current location, never a replacement for this owner.
  lifecycle::ItemOwner owner;
  std::optional<RankedLivePage> live;
};

struct RevealTemporaryPage {
  lifecycle::TemporaryTabId tab;
  RankedLivePage live;
};

// Also the selection payload. The executor must revalidate against the current
// model/native registry. This is a snapshot, not authority to navigate or move.
using RankingRevealTarget = std::variant<RevealSavedItem, RevealTemporaryPage>;
using RankingIdentity =
    std::variant<lifecycle::ItemId, lifecycle::TemporaryTabId>;

enum class RankedVerb { kSwitch, kOpenSavedDestination };
std::string_view RankedVerbLabel(RankedVerb verb);
inline constexpr std::string_view kRankingRevealLabel = "Reveal in Workspace";

// Declaration order is the documented ordering, not a weighted score.
enum class RankingMatch { kExact, kPrefix, kSubstring, kEmptyQuery };

struct RankingProvenanceLabels {
  std::string profile;
  std::optional<std::string> owning_space;
  std::optional<std::string> live_space;
  std::optional<std::string> live_window;
};

struct RankedCommand {
  lifecycle::ProfileId profile;
  RankingRevealTarget reveal;
  std::string title;
  RankingProvenanceLabels provenance;
  RankingMatch match;

  // Stable across label/navigation updates and saved close/reopen/transfer.
  // Keeping a temporary page deliberately changes its lifecycle identity.
  RankingIdentity identity() const;
  RankedVerb verb() const;
};

enum class RankingError {
  kPrivacyExcluded,
  kProfileNotFound,
  kCallerNotAttached,
  kProfileMismatch,
};
using RankingResult = std::variant<RankingError, std::vector<RankedCommand>>;

// Pure per-call projection of one explicitly regular Profile. Includes its
// saved items across Spaces, even without an attachment in the caller window.
// Temporary pages must additionally belong to that window. No truncation,
// persistent index, navigation store, lifecycle mutation or native URL policy.
//
// The caller serializes model writes and obtains authoritative privacy scope.
// OTR/guest/isolated contexts must have distinct lifecycle Profile IDs; the
// model itself cannot classify privacy. Private/unknown requests are refused.
//
// Matching folds ASCII case only, preserving other UTF-8 bytes. All query
// bytes must match contiguously in a label or destination. No URL parsing,
// fuzzy scoring, locale collation, MRU or selection stability is implied.
RankingResult RankCommands(const lifecycle::Model& model,
                           const RankingScope& scope,
                           const RankingDisplayLabels& labels,
                           std::string_view query);

}  // namespace arcified::commands

#endif  // ARCIFIED_NATIVE_FEATURES_COMMANDS_COMMAND_RANKING_H_
