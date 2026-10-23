#include "workspace_results.h"

#include <utility>

namespace arcified::commands {
namespace {

// Length framing preserves opaque IDs containing separators. These IDs are
// ephemeral presentation keys; no parser or persistent identity is introduced.
void Append(std::string& key, const std::string& field) {
  key += std::to_string(field.size()) + ":" + field;
}

void AppendAttachment(std::string& key,
                      const lifecycle::Attachment& attachment) {
  Append(key, attachment.window.value);
  Append(key, attachment.space.value);
}

void AppendSaved(std::string& key, const SavedItemTarget& target) {
  Append(key, target.item.value);
  Append(key, target.profile.value);
  if (const auto* space = std::get_if<lifecycle::SpaceId>(&target.owner)) {
    Append(key, "space");
    Append(key, space->value);
  } else {
    Append(key, "profile");
    Append(key, std::get<lifecycle::ProfileId>(target.owner).value);
  }
}

Result LiveResult(const RankedCommand& command,
                  const RankedLivePage& live,
                  std::optional<lifecycle::ItemId> item,
                  std::string action_label) {
  std::string key = "switch:";
  Append(key, live.page.value);
  Append(key, command.profile.value);
  AppendAttachment(key, live.attachment);
  // Keep a temporary-to-saved transition distinct even though PageId survives.
  Append(key, item ? "saved" : "temporary");
  if (item)
    Append(key, item->value);
  return {ResultId{std::move(key)}, command.title, std::move(action_label),
          SwitchToTabAction{live.page, command.profile, live.attachment, item}};
}

}  // namespace

Result MakeWorkspaceResult(const RankedCommand& command,
                           WorkspaceResultMode mode,
                           std::string action_label) {
  if (const auto* saved = std::get_if<RevealSavedItem>(&command.reveal)) {
    const SavedItemTarget target{saved->item, command.profile, saved->owner};
    if (mode == WorkspaceResultMode::kActivate && saved->live)
      return LiveResult(command, *saved->live, saved->item,
                        std::move(action_label));
    std::string key =
        mode == WorkspaceResultMode::kReveal ? "reveal-saved:" : "open-saved:";
    AppendSaved(key, target);
    Action action = OpenSavedItemAction{target};
    if (mode == WorkspaceResultMode::kReveal)
      action = RevealInWorkspaceAction{target};
    return {ResultId{std::move(key)}, command.title, std::move(action_label),
            std::move(action)};
  }
  const auto& temporary = std::get<RevealTemporaryPage>(command.reveal);
  if (mode == WorkspaceResultMode::kActivate)
    return LiveResult(command, temporary.live, std::nullopt,
                      std::move(action_label));
  std::string key = "reveal-temporary:";
  Append(key, temporary.live.page.value);
  Append(key, command.profile.value);
  AppendAttachment(key, temporary.live.attachment);
  return {
      ResultId{std::move(key)}, command.title, std::move(action_label),
      RevealInWorkspaceAction{TemporaryPageTarget{
          temporary.live.page, command.profile, temporary.live.attachment}}};
}

}  // namespace arcified::commands
