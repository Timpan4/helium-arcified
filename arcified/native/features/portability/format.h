#ifndef ARCIFIED_NATIVE_FEATURES_PORTABILITY_FORMAT_H_
#define ARCIFIED_NATIVE_FEATURES_PORTABILITY_FORMAT_H_

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::portability {

inline constexpr std::uint32_t kFormatVersion = 1;

// Source labels are provenance only, never native IDs or Profile authority.
struct Provenance {
  std::string source;
  std::string source_id;
  std::optional<std::uint64_t> ordinal;
};

struct Profile {
  lifecycle::ProfileId id;
  std::optional<std::string> name;
  std::optional<Provenance> provenance;
};
struct Space {
  lifecycle::SpaceId id;
  lifecycle::ProfileId profile;
  std::optional<std::string> name;
  // Opaque source metadata, not a parsed timestamp or lifecycle creation time.
  std::optional<std::string> saved_at;
  std::optional<Provenance> provenance;
};
struct Item {
  lifecycle::ItemId id;
  lifecycle::ItemOwner owner;
  std::string saved_destination;
  std::optional<std::string> name;
  std::optional<Provenance> provenance;
};
struct Shortcut {
  lifecycle::ShortcutId id;
  lifecycle::ProfileId profile;
  lifecycle::ItemId target;
  std::optional<Provenance> provenance;
};

// Data transfer value only. No native state, allocation authority or
// persistence. Vector order is portable record order, not inferred browser
// presentation order.
struct Workspace {
  std::uint32_t format_version = kFormatVersion;
  std::uint32_t lifecycle_contract_version = lifecycle::kContractVersion;
  std::vector<Profile> profiles;
  std::vector<Space> spaces;
  std::vector<Item> items;
  std::vector<Shortcut> shortcuts;
};

enum class ErrorCode {
  kUnsupportedFormatVersion,
  kUnsupportedContractVersion,
  kMalformedEncoding,
  kInvalidText,
  kDuplicateId,
  kMissingReference,
  kProfileMismatch,
  kInvalidDestination,
};
struct Error {
  ErrorCode code;
  // Field path only. Never includes potentially sensitive source values.
  std::string field;
};
using WorkspaceResult = std::variant<Workspace, Error>;
using EncodeResult = std::variant<std::string, Error>;

// Structural and graph checks. Does not authorize native URL navigation.
std::optional<Error> Validate(const Workspace& workspace);
// No IO. Encode validates first and Decode validates before returning a value.
EncodeResult Encode(const Workspace& workspace);
WorkspaceResult Decode(std::string_view bytes);
// Snapshot on the model's serialized owner sequence. Does not read live pages.
WorkspaceResult Project(const lifecycle::Model& model);

}  // namespace arcified::portability

#endif  // ARCIFIED_NATIVE_FEATURES_PORTABILITY_FORMAT_H_
