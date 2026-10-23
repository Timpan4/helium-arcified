#include "recent_files.h"

#include <algorithm>
#include <map>
#include <utility>

namespace arcified::recent_files {
namespace {

bool ValidReference(const FileReference& reference) {
  return !reference.profile.value.empty() && !reference.record.value.empty() &&
         !reference.revision.value.empty();
}

bool KnownType(FileType type) {
  switch (type) {
    case FileType::kImage:
    case FileType::kVideo:
    case FileType::kAudio:
    case FileType::kDocument:
    case FileType::kArchive:
    case FileType::kOther:
      return true;
    case FileType::kUnknown:
      return false;
  }
  return false;
}

bool KnownAction(Action action) {
  switch (action) {
    case Action::kOpen:
    case Action::kReveal:
    case Action::kCopyFile:
    case Action::kDragOut:
      return true;
  }
  return false;
}

}  // namespace

std::vector<Refusal> CheckEligibility(const lifecycle::ProfileId& scope,
                                      const FileObservation& observation,
                                      const TypeSelection& selection) {
  std::vector<Refusal> reasons;
  if (scope.value.empty() || !ValidReference(observation.reference))
    reasons.push_back(Refusal::kInvalidInput);
  if (scope != observation.reference.profile)
    reasons.push_back(Refusal::kProfileMismatch);
  if (std::any_of(selection.begin(), selection.end(),
                  [](FileType type) { return !KnownType(type); })) {
    reasons.push_back(Refusal::kInvalidInput);
  }
  if (!KnownType(observation.type)) {
    reasons.push_back(Refusal::kUnsupportedType);
  } else if (selection.count(observation.type) == 0) {
    reasons.push_back(Refusal::kTypeNotSelected);
  }

  switch (observation.completion) {
    case Completion::kComplete:
      break;
    case Completion::kPartial:
      reasons.push_back(Refusal::kPartial);
      break;
    case Completion::kUnknown:
      reasons.push_back(Refusal::kUnverified);
      break;
    default:
      reasons.push_back(Refusal::kInvalidInput);
  }
  switch (observation.safety) {
    case Safety::kAllowed:
      break;
    case Safety::kQuarantined:
      reasons.push_back(Refusal::kQuarantined);
      break;
    case Safety::kBlocked:
      reasons.push_back(Refusal::kBlocked);
      break;
    case Safety::kUnknown:
      reasons.push_back(Refusal::kUnverified);
      break;
    default:
      reasons.push_back(Refusal::kInvalidInput);
  }
  switch (observation.access) {
    case Access::kAccessible:
      break;
    case Access::kMissing:
      reasons.push_back(Refusal::kMissing);
      break;
    case Access::kDenied:
      reasons.push_back(Refusal::kDenied);
      break;
    case Access::kUnknown:
      reasons.push_back(Refusal::kUnverified);
      break;
    default:
      reasons.push_back(Refusal::kInvalidInput);
  }
  switch (observation.freshness) {
    case Freshness::kCurrent:
      break;
    case Freshness::kStale:
      reasons.push_back(Refusal::kStale);
      break;
    case Freshness::kUnknown:
      reasons.push_back(Refusal::kUnverified);
      break;
    default:
      reasons.push_back(Refusal::kInvalidInput);
  }
  if (std::any_of(observation.supported_actions.begin(),
                  observation.supported_actions.end(),
                  [](Action action) { return !KnownAction(action); })) {
    reasons.push_back(Refusal::kInvalidInput);
  }
  return reasons;
}

FilterResult FilterRecentFiles(const lifecycle::ProfileId& scope,
                               const std::vector<FileObservation>& observations,
                               const TypeSelection& selection) {
  using Key = std::pair<lifecycle::ProfileId, RecordId>;
  std::map<Key, std::size_t> counts;
  for (const auto& observation : observations)
    ++counts[{observation.reference.profile, observation.reference.record}];

  FilterResult result;
  for (std::size_t index = 0; index < observations.size(); ++index) {
    const auto& observation = observations[index];
    auto reasons = CheckEligibility(scope, observation, selection);
    if (counts.at({observation.reference.profile,
                   observation.reference.record}) > 1) {
      reasons.push_back(Refusal::kDuplicateRecord);
    }
    if (reasons.empty()) {
      result.files.push_back(observation.reference);
    } else {
      result.excluded.push_back({index, std::move(reasons)});
    }
  }
  return result;
}

Preparation PrepareAction(const lifecycle::ProfileId& scope,
                          OperationId operation,
                          const FileReference& selected,
                          Action action,
                          const FileObservation& current,
                          const TypeSelection& selection) {
  auto reasons = CheckEligibility(scope, current, selection);
  if (operation.value.empty() || !ValidReference(selected))
    reasons.push_back(Refusal::kInvalidInput);
  if (selected.profile != scope)
    reasons.push_back(Refusal::kProfileMismatch);
  if (selected.record != current.reference.record)
    reasons.push_back(Refusal::kRecordMismatch);
  if (selected.revision != current.reference.revision)
    reasons.push_back(Refusal::kStale);
  if (!KnownAction(action) || current.supported_actions.count(action) == 0)
    reasons.push_back(Refusal::kUnsupportedAction);
  if (!reasons.empty())
    return reasons;
  return PreparedAction{std::move(operation), current, action};
}

ActionResult ReconcileHostResult(const lifecycle::ProfileId& scope,
                                 const PreparedAction& prepared,
                                 const HostReport& report,
                                 const FileObservation& current,
                                 const TypeSelection& selection) {
  auto validation =
      PrepareAction(scope, prepared.operation, prepared.observation.reference,
                    prepared.action, current, selection);
  if (const auto* reasons = std::get_if<std::vector<Refusal>>(&validation))
    return *reasons;
  if (!(prepared.observation == current))
    return std::vector<Refusal>{Refusal::kStale};
  if (report.request.operation != prepared.operation ||
      !(report.request.observation == prepared.observation) ||
      report.request.action != prepared.action) {
    return std::vector<Refusal>{Refusal::kMismatchedResult};
  }
  switch (report.outcome) {
    case HostOutcome::kUnverified:
    case HostOutcome::kSubmitted:
    case HostOutcome::kCompleted:
    case HostOutcome::kCancelled:
    case HostOutcome::kUnsupported:
    case HostOutcome::kMissing:
    case HostOutcome::kDenied:
    case HostOutcome::kStale:
    case HostOutcome::kPartial:
    case HostOutcome::kQuarantined:
    case HostOutcome::kBlocked:
    case HostOutcome::kFailed:
      return report.outcome;
  }
  return std::vector<Refusal>{Refusal::kInvalidInput};
}

}  // namespace arcified::recent_files
