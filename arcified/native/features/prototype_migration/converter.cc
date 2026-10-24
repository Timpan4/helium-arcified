#include "converter.h"

#include <cstdint>
#include <set>
#include <utility>

#include "../portability/utf8.h"

namespace arcified::prototype_migration {
namespace {

struct Snapshot {
  std::string id;
  std::string name;
  std::vector<std::string> urls;
  std::optional<std::string> saved_at;
};

// Schema-bound parser, not a general JSON tree. Its call depth follows the
// fixed AppState schema; arbitrary nested input cannot recurse. RFC 8259 string
// escapes and UTF-8 scalar values are decoded without evaluating source text.
class Parser {
 public:
  explicit Parser(std::string_view input) : input_(input) {}

  std::variant<std::vector<Snapshot>, Error> Parse() {
    std::vector<Snapshot> snapshots;
    bool found_spaces = false;
    if (!Object("$", [&](const std::string& key) {
          if (key != "spaces")
            return Fail(ErrorCode::kUnexpectedField, "$");
          found_spaces = true;
          return Array("$.spaces", [&](std::size_t index) {
            Snapshot snapshot;
            if (!ReadSnapshot("$.spaces[" + std::to_string(index) + "]",
                              snapshot)) {
              return false;
            }
            snapshots.push_back(std::move(snapshot));
            return true;
          });
        })) {
      return *error_;
    }
    if (!found_spaces)
      return MakeError(ErrorCode::kMissingField, "$.spaces");
    Whitespace();
    if (position_ != input_.size())
      return MakeError(ErrorCode::kMalformedJson, "$");
    return snapshots;
  }

 private:
  Error MakeError(ErrorCode code, const std::string& field) const {
    return {code, field, position_, std::nullopt};
  }

  bool Fail(ErrorCode code, const std::string& field) {
    error_ = MakeError(code, field);
    return false;
  }

  void Whitespace() {
    while (position_ < input_.size()) {
      const char c = input_[position_];
      if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
        break;
      ++position_;
    }
  }

  bool Take(char expected) {
    Whitespace();
    if (position_ == input_.size() || input_[position_] != expected)
      return false;
    ++position_;
    return true;
  }

  bool Expect(char expected, const std::string& field) {
    return Take(expected) || Fail(ErrorCode::kMalformedJson, field);
  }

  template <typename ReadField>
  bool Object(const std::string& field, ReadField read_field) {
    if (!Expect('{', field))
      return false;
    std::set<std::string> keys;
    if (Take('}'))
      return true;
    do {
      std::string key;
      if (!String(field, key))
        return false;
      if (!keys.insert(key).second)
        return Fail(ErrorCode::kDuplicateField, field);
      if (!Expect(':', field) || !read_field(key))
        return false;
      if (Take('}'))
        return true;
    } while (Take(','));
    return Fail(ErrorCode::kMalformedJson, field);
  }

  template <typename ReadEntry>
  bool Array(const std::string& field, ReadEntry read_entry) {
    if (!Expect('[', field))
      return false;
    if (Take(']'))
      return true;
    std::size_t index = 0;
    do {
      if (!read_entry(index++))
        return false;
      if (Take(']'))
        return true;
    } while (Take(','));
    return Fail(ErrorCode::kMalformedJson, field);
  }

  bool HexQuad(const std::string& field, std::uint32_t& value) {
    value = 0;
    // JSON's \u escape contains exactly four hexadecimal digits.
    for (int digit = 0; digit < 4; ++digit) {
      if (position_ == input_.size())
        return Fail(ErrorCode::kMalformedJson, field);
      const char c = input_[position_++];
      value *= 16;
      if (c >= '0' && c <= '9')
        value += c - '0';
      else if (c >= 'a' && c <= 'f')
        value += c - 'a' + 10;
      else if (c >= 'A' && c <= 'F')
        value += c - 'A' + 10;
      else
        return Fail(ErrorCode::kMalformedJson, field);
    }
    return true;
  }

  static void AppendScalar(std::uint32_t scalar, std::string& output) {
    if (scalar <= 0x7f) {
      output.push_back(static_cast<char>(scalar));
    } else if (scalar <= 0x7ff) {
      output.push_back(static_cast<char>(0xc0 | (scalar >> 6)));
      output.push_back(static_cast<char>(0x80 | (scalar & 0x3f)));
    } else if (scalar <= 0xffff) {
      output.push_back(static_cast<char>(0xe0 | (scalar >> 12)));
      output.push_back(static_cast<char>(0x80 | ((scalar >> 6) & 0x3f)));
      output.push_back(static_cast<char>(0x80 | (scalar & 0x3f)));
    } else {
      output.push_back(static_cast<char>(0xf0 | (scalar >> 18)));
      output.push_back(static_cast<char>(0x80 | ((scalar >> 12) & 0x3f)));
      output.push_back(static_cast<char>(0x80 | ((scalar >> 6) & 0x3f)));
      output.push_back(static_cast<char>(0x80 | (scalar & 0x3f)));
    }
  }

  bool UnicodeEscape(const std::string& field, std::string& output) {
    std::uint32_t scalar;
    if (!HexQuad(field, scalar))
      return false;
    if (scalar >= 0xd800 && scalar <= 0xdbff) {
      // A high surrogate must be immediately followed by an escaped low one.
      if (position_ == input_.size() || input_[position_++] != '\\' ||
          position_ == input_.size() || input_[position_++] != 'u') {
        return Fail(ErrorCode::kMalformedJson, field);
      }
      std::uint32_t low;
      if (!HexQuad(field, low))
        return false;
      if (low < 0xdc00 || low > 0xdfff)
        return Fail(ErrorCode::kMalformedJson, field);
      scalar = 0x10000 + ((scalar - 0xd800) << 10) + low - 0xdc00;
    } else if (scalar >= 0xdc00 && scalar <= 0xdfff) {
      return Fail(ErrorCode::kMalformedJson, field);
    }
    AppendScalar(scalar, output);
    return true;
  }

  bool RawUtf8(const std::string& field, std::string& output) {
    const auto length =
        portability::internal::Utf8ScalarLength(input_.substr(position_));
    if (!length)
      return Fail(ErrorCode::kMalformedJson, field);
    output.append(input_.substr(position_, *length));
    position_ += *length;
    return true;
  }

  bool String(const std::string& field, std::string& output) {
    if (!Expect('"', field))
      return false;
    while (position_ < input_.size()) {
      const auto c = static_cast<unsigned char>(input_[position_]);
      if (c >= 0x80) {
        if (!RawUtf8(field, output))
          return false;
        continue;
      }
      ++position_;
      if (c == '"')
        return true;
      if (c < 0x20)
        return Fail(ErrorCode::kMalformedJson, field);
      if (c != '\\') {
        output.push_back(static_cast<char>(c));
        continue;
      }
      if (position_ == input_.size())
        return Fail(ErrorCode::kMalformedJson, field);
      switch (input_[position_++]) {
        case '"':
          output.push_back('"');
          break;
        case '\\':
          output.push_back('\\');
          break;
        case '/':
          output.push_back('/');
          break;
        case 'b':
          output.push_back('\b');
          break;
        case 'f':
          output.push_back('\f');
          break;
        case 'n':
          output.push_back('\n');
          break;
        case 'r':
          output.push_back('\r');
          break;
        case 't':
          output.push_back('\t');
          break;
        case 'u':
          if (!UnicodeEscape(field, output))
            return false;
          break;
        default:
          return Fail(ErrorCode::kMalformedJson, field);
      }
    }
    return Fail(ErrorCode::kMalformedJson, field);
  }

  bool ReadSnapshot(const std::string& field, Snapshot& snapshot) {
    std::set<std::string> present;
    if (!Object(field, [&](const std::string& key) {
          present.insert(key);
          if (key == "id")
            return String(field + ".id", snapshot.id);
          if (key == "name")
            return String(field + ".name", snapshot.name);
          if (key == "savedAt") {
            std::string timestamp;
            if (!String(field + ".savedAt", timestamp))
              return false;
            snapshot.saved_at = std::move(timestamp);
            return true;
          }
          if (key == "urls") {
            return Array(field + ".urls", [&](std::size_t index) {
              std::string url;
              if (!String(field + ".urls[" + std::to_string(index) + "]", url))
                return false;
              snapshot.urls.push_back(std::move(url));
              return true;
            });
          }
          return Fail(ErrorCode::kUnexpectedField, field);
        })) {
      return false;
    }
    for (const char* key : {"id", "name", "urls"}) {
      if (present.count(key) == 0)
        return Fail(ErrorCode::kMissingField, field + "." + key);
    }
    if (snapshot.id.empty())
      return Fail(ErrorCode::kInvalidSourceValue, field + ".id");
    if (snapshot.name.empty())
      return Fail(ErrorCode::kInvalidSourceValue, field + ".name");
    return true;
  }

  std::string_view input_;
  std::size_t position_ = 0;
  std::optional<Error> error_;
};

std::string Component(std::string_view value) {
  return std::to_string(value.size()) + ":" + std::string(value);
}

}  // namespace

Result Convert(std::optional<std::string_view> json,
               std::string_view source_namespace) {
  if (!json)
    return Unavailable{};
  if (source_namespace.empty()) {
    return Error{ErrorCode::kInvalidSourceIdentity, "source_namespace",
                 std::nullopt, std::nullopt};
  }
  auto parsed = Parser(*json).Parse();
  if (const auto* error = std::get_if<Error>(&parsed))
    return *error;

  Converted converted;
  auto& workspace = converted.workspace;
  const std::string source = "arcified-lab-v1:" + Component(source_namespace);
  const lifecycle::ProfileId profile(source + ":profile");
  workspace.profiles.push_back(
      {profile, std::nullopt,
       portability::Provenance{source, std::string(source_namespace),
                               std::nullopt}});
  std::set<std::string> source_ids;
  const auto& snapshots = std::get<std::vector<Snapshot>>(parsed);
  for (std::size_t i = 0; i < snapshots.size(); ++i) {
    const auto& snapshot = snapshots[i];
    const std::string field = "$.spaces[" + std::to_string(i) + "]";
    if (!source_ids.insert(snapshot.id).second) {
      return Error{ErrorCode::kDuplicateSourceId, field + ".id", std::nullopt,
                   std::nullopt};
    }
    const lifecycle::SpaceId space(source + ":space:" + Component(snapshot.id));
    std::optional<std::string> saved_at = snapshot.saved_at;
    if (!saved_at || saved_at->empty()) {
      saved_at.reset();
      converted.notices.push_back(
          {NoticeCode::kTimestampUnavailable, field + ".savedAt"});
    }
    workspace.spaces.push_back(
        {space, profile, snapshot.name, std::move(saved_at),
         portability::Provenance{source, snapshot.id, i}});
    for (std::size_t ordinal = 0; ordinal < snapshot.urls.size(); ++ordinal) {
      workspace.items.push_back(
          {lifecycle::ItemId(space.value + ":url:" + std::to_string(ordinal)),
           space, snapshot.urls[ordinal], std::nullopt,
           portability::Provenance{source, snapshot.id, ordinal}});
    }
  }
  if (auto error = portability::Validate(workspace)) {
    return Error{ErrorCode::kPortableValidation, "portable", std::nullopt,
                 std::move(error)};
  }
  return converted;
}

}  // namespace arcified::prototype_migration
