#include "diagnostics.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <variant>

namespace {

namespace d = arcified::browser_diagnostics;
namespace l = arcified::lifecycle;

void Require(bool condition, const char* behavior) {
  if (!condition) {
    std::cerr << "FAIL: " << behavior << '\n';
    std::exit(EXIT_FAILURE);
  }
}

void NoAction(d::Transition result, d::Status status = d::Status::kOk) {
  Require(result.status == status, "transition returns expected status");
  Require(!result.action, "transition does not dispatch another native action");
}

d::TraceAction Action(d::Transition result,
                      d::NativeAction action,
                      d::TraceId session) {
  Require(result.status == d::Status::kOk, "action request succeeds");
  Require(result.action.has_value(), "native action is emitted");
  Require(result.action->action() == action, "native action has intended kind");
  Require(result.action->session() == session,
          "action retains session identity");
  Require(result.action->scope() == d::Scope::kBrowserProcesses,
          "trace action explicitly covers browser processes");
  return *result.action;
}

struct Fixture {
  l::Model model;
  const l::WindowId window{"synthetic-window"};

  Fixture() {
    Require(model.AddProfile(l::ProfileId{"profile-a"}) == l::Status::kOk,
            "first synthetic Profile exists");
    Require(model.AddProfile(l::ProfileId{"profile-b"}) == l::Status::kOk,
            "second synthetic Profile exists");
    Require(model.AddSpace(l::SpaceId{"space-a"}, l::ProfileId{"profile-a"}) ==
                l::Status::kOk,
            "first synthetic Space exists");
    Require(model.AddSpace(l::SpaceId{"space-b"}, l::ProfileId{"profile-b"}) ==
                l::Status::kOk,
            "second synthetic Space exists");
    Require(model.AddWindow(window) == l::Status::kOk,
            "invoking window exists");
    Require(model.Attach({window, l::SpaceId{"space-a"}}) == l::Status::kOk,
            "first Space attached");
    Require(model.Attach({window, l::SpaceId{"space-b"}}) == l::Status::kOk,
            "second Space attached");
  }

  d::TraceAction Start(d::TraceSession& trace, d::TraceId id) {
    return Action(trace.Start(model, window, d::Capability::kAvailable, id),
                  d::NativeAction::kStart, id);
  }
};

void TaskManagerScopeAndRevalidation() {
  Fixture f;
  auto unavailable =
      d::PlanTaskManager(f.model, f.window, d::Capability::kUnavailable);
  Require(std::get<d::Status>(unavailable) == d::Status::kUnavailable,
          "unavailable native task manager emits no plan");
  for (const auto& space : {l::SpaceId{"space-a"}, l::SpaceId{"space-b"}}) {
    Require(f.model.SelectSpace(f.window, space) == l::Status::kOk,
            "synthetic Space selection succeeds");
    auto result =
        d::PlanTaskManager(f.model, f.window, d::Capability::kAvailable);
    const auto& plan = std::get<d::TaskManagerPlan>(result);
    Require(plan.invoking_window == f.window,
            "task manager uses explicit invocation window");
    Require(plan.scope == d::Scope::kBrowserProcesses,
            "changing selected Profile does not narrow task manager scope");
  }
  Require(f.model.CloseWindow(f.window) == l::Status::kOk,
          "empty synthetic window closes");
  const auto stale =
      d::PlanTaskManager(f.model, f.window, d::Capability::kAvailable);
  Require(std::get<d::Status>(stale) == d::Status::kWindowNotFound,
          "replanning rejects a closed invocation window");
}

void StopSummaryAndSessionIsolation() {
  Fixture f;
  d::TraceSession trace;
  const d::TraceId first{1};
  const d::TraceId second{2};
  Require(!trace.ExportSummary(), "idle trace has nothing to export");
  NoAction(trace.Stop(first), d::Status::kStaleSession);
  NoAction(trace.Cancel(first), d::Status::kStaleSession);
  NoAction(trace.Start(f.model, f.window, d::Capability::kUnavailable, first),
           d::Status::kUnavailable);
  NoAction(trace.Start(f.model, l::WindowId{"missing"},
                       d::Capability::kAvailable, first),
           d::Status::kWindowNotFound);
  Require(trace.phase() == d::Phase::kIdle,
          "rejected starts do not consume identity or change phase");
  const auto start = f.Start(trace, first);
  NoAction(trace.Start(f.model, f.window, d::Capability::kAvailable, second),
           d::Status::kBusy);
  NoAction(trace.Stop(first), d::Status::kWrongPhase);
  NoAction(trace.AcknowledgeStop(start, {1, 1, 1}), d::Status::kWrongPhase);
  d::TraceSession foreign;
  const auto foreign_start = f.Start(foreign, first);
  NoAction(trace.AcknowledgeStart(foreign_start, true),
           d::Status::kStaleOperation);
  Require(trace.phase() == d::Phase::kStarting,
          "foreign completion with same session number cannot start recorder");
  NoAction(trace.AcknowledgeStart(start, true));
  Require(trace.phase() == d::Phase::kRecording,
          "only native start acknowledgement publishes recording");
  NoAction(trace.AcknowledgeStart(start, true), d::Status::kStaleOperation);
  const auto stop =
      Action(trace.Stop(first), d::NativeAction::kStopForSummary, first);
  Require(!trace.ExportSummary(), "stop request alone cannot export metadata");
  NoAction(trace.Stop(first), d::Status::kWrongPhase);
  NoAction(trace.AcknowledgeDiscard(stop), d::Status::kWrongPhase);
  NoAction(trace.AcknowledgeStop(
      stop, {1250, 0, std::numeric_limits<std::uint64_t>::max()}));
  Require(trace.phase() == d::Phase::kReady,
          "acknowledged native stop and discard publishes summary");
  const std::string expected =
      "{\"kind\":\"arcified-diagnostics-summary\","
      "\"scope\":\"browser-processes\",\"elapsed_us\":1250,"
      "\"process_count\":0,\"event_count\":18446744073709551615}\n";
  Require(trace.ExportSummary() == expected,
          "export contains exactly the metadata schema and lossless counters");
  NoAction(trace.Start(f.model, f.window, d::Capability::kAvailable, first),
           d::Status::kStaleSession);
  NoAction(trace.Start(f.model, f.window, d::Capability::kUnavailable, second),
           d::Status::kUnavailable);
  Require(trace.ExportSummary() == expected,
          "failed start preserves prior completed metadata");
  const auto next_start = f.Start(trace, second);
  Require(!trace.ExportSummary(), "new session clears previous summary");
  NoAction(trace.AcknowledgeStop(stop, {9, 9, 9}), d::Status::kStaleOperation);
  NoAction(trace.AcknowledgeStart(start, true), d::Status::kStaleOperation);
  NoAction(trace.Cancel(first), d::Status::kStaleSession);
  NoAction(trace.AcknowledgeStart(next_start, false));
  Require(
      trace.phase() == d::Phase::kStartFailed,
      "native rejection publishes start failure without synthetic recording");
  Require(!trace.ExportSummary(), "failed start never exports a trace summary");
}

void CancelDuringStart() {
  Fixture f;
  d::TraceSession trace;
  const d::TraceId id{10};
  const auto start = f.Start(trace, id);
  NoAction(trace.Cancel(id));
  Require(trace.phase() == d::Phase::kCancelPendingStart,
          "cancel waits for outstanding native start");
  NoAction(trace.Cancel(id));
  NoAction(trace.AcknowledgeDiscard(start), d::Status::kWrongPhase);
  NoAction(trace.Start(f.model, f.window, d::Capability::kAvailable, {11}),
           d::Status::kBusy);
  const auto discard = Action(trace.AcknowledgeStart(start, true),
                              d::NativeAction::kStopAndDiscard, id);
  Require(trace.phase() == d::Phase::kDiscarding,
          "late successful start must stop and discard before cancellation");
  Require(!trace.ExportSummary(), "cancelled start cannot export metadata");
  NoAction(trace.AcknowledgeStart(start, true), d::Status::kStaleOperation);
  NoAction(trace.Cancel(id));
  NoAction(trace.AcknowledgeDiscard(discard));
  Require(trace.phase() == d::Phase::kCancelled,
          "native discard acknowledgement completes cancellation");
  NoAction(trace.AcknowledgeDiscard(discard), d::Status::kStaleOperation);

  const auto rejected = f.Start(trace, {11});
  NoAction(trace.Cancel({11}));
  NoAction(trace.AcknowledgeStart(rejected, false));
  Require(trace.phase() == d::Phase::kCancelled,
          "rejected native start needs no stop of another recorder");
}

void CancelDuringStop() {
  Fixture f;
  d::TraceSession trace;
  const d::TraceId id{20};
  const auto start = f.Start(trace, id);
  NoAction(trace.AcknowledgeStart(start, true));
  const auto stop =
      Action(trace.Stop(id), d::NativeAction::kStopForSummary, id);
  NoAction(trace.Cancel(id));
  Require(trace.phase() == d::Phase::kCancelPendingStop,
          "cancel keeps pending native stop without dispatching a second stop");
  NoAction(trace.Cancel(id));
  NoAction(trace.Start(f.model, f.window, d::Capability::kAvailable, {21}),
           d::Status::kBusy);
  Require(!trace.ExportSummary(), "cancellation request does not expose data");
  NoAction(trace.AcknowledgeStop(stop, {42, 3, 17}));
  Require(trace.phase() == d::Phase::kCancelled,
          "acknowledged stop and raw-data discard completes pending cancel");
  Require(!trace.ExportSummary(), "stop completion after cancel drops summary");
}

void CleanupFailureAndDelayedRetryEvents() {
  Fixture f;
  d::TraceSession trace;
  const d::TraceId id{30};
  const auto start = f.Start(trace, id);
  NoAction(trace.AcknowledgeStart(start, true));
  const auto discard =
      Action(trace.Cancel(id), d::NativeAction::kStopAndDiscard, id);
  NoAction(trace.ReportNativeFailure(discard));
  Require(trace.phase() == d::Phase::kCleanupRequired,
          "native discard failure remains unresolved");
  NoAction(trace.Start(f.model, f.window, d::Capability::kAvailable, {31}),
           d::Status::kBusy);
  Require(!trace.ExportSummary(), "cleanup failure does not export metadata");
  const auto retry =
      Action(trace.Cancel(id), d::NativeAction::kReconcileAndDiscard, id);
  NoAction(trace.AcknowledgeDiscard(discard), d::Status::kStaleOperation);
  NoAction(trace.ReportNativeFailure(discard), d::Status::kStaleOperation);
  Require(trace.phase() == d::Phase::kDiscarding,
          "delayed prior operation cannot complete or fail cleanup retry");
  NoAction(trace.AcknowledgeDiscard(retry));
  Require(trace.phase() == d::Phase::kCancelled,
          "current retry acknowledgement resolves cleanup");
}

void StopFailureAndWindowClosure() {
  Fixture f;
  d::TraceSession trace;
  const d::TraceId id{40};
  const auto start = f.Start(trace, id);
  NoAction(trace.AcknowledgeStart(start, true));
  Require(f.model.CloseWindow(f.window) == l::Status::kOk,
          "synthetic invocation window closes while trace is live");
  const auto stop =
      Action(trace.Stop(id), d::NativeAction::kStopForSummary, id);
  NoAction(trace.Cancel(id));
  NoAction(trace.ReportNativeFailure(stop));
  Require(trace.phase() == d::Phase::kCleanupRequired,
          "failed stop after cancel does not report cancellation complete");
  NoAction(trace.AcknowledgeStop(stop, {1, 1, 1}), d::Status::kStaleOperation);
  const auto reconcile =
      Action(trace.Cancel(id), d::NativeAction::kReconcileAndDiscard, id);
  NoAction(trace.AcknowledgeDiscard(reconcile));
  Require(trace.phase() == d::Phase::kCancelled,
          "browser-wide cleanup outlives closed invocation window");
  Require(!trace.ExportSummary(), "failed native stop never creates an export");
}

void CancelCompletedMetadata() {
  Fixture f;
  d::TraceSession trace;
  const d::TraceId id{50};
  const auto start = f.Start(trace, id);
  NoAction(trace.AcknowledgeStart(start, true));
  const auto stop =
      Action(trace.Stop(id), d::NativeAction::kStopForSummary, id);
  NoAction(trace.AcknowledgeStop(stop, {0, 0, 0}));
  Require(trace.ExportSummary().has_value(), "completed summary is exportable");
  NoAction(trace.Cancel(id));
  Require(trace.phase() == d::Phase::kCancelled,
          "completed native cleanup needs no new recorder command");
  Require(!trace.ExportSummary(), "cancel clears completed metadata");
  NoAction(trace.Cancel(id));
}

void NativeStartAndRecorderFailures() {
  Fixture f;
  d::TraceSession trace;
  auto start = f.Start(trace, {60});
  const auto retained_start = std::move(start);
  d::TraceSession idle;
  NoAction(idle.ReportNativeFailure(start), d::Status::kStaleOperation);
  Require(idle.phase() == d::Phase::kIdle,
          "moved-from completion cannot manufacture a native failure");
  NoAction(trace.Cancel({60}));
  NoAction(trace.ReportNativeFailure(retained_start));
  Require(
      trace.phase() == d::Phase::kCleanupRequired,
      "uncertain asynchronous start failure requires native reconciliation");
  NoAction(trace.AcknowledgeStart(retained_start, true),
           d::Status::kStaleOperation);
  const auto cleanup =
      Action(trace.Cancel({60}), d::NativeAction::kReconcileAndDiscard, {60});
  NoAction(trace.AcknowledgeDiscard(cleanup));

  const auto next_start = f.Start(trace, {61});
  NoAction(trace.AcknowledgeStart(next_start, true));
  NoAction(trace.ReportNativeFailure(retained_start),
           d::Status::kStaleOperation);
  NoAction(trace.ReportNativeFailure(next_start));
  Require(trace.phase() == d::Phase::kCleanupRequired,
          "native service failure while recording cannot leave a usable trace");
  Require(!trace.ExportSummary(), "native service failure cannot export data");
  const auto next_cleanup =
      Action(trace.Cancel({61}), d::NativeAction::kReconcileAndDiscard, {61});
  NoAction(trace.AcknowledgeDiscard(next_cleanup));
}

}  // namespace

int main() {
  TaskManagerScopeAndRevalidation();
  StopSummaryAndSessionIsolation();
  CancelDuringStart();
  CancelDuringStop();
  CleanupFailureAndDelayedRetryEvents();
  StopFailureAndWindowClosure();
  CancelCompletedMetadata();
  NativeStartAndRecorderFailures();
  std::cout << "browser diagnostics behavioral checks passed\n";
}
