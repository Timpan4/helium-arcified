#ifndef ARCIFIED_NATIVE_FEATURES_COMMANDS_SITE_SEARCH_H_
#define ARCIFIED_NATIVE_FEATURES_COMMANDS_SITE_SEARCH_H_

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::commands::site_search {

using ProviderId = lifecycle::Id<struct SearchProviderTag>;
using SnapshotRevision = lifecycle::Id<struct SearchSnapshotRevisionTag>;

enum class ServiceState { kLoading, kUnavailable, kBlocked, kReady };

// Already validated/authorized by the real Profile's native search service.
// No URL/template, credentials, suggestion endpoint or persistence lives here.
struct Provider {
  ProviderId id;
  std::u16string label;
  // Empty if native supplies no discoverable shortcut.
  std::u16string shortcut;
};

struct SnapshotData {
  lifecycle::ProfileId profile;
  // Caller-allocated, nonempty and never reused, even after service recreation.
  // Allocate afresh for every snapshot refresh and service/policy/context
  // change, including changes invisible in this projection. No local timer.
  SnapshotRevision revision;
  ServiceState state;
  std::vector<Provider> providers;  // Native order; eligible providers only.
  std::optional<ProviderId> default_provider;
};

enum class SnapshotError {
  kMissingIdentity,
  kInvalidState,
  kProvidersWhileNotReady,
  kInvalidProvider,
  kDuplicateProvider,
  kUnknownDefault,
};

enum class Refusal {
  kServiceLoading,
  kServiceUnavailable,
  kPolicyBlocked,
  kSelectionRequired,
  kNoDefault,
  kProviderUnavailable,
  kCompositionActive,
  kEmptyQuery,
  kProfileChanged,
  kSnapshotChanged,
  kDefaultChanged,
  kProviderChanged,
  kQueryChanged,
};

struct ProviderChoice {
  Provider provider;
  bool is_default;
};
using DiscoveryResult = std::variant<Refusal, std::vector<ProviderChoice>>;

struct DefaultProvider {};
using ProviderTarget = std::variant<DefaultProvider, ProviderId>;

enum class CompositionState { kCommitted, kComposing };
struct Input {
  // Literal text, not an omnibox URL/query classification.
  std::u16string query;
  CompositionState composition;
};

class ProviderSnapshot;

// Only an explicit Select action creates this owned preview. Discovery never
// selects. Editing the query requires a fresh preview before submission.
class Selection {
 public:
  const lifecycle::ProfileId& profile() const { return profile_; }
  const SnapshotRevision& revision() const { return revision_; }
  const Provider& provider() const { return provider_; }
  const std::u16string& query() const { return query_; }
  bool uses_default() const;

 private:
  friend class ProviderSnapshot;
  Selection(lifecycle::ProfileId profile,
            SnapshotRevision revision,
            ProviderTarget target,
            Provider provider,
            std::u16string query);

  lifecycle::ProfileId profile_;
  SnapshotRevision revision_;
  ProviderTarget target_;
  Provider provider_;
  std::u16string query_;
};
using SelectionResult = std::variant<Refusal, Selection>;

// A decision for a native adapter, never a URL, navigation or completion ack.
// Revalidate against the actual native service/policy/context before expansion.
struct SearchIntent {
  lifecycle::ProfileId profile;
  SnapshotRevision revision;
  Provider provider;
  std::u16string query;
};
using SubmitResult = std::variant<Refusal, SearchIntent>;

// Immutable, transient projection of one authoritative native service snapshot.
// Structural validation here does not validate native URLs or managed policy.
class ProviderSnapshot {
 public:
  static std::variant<SnapshotError, ProviderSnapshot> FromNative(
      SnapshotData data);

  // Empty prefix lists all providers. Nonempty prefix uses literal UTF-16
  // shortcut matching, retains native order and never picks a winner. Native
  // omnibox normalization, classification and keyword conflict resolution stay
  // with TemplateURLService. No suggestion/fetch callback exists in this core.
  DiscoveryResult Discover(const std::u16string& shortcut_prefix) const;
  SelectionResult Select(ProviderTarget target, const Input& input) const;
  SubmitResult Submit(const std::optional<Selection>& selection,
                      const Input& current_input) const;

 private:
  explicit ProviderSnapshot(SnapshotData data);
  std::optional<Refusal> Unavailable() const;
  const Provider* FindProvider(const ProviderId& id) const;

  SnapshotData data_;
};

}  // namespace arcified::commands::site_search

#endif  // ARCIFIED_NATIVE_FEATURES_COMMANDS_SITE_SEARCH_H_
