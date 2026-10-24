#ifndef ARCIFIED_NATIVE_FEATURES_LOCAL_FIND_FIND_COMMAND_H_
#define ARCIFIED_NATIVE_FEATURES_LOCAL_FIND_FIND_COMMAND_H_

#include <optional>
#include <string>
#include <variant>

#include "../../model/lifecycle.h"

namespace arcified::local_find {

enum class Command { kOpen, kNext, kPrevious, kClose };

// Supplied by the native/input owner after native IME processing. This module
// neither interprets key events nor infers eligibility from platform IME APIs.
enum class CommandEligibility { kEligible, kDeferToNativeInput };

// Concrete dispatch payloads. Open calls Show(false, true), advance calls
// Show(true, direction == kForward), end calls EndFindSession(kKeep, kKeep).
struct OpenFindBar {};
enum class Direction { kForward, kBackward };
struct AdvanceMatch {
  Direction direction;
};
struct EndSessionKeepingSelectionAndResults {};
using NativeOperation = std::
    variant<OpenFindBar, AdvanceMatch, EndSessionKeepingSelectionAndResults>;

// An opaque, nonempty caller token. It must change whenever focus continuity,
// controller/WebContents binding, document, or native session is invalidated,
// including an away-and-back transition. Never derive it from a URL or pointer.
struct BindingGeneration {
  std::string value;
};

// A borrowed observation, not a registry or a second Find session. Supply no
// snapshot when the controller, WebContents or FindTabHelper is unavailable.
struct ControllerSnapshot {
  lifecycle::PageId bound_page;
  lifecycle::ProfileId bound_profile;
  BindingGeneration generation;
  bool find_ui_active;
};

enum class Refusal {
  kDeferToNativeInput,
  kControllerUnavailable,
  kPageMismatch,
  kProfileMismatch,
  kMissingGeneration,
  kStaleGeneration,
  kNoActiveSession,
  kUnsupportedCommand,
};

class FindPlan;
using PlanResult = std::variant<Refusal, FindPlan>;

// Only PlanFindCommand can construct a plan. It grants no native authority and
// owns no query, match results, focus, attachment or navigation state.
class FindPlan {
 public:
  const NativeOperation& operation() const { return operation_; }
  const lifecycle::PageId& page() const { return page_; }
  const lifecycle::ProfileId& profile() const { return profile_; }
  const BindingGeneration& generation() const { return generation_; }

 private:
  friend PlanResult PlanFindCommand(const lifecycle::PageId&,
                                    const lifecycle::ProfileId&,
                                    Command,
                                    CommandEligibility,
                                    const std::optional<ControllerSnapshot>&);

  FindPlan(NativeOperation operation, const ControllerSnapshot& controller);

  NativeOperation operation_;
  lifecycle::PageId page_;
  lifecycle::ProfileId profile_;
  BindingGeneration generation_;
};

// Pass page/profile fields from the common commands::PageCommandTarget after
// its resolver succeeds. This function only checks the Find controller binding;
// it does not resolve a target or consult/mutate the lifecycle model.
PlanResult PlanFindCommand(const lifecycle::PageId& validated_focused_page,
                           const lifecycle::ProfileId& validated_profile,
                           Command command,
                           CommandEligibility eligibility,
                           const std::optional<ControllerSnapshot>& controller);

// No refusal means the observed Find binding is still suitable. Immediately
// before the effect, caller must also revalidate the common lifecycle target,
// authoritative focus, exact native objects, immutable Profile and attachment.
// Run checks and effect on the serialized UI sequence without yielding. A plan
// is not a reservation, and this check cannot make native callbacks atomic.
std::optional<Refusal> RevalidateFindPlan(
    const FindPlan& plan,
    const lifecycle::PageId& validated_focused_page,
    const lifecycle::ProfileId& validated_profile,
    CommandEligibility eligibility,
    const std::optional<ControllerSnapshot>& controller);

}  // namespace arcified::local_find

#endif  // ARCIFIED_NATIVE_FEATURES_LOCAL_FIND_FIND_COMMAND_H_
