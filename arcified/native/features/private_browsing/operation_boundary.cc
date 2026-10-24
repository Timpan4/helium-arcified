#include "operation_boundary.h"

#include <map>

namespace arcified::private_browsing {
namespace {

Decision ContextRefusal(ContextClass context) {
  switch (context) {
    case ContextClass::kUnknown:
      return Decision::kUnknownContext;
    case ContextClass::kRegular:
      return Decision::kEligible;
    case ContextClass::kOffTheRecord:
      return Decision::kPrivateContext;
    case ContextClass::kGuest:
      return Decision::kGuestContext;
    case ContextClass::kUnsupported:
      return Decision::kUnsupportedContext;
  }
  return Decision::kUnknownContext;
}

}  // namespace

Decision CheckOperation(const NativeContextFacts& source, Operation operation) {
  const auto context = ClassifyContext(source);
  switch (operation) {
    case Operation::kTransientOrganization:
      if (context == ContextClass::kRegular ||
          context == ContextClass::kOffTheRecord) {
        return Decision::kEligible;
      }
      return ContextRefusal(context);
    case Operation::kOptionalRemote:
      return context == ContextClass::kRegular
                 ? Decision::kRequiresRemoteAuthorization
                 : ContextRefusal(context);
    case Operation::kWorkspacePersistence:
    case Operation::kArchive:
    case Operation::kCapturePersistence:
    case Operation::kBrowsingHistory:
    case Operation::kAIHistory:
    case Operation::kIntegrationCache:
    case Operation::kDiagnostics:
    case Operation::kWorkspaceExport:
      return ContextRefusal(context);
  }
  return Decision::kUnknownOperation;
}

Decision CheckBatch(const std::vector<NativeContextFacts>& sources,
                    Operation operation) {
  if (sources.empty())
    return Decision::kNoSources;

  // A repeated native identity with different facts signals stale or invalid
  // input. Do not choose the most permissive report or collapse to the parent.
  std::map<NativeContextId, const NativeContextFacts*> seen;
  for (const auto& source : sources) {
    if (ClassifyContext(source) == ContextClass::kUnknown)
      return Decision::kUnknownContext;
    const auto [it, inserted] = seen.emplace(*source.context, &source);
    if (!inserted &&
        (it->second->kind != source.kind ||
         it->second->original_profile != source.original_profile)) {
      return Decision::kInconsistentFacts;
    }
  }

  // Only a fully eligible batch can return the remote-authorization
  // requirement. A later private source cannot be hidden by an earlier regular
  // source.
  Decision result = Decision::kEligible;
  for (const auto& source : sources) {
    const auto decision = CheckOperation(source, operation);
    switch (decision) {
      case Decision::kEligible:
        break;
      case Decision::kRequiresRemoteAuthorization:
        result = decision;
        break;
      default:
        return decision;
    }
    if (operation == Operation::kTransientOrganization &&
        source.context != sources.front().context) {
      return Decision::kContextMismatch;
    }
  }
  return result;
}

Decision CheckUserFile(const NativeContextFacts& source,
                       const UserFileRequest& request) {
  const auto context = ClassifyContext(source);
  if (context != ContextClass::kRegular &&
      context != ContextClass::kOffTheRecord) {
    return ContextRefusal(context);
  }
  if (request.context.value.empty() || source.context != request.context)
    return Decision::kContextMismatch;
  switch (request.action) {
    case UserFileAction::kDownload:
    case UserFileAction::kSaveFile:
    case UserFileAction::kCopyToClipboard:
    case UserFileAction::kExportSelectedContent:
      break;
    default:
      return Decision::kUnknownOperation;
  }
  switch (request.intent) {
    case UserFileIntent::kExplicitWithPersistenceNotice:
      return Decision::kPersistentUserCopy;
    case UserFileIntent::kUnspecified:
      return Decision::kExplicitUserActionRequired;
  }
  return Decision::kExplicitUserActionRequired;
}

}  // namespace arcified::private_browsing
