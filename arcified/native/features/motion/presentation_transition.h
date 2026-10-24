#ifndef ARCIFIED_NATIVE_FEATURES_MOTION_PRESENTATION_TRANSITION_H_
#define ARCIFIED_NATIVE_FEATURES_MOTION_PRESENTATION_TRANSITION_H_

#include <memory>
#include <optional>

namespace arcified::motion {

// A finite normalized presentation value or externally evaluated curve phase.
// [0, 1] is the interpolation domain, not a duration or product limit.
class Progress {
 public:
  static std::optional<Progress> Parse(double value);
  static Progress Start() { return Progress(0.0); }
  static Progress End() { return Progress(1.0); }
  double value() const { return value_; }

 private:
  friend class PresentationTransition;
  explicit Progress(double value) : value_(value) {}
  double value_;
};

enum class MotionMode { kAnimate, kReduced };

// A run identity, not a native animation or permission to change layout.
// Retained tokens keep their identity alive, so it cannot be reused while a
// stale callback can still present that token. No counter can wrap or expire.
class RunToken {
 public:
  RunToken(const RunToken&) = default;
  RunToken& operator=(const RunToken&) = default;

 private:
  friend class PresentationTransition;
  struct Identity {};
  RunToken() : identity_(std::make_shared<const Identity>()) {}
  std::shared_ptr<const Identity> identity_;
};

// Synchronous presentation interpolation, with no timer, easing, view, focus,
// visibility, Space selection or persistence. Use on the caller's UI sequence.
// The native caller owns rendering and must apply accepted values in order.
class PresentationTransition {
 public:
  explicit PresentationTransition(Progress initial) : value_(initial) {}
  PresentationTransition(const PresentationTransition&) = delete;
  PresentationTransition& operator=(const PresentationTransition&) = delete;

  Progress value() const { return value_; }

  // Invalidates the previous run before returning. Reversal starts at value().
  // Reduced motion and zero distance settle immediately, returning no run.
  // Mode is supplied anew for each request, never stored as a preference.
  [[nodiscard]] std::optional<RunToken> Retarget(Progress target,
                                                 MotionMode mode);

  // Phase is already evaluated by the caller's timing/easing implementation.
  // Any valid phase is accepted for the current run, including a falling curve.
  // Sampling phase 1 does not finish the native run; Finish is explicit.
  [[nodiscard]] bool Sample(const RunToken& token, Progress phase);
  [[nodiscard]] bool Finish(const RunToken& token);

  // Invalidates only the matching run, leaving value() unchanged. Returns no
  // geometry or target. The caller may then Reset a freshly queried native
  // authoritative presentation. A late cancellation cannot cancel a new run.
  [[nodiscard]] bool Cancel(const RunToken& token);
  void Reset(Progress authoritative_value);

  // Report a newly enabled reduced-motion setting. Settles the latest active
  // target and invalidates its token. After Cancel, there is no target to
  // replay. Disabling reduced motion does not restart anything; a new request
  // supplies kAnimate to Retarget. The caller must stop its own native
  // animation.
  [[nodiscard]] bool ReduceMotion();

 private:
  struct Run {
    RunToken token;
    Progress start;
    Progress target;
  };

  const Run* MatchingRun(const RunToken& token) const;

  Progress value_;
  std::optional<Run> run_;
};

}  // namespace arcified::motion

#endif  // ARCIFIED_NATIVE_FEATURES_MOTION_PRESENTATION_TRANSITION_H_
