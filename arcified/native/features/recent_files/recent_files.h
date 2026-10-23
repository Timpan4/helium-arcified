#ifndef ARCIFIED_NATIVE_FEATURES_RECENT_FILES_RECENT_FILES_H_
#define ARCIFIED_NATIVE_FEATURES_RECENT_FILES_RECENT_FILES_H_

#include <cstddef>
#include <set>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::recent_files {

using RecordId = lifecycle::Id<struct RecordTag>;
using Revision = lifecycle::Id<struct RevisionTag>;
using OperationId = lifecycle::Id<struct OperationTag>;

// Borrowed native identity, never a path or a second persisted record. The
// caller maps actual native Profile/OTR contexts to distinct lifecycle IDs.
struct FileReference {
  lifecycle::ProfileId profile;
  RecordId record;
  Revision revision;
  friend bool operator==(const FileReference& a, const FileReference& b) {
    return a.profile == b.profile && a.record == b.record &&
           a.revision == b.revision;
  }
};

enum class FileType {
  kUnknown,
  kImage,
  kVideo,
  kAudio,
  kDocument,
  kArchive,
  kOther
};
enum class Completion { kUnknown, kPartial, kComplete };
enum class Safety { kUnknown, kAllowed, kQuarantined, kBlocked };
enum class Access { kUnknown, kAccessible, kMissing, kDenied };
enum class Freshness { kUnknown, kCurrent, kStale };
enum class Action { kOpen, kReveal, kCopyFile, kDragOut };

struct FileObservation {
  FileReference reference;
  FileType type = FileType::kUnknown;
  Completion completion = Completion::kUnknown;
  Safety safety = Safety::kUnknown;
  Access access = Access::kUnknown;
  Freshness freshness = Freshness::kUnknown;
  std::set<Action> supported_actions = {};
  friend bool operator==(const FileObservation& a, const FileObservation& b) {
    return a.reference == b.reference && a.type == b.type &&
           a.completion == b.completion && a.safety == b.safety &&
           a.access == b.access && a.freshness == b.freshness &&
           a.supported_actions == b.supported_actions;
  }
};

// Explicit caller/user selection. Empty means show no files. The native
// provider classifies types; this component never guesses from a filename.
using TypeSelection = std::set<FileType>;

enum class Refusal {
  kInvalidInput,
  kProfileMismatch,
  kRecordMismatch,
  kStale,
  kUnverified,
  kPartial,
  kQuarantined,
  kBlocked,
  kMissing,
  kDenied,
  kUnsupportedType,
  kTypeNotSelected,
  kUnsupportedAction,
  kDuplicateRecord,
  kMismatchedResult,
};

// All applicable state exclusions are retained, rather than hiding a safety
// failure behind progress or vice versa. Invalid enums fail closed.
std::vector<Refusal> CheckEligibility(const lifecycle::ProfileId& scope,
                                      const FileObservation& observation,
                                      const TypeSelection& selection);

struct ExcludedFile {
  std::size_t input_index;
  std::vector<Refusal> reasons;
};
struct FilterResult {
  std::vector<FileReference> files;
  std::vector<ExcludedFile> excluded;
};

// Preserve caller-provided recency order, with no invented age/count cutoff.
// Duplicate native keys within a Profile exclude every duplicate, including
// conflicting revisions. Rejections expose input indexes, not foreign IDs.
FilterResult FilterRecentFiles(const lifecycle::ProfileId& scope,
                               const std::vector<FileObservation>& observations,
                               const TypeSelection& selection);

// Owned snapshot only, not permission to bypass native policy or a reservation.
struct PreparedAction {
  OperationId operation;
  FileObservation observation;
  Action action;
};
using Preparation = std::variant<std::vector<Refusal>, PreparedAction>;

Preparation PrepareAction(const lifecycle::ProfileId& scope,
                          OperationId operation,
                          const FileReference& selected,
                          Action action,
                          const FileObservation& current,
                          const TypeSelection& selection);

enum class HostOutcome {
  kUnverified,
  kSubmitted,
  kCompleted,
  kCancelled,
  kUnsupported,
  kMissing,
  kDenied,
  kStale,
  kPartial,
  kQuarantined,
  kBlocked,
  kFailed,
};
struct HostReport {
  PreparedAction request;
  HostOutcome outcome = HostOutcome::kUnverified;
};
using ActionResult = std::variant<std::vector<Refusal>, HostOutcome>;

// Revalidate current observation and exact request identity before returning
// the host's report. Submission stays submission. No OS effect is performed or
// inferred here. Caller serializes operations and consumes each report once.
ActionResult ReconcileHostResult(const lifecycle::ProfileId& scope,
                                 const PreparedAction& prepared,
                                 const HostReport& report,
                                 const FileObservation& current,
                                 const TypeSelection& selection);

}  // namespace arcified::recent_files

#endif  // ARCIFIED_NATIVE_FEATURES_RECENT_FILES_RECENT_FILES_H_
