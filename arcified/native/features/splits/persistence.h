#ifndef ARCIFIED_NATIVE_FEATURES_SPLITS_PERSISTENCE_H_
#define ARCIFIED_NATIVE_FEATURES_SPLITS_PERSISTENCE_H_

#include <map>
#include <optional>
#include <variant>
#include <vector>

#include "layout.h"

namespace arcified::splits {

// The durability owner validates and supplies this association after restoring
// lifecycle membership. No implicit identity mapping or ID allocation occurs.
using RestoredPageMap = std::map<lifecycle::PageId, lifecycle::PageId>;

enum class RestoreIssueReason {
  kInvalidReference,
  kMissingMapping,
  kPageNotRestored,
  kAttachmentMismatch,
  kPageNotInSplit,
  kInvalidWeight,
  kAmbiguousMapping,
};

struct RestoreIssue {
  lifecycle::PageId source;
  std::optional<lifecycle::PageId> target;
  RestoreIssueReason reason;
};

struct RestoredLayout {
  LayoutMetadata metadata;
  std::vector<RestoreIssue> issues;
};
using RestoreResult = std::variant<LayoutError, RestoredLayout>;

// Reconciles metadata already decoded by the durability owner. The caller
// resolves the saved SplitId/Attachment to restored_split before this call.
// Model::splits() exclusively determines membership, attachment and order.
// Missing/invalid/ambiguous weights recover through ReconcileLayout's default;
// valid survivor weights remain exact relative values. Distinct saved sources
// mapping to one current member invalidate all their competing weights.
// Invalid orientation/missing split returns LayoutError without changing the
// model. Selection, schema versions, wire encoding, navigation and IO belong to
// external owners. Work and storage are bounded by the supplied graph/metadata.
RestoreResult ReconcileRestoredLayout(const lifecycle::Model& model,
                                      const lifecycle::SplitId& restored_split,
                                      const LayoutMetadata& saved_layout,
                                      const RestoredPageMap& restored_pages);

}  // namespace arcified::splits

#endif  // ARCIFIED_NATIVE_FEATURES_SPLITS_PERSISTENCE_H_
