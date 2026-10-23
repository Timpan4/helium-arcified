#ifndef ARCIFIED_NATIVE_FEATURES_PINS_SAVED_ITEM_H_
#define ARCIFIED_NATIVE_FEATURES_PINS_SAVED_ITEM_H_

#include <optional>
#include <string>
#include <variant>

#include "../../model/lifecycle.h"

namespace arcified::pins {

// Caller-owned presentation values, keyed by canonical lifecycle::ItemId.
// No owner, saved destination, navigation, icon, order or persistence state.
struct SavedItemMetadata {
  std::optional<std::string> custom_label;
  friend bool operator==(const SavedItemMetadata& a,
                         const SavedItemMetadata& b) {
    return a.custom_label == b.custom_label;
  }
  friend bool operator!=(const SavedItemMetadata& a,
                         const SavedItemMetadata& b) {
    return !(a == b);
  }
};

enum class SavedItemError { kItemNotFound, kEmptyLabel, kStaleMetadata };

// Owned presentation snapshot. Recompute after model/metadata/title changes.
// A live_page is an identity reference, never an active/focused-page decision.
struct SavedItemPresentation {
  lifecycle::ItemId item;
  std::string label;
  bool uses_custom_label;
  std::optional<lifecycle::PageId> live_page;
};
using PresentationResult = std::variant<SavedItemError, SavedItemPresentation>;

// Context-free canonical lookup, not a visibility/authorization check. The
// caller validates its Space/Profile/shortcut presentation through Model first.
// Aliases use Shortcut.target and the same metadata as the kept item. The
// caller supplies a nonempty native/default display label; this is not stored.
PresentationResult DescribeSavedItem(const lifecycle::Model& model,
                                     lifecycle::ItemId item,
                                     const SavedItemMetadata& metadata,
                                     std::string default_label);

struct LabelEdit {
  lifecycle::ItemId item;
  SavedItemMetadata before;
  SavedItemMetadata after;
};
using LabelEditResult = std::variant<SavedItemError, LabelEdit>;
enum class EditDirection { kForward, kReverse };
using MetadataResult = std::variant<SavedItemError, SavedItemMetadata>;

// nullopt restores the caller's current default label. Empty strings are
// rejected rather than ambiguously encoding that operation. A no-op edit is
// valid; the caller decides whether it belongs in a future operation journal.
LabelEditResult PlanLabelEdit(const lifecycle::Model& model,
                              lifecycle::ItemId item,
                              const SavedItemMetadata& current,
                              std::optional<std::string> custom_label);

// Returns a replacement value only when the item still exists and current
// matches the expected side of edit. Caller serializes read/replace and owns
// all values. This is not a journal, persistence commit or native undo policy.
MetadataResult ApplyLabelEdit(const lifecycle::Model& model,
                              const LabelEdit& edit,
                              const SavedItemMetadata& current,
                              EditDirection direction);

}  // namespace arcified::pins

#endif  // ARCIFIED_NATIVE_FEATURES_PINS_SAVED_ITEM_H_
