#include "session_snapshot.h"

#include <cstdlib>
#include <iostream>

namespace arcified::durability {
namespace {

using namespace lifecycle;

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

template <typename T>
T Value(Result<T> result) {
  if (const auto* failure = std::get_if<Failure>(&result)) {
    std::cerr << failure->context << '\n';
    std::exit(EXIT_FAILURE);
  }
  return std::get<T>(std::move(result));
}
template <typename T>
void ExpectError(const Result<T>& result, Error error) {
  REQUIRE(std::holds_alternative<Failure>(result));
  REQUIRE(std::get<Failure>(result).code == error);
}

const ProfileId kWork{"work"};
const ProfileId kHome{"home"};
const SpaceId kDesk{"desk"};
const SpaceId kOther{"other"};
const SpaceId kHomeSpace{"home-space"};
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const PageId kSavedPage{"saved-page"};
const PageId kTemporaryPage{"temporary-page"};
const ItemId kItem{"item"};
const SplitId kSplit{"split"};
const Attachment kHere{kFirst, kDesk};
const Attachment kThere{kSecond, kDesk};
const std::string kSaved = "https://example.test/saved";
const std::string kNativeSaved = "https://example.test/native-committed";
const std::string kNativeTemporary = "https://example.test/native-temporary";
const std::string kRendererOnly = "https://example.test/renderer-memory-only";

// Synthetic booleans test admission protocol only. Production must bind the
// shared ClassifyContext and exact live native context mapping, not these
// mocks.
SnapshotAdmission SyntheticAdmission() {
  return {[](const ProfileId&) { return true; },
          [](const PageId&, const ProfileId&) { return true; }, true};
}

Model Workspace() {
  Model m;
  REQUIRE(m.AddProfile(kWork) == Status::kOk);
  REQUIRE(m.AddProfile(kHome) == Status::kOk);
  REQUIRE(m.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(m.AddSpace(kOther, kWork) == Status::kOk);
  REQUIRE(m.AddSpace(kHomeSpace, kHome) == Status::kOk);
  REQUIRE(m.AddWindow(kFirst) == Status::kOk);
  REQUIRE(m.AddWindow(kSecond) == Status::kOk);
  REQUIRE(m.Attach(kHere) == Status::kOk);
  REQUIRE(m.Attach(kThere) == Status::kOk);
  REQUIRE(m.Attach({kFirst, kOther}) == Status::kOk);
  REQUIRE(m.Attach({kFirst, kHomeSpace}) == Status::kOk);
  REQUIRE(m.SelectSpace(kFirst, kHomeSpace) == Status::kOk);
  REQUIRE(m.SelectSpace(kSecond, kDesk) == Status::kOk);
  REQUIRE(m.AddItem(kItem, kDesk, kSaved) == Status::kOk);
  // Equal destinations remain distinct explicit identities.
  REQUIRE(m.AddItem(ItemId("same-url"), kDesk, kSaved) == Status::kOk);
  REQUIRE(m.AddItem(ItemId("favorite"), kWork, kSaved) == Status::kOk);
  REQUIRE(m.AddShortcut(ShortcutId("alias"), kWork, kItem) == Status::kOk);
  REQUIRE(
      std::holds_alternative<OpenedPage>(m.OpenItem(kItem, kHere, kSavedPage)));
  REQUIRE(m.RecordNavigation(kSavedPage, kRendererOnly) == Status::kOk);
  REQUIRE(m.OpenTemporary(kTemporaryPage, TemporaryTabId("temporary"), kHere,
                          kRendererOnly) == Status::kOk);
  REQUIRE(m.ReportActivity(kSavedPage, Activity::kProtected) == Status::kOk);
  REQUIRE(m.ReportActivity(kTemporaryPage, Activity::kQuiescent) ==
          Status::kOk);
  REQUIRE(m.SetSplit(kSplit, kHere, {kTemporaryPage, kSavedPage}) ==
          Status::kOk);
  return m;
}

DurableSnapshot Snapshot() {
  return Value(CaptureSnapshot(Workspace(), SyntheticAdmission()));
}

std::vector<RestoredNavigation> NativeReports() {
  return {{kSavedPage, kWork, kNativeSaved},
          {kTemporaryPage, kWork, kNativeTemporary}};
}

void RoundTripAndNativeEvidence() {
  const auto snapshot = Snapshot();
  const auto bytes = Value(EncodeSnapshot(snapshot, SyntheticAdmission()));
  REQUIRE(bytes.find(kRendererOnly) == std::string::npos);
  REQUIRE(bytes.find(kSaved) != std::string::npos);
  const auto version = Value(ProbeEnvelope(bytes));
  REQUIRE(version.schema_version == 1);
  REQUIRE(version.contract_version == 3);
  const auto decoded = Value(DecodeSnapshot(bytes));
  REQUIRE(Value(EncodeSnapshot(decoded, SyntheticAdmission())) == bytes);
  auto restored =
      Value(RestoreSnapshot(decoded, NativeReports(), SyntheticAdmission()));
  REQUIRE(restored.profiles().size() == 2);
  REQUIRE(restored.windows().size() == 2);
  REQUIRE(restored.spaces().size() == 3);
  REQUIRE(restored.items().size() == 3);
  REQUIRE(restored.pages().size() == 2);
  REQUIRE(restored.windows().at(kFirst).selected_space == kHomeSpace);
  REQUIRE(restored.windows().at(kSecond).selected_space == kDesk);
  REQUIRE(restored.items().at(kItem).saved_destination == kSaved);
  REQUIRE(restored.pages().at(kSavedPage).current_navigation == kNativeSaved);
  REQUIRE(restored.pages().at(kTemporaryPage).current_navigation ==
          kNativeTemporary);
  REQUIRE(restored.pages().at(kSavedPage).profile == kWork);
  REQUIRE(restored.pages().at(kSavedPage).activity == Activity::kUnknown);
  REQUIRE(restored.pages().at(kTemporaryPage).activity == Activity::kUnknown);
  REQUIRE(restored.TemporaryPages(kThere).empty());
  REQUIRE(restored.TemporaryPages(kHere) ==
          std::vector<PageId>{kTemporaryPage});
  REQUIRE(restored.splits().at(kSplit).pages ==
          (std::vector<PageId>{kTemporaryPage, kSavedPage}));
  REQUIRE(restored.CloseWindow(kFirst) == Status::kWorkNotQuiescent);
  auto opened = restored.OpenItem(kItem, kThere, PageId("unused"));
  REQUIRE(std::get<OpenedPage>(opened).page == kSavedPage);
  REQUIRE(std::get<OpenedPage>(opened).disposition ==
          OpenDisposition::kExisting);
  REQUIRE(restored.pages().size() == 2);
  REQUIRE(restored.pages().at(kSavedPage).attachment() == kThere);

  auto missing = NativeReports();
  missing.pop_back();
  ExpectError(RestoreSnapshot(decoded, missing, SyntheticAdmission()),
              Error::kNavigationEvidenceRequired);
  auto wrong_profile = NativeReports();
  wrong_profile.front().profile = kHome;
  ExpectError(RestoreSnapshot(decoded, wrong_profile, SyntheticAdmission()),
              Error::kNavigationEvidenceRequired);
  auto duplicate = NativeReports();
  duplicate.back() = duplicate.front();
  ExpectError(RestoreSnapshot(decoded, duplicate, SyntheticAdmission()),
              Error::kNavigationEvidenceRequired);
  auto unknown = NativeReports();
  unknown.front().page = PageId("unknown");
  ExpectError(RestoreSnapshot(decoded, unknown, SyntheticAdmission()),
              Error::kNavigationEvidenceRequired);
  auto empty = NativeReports();
  empty.front().committed_destination.clear();
  ExpectError(RestoreSnapshot(decoded, empty, SyntheticAdmission()),
              Error::kNavigationEvidenceRequired);
}

void InvalidGraphs() {
  const auto original = Snapshot();
  const auto reject = [](const DurableSnapshot& snapshot) {
    const auto failure = ValidateSnapshot(snapshot);
    REQUIRE(failure.has_value());
    REQUIRE(failure->code == Error::kInvalidGraph);
    ExpectError(EncodeSnapshot(snapshot, SyntheticAdmission()),
                Error::kInvalidGraph);
  };
  auto s = original;
  s.profiles.push_back(s.profiles.front());
  reject(s);
  s = original;
  s.spaces.front().second.profile = ProfileId("missing");
  reject(s);
  s = original;
  s.windows.front().second.selected_space = SpaceId("missing");
  reject(s);
  s = original;
  s.attachments.push_back(s.attachments.front());
  reject(s);
  s = original;
  s.items.front().second.saved_destination.clear();
  reject(s);
  s = original;
  s.shortcuts.front().second.profile = kHome;
  reject(s);
  s = original;
  s.pages.front().profile = kHome;
  reject(s);
  s = original;
  s.pages.push_back(
      {PageId("duplicate-live-item"), kWork, SavedPage{kItem, kThere}});
  reject(s);
  s = original;
  s.pages.push_back(s.pages.front());
  reject(s);
  s = original;
  for (auto& page : s.pages)
    if (auto* saved = std::get_if<SavedPage>(&page.owner))
      saved->attachment.space = kOther;
  reject(s);
  s = original;
  s.splits.push_back(s.splits.front());
  reject(s);
  s = original;
  s.splits.front().second.pages.push_back(kSavedPage);
  reject(s);
  s = original;
  s.splits.front().second.attachment = kThere;
  reject(s);
  s = original;
  s.schema_version = 0;
  REQUIRE(ValidateSnapshot(s)->code == Error::kUnsupportedSchema);
  s = original;
  s.contract_version = 0;
  REQUIRE(ValidateSnapshot(s)->code == Error::kUnsupportedContract);
}

void AdmissionFailsClosed() {
  const auto model = Workspace();
  const auto snapshot = Snapshot();
  ExpectError(CaptureSnapshot(model, {}), Error::kUnsupportedFeatures);
  auto admission = SyntheticAdmission();
  admission.profile_may_persist = {};
  ExpectError(CaptureSnapshot(model, admission),
              Error::kPersistenceNotAdmitted);
  admission = SyntheticAdmission();
  admission.page_may_persist = {};
  ExpectError(EncodeSnapshot(snapshot, admission),
              Error::kPersistenceNotAdmitted);
  admission = SyntheticAdmission();
  admission.profile_may_persist = [](const ProfileId& id) {
    return id != kHome;
  };
  // Home has no pages but still requires regular-context admission.
  ExpectError(CaptureSnapshot(model, admission),
              Error::kPersistenceNotAdmitted);
  admission = SyntheticAdmission();
  admission.page_may_persist = [](const PageId& id, const ProfileId&) {
    return id != kTemporaryPage;
  };
  ExpectError(PrepareWrite({}, snapshot, admission),
              Error::kPersistenceNotAdmitted);
  ExpectError(RestoreSnapshot(snapshot, NativeReports(), admission),
              Error::kPersistenceNotAdmitted);
  admission = SyntheticAdmission();
  admission.unsupported_feature_payloads_absent = false;
  ExpectError(EncodeSnapshot(snapshot, admission), Error::kUnsupportedFeatures);
}

void CorruptionAndInterruption() {
  const auto snapshot = Snapshot();
  const auto original = Value(EncodeSnapshot(snapshot, SyntheticAdmission()));
  auto next = snapshot;
  next.windows.front().second.selected_space = kOther;
  const auto candidate = Value(EncodeSnapshot(next, SyntheticAdmission()));
  for (std::size_t length = 0; length < candidate.size(); ++length) {
    const StoreImage interrupted{original, candidate.substr(0, length)};
    const auto retained = interrupted;
    const auto recovery = Value(Recover(interrupted));
    REQUIRE(interrupted == retained);
    REQUIRE(Value(EncodeSnapshot(recovery.snapshot, SyntheticAdmission())) ==
            original);
    REQUIRE(recovery.notices.back() ==
            RecoveryNotice::kInterruptedCandidateRetained);
    ExpectError(PrepareWrite(interrupted, next, SyntheticAdmission()),
                Error::kRecoveryRequired);
    REQUIRE(std::holds_alternative<Failure>(
        DecodeSnapshot(candidate.substr(0, length))));
  }
  // Every single-bit corruption in the complete fixture is rejected, including
  // envelope, payload, lengths, IDs, tags, and checksum bytes.
  for (std::size_t index = 0; index < original.size(); ++index) {
    for (unsigned bit = 0; bit < 8; ++bit) {
      auto corrupt = original;
      corrupt[index] ^= static_cast<char>(1u << bit);
      REQUIRE(std::holds_alternative<Failure>(DecodeSnapshot(corrupt)));
    }
  }
  ExpectError(DecodeSnapshot(original + "trailing"), Error::kMalformed);
  auto future = original;
  future[8] = 2;  // Synthetic unknown schema header, no historical format.
  REQUIRE(Value(ProbeEnvelope(future)).schema_version == 2);
  ExpectError(DecodeSnapshot(future), Error::kUnsupportedSchema);
  ExpectError(PrepareWrite({future, std::nullopt}, next, SyntheticAdmission()),
              Error::kUnsupportedSchema);
  auto contract = original;
  contract[12] = 0;
  ExpectError(DecodeSnapshot(contract), Error::kUnsupportedContract);
  const StoreImage corrupt{"corrupt", candidate};
  const auto retained = corrupt;
  ExpectError(Recover(corrupt), Error::kMalformed);
  REQUIRE(corrupt == retained);
  ExpectError(Recover({std::nullopt, candidate}), Error::kRecoveryRequired);
  ExpectError(
      PrepareWrite({"corrupt", std::nullopt}, next, SyntheticAdmission()),
      Error::kMalformed);
}

void IndependentWireFixtures() {
  // CRCs calculated independently with Node's built-in zlib.crc32. Eight empty
  // record sequences follow the fixed header. This pins checksum convention,
  // section count, version offsets, byte order, and exact end-of-input.
  std::string empty(80, '\0');
  empty.replace(0, 8, "ARCSESS\n");
  empty[8] = 1;
  empty[12] = 3;
  const auto complete = empty + std::string("\x3e\x63\xbe\xf3", 4);
  REQUIRE(Value(EncodeSnapshot(DurableSnapshot{}, SyntheticAdmission())) ==
          complete);
  REQUIRE(Value(DecodeSnapshot(complete)).pages.empty());

  // A syntactically impossible count with a VALID independent checksum must
  // reach and fail the parser, rather than merely fail the checksum gate.
  empty.replace(16, 8, std::string(8, '\xff'));
  const auto impossible_count = empty + std::string("\x00\x89\x21\x82", 4);
  ExpectError(DecodeSnapshot(impossible_count), Error::kMalformed);
}

void WriteProtocol() {
  const auto snapshot = Snapshot();
  const auto first = Value(PrepareWrite({}, snapshot, SyntheticAdmission()));
  REQUIRE(!first.source().original);
  REQUIRE(!first.staged_image().original);
  REQUIRE(first.staged_image().candidate.has_value());
  ExpectError(Recover(first.staged_image()), Error::kRecoveryRequired);
  const auto committed =
      Value(CommitPreparedWrite(first.staged_image(), first));
  REQUIRE(committed.original.has_value());
  REQUIRE(!committed.candidate);
  const auto second =
      Value(PrepareWrite(committed, snapshot, SyntheticAdmission()));
  REQUIRE(second.source() == committed);
  REQUIRE(second.staged_image().original == committed.original);
  auto partial = second.staged_image();
  partial.candidate->pop_back();
  ExpectError(CommitPreparedWrite(partial, second), Error::kStaleSource);
  auto stale = second.staged_image();
  stale.original = "changed by another writer";
  ExpectError(CommitPreparedWrite(stale, second), Error::kStaleSource);
  const auto result = Value(CommitPreparedWrite(second.staged_image(), second));
  REQUIRE(Value(EncodeSnapshot(Value(Recover(result)).snapshot,
                               SyntheticAdmission())) == *committed.original);
  REQUIRE(second.source() == committed);  // Exact originals remain accessible.
}

}  // namespace
}  // namespace arcified::durability

int main() {
  arcified::durability::RoundTripAndNativeEvidence();
  arcified::durability::InvalidGraphs();
  arcified::durability::AdmissionFailsClosed();
  arcified::durability::CorruptionAndInterruption();
  arcified::durability::IndependentWireFixtures();
  arcified::durability::WriteProtocol();
  std::cout << "session durability synthetic checks passed\n";
}
