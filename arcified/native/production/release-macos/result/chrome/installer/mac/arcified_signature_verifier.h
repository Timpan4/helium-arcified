// Copyright 2026 The Arcified Authors
// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHROME_INSTALLER_MAC_ARCIFIED_SIGNATURE_VERIFIER_H_
#define CHROME_INSTALLER_MAC_ARCIFIED_SIGNATURE_VERIFIER_H_

#include <Security/Security.h>

#include <string_view>

#include "base/files/file_path.h"
#include "base/types/expected.h"

namespace arcified::release_macos {

enum class SignatureFailure {
  kInvalidInput,
  kInvalidRequirement,
  kNotApplicationBundle,
  kBundleIdentifierMismatch,
  kCannotReadCode,
  kSignatureRejected,
};

struct SignatureError {
  SignatureFailure failure;
  OSStatus status;
};

// Read-only, synchronous signature check for a quiescent app bundle. Run
// outside the browser UI thread. The caller must prevent concurrent
// modification and recheck after installation; the result does not authorize
// later changed bytes.
//
// Both policy arguments are mandatory, owner-approved inputs, never values read
// from the artifact itself. The signing requirement must constrain the approved
// signer/certificate and distribution class. This helper additionally binds the
// signing and Info.plist identifiers and requires a trusted certificate chain
// on every architecture. It does not approve any signer or distribution class.
//
// No network access is requested. Success proves only local static signature
// validation against that requirement, including nested code and resources.
// Notarization, fresh revocation, Gatekeeper, installer/package trust,
// supported architecture/OS, registration and retained user data need separate
// evidence.
base::expected<void, SignatureError> VerifyAppSignature(
    const base::FilePath& app_bundle,
    std::string_view expected_bundle_identifier,
    std::string_view approved_signing_requirement);

}  // namespace arcified::release_macos

#endif  // CHROME_INSTALLER_MAC_ARCIFIED_SIGNATURE_VERIFIER_H_
