#include "find_command.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {

using arcified::lifecycle::PageId;
using arcified::lifecycle::ProfileId;
using namespace arcified::local_find;

void Expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

const PageId kPage{"focused-page"};
const ProfileId kProfile{"profile"};
constexpr auto kEligible = CommandEligibility::kEligible;
constexpr auto kDefer = CommandEligibility::kDeferToNativeInput;

ControllerSnapshot BoundController() {
  return {kPage, kProfile, {"caller-epoch"}, true};
}

FindPlan Plan(Command command) {
  auto result =
      PlanFindCommand(kPage, kProfile, command, kEligible, BoundController());
  Expect(std::holds_alternative<FindPlan>(result), "expected native plan");
  return std::get<FindPlan>(result);
}

void ExpectRefusal(const PlanResult& result, Refusal expected) {
  const auto* refusal = std::get_if<Refusal>(&result);
  Expect(refusal && *refusal == expected, "unexpected planning result");
}

void ConcreteNativeOperations() {
  Expect(std::holds_alternative<OpenFindBar>(Plan(Command::kOpen).operation()),
         "opening must not request a match jump");
  Expect(std::get<AdvanceMatch>(Plan(Command::kNext).operation()).direction ==
             Direction::kForward,
         "next must advance forward");
  Expect(
      std::get<AdvanceMatch>(Plan(Command::kPrevious).operation()).direction ==
          Direction::kBackward,
      "previous must advance backward");
  Expect(std::holds_alternative<EndSessionKeepingSelectionAndResults>(
             Plan(Command::kClose).operation()),
         "close must keep page selection and native results");
  auto inactive = BoundController();
  inactive.find_ui_active = false;
  ExpectRefusal(
      PlanFindCommand(kPage, kProfile, Command::kClose, kEligible, inactive),
      Refusal::kNoActiveSession);
  Expect(std::holds_alternative<FindPlan>(PlanFindCommand(
             kPage, kProfile, Command::kOpen, kEligible, inactive)),
         "inactive native find must be openable");
}

void BindingRefusals() {
  ExpectRefusal(
      PlanFindCommand(kPage, kProfile, Command::kOpen, kEligible, std::nullopt),
      Refusal::kControllerUnavailable);
  auto other = BoundController();
  other.bound_page = PageId{"other-split-page"};
  ExpectRefusal(
      PlanFindCommand(kPage, kProfile, Command::kOpen, kEligible, other),
      Refusal::kPageMismatch);
  other = BoundController();
  other.bound_profile = ProfileId{"other-profile"};
  ExpectRefusal(
      PlanFindCommand(kPage, kProfile, Command::kNext, kEligible, other),
      Refusal::kProfileMismatch);
  other = BoundController();
  other.generation.value.clear();
  ExpectRefusal(
      PlanFindCommand(kPage, kProfile, Command::kOpen, kEligible, other),
      Refusal::kMissingGeneration);
}

void NativeInputOwnsEligibility() {
  for (auto command :
       {Command::kOpen, Command::kNext, Command::kPrevious, Command::kClose}) {
    ExpectRefusal(
        PlanFindCommand(kPage, kProfile, command, kDefer, BoundController()),
        Refusal::kDeferToNativeInput);
    auto plan = Plan(command);
    Expect(
        RevalidateFindPlan(plan, kPage, kProfile, kDefer, BoundController()) ==
            Refusal::kDeferToNativeInput,
        "new native input ownership must block an existing plan");
    Expect(!RevalidateFindPlan(plan, kPage, kProfile, kEligible,
                               BoundController()),
           "caller may retry with fresh positive eligibility");
  }
}

void RevalidateAtEffect() {
  auto plan = Plan(Command::kNext);
  Expect(
      !RevalidateFindPlan(plan, kPage, kProfile, kEligible, BoundController()),
      "unchanged binding must remain usable");
  Expect(RevalidateFindPlan(plan, kPage, kProfile, kEligible, std::nullopt) ==
             Refusal::kControllerUnavailable,
         "closing page must invalidate effect");
  auto current = BoundController();
  current.bound_page = PageId{"new-focus"};
  Expect(RevalidateFindPlan(plan, current.bound_page, kProfile, kEligible,
                            current) == Refusal::kPageMismatch,
         "new focus must not inherit old find command");
  current = BoundController();
  current.bound_profile = ProfileId{"new-profile"};
  Expect(RevalidateFindPlan(plan, kPage, current.bound_profile, kEligible,
                            current) == Refusal::kProfileMismatch,
         "Profile switch must not inherit old find command");
  current = BoundController();
  current.generation.value = "after-away-and-back";
  Expect(RevalidateFindPlan(plan, kPage, kProfile, kEligible, current) ==
             Refusal::kStaleGeneration,
         "focus ABA, navigation or WebContents replacement must invalidate");
  Expect(plan.generation().value == "caller-epoch",
         "plan must own its captured token");
  auto close = Plan(Command::kClose);
  current = BoundController();
  current.find_ui_active = false;
  Expect(RevalidateFindPlan(close, kPage, kProfile, kEligible, current) ==
             Refusal::kNoActiveSession,
         "late close must not restore focus for an inactive native session");
}

}  // namespace

int main() {
  static_assert(!std::is_default_constructible_v<FindPlan>);
  ConcreteNativeOperations();
  BindingRefusals();
  NativeInputOwnsEligibility();
  RevalidateAtEffect();
  std::cout << "local_find: command and binding checks passed\n";
}
