#ifndef ARCIFIED_NATIVE_FEATURES_PRIVATE_BROWSING_CONTEXT_BOUNDARY_H_
#define ARCIFIED_NATIVE_FEATURES_PRIVATE_BROWSING_CONTEXT_BOUNDARY_H_

#include <optional>

#include "../../model/lifecycle.h"

namespace arcified::private_browsing {

// Caller-owned live registry identity for the exact native Profile object.
// Never a Profile path, original Profile, OTRProfileID alone, or durable ID.
// The caller invalidates it on destruction and never reuses it.
using NativeContextId = lifecycle::Id<struct NativeContextTag>;

// Captured by the native caller, not inferred from a lifecycle ProfileId.
// Guest and system have both original and OTR objects in Chromium.
enum class NativeContextKind {
  kUnknown,
  kRegular,
  kOffTheRecord,
  kGuest,
  kGuestOffTheRecord,
  kSystem,
  kSystemOffTheRecord,
  kOther,
};

struct NativeContextFacts {
  NativeContextKind kind = NativeContextKind::kUnknown;
  std::optional<NativeContextId> context;
  // Mapping metadata only. For regular contexts this identifies that Profile;
  // for OTR it may identify the original. It never establishes context
  // equality.
  std::optional<lifecycle::ProfileId> original_profile;
};

enum class ContextClass {
  kUnknown,
  kRegular,
  kOffTheRecord,
  kGuest,
  kUnsupported,
};

// Header-only so consumers need no operation-policy linkage. This establishes
// neither model membership, native liveness, target binding nor authorization.
inline ContextClass ClassifyContext(const NativeContextFacts& facts) {
  if (!facts.context || facts.context->value.empty())
    return ContextClass::kUnknown;
  switch (facts.kind) {
    case NativeContextKind::kUnknown:
      return ContextClass::kUnknown;
    case NativeContextKind::kRegular:
      return facts.original_profile && !facts.original_profile->value.empty()
                 ? ContextClass::kRegular
                 : ContextClass::kUnknown;
    case NativeContextKind::kOffTheRecord:
      return ContextClass::kOffTheRecord;
    case NativeContextKind::kGuest:
    case NativeContextKind::kGuestOffTheRecord:
      return ContextClass::kGuest;
    case NativeContextKind::kSystem:
    case NativeContextKind::kSystemOffTheRecord:
    case NativeContextKind::kOther:
      return ContextClass::kUnsupported;
  }
  return ContextClass::kUnknown;
}

}  // namespace arcified::private_browsing

#endif  // ARCIFIED_NATIVE_FEATURES_PRIVATE_BROWSING_CONTEXT_BOUNDARY_H_
