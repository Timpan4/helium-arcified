#ifndef ARCIFIED_NATIVE_FEATURES_MIGRATIONS_SCHEMA_GATE_H_
#define ARCIFIED_NATIVE_FEATURES_MIGRATIONS_SCHEMA_GATE_H_

#include <array>
#include <optional>
#include <string>
#include <vector>

#include "../durability/session_snapshot.h"

namespace arcified::migrations {

// Only actual formats belong here. Semantic lifecycle revisions are not
// historical disk schemas, and no predecessor transform is registered.
inline constexpr std::array<durability::EnvelopeVersion, 1> kSupportedFormats{{
    {durability::kSchemaVersion, lifecycle::kContractVersion},
}};

enum class ImageSource { kOriginal, kCandidate };
enum class ImageState {
  kAbsent,
  kCurrent,
  kFutureSchema,
  kUnsupportedSchema,
  kUnsupportedContract,
  kCorrupt,
};
struct ImageInspection {
  ImageState state;
  std::optional<durability::EnvelopeVersion> version;
  std::optional<durability::Failure> failure;
};

// Immutable inspection of exact byte images, not another workspace model or
// filesystem transaction. Caller serializes writers across inspection, staging
// and commit. A valid candidate is an explicit recovery choice, never a commit.
class CompatibilityGate {
 public:
  explicit CompatibilityGate(durability::StoreImage source);

  const durability::StoreImage& source() const { return source_; }
  const ImageInspection& inspection(ImageSource source) const;
  std::vector<ImageSource> recovery_choices() const;
  durability::Result<durability::DurableSnapshot> ReadSnapshot(
      ImageSource source) const;

  // Refuses unsupported input in either slot, changed source bytes and all
  // unresolved recovery states. Delegates staging/validation to #50. Successful
  // inspection alone never grants a native IO writer permission to replace
  // data.
  durability::Result<durability::PreparedWrite> PrepareWrite(
      const durability::StoreImage& current,
      const durability::DurableSnapshot& snapshot,
      const durability::SnapshotAdmission& admission) const;

 private:
  durability::StoreImage source_;
  ImageInspection original_;
  ImageInspection candidate_;
};

}  // namespace arcified::migrations

#endif  // ARCIFIED_NATIVE_FEATURES_MIGRATIONS_SCHEMA_GATE_H_
