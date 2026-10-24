#include "recent_files.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace arcified::recent_files {
namespace {

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const lifecycle::ProfileId kWork{"synthetic-work"};
const lifecycle::ProfileId kOther{"synthetic-other-context"};
const TypeSelection kImages{FileType::kImage};
const OperationId kOperation{"synthetic-operation"};
static_assert(lifecycle::kContractVersion == 3);
static_assert(!std::is_convertible_v<RecordId, lifecycle::ItemId>);
static_assert(!std::is_convertible_v<Revision, OperationId>);

template <typename Enum>
Enum InvalidBoundaryEnum() {
  // Deliberately model an unsupported native value to test fail-closed input.
  // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
  return static_cast<Enum>(-1);
}

FileObservation Accessible(std::string record = "synthetic-file") {
  FileObservation observation{
      {kWork, RecordId{std::move(record)}, Revision{"synthetic-revision"}}};
  observation.type = FileType::kImage;
  observation.completion = Completion::kComplete;
  observation.safety = Safety::kAllowed;
  observation.access = Access::kAccessible;
  observation.freshness = Freshness::kCurrent;
  observation.supported_actions = {Action::kOpen, Action::kReveal,
                                   Action::kCopyFile, Action::kDragOut};
  return observation;
}

bool Contains(const std::vector<Refusal>& reasons, Refusal reason) {
  return std::find(reasons.begin(), reasons.end(), reason) != reasons.end();
}

template <typename Result>
void ExpectRefusal(const Result& result, Refusal reason) {
  const auto* reasons = std::get_if<std::vector<Refusal>>(&result);
  REQUIRE(reasons != nullptr);
  REQUIRE(Contains(*reasons, reason));
}

PreparedAction Prepare(const FileObservation& observation,
                       Action action = Action::kOpen) {
  auto result = PrepareAction(kWork, kOperation, observation.reference, action,
                              observation, kImages);
  REQUIRE(std::holds_alternative<PreparedAction>(result));
  return std::get<PreparedAction>(result);
}

void FilteringAndExplicitTypes() {
  const auto first = Accessible("first");
  const auto second = Accessible("second");
  auto document = Accessible("document");
  document.type = FileType::kDocument;
  auto foreign = Accessible("foreign");
  foreign.reference.profile = kOther;
  const auto filtered =
      FilterRecentFiles(kWork, {second, document, foreign, first}, kImages);
  REQUIRE(filtered.files ==
          (std::vector<FileReference>{second.reference, first.reference}));
  REQUIRE(filtered.excluded.size() == 2);
  REQUIRE(filtered.excluded[0].input_index == 1);
  REQUIRE(Contains(filtered.excluded[0].reasons, Refusal::kTypeNotSelected));
  REQUIRE(filtered.excluded[1].input_index == 2);
  REQUIRE(Contains(filtered.excluded[1].reasons, Refusal::kProfileMismatch));
  REQUIRE(FilterRecentFiles(kWork, {first}, {}).files.empty());
  REQUIRE(FilterRecentFiles(kWork, {}, kImages).files.empty());
  for (auto type :
       {FileType::kImage, FileType::kVideo, FileType::kAudio,
        FileType::kDocument, FileType::kArchive, FileType::kOther}) {
    auto observation = Accessible();
    observation.type = type;
    REQUIRE(FilterRecentFiles(kWork, {observation}, {type}).files.size() == 1);
  }
}

void UnsafeAndUnavailableStates() {
  auto observation = Accessible();
  observation.completion = Completion::kPartial;
  observation.safety = Safety::kQuarantined;
  observation.access = Access::kDenied;
  observation.freshness = Freshness::kStale;
  const auto reasons = CheckEligibility(kWork, observation, kImages);
  REQUIRE(Contains(reasons, Refusal::kPartial));
  REQUIRE(Contains(reasons, Refusal::kQuarantined));
  REQUIRE(Contains(reasons, Refusal::kDenied));
  REQUIRE(Contains(reasons, Refusal::kStale));
  REQUIRE(FilterRecentFiles(kWork, {observation}, kImages).files.empty());
  observation = Accessible();
  observation.safety = Safety::kBlocked;
  observation.access = Access::kMissing;
  REQUIRE(Contains(CheckEligibility(kWork, observation, kImages),
                   Refusal::kBlocked));
  REQUIRE(Contains(CheckEligibility(kWork, observation, kImages),
                   Refusal::kMissing));
  for (auto action :
       {Action::kOpen, Action::kReveal, Action::kCopyFile, Action::kDragOut}) {
    ExpectRefusal(PrepareAction(kWork, kOperation, observation.reference,
                                action, observation, kImages),
                  Refusal::kBlocked);
  }
  FileObservation unknown{Accessible().reference};
  REQUIRE(Contains(CheckEligibility(kWork, unknown, kImages),
                   Refusal::kUnverified));
  REQUIRE(FilterRecentFiles(kWork, {unknown}, kImages).files.empty());
}

void InvalidBoundaryFacts() {
  for (auto field : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) {
    auto observation = Accessible();
    switch (field) {
      case 0:
        observation.reference.profile = lifecycle::ProfileId{""};
        break;
      case 1:
        observation.reference.record = RecordId{""};
        break;
      case 2:
        observation.reference.revision = Revision{""};
        break;
      case 3:
        observation.type = InvalidBoundaryEnum<FileType>();
        break;
      case 4:
        observation.completion = InvalidBoundaryEnum<Completion>();
        break;
      case 5:
        observation.safety = InvalidBoundaryEnum<Safety>();
        break;
      case 6:
        observation.access = InvalidBoundaryEnum<Access>();
        break;
      case 7:
        observation.freshness = InvalidBoundaryEnum<Freshness>();
        break;
      case 8:
        observation.supported_actions.insert(InvalidBoundaryEnum<Action>());
        break;
      case 9:
        observation.type = FileType::kUnknown;
        break;
      case 10:
        observation.access = Access::kUnknown;
        break;
    }
    REQUIRE(!CheckEligibility(kWork, observation, kImages).empty());
    REQUIRE(FilterRecentFiles(kWork, {observation}, kImages).files.empty());
    REQUIRE(std::holds_alternative<std::vector<Refusal>>(
        PrepareAction(kWork, kOperation, observation.reference, Action::kOpen,
                      observation, kImages)));
  }
  const auto observation = Accessible();
  REQUIRE(!CheckEligibility(lifecycle::ProfileId{""}, observation, kImages)
               .empty());
  REQUIRE(
      !CheckEligibility(kWork, observation, {InvalidBoundaryEnum<FileType>()})
           .empty());
  REQUIRE(!CheckEligibility(kWork, observation, {FileType::kUnknown}).empty());
}

void DuplicateRecordsNeverChooseASafeLookingCopy() {
  const auto original = Accessible();
  auto newer = original;
  newer.reference.revision = Revision{"newer"};
  newer.safety = Safety::kBlocked;
  const auto filtered = FilterRecentFiles(kWork, {original, newer}, kImages);
  REQUIRE(filtered.files.empty());
  REQUIRE(filtered.excluded.size() == 2);
  for (const auto& excluded : filtered.excluded)
    REQUIRE(Contains(excluded.reasons, Refusal::kDuplicateRecord));
  auto foreign = original;
  foreign.reference.profile = kOther;
  REQUIRE(FilterRecentFiles(kWork, {original, foreign}, kImages).files ==
          (std::vector<FileReference>{original.reference}));
}

void PreparationChecksIdentityScopeAndCapability() {
  auto current = Accessible();
  const auto selected = current.reference;
  for (auto action : current.supported_actions) {
    const auto prepared = Prepare(current, action);
    REQUIRE(prepared.observation.reference == selected);
    REQUIRE(prepared.action == action);
  }
  current.supported_actions.erase(Action::kDragOut);
  ExpectRefusal(PrepareAction(kWork, kOperation, selected, Action::kDragOut,
                              current, kImages),
                Refusal::kUnsupportedAction);
  ExpectRefusal(PrepareAction(kOther, kOperation, selected, Action::kOpen,
                              current, kImages),
                Refusal::kProfileMismatch);
  current.reference.revision = Revision{"changed"};
  ExpectRefusal(PrepareAction(kWork, kOperation, selected, Action::kOpen,
                              current, kImages),
                Refusal::kStale);
  current.reference.record = RecordId{"replacement"};
  ExpectRefusal(PrepareAction(kWork, kOperation, selected, Action::kOpen,
                              current, kImages),
                Refusal::kRecordMismatch);
  current = Accessible();
  ExpectRefusal(PrepareAction(kWork, OperationId{""}, selected, Action::kOpen,
                              current, kImages),
                Refusal::kInvalidInput);
  ExpectRefusal(PrepareAction(kWork, kOperation, selected,
                              InvalidBoundaryEnum<Action>(), current, kImages),
                Refusal::kUnsupportedAction);
  auto invalid = selected;
  invalid.revision = Revision{""};
  ExpectRefusal(PrepareAction(kWork, kOperation, invalid, Action::kOpen,
                              current, kImages),
                Refusal::kInvalidInput);
}

void NativeSubmissionIsNotCompletion() {
  const auto current = Accessible();
  const auto prepared = Prepare(current);
  HostReport report{prepared};
  REQUIRE(std::get<HostOutcome>(
              ReconcileHostResult(kWork, prepared, report, current, kImages)) ==
          HostOutcome::kUnverified);
  for (auto outcome :
       {HostOutcome::kSubmitted, HostOutcome::kCompleted,
        HostOutcome::kCancelled, HostOutcome::kUnsupported,
        HostOutcome::kMissing, HostOutcome::kDenied, HostOutcome::kStale,
        HostOutcome::kPartial, HostOutcome::kQuarantined, HostOutcome::kBlocked,
        HostOutcome::kFailed}) {
    report.outcome = outcome;
    REQUIRE(std::get<HostOutcome>(ReconcileHostResult(
                kWork, prepared, report, current, kImages)) == outcome);
  }
  report.outcome = InvalidBoundaryEnum<HostOutcome>();
  ExpectRefusal(ReconcileHostResult(kWork, prepared, report, current, kImages),
                Refusal::kInvalidInput);
}

void UnrelatedAndLateResultsCannotSucceed() {
  const auto current = Accessible();
  const auto prepared = Prepare(current);
  const HostReport completed{prepared, HostOutcome::kCompleted};
  auto report = completed;
  report.request.operation = OperationId{"another-operation"};
  ExpectRefusal(ReconcileHostResult(kWork, prepared, report, current, kImages),
                Refusal::kMismatchedResult);
  report = completed;
  report.request.action = Action::kReveal;
  ExpectRefusal(ReconcileHostResult(kWork, prepared, report, current, kImages),
                Refusal::kMismatchedResult);
  report = completed;
  report.request.observation.reference.profile = kOther;
  ExpectRefusal(ReconcileHostResult(kWork, prepared, report, current, kImages),
                Refusal::kMismatchedResult);
  ExpectRefusal(
      ReconcileHostResult(kOther, prepared, completed, current, kImages),
      Refusal::kProfileMismatch);
  auto changed = current;
  changed.reference.revision = Revision{"changed"};
  ExpectRefusal(
      ReconcileHostResult(kWork, prepared, completed, changed, kImages),
      Refusal::kStale);
  changed = current;
  changed.supported_actions.erase(Action::kDragOut);
  ExpectRefusal(
      ReconcileHostResult(kWork, prepared, completed, changed, kImages),
      Refusal::kStale);
  changed = current;
  changed.access = Access::kMissing;
  ExpectRefusal(
      ReconcileHostResult(kWork, prepared, completed, changed, kImages),
      Refusal::kMissing);
  changed.access = Access::kDenied;
  ExpectRefusal(
      ReconcileHostResult(kWork, prepared, completed, changed, kImages),
      Refusal::kDenied);
  changed = current;
  changed.completion = Completion::kPartial;
  ExpectRefusal(
      ReconcileHostResult(kWork, prepared, completed, changed, kImages),
      Refusal::kPartial);
  changed = current;
  changed.safety = Safety::kQuarantined;
  ExpectRefusal(
      ReconcileHostResult(kWork, prepared, completed, changed, kImages),
      Refusal::kQuarantined);
  changed.safety = Safety::kBlocked;
  ExpectRefusal(
      ReconcileHostResult(kWork, prepared, completed, changed, kImages),
      Refusal::kBlocked);
}

}  // namespace
}  // namespace arcified::recent_files

int main() {
  using namespace arcified::recent_files;
  FilteringAndExplicitTypes();
  UnsafeAndUnavailableStates();
  InvalidBoundaryFacts();
  DuplicateRecordsNeverChooseASafeLookingCopy();
  PreparationChecksIdentityScopeAndCapability();
  NativeSubmissionIsNotCompletion();
  UnrelatedAndLateResultsCannotSucceed();
  std::cout << "recent-files: all fixtures passed\n";
}
