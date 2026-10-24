#ifndef ARCIFIED_NATIVE_FEATURES_WORKSPACE_IMPORT_PLANNER_H_
#define ARCIFIED_NATIVE_FEATURES_WORKSPACE_IMPORT_PLANNER_H_

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "../portability/format.h"

namespace arcified::workspace_import {

// Match-existing asserts a caller-retained association, never URL equality.
enum class Intent { kCreate, kMatchExisting };
template <typename Id>
struct Destination {
  Id id;
  Intent intent;
};
struct Mapping {
  std::map<lifecycle::ProfileId, lifecycle::ProfileId> profiles;
  std::map<lifecycle::SpaceId, Destination<lifecycle::SpaceId>> spaces;
  std::map<lifecycle::ItemId, Destination<lifecycle::ItemId>> items;
  std::map<lifecycle::ShortcutId, Destination<lifecycle::ShortcutId>> shortcuts;
};

enum class Outcome { kMapped, kNew, kDuplicate, kConflict, kUnmapped };
enum class Reason {
  kNone,
  kMissingMapping,
  kUnknownSource,
  kEmptyDestinationId,
  kDestinationMissing,
  kDestinationExists,
  kProfileMismatch,
  kContentMismatch,
  kTargetCollision,
  kDependencyBlocked,
};
enum class RecordKind { kProfile, kSpace, kItem, kShortcut };
struct MappingProblem {
  RecordKind kind;
  Reason reason;
};
template <typename Id, typename Payload>
struct RecordPlan {
  Id source;
  std::optional<Id> destination;
  std::optional<Payload> proposed;
  Outcome outcome;
  Reason reason;
};
using ProfilePlan = RecordPlan<lifecycle::ProfileId, lifecycle::Profile>;
using SpacePlan = RecordPlan<lifecycle::SpaceId, lifecycle::Space>;
using ItemPlan = RecordPlan<lifecycle::ItemId, lifecycle::PersistentItem>;
using ShortcutPlan = RecordPlan<lifecycle::ShortcutId, lifecycle::Shortcut>;

enum class UnmappedField { kName, kSavedAt, kProvenance, kRecordOrder };
struct MetadataNotice {
  RecordKind kind;
  // Index into the corresponding source vector. Record order has no index.
  std::optional<std::size_t> index;
  UnmappedField field;
};

struct Manifest {
  // Own the validated copy, including labels, timestamps, order and provenance.
  // Provenance is untrusted attribution, not proof of an import association.
  portability::Workspace source;
  std::vector<ProfilePlan> profiles;
  std::vector<SpacePlan> spaces;
  std::vector<ItemPlan> items;
  std::vector<ShortcutPlan> shortcuts;
  std::vector<MappingProblem> mapping_problems;
  std::vector<MetadataNotice> unmapped_metadata;

  bool HasUnresolvedRecords() const;
};
using PlanResult = std::variant<Manifest, portability::Error>;

// Pure previews. Validate before planning, including typed DTO callers. Rows
// are not commands, reservations, native URL approvals or transaction tokens.
// Call on the model's serialized owner sequence. Replan after destination
// edits.
PlanResult Plan(const portability::Workspace& source,
                const lifecycle::Model& destination,
                const Mapping& mapping);
PlanResult PreviewCopy(std::string_view bytes,
                       const lifecycle::Model& destination,
                       const Mapping& mapping);

}  // namespace arcified::workspace_import

#endif  // ARCIFIED_NATIVE_FEATURES_WORKSPACE_IMPORT_PLANNER_H_
