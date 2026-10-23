#include "format.h"

#include <charconv>
#include <limits>
#include <map>
#include <set>
#include <utility>

#include "utf8.h"

namespace arcified::portability {
namespace {

constexpr std::string_view kMagic = "arcified-workspace\n";
static_assert(lifecycle::kContractVersion == 3,
              "Review portability semantics when lifecycle authority changes");

bool ValidText(std::string_view text) {
  for (std::size_t i = 0; i < text.size();) {
    const auto length = internal::Utf8ScalarLength(text.substr(i));
    if (!length || text[i] == 0)
      return false;
    i += *length;
  }
  return true;
}

bool Digit(char c) {
  return c >= '0' && c <= '9';
}
bool Hex(char c) {
  return Digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}
bool Alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Ipv4(std::string_view host) {
  unsigned parts = 0;
  while (!host.empty()) {
    const auto dot = host.find('.');
    const auto part = host.substr(0, dot);
    unsigned value = 0;
    if (part.empty() || (part.size() > 1 && part.front() == '0'))
      return false;
    for (const auto c : part) {
      if (!Digit(c))
        return false;
      value = value * 10 + static_cast<unsigned>(c - '0');
      if (value > 255)
        return false;
    }
    ++parts;
    if (dot == std::string_view::npos)
      return parts == 4;
    host.remove_prefix(dot + 1);
  }
  return false;
}

// Counts 16-bit groups, including a final IPv4 address as two groups.
std::optional<unsigned> Ipv6Groups(std::string_view part, bool allow_ipv4) {
  unsigned groups = 0;
  while (!part.empty()) {
    const auto colon = part.find(':');
    const auto group = part.substr(0, colon);
    if (group.find('.') != std::string_view::npos) {
      if (!allow_ipv4 || colon != std::string_view::npos || !Ipv4(group))
        return std::nullopt;
      return groups + 2;
    }
    if (group.empty() || group.size() > 4)
      return std::nullopt;
    for (const auto c : group) {
      if (!Hex(c))
        return std::nullopt;
    }
    if (++groups > 8)
      return std::nullopt;
    if (colon == std::string_view::npos)
      return groups;
    part.remove_prefix(colon + 1);
    if (part.empty())
      return std::nullopt;
  }
  return groups;
}

bool Ipv6(std::string_view host) {
  const auto gap = host.find("::");
  if (gap == std::string_view::npos) {
    const auto groups = Ipv6Groups(host, true);
    return groups && *groups == 8;
  }
  if (host.find("::", gap + 2) != std::string_view::npos)
    return false;
  const auto left = Ipv6Groups(host.substr(0, gap), false);
  const auto right = Ipv6Groups(host.substr(gap + 2), true);
  return left && right && *left + *right < 8;
}

bool Domain(std::string_view host) {
  bool numeric = true;
  for (const auto c : host)
    numeric = numeric && (Digit(c) || c == '.');
  if (numeric)
    return Ipv4(host);
  if (!host.empty() && host.back() == '.')
    host.remove_suffix(1);
  while (!host.empty()) {
    const auto dot = host.find('.');
    const auto label = host.substr(0, dot);
    if (label.empty() || label.front() == '-' || label.back() == '-')
      return false;
    for (const auto c : label) {
      if (!Alpha(c) && !Digit(c) && c != '-')
        return false;
    }
    if (dot == std::string_view::npos)
      return true;
    host.remove_prefix(dot + 1);
  }
  return false;
}

bool WebDestination(std::string_view url) {
  // Exact-preserving, conservative interchange syntax. Not a browser parser.
  if (!ValidText(url))
    return false;
  for (std::size_t i = 0; i < url.size(); ++i) {
    const auto c = static_cast<unsigned char>(url[i]);
    if (c <= 0x20 || c == 0x7f || c == '\\')
      return false;
    if (c == '%') {
      if (url.size() - i < 3 || !Hex(url[i + 1]) || !Hex(url[i + 2]))
        return false;
      i += 2;
    }
  }
  std::size_t start;
  if (url.substr(0, 7) == "http://")
    start = 7;
  else if (url.substr(0, 8) == "https://")
    start = 8;
  else
    return false;
  const auto end = url.find_first_of("/?#", start);
  auto authority =
      url.substr(start, end == std::string_view::npos ? end : end - start);
  if (authority.empty() || authority.find('@') != std::string_view::npos)
    return false;
  std::string_view port;
  if (authority.front() == '[') {
    const auto close = authority.find(']');
    if (close == std::string_view::npos ||
        !Ipv6(authority.substr(1, close - 1)))
      return false;
    const auto suffix = authority.substr(close + 1);
    if (!suffix.empty()) {
      if (suffix.front() != ':')
        return false;
      port = suffix.substr(1);
      if (port.empty())
        return false;
    }
  } else {
    const auto colon = authority.find(':');
    if (!Domain(authority.substr(0, colon)))
      return false;
    if (colon != std::string_view::npos) {
      port = authority.substr(colon + 1);
      if (port.empty())
        return false;
    }
  }
  unsigned number = 0;
  for (const auto c : port) {
    if (!Digit(c))
      return false;
    number = number * 10 + static_cast<unsigned>(c - '0');
    if (number > 65535)
      return false;
  }
  return true;
}

std::string Field(std::string_view section, std::size_t i) {
  return std::string(section) + "[" + std::to_string(i) + "]";
}

std::optional<Error> TextError(const std::optional<std::string>& value,
                               const std::string& field) {
  if (value && !ValidText(*value))
    return Error{ErrorCode::kInvalidText, field};
  return std::nullopt;
}
std::optional<Error> ProvenanceError(const std::optional<Provenance>& value,
                                     const std::string& field) {
  if (value && (value->source.empty() || value->source_id.empty() ||
                !ValidText(value->source) || !ValidText(value->source_id)))
    return Error{ErrorCode::kInvalidText, field + ".provenance"};
  return std::nullopt;
}

template <typename Records, typename Map>
std::optional<Error> Index(const Records& records,
                           Map& index,
                           std::string_view section) {
  for (std::size_t i = 0; i < records.size(); ++i) {
    const auto& record = records[i];
    const auto field = Field(section, i);
    if (record.id.value.empty() || !ValidText(record.id.value))
      return Error{ErrorCode::kInvalidText, field + ".id"};
    if (!index.emplace(record.id, &record).second)
      return Error{ErrorCode::kDuplicateId, field + ".id"};
    if (auto error = ProvenanceError(record.provenance, field))
      return error;
  }
  return std::nullopt;
}

class Writer {
 public:
  void Number(std::uint64_t value) { bytes += std::to_string(value) + '\n'; }
  void Text(const std::string& value) {
    bytes += std::to_string(value.size()) + ':';
    bytes += value;
    bytes += ',';
  }
  void Optional(const std::optional<std::string>& value) {
    Number(value ? 1 : 0);
    if (value)
      Text(*value);
  }
  void Source(const std::optional<Provenance>& value) {
    Number(value ? 1 : 0);
    if (value) {
      Text(value->source);
      Text(value->source_id);
      Number(value->ordinal ? 1 : 0);
      if (value->ordinal)
        Number(*value->ordinal);
    }
  }
  std::string bytes{kMagic};
};

class Reader {
 public:
  explicit Reader(std::string_view bytes) : bytes_(bytes) {}
  bool Number(std::uint64_t& value, char separator = '\n') {
    const auto end = bytes_.find(separator);
    if (end == std::string_view::npos || end == 0 ||
        (end > 1 && bytes_.front() == '0'))
      return false;
    for (const auto c : bytes_.substr(0, end)) {
      if (!Digit(c))
        return false;
    }
    const auto result =
        std::from_chars(bytes_.data(), bytes_.data() + end, value);
    if (result.ec != std::errc{})
      return false;
    bytes_.remove_prefix(end + 1);
    return true;
  }
  bool Text(std::string& value) {
    std::uint64_t size;
    if (!Number(size, ':') || size >= bytes_.size())
      return false;
    const auto count = static_cast<std::size_t>(size);
    if (bytes_[count] != ',')
      return false;
    value.assign(bytes_.substr(0, count));
    bytes_.remove_prefix(count + 1);
    return true;
  }
  bool Flag(bool& value) {
    std::uint64_t number;
    if (!Number(number) || number > 1)
      return false;
    value = number == 1;
    return true;
  }
  bool Optional(std::optional<std::string>& value) {
    bool present;
    if (!Flag(present))
      return false;
    if (!present)
      return true;
    std::string text;
    if (!Text(text))
      return false;
    value = std::move(text);
    return true;
  }
  bool Source(std::optional<Provenance>& value) {
    bool present;
    if (!Flag(present))
      return false;
    if (!present)
      return true;
    Provenance source;
    bool ordinal;
    if (!Text(source.source) || !Text(source.source_id) || !Flag(ordinal))
      return false;
    if (ordinal) {
      std::uint64_t number;
      if (!Number(number))
        return false;
      source.ordinal = number;
    }
    value = std::move(source);
    return true;
  }
  bool Count(std::uint64_t& count) {
    // Every record consumes bytes. Bound by the supplied input, never reserve
    // attacker-declared counts or introduce an arbitrary format capacity.
    return Number(count) && count <= bytes_.size();
  }
  bool empty() const { return bytes_.empty(); }

 private:
  std::string_view bytes_;
};

}  // namespace

std::optional<Error> Validate(const Workspace& workspace) {
  if (workspace.format_version != kFormatVersion)
    return Error{ErrorCode::kUnsupportedFormatVersion, "format_version"};
  if (workspace.lifecycle_contract_version != lifecycle::kContractVersion)
    return Error{ErrorCode::kUnsupportedContractVersion,
                 "lifecycle_contract_version"};
  std::map<lifecycle::ProfileId, const Profile*> profiles;
  std::map<lifecycle::SpaceId, const Space*> spaces;
  std::map<lifecycle::ItemId, const Item*> items;
  std::map<lifecycle::ShortcutId, const Shortcut*> shortcuts;
  if (auto error = Index(workspace.profiles, profiles, "profiles"))
    return error;
  if (auto error = Index(workspace.spaces, spaces, "spaces"))
    return error;
  if (auto error = Index(workspace.items, items, "items"))
    return error;
  if (auto error = Index(workspace.shortcuts, shortcuts, "shortcuts"))
    return error;
  for (std::size_t i = 0; i < workspace.profiles.size(); ++i) {
    if (auto error = TextError(workspace.profiles[i].name,
                               Field("profiles", i) + ".name"))
      return error;
  }
  for (std::size_t i = 0; i < workspace.spaces.size(); ++i) {
    const auto& space = workspace.spaces[i];
    const auto field = Field("spaces", i);
    if (!profiles.count(space.profile))
      return Error{ErrorCode::kMissingReference, field + ".profile"};
    if (auto error = TextError(space.name, field + ".name"))
      return error;
    if (auto error = TextError(space.saved_at, field + ".saved_at"))
      return error;
  }
  std::map<lifecycle::ItemId, lifecycle::ProfileId> item_profiles;
  for (std::size_t i = 0; i < workspace.items.size(); ++i) {
    const auto& item = workspace.items[i];
    const auto field = Field("items", i);
    if (auto error = TextError(item.name, field + ".name"))
      return error;
    if (!WebDestination(item.saved_destination))
      return Error{ErrorCode::kInvalidDestination,
                   field + ".saved_destination"};
    if (const auto* space = std::get_if<lifecycle::SpaceId>(&item.owner)) {
      const auto owner = spaces.find(*space);
      if (owner == spaces.end())
        return Error{ErrorCode::kMissingReference, field + ".owner"};
      item_profiles.emplace(item.id, owner->second->profile);
    } else {
      const auto& profile = std::get<lifecycle::ProfileId>(item.owner);
      if (!profiles.count(profile))
        return Error{ErrorCode::kMissingReference, field + ".owner"};
      item_profiles.emplace(item.id, profile);
    }
  }
  for (std::size_t i = 0; i < workspace.shortcuts.size(); ++i) {
    const auto& shortcut = workspace.shortcuts[i];
    const auto field = Field("shortcuts", i);
    if (!profiles.count(shortcut.profile))
      return Error{ErrorCode::kMissingReference, field + ".profile"};
    const auto target = item_profiles.find(shortcut.target);
    if (target == item_profiles.end())
      return Error{ErrorCode::kMissingReference, field + ".target"};
    if (target->second != shortcut.profile)
      return Error{ErrorCode::kProfileMismatch, field + ".target"};
  }
  return std::nullopt;
}

EncodeResult Encode(const Workspace& workspace) {
  if (auto error = Validate(workspace))
    return *error;
  Writer writer;
  writer.Number(workspace.format_version);
  writer.Number(workspace.lifecycle_contract_version);
  writer.Number(workspace.profiles.size());
  for (const auto& profile : workspace.profiles) {
    writer.Text(profile.id.value);
    writer.Optional(profile.name);
    writer.Source(profile.provenance);
  }
  writer.Number(workspace.spaces.size());
  for (const auto& space : workspace.spaces) {
    writer.Text(space.id.value);
    writer.Text(space.profile.value);
    writer.Optional(space.name);
    writer.Optional(space.saved_at);
    writer.Source(space.provenance);
  }
  writer.Number(workspace.items.size());
  for (const auto& item : workspace.items) {
    writer.Text(item.id.value);
    const auto* space = std::get_if<lifecycle::SpaceId>(&item.owner);
    writer.Number(space ? 0 : 1);
    writer.Text(space ? space->value
                      : std::get<lifecycle::ProfileId>(item.owner).value);
    writer.Text(item.saved_destination);
    writer.Optional(item.name);
    writer.Source(item.provenance);
  }
  writer.Number(workspace.shortcuts.size());
  for (const auto& shortcut : workspace.shortcuts) {
    writer.Text(shortcut.id.value);
    writer.Text(shortcut.profile.value);
    writer.Text(shortcut.target.value);
    writer.Source(shortcut.provenance);
  }
  return std::move(writer.bytes);
}

WorkspaceResult Decode(std::string_view bytes) {
  const auto malformed = [](std::string field) -> WorkspaceResult {
    return Error{ErrorCode::kMalformedEncoding, std::move(field)};
  };
  if (bytes.substr(0, kMagic.size()) != kMagic)
    return malformed("header");
  Reader reader(bytes.substr(kMagic.size()));
  Workspace workspace;
  std::uint64_t number;
  if (!reader.Number(number))
    return malformed("format_version");
  if (number != kFormatVersion)
    return Error{ErrorCode::kUnsupportedFormatVersion, "format_version"};
  if (!reader.Number(number))
    return malformed("lifecycle_contract_version");
  if (number != lifecycle::kContractVersion)
    return Error{ErrorCode::kUnsupportedContractVersion,
                 "lifecycle_contract_version"};
  std::uint64_t count;
  if (!reader.Count(count))
    return malformed("profiles.count");
  for (std::uint64_t i = 0; i < count; ++i) {
    Profile record{lifecycle::ProfileId{""}, std::nullopt, std::nullopt};
    if (!reader.Text(record.id.value) || !reader.Optional(record.name) ||
        !reader.Source(record.provenance))
      return malformed(Field("profiles", i));
    workspace.profiles.push_back(std::move(record));
  }
  if (!reader.Count(count))
    return malformed("spaces.count");
  for (std::uint64_t i = 0; i < count; ++i) {
    Space record{lifecycle::SpaceId{""}, lifecycle::ProfileId{""}, std::nullopt,
                 std::nullopt, std::nullopt};
    if (!reader.Text(record.id.value) || !reader.Text(record.profile.value) ||
        !reader.Optional(record.name) || !reader.Optional(record.saved_at) ||
        !reader.Source(record.provenance))
      return malformed(Field("spaces", i));
    workspace.spaces.push_back(std::move(record));
  }
  if (!reader.Count(count))
    return malformed("items.count");
  for (std::uint64_t i = 0; i < count; ++i) {
    Item record{lifecycle::ItemId{""}, lifecycle::SpaceId{""}, "", std::nullopt,
                std::nullopt};
    bool profile_owner;
    std::string owner;
    if (!reader.Text(record.id.value) || !reader.Flag(profile_owner) ||
        !reader.Text(owner) || !reader.Text(record.saved_destination) ||
        !reader.Optional(record.name) || !reader.Source(record.provenance))
      return malformed(Field("items", i));
    if (profile_owner)
      record.owner = lifecycle::ProfileId{std::move(owner)};
    else
      record.owner = lifecycle::SpaceId{std::move(owner)};
    workspace.items.push_back(std::move(record));
  }
  if (!reader.Count(count))
    return malformed("shortcuts.count");
  for (std::uint64_t i = 0; i < count; ++i) {
    Shortcut record{lifecycle::ShortcutId{""}, lifecycle::ProfileId{""},
                    lifecycle::ItemId{""}, std::nullopt};
    if (!reader.Text(record.id.value) || !reader.Text(record.profile.value) ||
        !reader.Text(record.target.value) || !reader.Source(record.provenance))
      return malformed(Field("shortcuts", i));
    workspace.shortcuts.push_back(std::move(record));
  }
  if (!reader.empty())
    return malformed("trailing_bytes");
  if (auto error = Validate(workspace))
    return *error;
  return workspace;
}

WorkspaceResult Project(const lifecycle::Model& model) {
  Workspace workspace;
  for (const auto& [id, profile] : model.profiles())
    workspace.profiles.push_back({id, std::nullopt, std::nullopt});
  for (const auto& [id, space] : model.spaces())
    workspace.spaces.push_back(
        {id, space.profile, std::nullopt, std::nullopt, std::nullopt});
  for (const auto& [id, item] : model.items())
    workspace.items.push_back(
        {id, item.owner, item.saved_destination, std::nullopt, std::nullopt});
  for (const auto& [id, shortcut] : model.shortcuts())
    workspace.shortcuts.push_back(
        {id, shortcut.profile, shortcut.target, std::nullopt});
  if (auto error = Validate(workspace))
    return *error;
  return workspace;
}

}  // namespace arcified::portability
