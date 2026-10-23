#include "command_target.h"

namespace arcified::site_controls {
namespace {

bool IsSupported(Command command) {
  switch (command) {
    case Command::kPageInfo:
    case Command::kSiteSettings:
    case Command::kPrint:
      return true;
  }
  return false;
}

std::optional<Refusal> ValidateTarget(const lifecycle::Model& model,
                                      const Target& target) {
  // The shared command contract is the sole page/Profile/attachment validator.
  switch (commands::RevalidatePageCommandTarget(model, target.page_target)) {
    case commands::TargetError::kOk:
      break;
    case commands::TargetError::kInvalidInput:
      return Refusal::kInvalidLifecycleIdentity;
    case commands::TargetError::kPageNotFound:
      return Refusal::kPageNotFound;
    case commands::TargetError::kAttachmentNotFound:
      return Refusal::kAttachmentNotFound;
    case commands::TargetError::kAttachmentMismatch:
      return Refusal::kAttachmentChanged;
    case commands::TargetError::kProfileMismatch:
      return Refusal::kProfileMismatch;
  }
  if (target.document.value.empty() || target.origin.value.empty() ||
      target.security_state.value.empty() || target.surface.value.empty() ||
      target.selection.value.empty()) {
    return Refusal::kInvalidNativeIdentity;
  }
  const auto& attachment = target.page_target.attachment;
  const auto window = model.windows().find(attachment.window);
  if (window == model.windows().end() ||
      window->second.selected_space != attachment.space) {
    return Refusal::kSpaceNotSelected;
  }
  return std::nullopt;
}

std::optional<Refusal> ValidateAvailability(Command command,
                                            const HostContext& host) {
  if (!IsSupported(command))
    return Refusal::kUnsupportedCommand;
  if (host.available_commands.count(command) == 0)
    return Refusal::kCommandUnavailable;
  if (command == Command::kPageInfo &&
      (!host.page_info_anchor || host.page_info_anchor->value.empty())) {
    return Refusal::kAnchorUnavailable;
  }
  return std::nullopt;
}

}  // namespace

PrepareResult PrepareCommand(const lifecycle::Model& model,
                             Command command,
                             const HostContext& host) {
  if (!host.selected)
    return Refusal::kNativeSelectionUnavailable;
  if (const auto refusal = ValidateTarget(model, *host.selected))
    return *refusal;
  if (const auto refusal = ValidateAvailability(command, host))
    return *refusal;
  return CommandTarget{
      command, *host.selected,
      command == Command::kPageInfo ? host.page_info_anchor : std::nullopt};
}

std::optional<Refusal> RevalidateCommand(const lifecycle::Model& model,
                                         const CommandTarget& request,
                                         const HostContext& host) {
  if (const auto refusal = ValidateTarget(model, request.target))
    return refusal;
  if (!host.selected)
    return Refusal::kNativeSelectionUnavailable;
  if (const auto refusal = ValidateTarget(model, *host.selected))
    return refusal;
  const Target& expected = request.target;
  const Target& current = *host.selected;
  if (current.page_target.page != expected.page_target.page)
    return Refusal::kPageNotSelected;
  if (current.document != expected.document)
    return Refusal::kDocumentChanged;
  if (current.origin != expected.origin)
    return Refusal::kOriginChanged;
  if (current.security_state != expected.security_state)
    return Refusal::kSecurityStateChanged;
  if (current.surface != expected.surface)
    return Refusal::kSurfaceChanged;
  if (current.selection != expected.selection)
    return Refusal::kSelectionChanged;
  if (const auto refusal = ValidateAvailability(request.command, host))
    return refusal;
  if (request.command == Command::kPageInfo &&
      request.anchor != host.page_info_anchor) {
    return Refusal::kAnchorChanged;
  }
  return std::nullopt;
}

}  // namespace arcified::site_controls
