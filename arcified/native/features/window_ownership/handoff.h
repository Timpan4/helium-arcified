#ifndef ARCIFIED_NATIVE_FEATURES_WINDOW_OWNERSHIP_HANDOFF_H_
#define ARCIFIED_NATIVE_FEATURES_WINDOW_OWNERSHIP_HANDOFF_H_

#include <optional>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::window_ownership {

enum class HandoffCommand { kTransferSavedPage, kOpenItem };
enum class HandoffRefusal {
  kPageNotFound,
  kPageNotSaved,
  kItemNotFound,
  kItemNotLive,
  kNotAttached,
  kProfileMismatch,
  kWrongOwningSpace,
  kInvalidActivePage,
};

// Only references affected by TransferSavedPage. Empty remaining_pages means
// the Model removes this layout; a single remaining reference is retained.
struct SplitEffect {
  lifecycle::SplitId split;
  lifecycle::Attachment attachment;
  std::vector<lifecycle::PageId> before_pages;
  std::vector<lifecycle::PageId> remaining_pages;
  friend bool operator==(const SplitEffect& a, const SplitEffect& b);
};

struct FocusIntent {
  // Supplied by the native caller, never inferred from Model/map order.
  std::optional<lifecycle::PageId> source_active;
  // Reference to present if the caller completes native transfer/selection.
  // This does not activate it or choose a replacement active source page.
  lifecycle::PageId destination_page;
};

// Advisory owned snapshot, not a reservation, authorization or native success.
// No navigation, renderer state, activity, persistence or selected-page store.
// Keep this value unchanged between preparation and revalidation.
struct HandoffPlan {
  HandoffCommand command;
  lifecycle::PageId page;
  lifecycle::ItemId item;
  lifecycle::ProfileId profile;
  lifecycle::Attachment source;
  lifecycle::Attachment requested_destination;
  lifecycle::Attachment destination;
  std::optional<lifecycle::SpaceId> source_selected_space;
  std::optional<lifecycle::SpaceId> destination_selected_space;
  FocusIntent focus;
  std::vector<SplitEffect> split_effects;
};

using HandoffResult = std::variant<HandoffRefusal, HandoffPlan>;

// Preconditions/destination match Model::TransferSavedPage. Temporary pages
// are refused. Same-attachment preparation has no split effects.
HandoffResult PrepareSavedPageHandoff(
    const lifecycle::Model& model,
    lifecycle::PageId page,
    lifecycle::Attachment destination,
    std::optional<lifecycle::PageId> native_source_active);

// Only the already-live branch of Model::OpenItem. A Space-owned item resolves
// to its owning Space in the requesting window; both attachments must exist.
// Closed-item creation and alias lookup stay with the existing Model APIs.
HandoffResult PrepareOpenItemHandoff(
    const lifecycle::Model& model,
    lifecycle::ItemId item,
    lifecycle::Attachment requesting,
    std::optional<lifecycle::PageId> native_source_active);

enum class Revalidation {
  kCurrent,
  kSourceChanged,
  kDestinationChanged,
  kSelectionChanged,
  kFocusChanged,
  kSplitEffectsChanged,
};

// Reads the current Model and a fresh native active-page reference. Navigation
// and activity may evolve without invalidating the plan. This cannot detect
// native handle replacement, shutdown, ABA events or synchronous callbacks.
// The caller serializes native/model operations and revalidates native objects
// separately. No semantic or native transition is committed here.
Revalidation RevalidateHandoff(
    const lifecycle::Model& model,
    const HandoffPlan& plan,
    std::optional<lifecycle::PageId> native_source_active);

}  // namespace arcified::window_ownership

#endif  // ARCIFIED_NATIVE_FEATURES_WINDOW_OWNERSHIP_HANDOFF_H_
