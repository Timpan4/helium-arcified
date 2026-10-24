// Copyright 2026 The Arcified Authors
// SPDX-License-Identifier: GPL-3.0-only

#include "chrome/installer/mac/arcified_signature_verifier.h"

#include <CoreFoundation/CoreFoundation.h>

#include "base/apple/foundation_util.h"
#include "base/apple/scoped_cftyperef.h"
#include "base/files/file_util.h"
#include "base/strings/strcat.h"
#include "base/strings/string_util.h"
#include "base/strings/sys_string_conversions.h"

namespace arcified::release_macos {

base::expected<void, SignatureError> VerifyAppSignature(
    const base::FilePath& app_bundle,
    std::string_view expected_bundle_identifier,
    std::string_view approved_signing_requirement) {
  // Apple's bundle identifier alphabet also prevents requirement injection.
  if (!app_bundle.IsAbsolute() || app_bundle.ReferencesParent() ||
      app_bundle.value().find('\0') != std::string::npos ||
      expected_bundle_identifier.empty() ||
      !base::ContainsOnlyChars(expected_bundle_identifier,
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz0123456789.-") ||
      base::TrimWhitespaceASCII(approved_signing_requirement, base::TRIM_ALL)
          .empty() ||
      !base::IsStringUTF8(approved_signing_requirement) ||
      approved_signing_requirement.find('\0') != std::string_view::npos) {
    return base::unexpected(
        SignatureError{SignatureFailure::kInvalidInput, errSecParam});
  }

  // 'anchor trusted' prevents ad hoc signatures, including non-host slices,
  // from satisfying even a permissive supplied requirement. Parentheses keep
  // an owner's 'or' clauses from bypassing the identifier/trust constraints.
  const std::string requirement_text = base::StrCat(
      {"(", approved_signing_requirement,
       ") and anchor trusted and identifier \"", expected_bundle_identifier,
       "\" and info[CFBundleIdentifier] = \"", expected_bundle_identifier,
       "\""});
  auto requirement_string = base::SysUTF8ToCFStringRef(requirement_text);
  base::apple::ScopedCFTypeRef<SecRequirementRef> requirement;
  OSStatus status = SecRequirementCreateWithString(
      requirement_string.get(), kSecCSDefaultFlags,
      requirement.InitializeInto());
  if (status != errSecSuccess) {
    return base::unexpected(
        SignatureError{SignatureFailure::kInvalidRequirement, status});
  }

  auto bundle_url = base::apple::FilePathToCFURL(app_bundle);
  base::apple::ScopedCFTypeRef<CFBundleRef> bundle(
      CFBundleCreate(kCFAllocatorDefault, bundle_url.get()));
  CFStringRef package_type =
      bundle ? base::apple::CFCast<CFStringRef>(
                   CFBundleGetValueForInfoDictionaryKey(
                       bundle.get(), CFSTR("CFBundlePackageType")))
             : nullptr;
  if (!base::DirectoryExists(app_bundle) || !package_type ||
      !CFEqual(package_type, CFSTR("APPL"))) {
    return base::unexpected(
        SignatureError{SignatureFailure::kNotApplicationBundle, errSecParam});
  }
  CFStringRef identifier =
      base::apple::CFCast<CFStringRef>(CFBundleGetValueForInfoDictionaryKey(
          bundle.get(), kCFBundleIdentifierKey));
  if (!identifier ||
      base::SysCFStringRefToUTF8(identifier) != expected_bundle_identifier) {
    return base::unexpected(SignatureError{
        SignatureFailure::kBundleIdentifierMismatch, errSecCSReqFailed});
  }

  base::apple::ScopedCFTypeRef<SecStaticCodeRef> code;
  status = SecStaticCodeCreateWithPath(bundle_url.get(), kSecCSDefaultFlags,
                                       code.InitializeInto());
  if (status != errSecSuccess) {
    return base::unexpected(
        SignatureError{SignatureFailure::kCannotReadCode, status});
  }
  status = SecStaticCodeCheckValidity(
      code.get(),
      kSecCSCheckAllArchitectures | kSecCSCheckNestedCode |
          kSecCSStrictValidate | kSecCSNoNetworkAccess,
      requirement.get());
  if (status != errSecSuccess) {
    return base::unexpected(
        SignatureError{SignatureFailure::kSignatureRejected, status});
  }
  return base::ok();
}

}  // namespace arcified::release_macos
