#include "presentation_transition.h"

#include <algorithm>
#include <cmath>

namespace arcified::motion {

std::optional<Progress> Progress::Parse(double value) {
  if (!std::isfinite(value) || value < 0.0 || value > 1.0) {
    return std::nullopt;
  }
  return Progress(value);
}

std::optional<RunToken> PresentationTransition::Retarget(Progress target,
                                                         MotionMode mode) {
  run_.reset();
  if (mode == MotionMode::kReduced || value_.value() == target.value()) {
    value_ = target;
    return std::nullopt;
  }
  run_ = Run{RunToken(), value_, target};
  return run_->token;
}

const PresentationTransition::Run* PresentationTransition::MatchingRun(
    const RunToken& token) const {
  if (!run_ || run_->token.identity_ != token.identity_) {
    return nullptr;
  }
  return &*run_;
}

bool PresentationTransition::Sample(const RunToken& token, Progress phase) {
  const Run* run = MatchingRun(token);
  if (!run) {
    return false;
  }
  if (phase.value() == 1.0) {
    value_ = run->target;
  } else if (phase.value() == 0.0) {
    value_ = run->start;
  } else {
    const double start = run->start.value();
    const double target = run->target.value();
    // Keep floating-point rounding inside the supplied endpoints.
    value_ =
        Progress(std::clamp(start + (target - start) * phase.value(),
                            std::min(start, target), std::max(start, target)));
  }
  return true;
}

bool PresentationTransition::Finish(const RunToken& token) {
  const Run* run = MatchingRun(token);
  if (!run) {
    return false;
  }
  value_ = run->target;
  run_.reset();
  return true;
}

bool PresentationTransition::Cancel(const RunToken& token) {
  if (!MatchingRun(token)) {
    return false;
  }
  run_.reset();
  return true;
}

void PresentationTransition::Reset(Progress authoritative_value) {
  run_.reset();
  value_ = authoritative_value;
}

bool PresentationTransition::ReduceMotion() {
  if (!run_) {
    return false;
  }
  value_ = run_->target;
  run_.reset();
  return true;
}

}  // namespace arcified::motion
