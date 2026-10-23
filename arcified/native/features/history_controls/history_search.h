#ifndef ARCIFIED_NATIVE_FEATURES_HISTORY_CONTROLS_HISTORY_SEARCH_H_
#define ARCIFIED_NATIVE_FEATURES_HISTORY_CONTROLS_HISTORY_SEARCH_H_

#include <optional>
#include <string>
#include <variant>

#include "../../model/lifecycle.h"

namespace arcified::history_controls {

using HistoryQueryId = lifecycle::Id<struct HistoryQueryTag>;

// A fresh report from the actual native browsing context, not from a Profile
// display name, path, or GetOriginalProfile(). No native pointer is retained.
// The embedding registry must bind profile to that exact context. In Chromium,
// check IsOffTheRecord() before IsRegularProfile(); all other kinds are
// excluded.
enum class HistoryContextKind { kRegular, kOffTheRecord, kUnsupported };
struct HistoryContext {
  lifecycle::ProfileId profile;
  HistoryContextKind kind;
};

// Only the native history UI's leading, case-sensitive "host:" syntax is
// translated. The native provider still owns text matching and URL handling.
struct HistorySearchText {
  std::u16string text;
  bool host_only;
};
HistorySearchText ParseHistorySearchText(std::u16string text);

enum class HistorySearchRefusal {
  kInvalidQueryId,
  kWindowNotFound,
  kNoSelectedSpace,
  kNotAttached,
  kPrivateContext,
  kUnsupportedContext,
  kProfileMismatch,
  kStaleQuery,
  kSelectionChanged,
  kInvalidPageSize,
  kServiceUnavailable,
  kTaskNotScheduled,
};

// An owned request snapshot, not another provider, selection owner,
// reservation, or persistence format. History is Profile-wide, including every
// Space sharing that Profile. The attachment identifies the requesting view
// only.
struct HistorySearchPlan {
  HistoryQueryId query;
  lifecycle::Attachment attachment;
  lifecycle::ProfileId profile;
  HistorySearchText search;
};
using HistorySearchResult =
    std::variant<HistorySearchRefusal, HistorySearchPlan>;

// Pure preparation. Does not read records, navigate, persist, or delete data.
// Caller supplies a fresh, non-reused query ID for every request and
// invalidates the active ID on selection/context change, cancellation, or view
// destruction.
HistorySearchResult PlanHistorySearch(const lifecycle::Model& model,
                                      lifecycle::WindowId window,
                                      const HistoryContext& context,
                                      HistoryQueryId query,
                                      std::u16string text);

// nullopt permits publishing this completion into the still-current view.
// Caller owns the active ID and cancels work through the native task tracker.
// Recheck the live native context and this guard on the UI sequence immediately
// before publishing, then consume the active ID. This is not a DB success
// check.
std::optional<HistorySearchRefusal> CheckHistorySearchCompletion(
    const lifecycle::Model& model,
    const std::optional<HistoryQueryId>& active_query,
    const HistoryContext& current_context,
    const HistorySearchPlan& completed);

}  // namespace arcified::history_controls

#endif  // ARCIFIED_NATIVE_FEATURES_HISTORY_CONTROLS_HISTORY_SEARCH_H_
