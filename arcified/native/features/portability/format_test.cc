#include "format.h"

#include <cstdlib>
#include <iostream>
#include <limits>

namespace arcified::portability {
namespace {

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

Workspace Fixture() {
  Workspace workspace;
  workspace.profiles = {{lifecycle::ProfileId{"p"}, "Synthetic", std::nullopt},
                        {lifecycle::ProfileId{"q"}, "Synthetic", std::nullopt}};
  workspace.spaces = {
      {lifecycle::SpaceId{"s"}, lifecycle::ProfileId{"p"}, "Same name",
       "2026-10-23T00:00:00Z", Provenance{"arcified-lab-v1", "source-s", {}}},
      {lifecycle::SpaceId{"t"}, lifecycle::ProfileId{"q"}, "Same name",
       std::string{}, std::nullopt}};
  workspace.items = {
      {lifecycle::ItemId{"z"}, lifecycle::SpaceId{"s"},
       "https://example.test/saved?x=1#fragment", "Title\n:, \xc3\xa5",
       Provenance{"arcified-lab-v1", "source-s", 0}},
      {lifecycle::ItemId{"a"}, lifecycle::ProfileId{"p"},
       "https://example.test/saved?x=1#fragment", std::nullopt,
       Provenance{"arcified-lab-v1", "source-s", 1}}};
  workspace.shortcuts = {
      {lifecycle::ShortcutId{"alias"}, lifecycle::ProfileId{"p"},
       lifecycle::ItemId{"z"},
       Provenance{"synthetic", "alias-source",
                  std::numeric_limits<std::uint64_t>::max()}}};
  return workspace;
}

std::string Bytes(const Workspace& workspace) {
  auto encoded = Encode(workspace);
  REQUIRE(std::holds_alternative<std::string>(encoded));
  return std::get<std::string>(encoded);
}

void ExpectInvalid(const Workspace& workspace, ErrorCode code) {
  const auto error = Validate(workspace);
  REQUIRE(error && error->code == code);
  const auto encoded = Encode(workspace);
  REQUIRE(std::holds_alternative<Error>(encoded));
  REQUIRE(std::get<Error>(encoded).code == code);
}

void GoldenAndRoundTrip() {
  const std::string golden = "arcified-workspace\n1\n3\n0\n0\n0\n0\n";
  REQUIRE(Bytes(Workspace{}) == golden);
  const auto empty = Decode(golden);
  REQUIRE(std::holds_alternative<Workspace>(empty));
  REQUIRE(std::get<Workspace>(empty).profiles.empty());
  auto workspace = Fixture();
  const auto bytes = Bytes(workspace);
  const auto decoded = Decode(bytes);
  REQUIRE(std::holds_alternative<Workspace>(decoded));
  const auto& value = std::get<Workspace>(decoded);
  REQUIRE(value.format_version == 1 && value.lifecycle_contract_version == 3);
  REQUIRE(value.profiles.size() == 2 && value.spaces.size() == 2);
  REQUIRE(value.items.size() == 2 && value.shortcuts.size() == 1);
  REQUIRE(value.items[0].id == lifecycle::ItemId{"z"});
  REQUIRE(value.items[1].id == lifecycle::ItemId{"a"});
  REQUIRE(value.items[0].saved_destination == value.items[1].saved_destination);
  REQUIRE(value.items[0].name == "Title\n:, \xc3\xa5");
  REQUIRE(!value.items[1].name);
  REQUIRE(value.spaces[0].saved_at == "2026-10-23T00:00:00Z");
  REQUIRE(value.spaces[1].saved_at && value.spaces[1].saved_at->empty());
  REQUIRE(value.items[0].provenance->source == "arcified-lab-v1");
  REQUIRE(value.items[0].provenance->source_id == "source-s");
  REQUIRE(value.items[0].provenance->ordinal == 0);
  REQUIRE(value.items[1].provenance->ordinal == 1);
  REQUIRE(value.shortcuts[0].provenance->ordinal ==
          std::numeric_limits<std::uint64_t>::max());
  REQUIRE(std::get<lifecycle::SpaceId>(value.items[0].owner).value == "s");
  REQUIRE(std::get<lifecycle::ProfileId>(value.items[1].owner).value == "p");
  REQUIRE(value.shortcuts[0].target == lifecycle::ItemId{"z"});
  REQUIRE(Bytes(value) == bytes);
  // Order is explicit, even when it differs from ID sorting.
  std::swap(workspace.items[0], workspace.items[1]);
  REQUIRE(Bytes(workspace) != bytes);
}

void MalformedAndTruncated() {
  const auto bytes = Bytes(Fixture());
  for (std::size_t size = 0; size < bytes.size(); ++size)
    REQUIRE(std::holds_alternative<Error>(
        Decode(std::string_view(bytes).substr(0, size))));
  REQUIRE(std::holds_alternative<Error>(Decode(bytes + "garbage")));
  const std::string prefix = "arcified-workspace\n1\n3\n";
  for (const auto& invalid :
       {"", "+0\n0\n0\n0\n", "00\n0\n0\n0\n", "-1\n", "18446744073709551616\n",
        "999999999999999999\n0\n", "1\n18446744073709551616:x,",
        "1\n1:x;0\n0\n0\n0\n0\n", "1\n1:p,2\n0\n0\n0\n0\n",
        "1\n1:p,0\n1\n0:,1:x,0\n0\n0\n0\n"}) {
    REQUIRE(std::holds_alternative<Error>(Decode(prefix + invalid)));
  }
  const auto future = Decode("arcified-workspace\n2\n3\n0\n0\n0\n0\n");
  REQUIRE(std::get<Error>(future).code == ErrorCode::kUnsupportedFormatVersion);
  const auto old = Decode("arcified-workspace\n1\n2\n0\n0\n0\n0\n");
  REQUIRE(std::get<Error>(old).code == ErrorCode::kUnsupportedContractVersion);
  // Decoder must graph-validate, even when framing is intact.
  auto dangling = bytes;
  const auto target = dangling.rfind("1:z,");
  REQUIRE(target != std::string::npos);
  dangling[target + 2] = 'x';
  REQUIRE(std::get<Error>(Decode(dangling)).code ==
          ErrorCode::kMissingReference);
  // Decoder must validate text, not merely accept a byte-length claim.
  const auto invalid_utf8 =
      prefix + "1\n1:" + std::string(1, '\xff') + ",0\n0\n0\n0\n0\n";
  REQUIRE(std::get<Error>(Decode(invalid_utf8)).code ==
          ErrorCode::kInvalidText);
}

void GraphAndTextRejections() {
  auto workspace = Fixture();
  workspace.format_version = 0;
  ExpectInvalid(workspace, ErrorCode::kUnsupportedFormatVersion);
  workspace = Fixture();
  workspace.lifecycle_contract_version = 2;
  ExpectInvalid(workspace, ErrorCode::kUnsupportedContractVersion);
  workspace = Fixture();
  workspace.profiles.push_back(workspace.profiles[0]);
  ExpectInvalid(workspace, ErrorCode::kDuplicateId);
  workspace = Fixture();
  workspace.spaces.push_back(workspace.spaces[0]);
  ExpectInvalid(workspace, ErrorCode::kDuplicateId);
  workspace = Fixture();
  workspace.items.push_back(workspace.items[0]);
  ExpectInvalid(workspace, ErrorCode::kDuplicateId);
  workspace = Fixture();
  workspace.shortcuts.push_back(workspace.shortcuts[0]);
  ExpectInvalid(workspace, ErrorCode::kDuplicateId);
  workspace = Fixture();
  workspace.profiles[0].id.value.clear();
  ExpectInvalid(workspace, ErrorCode::kInvalidText);
  workspace = Fixture();
  workspace.spaces[0].profile = lifecycle::ProfileId{"absent"};
  ExpectInvalid(workspace, ErrorCode::kMissingReference);
  workspace = Fixture();
  workspace.items[0].owner = lifecycle::SpaceId{"absent"};
  ExpectInvalid(workspace, ErrorCode::kMissingReference);
  workspace = Fixture();
  workspace.items[0].owner = lifecycle::ProfileId{"absent"};
  ExpectInvalid(workspace, ErrorCode::kMissingReference);
  workspace = Fixture();
  workspace.shortcuts[0].target = lifecycle::ItemId{"absent"};
  ExpectInvalid(workspace, ErrorCode::kMissingReference);
  workspace = Fixture();
  workspace.shortcuts[0].profile = lifecycle::ProfileId{"q"};
  ExpectInvalid(workspace, ErrorCode::kProfileMismatch);
  workspace = Fixture();
  workspace.items[0].provenance->source_id.clear();
  ExpectInvalid(workspace, ErrorCode::kInvalidText);
  for (const auto& text : {std::string("\xc0\xaf"), std::string("\xed\xa0\x80"),
                           std::string("\xf4\x90\x80\x80"),
                           std::string("\xe2\x82"), std::string("a\0b", 3)}) {
    workspace = Fixture();
    workspace.spaces[0].name = text;
    ExpectInvalid(workspace, ErrorCode::kInvalidText);
  }
}

void Destinations() {
  auto workspace = Fixture();
  for (const auto* destination :
       {"https://example.test/", "http://localhost:8080/x%20y",
        "https://127.0.0.1:443/", "https://[::1]/", "https://[2001:db8::1]:0/",
        "https://[::ffff:192.0.2.1]/", "https://xn--example.test./path?x=@ok",
        "https://example.test/\xc3\xa5"}) {
    workspace.items[0].saved_destination = destination;
    REQUIRE(!Validate(workspace));
    REQUIRE(std::holds_alternative<Workspace>(Decode(Bytes(workspace))));
  }
  for (const auto* destination : {"",
                                  "javascript:alert(1)",
                                  "data:text/plain,x",
                                  "file:///tmp/x",
                                  "chrome://settings",
                                  "/relative",
                                  "//example.test/",
                                  "https:example.test",
                                  "https:///path",
                                  "https://user:password@example.test/",
                                  "https://@example.test/",
                                  "https://example.test\\@evil.test/",
                                  "https://example.test/\n",
                                  "https://example.test/a b",
                                  "https://example.test/%",
                                  "https://example.test/%xz",
                                  "https://%65xample.test/",
                                  "https://.test/",
                                  "https://x..test/",
                                  "https://-x.test/",
                                  "https://127.0.0.999/",
                                  "https://0127.0.0.1/",
                                  "https://[1:2:3]/",
                                  "https://[1::2::3]/",
                                  "https://[::1]suffix/",
                                  "https://[1:2:3:4:5:6:7:8:9]/",
                                  "https://[1:2:3:4:5:6:7:]/",
                                  "https://example.test:/",
                                  "https://example.test:65536/",
                                  "https://example.test:999999999999999999/",
                                  "https://example.test:abc/"}) {
    workspace.items[0].saved_destination = destination;
    ExpectInvalid(workspace, ErrorCode::kInvalidDestination);
  }
}

void ProjectionExcludesRuntime() {
  lifecycle::Model model;
  const lifecycle::ProfileId profile{"p"};
  const lifecycle::SpaceId space{"s"};
  const lifecycle::Attachment attachment{lifecycle::WindowId{"w"}, space};
  const lifecycle::ItemId saved{"saved"};
  REQUIRE(model.AddProfile(profile) == lifecycle::Status::kOk);
  REQUIRE(model.AddSpace(space, profile) == lifecycle::Status::kOk);
  REQUIRE(model.AddItem(saved, space, "https://example.test/saved") ==
          lifecycle::Status::kOk);
  REQUIRE(model.AddShortcut(lifecycle::ShortcutId{"alias"}, profile, saved) ==
          lifecycle::Status::kOk);
  const auto before = Project(model);
  REQUIRE(std::holds_alternative<Workspace>(before));
  const auto before_bytes = Bytes(std::get<Workspace>(before));
  REQUIRE(model.AddWindow(attachment.window) == lifecycle::Status::kOk);
  REQUIRE(model.Attach(attachment) == lifecycle::Status::kOk);
  REQUIRE(model.SelectSpace(attachment.window, space) ==
          lifecycle::Status::kOk);
  REQUIRE(std::holds_alternative<lifecycle::OpenedPage>(
      model.OpenItem(saved, attachment, lifecycle::PageId{"live"})));
  REQUIRE(model.RecordNavigation(lifecycle::PageId{"live"},
                                 "https://example.test/current") ==
          lifecycle::Status::kOk);
  REQUIRE(model.OpenTemporary(lifecycle::PageId{"temp"},
                              lifecycle::TemporaryTabId{"temp"}, attachment,
                              "https://example.test/temporary") ==
          lifecycle::Status::kOk);
  REQUIRE(
      model.SetSplit(lifecycle::SplitId{"split"}, attachment,
                     {lifecycle::PageId{"live"}, lifecycle::PageId{"temp"}}) ==
      lifecycle::Status::kOk);
  const auto projected = Project(model);
  REQUIRE(std::holds_alternative<Workspace>(projected));
  const auto& value = std::get<Workspace>(projected);
  REQUIRE(Bytes(value) == before_bytes);
  REQUIRE(!value.spaces[0].name && !value.spaces[0].saved_at);
  REQUIRE(!value.profiles[0].name && !value.items[0].name);
  REQUIRE(model.pages().size() == 2 && model.splits().size() == 1);
  REQUIRE(model.pages().at(lifecycle::PageId{"live"}).current_navigation ==
          "https://example.test/current");
  REQUIRE(model.pages().at(lifecycle::PageId{"live"}).activity ==
          lifecycle::Activity::kUnknown);
  // Unsupported destination fails the whole projection, never silently drops
  // it.
  REQUIRE(model.AddItem(lifecycle::ItemId{"internal"}, space,
                        "chrome://settings") == lifecycle::Status::kOk);
  REQUIRE(std::get<Error>(Project(model)).code ==
          ErrorCode::kInvalidDestination);
  REQUIRE(model.items().size() == 2 && model.pages().size() == 2);
}

}  // namespace
}  // namespace arcified::portability

int main() {
  using namespace arcified::portability;
  const std::pair<const char*, void (*)()> tests[] = {
      {"golden-and-round-trip", GoldenAndRoundTrip},
      {"malformed-and-truncated", MalformedAndTruncated},
      {"graph-and-text-rejections", GraphAndTextRejections},
      {"destinations", Destinations},
      {"projection-excludes-runtime", ProjectionExcludesRuntime},
  };
  for (const auto& [name, run] : tests) {
    run();
    std::cout << "PASS " << name << '\n';
  }
}
