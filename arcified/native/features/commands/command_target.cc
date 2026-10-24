#include "command_target.h"

namespace arcified::commands {

PageTargetResult ResolvePageCommandTarget(
    const lifecycle::Model& model,
    const lifecycle::PageId& focused_page,
    const lifecycle::Attachment& expected_attachment) {
  if (focused_page.value.empty() || expected_attachment.window.value.empty() ||
      expected_attachment.space.value.empty()) {
    return TargetError::kInvalidInput;
  }
  const auto page = model.pages().find(focused_page);
  if (page == model.pages().end()) {
    return TargetError::kPageNotFound;
  }
  if (model.attachments().count(expected_attachment) == 0) {
    return TargetError::kAttachmentNotFound;
  }
  if (!(page->second.attachment() == expected_attachment)) {
    return TargetError::kAttachmentMismatch;
  }
  return PageCommandTarget{focused_page, page->second.profile,
                           page->second.attachment()};
}

TargetError RevalidatePageCommandTarget(const lifecycle::Model& model,
                                        const PageCommandTarget& target) {
  if (target.profile.value.empty()) {
    return TargetError::kInvalidInput;
  }
  const auto result =
      ResolvePageCommandTarget(model, target.page, target.attachment);
  if (const auto* error = std::get_if<TargetError>(&result)) {
    return *error;
  }
  if (std::get<PageCommandTarget>(result).profile != target.profile) {
    return TargetError::kProfileMismatch;
  }
  return TargetError::kOk;
}

}  // namespace arcified::commands
