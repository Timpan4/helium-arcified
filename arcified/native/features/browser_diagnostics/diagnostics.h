#ifndef ARCIFIED_NATIVE_FEATURES_BROWSER_DIAGNOSTICS_DIAGNOSTICS_H_
#define ARCIFIED_NATIVE_FEATURES_BROWSER_DIAGNOSTICS_DIAGNOSTICS_H_

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>

#include "../../model/lifecycle.h"

namespace arcified::browser_diagnostics {

// Supplied by the host after checking the native implementation and policy.
enum class Capability { kUnavailable, kAvailable };
enum class Scope { kBrowserProcesses };
enum class Status {
  kOk,
  kUnavailable,
  kWindowNotFound,
  kBusy,
  kStaleSession,
  kStaleOperation,
  kWrongPhase,
};

// The window is an invocation context, never a process/Profile/page filter.
// Replan at actuation; this is not a native window liveness/focus guarantee.
struct TaskManagerPlan {
  lifecycle::WindowId invoking_window;
  Scope scope = Scope::kBrowserProcesses;
};
using TaskManagerResult = std::variant<Status, TaskManagerPlan>;
TaskManagerResult PlanTaskManager(const lifecycle::Model& model,
                                  lifecycle::WindowId invoking_window,
                                  Capability capability);

struct TraceId {
  std::uint64_t value;
  friend bool operator==(TraceId a, TraceId b) { return a.value == b.value; }
};

enum class Phase {
  kIdle,
  kStarting,
  kRecording,
  kStopping,
  kCancelPendingStart,
  kCancelPendingStop,
  kDiscarding,
  kReady,
  kCancelled,
  kStartFailed,
  kCleanupRequired,
};

enum class NativeAction {
  kStart,
  kStopForSummary,
  kStopAndDiscard,
  // Prior native failure leaves ownership uncertain. Reconcile only this
  // session's recorder, never stop another client's global trace.
  kReconcileAndDiscard,
};
// Each emitted action carries an opaque in-process completion identity. Copies
// retain that identity; a later retry gets a new one, even in the same session.
class TraceAction {
 public:
  TraceId session() const { return session_; }
  NativeAction action() const { return action_; }
  Scope scope() const { return Scope::kBrowserProcesses; }

 private:
  friend class TraceSession;
  struct Token {};
  TraceAction(TraceId session, NativeAction action)
      : session_(session), action_(action), token_(std::make_shared<Token>()) {}
  TraceId session_;
  NativeAction action_;
  std::shared_ptr<const Token> token_;
};
struct Transition {
  Status status;
  std::optional<TraceAction> action;
};

// Allowlisted aggregate metadata, not a trace payload or a redaction format.
// The host must measure these counters; arbitrary data encoded as numbers is
// outside this contract. No titles, URLs, IDs, paths, labels or event
// arguments.
struct TraceSummary {
  std::uint64_t elapsed_microseconds;
  std::uint64_t process_count;
  std::uint64_t event_count;
};

// Single-sequence protocol coordinator. Does not implement a native recorder,
// own native resources, modify Model, infer focus, or arbitrate global tracing.
// The host serializes native events, owns resources through cancellation, and
// binds every event to its actual session. IDs must increase across starts on
// this object; global non-reuse across object lifetimes belongs to the host.
class TraceSession {
 public:
  TraceSession() = default;
  TraceSession(const TraceSession&) = delete;
  TraceSession& operator=(const TraceSession&) = delete;

  Transition Start(const lifecycle::Model& model,
                   lifecycle::WindowId invoking_window,
                   Capability capability,
                   TraceId session);
  Transition Stop(TraceId session);
  Transition Cancel(TraceId session);

  // false means native start was rejected and this session owns no recorder
  // or trace data. True means the native start callback has completed.
  Transition AcknowledgeStart(const TraceAction& operation, bool started);
  // Only after native stop finalization AND all raw data has been discarded.
  // A cancel pending on stop ignores the summary and becomes cancelled.
  Transition AcknowledgeStop(const TraceAction& operation,
                             TraceSummary summary);
  // Only after native tracing has stopped AND all session data is discarded.
  Transition AcknowledgeDiscard(const TraceAction& operation);
  // Native failure is unresolved ownership, never cleanup. During recording,
  // use the acknowledged start action for a recorder/service failure. During
  // a pending action, use that action. A confirmed start rejection instead
  // uses AcknowledgeStart(false).
  Transition ReportNativeFailure(const TraceAction& operation);

  Phase phase() const { return phase_; }
  // Explicit caller export, available only after acknowledged successful stop.
  // This JSON is summary metadata. It cannot export/redact native trace data.
  std::optional<std::string> ExportSummary() const;

 private:
  bool Matches(TraceId session) const;
  bool Matches(const TraceAction& operation) const;
  Transition Act(NativeAction action);

  Phase phase_ = Phase::kIdle;
  std::optional<TraceId> session_;
  std::optional<TraceAction> pending_;
  std::shared_ptr<const TraceAction::Token> recorder_token_;
  std::optional<TraceSummary> summary_;
};

}  // namespace arcified::browser_diagnostics

#endif  // ARCIFIED_NATIVE_FEATURES_BROWSER_DIAGNOSTICS_DIAGNOSTICS_H_
