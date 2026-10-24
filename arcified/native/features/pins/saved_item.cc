#include "saved_item.h"

#include <utility>

namespace arcified::pins {
namespace {

bool HasEmptyLabel(const SavedItemMetadata& metadata) {
  return metadata.custom_label && metadata.custom_label->empty();
}

}  // namespace

PresentationResult DescribeSavedItem(const lifecycle::Model& model,
                                     lifecycle::ItemId item,
                                     const SavedItemMetadata& metadata,
                                     std::string default_label) {
  if (!model.items().count(item))
    return SavedItemError::kItemNotFound;
  if (default_label.empty() || HasEmptyLabel(metadata))
    return SavedItemError::kEmptyLabel;
  std::optional<lifecycle::PageId> live_page;
  for (const auto& [id, page] : model.pages()) {
    const auto* saved = std::get_if<lifecycle::SavedPage>(&page.owner);
    if (saved && saved->item == item) {
      live_page = id;
      break;
    }
  }
  return SavedItemPresentation{
      std::move(item), metadata.custom_label.value_or(std::move(default_label)),
      metadata.custom_label.has_value(), std::move(live_page)};
}

LabelEditResult PlanLabelEdit(const lifecycle::Model& model,
                              lifecycle::ItemId item,
                              const SavedItemMetadata& current,
                              std::optional<std::string> custom_label) {
  if (!model.items().count(item))
    return SavedItemError::kItemNotFound;
  SavedItemMetadata after{std::move(custom_label)};
  if (HasEmptyLabel(current) || HasEmptyLabel(after))
    return SavedItemError::kEmptyLabel;
  return LabelEdit{std::move(item), current, std::move(after)};
}

MetadataResult ApplyLabelEdit(const lifecycle::Model& model,
                              const LabelEdit& edit,
                              const SavedItemMetadata& current,
                              EditDirection direction) {
  if (!model.items().count(edit.item))
    return SavedItemError::kItemNotFound;
  if (HasEmptyLabel(current) || HasEmptyLabel(edit.before) ||
      HasEmptyLabel(edit.after)) {
    return SavedItemError::kEmptyLabel;
  }
  const auto& expected =
      direction == EditDirection::kForward ? edit.before : edit.after;
  if (current != expected)
    return SavedItemError::kStaleMetadata;
  return direction == EditDirection::kForward ? edit.after : edit.before;
}

}  // namespace arcified::pins
