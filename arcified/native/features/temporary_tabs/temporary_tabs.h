#ifndef ARCIFIED_NATIVE_FEATURES_TEMPORARY_TABS_TEMPORARY_TABS_H_
#define ARCIFIED_NATIVE_FEATURES_TEMPORARY_TABS_TEMPORARY_TABS_H_

#include <optional>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::temporary_tabs {

enum class Refusal {
  kPageNotFound,
  kNotTemporary,
  kIdentityChanged,
  kAttachmentNotFound,
  kWrongAttachment,
  kDuplicatePage,
  kIncompleteOrder,
};

using IdentityResult = std::variant<Refusal, lifecycle::TemporaryTabId>;

// Model lookup only. No selected-page, attachment or Profile resolution.
IdentityResult ResolveTemporaryIdentity(const lifecycle::Model& model,
                                        const lifecycle::PageId& page);

// A delayed temporary-specific intent must still name the same temporary
// owner. Keep/close invalidate it. Navigation/activity changes do not.
// This does not authorize a command or validate a native object/generation.
std::optional<Refusal> ValidateTemporaryIdentity(
    const lifecycle::Model& model,
    const lifecycle::PageId& page,
    const lifecycle::TemporaryTabId& expected_tab);

struct TemporaryIdentity {
  lifecycle::PageId page;
  lifecycle::TemporaryTabId tab;
};
using ProjectionResult = std::variant<Refusal, std::vector<TemporaryIdentity>>;

// The caller supplies the complete native order of temporary pages for this
// attachment, after filtering out saved pages and other attachments. No
// fallback to map/ID order. Rejects stale, saved, foreign, duplicate or omitted
// pages; never returns a partial projection. Empty is valid for an attached
// scope with no temporary pages. Returned snapshots do not authorize commands.
ProjectionResult ProjectTemporaryIdentities(
    const lifecycle::Model& model,
    const lifecycle::Attachment& attachment,
    const std::vector<lifecycle::PageId>& native_order);

}  // namespace arcified::temporary_tabs

#endif  // ARCIFIED_NATIVE_FEATURES_TEMPORARY_TABS_TEMPORARY_TABS_H_
