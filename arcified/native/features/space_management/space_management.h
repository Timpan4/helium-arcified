#ifndef ARCIFIED_NATIVE_FEATURES_SPACE_MANAGEMENT_SPACE_MANAGEMENT_H_
#define ARCIFIED_NATIVE_FEATURES_SPACE_MANAGEMENT_SPACE_MANAGEMENT_H_

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::space_management {

// Display values only. Missing values request caller-provided presentation
// defaults; an explicitly empty string is retained. Keys identify resources,
// not URLs or files to load here. No name uniqueness or size policy is implied.
struct DisplayIdentity {
  std::optional<std::string> name;
  std::optional<std::string> icon_key;
  std::optional<std::string> theme_key;

  friend bool operator==(const DisplayIdentity& a, const DisplayIdentity& b) {
    return a.name == b.name && a.icon_key == b.icon_key &&
           a.theme_key == b.theme_key;
  }
};

struct SpaceMetadata {
  lifecycle::SpaceId space;
  DisplayIdentity identity;

  friend bool operator==(const SpaceMetadata& a, const SpaceMetadata& b) {
    return a.space == b.space && a.identity == b.identity;
  }
};

// Caller-owned value. Sequence is explicit display order, independent of
// Profile, window selection and Model map order. This is not a registry or an
// on-disk schema. Model remains the sole authority for whether a Space exists.
using Metadata = std::vector<SpaceMetadata>;

enum class MetadataErrorCode {
  kDuplicateSpace,
  kUnknownSpace,
  kMissingSpace,
};
struct MetadataError {
  MetadataErrorCode code;
  lifecycle::SpaceId space;
};

// Requires exactly one record for each Model Space, including unattached ones.
// Errors describe duplicate/unknown input first, then missing Model Spaces.
std::optional<MetadataError> ValidateMetadata(const lifecycle::Model& model,
                                              const Metadata& metadata);

struct Reconciliation {
  Metadata metadata;
  // Unknown records retain their payload for caller review/recovery. Removing
  // them from the returned projection never removes anything from Model.
  Metadata removed;
  // Missing Spaces append in Model iteration order with unspecified display
  // values. This fallback is reported explicitly, not inferred user intent.
  std::vector<lifecycle::SpaceId> added;
};
using ReconciliationResult = std::variant<MetadataError, Reconciliation>;

// Duplicate IDs are ambiguous and refused, including duplicate unknown IDs.
// Known records retain their relative order and exact display values.
ReconciliationResult ReconcileMetadata(const lifecycle::Model& model,
                                       Metadata metadata);

using MetadataResult = std::variant<MetadataError, Metadata>;

// These pure transformations require already reconciled metadata. Owned inputs
// support IDs/values borrowed from caller metadata. Originals remain available
// for cancellation or failed publication; there is no journal or IO here.
MetadataResult ReplaceIdentity(const lifecycle::Model& model,
                               Metadata metadata,
                               lifecycle::SpaceId space,
                               DisplayIdentity identity);
MetadataResult ReorderSpaces(const lifecycle::Model& model,
                             Metadata metadata,
                             std::vector<lifecycle::SpaceId> order);

// Owned, advisory reference snapshot, not a disposition plan, reservation or
// proof of native success. There is no eligibility flag or destructive action.
// Folders, routing rules and native-only references are outside this inventory.
struct SpaceReferences {
  lifecycle::SpaceId space;
  std::vector<lifecycle::ItemId> kept_items;
  // Includes temporary and saved pages, including Profile-owned shortcuts
  // presented in this Space. Attachment does not imply Space ownership.
  std::vector<lifecycle::PageId> attached_pages;
  std::vector<lifecycle::ShortcutId> aliases_of_kept_items;
  std::vector<lifecycle::Attachment> attachments;
  std::vector<lifecycle::SplitId> splits;
  std::vector<lifecycle::WindowId> selected_by;
};

// Missing Space returns nullopt. Re-read Model when needed; a snapshot can be
// stale after any lifecycle mutation and conveys no native authorization.
std::optional<SpaceReferences> InspectReferences(const lifecycle::Model& model,
                                                 lifecycle::SpaceId space);

}  // namespace arcified::space_management

#endif  // ARCIFIED_NATIVE_FEATURES_SPACE_MANAGEMENT_SPACE_MANAGEMENT_H_
