#include "diagnostics.h"

#include <utility>

namespace arcified::browser_diagnostics {
namespace {

Transition Result(Status status = Status::kOk) {
  return {status, std::nullopt};
}

bool CanStart(Phase phase) {
  switch (phase) {
    case Phase::kIdle:
    case Phase::kReady:
    case Phase::kCancelled:
    case Phase::kStartFailed:
      return true;
    case Phase::kStarting:
    case Phase::kRecording:
    case Phase::kStopping:
    case Phase::kCancelPendingStart:
    case Phase::kCancelPendingStop:
    case Phase::kDiscarding:
    case Phase::kCleanupRequired:
      return false;
  }
  return false;
}

}  // namespace

TaskManagerResult PlanTaskManager(const lifecycle::Model& model,
                                  lifecycle::WindowId invoking_window,
                                  Capability capability) {
  if (capability != Capability::kAvailable)
    return Status::kUnavailable;
  if (model.windows().find(invoking_window) == model.windows().end())
    return Status::kWindowNotFound;
  return TaskManagerPlan{std::move(invoking_window)};
}

bool TraceSession::Matches(TraceId session) const {
  return session_ && *session_ == session;
}

bool TraceSession::Matches(const TraceAction& operation) const {
  return pending_ && pending_->token_ == operation.token_;
}

Transition TraceSession::Act(NativeAction action) {
  pending_ = TraceAction{*session_, action};
  return {Status::kOk, pending_};
}

Transition TraceSession::Start(const lifecycle::Model& model,
                               lifecycle::WindowId invoking_window,
                               Capability capability,
                               TraceId session) {
  if (!CanStart(phase_))
    return Result(Status::kBusy);
  if (session_ && session.value <= session_->value)
    return Result(Status::kStaleSession);
  if (capability != Capability::kAvailable)
    return Result(Status::kUnavailable);
  if (model.windows().find(invoking_window) == model.windows().end())
    return Result(Status::kWindowNotFound);
  session_ = session;
  summary_.reset();
  phase_ = Phase::kStarting;
  return Act(NativeAction::kStart);
}

Transition TraceSession::Stop(TraceId session) {
  if (!Matches(session))
    return Result(Status::kStaleSession);
  if (phase_ != Phase::kRecording)
    return Result(Status::kWrongPhase);
  recorder_token_.reset();
  phase_ = Phase::kStopping;
  return Act(NativeAction::kStopForSummary);
}

Transition TraceSession::Cancel(TraceId session) {
  if (!Matches(session))
    return Result(Status::kStaleSession);
  switch (phase_) {
    case Phase::kStarting:
      phase_ = Phase::kCancelPendingStart;
      return Result();
    case Phase::kRecording:
      recorder_token_.reset();
      phase_ = Phase::kDiscarding;
      return Act(NativeAction::kStopAndDiscard);
    case Phase::kStopping:
      phase_ = Phase::kCancelPendingStop;
      return Result();
    case Phase::kCleanupRequired:
      phase_ = Phase::kDiscarding;
      return Act(NativeAction::kReconcileAndDiscard);
    case Phase::kReady:
    case Phase::kStartFailed:
      summary_.reset();
      phase_ = Phase::kCancelled;
      return Result();
    case Phase::kCancelPendingStart:
    case Phase::kCancelPendingStop:
    case Phase::kDiscarding:
    case Phase::kCancelled:
      return Result();
    case Phase::kIdle:
      return Result(Status::kWrongPhase);
  }
  return Result(Status::kWrongPhase);
}

Transition TraceSession::AcknowledgeStart(const TraceAction& operation,
                                          bool started) {
  if (!Matches(operation))
    return Result(Status::kStaleOperation);
  if (phase_ != Phase::kStarting && phase_ != Phase::kCancelPendingStart)
    return Result(Status::kWrongPhase);
  const bool cancelling = phase_ == Phase::kCancelPendingStart;
  pending_.reset();
  if (!started) {
    phase_ = cancelling ? Phase::kCancelled : Phase::kStartFailed;
    return Result();
  }
  phase_ = cancelling ? Phase::kDiscarding : Phase::kRecording;
  if (!cancelling)
    recorder_token_ = operation.token_;
  return cancelling ? Act(NativeAction::kStopAndDiscard) : Result();
}

Transition TraceSession::AcknowledgeStop(const TraceAction& operation,
                                         TraceSummary summary) {
  if (!Matches(operation))
    return Result(Status::kStaleOperation);
  if (phase_ == Phase::kCancelPendingStop) {
    pending_.reset();
    phase_ = Phase::kCancelled;
    return Result();
  }
  if (phase_ != Phase::kStopping)
    return Result(Status::kWrongPhase);
  pending_.reset();
  summary_ = summary;
  phase_ = Phase::kReady;
  return Result();
}

Transition TraceSession::AcknowledgeDiscard(const TraceAction& operation) {
  if (!Matches(operation))
    return Result(Status::kStaleOperation);
  if (phase_ != Phase::kDiscarding)
    return Result(Status::kWrongPhase);
  pending_.reset();
  phase_ = Phase::kCancelled;
  return Result();
}

Transition TraceSession::ReportNativeFailure(const TraceAction& operation) {
  if (!Matches(operation) &&
      !(recorder_token_ && recorder_token_ == operation.token_))
    return Result(Status::kStaleOperation);
  pending_.reset();
  recorder_token_.reset();
  phase_ = Phase::kCleanupRequired;
  return Result();
}

std::optional<std::string> TraceSession::ExportSummary() const {
  if (phase_ != Phase::kReady)
    return std::nullopt;
  return "{\"kind\":\"arcified-diagnostics-summary\","
         "\"scope\":\"browser-processes\",\"elapsed_us\":" +
         std::to_string(summary_->elapsed_microseconds) +
         ",\"process_count\":" + std::to_string(summary_->process_count) +
         ",\"event_count\":" + std::to_string(summary_->event_count) + "}\n";
}

}  // namespace arcified::browser_diagnostics
