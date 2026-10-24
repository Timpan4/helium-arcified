#ifndef ARCIFIED_NATIVE_FEATURES_ORGANIZATION_FOLDER_PREVIEW_H_
#define ARCIFIED_NATIVE_FEATURES_ORGANIZATION_FOLDER_PREVIEW_H_

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "folders.h"

namespace arcified::organization {

enum class PreviewError {
  kNotAttached,
  kFolderNotFound,
  kActivePageNotFound,
  kActivePageOutsideAttachment,
  kMissingItemLabel,
};

struct FolderBreadcrumb {
  FolderId folder;
  std::string label;
  bool expanded;
};

struct PreviewRow {
  lifecycle::ItemId item;
  std::optional<lifecycle::PageId> live_page;
  std::string label;
  // Full Space-root-to-parent path, including the requested folder.
  std::vector<FolderBreadcrumb> ancestry;
  Location location;
  bool active;
};

struct FolderPreview {
  std::vector<PreviewRow> matches;
  // Independent of the search filter; absent for an expanded folder.
  std::optional<PreviewRow> active_descendant;
};

// FolderStatus is propagated from the authoritative validated Snapshot.
using PreviewResult = std::variant<FolderStatus, PreviewError, FolderPreview>;

// Pure, owned observation on the caller's serialized lifecycle/metadata state.
// Searches all saved descendants in hierarchy order, regardless of expansion.
// Matching is literal case-sensitive UTF-8 substring; empty query matches all.
// Every descendant needs a caller-supplied label. No URL/navigation fallback.
// active_page is an observed selection, never inferred from a background open.
// Its page must still exist in context; a temporary active page projects none.
// This neither performs browser actions nor changes expansion/focus/selection.
// Requery and revalidate before acting on a result after any state mutation.
PreviewResult ProjectFolderPreview(
    const lifecycle::Model& model,
    const Folders& folders,
    FolderId folder,
    lifecycle::Attachment context,
    const std::map<lifecycle::ItemId, std::string>& item_labels,
    std::string_view query,
    std::optional<lifecycle::PageId> active_page = std::nullopt);

}  // namespace arcified::organization

#endif  // ARCIFIED_NATIVE_FEATURES_ORGANIZATION_FOLDER_PREVIEW_H_
