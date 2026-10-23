#ifndef ARCIFIED_NATIVE_FEATURES_COMMANDS_COMMAND_TARGET_H_
#define ARCIFIED_NATIVE_FEATURES_COMMANDS_COMMAND_TARGET_H_

#include <variant>

#include "../../model/lifecycle.h"

namespace arcified::commands {

// An owned semantic snapshot, never a native handle or a focus registry.
struct PageCommandTarget {
  lifecycle::PageId page;
  lifecycle::ProfileId profile;
  lifecycle::Attachment attachment;
};

enum class TargetError {
  kOk,
  kInvalidInput,
  kPageNotFound,
  kAttachmentNotFound,
  kAttachmentMismatch,
  kProfileMismatch,
};

using PageTargetResult = std::variant<TargetError, PageCommandTarget>;

// The native focus owner supplies the page and its expected attachment. No
// selected-space inference, focus change, navigation or Model mutation occurs.
// Does not classify privacy or authorize an action on an otherwise valid page.
PageTargetResult ResolvePageCommandTarget(
    const lifecycle::Model& model,
    const lifecycle::PageId& focused_page,
    const lifecycle::Attachment& expected_attachment);

// Checks current semantic liveness, attachment and immutable Profile. The
// native executor must ALSO revalidate its own native binding/operation
// generation on the serialized UI sequence immediately before effects. This
// snapshot cannot detect native replacement or a page moved away and back
// (ABA).
TargetError RevalidatePageCommandTarget(const lifecycle::Model& model,
                                        const PageCommandTarget& target);

}  // namespace arcified::commands

#endif  // ARCIFIED_NATIVE_FEATURES_COMMANDS_COMMAND_TARGET_H_
