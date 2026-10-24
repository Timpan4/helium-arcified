#ifndef ARCIFIED_NATIVE_FEATURES_AIR_TRAFFIC_ROUTING_H_
#define ARCIFIED_NATIVE_FEATURES_AIR_TRAFFIC_ROUTING_H_

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::air_traffic {

using RuleId = lifecycle::Id<struct RoutingRuleTag>;
using Target = std::variant<lifecycle::SpaceId, lifecycle::ProfileId>;

enum class Surface { kMainWindow, kLightweightWindow };
struct Destination {
  Target target;
  Surface surface = Surface::kMainWindow;
};

enum class MatchField { kCanonicalHost, kCanonicalUrl };
enum class MatchMode { kExact, kContains };
struct Predicate {
  MatchField field;
  MatchMode mode;
  // Literal bytes in the browser's canonical representation. Not a URL policy.
  std::string literal;
};
struct Rule {
  RuleId id;
  Predicate predicate;
  Destination destination;
};

enum class ConfigStatus {
  kOk,
  kInvalidRule,
  kInvalidTarget,
  kDuplicateRule,
  kRuleNotFound,
  kInvalidPosition,
};

// Caller reports provenance. Peek, restores and ordinary browser navigation
// must never be relabeled as an external handoff to enter this router.
enum class EntryPoint {
  kExternalMainWindow,
  kExternalLightweightWindow,
  kPeek,
  kArchiveRestore,
  kOther,
};
enum class UrlAuthorization { kUnknown, kRejected, kAllowed };
enum class Privacy { kUnknown, kRegular, kPrivate };
enum class Capability { kUnknown, kUnavailable, kAvailable };
struct UnknownHost {};
struct NoHost {};
struct CanonicalHost {
  std::string value;
};

// Ephemeral browser-authoritative facts for this exact external event. The
// caller, not this component, validates URL policy and canonicalizes URL/host
// and rule literals. NoHost is an affirmative browser classification.
struct BrowserFacts {
  UrlAuthorization authorization = UrlAuthorization::kUnknown;
  std::string canonical_url;
  std::variant<UnknownHost, NoHost, CanonicalHost> host = UnknownHost{};
  Capability main_window = Capability::kUnknown;
  Capability lightweight_window = Capability::kUnknown;
  std::function<Privacy(const lifecycle::ProfileId&)> privacy_for;
};

struct FallbackSelection {};
struct OneTimeSelection {};
using Selection = std::variant<RuleId, FallbackSelection, OneTimeSelection>;
struct Candidate {
  Destination destination;
  Selection selection;
};
struct MatchPreview {
  RuleId rule;
  Destination destination;
  bool target_exists;
};

// A routing choice only, not a prepared attachment or navigation permission.
// A Profile destination intentionally leaves Space selection to the caller.
struct ResolvedTarget {
  lifecycle::ProfileId profile;
  std::optional<lifecycle::SpaceId> space;
  Surface surface;
};
struct PrivateConfirmationRequired {
  ResolvedTarget choice;
};
enum class Refusal {
  kOutsideExternalEntry,
  kUrlAuthorizationRequired,
  kUrlRejected,
  kEmptyCanonicalUrl,
  kHostClassificationRequired,
  kEmptyCanonicalHost,
  kNoFallback,
  kTargetDeleted,
  kPrivacyUnknown,
  kCapabilityUnknown,
  kCapabilityUnavailable,
};
using Decision =
    std::variant<Refusal, PrivateConfirmationRequired, ResolvedTarget>;
struct Preview {
  // All literal matches in precedence order, including shadowed/deleted ones.
  std::vector<MatchPreview> matches;
  std::optional<Candidate> selected;
  Decision decision;
};

struct PruneReport {
  std::vector<RuleId> removed_rules;
  std::optional<Destination> removed_fallback;
};

// In-memory rule configuration only. No lifecycle ownership, page state,
// browser objects, navigation, persistence, private launch or fallback retry.
class RuleBook {
 public:
  // First matching rule wins. Position is the zero-based precedence index.
  ConfigStatus Insert(std::size_t position, Rule rule);
  ConfigStatus Edit(Rule rule);  // Retains the rule's precedence position.
  ConfigStatus Move(RuleId rule, std::size_t final_position);
  ConfigStatus Erase(RuleId rule);
  ConfigStatus SetFallback(Destination destination);
  void ClearFallback();

  // Reads authoritative Space/Profile existence and mapping without copying
  // ownership. A stale selected target blocks; it never falls through.
  Preview PreviewExternal(
      const lifecycle::Model& model,
      const BrowserFacts& browser,
      EntryPoint entry,
      std::optional<Destination> one_time_override = std::nullopt) const;

  // Explicit configuration cleanup. Reports removals and never changes model.
  PruneReport PruneDeletedTargets(const lifecycle::Model& model);

  const auto& rules() const { return rules_; }
  const auto& fallback() const { return fallback_; }

 private:
  std::vector<Rule> rules_;
  std::optional<Destination> fallback_;
};

}  // namespace arcified::air_traffic

#endif  // ARCIFIED_NATIVE_FEATURES_AIR_TRAFFIC_ROUTING_H_
