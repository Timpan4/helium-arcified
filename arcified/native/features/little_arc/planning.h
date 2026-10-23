#ifndef ARCIFIED_NATIVE_FEATURES_LITTLE_ARC_PLANNING_H_
#define ARCIFIED_NATIVE_FEATURES_LITTLE_ARC_PLANNING_H_

#include "../../model/lifecycle.h"

namespace arcified::little_arc {

// Caller-resolved identity snapshot, not routing or Profile ownership state.
// An integrated caller adapts its resolver output; this module has no rule
// store.
struct ResolvedTarget {
  lifecycle::ProfileId profile;
  lifecycle::Attachment attachment;
};

// Browser URL/security policy must already have validated the destination.
// This is an initial request, never a copy of a live page's navigation.
struct ExternalLink {
  std::string destination;
};
// Presents lookup chrome without inventing a URL, search provider or live page.
struct QuickLookup {};
using EntryIntent = std::variant<ExternalLink, QuickLookup>;
enum class Presentation { kMainWindow, kLightweightWindow };

enum class Refusal {
  kTargetNotFound,
  kTargetNotAttached,
  kTargetProfileMismatch,
  kEmptyDestination,
  kPageNotFound,
  kPageProfileMismatch,
  kTemporaryTransferUnavailable,
  kSavedSpaceMismatch,
};

struct EntryPlan {
  ResolvedTarget target;
  EntryIntent intent;
  Presentation presentation;
};
using EntryResult = std::variant<Refusal, EntryPlan>;

// No allocation, URL matching, duplicate-link policy or window creation. The
// caller explicitly selects presentation and provisions the target attachment.
EntryResult PlanEntry(const lifecycle::Model& model,
                      ResolvedTarget target,
                      EntryIntent intent,
                      Presentation presentation);

// References only. lifecycle::LivePage remains the sole navigation/page owner.
struct PageReference {
  lifecycle::PageId page;
  lifecycle::ProfileId profile;
  lifecycle::Attachment attachment;
};
struct PresentInMainWindow {
  PageReference source;  // Same page, attachment and actual host window.
};
struct TransferSavedPage {
  PageReference source;
  lifecycle::ItemId item;
  lifecycle::Attachment destination;
};
using PromotionResult =
    std::variant<Refusal, PresentInMainWindow, TransferSavedPage>;

// Preflight only, never Keep/SelectSpace/TransferSavedPage or a native command.
// Plans own snapshots. Replan and revalidate native identities before
// actuation; no result is a reservation, completion report or stale-event
// protocol.
PromotionResult PlanPromotion(const lifecycle::Model& model,
                              lifecycle::PageId page,
                              ResolvedTarget target);

}  // namespace arcified::little_arc

#endif  // ARCIFIED_NATIVE_FEATURES_LITTLE_ARC_PLANNING_H_
