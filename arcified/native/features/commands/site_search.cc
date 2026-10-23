#include "site_search.h"

#include <algorithm>
#include <set>
#include <utility>

namespace arcified::commands::site_search {

Selection::Selection(lifecycle::ProfileId profile,
                     SnapshotRevision revision,
                     ProviderTarget target,
                     Provider provider,
                     std::u16string query)
    : profile_(std::move(profile)),
      revision_(std::move(revision)),
      target_(std::move(target)),
      provider_(std::move(provider)),
      query_(std::move(query)) {}

bool Selection::uses_default() const {
  return std::holds_alternative<DefaultProvider>(target_);
}

ProviderSnapshot::ProviderSnapshot(SnapshotData data)
    : data_(std::move(data)) {}

std::variant<SnapshotError, ProviderSnapshot> ProviderSnapshot::FromNative(
    SnapshotData data) {
  if (data.profile.value.empty() || data.revision.value.empty())
    return SnapshotError::kMissingIdentity;
  switch (data.state) {
    case ServiceState::kLoading:
    case ServiceState::kUnavailable:
    case ServiceState::kBlocked:
      if (!data.providers.empty() || data.default_provider)
        return SnapshotError::kProvidersWhileNotReady;
      return ProviderSnapshot(std::move(data));
    case ServiceState::kReady:
      break;
    default:
      return SnapshotError::kInvalidState;
  }
  std::set<ProviderId> ids;
  for (const auto& provider : data.providers) {
    if (provider.id.value.empty() || provider.label.empty())
      return SnapshotError::kInvalidProvider;
    if (!ids.insert(provider.id).second)
      return SnapshotError::kDuplicateProvider;
  }
  if (data.default_provider && !ids.count(*data.default_provider))
    return SnapshotError::kUnknownDefault;
  return ProviderSnapshot(std::move(data));
}

std::optional<Refusal> ProviderSnapshot::Unavailable() const {
  switch (data_.state) {
    case ServiceState::kLoading:
      return Refusal::kServiceLoading;
    case ServiceState::kUnavailable:
      return Refusal::kServiceUnavailable;
    case ServiceState::kBlocked:
      return Refusal::kPolicyBlocked;
    case ServiceState::kReady:
      return std::nullopt;
  }
  return Refusal::kServiceUnavailable;
}

const Provider* ProviderSnapshot::FindProvider(const ProviderId& id) const {
  auto found =
      std::find_if(data_.providers.begin(), data_.providers.end(),
                   [&id](const auto& provider) { return provider.id == id; });
  return found == data_.providers.end() ? nullptr : &*found;
}

DiscoveryResult ProviderSnapshot::Discover(
    const std::u16string& shortcut_prefix) const {
  if (auto refusal = Unavailable())
    return *refusal;
  std::vector<ProviderChoice> choices;
  for (const auto& provider : data_.providers) {
    if (provider.shortcut.compare(0, shortcut_prefix.size(), shortcut_prefix) ==
        0) {
      choices.push_back({provider, data_.default_provider == provider.id});
    }
  }
  return choices;
}

SelectionResult ProviderSnapshot::Select(ProviderTarget target,
                                         const Input& input) const {
  if (auto refusal = Unavailable())
    return *refusal;
  if (input.composition == CompositionState::kComposing)
    return Refusal::kCompositionActive;
  if (input.query.empty())
    return Refusal::kEmptyQuery;
  const ProviderId* id = std::get_if<ProviderId>(&target);
  if (!id) {
    if (!data_.default_provider)
      return Refusal::kNoDefault;
    id = &*data_.default_provider;
  }
  const auto* provider = FindProvider(*id);
  if (!provider)
    return Refusal::kProviderUnavailable;
  return Selection(data_.profile, data_.revision, std::move(target), *provider,
                   input.query);
}

SubmitResult ProviderSnapshot::Submit(const std::optional<Selection>& selection,
                                      const Input& current_input) const {
  if (auto refusal = Unavailable())
    return *refusal;
  if (current_input.composition == CompositionState::kComposing)
    return Refusal::kCompositionActive;
  if (!selection)
    return Refusal::kSelectionRequired;
  if (selection->profile() != data_.profile)
    return Refusal::kProfileChanged;
  if (selection->revision() != data_.revision)
    return Refusal::kSnapshotChanged;
  if (selection->uses_default() &&
      data_.default_provider != selection->provider().id) {
    return Refusal::kDefaultChanged;
  }
  const auto* provider = FindProvider(selection->provider().id);
  if (!provider)
    return Refusal::kProviderUnavailable;
  if (provider->label != selection->provider().label ||
      provider->shortcut != selection->provider().shortcut) {
    return Refusal::kProviderChanged;
  }
  if (selection->query() != current_input.query)
    return Refusal::kQueryChanged;
  return SearchIntent{data_.profile, data_.revision, *provider,
                      current_input.query};
}

}  // namespace arcified::commands::site_search
