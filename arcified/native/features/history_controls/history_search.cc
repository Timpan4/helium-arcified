#include "history_search.h"

#include <utility>

namespace arcified::history_controls {
namespace {

struct Scope {
  lifecycle::Attachment attachment;
  lifecycle::ProfileId profile;
};

using ScopeResult = std::variant<HistorySearchRefusal, Scope>;

ScopeResult ResolveScope(const lifecycle::Model& model,
                         const lifecycle::WindowId& window,
                         const HistoryContext& context) {
  // Never allow the native service factory's OTR-to-original redirection to
  // turn this surface into a reader of the original Profile's history.
  switch (context.kind) {
    case HistoryContextKind::kOffTheRecord:
      return HistorySearchRefusal::kPrivateContext;
    case HistoryContextKind::kUnsupported:
      return HistorySearchRefusal::kUnsupportedContext;
    case HistoryContextKind::kRegular:
      break;
  }

  const auto found = model.windows().find(window);
  if (found == model.windows().end())
    return HistorySearchRefusal::kWindowNotFound;
  if (!found->second.selected_space)
    return HistorySearchRefusal::kNoSelectedSpace;

  lifecycle::Attachment attachment{window, *found->second.selected_space};
  if (!model.attachments().count(attachment))
    return HistorySearchRefusal::kNotAttached;
  const auto& profile = model.spaces().at(attachment.space).profile;
  if (profile != context.profile)
    return HistorySearchRefusal::kProfileMismatch;
  return Scope{std::move(attachment), profile};
}

}  // namespace

HistorySearchText ParseHistorySearchText(std::u16string text) {
  constexpr char16_t kHostPrefix[] = u"host:";
  const bool host_only = text.rfind(kHostPrefix, 0) == 0;
  if (host_only)
    text.erase(0, std::char_traits<char16_t>::length(kHostPrefix));
  return {std::move(text), host_only};
}

HistorySearchResult PlanHistorySearch(const lifecycle::Model& model,
                                      lifecycle::WindowId window,
                                      const HistoryContext& context,
                                      HistoryQueryId query,
                                      std::u16string text) {
  if (query.value.empty())
    return HistorySearchRefusal::kInvalidQueryId;
  auto scope = ResolveScope(model, window, context);
  if (const auto* refusal = std::get_if<HistorySearchRefusal>(&scope))
    return *refusal;
  auto& resolved = std::get<Scope>(scope);
  return HistorySearchPlan{std::move(query), std::move(resolved.attachment),
                           std::move(resolved.profile),
                           ParseHistorySearchText(std::move(text))};
}

std::optional<HistorySearchRefusal> CheckHistorySearchCompletion(
    const lifecycle::Model& model,
    const std::optional<HistoryQueryId>& active_query,
    const HistoryContext& current_context,
    const HistorySearchPlan& completed) {
  if (!active_query || *active_query != completed.query)
    return HistorySearchRefusal::kStaleQuery;
  auto scope =
      ResolveScope(model, completed.attachment.window, current_context);
  if (const auto* refusal = std::get_if<HistorySearchRefusal>(&scope))
    return *refusal;
  const auto& resolved = std::get<Scope>(scope);
  if (!(resolved.attachment == completed.attachment))
    return HistorySearchRefusal::kSelectionChanged;
  if (resolved.profile != completed.profile)
    return HistorySearchRefusal::kProfileMismatch;
  return std::nullopt;
}

}  // namespace arcified::history_controls
