#ifndef ARCIFIED_NATIVE_FEATURES_PRIVATE_BROWSING_OPERATION_BOUNDARY_H_
#define ARCIFIED_NATIVE_FEATURES_PRIVATE_BROWSING_OPERATION_BOUNDARY_H_

#include <vector>

#include "context_boundary.h"

namespace arcified::private_browsing {

enum class Operation {
  // In-memory presentation references only, within one exact native context.
  // Does not authorize Keep, a normal Space edit, transfer, or navigation.
  kTransientOrganization,
  kWorkspacePersistence,
  kArchive,
  kCapturePersistence,
  kBrowsingHistory,
  kAIHistory,
  kIntegrationCache,
  kDiagnostics,
  kWorkspaceExport,
  kOptionalRemote,
};

enum class Decision {
  kEligible,
  kRequiresRemoteAuthorization,
  kPersistentUserCopy,
  kNoSources,
  kUnknownContext,
  kPrivateContext,
  kGuestContext,
  kUnsupportedContext,
  kContextMismatch,
  kInconsistentFacts,
  kExplicitUserActionRequired,
  kUnknownOperation,
};

// Pure privacy preflight. kEligible is not native success or authorization.
// Regular remote requests always require a separate per-request permission.
Decision CheckOperation(const NativeContextFacts& source, Operation operation);

// All-or-nothing preflight for the complete provenance of a payload, including
// derived captures/caches/history. Never silently filters private sources out.
// Call before collecting/serializing/logging payload, then revalidate at use.
Decision CheckBatch(const std::vector<NativeContextFacts>& sources,
                    Operation operation);

enum class UserFileAction {
  kDownload,
  kSaveFile,
  kCopyToClipboard,
  kExportSelectedContent,
};

enum class UserFileIntent {
  kUnspecified,
  // Actual deliberate command for this exact content/context. Native caller
  // explains that the file/clipboard copy may outlive private browsing.
  kExplicitWithPersistenceNotice,
};

struct UserFileRequest {
  NativeContextId context;
  UserFileAction action;
  UserFileIntent intent = UserFileIntent::kUnspecified;
};

// Separate exception for selected user output, never a flag on persistence or
// remote requests. Does not authorize a public share, ordinary workspace
// export, browser history/cache entry or automatic Library/Recent-files
// registration. Native file/URL/clipboard/security checks and cancellation
// still apply.
Decision CheckUserFile(const NativeContextFacts& source,
                       const UserFileRequest& request);

}  // namespace arcified::private_browsing

#endif  // ARCIFIED_NATIVE_FEATURES_PRIVATE_BROWSING_OPERATION_BOUNDARY_H_
