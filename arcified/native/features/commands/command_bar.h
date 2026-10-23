#ifndef ARCIFIED_NATIVE_FEATURES_COMMANDS_COMMAND_BAR_H_
#define ARCIFIED_NATIVE_FEATURES_COMMANDS_COMMAND_BAR_H_

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::commands {

using SessionToken = lifecycle::Id<struct CommandBarSessionTag>;
using QueryToken = lifecycle::Id<struct CommandBarQueryTag>;
using SurfaceToken = lifecycle::Id<struct CommandBarSurfaceTag>;
using ResultId = lifecycle::Id<struct CommandBarResultTag>;
using CommandKey = lifecycle::Id<struct CommandBarCommandTag>;
using SearchProviderKey = lifecycle::Id<struct CommandBarSearchProviderTag>;

// Opaque caller-allocated tokens, never derived from text or sequence ordering.
// Callers must not reuse session/query tokens while any callback may survive,
// including across sessions or replacement CommandBar instances.
struct RequestKey {
  SessionToken session;
  QueryToken query;
  friend bool operator==(const RequestKey& a, const RequestKey& b) {
    return a.session == b.session && a.query == b.query;
  }
};

// A snapshot of caller-resolved provenance, not ownership or current
// navigation. Native dispatch must revalidate identity, attachment, Profile and
// permissions.
struct Context {
  lifecycle::ProfileId profile;
  lifecycle::Attachment attachment;
  std::optional<lifecycle::PageId> page;
  std::optional<lifecycle::ItemId> item;
};

struct Origin {
  SurfaceToken surface;
  Context context;
};

// Destinations and queries are inert caller-supplied text. None of these types
// grants URL policy approval or authorizes a browser/lifecycle side effect.
struct NavigateAction {
  Context target;
  std::string destination;
};
struct SearchAction {
  Context target;
  std::string query;
  std::optional<SearchProviderKey> provider;
};
struct SwitchToTabAction {
  lifecycle::PageId page;
  lifecycle::ProfileId profile;  // Target page's immutable Profile.
  lifecycle::Attachment source;
  std::optional<lifecycle::ItemId> item;
};
struct ExecuteCommandAction {
  Context target;
  CommandKey command;
};
// Reference to existing saved ownership, never a new URL-backed destination.
// SpaceId in owner identifies the owning Space; ProfileId means a standalone
// Profile shortcut. The native executor resolves/validates the current graph.
struct SavedItemTarget {
  lifecycle::ItemId item;
  lifecycle::ProfileId profile;
  lifecycle::ItemOwner owner;
};
struct OpenSavedItemAction {
  SavedItemTarget target;
};
struct TemporaryPageTarget {
  lifecycle::PageId page;
  lifecycle::ProfileId profile;
  lifecycle::Attachment attachment;
};
struct RevealInWorkspaceAction {
  std::variant<SavedItemTarget, TemporaryPageTarget> target;
};
using Action = std::variant<NavigateAction,
                            SearchAction,
                            SwitchToTabAction,
                            ExecuteCommandAction,
                            OpenSavedItemAction,
                            RevealInWorkspaceAction>;
enum class ActionKind {
  kNavigate,
  kSearch,
  kSwitchToTab,
  kExecuteCommand,
  kOpenSavedItem,
  kRevealInWorkspace
};
ActionKind KindOf(const Action& action);

struct Result {
  // Stable identity for one action/target across a query's ordered refreshes.
  // A changed action/target needs a new ID. Titles/URLs are not identities.
  ResultId id;
  std::string title;
  // Caller-localized, explicit consequence, e.g. "Switch to tab: Example".
  // Native UI exposes this with KindOf(action) and the selected row on Enter.
  std::string action_label;
  Action action;
};

struct ActivationIntent {
  RequestKey request;
  Origin origin;
  Result result;
};
struct CancellationIntent {
  RequestKey request;
  Origin origin;  // Restore this exact initiating surface if still valid.
  std::string abandoned_query;
};

enum class BarStatus {
  kOk,
  kClosed,
  kAlreadyOpen,
  kInvalidInput,
  kStaleQuery,
  kNotFound,
  kNoSelection,
};
enum class SelectionDirection { kPrevious, kNext };
using ActivationResult = std::variant<BarStatus, ActivationIntent>;

struct Session {
  RequestKey request;
  Origin origin;
  std::string query;
  std::vector<Result> results;  // Caller order, never ranked here.
  std::optional<ResultId> selected;
};

// Single-writer presentation/session core. It has no model, native pointers,
// service callbacks, persistence, global focus, command execution or I/O.
class CommandBar {
 public:
  // Explicit seed lets the caller decide whether/where to retain abandoned
  // text. A second invocation cannot replace an open session's origin.
  BarStatus Invoke(RequestKey request, Origin origin, std::string query);
  // A fresh query token is required even when returning to earlier text.
  // Clears rows so Enter cannot activate results for the previous query.
  BarStatus SetQuery(QueryToken token, std::string query);
  // Atomic replacement. Rejects stale keys, empty IDs/labels/payloads and
  // duplicate row IDs. Selection follows ID, never a vector position.
  BarStatus PublishResults(RequestKey request, std::vector<Result> results);
  BarStatus Select(ResultId result);
  // Clamps at either end; from no selection chooses first/last respectively.
  BarStatus MoveSelection(SelectionDirection direction);
  // Only this explicit call emits an activation intent. Success closes the
  // session; failure leaves it open. The returned intent owns its payload.
  ActivationResult Activate();
  // Escape emits only restoration intent and abandoned text, then closes.
  std::optional<CancellationIntent> Escape();

  // Views may be invalidated by subsequent calls. Mutation inputs are owned
  // values, so passing IDs/keys/query text directly from these views is safe.
  const std::optional<Session>& session() const { return session_; }
  const Result* selected_result() const;

 private:
  std::optional<Session> session_;
  // Initially choose the first nonempty publication for each query. If the
  // selected ID disappears, require explicit reselection, never a replacement.
  bool choose_first_ = false;
};

}  // namespace arcified::commands

#endif  // ARCIFIED_NATIVE_FEATURES_COMMANDS_COMMAND_BAR_H_
