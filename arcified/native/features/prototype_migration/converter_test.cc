#include "converter.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace {

namespace migration = arcified::prototype_migration;
namespace portable = arcified::portability;

void Require(bool condition, const char* message) {
  if (!condition)
    throw std::runtime_error(message);
}

migration::Converted Converted(std::string_view json,
                               std::string_view source = "synthetic-source") {
  auto result = migration::Convert(json, source);
  Require(std::holds_alternative<migration::Converted>(result),
          "conversion must succeed");
  return std::get<migration::Converted>(std::move(result));
}

migration::Error Rejected(std::string_view json) {
  auto result = migration::Convert(json, "synthetic-source");
  Require(std::holds_alternative<migration::Error>(result),
          "invalid input must return only an error");
  return std::get<migration::Error>(std::move(result));
}

std::string Encoded(const portable::Workspace& workspace) {
  auto result = portable::Encode(workspace);
  Require(std::holds_alternative<std::string>(result),
          "real portable encoder must accept converted workspace");
  return std::get<std::string>(std::move(result));
}

std::string OneSnapshot(std::string_view name,
                        std::string_view url = "https://example.test/a") {
  return "{\"spaces\":[{\"id\":\"a\",\"name\":\"" + std::string(name) +
         "\",\"urls\":[\"" + std::string(url) + "\"]}]}";
}

void SourceFixture(const std::string& input) {
  const std::string original = input;
  const auto converted = Converted(input);
  const auto& workspace = converted.workspace;
  Require(input == original, "supplied input must remain intact");
  Require(!portable::Validate(workspace), "shared graph validation must pass");
  Require(workspace.profiles.size() == 1 && !workspace.profiles[0].name,
          "source grouping must be unnamed, not an inferred native Profile");
  Require(workspace.spaces.size() == 3 && workspace.items.size() == 4 &&
              workspace.shortcuts.empty(),
          "preserve all source records, including an empty snapshot");
  Require(workspace.spaces[0].name == "Research" &&
              workspace.spaces[1].name == "Research" &&
              workspace.spaces[2].name == "Empty snapshot",
          "preserve source names and Space order");
  Require(workspace.spaces[0].id != workspace.spaces[1].id,
          "duplicate names must retain distinct source identity");
  Require(workspace.spaces[0].saved_at == "2026-10-23T12:34:56.789Z" &&
              !workspace.spaces[1].saved_at &&
              workspace.spaces[2].saved_at == "2026-10-22T00:00:00.000Z",
          "preserve available source savedAt without inventing a timestamp");
  Require(converted.notices.size() == 1 &&
              converted.notices[0].code ==
                  migration::NoticeCode::kTimestampUnavailable &&
              converted.notices[0].field == "$.spaces[1].savedAt",
          "report an unavailable timestamp at its source field");
  Require(
      workspace.items[0].saved_destination ==
              "https://example.test/second?order=2" &&
          workspace.items[1].saved_destination ==
              "https://example.test/first?order=1" &&
          workspace.items[2].saved_destination ==
              "https://example.test/second?order=2" &&
          workspace.items[3].saved_destination == "http://example.test/other",
      "preserve source URL order and repeated URLs");
  Require(workspace.items[0].id != workspace.items[2].id &&
              workspace.items[0].owner == workspace.items[2].owner,
          "same URL at different positions must remain distinct items");
  Require(workspace.spaces[0].provenance->source_id == "snapshot-a" &&
              workspace.spaces[1].provenance->source_id == "snapshot-b" &&
              workspace.spaces[1].provenance->ordinal == 1 &&
              workspace.items[2].provenance->source_id == "snapshot-a" &&
              workspace.items[2].provenance->ordinal == 2,
          "preserve original IDs and array positions as provenance");
  const std::string bytes = Encoded(workspace);
  auto decoded = portable::Decode(bytes);
  Require(std::holds_alternative<portable::Workspace>(decoded),
          "real portable decoder must accept converted bytes");
  Require(Encoded(std::get<portable::Workspace>(decoded)) == bytes,
          "portable round trip must preserve converted data");
  Require(Encoded(Converted(input).workspace) == bytes,
          "repeat conversion must have stable source identities and bytes");
  Require(Converted(input, "another-source").workspace.spaces[0].id !=
              workspace.spaces[0].id,
          "independent source namespaces must not collide");
  const auto changed =
      Converted(OneSnapshot("Research", "https://example.test/new"));
  const auto before = Converted(OneSnapshot("Research"));
  Require(changed.workspace.items[0].id == before.workspace.items[0].id &&
              Encoded(changed.workspace) != Encoded(before.workspace),
          "changed source content retains identity for import conflict review");
  const std::size_t end = input.find_last_not_of(" \r\n\t") + 1;
  for (std::size_t cut = 0; cut < end; ++cut)
    Rejected(std::string_view(input).substr(0, cut));
}

void SourceFailures() {
  Require(std::holds_alternative<migration::Unavailable>(
              migration::Convert(std::nullopt, "synthetic-source")),
          "unavailable data must differ from empty or malformed data");
  const auto empty = Converted(R"({"spaces":[]})");
  Require(empty.workspace.spaces.empty() && empty.workspace.items.empty(),
          "available empty state must remain available");
  Require(std::holds_alternative<migration::Error>(
              migration::Convert(R"({"spaces":[]})", "")),
          "caller must supply a stable source namespace");
  for (const auto* json : {
           "",
           "null",
           "[]",
           "{}",
           R"({"spaces":null})",
           R"({"spaces":{}})",
           R"({"spaces":[null]})",
           R"({"spaces":[{}]})",
           R"({"spaces":[{"id":3,"name":"A","urls":[]}]})",
           R"({"spaces":[{"id":"a","name":true,"urls":[]}]})",
           R"({"spaces":[{"id":"a","name":"A","urls":"not-array"}]})",
           R"({"spaces":[{"id":"a","name":"A","urls":[null]}]})",
           R"({"spaces":[{"id":"a","name":"A","urls":[],"savedAt":3}]})",
           R"({"spaces":[{"id":"a","name":"A","urls":[],"savedAt":null}]})",
           R"({"spaces":[{"id":"","name":"A","urls":[]}]})",
           R"({"spaces":[{"id":"a","name":"","urls":[]}]})",
           R"({"spaces":[],})",
           R"({"spaces":[,]})",
           R"({"spaces":[]}{})",
           R"({/*comment*/"spaces":[]})",
           R"({spaces:[]})",
       }) {
    Rejected(json);
  }
  Require(
      Rejected(R"({"spaces":[],"spaces":[]})").code ==
              migration::ErrorCode::kDuplicateField &&
          Rejected(
              R"({"spaces":[{"id":"a","\u0069d":"b","name":"A","urls":[]}]})")
                  .code == migration::ErrorCode::kDuplicateField,
      "reject duplicate keys after JSON escape decoding");
  const auto unknown = Rejected(R"({"spaces":[],"private-source-key":[]})");
  Require(unknown.code == migration::ErrorCode::kUnexpectedField &&
              unknown.field == "$" && unknown.byte_offset.has_value(),
          "unknown-field diagnostics must not copy imported field names");
  Require(
      Rejected(
          R"({"spaces":[{"id":"a","name":"A","urls":[],"html":"<script/>"}]})")
              .code == migration::ErrorCode::kUnexpectedField,
      "unsupported attached content must be reported, not interpreted");
  Require(
      Rejected(
          R"({"spaces":[{"id":"a","name":"A","urls":[]},{"id":"a","name":"B","urls":[]}]})")
              .code == migration::ErrorCode::kDuplicateSourceId,
      "duplicate source identity must not silently replace a snapshot");
  Require(Rejected(R"({"spaces":[{"id":"a","urls":[]}]})").field ==
              "$.spaces[0].name",
          "missing-field diagnostics must identify the source field");
}

void UnicodeAndEscapes() {
  const auto converted =
      Converted(OneSnapshot(R"(R\u0065search \ud83d\ude80 café \" \\ \/)"));
  Require(converted.workspace.spaces[0].name == "Research 🚀 café \" \\ /",
          "decode JSON escapes, paired surrogates and raw UTF-8 exactly");
  Require(Converted(OneSnapshot("  <script>text</script>  "))
                  .workspace.spaces[0]
                  .name == "  <script>text</script>  ",
          "preserve inert names without trimming or interpreting HTML");
  for (const auto* escaped :
       {R"(\q)", R"(\u00xz)", R"(\u123)", R"(\ud800)", R"(\udfff)",
        R"(\ud800\u0041)", R"(\ud800x)", R"(\ud800\ud800)"}) {
    Require(Rejected(OneSnapshot(escaped)).code ==
                migration::ErrorCode::kMalformedJson,
            "reject malformed escape and unpaired surrogate");
  }
  for (const auto* bytes :
       {"\x80", "\xc0\xaf", "\xe0\x80\xaf", "\xed\xa0\x80", "\xf0\x80\x80\xaf",
        "\xf4\x90\x80\x80", "\xf5\x80\x80\x80", "\xe2\x82", "\xc2x"}) {
    Require(Rejected(OneSnapshot(bytes)).code ==
                migration::ErrorCode::kMalformedJson,
            "reject malformed or non-scalar UTF-8");
  }
  Require(Rejected(OneSnapshot("literal\nnewline")).code ==
              migration::ErrorCode::kMalformedJson,
          "reject unescaped JSON control bytes");
}

void InertDestinations() {
  for (const auto* url : {
           "javascript:alert(1)",
           "data:text/html,<script/>",
           "file:///tmp/source",
           "chrome://settings",
           "../relative",
           "//example.test/a",
           "/tmp/source",
           "https://user:password@example.test/",
           "https://",
           "https://example.test/%zz",
           "https://example.test:bad/",
           "https://example.test/has space",
       }) {
    const auto error = Rejected(OneSnapshot("A", url));
    Require(
        error.code == migration::ErrorCode::kPortableValidation &&
            error.portable_error->code ==
                portable::ErrorCode::kInvalidDestination,
        "shared validator must reject unsupported or malformed destinations");
  }
}

}  // namespace

int main(int argc, char** argv) {
  try {
    Require(argc == 2, "pass the synthetic snapshot fixture path");
    std::ifstream fixture(argv[1], std::ios::binary);
    Require(fixture.good(), "synthetic fixture must be readable");
    const std::string input{std::istreambuf_iterator<char>(fixture),
                            std::istreambuf_iterator<char>()};
    SourceFixture(input);
    SourceFailures();
    UnicodeAndEscapes();
    InertDestinations();
    std::cout << "Prototype migration conversion checks passed\n";
    return EXIT_SUCCESS;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
