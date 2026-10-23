#ifndef ARCIFIED_NATIVE_FEATURES_HISTORY_CONTROLS_NATIVE_HISTORY_SEARCH_H_
#define ARCIFIED_NATIVE_FEATURES_HISTORY_CONTROLS_NATIVE_HISTORY_SEARCH_H_

#include <variant>

#include "base/task/cancelable_task_tracker.h"
#include "components/history/core/browser/history_service.h"
#include "history_search.h"

class Profile;

namespace arcified::history_controls {

// Borrowed only during dispatch. The embedding registry must supply the exact
// native Profile corresponding to id, not an original-Profile substitution.
// This feature neither implements that registry nor extends Profile lifetime.
struct NativeHistoryProfile {
  lifecycle::ProfileId id;
  Profile& profile;
};

using NativeHistorySearchResult =
    std::variant<HistorySearchRefusal, base::CancelableTaskTracker::TaskId>;

// Source-backed Chromium integration slice, not wired into a GN target or UI.
// Must run on the browser UI sequence while the native Profile is live.
// page_size is supplied by the consuming surface, with no feature default.
// Refusal drops callback without invoking it. Success means scheduled only.
// Callback receives authoritative history::QueryResults unchanged. Bind it to
// the view's WeakPtr and run CheckHistorySearchCompletion with a fresh native
// context report immediately before publishing. Cancel through tracker and
// invalidate the active ID on scope change, shutdown, cancellation, or
// teardown.
NativeHistorySearchResult StartNativeHistorySearch(
    const lifecycle::Model& model,
    const std::optional<HistoryQueryId>& active_query,
    const HistorySearchPlan& plan,
    const NativeHistoryProfile& native_profile,
    int page_size,
    history::HistoryService::QueryHistoryCallback callback,
    base::CancelableTaskTracker& tracker);

}  // namespace arcified::history_controls

#endif  // ARCIFIED_NATIVE_FEATURES_HISTORY_CONTROLS_NATIVE_HISTORY_SEARCH_H_
