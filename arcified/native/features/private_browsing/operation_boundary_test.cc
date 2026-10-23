#include "operation_boundary.h"

#include <cstdlib>
#include <iostream>
#include <vector>

namespace arcified::private_browsing {
namespace {

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const lifecycle::ProfileId kOriginal{"work"};
const NativeContextFacts kRegular{NativeContextKind::kRegular,
                                  NativeContextId{"regular-live"}, kOriginal};
const NativeContextFacts kPrivate{NativeContextKind::kOffTheRecord,
                                  NativeContextId{"private-live"}, kOriginal};
const NativeContextFacts kSibling{NativeContextKind::kOffTheRecord,
                                  NativeContextId{"sibling-live"}, kOriginal};

const std::vector<Operation> kOrdinarySinks{Operation::kWorkspacePersistence,
                                            Operation::kArchive,
                                            Operation::kCapturePersistence,
                                            Operation::kBrowsingHistory,
                                            Operation::kAIHistory,
                                            Operation::kIntegrationCache,
                                            Operation::kDiagnostics,
                                            Operation::kWorkspaceExport};

void PersistenceAndRemote() {
  for (auto operation : kOrdinarySinks) {
    REQUIRE(CheckOperation(kRegular, operation) == Decision::kEligible);
    REQUIRE(CheckOperation(kPrivate, operation) == Decision::kPrivateContext);
    REQUIRE(CheckOperation(kSibling, operation) == Decision::kPrivateContext);
    REQUIRE(CheckOperation({}, operation) == Decision::kUnknownContext);
  }
  // No user intent field exists on this API, so a download/save gesture cannot
  // accidentally opt private data into an ordinary sink or remote provider.
  REQUIRE(CheckOperation(kRegular, Operation::kOptionalRemote) ==
          Decision::kRequiresRemoteAuthorization);
  REQUIRE(CheckOperation(kPrivate, Operation::kOptionalRemote) ==
          Decision::kPrivateContext);
  REQUIRE(CheckOperation({}, Operation::kOptionalRemote) ==
          Decision::kUnknownContext);

  for (auto kind :
       {NativeContextKind::kGuest, NativeContextKind::kGuestOffTheRecord}) {
    const NativeContextFacts guest{kind, NativeContextId{"guest"}, kOriginal};
    for (auto operation : kOrdinarySinks)
      REQUIRE(CheckOperation(guest, operation) == Decision::kGuestContext);
    REQUIRE(CheckOperation(guest, Operation::kOptionalRemote) ==
            Decision::kGuestContext);
    REQUIRE(CheckOperation(guest, Operation::kTransientOrganization) ==
            Decision::kGuestContext);
  }
  for (auto kind :
       {NativeContextKind::kSystem, NativeContextKind::kSystemOffTheRecord,
        NativeContextKind::kOther}) {
    const NativeContextFacts special{kind, NativeContextId{"special"},
                                     kOriginal};
    for (auto operation : kOrdinarySinks)
      REQUIRE(CheckOperation(special, operation) ==
              Decision::kUnsupportedContext);
    REQUIRE(CheckOperation(special, Operation::kOptionalRemote) ==
            Decision::kUnsupportedContext);
    REQUIRE(CheckOperation(special, Operation::kTransientOrganization) ==
            Decision::kUnsupportedContext);
  }
  REQUIRE(CheckOperation(kRegular, static_cast<Operation>(-1)) ==
          Decision::kUnknownOperation);
}

void CompleteBatchProvenance() {
  const NativeContextFacts personal{NativeContextKind::kRegular,
                                    NativeContextId{"personal-live"},
                                    lifecycle::ProfileId{"personal"}};
  REQUIRE(CheckBatch({}, Operation::kWorkspaceExport) == Decision::kNoSources);
  for (auto operation : kOrdinarySinks) {
    REQUIRE(CheckBatch({kRegular, personal}, operation) == Decision::kEligible);
    REQUIRE(CheckBatch({kRegular, kPrivate}, operation) ==
            Decision::kPrivateContext);
    REQUIRE(CheckBatch({kPrivate, kRegular}, operation) ==
            Decision::kPrivateContext);
    REQUIRE(CheckBatch({kRegular, {}}, operation) == Decision::kUnknownContext);
  }
  REQUIRE(CheckBatch({kRegular, personal}, Operation::kOptionalRemote) ==
          Decision::kRequiresRemoteAuthorization);
  REQUIRE(CheckBatch({kRegular, kPrivate}, Operation::kOptionalRemote) ==
          Decision::kPrivateContext);
  REQUIRE(CheckBatch({kPrivate, kRegular}, Operation::kOptionalRemote) ==
          Decision::kPrivateContext);
  REQUIRE(CheckBatch({kRegular}, static_cast<Operation>(-1)) ==
          Decision::kUnknownOperation);

  auto contradictory = kPrivate;
  contradictory.context = kRegular.context;
  REQUIRE(CheckBatch({kRegular, contradictory}, Operation::kWorkspaceExport) ==
          Decision::kInconsistentFacts);
  contradictory = kRegular;
  contradictory.original_profile = personal.original_profile;
  REQUIRE(CheckBatch({kRegular, contradictory}, Operation::kWorkspaceExport) ==
          Decision::kInconsistentFacts);
  REQUIRE(CheckBatch({kRegular, kRegular}, Operation::kWorkspaceExport) ==
          Decision::kEligible);
}

void OrganizationStaysInExactContext() {
  REQUIRE(CheckOperation(kPrivate, Operation::kTransientOrganization) ==
          Decision::kEligible);
  REQUIRE(CheckBatch({kPrivate, kPrivate}, Operation::kTransientOrganization) ==
          Decision::kEligible);
  REQUIRE(CheckBatch({kPrivate, kSibling}, Operation::kTransientOrganization) ==
          Decision::kContextMismatch);
  REQUIRE(CheckBatch({kPrivate, kRegular}, Operation::kTransientOrganization) ==
          Decision::kContextMismatch);
  REQUIRE(CheckBatch({kRegular, {}}, Operation::kTransientOrganization) ==
          Decision::kUnknownContext);
  // The same private page cannot be silently kept by changing only the action.
  REQUIRE(CheckOperation(kPrivate, Operation::kWorkspacePersistence) ==
          Decision::kPrivateContext);
}

void DeliberateUserCopies() {
  for (auto action : {UserFileAction::kDownload, UserFileAction::kSaveFile,
                      UserFileAction::kCopyToClipboard,
                      UserFileAction::kExportSelectedContent}) {
    UserFileRequest request{*kPrivate.context, action};
    REQUIRE(CheckUserFile(kPrivate, request) ==
            Decision::kExplicitUserActionRequired);
    request.intent = UserFileIntent::kExplicitWithPersistenceNotice;
    REQUIRE(CheckUserFile(kPrivate, request) == Decision::kPersistentUserCopy);
    REQUIRE(CheckUserFile(kSibling, request) == Decision::kContextMismatch);
    REQUIRE(CheckUserFile({}, request) == Decision::kUnknownContext);
    request.context = *kRegular.context;
    REQUIRE(CheckUserFile(kRegular, request) == Decision::kPersistentUserCopy);
    request.context = NativeContextId{""};
    REQUIRE(CheckUserFile(kRegular, request) == Decision::kContextMismatch);
  }

  UserFileRequest request{*kPrivate.context, UserFileAction::kSaveFile,
                          UserFileIntent::kExplicitWithPersistenceNotice};
  request.action = static_cast<UserFileAction>(-1);
  REQUIRE(CheckUserFile(kPrivate, request) == Decision::kUnknownOperation);
  request.action = UserFileAction::kSaveFile;
  request.intent = static_cast<UserFileIntent>(-1);
  REQUIRE(CheckUserFile(kPrivate, request) ==
          Decision::kExplicitUserActionRequired);

  for (auto kind :
       {NativeContextKind::kGuest, NativeContextKind::kGuestOffTheRecord}) {
    const NativeContextFacts guest{kind, request.context, kOriginal};
    REQUIRE(CheckUserFile(guest, request) == Decision::kGuestContext);
  }
  for (auto kind :
       {NativeContextKind::kSystem, NativeContextKind::kSystemOffTheRecord,
        NativeContextKind::kOther}) {
    const NativeContextFacts special{kind, request.context, kOriginal};
    REQUIRE(CheckUserFile(special, request) == Decision::kUnsupportedContext);
  }
}

}  // namespace
}  // namespace arcified::private_browsing

int main() {
  arcified::private_browsing::PersistenceAndRemote();
  arcified::private_browsing::CompleteBatchProvenance();
  arcified::private_browsing::OrganizationStaysInExactContext();
  arcified::private_browsing::DeliberateUserCopies();
  std::cout << "private operation boundary fixtures passed\n";
}
