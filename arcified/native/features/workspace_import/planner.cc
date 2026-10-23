#include "planner.h"

#include <algorithm>
#include <set>
#include <type_traits>

namespace arcified::workspace_import {
namespace {

bool Resolved(Outcome outcome) {
  return outcome == Outcome::kMapped || outcome == Outcome::kNew ||
         outcome == Outcome::kDuplicate;
}

template <typename Rows, typename Id>
const typename Rows::value_type* Find(const Rows& rows, const Id& id) {
  auto row = std::find_if(rows.begin(), rows.end(),
                          [&](const auto& row) { return row.source == id; });
  return row == rows.end() ? nullptr : &*row;
}

template <typename Records, typename Mappings>
void CheckUnknownMappings(const Records& records,
                          const Mappings& mappings,
                          RecordKind kind,
                          Manifest& manifest) {
  for (const auto& entry : mappings) {
    const auto& id = entry.first;
    if (std::none_of(records.begin(), records.end(),
                     [&](const auto& record) { return record.id == id; })) {
      manifest.mapping_problems.push_back({kind, Reason::kUnknownSource});
    }
  }
}

template <typename Mappings>
auto Collisions(const Mappings& mappings) {
  using Id = decltype(std::declval<typename Mappings::mapped_type>().id);
  std::set<Id> seen;
  std::set<Id> collisions;
  for (const auto& [source, destination] : mappings) {
    if (!seen.insert(destination.id).second)
      collisions.insert(destination.id);
  }
  return collisions;
}

template <typename Row, typename Choice, typename Existing, typename Equal>
void Classify(Row& row,
              const Choice& choice,
              const Existing& existing,
              bool collision,
              Equal equal) {
  row.destination = choice.id;
  row.outcome = Outcome::kConflict;
  if (choice.id.value.empty()) {
    row.reason = Reason::kEmptyDestinationId;
  } else if (collision) {
    row.reason = Reason::kTargetCollision;
  } else {
    auto found = existing.find(choice.id);
    if (choice.intent == Intent::kCreate) {
      row.reason =
          found == existing.end() ? Reason::kNone : Reason::kDestinationExists;
      if (found == existing.end())
        row.outcome = Outcome::kNew;
    } else if (found == existing.end()) {
      row.reason = Reason::kDestinationMissing;
    } else {
      row.reason = equal(found->second);
      if (row.reason == Reason::kNone)
        row.outcome = Outcome::kDuplicate;
    }
  }
}

template <typename Records>
void Metadata(const Records& records, RecordKind kind, Manifest& manifest) {
  if (!records.empty()) {
    manifest.unmapped_metadata.push_back(
        {kind, std::nullopt, UnmappedField::kRecordOrder});
  }
  for (std::size_t i = 0; i < records.size(); ++i) {
    if constexpr (!std::is_same_v<typename Records::value_type,
                                  portability::Shortcut>) {
      if (records[i].name)
        manifest.unmapped_metadata.push_back({kind, i, UnmappedField::kName});
    }
    if constexpr (std::is_same_v<typename Records::value_type,
                                 portability::Space>) {
      if (records[i].saved_at) {
        manifest.unmapped_metadata.push_back(
            {kind, i, UnmappedField::kSavedAt});
      }
    }
    if (records[i].provenance) {
      manifest.unmapped_metadata.push_back(
          {kind, i, UnmappedField::kProvenance});
    }
  }
}

}  // namespace

bool Manifest::HasUnresolvedRecords() const {
  const auto unresolved = [](const auto& rows) {
    return std::any_of(rows.begin(), rows.end(),
                       [](const auto& row) { return !Resolved(row.outcome); });
  };
  return !mapping_problems.empty() || unresolved(profiles) ||
         unresolved(spaces) || unresolved(items) || unresolved(shortcuts);
}

PlanResult Plan(const portability::Workspace& source,
                const lifecycle::Model& destination,
                const Mapping& mapping) {
  if (auto error = portability::Validate(source))
    return *error;
  Manifest manifest;
  manifest.source = source;
  CheckUnknownMappings(source.profiles, mapping.profiles, RecordKind::kProfile,
                       manifest);
  CheckUnknownMappings(source.spaces, mapping.spaces, RecordKind::kSpace,
                       manifest);
  CheckUnknownMappings(source.items, mapping.items, RecordKind::kItem,
                       manifest);
  CheckUnknownMappings(source.shortcuts, mapping.shortcuts,
                       RecordKind::kShortcut, manifest);
  Metadata(source.profiles, RecordKind::kProfile, manifest);
  Metadata(source.spaces, RecordKind::kSpace, manifest);
  Metadata(source.items, RecordKind::kItem, manifest);
  Metadata(source.shortcuts, RecordKind::kShortcut, manifest);

  for (const auto& profile : source.profiles) {
    ProfilePlan row{profile.id, std::nullopt, std::nullopt, Outcome::kUnmapped,
                    Reason::kMissingMapping};
    auto found = mapping.profiles.find(profile.id);
    if (found != mapping.profiles.end()) {
      row.destination = found->second;
      if (found->second.value.empty()) {
        row.reason = Reason::kEmptyDestinationId;
      } else if (!destination.profiles().count(found->second)) {
        row.reason = Reason::kDestinationMissing;
      } else {
        row.proposed = lifecycle::Profile{};
        row.outcome = Outcome::kMapped;
        row.reason = Reason::kNone;
      }
    }
    manifest.profiles.push_back(std::move(row));
  }

  const auto space_collisions = Collisions(mapping.spaces);
  for (const auto& space : source.spaces) {
    SpacePlan row{space.id, std::nullopt, std::nullopt, Outcome::kUnmapped,
                  Reason::kMissingMapping};
    auto found = mapping.spaces.find(space.id);
    if (found != mapping.spaces.end()) {
      row.destination = found->second.id;
      const auto* profile = Find(manifest.profiles, space.profile);
      if (!profile || !Resolved(profile->outcome)) {
        row.reason = Reason::kDependencyBlocked;
      } else {
        row.proposed = lifecycle::Space{*profile->destination};
        Classify(row, found->second, destination.spaces(),
                 space_collisions.count(found->second.id),
                 [&](const auto& old) {
                   return old.profile == row.proposed->profile
                              ? Reason::kNone
                              : Reason::kProfileMismatch;
                 });
      }
    }
    manifest.spaces.push_back(std::move(row));
  }

  const auto item_collisions = Collisions(mapping.items);
  for (const auto& item : source.items) {
    ItemPlan row{item.id, std::nullopt, std::nullopt, Outcome::kUnmapped,
                 Reason::kMissingMapping};
    auto found = mapping.items.find(item.id);
    if (found != mapping.items.end()) {
      row.destination = found->second.id;
      std::optional<lifecycle::ItemOwner> owner;
      std::visit(
          [&](const auto& id) {
            using Id = std::decay_t<decltype(id)>;
            if constexpr (std::is_same_v<Id, lifecycle::SpaceId>) {
              const auto* parent = Find(manifest.spaces, id);
              if (parent && Resolved(parent->outcome))
                owner = *parent->destination;
            } else {
              const auto* parent = Find(manifest.profiles, id);
              if (parent && Resolved(parent->outcome))
                owner = *parent->destination;
            }
          },
          item.owner);
      if (!owner) {
        row.reason = Reason::kDependencyBlocked;
      } else {
        row.proposed =
            lifecycle::PersistentItem{*owner, item.saved_destination};
        Classify(row, found->second, destination.items(),
                 item_collisions.count(found->second.id), [&](const auto& old) {
                   return old.owner == row.proposed->owner &&
                                  old.saved_destination ==
                                      item.saved_destination
                              ? Reason::kNone
                              : Reason::kContentMismatch;
                 });
      }
    }
    manifest.items.push_back(std::move(row));
  }

  const auto shortcut_collisions = Collisions(mapping.shortcuts);
  for (const auto& shortcut : source.shortcuts) {
    ShortcutPlan row{shortcut.id, std::nullopt, std::nullopt,
                     Outcome::kUnmapped, Reason::kMissingMapping};
    auto found = mapping.shortcuts.find(shortcut.id);
    if (found != mapping.shortcuts.end()) {
      row.destination = found->second.id;
      const auto* profile = Find(manifest.profiles, shortcut.profile);
      const auto* target = Find(manifest.items, shortcut.target);
      if (!profile || !Resolved(profile->outcome) || !target ||
          !Resolved(target->outcome)) {
        row.reason = Reason::kDependencyBlocked;
      } else {
        row.proposed =
            lifecycle::Shortcut{*profile->destination, *target->destination};
        Classify(row, found->second, destination.shortcuts(),
                 shortcut_collisions.count(found->second.id),
                 [&](const auto& old) {
                   return old.profile == row.proposed->profile &&
                                  old.target == row.proposed->target
                              ? Reason::kNone
                              : Reason::kContentMismatch;
                 });
      }
    }
    manifest.shortcuts.push_back(std::move(row));
  }
  return manifest;
}

PlanResult PreviewCopy(std::string_view bytes,
                       const lifecycle::Model& destination,
                       const Mapping& mapping) {
  auto decoded = portability::Decode(bytes);
  if (const auto* error = std::get_if<portability::Error>(&decoded))
    return *error;
  return Plan(std::get<portability::Workspace>(decoded), destination, mapping);
}

}  // namespace arcified::workspace_import
