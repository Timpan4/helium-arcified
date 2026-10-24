#include "routing.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <utility>

namespace arcified::air_traffic {
namespace {

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

using lifecycle::ProfileId;
using lifecycle::SpaceId;
using lifecycle::Status;
static_assert(!std::is_convertible_v<ProfileId, SpaceId>);
static_assert(!std::is_convertible_v<RuleId, ProfileId>);
static_assert(lifecycle::kContractVersion == 3);

const ProfileId kWork{"work"};
const ProfileId kPersonal{"personal"};
const ProfileId kPrivate{"private-context"};
const SpaceId kDesk{"desk"};
const SpaceId kHome{"home"};
const RuleId kExact{"exact"};
const RuleId kContains{"contains"};
const RuleId kHost{"host"};
const std::string kUrl = "https://docs.example.test/guide?topic=routing";

lifecycle::Model Workspace() {
  lifecycle::Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kPersonal) == Status::kOk);
  REQUIRE(model.AddProfile(kPrivate) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kHome, kPersonal) == Status::kOk);
  return model;
}

BrowserFacts Browser() {
  BrowserFacts browser;
  browser.authorization = UrlAuthorization::kAllowed;
  browser.canonical_url = kUrl;
  browser.host = CanonicalHost{"docs.example.test"};
  browser.main_window = Capability::kAvailable;
  // Lightweight is deliberately unknown in the main-window fixture.
  browser.privacy_for = [](const ProfileId& profile) {
    return profile == kPrivate ? Privacy::kPrivate : Privacy::kRegular;
  };
  return browser;
}

Rule UrlRule(RuleId id, MatchMode mode, std::string literal, Target target) {
  return {std::move(id),
          {MatchField::kCanonicalUrl, mode, std::move(literal)},
          {std::move(target)}};
}

Preview Main(const RuleBook& rules,
             const lifecycle::Model& model,
             const BrowserFacts& browser) {
  return rules.PreviewExternal(model, browser, EntryPoint::kExternalMainWindow);
}

void ExpectRefusal(const Preview& preview, Refusal refusal) {
  REQUIRE(std::holds_alternative<Refusal>(preview.decision));
  REQUIRE(std::get<Refusal>(preview.decision) == refusal);
}

void ExpectTarget(const Preview& preview,
                  ProfileId profile,
                  std::optional<SpaceId> space,
                  Surface surface = Surface::kMainWindow) {
  REQUIRE(std::holds_alternative<ResolvedTarget>(preview.decision));
  const auto& target = std::get<ResolvedTarget>(preview.decision);
  REQUIRE(target.profile == profile);
  REQUIRE(target.space == space);
  REQUIRE(target.surface == surface);
}

void AuthorityAndScope() {
  const auto model = Workspace();
  RuleBook rules;
  REQUIRE(rules.SetFallback({kDesk}) == ConfigStatus::kOk);
  auto browser = Browser();
  for (const auto entry :
       {EntryPoint::kPeek, EntryPoint::kArchiveRestore, EntryPoint::kOther}) {
    const auto preview =
        rules.PreviewExternal(model, browser, entry, {{kHome}});
    ExpectRefusal(preview, Refusal::kOutsideExternalEntry);
    REQUIRE(preview.matches.empty());
    REQUIRE(!preview.selected);
  }
  browser.authorization = UrlAuthorization::kUnknown;
  ExpectRefusal(Main(rules, model, browser),
                Refusal::kUrlAuthorizationRequired);
  browser.authorization = UrlAuthorization::kRejected;
  ExpectRefusal(rules.PreviewExternal(
                    model, browser, EntryPoint::kExternalMainWindow, {{kHome}}),
                Refusal::kUrlRejected);
  browser.authorization = UrlAuthorization::kAllowed;
  browser.canonical_url.clear();
  ExpectRefusal(Main(rules, model, browser), Refusal::kEmptyCanonicalUrl);
  browser = Browser();
  browser.host = UnknownHost{};
  ExpectRefusal(Main(rules, model, browser),
                Refusal::kHostClassificationRequired);
  browser.host = CanonicalHost{""};
  ExpectRefusal(Main(rules, model, browser), Refusal::kEmptyCanonicalHost);
  browser.host = NoHost{};
  ExpectTarget(Main(rules, model, browser), kWork, kDesk);
}

void LiteralMatchingAndPrecedence() {
  const auto model = Workspace();
  auto browser = Browser();
  RuleBook rules;
  REQUIRE(rules.Insert(0, UrlRule(kContains, MatchMode::kContains, "/guide",
                                  kHome)) == ConfigStatus::kOk);
  REQUIRE(rules.Insert(1, UrlRule(kExact, MatchMode::kExact, kUrl, kDesk)) ==
          ConfigStatus::kOk);
  REQUIRE(rules.Insert(2, {kHost,
                           {MatchField::kCanonicalHost, MatchMode::kExact,
                            "docs.example.test"},
                           {kWork}}) == ConfigStatus::kOk);
  auto preview = Main(rules, model, browser);
  REQUIRE(preview.matches.size() == 3);
  REQUIRE(preview.matches[0].rule == kContains);
  REQUIRE(preview.matches[1].rule == kExact);
  REQUIRE(preview.matches[2].rule == kHost);
  REQUIRE(preview.matches[0].target_exists);
  REQUIRE(std::get<RuleId>(preview.selected->selection) == kContains);
  ExpectTarget(preview, kPersonal, kHome);

  REQUIRE(rules.Move(kExact, 0) == ConfigStatus::kOk);
  preview = Main(rules, model, browser);
  ExpectTarget(preview, kWork, kDesk);
  REQUIRE(preview.matches[0].rule == kExact);
  REQUIRE(rules.Move(rules.rules()[0].id, 2) == ConfigStatus::kOk);
  ExpectTarget(Main(rules, model, browser), kPersonal, kHome);
  REQUIRE(rules.Edit(UrlRule(kContains, MatchMode::kContains, "missing",
                             kHome)) == ConfigStatus::kOk);
  preview = Main(rules, model, browser);
  REQUIRE(preview.matches.size() == 2);
  REQUIRE(preview.matches[0].rule == kHost);
  ExpectTarget(preview, kWork, std::nullopt);

  // Host classification is supplied by the browser, never parsed from URL text.
  browser.canonical_url = "https://elsewhere.test/path/docs.example.test";
  ExpectTarget(Main(rules, model, browser), kWork, std::nullopt);
  browser.host = CanonicalHost{"sub.docs.example.test"};
  ExpectRefusal(Main(rules, model, browser), Refusal::kNoFallback);
  REQUIRE(rules.Edit({kHost,
                      {MatchField::kCanonicalHost, MatchMode::kContains,
                       "docs.example.test"},
                      {kWork}}) == ConfigStatus::kOk);
  ExpectTarget(Main(rules, model, browser), kWork, std::nullopt);
  // Contains is a user-visible substring predicate, never a site boundary.
  browser.host = CanonicalHost{"docs.example.test.unrelated.test"};
  ExpectTarget(Main(rules, model, browser), kWork, std::nullopt);
  browser.host = CanonicalHost{"DOCS.EXAMPLE.TEST"};
  ExpectRefusal(Main(rules, model, browser), Refusal::kNoFallback);
  browser.host = NoHost{};
  ExpectRefusal(Main(rules, model, browser), Refusal::kNoFallback);
  browser.canonical_url = kUrl;
  ExpectTarget(Main(rules, model, browser), kWork, kDesk);
  browser.canonical_url += "#fragment";
  ExpectRefusal(Main(rules, model, browser), Refusal::kNoFallback);
}

void ConfigurationChanges() {
  const auto model = Workspace();
  const auto browser = Browser();
  RuleBook rules;
  auto exact = UrlRule(kExact, MatchMode::kExact, kUrl, kDesk);
  REQUIRE(rules.Insert(1, exact) == ConfigStatus::kInvalidPosition);
  REQUIRE(rules.rules().empty());
  REQUIRE(rules.Insert(0, exact) == ConfigStatus::kOk);
  REQUIRE(rules.Insert(0, exact) == ConfigStatus::kDuplicateRule);
  exact.predicate.literal.clear();
  REQUIRE(rules.Edit(exact) == ConfigStatus::kInvalidRule);
  ExpectTarget(Main(rules, model, browser), kWork, kDesk);
  REQUIRE(rules.Edit(UrlRule(kContains, MatchMode::kExact, kUrl, kHome)) ==
          ConfigStatus::kRuleNotFound);
  REQUIRE(rules.Move(kExact, 1) == ConfigStatus::kInvalidPosition);
  REQUIRE(rules.Move(kContains, 0) == ConfigStatus::kRuleNotFound);
  REQUIRE(rules.Edit(rules.rules()[0]) == ConfigStatus::kOk);
  REQUIRE(rules.Move(rules.rules()[0].id, 0) == ConfigStatus::kOk);
  REQUIRE(rules.Erase(rules.rules()[0].id) == ConfigStatus::kOk);
  REQUIRE(rules.Erase(kExact) == ConfigStatus::kRuleNotFound);
  REQUIRE(rules.rules().empty());
  REQUIRE(rules.Insert(0, UrlRule(RuleId{""}, MatchMode::kExact, kUrl,
                                  kDesk)) == ConfigStatus::kInvalidRule);
  REQUIRE(rules.Insert(0, UrlRule(kExact, MatchMode::kContains, "", kDesk)) ==
          ConfigStatus::kInvalidRule);
  REQUIRE(
      rules.Insert(0, UrlRule(kExact, MatchMode::kExact, kUrl, SpaceId{""})) ==
      ConfigStatus::kInvalidTarget);
  REQUIRE(rules.SetFallback({kHome}) == ConfigStatus::kOk);
  REQUIRE(rules.SetFallback({ProfileId{""}}) == ConfigStatus::kInvalidTarget);
  ExpectTarget(Main(rules, model, browser), kPersonal, kHome);
  rules.ClearFallback();
  ExpectRefusal(Main(rules, model, browser), Refusal::kNoFallback);
}

void FallbackAndOneTimeOverride() {
  const auto model = Workspace();
  auto browser = Browser();
  RuleBook rules;
  REQUIRE(rules.SetFallback({kPersonal}) == ConfigStatus::kOk);
  auto preview = Main(rules, model, browser);
  REQUIRE(
      std::holds_alternative<FallbackSelection>(preview.selected->selection));
  ExpectTarget(preview, kPersonal, std::nullopt);
  REQUIRE(rules.Insert(0, UrlRule(kExact, MatchMode::kExact, kUrl, kDesk)) ==
          ConfigStatus::kOk);
  preview = rules.PreviewExternal(model, browser,
                                  EntryPoint::kExternalMainWindow, {{kHome}});
  ExpectTarget(preview, kPersonal, kHome);
  REQUIRE(
      std::holds_alternative<OneTimeSelection>(preview.selected->selection));
  REQUIRE(preview.matches.size() == 1);
  ExpectTarget(Main(rules, model, browser), kWork, kDesk);
  ExpectRefusal(
      rules.PreviewExternal(model, browser, EntryPoint::kExternalMainWindow,
                            {{SpaceId{"deleted"}}}),
      Refusal::kTargetDeleted);
  ExpectTarget(Main(rules, model, browser), kWork, kDesk);

  // Entry provenance does not rewrite the selected surface.
  preview = rules.PreviewExternal(model, browser,
                                  EntryPoint::kExternalLightweightWindow);
  ExpectTarget(preview, kWork, kDesk);
  REQUIRE(rules.Edit({kExact,
                      {MatchField::kCanonicalUrl, MatchMode::kExact, kUrl},
                      {kDesk, Surface::kLightweightWindow}}) ==
          ConfigStatus::kOk);
  ExpectRefusal(Main(rules, model, browser), Refusal::kCapabilityUnknown);
  browser.lightweight_window = Capability::kUnavailable;
  ExpectRefusal(Main(rules, model, browser), Refusal::kCapabilityUnavailable);
  browser.lightweight_window = Capability::kAvailable;
  ExpectTarget(Main(rules, model, browser), kWork, kDesk,
               Surface::kLightweightWindow);
  ExpectTarget(rules.PreviewExternal(model, browser,
                                     EntryPoint::kExternalLightweightWindow),
               kWork, kDesk, Surface::kLightweightWindow);
  // Failure of a selected lightweight target does not use the main fallback.
  browser.main_window = Capability::kUnknown;
  ExpectRefusal(
      rules.PreviewExternal(model, browser,
                            EntryPoint::kExternalLightweightWindow, {{kHome}}),
      Refusal::kCapabilityUnknown);
  browser.main_window = Capability::kUnavailable;
  ExpectRefusal(rules.PreviewExternal(
                    model, browser, EntryPoint::kExternalMainWindow, {{kHome}}),
                Refusal::kCapabilityUnavailable);
}

void PrivateAndUnknownDestinations() {
  auto model = Workspace();
  auto browser = Browser();
  RuleBook rules;
  REQUIRE(rules.SetFallback({kPrivate}) == ConfigStatus::kOk);
  auto preview = Main(rules, model, browser);
  REQUIRE(
      std::holds_alternative<PrivateConfirmationRequired>(preview.decision));
  REQUIRE(
      std::get<PrivateConfirmationRequired>(preview.decision).choice.profile ==
      kPrivate);
  REQUIRE(rules.SetFallback({kHome}) == ConfigStatus::kOk);
  REQUIRE(rules.Insert(0, UrlRule(kExact, MatchMode::kExact, kUrl, kPrivate)) ==
          ConfigStatus::kOk);
  REQUIRE(rules.Insert(1, UrlRule(kContains, MatchMode::kContains, "guide",
                                  kDesk)) == ConfigStatus::kOk);
  preview = Main(rules, model, browser);
  REQUIRE(
      std::holds_alternative<PrivateConfirmationRequired>(preview.decision));
  REQUIRE(preview.matches.size() == 2);
  REQUIRE(std::get<RuleId>(preview.selected->selection) == kExact);
  preview = rules.PreviewExternal(
      model, browser, EntryPoint::kExternalMainWindow, {{kPrivate}});
  REQUIRE(
      std::holds_alternative<PrivateConfirmationRequired>(preview.decision));
  REQUIRE(
      std::holds_alternative<OneTimeSelection>(preview.selected->selection));
  ExpectTarget(rules.PreviewExternal(
                   model, browser, EntryPoint::kExternalMainWindow, {{kDesk}}),
               kWork, kDesk);
  REQUIRE(model.ReassignEmptySpace(kDesk, kPrivate) == Status::kOk);
  preview = rules.PreviewExternal(model, browser,
                                  EntryPoint::kExternalMainWindow, {{kDesk}});
  REQUIRE(
      std::holds_alternative<PrivateConfirmationRequired>(preview.decision));
  const auto& private_space =
      std::get<PrivateConfirmationRequired>(preview.decision).choice;
  REQUIRE(private_space.profile == kPrivate);
  REQUIRE(private_space.space == kDesk);
  browser.privacy_for = {};
  ExpectRefusal(Main(rules, model, browser), Refusal::kPrivacyUnknown);
  browser.privacy_for = [](const ProfileId&) { return Privacy::kUnknown; };
  ExpectRefusal(Main(rules, model, browser), Refusal::kPrivacyUnknown);
  browser = Browser();
  browser.main_window = Capability::kUnknown;
  ExpectRefusal(Main(rules, model, browser), Refusal::kCapabilityUnknown);
}

void DeletedTargetsAndAuthoritativeMapping() {
  auto model = Workspace();
  const auto browser = Browser();
  RuleBook rules;
  REQUIRE(rules.Insert(0, UrlRule(kExact, MatchMode::kExact, kUrl, kDesk)) ==
          ConfigStatus::kOk);
  REQUIRE(rules.Insert(1, UrlRule(kContains, MatchMode::kContains, "guide",
                                  kHome)) == ConfigStatus::kOk);
  const RuleId private_rule{"private-rule"};
  REQUIRE(rules.Insert(2, UrlRule(private_rule, MatchMode::kContains, "guide",
                                  kPrivate)) == ConfigStatus::kOk);
  REQUIRE(rules.SetFallback({kPrivate}) == ConfigStatus::kOk);
  ExpectTarget(Main(rules, model, browser), kWork, kDesk);
  REQUIRE(model.ReassignEmptySpace(kDesk, kPersonal) == Status::kOk);
  ExpectTarget(Main(rules, model, browser), kPersonal, kDesk);
  REQUIRE(model.RemoveSpace(kDesk) == Status::kOk);
  REQUIRE(model.RemoveProfile(kPrivate) == Status::kOk);
  auto preview = Main(rules, model, browser);
  ExpectRefusal(preview, Refusal::kTargetDeleted);
  REQUIRE(preview.matches.size() == 3);
  REQUIRE(!preview.matches[0].target_exists);
  REQUIRE(preview.matches[1].target_exists);
  REQUIRE(!preview.matches[2].target_exists);
  REQUIRE(std::get<RuleId>(preview.selected->selection) == kExact);
  const auto report = rules.PruneDeletedTargets(model);
  REQUIRE((report.removed_rules == std::vector<RuleId>{kExact, private_rule}));
  REQUIRE(report.removed_fallback.has_value());
  REQUIRE(std::get<ProfileId>(report.removed_fallback->target) == kPrivate);
  REQUIRE(rules.rules().size() == 1);
  REQUIRE(!rules.fallback());
  ExpectTarget(Main(rules, model, browser), kPersonal, kHome);
  REQUIRE(model.spaces().size() == 1);
  REQUIRE(model.spaces().at(kHome).profile == kPersonal);
  REQUIRE(model.profiles().size() == 2);
  REQUIRE(model.windows().empty());
  REQUIRE(model.pages().empty());
  REQUIRE(rules.PruneDeletedTargets(model).removed_rules.empty());
  REQUIRE(rules.SetFallback({kDesk}) == ConfigStatus::kOk);
  auto unrelated = Browser();
  unrelated.canonical_url = "https://unrelated.test/";
  ExpectRefusal(Main(rules, model, unrelated), Refusal::kTargetDeleted);
  REQUIRE(rules.PruneDeletedTargets(model).removed_fallback.has_value());
  ExpectRefusal(Main(rules, model, unrelated), Refusal::kNoFallback);
}

}  // namespace
}  // namespace arcified::air_traffic

int main() {
  using namespace arcified::air_traffic;
  AuthorityAndScope();
  LiteralMatchingAndPrecedence();
  ConfigurationChanges();
  FallbackAndOneTimeOverride();
  PrivateAndUnknownDestinations();
  DeletedTargetsAndAuthoritativeMapping();
  std::cout << "air_traffic: all focused routing checks passed\n";
}
