#include "planner.h"

#include <cstdlib>
#include <iostream>

#include "../prototype_migration/converter.h"

namespace arcified::workspace_import {
namespace {

using namespace lifecycle;

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const ProfileId kSourceProfile{"source-profile"};
const SpaceId kSourceSpace{"source-space"};
const ItemId kSourceItem{"source-item"};
const ItemId kSourceFavorite{"source-favorite"};
const ShortcutId kSourceAlias{"source-alias"};
const ProfileId kProfile{"destination-profile"};
const ProfileId kOtherProfile{"other-profile"};
const SpaceId kSpace{"destination-space"};
const ItemId kItem{"destination-item"};
const ItemId kFavorite{"destination-favorite"};
const ShortcutId kAlias{"destination-alias"};
const WindowId kWindow{"destination-window"};
const PageId kPage{"existing-page"};
const std::string kSaved = "https://example.test/saved";

portability::Workspace Source() {
  Model model;
  REQUIRE(model.AddProfile(kSourceProfile) == Status::kOk);
  REQUIRE(model.AddSpace(kSourceSpace, kSourceProfile) == Status::kOk);
  REQUIRE(model.AddItem(kSourceItem, kSourceSpace, kSaved) == Status::kOk);
  // Equal URLs are distinct explicit records, not duplicates.
  REQUIRE(model.AddItem(kSourceFavorite, kSourceProfile, kSaved) ==
          Status::kOk);
  REQUIRE(model.AddShortcut(kSourceAlias, kSourceProfile, kSourceItem) ==
          Status::kOk);
  auto projection = portability::Project(model);
  REQUIRE(std::holds_alternative<portability::Workspace>(projection));
  auto source = std::get<portability::Workspace>(projection);
  source.profiles[0].name = "Supplied group";
  source.spaces[0].name = "<script>inert label</script>";
  source.spaces[0].saved_at = "source timestamp";
  source.spaces[0].provenance = portability::Provenance{
      "synthetic-fixture", "original-space", std::nullopt};
  source.items[0].provenance =
      portability::Provenance{"synthetic-fixture", "original-space", 0};
  return source;
}

Mapping Mappings(Intent intent) {
  return {
      {{kSourceProfile, kProfile}},
      {{kSourceSpace, {kSpace, intent}}},
      {{kSourceItem, {kItem, intent}}, {kSourceFavorite, {kFavorite, intent}}},
      {{kSourceAlias, {kAlias, intent}}}};
}

Model DestinationModel(bool populated) {
  Model model;
  REQUIRE(model.AddProfile(kProfile) == Status::kOk);
  REQUIRE(model.AddProfile(kOtherProfile) == Status::kOk);
  if (populated) {
    REQUIRE(model.AddSpace(kSpace, kProfile) == Status::kOk);
    REQUIRE(model.AddItem(kItem, kSpace, kSaved) == Status::kOk);
    REQUIRE(model.AddItem(kFavorite, kProfile, kSaved) == Status::kOk);
    REQUIRE(model.AddShortcut(kAlias, kProfile, kItem) == Status::kOk);
    REQUIRE(model.AddWindow(kWindow) == Status::kOk);
    REQUIRE(model.Attach({kWindow, kSpace}) == Status::kOk);
    REQUIRE(model.SelectSpace(kWindow, kSpace) == Status::kOk);
    auto opened = model.OpenItem(kItem, {kWindow, kSpace}, kPage);
    REQUIRE(std::holds_alternative<OpenedPage>(opened));
    REQUIRE(model.RecordNavigation(
                kPage, "https://example.test/unsaved-work") == Status::kOk);
    REQUIRE(model.ReportActivity(kPage, Activity::kProtected) == Status::kOk);
  }
  return model;
}

std::string Bytes(const portability::Workspace& source) {
  auto result = portability::Encode(source);
  REQUIRE(std::holds_alternative<std::string>(result));
  return std::get<std::string>(result);
}

Manifest Preview(const portability::Workspace& source,
                 const Model& model,
                 const Mapping& mapping) {
  auto result = PreviewCopy(Bytes(source), model, mapping);
  REQUIRE(std::holds_alternative<Manifest>(result));
  return std::get<Manifest>(result);
}

template <typename Id, typename Payload>
const RecordPlan<Id, Payload>& Row(
    const std::vector<RecordPlan<Id, Payload>>& rows,
    const Id& id) {
  for (const auto& row : rows)
    if (row.source == id)
      return row;
  std::abort();
}

void RoundTripNewAndCancellation() {
  auto source = Source();
  auto model = DestinationModel(false);
  auto bytes = Bytes(source);
  const auto original_bytes = bytes;
  auto result = PreviewCopy(bytes, model, Mappings(Intent::kCreate));
  REQUIRE(std::holds_alternative<Manifest>(result));
  const auto& manifest = std::get<Manifest>(result);
  REQUIRE(!manifest.HasUnresolvedRecords());
  REQUIRE(manifest.profiles[0].outcome == Outcome::kMapped);
  REQUIRE(manifest.spaces[0].outcome == Outcome::kNew);
  REQUIRE(Row(manifest.items, kSourceItem).outcome == Outcome::kNew);
  REQUIRE(Row(manifest.items, kSourceFavorite).outcome == Outcome::kNew);
  REQUIRE(manifest.shortcuts[0].outcome == Outcome::kNew);
  REQUIRE(manifest.spaces[0].destination == kSpace);
  REQUIRE(manifest.spaces[0].proposed->profile == kProfile);
  REQUIRE(Row(manifest.items, kSourceItem).proposed->owner ==
          ItemOwner{kSpace});
  REQUIRE(Row(manifest.items, kSourceFavorite).proposed->owner ==
          ItemOwner{kProfile});
  REQUIRE(manifest.shortcuts[0].proposed->target == kItem);
  REQUIRE(Bytes(manifest.source) == original_bytes);
  REQUIRE(manifest.source.spaces[0].name == "<script>inert label</script>");
  bool name = false;
  bool timestamp = false;
  bool provenance = false;
  bool order = false;
  for (const auto& notice : manifest.unmapped_metadata) {
    name |= notice.field == UnmappedField::kName;
    timestamp |= notice.field == UnmappedField::kSavedAt;
    provenance |= notice.field == UnmappedField::kProvenance;
    order |= notice.field == UnmappedField::kRecordOrder;
  }
  REQUIRE(name && timestamp && provenance && order);
  // The manifest owns all input. Caller cancellation is just dropping it.
  bytes.clear();
  source.spaces.clear();
  REQUIRE(Bytes(manifest.source) == original_bytes);
  result =
      portability::Error{portability::ErrorCode::kMalformedEncoding, "test"};
  REQUIRE(model.spaces().empty());
  REQUIRE(model.items().empty());
  REQUIRE(model.shortcuts().empty());
  REQUIRE(model.pages().empty());
  REQUIRE(model.windows().empty());
}

void RepeatIdentityAndConflicts() {
  auto source = Source();
  auto model = DestinationModel(true);
  auto duplicate = Preview(source, model, Mappings(Intent::kMatchExisting));
  REQUIRE(!duplicate.HasUnresolvedRecords());
  REQUIRE(duplicate.spaces[0].outcome == Outcome::kDuplicate);
  REQUIRE(Row(duplicate.items, kSourceItem).outcome == Outcome::kDuplicate);
  REQUIRE(Row(duplicate.items, kSourceFavorite).outcome == Outcome::kDuplicate);
  REQUIRE(duplicate.shortcuts[0].outcome == Outcome::kDuplicate);

  auto collision = Preview(source, model, Mappings(Intent::kCreate));
  REQUIRE(collision.HasUnresolvedRecords());
  REQUIRE(collision.spaces[0].reason == Reason::kDestinationExists);
  REQUIRE(Row(collision.items, kSourceItem).reason ==
          Reason::kDependencyBlocked);
  REQUIRE(Row(collision.items, kSourceFavorite).reason ==
          Reason::kDestinationExists);

  for (auto& item : source.items)
    if (item.id == kSourceItem)
      item.saved_destination = "https://example.test/changed";
  auto conflict = Preview(source, model, Mappings(Intent::kMatchExisting));
  REQUIRE(Row(conflict.items, kSourceItem).outcome == Outcome::kConflict);
  REQUIRE(Row(conflict.items, kSourceItem).reason == Reason::kContentMismatch);
  REQUIRE(conflict.shortcuts[0].reason == Reason::kDependencyBlocked);
  REQUIRE(model.items().at(kItem).saved_destination == kSaved);
  REQUIRE(model.pages().size() == 1);
  REQUIRE(model.pages().at(kPage).current_navigation ==
          "https://example.test/unsaved-work");
  REQUIRE(model.pages().at(kPage).profile == kProfile);
  REQUIRE(model.pages().at(kPage).activity == Activity::kProtected);
  REQUIRE(model.pages().at(kPage).attachment() ==
          (Attachment{kWindow, kSpace}));
  REQUIRE(model.windows().at(kWindow).selected_space == kSpace);
}

void MappingFailuresAndProfileIsolation() {
  auto source = Source();
  auto model = DestinationModel(true);
  auto mapping = Mappings(Intent::kMatchExisting);
  mapping.profiles.clear();
  auto missing = Preview(source, model, mapping);
  REQUIRE(missing.HasUnresolvedRecords());
  REQUIRE(missing.profiles[0].reason == Reason::kMissingMapping);
  REQUIRE(missing.spaces[0].reason == Reason::kDependencyBlocked);

  mapping.profiles.emplace(kSourceProfile, ProfileId{"nonexistent"});
  REQUIRE(Preview(source, model, mapping).profiles[0].reason ==
          Reason::kDestinationMissing);
  mapping.profiles.at(kSourceProfile) = kOtherProfile;
  auto mismatch = Preview(source, model, mapping);
  REQUIRE(mismatch.spaces[0].reason == Reason::kProfileMismatch);
  REQUIRE(Row(mismatch.items, kSourceItem).reason ==
          Reason::kDependencyBlocked);
  REQUIRE(Row(mismatch.items, kSourceFavorite).reason ==
          Reason::kContentMismatch);
  REQUIRE(model.spaces().at(kSpace).profile == kProfile);

  mapping = Mappings(Intent::kMatchExisting);
  mapping.items.at(kSourceItem).id = ItemId{"absent"};
  REQUIRE(Row(Preview(source, model, mapping).items, kSourceItem).reason ==
          Reason::kDestinationMissing);
  mapping.items.at(kSourceItem).id = ItemId{""};
  REQUIRE(Row(Preview(source, model, mapping).items, kSourceItem).reason ==
          Reason::kEmptyDestinationId);
  mapping.items.erase(kSourceItem);
  REQUIRE(Row(Preview(source, model, mapping).items, kSourceItem).reason ==
          Reason::kMissingMapping);

  mapping = Mappings(Intent::kMatchExisting);
  mapping.items.at(kSourceFavorite).id = kItem;
  auto same_target = Preview(source, model, mapping);
  REQUIRE(Row(same_target.items, kSourceItem).reason ==
          Reason::kTargetCollision);
  REQUIRE(Row(same_target.items, kSourceFavorite).reason ==
          Reason::kTargetCollision);
  mapping.items.emplace(ItemId{"unknown-source"},
                        Destination<ItemId>{ItemId{"unused"}, Intent::kCreate});
  auto unknown = Preview(source, model, mapping);
  REQUIRE(unknown.mapping_problems.size() == 1);
  REQUIRE(unknown.mapping_problems[0].kind == RecordKind::kItem);
  REQUIRE(unknown.mapping_problems[0].reason == Reason::kUnknownSource);
}

void InvalidAndInterruptedCopies() {
  auto source = Source();
  auto bytes = Bytes(source);
  auto model = DestinationModel(true);
  const auto mapping = Mappings(Intent::kMatchExisting);
  // Every interrupted prefix of the actual codec output must fail atomically.
  for (std::size_t length = 0; length < bytes.size(); ++length) {
    REQUIRE(std::holds_alternative<portability::Error>(PreviewCopy(
        std::string_view(bytes).substr(0, length), model, mapping)));
  }
  REQUIRE(std::holds_alternative<portability::Error>(
      PreviewCopy(bytes + "trailing", model, mapping)));
  REQUIRE(std::holds_alternative<portability::Error>(
      PreviewCopy("not a workspace", model, mapping)));
  // Version fields are newline-delimited numbers after the documented header.
  const std::string header = "arcified-workspace\n";
  REQUIRE(bytes.compare(0, header.size(), header) == 0);
  const auto format_end = bytes.find('\n', header.size());
  REQUIRE(format_end != std::string::npos);
  auto unknown_version = bytes;
  unknown_version.replace(header.size(), format_end - header.size(),
                          std::to_string(portability::kFormatVersion + 1));
  REQUIRE(
      std::get<portability::Error>(PreviewCopy(unknown_version, model, mapping))
          .code == portability::ErrorCode::kUnsupportedFormatVersion);
  const auto contract_start = format_end + 1;
  const auto contract_end = bytes.find('\n', contract_start);
  REQUIRE(contract_end != std::string::npos);
  unknown_version = bytes;
  unknown_version.replace(contract_start, contract_end - contract_start, "0");
  REQUIRE(
      std::get<portability::Error>(PreviewCopy(unknown_version, model, mapping))
          .code == portability::ErrorCode::kUnsupportedContractVersion);
  source.format_version = portability::kFormatVersion + 1;
  auto invalid = Plan(source, model, mapping);
  REQUIRE(std::get<portability::Error>(invalid).code ==
          portability::ErrorCode::kUnsupportedFormatVersion);
  source = Source();
  source.lifecycle_contract_version = 0;
  REQUIRE(std::get<portability::Error>(Plan(source, model, mapping)).code ==
          portability::ErrorCode::kUnsupportedContractVersion);
  source = Source();
  source.items[0].saved_destination = "javascript:alert(1)";
  REQUIRE(std::get<portability::Error>(Plan(source, model, mapping)).code ==
          portability::ErrorCode::kInvalidDestination);
  source = Source();
  source.items[0].owner = SpaceId{"missing"};
  REQUIRE(std::get<portability::Error>(Plan(source, model, mapping)).code ==
          portability::ErrorCode::kMissingReference);
  REQUIRE(model.items().at(kItem).saved_destination == kSaved);
  REQUIRE(model.pages().at(kPage).activity == Activity::kProtected);
}

void StalePreviewIsNotReservation() {
  auto source = Source();
  auto model = DestinationModel(false);
  auto mapping = Mappings(Intent::kCreate);
  auto initial = Preview(source, model, mapping);
  REQUIRE(initial.spaces[0].outcome == Outcome::kNew);
  REQUIRE(model.AddSpace(kSpace, kProfile) == Status::kOk);
  auto refreshed = Preview(source, model, mapping);
  REQUIRE(initial.spaces[0].outcome == Outcome::kNew);
  REQUIRE(refreshed.spaces[0].reason == Reason::kDestinationExists);
  REQUIRE(model.items().empty());
}

void ConvertedSnapshotsRequireExplicitRepeatMappings() {
  const std::string original =
      R"({"spaces":[{"id":"one","name":"Same","urls":["https://example.test/a"],"savedAt":"saved"},{"id":"two","name":"Same","urls":["https://example.test/a"]}]})";
  auto converted = prototype_migration::Convert(original, "synthetic-source");
  REQUIRE(std::holds_alternative<prototype_migration::Converted>(converted));
  const auto& source =
      std::get<prototype_migration::Converted>(converted).workspace;
  REQUIRE(source.spaces.size() == 2);
  REQUIRE(source.items.size() == 2);
  REQUIRE(source.spaces[0].id != source.spaces[1].id);
  Model model = DestinationModel(false);
  Mapping mapping;
  mapping.profiles.emplace(source.profiles[0].id, kProfile);
  for (const auto& space : source.spaces) {
    mapping.spaces.emplace(
        space.id, Destination<SpaceId>{SpaceId{"mapped:" + space.id.value},
                                       Intent::kCreate});
  }
  for (const auto& item : source.items) {
    mapping.items.emplace(item.id,
                          Destination<ItemId>{ItemId{"mapped:" + item.id.value},
                                              Intent::kCreate});
  }
  auto fresh = Preview(source, model, mapping);
  REQUIRE(!fresh.HasUnresolvedRecords());
  for (const auto& item : fresh.items)
    REQUIRE(item.outcome == Outcome::kNew);
  // Fixture setup represents an earlier confirmed metadata import. This is
  // not a production Apply implementation or transaction/recovery proof.
  for (const auto& space : fresh.spaces) {
    REQUIRE(model.AddSpace(*space.destination, space.proposed->profile) ==
            Status::kOk);
  }
  for (const auto& item : fresh.items) {
    REQUIRE(model.AddItem(*item.destination, item.proposed->owner,
                          item.proposed->saved_destination) == Status::kOk);
  }
  for (auto& [id, destination] : mapping.spaces)
    destination.intent = Intent::kMatchExisting;
  for (auto& [id, destination] : mapping.items)
    destination.intent = Intent::kMatchExisting;
  auto repeated = Preview(source, model, mapping);
  REQUIRE(!repeated.HasUnresolvedRecords());
  for (const auto& item : repeated.items)
    REQUIRE(item.outcome == Outcome::kDuplicate);

  auto changed_json = original;
  const auto position = changed_json.find("https://example.test/a");
  REQUIRE(position != std::string::npos);
  changed_json.replace(position, std::string("https://example.test/a").size(),
                       "https://example.test/changed");
  auto changed = prototype_migration::Convert(changed_json, "synthetic-source");
  REQUIRE(std::holds_alternative<prototype_migration::Converted>(changed));
  const auto& changed_source =
      std::get<prototype_migration::Converted>(changed).workspace;
  REQUIRE(changed_source.items[0].id == source.items[0].id);
  const auto conflict = Preview(changed_source, model, mapping);
  REQUIRE(conflict.items[0].outcome == Outcome::kConflict);
  REQUIRE(conflict.items[1].outcome == Outcome::kDuplicate);
  REQUIRE(model.items().at(*fresh.items[0].destination).saved_destination ==
          "https://example.test/a");
  REQUIRE(model.pages().empty());
}

}  // namespace
}  // namespace arcified::workspace_import

int main() {
  using namespace arcified::workspace_import;
  RoundTripNewAndCancellation();
  RepeatIdentityAndConflicts();
  MappingFailuresAndProfileIsolation();
  InvalidAndInterruptedCopies();
  StalePreviewIsNotReservation();
  ConvertedSnapshotsRequireExplicitRepeatMappings();
  std::cout << "workspace import planner checks passed\n";
}
