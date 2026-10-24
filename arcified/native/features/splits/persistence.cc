#include "persistence.h"

#include <cmath>
#include <set>
#include <utility>

namespace arcified::splits {

RestoreResult ReconcileRestoredLayout(const lifecycle::Model& model,
                                      const lifecycle::SplitId& restored_split,
                                      const LayoutMetadata& saved_layout,
                                      const RestoredPageMap& restored_pages) {
  // Validate orientation and obtain only canonical members/defaults from the
  // shared presentation core. Corrupt weights are recovered individually below.
  auto baseline = ReconcileLayout(model, restored_split,
                                  LayoutMetadata{saved_layout.orientation, {}});
  if (const auto* error = std::get_if<LayoutError>(&baseline))
    return *error;
  RestoredLayout result{std::get<LayoutMetadata>(std::move(baseline)), {}};
  const auto& split = model.splits().at(restored_split);
  const std::set<lifecycle::PageId> members(split.pages.begin(),
                                            split.pages.end());
  using Candidate = std::pair<lifecycle::PageId, double>;
  std::map<lifecycle::PageId, std::vector<Candidate>> candidates;
  for (const auto& entry : saved_layout.weights) {
    const auto& source = entry.first;
    const double weight = entry.second;
    const auto mapping = restored_pages.find(source);
    std::optional<lifecycle::PageId> target;
    if (mapping != restored_pages.end())
      target = mapping->second;
    const auto report = [&](RestoreIssueReason reason) {
      result.issues.push_back({source, target, reason});
    };
    if (source.value.empty() || (target && target->value.empty())) {
      report(RestoreIssueReason::kInvalidReference);
      continue;
    }
    if (!target) {
      report(RestoreIssueReason::kMissingMapping);
      continue;
    }
    const auto page = model.pages().find(*target);
    if (page == model.pages().end()) {
      report(RestoreIssueReason::kPageNotRestored);
      continue;
    }
    if (!model.attachments().count(split.attachment) ||
        !(page->second.attachment() == split.attachment)) {
      report(RestoreIssueReason::kAttachmentMismatch);
      continue;
    }
    if (!members.count(*target)) {
      report(RestoreIssueReason::kPageNotInSplit);
      continue;
    }
    candidates[*target].push_back({source, weight});
  }

  for (const auto& [target, sources] : candidates) {
    if (sources.size() > 1) {
      for (const auto& [source, weight] : sources)
        result.issues.push_back(
            {source, target, RestoreIssueReason::kAmbiguousMapping});
      continue;
    }
    const auto& [source, weight] = sources.front();
    if (!std::isfinite(weight) || weight <= 0.0) {
      result.issues.push_back(
          {source, target, RestoreIssueReason::kInvalidWeight});
      continue;
    }
    result.metadata.weights.at(target) = weight;
  }
  return result;
}

}  // namespace arcified::splits
