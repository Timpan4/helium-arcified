#ifndef ARCIFIED_NATIVE_FEATURES_LIBRARY_SAVED_SOURCE_H_
#define ARCIFIED_NATIVE_FEATURES_LIBRARY_SAVED_SOURCE_H_

#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::library {

enum class SavedSource { kKeptInSpace, kProfileShortcut, kShortcutAlias };

// Disposable display projection. Identity, visibility and selection always
// resolve against the current lifecycle model, never this copied destination.
struct SavedEntry {
  lifecycle::ItemId item;
  SavedSource source;
  std::string saved_destination;
};

using SavedSearchResult =
    std::variant<lifecycle::Status, std::vector<SavedEntry>>;

// Searches only saved destinations visible through the lifecycle attachment.
// One row per ItemId, including aliases to otherwise hidden same-Profile items.
// Empty query lists all rows; substring matching folds ASCII case only and
// compares other UTF-8 bytes exactly. No history, live navigation or titles.
// An unattached context returns kNotAttached, distinct from an empty result.
SavedSearchResult SearchSaved(const lifecycle::Model& model,
                              const lifecycle::Attachment& attachment,
                              std::string_view query);

// Revalidates current visibility before delegating to Model::OpenItem. A stale
// or invisible item returns kNotFound; missing attachments return kNotAttached.
// This performs an in-memory semantic transition only. It does not open,
// transfer, focus or navigate native contents. A native adapter must coordinate
// actual native success before recording the corresponding model transition.
lifecycle::OpenResult OpenSavedResult(lifecycle::Model& model,
                                      lifecycle::Attachment attachment,
                                      lifecycle::ItemId item,
                                      lifecycle::PageId new_page);

}  // namespace arcified::library

#endif  // ARCIFIED_NATIVE_FEATURES_LIBRARY_SAVED_SOURCE_H_
