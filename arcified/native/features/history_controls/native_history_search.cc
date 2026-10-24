#include "native_history_search.h"

#include <utility>

#include "chrome/browser/history/history_service_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "components/history/core/browser/history_types.h"
#include "components/keyed_service/core/service_access_type.h"
#include "content/public/browser/browser_thread.h"

namespace arcified::history_controls {

NativeHistorySearchResult StartNativeHistorySearch(
    const lifecycle::Model& model,
    const std::optional<HistoryQueryId>& active_query,
    const HistorySearchPlan& plan,
    const NativeHistoryProfile& native_profile,
    int page_size,
    history::HistoryService::QueryHistoryCallback callback,
    base::CancelableTaskTracker& tracker) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  Profile& profile = native_profile.profile;
  const HistoryContextKind kind =
      profile.IsOffTheRecord()     ? HistoryContextKind::kOffTheRecord
      : profile.IsRegularProfile() ? HistoryContextKind::kRegular
                                   : HistoryContextKind::kUnsupported;
  if (const auto refusal = CheckHistorySearchCompletion(
          model, active_query, {native_profile.id, kind}, plan)) {
    return *refusal;
  }
  // QueryOptions defines zero as unbounded. Require the consuming surface's
  // positive page size rather than choosing another UI's quota here.
  if (page_size <= 0)
    return HistorySearchRefusal::kInvalidPageSize;

  // Explicit history inspection may read existing history even when recording
  // is disabled. Private/unsupported contexts were rejected before this lookup,
  // whose native factory can otherwise redirect OTR to the original Profile.
  auto* service = HistoryServiceFactory::GetForProfile(
      &profile, ServiceAccessType::EXPLICIT_ACCESS);
  if (!service)
    return HistorySearchRefusal::kServiceUnavailable;

  history::QueryOptions options;
  options.max_count = page_size;
  options.policy_for_404_visits = history::VisitQuery404sPolicy::kExclude404s;
  options.duplicate_policy = history::QueryOptions::REMOVE_DUPLICATES_PER_DAY;
  options.host_only = plan.search.host_only;
  const auto task = service->QueryHistory(plan.search.text, options,
                                          std::move(callback), &tracker);
  if (task == base::CancelableTaskTracker::kBadTaskId)
    return HistorySearchRefusal::kTaskNotScheduled;
  return task;
}

}  // namespace arcified::history_controls
