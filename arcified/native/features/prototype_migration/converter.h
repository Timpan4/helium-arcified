#ifndef ARCIFIED_NATIVE_FEATURES_PROTOTYPE_MIGRATION_CONVERTER_H_
#define ARCIFIED_NATIVE_FEATURES_PROTOTYPE_MIGRATION_CONVERTER_H_

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "../portability/format.h"

namespace arcified::prototype_migration {

enum class ErrorCode {
  kMalformedJson,
  kUnexpectedField,
  kDuplicateField,
  kMissingField,
  kDuplicateSourceId,
  kInvalidSourceIdentity,
  kInvalidSourceValue,
  kPortableValidation,
};

struct Error {
  ErrorCode code;
  // Known schema path, never an imported key, URL, name or source ID.
  std::string field;
  // Offset in the supplied JSON when the source parser found the failure.
  std::optional<std::size_t> byte_offset;
  std::optional<portability::Error> portable_error;
};

enum class NoticeCode { kTimestampUnavailable };
struct Notice {
  NoticeCode code;
  std::string field;
};

struct Converted {
  portability::Workspace workspace;
  std::vector<Notice> notices;
};
struct Unavailable {};
using Result = std::variant<Converted, Unavailable, Error>;

// Convert explicitly supplied AppState JSON, never storage or an imported path.
// nullopt means unavailable; an empty JSON string is malformed, not
// unavailable. source_namespace is a caller-retained source identity, not a
// path or Profile. Reusing it preserves portable IDs. Import must explicitly
// map the unnamed source-group Profile to a real target Profile and
// allocate/map native IDs. No IO, native mutation, URL opening or removal of
// originals occurs here.
Result Convert(std::optional<std::string_view> json,
               std::string_view source_namespace);

}  // namespace arcified::prototype_migration

#endif  // ARCIFIED_NATIVE_FEATURES_PROTOTYPE_MIGRATION_CONVERTER_H_
