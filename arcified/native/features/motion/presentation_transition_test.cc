#include "presentation_transition.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <type_traits>

namespace arcified::motion {
namespace {

[[noreturn]] void Fail(const char* expression, int line) {
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression)        \
  do {                             \
    if (!(expression)) {           \
      Fail(#expression, __LINE__); \
    }                              \
  } while (false)

Progress At(double value) {
  const auto parsed = Progress::Parse(value);
  REQUIRE(parsed.has_value());
  return *parsed;
}

static_assert(!std::is_constructible_v<Progress, double>);
static_assert(!std::is_default_constructible_v<RunToken>);
static_assert(!std::is_copy_constructible_v<PresentationTransition>);

void BoundaryValidation() {
  REQUIRE(At(0.0).value() == 0.0);
  REQUIRE(At(1.0).value() == 1.0);
  REQUIRE(At(0.5).value() == 0.5);
  REQUIRE(!Progress::Parse(-0.5));
  REQUIRE(!Progress::Parse(1.5));
  REQUIRE(!Progress::Parse(std::numeric_limits<double>::infinity()));
  REQUIRE(!Progress::Parse(-std::numeric_limits<double>::infinity()));
  REQUIRE(!Progress::Parse(std::numeric_limits<double>::quiet_NaN()));
}

void ExternalCurveAndExplicitCompletion() {
  PresentationTransition transition(At(0.25));
  const auto run = transition.Retarget(At(0.75), MotionMode::kAnimate);
  REQUIRE(run.has_value());
  REQUIRE(transition.Sample(*run, At(0.5)));
  REQUIRE(transition.value().value() == 0.5);
  // The component does not impose a monotonic easing curve.
  REQUIRE(transition.Sample(*run, At(0.25)));
  REQUIRE(transition.value().value() == 0.375);
  REQUIRE(transition.Sample(*run, Progress::Start()));
  REQUIRE(transition.value().value() == 0.25);
  REQUIRE(transition.Sample(*run, Progress::End()));
  REQUIRE(transition.value().value() == 0.75);
  REQUIRE(transition.Finish(*run));
  REQUIRE(!transition.Finish(*run));
  REQUIRE(!transition.Sample(*run, Progress::Start()));
  REQUIRE(transition.value().value() == 0.75);
}

void RepeatedReversalRejectsOldCallbacks() {
  PresentationTransition transition(Progress::Start());
  const auto opening =
      transition.Retarget(Progress::End(), MotionMode::kAnimate);
  REQUIRE(opening.has_value());
  REQUIRE(transition.Sample(*opening, At(0.5)));
  const auto closing =
      transition.Retarget(Progress::Start(), MotionMode::kAnimate);
  REQUIRE(closing.has_value());
  REQUIRE(transition.value().value() == 0.5);
  REQUIRE(!transition.Sample(*opening, Progress::End()));
  REQUIRE(!transition.Finish(*opening));
  REQUIRE(!transition.Cancel(*opening));
  REQUIRE(transition.Sample(*closing, At(0.5)));
  REQUIRE(transition.value().value() == 0.25);
  const auto reopening =
      transition.Retarget(Progress::End(), MotionMode::kAnimate);
  REQUIRE(reopening.has_value());
  REQUIRE(transition.Sample(*reopening, At(0.5)));
  REQUIRE(transition.value().value() == 0.625);
  REQUIRE(!transition.Finish(*closing));
  REQUIRE(transition.Finish(*reopening));
  REQUIRE(transition.value().value() == 1.0);
}

void CancellationAndAuthoritativeSnap() {
  PresentationTransition transition(Progress::Start());
  const auto run = transition.Retarget(Progress::End(), MotionMode::kAnimate);
  REQUIRE(run.has_value());
  REQUIRE(transition.Sample(*run, At(0.5)));
  REQUIRE(transition.Cancel(*run));
  REQUIRE(transition.value().value() == 0.5);
  REQUIRE(!transition.Cancel(*run));
  REQUIRE(!transition.Finish(*run));
  REQUIRE(!transition.ReduceMotion());
  REQUIRE(transition.value().value() == 0.5);
  // Layout queried a different authoritative presentation after cancellation.
  transition.Reset(At(0.75));
  REQUIRE(!transition.Sample(*run, Progress::End()));
  REQUIRE(transition.value().value() == 0.75);
  const auto next =
      transition.Retarget(Progress::Start(), MotionMode::kAnimate);
  REQUIRE(next.has_value());
  REQUIRE(transition.Sample(*next, At(0.5)));
  REQUIRE(transition.value().value() == 0.375);
  // Reset also cancels a run when native rendering is replaced externally.
  transition.Reset(At(0.25));
  REQUIRE(!transition.Finish(*next));
  REQUIRE(!transition.ReduceMotion());
  REQUIRE(transition.value().value() == 0.25);
}

void ReducedMotionDuringReversalAndToggle() {
  PresentationTransition transition(Progress::Start());
  const auto opening =
      transition.Retarget(Progress::End(), MotionMode::kAnimate);
  REQUIRE(opening.has_value());
  REQUIRE(transition.Sample(*opening, At(0.5)));
  const auto closing =
      transition.Retarget(Progress::Start(), MotionMode::kAnimate);
  REQUIRE(closing.has_value());
  REQUIRE(transition.ReduceMotion());
  REQUIRE(transition.value().value() == 0.0);
  REQUIRE(!transition.Sample(*closing, Progress::Start()));
  REQUIRE(!transition.Finish(*opening));
  REQUIRE(!transition.ReduceMotion());
  REQUIRE(!transition.Retarget(Progress::End(), MotionMode::kReduced));
  REQUIRE(transition.value().value() == 1.0);
  // Re-enabling animation takes a fresh explicit request with a new token.
  const auto enabled =
      transition.Retarget(Progress::Start(), MotionMode::kAnimate);
  REQUIRE(enabled.has_value());
  REQUIRE(!transition.Cancel(*closing));
  REQUIRE(transition.Sample(*enabled, At(0.5)));
  REQUIRE(transition.value().value() == 0.5);
  REQUIRE(!transition.Retarget(At(0.75), MotionMode::kReduced));
  REQUIRE(!transition.Finish(*enabled));
  REQUIRE(transition.value().value() == 0.75);
}

void NoDistanceAndSameTargetReplacement() {
  PresentationTransition transition(Progress::Start());
  REQUIRE(!transition.Retarget(Progress::Start(), MotionMode::kAnimate));
  const auto first = transition.Retarget(Progress::End(), MotionMode::kAnimate);
  REQUIRE(first.has_value());
  REQUIRE(transition.Sample(*first, At(0.5)));
  const auto second =
      transition.Retarget(Progress::End(), MotionMode::kAnimate);
  REQUIRE(second.has_value());
  REQUIRE(!transition.Sample(*first, Progress::End()));
  REQUIRE(transition.Sample(*second, At(0.5)));
  REQUIRE(transition.value().value() == 0.75);
  REQUIRE(!transition.Retarget(At(0.75), MotionMode::kAnimate));
  REQUIRE(!transition.Finish(*second));
  REQUIRE(!transition.ReduceMotion());
}

void TokensAreScopedToTheirRunAndOwner() {
  PresentationTransition first(Progress::Start());
  PresentationTransition second(Progress::Start());
  const auto first_run = first.Retarget(Progress::End(), MotionMode::kAnimate);
  const auto second_run =
      second.Retarget(Progress::End(), MotionMode::kAnimate);
  REQUIRE(first_run.has_value());
  REQUIRE(second_run.has_value());
  const RunToken retained = *first_run;
  REQUIRE(!second.Sample(retained, Progress::End()));
  REQUIRE(!second.Cancel(retained));
  REQUIRE(first.Cancel(*first_run));
  const auto replacement =
      first.Retarget(Progress::End(), MotionMode::kAnimate);
  REQUIRE(replacement.has_value());
  REQUIRE(!first.Finish(retained));
  REQUIRE(first.Finish(*replacement));
  REQUIRE(second.Finish(*second_run));
  const RunToken destroyed_owner = [] {
    PresentationTransition temporary(Progress::Start());
    const auto run = temporary.Retarget(Progress::End(), MotionMode::kAnimate);
    REQUIRE(run.has_value());
    return *run;
  }();
  const auto later = second.Retarget(Progress::Start(), MotionMode::kAnimate);
  REQUIRE(later.has_value());
  REQUIRE(!second.Sample(destroyed_owner, Progress::End()));
  REQUIRE(second.Finish(*later));
}

}  // namespace
}  // namespace arcified::motion

int main() {
  using namespace arcified::motion;
  BoundaryValidation();
  ExternalCurveAndExplicitCompletion();
  RepeatedReversalRejectsOldCallbacks();
  CancellationAndAuthoritativeSnap();
  ReducedMotionDuringReversalAndToggle();
  NoDistanceAndSameTargetReplacement();
  TokensAreScopedToTheirRunAndOwner();
  std::cout << "7 presentation transition fixtures passed\n";
}
