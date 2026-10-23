#ifndef ARCIFIED_NATIVE_FEATURES_WINDOW_RESTORE_WINDOW_RESTORE_H_
#define ARCIFIED_NATIVE_FEATURES_WINDOW_RESTORE_WINDOW_RESTORE_H_

#include <map>
#include <optional>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::window_restore {

using UndoId = lifecycle::Id<struct UndoTag>;
using AttemptId = lifecycle::Id<struct AttemptTag>;
// A bridge-owned, Profile-scoped runtime lookup key, never a URL or raw
// SessionID. It is not portable/durable data and must not be serialized.
using NativeRestoreId = lifecycle::Id<struct NativeRestoreTag>;

enum class CloseReason { kDeliberate, kCrash, kSessionShutdown };
enum class Error {
  kOk,
  kExcluded,
  kNotFound,
  kDuplicateId,
  kInvalidPresentation,
  kNativeRestoreRequired,
  kSplitIntegrationPending,
  kWrongState,
  kCloseIncomplete,
  kOwnerChanged,
  kSavedPageAlreadyLive,
  kIdentityConflict,
  kRestoreIncomplete,
};

// Actual presentation order is supplied by the embedding browser. Map order
// is not user order. Selection is a reference, not another navigation owner.
struct Presentation {
  std::vector<lifecycle::SpaceId> spaces;
  std::vector<lifecycle::PageId> pages;
  std::optional<lifecycle::PageId> selected_page;
};

struct NativePageJoin {
  lifecycle::PageId page;
  NativeRestoreId native;
};

struct ClosedSpace {
  lifecycle::SpaceId space;
  lifecycle::ProfileId profile;
};

struct ClosedPage {
  lifecycle::PageId page;
  lifecycle::ProfileId profile;
  lifecycle::PageOwner owner;
  NativeRestoreId native;
};

// Historical references only. Shared items, destinations, navigation, activity,
// credentials, group payloads and renderer memory are deliberately absent.
struct ClosedWindow {
  lifecycle::WindowId window;
  std::vector<ClosedSpace> spaces;
  std::vector<ClosedPage> pages;
  std::optional<lifecycle::SpaceId> selected_space;
  std::optional<lifecycle::PageId> selected_page;
};

struct NewPageIdentity {
  lifecycle::PageId closed_page;
  lifecycle::PageId new_page;
  std::optional<lifecycle::TemporaryTabId> new_temporary;
};

struct ReopenPlan {
  lifecycle::WindowId window;
  std::vector<ClosedSpace> spaces;
  // Remapped page/owner identities, in the original presentation order.
  std::vector<ClosedPage> pages;
  std::optional<lifecycle::SpaceId> selected_space;
  std::optional<lifecycle::PageId> selected_page;
};
using ReopenResult = std::variant<Error, ReopenPlan>;

// Read-only view of the root integration's shared native privacy classifier.
// This is not a classifier implementation. The trusted adapter must resolve
// actual native contexts for these exact identities; ProfileId alone proves
// nothing. Unknown, off-the-record and guest contexts must all return false.
// Synthetic implementations in tests establish protocol behavior only.
class CaptureEligibility {
 public:
  virtual ~CaptureEligibility() = default;
  virtual bool IsWindowVerifiedRegular(
      const lifecycle::WindowId& window) const = 0;
  virtual bool IsPageVerifiedRegular(
      const lifecycle::WindowId& window,
      const lifecycle::PageId& page,
      const lifecycle::ProfileId& profile) const = 0;
};

// Single-writer, in-memory close-undo protocol, separate from crash/session
// recovery. It never mutates Model or invokes a browser. The caller allocates
// non-reused operation and restored window/page/tab IDs, including after
// cancellation/consumption. Shared Space/item references retain their IDs.
// No persistence, expiry, eviction or last-window policy is implemented. The
// coordinator serializes model/native events across capture and confirmation;
// it cancels/recaptures if the closing window's contents change in that gap.
class Journal {
 public:
  Journal() = default;
  Journal(const Journal&) = delete;
  Journal& operator=(const Journal&) = delete;

  // Stage immediately before close under the embedding coordinator's serialized
  // operation. This is not close authorization and does not change activity.
  Error StageClose(UndoId undo,
                   const lifecycle::Model& model,
                   CloseReason reason,
                   const CaptureEligibility& eligibility,
                   lifecycle::WindowId window,
                   Presentation presentation,
                   std::vector<NativePageJoin> joins);
  // Call only after observed native completion AND its model close report.
  Error ConfirmClose(UndoId undo, const lifecycle::Model& model);
  Error CancelClose(UndoId undo);

  ReopenResult BeginReopen(UndoId undo,
                           AttemptId attempt,
                           const lifecycle::Model& model,
                           lifecycle::WindowId target,
                           std::vector<NewPageIdentity> identities);
  // Recheck immediately before native actuation. A plan is not a reservation.
  Error RevalidateReopen(UndoId undo,
                         AttemptId attempt,
                         const lifecycle::Model& model) const;
  // Failure/cancellation retains history. Native partial success must first be
  // reconciled by the embedding coordinator; this method performs no rollback.
  Error CancelReopen(UndoId undo, AttemptId attempt);
  // Call after native success and model reports. Exact membership, ownership,
  // order and selection must agree before the entry can be consumed.
  Error ConfirmReopen(UndoId undo,
                      AttemptId attempt,
                      const lifecycle::Model& model,
                      Presentation observed);

  // Confirmation order, oldest first. Includes entries currently reopening.
  std::vector<UndoId> history() const { return history_; }

 private:
  struct PendingClose {};
  struct Closed {};
  struct Reopening {
    AttemptId attempt;
    ReopenPlan plan;
  };
  struct Entry {
    ClosedWindow record;
    std::variant<PendingClose, Closed, Reopening> state;
  };

  std::map<UndoId, Entry> entries_;
  std::vector<UndoId> history_;
};

}  // namespace arcified::window_restore

#endif  // ARCIFIED_NATIVE_FEATURES_WINDOW_RESTORE_WINDOW_RESTORE_H_
