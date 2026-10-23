#include "routing.h"

#include <algorithm>
#include <utility>

namespace arcified::air_traffic {
namespace {

bool ValidTarget(const Target& target) {
  return std::visit([](const auto& id) { return !id.value.empty(); }, target);
}

std::optional<ResolvedTarget> Resolve(const lifecycle::Model& model,
                                      const Destination& destination) {
  if (const auto* space =
          std::get_if<lifecycle::SpaceId>(&destination.target)) {
    const auto found = model.spaces().find(*space);
    if (found == model.spaces().end())
      return std::nullopt;
    return ResolvedTarget{found->second.profile, *space, destination.surface};
  }
  const auto& profile = std::get<lifecycle::ProfileId>(destination.target);
  if (!model.profiles().count(profile))
    return std::nullopt;
  return ResolvedTarget{profile, std::nullopt, destination.surface};
}

bool Matches(const Predicate& predicate, const BrowserFacts& browser) {
  const std::string* value = nullptr;
  switch (predicate.field) {
    case MatchField::kCanonicalHost: {
      const auto* host = std::get_if<CanonicalHost>(&browser.host);
      if (!host)
        return false;
      value = &host->value;
      break;
    }
    case MatchField::kCanonicalUrl:
      value = &browser.canonical_url;
      break;
  }
  switch (predicate.mode) {
    case MatchMode::kExact:
      return *value == predicate.literal;
    case MatchMode::kContains:
      return value->find(predicate.literal) != std::string::npos;
  }
  return false;
}

Decision Decide(const lifecycle::Model& model,
                const BrowserFacts& browser,
                const Destination& destination) {
  auto resolved = Resolve(model, destination);
  if (!resolved)
    return Refusal::kTargetDeleted;
  const auto privacy = browser.privacy_for
                           ? browser.privacy_for(resolved->profile)
                           : Privacy::kUnknown;
  switch (privacy) {
    case Privacy::kUnknown:
      return Refusal::kPrivacyUnknown;
    case Privacy::kRegular:
    case Privacy::kPrivate:
      break;
  }
  Capability capability = Capability::kUnknown;
  switch (destination.surface) {
    case Surface::kMainWindow:
      capability = browser.main_window;
      break;
    case Surface::kLightweightWindow:
      capability = browser.lightweight_window;
      break;
  }
  switch (capability) {
    case Capability::kUnknown:
      return Refusal::kCapabilityUnknown;
    case Capability::kUnavailable:
      return Refusal::kCapabilityUnavailable;
    case Capability::kAvailable:
      break;
  }
  if (privacy == Privacy::kPrivate)
    return PrivateConfirmationRequired{std::move(*resolved)};
  return std::move(*resolved);
}

}  // namespace

ConfigStatus RuleBook::Insert(std::size_t position, Rule rule) {
  if (rule.id.value.empty() || rule.predicate.literal.empty())
    return ConfigStatus::kInvalidRule;
  if (!ValidTarget(rule.destination.target))
    return ConfigStatus::kInvalidTarget;
  if (std::any_of(rules_.begin(), rules_.end(),
                  [&](const Rule& other) { return other.id == rule.id; }))
    return ConfigStatus::kDuplicateRule;
  if (position > rules_.size())
    return ConfigStatus::kInvalidPosition;
  rules_.insert(rules_.begin() + position, std::move(rule));
  return ConfigStatus::kOk;
}

ConfigStatus RuleBook::Edit(Rule rule) {
  if (rule.id.value.empty() || rule.predicate.literal.empty())
    return ConfigStatus::kInvalidRule;
  if (!ValidTarget(rule.destination.target))
    return ConfigStatus::kInvalidTarget;
  const auto found =
      std::find_if(rules_.begin(), rules_.end(),
                   [&](const Rule& other) { return other.id == rule.id; });
  if (found == rules_.end())
    return ConfigStatus::kRuleNotFound;
  *found = std::move(rule);
  return ConfigStatus::kOk;
}

ConfigStatus RuleBook::Move(RuleId rule, std::size_t final_position) {
  const auto found =
      std::find_if(rules_.begin(), rules_.end(),
                   [&](const Rule& other) { return other.id == rule; });
  if (found == rules_.end())
    return ConfigStatus::kRuleNotFound;
  if (final_position >= rules_.size())
    return ConfigStatus::kInvalidPosition;
  const auto destination = rules_.begin() + final_position;
  if (found < destination)
    std::rotate(found, found + 1, destination + 1);
  else if (found > destination)
    std::rotate(destination, found, found + 1);
  return ConfigStatus::kOk;
}

ConfigStatus RuleBook::Erase(RuleId rule) {
  const auto found =
      std::find_if(rules_.begin(), rules_.end(),
                   [&](const Rule& other) { return other.id == rule; });
  if (found == rules_.end())
    return ConfigStatus::kRuleNotFound;
  rules_.erase(found);
  return ConfigStatus::kOk;
}

ConfigStatus RuleBook::SetFallback(Destination destination) {
  if (!ValidTarget(destination.target))
    return ConfigStatus::kInvalidTarget;
  fallback_ = std::move(destination);
  return ConfigStatus::kOk;
}

void RuleBook::ClearFallback() {
  fallback_.reset();
}

Preview RuleBook::PreviewExternal(
    const lifecycle::Model& model,
    const BrowserFacts& browser,
    EntryPoint entry,
    std::optional<Destination> one_time_override) const {
  Preview preview{{}, std::nullopt, Refusal::kOutsideExternalEntry};
  switch (entry) {
    case EntryPoint::kPeek:
    case EntryPoint::kArchiveRestore:
    case EntryPoint::kOther:
      return preview;
    case EntryPoint::kExternalMainWindow:
    case EntryPoint::kExternalLightweightWindow:
      break;
  }
  switch (browser.authorization) {
    case UrlAuthorization::kUnknown:
      preview.decision = Refusal::kUrlAuthorizationRequired;
      return preview;
    case UrlAuthorization::kRejected:
      preview.decision = Refusal::kUrlRejected;
      return preview;
    case UrlAuthorization::kAllowed:
      break;
  }
  if (browser.canonical_url.empty()) {
    preview.decision = Refusal::kEmptyCanonicalUrl;
    return preview;
  }
  if (std::holds_alternative<UnknownHost>(browser.host)) {
    preview.decision = Refusal::kHostClassificationRequired;
    return preview;
  }
  const auto* host = std::get_if<CanonicalHost>(&browser.host);
  if (host && host->value.empty()) {
    preview.decision = Refusal::kEmptyCanonicalHost;
    return preview;
  }
  for (const auto& rule : rules_) {
    if (Matches(rule.predicate, browser)) {
      preview.matches.push_back({rule.id, rule.destination,
                                 Resolve(model, rule.destination).has_value()});
    }
  }
  if (one_time_override) {
    preview.selected =
        Candidate{std::move(*one_time_override), OneTimeSelection{}};
  } else if (!preview.matches.empty()) {
    const auto& first = preview.matches.front();
    preview.selected = Candidate{first.destination, first.rule};
  } else if (fallback_) {
    preview.selected = Candidate{*fallback_, FallbackSelection{}};
  } else {
    preview.decision = Refusal::kNoFallback;
    return preview;
  }
  preview.decision = Decide(model, browser, preview.selected->destination);
  return preview;
}

PruneReport RuleBook::PruneDeletedTargets(const lifecycle::Model& model) {
  PruneReport report;
  auto rule = rules_.begin();
  while (rule != rules_.end()) {
    if (Resolve(model, rule->destination)) {
      ++rule;
    } else {
      report.removed_rules.push_back(rule->id);
      rule = rules_.erase(rule);
    }
  }
  if (fallback_ && !Resolve(model, *fallback_)) {
    report.removed_fallback = std::move(fallback_);
    fallback_.reset();
  }
  return report;
}

}  // namespace arcified::air_traffic
