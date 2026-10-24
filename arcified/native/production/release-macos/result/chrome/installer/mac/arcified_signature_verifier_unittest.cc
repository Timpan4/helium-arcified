// Copyright 2026 The Arcified Authors
// SPDX-License-Identifier: GPL-3.0-only

#include "chrome/installer/mac/arcified_signature_verifier.h"

#include <string>

#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace arcified::release_macos {
namespace {

constexpr char kBundleId[] = "test.arcified.signature";

class ArcifiedSignatureVerifierTest : public testing::Test {
 protected:
  void SetUp() override {
    ASSERT_TRUE(directory_.CreateUniqueTempDir());
    bundle_ = directory_.GetPath().Append("Unsigned.app");
    ASSERT_TRUE(base::CreateDirectory(bundle_.Append("Contents/MacOS")));
    ASSERT_TRUE(base::WriteFile(
        bundle_.Append("Contents/Info.plist"),
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<plist version=\"1.0\"><dict>"
        "<key>CFBundleIdentifier</key><string>test.arcified.signature</string>"
        "<key>CFBundlePackageType</key><string>APPL</string>"
        "<key>CFBundleExecutable</key><string>unsigned</string>"
        "</dict></plist>"));
    ASSERT_TRUE(base::WriteFile(bundle_.Append("Contents/MacOS/unsigned"),
                                "#!/bin/sh\nexit 0\n"));
    ASSERT_TRUE(base::SetPosixFilePermissions(
        bundle_.Append("Contents/MacOS/unsigned"), 0755));
  }

  base::ScopedTempDir directory_;
  base::FilePath bundle_;
};

TEST_F(ArcifiedSignatureVerifierTest, RejectsMissingPolicyAndRelativePaths) {
  for (const auto& result :
       {VerifyAppSignature(bundle_, "", "true"),
        VerifyAppSignature(bundle_, kBundleId, " \n"),
        VerifyAppSignature(base::FilePath("Unsigned.app"), kBundleId, "true"),
        VerifyAppSignature(bundle_, "test\" or true", "true"),
        VerifyAppSignature(bundle_, kBundleId, std::string("\xff", 1)),
        VerifyAppSignature(bundle_, kBundleId, std::string("true\0", 5))}) {
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(SignatureFailure::kInvalidInput, result.error().failure);
  }
}

TEST_F(ArcifiedSignatureVerifierTest, RejectsMalformedNativeRequirement) {
  const auto result = VerifyAppSignature(bundle_, kBundleId, "(");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(SignatureFailure::kInvalidRequirement, result.error().failure);
  EXPECT_NE(errSecSuccess, result.error().status);
}

TEST_F(ArcifiedSignatureVerifierTest, RejectsNonApplicationDirectory) {
  const auto result =
      VerifyAppSignature(directory_.GetPath(), kBundleId, "true");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(SignatureFailure::kNotApplicationBundle, result.error().failure);
}

TEST_F(ArcifiedSignatureVerifierTest, RejectsUnexpectedBundleIdentifier) {
  const auto result =
      VerifyAppSignature(bundle_, "test.arcified.other", "true");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(SignatureFailure::kBundleIdentifierMismatch,
            result.error().failure);
}

TEST_F(ArcifiedSignatureVerifierTest, UnsignedAppCannotPassPermissivePolicy) {
  const auto result = VerifyAppSignature(bundle_, kBundleId, "true");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(SignatureFailure::kSignatureRejected, result.error().failure);
  EXPECT_EQ(errSecCSUnsigned, result.error().status);
  std::string executable;
  ASSERT_TRUE(base::ReadFileToString(bundle_.Append("Contents/MacOS/unsigned"),
                                     &executable));
  EXPECT_EQ("#!/bin/sh\nexit 0\n", executable);
}

}  // namespace
}  // namespace arcified::release_macos
