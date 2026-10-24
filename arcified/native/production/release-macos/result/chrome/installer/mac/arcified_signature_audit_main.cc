// Copyright 2026 The Arcified Authors
// SPDX-License-Identifier: GPL-3.0-only

#include <cstdio>
#include <string>

#include "base/command_line.h"
#include "base/files/file_util.h"
#include "chrome/installer/mac/arcified_signature_verifier.h"

namespace {

const char* FailureName(arcified::release_macos::SignatureFailure failure) {
  using arcified::release_macos::SignatureFailure;
  switch (failure) {
    case SignatureFailure::kInvalidInput:
      return "invalid-input";
    case SignatureFailure::kInvalidRequirement:
      return "invalid-requirement";
    case SignatureFailure::kNotApplicationBundle:
      return "not-application-bundle";
    case SignatureFailure::kBundleIdentifierMismatch:
      return "bundle-identifier-mismatch";
    case SignatureFailure::kCannotReadCode:
      return "cannot-read-code";
    case SignatureFailure::kSignatureRejected:
      return "signature-rejected";
  }
}

}  // namespace

int main(int argc, char** argv) {
  base::CommandLine::Init(argc, argv);
  const auto& command = *base::CommandLine::ForCurrentProcess();
  const auto requirement_file =
      command.GetSwitchValuePath("signing-requirement-file");
  std::string requirement;
  if (!command.GetArgs().empty() || !requirement_file.IsAbsolute() ||
      !base::ReadFileToString(requirement_file, &requirement)) {
    std::fputs(
        "Usage: arcified_signature_audit --app-bundle=/absolute/app.app "
        "--bundle-id=APPROVED_ID "
        "--signing-requirement-file=/absolute/approved-requirement.txt\n",
        stderr);
    return 1;
  }
  const auto result = arcified::release_macos::VerifyAppSignature(
      command.GetSwitchValuePath("app-bundle"),
      command.GetSwitchValueASCII("bundle-id"), requirement);
  if (!result.has_value()) {
    std::fprintf(stderr, "Signature check failed: %s, OSStatus=%d\n",
                 FailureName(result.error().failure),
                 static_cast<int>(result.error().status));
    return 1;
  }
  std::puts(
      "Local static signature requirement satisfied. "
      "Release/install acceptance remains OPEN. "
      "Notarization and current revocation were not checked.");
  return 0;
}
