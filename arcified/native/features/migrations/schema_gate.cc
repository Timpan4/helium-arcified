#include "schema_gate.h"

#include <utility>
#include <variant>

namespace arcified::migrations {
namespace {

ImageInspection InspectImage(const std::optional<std::string>& bytes) {
  if (!bytes)
    return {ImageState::kAbsent, std::nullopt, std::nullopt};
  const auto probe = durability::ProbeEnvelope(*bytes);
  if (const auto* failure = std::get_if<durability::Failure>(&probe))
    return {ImageState::kCorrupt, std::nullopt, *failure};
  const auto version = std::get<durability::EnvelopeVersion>(probe);
  bool supported = false;
  for (const auto& format : kSupportedFormats) {
    if (format.schema_version == version.schema_version &&
        format.contract_version == version.contract_version) {
      supported = true;
      break;
    }
  }
  if (!supported) {
    if (version.schema_version != durability::kSchemaVersion) {
      return {version.schema_version > durability::kSchemaVersion
                  ? ImageState::kFutureSchema
                  : ImageState::kUnsupportedSchema,
              version,
              durability::Failure{durability::Error::kUnsupportedSchema,
                                  "unregistered workspace schema"}};
    }
    return {ImageState::kUnsupportedContract, version,
            durability::Failure{durability::Error::kUnsupportedContract,
                                "unregistered lifecycle contract"}};
  }
  const auto decoded = durability::DecodeSnapshot(*bytes);
  if (const auto* failure = std::get_if<durability::Failure>(&decoded))
    return {ImageState::kCorrupt, version, *failure};
  return {ImageState::kCurrent, version, std::nullopt};
}

bool Unsupported(ImageState state) {
  return state == ImageState::kFutureSchema ||
         state == ImageState::kUnsupportedSchema ||
         state == ImageState::kUnsupportedContract;
}

}  // namespace

CompatibilityGate::CompatibilityGate(durability::StoreImage source)
    : source_(std::move(source)),
      original_(InspectImage(source_.original)),
      candidate_(InspectImage(source_.candidate)) {}

const ImageInspection& CompatibilityGate::inspection(ImageSource source) const {
  return source == ImageSource::kOriginal ? original_ : candidate_;
}

std::vector<ImageSource> CompatibilityGate::recovery_choices() const {
  std::vector<ImageSource> choices;
  if (original_.state == ImageState::kCurrent)
    choices.push_back(ImageSource::kOriginal);
  if (candidate_.state == ImageState::kCurrent)
    choices.push_back(ImageSource::kCandidate);
  return choices;
}

durability::Result<durability::DurableSnapshot> CompatibilityGate::ReadSnapshot(
    ImageSource source) const {
  const auto& bytes =
      source == ImageSource::kOriginal ? source_.original : source_.candidate;
  if (!bytes)
    return durability::Failure{durability::Error::kRecoveryRequired,
                               "requested workspace image is absent"};
  return durability::DecodeSnapshot(*bytes);
}

durability::Result<durability::PreparedWrite> CompatibilityGate::PrepareWrite(
    const durability::StoreImage& current,
    const durability::DurableSnapshot& snapshot,
    const durability::SnapshotAdmission& admission) const {
  if (!(current == source_))
    return durability::Failure{durability::Error::kStaleSource,
                               "workspace images changed after inspection"};
  // Check both slots before #50 recovery/write admission. A future candidate
  // must not be hidden by a readable older original, or vice versa.
  if (Unsupported(original_.state))
    return *original_.failure;
  if (Unsupported(candidate_.state))
    return *candidate_.failure;
  return durability::PrepareWrite(source_, snapshot, admission);
}

}  // namespace arcified::migrations
