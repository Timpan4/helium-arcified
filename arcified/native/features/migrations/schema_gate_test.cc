#include "schema_gate.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <variant>

namespace arcified::migrations {
namespace {

using durability::DurableSnapshot;
using durability::Error;
using durability::Failure;
using durability::StoreImage;
using namespace lifecycle;

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

template <typename T>
T Success(durability::Result<T> result) {
  REQUIRE(std::holds_alternative<T>(result));
  return std::get<T>(std::move(result));
}

template <typename T>
void ExpectFailure(const durability::Result<T>& result, Error error) {
  REQUIRE(std::holds_alternative<Failure>(result));
  if (std::get<Failure>(result).code != error)
    std::cerr << "expected error " << static_cast<int>(error) << ", got "
              << static_cast<int>(std::get<Failure>(result).code) << ": "
              << std::get<Failure>(result).context << '\n';
  REQUIRE(std::get<Failure>(result).code == error);
}

// Synthetic regular-context admission, never native privacy classification.
// Reject unknown IDs so tests cannot accidentally admit another fixture
// context.
durability::SnapshotAdmission RegularFixture() {
  return {
      [](const DurableSnapshot&) { return std::optional<Failure>{}; },
      [](const ProfileId& profile) { return profile == ProfileId{"regular"}; },
      [](const PageId& page, const ProfileId& profile) {
        return page == PageId{"page"} && profile == ProfileId{"regular"};
      },
      true,
  };
}

DurableSnapshot FixtureSnapshot() {
  Model model;
  REQUIRE(model.AddProfile(ProfileId{"regular"}) == Status::kOk);
  REQUIRE(model.AddSpace(SpaceId{"space"}, ProfileId{"regular"}) ==
          Status::kOk);
  REQUIRE(model.AddWindow(WindowId{"window"}) == Status::kOk);
  REQUIRE(model.Attach({WindowId{"window"}, SpaceId{"space"}}) == Status::kOk);
  REQUIRE(model.SelectSpace(WindowId{"window"}, SpaceId{"space"}) ==
          Status::kOk);
  REQUIRE(model.AddItem(ItemId{"saved"}, SpaceId{"space"},
                        "https://saved.test/") == Status::kOk);
  REQUIRE(std::holds_alternative<OpenedPage>(
      model.OpenItem(ItemId{"saved"}, {WindowId{"window"}, SpaceId{"space"}},
                     PageId{"page"})));
  REQUIRE(model.RecordNavigation(PageId{"page"}, "https://live.test/") ==
          Status::kOk);
  REQUIRE(model.ReportActivity(PageId{"page"}, Activity::kProtected) ==
          Status::kOk);
  return Success(durability::CaptureSnapshot(model, RegularFixture()));
}

std::string Encode(const DurableSnapshot& snapshot) {
  return Success(durability::EncodeSnapshot(snapshot, RegularFixture()));
}

void CurrentNoopAndOwnedSource() {
  const auto snapshot = FixtureSnapshot();
  const auto bytes = Encode(snapshot);
  StoreImage image{bytes, std::nullopt};
  const CompatibilityGate gate(image);
  image.original = "changed outside gate";
  REQUIRE(gate.source().original == bytes);
  REQUIRE(!gate.source().candidate);
  REQUIRE(gate.inspection(ImageSource::kOriginal).state ==
          ImageState::kCurrent);
  REQUIRE(gate.inspection(ImageSource::kCandidate).state ==
          ImageState::kAbsent);
  REQUIRE(gate.recovery_choices() ==
          std::vector<ImageSource>{ImageSource::kOriginal});
  const auto read = Success(gate.ReadSnapshot(ImageSource::kOriginal));
  REQUIRE(read.windows.front().second.selected_space == SpaceId{"space"});
  REQUIRE(read.items.front().second.saved_destination == "https://saved.test/");
  REQUIRE(read.pages.front().profile == ProfileId{"regular"});
  REQUIRE(std::get<SavedPage>(read.pages.front().owner).item ==
          ItemId{"saved"});
  REQUIRE(Encode(read) == bytes);
  REQUIRE(gate.source().original == bytes);
  ExpectFailure(gate.ReadSnapshot(ImageSource::kCandidate),
                Error::kRecoveryRequired);
}

void EmptyIsDifferentFromCorrupt() {
  const auto snapshot = FixtureSnapshot();
  const StoreImage empty;
  const CompatibilityGate gate(empty);
  REQUIRE(gate.recovery_choices().empty());
  const auto prepared =
      Success(gate.PrepareWrite(empty, snapshot, RegularFixture()));
  REQUIRE(prepared.source() == empty);
  REQUIRE(!prepared.staged_image().original);
  REQUIRE(prepared.staged_image().candidate == Encode(snapshot));
  REQUIRE(gate.source() == empty);

  const StoreImage corrupt{std::string{}, std::nullopt};
  const CompatibilityGate corrupt_gate(corrupt);
  REQUIRE(corrupt_gate.inspection(ImageSource::kOriginal).state ==
          ImageState::kCorrupt);
  REQUIRE(std::holds_alternative<Failure>(
      corrupt_gate.PrepareWrite(corrupt, snapshot, RegularFixture())));
  REQUIRE(corrupt_gate.source() == corrupt);
}

void InterruptedCandidatesRequireExplicitRecovery() {
  const auto snapshot = FixtureSnapshot();
  const auto original = Encode(snapshot);
  auto changed = snapshot;
  changed.items.front().second.saved_destination = "https://candidate.test/";
  const auto candidate = Encode(changed);
  for (const auto& interrupted :
       {candidate, candidate.substr(0, candidate.size() - 1), std::string{}}) {
    const StoreImage image{original, interrupted};
    const CompatibilityGate gate(image);
    ExpectFailure(gate.PrepareWrite(image, changed, RegularFixture()),
                  Error::kRecoveryRequired);
    const auto recovered = Success(durability::Recover(gate.source()));
    REQUIRE(recovered.snapshot.items.front().second.saved_destination ==
            "https://saved.test/");
    REQUIRE(gate.source() == image);
  }

  const StoreImage orphan{std::nullopt, candidate};
  const CompatibilityGate orphan_gate(orphan);
  REQUIRE(orphan_gate.recovery_choices() ==
          std::vector<ImageSource>{ImageSource::kCandidate});
  REQUIRE(Success(orphan_gate.ReadSnapshot(ImageSource::kCandidate))
              .items.front()
              .second.saved_destination == "https://candidate.test/");
  ExpectFailure(orphan_gate.PrepareWrite(orphan, changed, RegularFixture()),
                Error::kRecoveryRequired);
  REQUIRE(orphan_gate.source() == orphan);

  const StoreImage damaged{std::string("\0bad\0", 5), candidate};
  const CompatibilityGate damaged_gate(damaged);
  REQUIRE(damaged_gate.recovery_choices() ==
          std::vector<ImageSource>{ImageSource::kCandidate});
  REQUIRE(std::holds_alternative<Failure>(
      damaged_gate.PrepareWrite(damaged, changed, RegularFixture())));
  REQUIRE(damaged_gate.source() == damaged);
}

// These are unsupported-header fixtures, not historical schemas. Offsets and
// little endian U32 fields come from #50's actual ARCSESS\n envelope. No
// checksum repair is intentional: unsupported headers must be recognized before
// decoding.
std::string VersionHeader(std::uint32_t schema, std::uint32_t contract) {
  std::string bytes = "ARCSESS\n";
  for (const auto value : {schema, contract}) {
    for (unsigned shift = 0; shift < 32; shift += 8)
      bytes.push_back(static_cast<char>((value >> shift) & 0xff));
  }
  return bytes;
}

void UnsupportedVersionsNeverAdmitWrites() {
  const auto snapshot = FixtureSnapshot();
  const auto current = Encode(snapshot);
  struct UnsupportedFixture {
    std::uint32_t schema;
    std::uint32_t contract;
    ImageState state;
    Error error;
  };
  const UnsupportedFixture fixtures[] = {
      {durability::kSchemaVersion + 1, kContractVersion,
       ImageState::kFutureSchema, Error::kUnsupportedSchema},
      {std::numeric_limits<std::uint32_t>::max(), kContractVersion,
       ImageState::kFutureSchema, Error::kUnsupportedSchema},
      {0, kContractVersion, ImageState::kUnsupportedSchema,
       Error::kUnsupportedSchema},
      {durability::kSchemaVersion, kContractVersion - 1,
       ImageState::kUnsupportedContract, Error::kUnsupportedContract},
      {durability::kSchemaVersion, kContractVersion + 1,
       ImageState::kUnsupportedContract, Error::kUnsupportedContract},
  };
  for (const auto& fixture : fixtures) {
    const auto unsupported = VersionHeader(fixture.schema, fixture.contract);
    for (const auto slot : {ImageSource::kOriginal, ImageSource::kCandidate}) {
      const StoreImage image = slot == ImageSource::kOriginal
                                   ? StoreImage{unsupported, current}
                                   : StoreImage{current, unsupported};
      const CompatibilityGate gate(image);
      REQUIRE(gate.inspection(slot).state == fixture.state);
      REQUIRE(gate.inspection(slot).version->schema_version == fixture.schema);
      REQUIRE(gate.inspection(slot).version->contract_version ==
              fixture.contract);
      ExpectFailure(gate.PrepareWrite(image, snapshot, RegularFixture()),
                    fixture.error);
      ExpectFailure(gate.ReadSnapshot(slot), fixture.error);
      REQUIRE(gate.source() == image);
      REQUIRE(gate.recovery_choices().size() == 1);
    }
  }
}

void CorruptBodiesPreserveBytes() {
  const auto snapshot = FixtureSnapshot();
  const auto current = Encode(snapshot);
  auto checksum_mismatch = current;
  checksum_mismatch.back() ^= 1;
  for (const auto& corrupt :
       {current.substr(0, current.size() - 1), current + "trailing",
        checksum_mismatch, std::string("\0unknown\0", 9)}) {
    const StoreImage image{corrupt, std::nullopt};
    const CompatibilityGate gate(image);
    REQUIRE(gate.inspection(ImageSource::kOriginal).state ==
            ImageState::kCorrupt);
    REQUIRE(gate.recovery_choices().empty());
    REQUIRE(std::holds_alternative<Failure>(
        gate.ReadSnapshot(ImageSource::kOriginal)));
    REQUIRE(std::holds_alternative<Failure>(
        gate.PrepareWrite(image, snapshot, RegularFixture())));
    REQUIRE(gate.source() == image);
  }
}

void SharedTransactionAndStaleSources() {
  const auto snapshot = FixtureSnapshot();
  const StoreImage image{Encode(snapshot), std::nullopt};
  const CompatibilityGate gate(image);
  auto changed = snapshot;
  changed.items.front().second.saved_destination = "https://new.test/";
  const auto prepared =
      Success(gate.PrepareWrite(image, changed, RegularFixture()));
  REQUIRE(prepared.source() == image);
  REQUIRE(prepared.staged_image().original == image.original);
  REQUIRE(prepared.staged_image().candidate == Encode(changed));

  for (const auto& stale :
       {StoreImage{Encode(changed), std::nullopt},
        StoreImage{image.original, std::string{}}, StoreImage{}}) {
    ExpectFailure(gate.PrepareWrite(stale, changed, RegularFixture()),
                  Error::kStaleSource);
    REQUIRE(gate.source() == image);
  }
  auto tampered_stage = prepared.staged_image();
  tampered_stage.original = "changed source";
  ExpectFailure(durability::CommitPreparedWrite(tampered_stage, prepared,
                                                RegularFixture()),
                Error::kStaleSource);
  const auto committed = Success(durability::CommitPreparedWrite(
      prepared.staged_image(), prepared, RegularFixture()));
  REQUIRE(committed.original == Encode(changed));
  REQUIRE(!committed.candidate);
  REQUIRE(prepared.source() == image);
  REQUIRE(gate.source() == image);
}

void PrivacyAndFeatureAdmissionStayFailClosed() {
  const auto snapshot = FixtureSnapshot();
  const StoreImage image{Encode(snapshot), std::nullopt};
  const CompatibilityGate gate(image);
  auto missing = RegularFixture();
  missing.snapshot_may_persist = {};
  ExpectFailure(gate.PrepareWrite(image, snapshot, missing),
                Error::kPersistenceNotAdmitted);
  missing = RegularFixture();
  missing.profile_may_persist = {};
  ExpectFailure(gate.PrepareWrite(image, snapshot, missing),
                Error::kPersistenceNotAdmitted);
  missing = RegularFixture();
  missing.page_may_persist = {};
  ExpectFailure(gate.PrepareWrite(image, snapshot, missing),
                Error::kPersistenceNotAdmitted);
  auto denied = RegularFixture();
  denied.profile_may_persist = [](const ProfileId&) { return false; };
  ExpectFailure(gate.PrepareWrite(image, snapshot, denied),
                Error::kPersistenceNotAdmitted);
  denied = RegularFixture();
  denied.page_may_persist = [](const PageId&, const ProfileId&) {
    return false;
  };
  ExpectFailure(gate.PrepareWrite(image, snapshot, denied),
                Error::kPersistenceNotAdmitted);
  auto features_present = RegularFixture();
  features_present.unsupported_feature_payloads_absent = false;
  ExpectFailure(gate.PrepareWrite(image, snapshot, features_present),
                Error::kUnsupportedFeatures);
  auto invalid = snapshot;
  invalid.spaces.front().second.profile = ProfileId{"missing"};
  REQUIRE(std::holds_alternative<Failure>(
      gate.PrepareWrite(image, invalid, RegularFixture())));
  REQUIRE(gate.source() == image);
}

}  // namespace
}  // namespace arcified::migrations

int main() {
  using namespace arcified::migrations;
  CurrentNoopAndOwnedSource();
  EmptyIsDifferentFromCorrupt();
  InterruptedCandidatesRequireExplicitRecovery();
  UnsupportedVersionsNeverAdmitWrites();
  CorruptBodiesPreserveBytes();
  SharedTransactionAndStaleSources();
  PrivacyAndFeatureAdmissionStayFailClosed();
  std::cout << "schema compatibility fixtures passed\n";
}
