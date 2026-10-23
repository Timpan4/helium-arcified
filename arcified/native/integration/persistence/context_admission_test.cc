#include "context_admission.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>

#include "../../features/migrations/schema_gate.h"
#include "../../features/private_browsing/operation_boundary.h"

namespace arcified::persistence {
namespace {

using namespace lifecycle;
using durability::Error;
using durability::Failure;
using private_browsing::NativeContextId;
using private_browsing::NativeContextKind;

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

template <typename T>
T Value(durability::Result<T> result) {
  if (const auto* failure = std::get_if<Failure>(&result)) {
    std::cerr << failure->context << '\n';
    std::exit(EXIT_FAILURE);
  }
  return std::get<T>(std::move(result));
}

template <typename T>
void ExpectError(const durability::Result<T>& result, Error error) {
  REQUIRE(std::holds_alternative<Failure>(result));
  REQUIRE(std::get<Failure>(result).code == error);
}

const space_management::Metadata kNoMetadata;
const ProfileId kWork{"work"};
const ProfileId kHome{"home"};
const ProfileId kUnattached{"unattached"};
const SpaceId kDesk{"desk"};
const SpaceId kHomeSpace{"home-space"};
const SpaceId kUnattachedSpace{"unattached-space"};
const WindowId kWindow{"window"};
const WindowId kEmptyWindow{"empty-window"};
const PageId kSaved{"saved-page"};
const PageId kTemporary{"temporary-page"};
const PageId kHomePage{"home-page"};
const ItemId kItem{"saved-item"};
const Attachment kHere{kWindow, kDesk};

ContextObservation Regular(const ProfileId& profile) {
  const NativeContextId id("runtime-native-only-" + profile.value);
  return {{profile, id}, {NativeContextKind::kRegular, id, profile}};
}

// Synthetic observations only. This fixture is not a production registry,
// native binding producer or browser security acceptance test.
struct Observations final : NativeContextSource {
  std::map<ProfileId, ContextObservation> profiles;
  std::map<PageId, PageObservation> pages;
  std::map<WindowId, std::vector<ContextObservation>> windows;
  UnsupportedPeerPayloads payloads{
      PayloadPresence::kAbsent, PayloadPresence::kAbsent,
      PayloadPresence::kAbsent, PayloadPresence::kAbsent};

  std::optional<ContextObservation> ReadProfile(
      const ProfileId& id) const override {
    const auto found = profiles.find(id);
    return found == profiles.end() ? std::nullopt
                                   : std::optional(found->second);
  }
  std::optional<PageObservation> ReadPage(const PageId& id) const override {
    const auto found = pages.find(id);
    return found == pages.end() ? std::nullopt : std::optional(found->second);
  }
  std::optional<std::vector<ContextObservation>> ReadWindow(
      const WindowId& id) const override {
    const auto found = windows.find(id);
    return found == windows.end() ? std::nullopt : std::optional(found->second);
  }
  UnsupportedPeerPayloads ReadPeerPayloads() const override { return payloads; }
};

struct Fixture {
  Model model;
  space_management::Metadata metadata{
      {kUnattachedSpace, {std::nullopt, "", "theme:夜"}},
      {SpaceId("other-work"), {"仕事 🌌", std::nullopt, ""}},
      {kHomeSpace, {"", "星", std::nullopt}},
      {kDesk, {std::nullopt, std::nullopt, std::nullopt}}};
  Observations facts;

  Fixture() {
    for (const auto& profile : {kWork, kHome, kUnattached}) {
      REQUIRE(model.AddProfile(profile) == Status::kOk);
      facts.profiles.emplace(profile, Regular(profile));
    }
    REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
    REQUIRE(model.AddSpace(kHomeSpace, kHome) == Status::kOk);
    REQUIRE(model.AddSpace(kUnattachedSpace, kUnattached) == Status::kOk);
    REQUIRE(model.AddSpace(SpaceId("other-work"), kWork) == Status::kOk);
    REQUIRE(model.AddWindow(kWindow) == Status::kOk);
    REQUIRE(model.AddWindow(kEmptyWindow) == Status::kOk);
    REQUIRE(model.Attach(kHere) == Status::kOk);
    REQUIRE(model.Attach({kWindow, kHomeSpace}) == Status::kOk);
    REQUIRE(model.SelectSpace(kWindow, kHomeSpace) == Status::kOk);
    REQUIRE(model.AddItem(kItem, kDesk, "https://example.test/saved") ==
            Status::kOk);
    REQUIRE(std::holds_alternative<OpenedPage>(
        model.OpenItem(kItem, kHere, kSaved)));
    REQUIRE(model.RecordNavigation(kSaved, "https://example.test/committed") ==
            Status::kOk);
    REQUIRE(model.OpenTemporary(kTemporary, TemporaryTabId("temporary"), kHere,
                                "https://example.test/temporary") ==
            Status::kOk);
    REQUIRE(model.OpenTemporary(kHomePage, TemporaryTabId("home-tab"),
                                {kWindow, kHomeSpace},
                                "https://example.test/home") == Status::kOk);
    for (const auto& [id, page] : model.pages())
      facts.pages.emplace(
          id, PageObservation{page.attachment(), Regular(page.profile)});
    facts.windows.emplace(kWindow, std::vector<ContextObservation>{
                                       Regular(kWork), Regular(kHome)});
    facts.windows.emplace(kEmptyWindow,
                          std::vector<ContextObservation>{Regular(kWork)});
  }

  durability::SnapshotAdmission Admission() const {
    return Value(ComposeSnapshotAdmission(model, metadata, facts));
  }
  std::vector<durability::RestoredNavigation> Navigation() const {
    std::vector<durability::RestoredNavigation> reports;
    for (const auto& [id, page] : model.pages())
      reports.push_back({id, page.profile, page.current_navigation});
    return reports;
  }
};

void EligibleRoundTrip() {
  const Fixture f;
  const auto admission = f.Admission();
  const auto snapshot =
      Value(durability::CaptureSnapshot(f.model, f.metadata, admission));
  REQUIRE(snapshot.profiles.size() == f.model.profiles().size());
  REQUIRE(snapshot.pages.size() == f.model.pages().size());
  const auto bytes = Value(durability::EncodeSnapshot(snapshot, admission));
  const durability::StoreImage source{bytes, std::nullopt};
  const migrations::CompatibilityGate gate(source);
  REQUIRE(gate.inspection(migrations::ImageSource::kOriginal).state ==
          migrations::ImageState::kCurrent);
  const auto read =
      Value(gate.ReadSnapshot(migrations::ImageSource::kOriginal));
  const auto prepared = Value(gate.PrepareWrite(source, read, admission));
  REQUIRE(prepared.source() == source);
  REQUIRE(prepared.staged_image().original == source.original);
  REQUIRE(prepared.staged_image().candidate == bytes);
  const auto committed = Value(durability::CommitPreparedWrite(
      prepared.staged_image(), prepared, f.Admission()));
  REQUIRE(committed.original == bytes);
  REQUIRE(!committed.candidate);
  const auto restored =
      Value(durability::RestoreSnapshot(read, f.Navigation(), admission));
  REQUIRE(restored.model.pages().at(kSaved).current_navigation ==
          f.model.pages().at(kSaved).current_navigation);
  REQUIRE(restored.model.windows().at(kWindow).selected_space == kHomeSpace);
  REQUIRE(restored.model.profiles().count(kUnattached));
  REQUIRE(restored.space_metadata == f.metadata);
  REQUIRE(restored.model.pages().at(kSaved).profile == kWork);
  REQUIRE(restored.model.pages().at(kSaved).activity == Activity::kUnknown);
  REQUIRE(Value(durability::EncodeSnapshot(
              Value(durability::CaptureSnapshot(
                  restored.model, restored.space_metadata, admission)),
              admission)) == bytes);
  REQUIRE(source == gate.source());
  REQUIRE(!admission.profile_may_persist(ProfileId("unknown")));
  REQUIRE(!admission.page_may_persist(PageId("unknown"), kWork));
  REQUIRE(!admission.page_may_persist(kSaved, kHome));
}

template <typename Change>
void RefusalPreserves(Change change,
                      Error compose_error = Error::kPersistenceNotAdmitted) {
  Fixture f;
  const auto admission = f.Admission();
  const auto snapshot =
      Value(durability::CaptureSnapshot(f.model, f.metadata, admission));
  const auto bytes = Value(durability::EncodeSnapshot(snapshot, admission));
  const durability::StoreImage source{bytes, std::nullopt};
  const migrations::CompatibilityGate gate(source);
  const auto retained_facts = f.facts;
  const auto navigation = f.Navigation();
  const auto prepared = Value(gate.PrepareWrite(source, snapshot, admission));
  const auto staged = prepared.staged_image();
  change(f.facts);
  ExpectError(ComposeSnapshotAdmission(f.model, f.metadata, f.facts),
              compose_error);
  // Previously composed callbacks re-read the caller. No cached allow decision.
  ExpectError(durability::CaptureSnapshot(f.model, f.metadata, admission),
              compose_error);
  ExpectError(durability::EncodeSnapshot(snapshot, admission), compose_error);
  ExpectError(gate.PrepareWrite(source, snapshot, admission), compose_error);
  ExpectError(durability::RestoreSnapshot(snapshot, navigation, admission),
              compose_error);
  ExpectError(durability::CommitPreparedWrite(staged, prepared, admission),
              compose_error);
  REQUIRE(prepared.source() == source);
  REQUIRE(prepared.staged_image() == staged);
  REQUIRE(staged.original == bytes);
  REQUIRE(staged.candidate == bytes);
  REQUIRE(source == gate.source());
  REQUIRE(source.original == bytes);
  REQUIRE(!source.candidate);
  REQUIRE(f.Navigation().size() == navigation.size());
  f.facts = retained_facts;
  REQUIRE(Value(durability::EncodeSnapshot(Value(durability::CaptureSnapshot(
                                               f.model, f.metadata, admission)),
                                           admission)) == bytes);
  for (const auto& report : navigation)
    REQUIRE(f.model.pages().at(report.page).current_navigation ==
            report.committed_destination);
}

void MissingObservations() {
  for (const auto& profile : {kWork, kHome, kUnattached})
    RefusalPreserves(
        [&](Observations& facts) { facts.profiles.erase(profile); });
  for (const auto& page : {kSaved, kTemporary, kHomePage})
    RefusalPreserves([&](Observations& facts) { facts.pages.erase(page); });
  for (const auto& window : {kWindow, kEmptyWindow}) {
    RefusalPreserves([&](Observations& facts) { facts.windows.erase(window); });
    RefusalPreserves(
        [&](Observations& facts) { facts.windows.at(window).clear(); });
  }
  RefusalPreserves([](Observations& facts) {
    // The selected Home context cannot conceal the Work attachment.
    facts.windows.at(kWindow) = {Regular(kHome)};
  });
  Model empty;
  Observations absent;
  REQUIRE(empty.AddWindow(kEmptyWindow) == Status::kOk);
  // A Window without a registered regular Profile context cannot be admitted.
  ExpectError(ComposeSnapshotAdmission(empty, kNoMetadata, absent),
              Error::kPersistenceNotAdmitted);
}

void SharedClassifierRefusals() {
  // C++17 permits this for a scoped enum's fixed int underlying type.
  // Intentionally unnamed to exercise fail-closed boundary handling.
  // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
  constexpr NativeContextKind kUnrecognizedKind{-1};
  for (const auto kind :
       {NativeContextKind::kUnknown, NativeContextKind::kOffTheRecord,
        NativeContextKind::kGuest, NativeContextKind::kGuestOffTheRecord,
        NativeContextKind::kSystem, NativeContextKind::kSystemOffTheRecord,
        NativeContextKind::kOther, kUnrecognizedKind}) {
    auto observed = Regular(kWork);
    observed.facts.kind = kind;
    REQUIRE(private_browsing::CheckOperation(
                observed.facts,
                private_browsing::Operation::kWorkspacePersistence) !=
            private_browsing::Decision::kEligible);
    RefusalPreserves([&](Observations& facts) {
      facts.profiles.at(kUnattached).facts.kind = kind;
    });
    for (const auto& page : {kSaved, kTemporary, kHomePage})
      RefusalPreserves([&](Observations& facts) {
        facts.pages.at(page).context.facts.kind = kind;
      });
    RefusalPreserves([&](Observations& facts) {
      facts.windows.at(kEmptyWindow).front().facts.kind = kind;
    });
  }
  RefusalPreserves([](Observations& facts) {
    facts.profiles.at(kWork).facts.context.reset();
  });
  RefusalPreserves([](Observations& facts) {
    facts.profiles.at(kWork).facts.original_profile.reset();
  });
  RefusalPreserves([](Observations& facts) {
    facts.pages.at(kSaved).context.facts.context = NativeContextId("");
  });
}

void BindingConfusion() {
  RefusalPreserves(
      [](Observations& facts) { facts.profiles.at(kWork) = Regular(kHome); });
  RefusalPreserves([](Observations& facts) {
    facts.profiles.at(kWork).facts.original_profile = kHome;
  });
  RefusalPreserves([](Observations& facts) {
    facts.profiles.at(kWork).binding.context = NativeContextId("sibling");
  });
  RefusalPreserves([](Observations& facts) {
    // Each local report looks regular, but two Profiles claim one native ID.
    auto& other = facts.profiles.at(kUnattached);
    other.binding.context = facts.profiles.at(kWork).binding.context;
    other.facts.context = other.binding.context;
  });
  RefusalPreserves([](Observations& facts) {
    facts.pages.at(kSaved).context = Regular(kHome);
  });
  RefusalPreserves([](Observations& facts) {
    auto& page = facts.pages.at(kSaved).context;
    page.binding.context = NativeContextId("different-exact-regular-object");
    page.facts.context = page.binding.context;
  });
  RefusalPreserves([](Observations& facts) {
    facts.pages.at(kSaved).attachment.space = kHomeSpace;
  });
  RefusalPreserves([](Observations& facts) {
    facts.pages.at(kSaved).attachment.window = kEmptyWindow;
  });
  RefusalPreserves([](Observations& facts) {
    facts.windows.at(kWindow).push_back(Regular(kWork));
  });
  RefusalPreserves([](Observations& facts) {
    auto& context = facts.windows.at(kWindow).front();
    context.binding.context = NativeContextId("stale-window-context");
    context.facts.context = context.binding.context;
  });
  RefusalPreserves([](Observations& facts) {
    facts.windows.at(kEmptyWindow) = {Regular(ProfileId("unregistered"))};
  });
}

void PeerPayloadsRemainExplicit() {
  for (const auto member :
       {&UnsupportedPeerPayloads::folders, &UnsupportedPeerPayloads::themes,
        &UnsupportedPeerPayloads::split_presentation,
        &UnsupportedPeerPayloads::other}) {
    for (const auto presence :
         {PayloadPresence::kUnknown, PayloadPresence::kPresent}) {
      RefusalPreserves(
          [&](Observations& facts) { facts.payloads.*member = presence; },
          Error::kUnsupportedFeatures);
    }
  }
  Model empty;
  Observations unknown;
  unknown.payloads = {};
  ExpectError(ComposeSnapshotAdmission(empty, kNoMetadata, unknown),
              Error::kUnsupportedFeatures);
}

void NativeIdentityNeverSerialized() {
  Fixture f;
  const auto first = f.Admission();
  const auto snapshot =
      Value(durability::CaptureSnapshot(f.model, f.metadata, first));
  const auto bytes = Value(durability::EncodeSnapshot(snapshot, first));
  for (const auto& [profile, context] : f.facts.profiles)
    REQUIRE(bytes.find(context.binding.context.value) == std::string::npos);
  auto replace = [](ContextObservation& context) {
    context.binding.context.value += "-new-live-object";
    context.facts.context = context.binding.context;
  };
  for (auto& [id, context] : f.facts.profiles)
    replace(context);
  for (auto& [id, page] : f.facts.pages)
    replace(page.context);
  for (auto& [id, contexts] : f.facts.windows)
    for (auto& context : contexts)
      replace(context);
  const auto second = f.Admission();
  REQUIRE(Value(durability::EncodeSnapshot(snapshot, second)) == bytes);
}

void ChangedAttachmentRequiresNewNativeObservation() {
  Fixture f;
  const auto admission = f.Admission();
  const auto old =
      Value(durability::CaptureSnapshot(f.model, f.metadata, admission));
  const auto bytes = Value(durability::EncodeSnapshot(old, admission));
  const durability::StoreImage source{bytes, std::nullopt};
  const auto prepared = Value(durability::PrepareWrite(source, old, admission));
  const auto staged = prepared.staged_image();
  REQUIRE(f.model.Attach({kEmptyWindow, kDesk}) == Status::kOk);
  REQUIRE(f.model.TransferSavedPage(kSaved, {kEmptyWindow, kDesk}) ==
          Status::kOk);
  // Retaining the old Page observation after an actual model transfer refuses.
  // The old callback does not retain a prepared Page or attachment permit.
  ExpectError(durability::CaptureSnapshot(f.model, f.metadata, admission),
              Error::kPersistenceNotAdmitted);
  REQUIRE(f.model.pages().at(kSaved).attachment() ==
          (Attachment{kEmptyWindow, kDesk}));
  f.facts.pages.at(kSaved).attachment = {kEmptyWindow, kDesk};
  const auto refreshed = f.Admission();
  // Fresh, coherent native observations for a different graph cannot authorize
  // the old snapshot's Window/attachment/Page-owner provenance.
  ExpectError(durability::EncodeSnapshot(old, refreshed),
              Error::kPersistenceNotAdmitted);
  ExpectError(durability::RestoreSnapshot(old, f.Navigation(), refreshed),
              Error::kPersistenceNotAdmitted);
  const migrations::CompatibilityGate gate(source);
  ExpectError(gate.PrepareWrite(source, old, refreshed),
              Error::kPersistenceNotAdmitted);
  ExpectError(durability::CommitPreparedWrite(staged, prepared, refreshed),
              Error::kPersistenceNotAdmitted);
  REQUIRE(prepared.source() == source);
  REQUIRE(prepared.staged_image() == staged);
  REQUIRE(staged.original == bytes);
  REQUIRE(staged.candidate == bytes);
  REQUIRE(gate.source() == source);
  const auto snapshot =
      Value(durability::CaptureSnapshot(f.model, f.metadata, refreshed));
  const auto restored =
      Value(durability::RestoreSnapshot(snapshot, f.Navigation(), refreshed));
  REQUIRE(restored.model.pages().at(kSaved).attachment() ==
          (Attachment{kEmptyWindow, kDesk}));
  const auto next = Value(gate.PrepareWrite(source, snapshot, refreshed));
  const auto committed = Value(durability::CommitPreparedWrite(
      next.staged_image(), next, f.Admission()));
  const auto recovered = Value(durability::Recover(committed));
  REQUIRE(Value(durability::RestoreSnapshot(recovered.snapshot, f.Navigation(),
                                            f.Admission()))
              .model.pages()
              .at(kSaved)
              .attachment() == (Attachment{kEmptyWindow, kDesk}));
}

void EveryDurableRecordRequiresProvenance() {
  const Fixture f;
  const auto admission = f.Admission();
  const auto snapshot =
      Value(durability::CaptureSnapshot(f.model, f.metadata, admission));
  const auto bytes = Value(durability::EncodeSnapshot(snapshot, admission));
  const durability::StoreImage source{bytes, std::nullopt};
  const migrations::CompatibilityGate gate(source);
  const std::function<void(durability::DurableSnapshot&)> changes[] = {
      [](auto& s) { s.profiles.push_back(ProfileId("extra")); },
      [](auto& s) { s.spaces.back().second.profile = kWork; },
      [](auto& s) { s.windows.front().first = WindowId("other-empty-window"); },
      [](auto& s) { s.windows.back().second.selected_space = kDesk; },
      [](auto& s) { s.attachments.push_back({kEmptyWindow, kDesk}); },
      [](auto& s) { s.items.front().second.saved_destination += "/changed"; },
      [](auto& s) { s.items.front().second.owner = kWork; },
      [](auto& s) {
        s.shortcuts.push_back({ShortcutId("alias"), {kWork, kItem}});
      },
      [](auto& s) {
        for (auto& page : s.pages)
          if (page.id == kTemporary)
            std::get<TemporaryTab>(page.owner).id = TemporaryTabId("other-tab");
      },
      [](auto& s) {
        s.splits.push_back({SplitId("split"), {kHere, {kSaved, kTemporary}}});
      },
  };
  for (const auto& change : changes) {
    auto different = snapshot;
    change(different);
    REQUIRE(!durability::ValidateSnapshot(different));
    ExpectError(durability::EncodeSnapshot(different, f.Admission()),
                Error::kPersistenceNotAdmitted);
    ExpectError(gate.PrepareWrite(source, different, f.Admission()),
                Error::kPersistenceNotAdmitted);
    ExpectError(
        durability::RestoreSnapshot(different, f.Navigation(), f.Admission()),
        Error::kPersistenceNotAdmitted);
    REQUIRE(gate.source() == source);
  }
  // Sequence order is not provenance; split membership order remains durable.
  auto reordered = snapshot;
  std::reverse(reordered.profiles.begin(), reordered.profiles.end());
  std::reverse(reordered.pages.begin(), reordered.pages.end());
  Value(durability::EncodeSnapshot(reordered, f.Admission()));
}

void CompleteMetadataAndCandidateProvenance() {
  Fixture f;
  const auto admission = f.Admission();
  const auto snapshot =
      Value(durability::CaptureSnapshot(f.model, f.metadata, admission));
  const auto bytes = Value(durability::EncodeSnapshot(snapshot, admission));
  const durability::StoreImage source{bytes, std::nullopt};
  const migrations::CompatibilityGate gate(source);
  const auto prepared = Value(gate.PrepareWrite(source, snapshot, admission));
  const auto staged = prepared.staged_image();
  const auto original_metadata = f.metadata;

  auto missing = original_metadata;
  missing.pop_back();
  auto duplicate = original_metadata;
  duplicate.push_back(duplicate.front());
  auto unknown = original_metadata;
  unknown.front().space = SpaceId("unknown");
  for (const auto& invalid :
       {space_management::Metadata{}, missing, duplicate, unknown}) {
    f.metadata = invalid;
    ExpectError(ComposeSnapshotAdmission(f.model, f.metadata, f.facts),
                Error::kInvalidMetadata);
    ExpectError(durability::CaptureSnapshot(f.model, f.metadata, admission),
                Error::kInvalidMetadata);
    auto malformed = snapshot;
    malformed.space_metadata = invalid;
    REQUIRE(durability::ValidateSnapshot(malformed)->code ==
            Error::kInvalidMetadata);
    ExpectError(durability::EncodeSnapshot(malformed, admission),
                Error::kInvalidMetadata);
    ExpectError(gate.PrepareWrite(source, malformed, admission),
                Error::kInvalidMetadata);
    ExpectError(
        durability::RestoreSnapshot(malformed, f.Navigation(), admission),
        Error::kInvalidMetadata);
    REQUIRE(!durability::SnapshotMatchesModel(snapshot, f.model, invalid));
    REQUIRE(prepared.source() == source);
    REQUIRE(prepared.staged_image() == staged);
    REQUIRE(gate.source() == source);
  }
  f.metadata = original_metadata;
  const std::function<void(space_management::Metadata&)> changes[] = {
      [](auto& m) { std::reverse(m.begin(), m.end()); },
      [](auto& m) { m.front().identity.name = ""; },
      [](auto& m) { m.front().identity.icon_key.reset(); },
      [](auto& m) { m.front().identity.theme_key = "other-theme"; },
  };
  for (const auto& change : changes) {
    auto forged = snapshot;
    change(forged.space_metadata);
    REQUIRE(!durability::ValidateSnapshot(forged));
    ExpectError(durability::EncodeSnapshot(forged, admission),
                Error::kPersistenceNotAdmitted);
    ExpectError(gate.PrepareWrite(source, forged, admission),
                Error::kPersistenceNotAdmitted);
    ExpectError(durability::RestoreSnapshot(forged, f.Navigation(), admission),
                Error::kPersistenceNotAdmitted);

    // A legitimate display edit after preparation invalidates the old candidate
    // with both retained callbacks and newly composed current admission.
    f.metadata = forged.space_metadata;
    ExpectError(
        durability::CaptureSnapshot(f.model, original_metadata, admission),
        Error::kPersistenceNotAdmitted);
    const auto current = f.Admission();
    ExpectError(durability::EncodeSnapshot(snapshot, current),
                Error::kPersistenceNotAdmitted);
    ExpectError(gate.PrepareWrite(source, snapshot, current),
                Error::kPersistenceNotAdmitted);
    ExpectError(durability::RestoreSnapshot(snapshot, f.Navigation(), current),
                Error::kPersistenceNotAdmitted);
    for (const auto& check : {admission, current})
      ExpectError(durability::CommitPreparedWrite(staged, prepared, check),
                  Error::kPersistenceNotAdmitted);
    REQUIRE(prepared.source() == source);
    REQUIRE(prepared.staged_image() == staged);
    REQUIRE(gate.source() == source);

    // Candidate metadata, rather than the older committed original, is
    // admitted.
    const auto next =
        Value(durability::CaptureSnapshot(f.model, f.metadata, current));
    const auto next_write = Value(gate.PrepareWrite(source, next, current));
    const auto next_staged = next_write.staged_image();
    REQUIRE(next_staged.original == bytes);
    REQUIRE(next_staged.candidate != bytes);
    const auto committed = Value(
        durability::CommitPreparedWrite(next_staged, next_write, current));
    const auto recovered = Value(durability::Recover(committed));
    const auto restored = Value(durability::RestoreSnapshot(
        recovered.snapshot, f.Navigation(), current));
    REQUIRE(restored.space_metadata == f.metadata);
    REQUIRE(Value(durability::EncodeSnapshot(
                Value(durability::CaptureSnapshot(
                    restored.model, restored.space_metadata, current)),
                current)) == *committed.original);
    REQUIRE(next_write.source() == source);
    REQUIRE(next_write.staged_image() == next_staged);
    f.metadata = original_metadata;
  }
}

void EmptyAndWindowOnlyGraphsRecheckAtUse() {
  Model model;
  const space_management::Metadata metadata;
  Observations facts;
  const auto admission =
      Value(ComposeSnapshotAdmission(model, metadata, facts));
  const auto empty =
      Value(durability::CaptureSnapshot(model, metadata, admission));
  const auto bytes = Value(durability::EncodeSnapshot(empty, admission));
  const durability::StoreImage source{bytes, std::nullopt};
  const auto prepared =
      Value(durability::PrepareWrite(source, empty, admission));
  const auto staged = prepared.staged_image();
  facts.payloads.folders = PayloadPresence::kPresent;
  ExpectError(durability::CaptureSnapshot(model, metadata, admission),
              Error::kUnsupportedFeatures);
  ExpectError(durability::EncodeSnapshot(empty, admission),
              Error::kUnsupportedFeatures);
  ExpectError(durability::PrepareWrite(source, empty, admission),
              Error::kUnsupportedFeatures);
  ExpectError(durability::RestoreSnapshot(empty, {}, admission),
              Error::kUnsupportedFeatures);
  ExpectError(durability::CommitPreparedWrite(staged, prepared, admission),
              Error::kUnsupportedFeatures);
  REQUIRE(prepared.source() == source);
  REQUIRE(prepared.staged_image() == staged);
  facts.payloads.folders = PayloadPresence::kAbsent;
  REQUIRE(model.AddWindow(kEmptyWindow) == Status::kOk);
  // No per-Profile or per-Page check can see this unobserved Window.
  ExpectError(durability::CaptureSnapshot(model, metadata, admission),
              Error::kPersistenceNotAdmitted);
  ExpectError(ComposeSnapshotAdmission(model, metadata, facts),
              Error::kPersistenceNotAdmitted);
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  facts.profiles.emplace(kWork, Regular(kWork));
  facts.windows.emplace(kEmptyWindow,
                        std::vector<ContextObservation>{Regular(kWork)});
  const auto current = Value(ComposeSnapshotAdmission(model, metadata, facts));
  const auto window =
      Value(durability::CaptureSnapshot(model, metadata, current));
  REQUIRE(window.pages.empty());
  REQUIRE(window.spaces.empty());
  facts.windows.at(kEmptyWindow).front().facts.kind =
      NativeContextKind::kOffTheRecord;
  ExpectError(durability::EncodeSnapshot(window, current),
              Error::kPersistenceNotAdmitted);
}

void SchemaGatePreservesImages() {
  const Fixture f;
  const auto admission = f.Admission();
  const auto snapshot =
      Value(durability::CaptureSnapshot(f.model, f.metadata, admission));
  const auto bytes = Value(durability::EncodeSnapshot(snapshot, admission));
  auto future = bytes;
  future[8] = static_cast<char>(durability::kSchemaVersion + 1);
  for (const auto& source : {durability::StoreImage{future, std::nullopt},
                             durability::StoreImage{bytes, future}}) {
    const migrations::CompatibilityGate gate(source);
    ExpectError(gate.PrepareWrite(source, snapshot, admission),
                Error::kUnsupportedSchema);
    REQUIRE(source == gate.source());
  }
  const durability::StoreImage interrupted{bytes, bytes};
  const migrations::CompatibilityGate gate(interrupted);
  ExpectError(gate.PrepareWrite(interrupted, snapshot, admission),
              Error::kRecoveryRequired);
  REQUIRE(interrupted == gate.source());
  auto changed = interrupted;
  changed.original = "independent writer";
  ExpectError(gate.PrepareWrite(changed, snapshot, admission),
              Error::kStaleSource);
  REQUIRE(changed.original == "independent writer");
  REQUIRE(changed.candidate == bytes);
  REQUIRE(gate.source() == interrupted);
  auto missing = f.Navigation();
  missing.pop_back();
  ExpectError(durability::RestoreSnapshot(snapshot, missing, admission),
              Error::kNavigationEvidenceRequired);
  REQUIRE(gate.source() == interrupted);
}

}  // namespace
}  // namespace arcified::persistence

int main() {
  arcified::persistence::CompleteMetadataAndCandidateProvenance();
  arcified::persistence::EligibleRoundTrip();
  arcified::persistence::MissingObservations();
  arcified::persistence::SharedClassifierRefusals();
  arcified::persistence::BindingConfusion();
  arcified::persistence::PeerPayloadsRemainExplicit();
  arcified::persistence::NativeIdentityNeverSerialized();
  arcified::persistence::ChangedAttachmentRequiresNewNativeObservation();
  arcified::persistence::EveryDurableRecordRequiresProvenance();
  arcified::persistence::EmptyAndWindowOnlyGraphsRecheckAtUse();
  arcified::persistence::SchemaGatePreservesImages();
  std::cout << "composed persistence synthetic checks passed\n";
}
