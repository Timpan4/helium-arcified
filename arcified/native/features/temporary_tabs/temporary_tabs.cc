#include "temporary_tabs.h"

#include <set>

namespace arcified::temporary_tabs {

IdentityResult ResolveTemporaryIdentity(const lifecycle::Model& model,
                                        const lifecycle::PageId& page) {
  const auto live = model.pages().find(page);
  if (live == model.pages().end())
    return Refusal::kPageNotFound;
  const auto* temporary =
      std::get_if<lifecycle::TemporaryTab>(&live->second.owner);
  if (!temporary)
    return Refusal::kNotTemporary;
  return temporary->id;
}

std::optional<Refusal> ValidateTemporaryIdentity(
    const lifecycle::Model& model,
    const lifecycle::PageId& page,
    const lifecycle::TemporaryTabId& expected_tab) {
  const auto resolved = ResolveTemporaryIdentity(model, page);
  if (const auto* refusal = std::get_if<Refusal>(&resolved))
    return *refusal;
  if (std::get<lifecycle::TemporaryTabId>(resolved) != expected_tab)
    return Refusal::kIdentityChanged;
  return std::nullopt;
}

ProjectionResult ProjectTemporaryIdentities(
    const lifecycle::Model& model,
    const lifecycle::Attachment& attachment,
    const std::vector<lifecycle::PageId>& native_order) {
  if (!model.attachments().count(attachment))
    return Refusal::kAttachmentNotFound;

  std::set<lifecycle::PageId> seen;
  std::vector<TemporaryIdentity> projection;
  for (const auto& page : native_order) {
    if (!seen.insert(page).second)
      return Refusal::kDuplicatePage;
    auto resolved = ResolveTemporaryIdentity(model, page);
    if (const auto* refusal = std::get_if<Refusal>(&resolved))
      return *refusal;
    if (!(model.pages().at(page).attachment() == attachment))
      return Refusal::kWrongAttachment;
    projection.push_back(
        {page, std::get<lifecycle::TemporaryTabId>(std::move(resolved))});
  }

  // Every supplied page is already a distinct temporary in this attachment.
  // Equal cardinality therefore proves full coverage, not just a valid subset.
  if (seen.size() != model.TemporaryPages(attachment).size())
    return Refusal::kIncompleteOrder;
  return projection;
}

}  // namespace arcified::temporary_tabs
