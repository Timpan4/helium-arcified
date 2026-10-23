#include "site_search.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <utility>

namespace arcified::commands::site_search {
namespace {

static_assert(!std::is_convertible_v<ProviderId, SnapshotRevision>);
static_assert(!std::is_convertible_v<ProviderId, lifecycle::ProfileId>);
static_assert(!std::is_default_constructible_v<Selection>);

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const lifecycle::ProfileId kWork{"synthetic-work"};
const lifecycle::ProfileId kPersonal{"synthetic-personal"};
const ProviderId kDocs{"native-docs"};
const ProviderId kLibrary{"native-library"};
const ProviderId kJapanese{"native-japanese"};
const Input kInput{u"test query", CompositionState::kCommitted};

SnapshotData Data(const char* revision) {
  return {kWork,
          SnapshotRevision{revision},
          ServiceState::kReady,
          {{kDocs, u"Example documentation", u"docs"},
           {kLibrary, u"Example library", u"!lib"},
           {kJapanese, u"日本語の資料", u"資料"}},
          kLibrary};
}

ProviderSnapshot Snapshot(SnapshotData data) {
  auto result = ProviderSnapshot::FromNative(std::move(data));
  REQUIRE(std::holds_alternative<ProviderSnapshot>(result));
  return std::get<ProviderSnapshot>(std::move(result));
}

Selection Choose(const ProviderSnapshot& snapshot,
                 ProviderTarget target,
                 const Input& input = kInput) {
  auto result = snapshot.Select(std::move(target), input);
  REQUIRE(std::holds_alternative<Selection>(result));
  return std::get<Selection>(std::move(result));
}

std::vector<ProviderChoice> Discover(const ProviderSnapshot& snapshot,
                                     const std::u16string& prefix = u"") {
  auto result = snapshot.Discover(prefix);
  REQUIRE(std::holds_alternative<std::vector<ProviderChoice>>(result));
  return std::get<std::vector<ProviderChoice>>(std::move(result));
}

SearchIntent Submit(const ProviderSnapshot& snapshot,
                    const Selection& selection,
                    const Input& input = kInput) {
  auto result = snapshot.Submit(selection, input);
  REQUIRE(std::holds_alternative<SearchIntent>(result));
  return std::get<SearchIntent>(std::move(result));
}

template <typename Result>
void ExpectRefusal(const Result& result, Refusal refusal) {
  REQUIRE(std::holds_alternative<Refusal>(result));
  REQUIRE(std::get<Refusal>(result) == refusal);
}

void ExpectInvalid(SnapshotData data, SnapshotError error) {
  auto result = ProviderSnapshot::FromNative(std::move(data));
  REQUIRE(std::holds_alternative<SnapshotError>(result));
  REQUIRE(std::get<SnapshotError>(result) == error);
}

void ValidateNativeProjection() {
  auto data = Data("validation");
  data.profile = lifecycle::ProfileId{""};
  ExpectInvalid(data, SnapshotError::kMissingIdentity);
  data = Data("");
  ExpectInvalid(data, SnapshotError::kMissingIdentity);
  data = Data("validation");
  data.state = static_cast<ServiceState>(-1);
  ExpectInvalid(data, SnapshotError::kInvalidState);
  data = Data("validation");
  data.providers.front().id = ProviderId{""};
  ExpectInvalid(data, SnapshotError::kInvalidProvider);
  data = Data("validation");
  data.providers.front().label.clear();
  ExpectInvalid(data, SnapshotError::kInvalidProvider);
  data = Data("validation");
  data.providers.push_back(data.providers.front());
  ExpectInvalid(data, SnapshotError::kDuplicateProvider);
  data = Data("validation");
  data.default_provider = ProviderId{"absent"};
  ExpectInvalid(data, SnapshotError::kUnknownDefault);
  data = Data("validation");
  data.state = ServiceState::kUnavailable;
  ExpectInvalid(data, SnapshotError::kProvidersWhileNotReady);
  data.providers.clear();
  ExpectInvalid(data, SnapshotError::kProvidersWhileNotReady);
}

void DiscoverActualProvidersWithoutChoosing() {
  const auto snapshot = Snapshot(Data("discovery"));
  const auto all = Discover(snapshot);
  REQUIRE(all.size() == 3);
  REQUIRE(all[0].provider.id == kDocs);
  REQUIRE(all[0].provider.label == u"Example documentation");
  REQUIRE(all[0].provider.shortcut == u"docs");
  REQUIRE(!all[0].is_default);
  REQUIRE(all[1].provider.id == kLibrary);
  REQUIRE(all[1].provider.label == u"Example library");
  REQUIRE(all[1].is_default);
  REQUIRE(all[2].provider.label == u"日本語の資料");
  REQUIRE(!all[2].is_default);
  const auto docs = Discover(snapshot, u"doc");
  REQUIRE(docs.size() == 1);
  REQUIRE(docs.front().provider.id == kDocs);
  REQUIRE(Discover(snapshot, u"!").front().provider.id == kLibrary);
  REQUIRE(Discover(snapshot, u"資").front().provider.id == kJapanese);
  REQUIRE(Discover(snapshot, u"DOC").empty());
  REQUIRE(Discover(snapshot, u"https://example.test/").empty());
  REQUIRE(Discover(snapshot, u"docs longer than the keyword").empty());
  ExpectRefusal(snapshot.Submit(std::nullopt, kInput),
                Refusal::kSelectionRequired);

  auto data = Data("ambiguous");
  data.providers[1].label = data.providers[0].label;
  data.providers[1].shortcut = data.providers[0].shortcut;
  const auto ambiguous = Snapshot(data);
  const auto choices = Discover(ambiguous, u"docs");
  REQUIRE(choices.size() == 2);
  REQUIRE(choices[0].provider.id == kDocs);
  REQUIRE(choices[1].provider.id == kLibrary);
  REQUIRE(Submit(ambiguous, Choose(ambiguous, kLibrary)).provider.id ==
          kLibrary);
}

void ExplicitPreviewAndLiteralInput() {
  const auto snapshot = Snapshot(Data("literal"));
  const Input literal{
      u"  東京 العربية 한글 cafe\u0301 \U0001f680 "
      u"https://example.test/?q=%{searchTerms} + !lib  ",
      CompositionState::kCommitted};
  const auto selected = Choose(snapshot, kJapanese, literal);
  REQUIRE(selected.profile() == kWork);
  REQUIRE(selected.revision() == SnapshotRevision{"literal"});
  REQUIRE(selected.provider().id == kJapanese);
  REQUIRE(selected.provider().label == u"日本語の資料");
  REQUIRE(selected.query() == literal.query);
  REQUIRE(!selected.uses_default());
  const auto intent = Submit(snapshot, selected, literal);
  REQUIRE(intent.profile == kWork);
  REQUIRE(intent.revision == SnapshotRevision{"literal"});
  REQUIRE(intent.provider.id == kJapanese);
  REQUIRE(intent.provider.label == u"日本語の資料");
  REQUIRE(intent.query == literal.query);

  const auto default_choice = Choose(snapshot, DefaultProvider{});
  REQUIRE(default_choice.uses_default());
  REQUIRE(default_choice.provider().label == u"Example library");
  REQUIRE(Submit(snapshot, default_choice).provider.id == kLibrary);
  ExpectRefusal(snapshot.Select(ProviderId{"absent"}, literal),
                Refusal::kProviderUnavailable);
  ExpectRefusal(snapshot.Select(kDocs, {u"", CompositionState::kCommitted}),
                Refusal::kEmptyQuery);
  // Explicit provider mode treats even URL-like text as the selected query.
  // Only native code may decide whether ordinary unscoped input is a URL.
  const Input url_text{u"https://example.test/", CompositionState::kCommitted};
  REQUIRE(Submit(snapshot, Choose(snapshot, kDocs, url_text), url_text).query ==
          url_text.query);
}

void CompositionAndQueryChangesRequireFreshPreview() {
  const auto snapshot = Snapshot(Data("ime"));
  const auto selected = Choose(snapshot, kJapanese);
  Input input{u"資料", CompositionState::kComposing};
  ExpectRefusal(snapshot.Select(kJapanese, input), Refusal::kCompositionActive);
  ExpectRefusal(snapshot.Submit(selected, input), Refusal::kCompositionActive);
  input.composition = CompositionState::kCommitted;
  ExpectRefusal(snapshot.Submit(selected, input), Refusal::kQueryChanged);
  const auto committed = Choose(snapshot, kJapanese, input);
  REQUIRE(Submit(snapshot, committed, input).query == u"資料");
  input.composition = CompositionState::kComposing;
  ExpectRefusal(snapshot.Submit(committed, input), Refusal::kCompositionActive);
  input = {u"資料 ", CompositionState::kCommitted};
  ExpectRefusal(snapshot.Submit(committed, input), Refusal::kQueryChanged);
  input.query.clear();
  ExpectRefusal(snapshot.Submit(committed, input), Refusal::kQueryChanged);
  // Cancellation retains no implicit default or last submitted selection.
  ExpectRefusal(snapshot.Submit(std::nullopt, kInput),
                Refusal::kSelectionRequired);
}

void AuthoritativeRefreshReconcilesProviderAndDefaultChanges() {
  const auto original = Snapshot(Data("before-edit"));
  const auto selected = Choose(original, kDocs);
  const auto old_default = Choose(original, DefaultProvider{});
  auto data = Data("after-edit");
  data.providers.front().label = u"Renamed documentation";
  data.providers.front().shortcut = u"manual";
  data.providers.erase(data.providers.begin() + 1);
  const ProviderId added{"native-added"};
  data.providers.push_back({added, u"New native provider", u"new"});
  data.default_provider = added;
  const auto refreshed = Snapshot(data);
  ExpectRefusal(refreshed.Submit(selected, kInput), Refusal::kSnapshotChanged);
  ExpectRefusal(refreshed.Submit(old_default, kInput),
                Refusal::kSnapshotChanged);
  REQUIRE(Discover(refreshed, u"doc").empty());
  REQUIRE(Discover(refreshed, u"!lib").empty());
  REQUIRE(Discover(refreshed, u"manual").front().provider.label ==
          u"Renamed documentation");
  REQUIRE(Discover(refreshed, u"new").front().is_default);
  const auto new_default = Choose(refreshed, DefaultProvider{});
  REQUIRE(new_default.provider().label == u"New native provider");
  REQUIRE(Submit(refreshed, new_default).provider.id == added);
  ExpectRefusal(refreshed.Select(kLibrary, kInput),
                Refusal::kProviderUnavailable);
  REQUIRE(selected.provider().label == u"Example documentation");

  data.revision = SnapshotRevision{"after-policy-refresh"};
  const auto same_rows_new_revision = Snapshot(data);
  ExpectRefusal(same_rows_new_revision.Submit(new_default, kInput),
                Refusal::kSnapshotChanged);
  data.profile = kPersonal;
  data.revision = SnapshotRevision{"new-context"};
  const auto personal = Snapshot(data);
  ExpectRefusal(personal.Submit(new_default, kInput), Refusal::kProfileChanged);
  REQUIRE(Submit(personal, Choose(personal, added)).profile == kPersonal);
}

void RevalidateVisibleIdentityEvenWithIncorrectlyReusedRevision() {
  const auto original = Snapshot(Data("caller-contract-violation"));
  const auto selected = Choose(original, kDocs);
  const auto default_choice = Choose(original, DefaultProvider{});
  auto data = Data("caller-contract-violation");
  data.providers.front().label = u"Different native name";
  ExpectRefusal(Snapshot(data).Submit(selected, kInput),
                Refusal::kProviderChanged);
  data = Data("caller-contract-violation");
  data.providers.front().shortcut = u"other";
  ExpectRefusal(Snapshot(data).Submit(selected, kInput),
                Refusal::kProviderChanged);
  data = Data("caller-contract-violation");
  data.providers.erase(data.providers.begin());
  ExpectRefusal(Snapshot(data).Submit(selected, kInput),
                Refusal::kProviderUnavailable);
  data = Data("caller-contract-violation");
  data.default_provider = kJapanese;
  ExpectRefusal(Snapshot(data).Submit(default_choice, kInput),
                Refusal::kDefaultChanged);
  data.default_provider.reset();
  ExpectRefusal(Snapshot(data).Submit(default_choice, kInput),
                Refusal::kDefaultChanged);
  data = Data("caller-contract-violation");
  data.profile = kPersonal;
  ExpectRefusal(Snapshot(data).Submit(selected, kInput),
                Refusal::kProfileChanged);
  // URL/policy changes invisible to this projection require a fresh revision.
}

void UnavailableAndNoDefaultNeverInventFallbacks() {
  const auto ready = Snapshot(Data("ready"));
  const auto selected = Choose(ready, kDocs);
  for (const auto& entry :
       {std::pair{ServiceState::kLoading, Refusal::kServiceLoading},
        std::pair{ServiceState::kUnavailable, Refusal::kServiceUnavailable},
        std::pair{ServiceState::kBlocked, Refusal::kPolicyBlocked}}) {
    const auto unavailable = Snapshot({kWork,
                                       SnapshotRevision{"unavailable"},
                                       entry.first,
                                       {},
                                       std::nullopt});
    ExpectRefusal(unavailable.Discover(u""), entry.second);
    ExpectRefusal(unavailable.Select(DefaultProvider{}, kInput), entry.second);
    ExpectRefusal(unavailable.Select(kDocs, kInput), entry.second);
    ExpectRefusal(unavailable.Submit(selected, kInput), entry.second);
  }
  auto data = Data("no-default");
  data.default_provider.reset();
  data.providers.front().shortcut.clear();
  const auto no_default = Snapshot(data);
  for (const auto& choice : Discover(no_default))
    REQUIRE(!choice.is_default);
  REQUIRE(Discover(no_default).size() == 3);
  REQUIRE(Discover(no_default, u"doc").empty());
  ExpectRefusal(no_default.Select(DefaultProvider{}, kInput),
                Refusal::kNoDefault);
  REQUIRE(Submit(no_default, Choose(no_default, kDocs)).provider.id == kDocs);
  data.providers.clear();
  data.revision = SnapshotRevision{"empty-service"};
  const auto empty = Snapshot(data);
  REQUIRE(Discover(empty).empty());
  ExpectRefusal(empty.Select(DefaultProvider{}, kInput), Refusal::kNoDefault);
  ExpectRefusal(empty.Select(kDocs, kInput), Refusal::kProviderUnavailable);
}

void PreviewOwnsItsMetadataAndQuery() {
  Input input = kInput;
  auto data = Data("owned-preview");
  auto selected = Choose(Snapshot(data), kDocs, input);
  data.providers.front().label = u"Caller changed its source buffer";
  input.query = u"Caller changed its input buffer";
  REQUIRE(selected.provider().label == u"Example documentation");
  REQUIRE(selected.query() == u"test query");
  const auto snapshot = Snapshot(Data("owned-preview"));
  ExpectRefusal(snapshot.Submit(selected, input), Refusal::kQueryChanged);
  auto intent = Submit(snapshot, selected);
  selected = Choose(snapshot, kLibrary);
  REQUIRE(intent.provider.id == kDocs);
  REQUIRE(intent.provider.label == u"Example documentation");
  REQUIRE(intent.query == u"test query");
}

}  // namespace
}  // namespace arcified::commands::site_search

int main() {
  using namespace arcified::commands::site_search;
  ValidateNativeProjection();
  DiscoverActualProvidersWithoutChoosing();
  ExplicitPreviewAndLiteralInput();
  CompositionAndQueryChangesRequireFreshPreview();
  AuthoritativeRefreshReconcilesProviderAndDefaultChanges();
  RevalidateVisibleIdentityEvenWithIncorrectlyReusedRevision();
  UnavailableAndNoDefaultNeverInventFallbacks();
  PreviewOwnsItsMetadataAndQuery();
  std::cout << "site-search synthetic checks passed\n";
}
