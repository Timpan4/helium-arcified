// Copyright 2026 The Arcified Authors
// SPDX-License-Identifier: GPL-3.0-only

#include <string>

#include "base/command_line.h"
#include "base/files/file_util.h"
#include "chrome/installer/mac/arcified_signature_verifier.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace arcified::release_macos {
namespace {

// Separate opt-in target. No fixture is signed, installed or invented here.
// Absence of the owner's immutable signed app/policy fails this target.
TEST(ArcifiedSignatureArtifactTest, ApprovedAppMatchesOnlyApprovedRequirement) {
  const auto& command = *base::CommandLine::ForCurrentProcess();
  const auto bundle = command.GetSwitchValuePath("app-bundle");
  const auto requirement_file =
      command.GetSwitchValuePath("signing-requirement-file");
  const std::string identifier = command.GetSwitchValueASCII("bundle-id");
  ASSERT_TRUE(bundle.IsAbsolute());
  ASSERT_TRUE(requirement_file.IsAbsolute());
  ASSERT_FALSE(identifier.empty());
  std::string requirement;
  ASSERT_TRUE(base::ReadFileToString(requirement_file, &requirement));
  const auto accepted = VerifyAppSignature(bundle, identifier, requirement);
  ASSERT_TRUE(accepted.has_value()) << "OSStatus=" << accepted.error().status;

  const auto rejected = VerifyAppSignature(bundle, identifier, "false");
  ASSERT_FALSE(rejected.has_value());
  EXPECT_EQ(SignatureFailure::kSignatureRejected, rejected.error().failure);
  EXPECT_EQ(errSecCSReqFailed, rejected.error().status);
}

}  // namespace
}  // namespace arcified::release_macos
