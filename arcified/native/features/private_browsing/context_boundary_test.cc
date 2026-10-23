#include "context_boundary.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace arcified::private_browsing {
namespace {

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

static_assert(!std::is_convertible_v<lifecycle::ProfileId, NativeContextId>);

void Classification() {
  NativeContextFacts facts;
  REQUIRE(ClassifyContext(facts) == ContextClass::kUnknown);
  facts.kind = NativeContextKind::kRegular;
  facts.original_profile = lifecycle::ProfileId{"work"};
  REQUIRE(ClassifyContext(facts) == ContextClass::kUnknown);
  facts.context = NativeContextId{""};
  REQUIRE(ClassifyContext(facts) == ContextClass::kUnknown);
  facts.context = NativeContextId{"work-live-instance"};
  REQUIRE(ClassifyContext(facts) == ContextClass::kRegular);
  facts.original_profile.reset();
  REQUIRE(ClassifyContext(facts) == ContextClass::kUnknown);
  facts.original_profile = lifecycle::ProfileId{""};
  REQUIRE(ClassifyContext(facts) == ContextClass::kUnknown);

  // Knowing the original does not make its OTR context regular. OTR remains
  // private even when no lifecycle mapping for its original is available.
  facts.kind = NativeContextKind::kOffTheRecord;
  facts.original_profile = lifecycle::ProfileId{"work"};
  REQUIRE(ClassifyContext(facts) == ContextClass::kOffTheRecord);
  facts.original_profile.reset();
  REQUIRE(ClassifyContext(facts) == ContextClass::kOffTheRecord);

  for (auto kind :
       {NativeContextKind::kGuest, NativeContextKind::kGuestOffTheRecord}) {
    facts.kind = kind;
    REQUIRE(ClassifyContext(facts) == ContextClass::kGuest);
  }
  for (auto kind :
       {NativeContextKind::kSystem, NativeContextKind::kSystemOffTheRecord,
        NativeContextKind::kOther}) {
    facts.kind = kind;
    REQUIRE(ClassifyContext(facts) == ContextClass::kUnsupported);
  }
  facts.kind = static_cast<NativeContextKind>(-1);
  REQUIRE(ClassifyContext(facts) == ContextClass::kUnknown);
  facts.kind = NativeContextKind::kUnknown;
  REQUIRE(ClassifyContext(facts) == ContextClass::kUnknown);
}

void ExactOTRIdentity() {
  const lifecycle::ProfileId original{"work"};
  const NativeContextFacts first{NativeContextKind::kOffTheRecord,
                                 NativeContextId{"work-otr-one"}, original};
  const NativeContextFacts sibling{NativeContextKind::kOffTheRecord,
                                   NativeContextId{"work-otr-two"}, original};
  REQUIRE(first.original_profile == sibling.original_profile);
  REQUIRE(first.context != sibling.context);
  REQUIRE(ClassifyContext(first) == ContextClass::kOffTheRecord);
  REQUIRE(ClassifyContext(sibling) == ContextClass::kOffTheRecord);
}

}  // namespace
}  // namespace arcified::private_browsing

int main() {
  arcified::private_browsing::Classification();
  arcified::private_browsing::ExactOTRIdentity();
  std::cout << "private context classification fixtures passed\n";
}
