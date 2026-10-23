#ifndef ARCIFIED_NATIVE_FEATURES_DURABILITY_SESSION_SNAPSHOT_H_
#define ARCIFIED_NATIVE_FEATURES_DURABILITY_SESSION_SNAPSHOT_H_

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::durability {

inline constexpr std::uint32_t kSchemaVersion = 1;

enum class Error {
  kMalformed,
  kUnsupportedSchema,
  kUnsupportedContract,
  kInvalidGraph,
  kNavigationEvidenceRequired,
  kRecoveryRequired,
  kStaleSource,
  kPersistenceNotAdmitted,
  kUnsupportedFeatures,
};
struct Failure {
  Error code;
  std::string context;
};
template <typename T>
using Result = std::variant<T, Failure>;

struct EnvelopeVersion {
  std::uint32_t schema_version;
  std::uint32_t contract_version;
};

// A dormant identity/ownership record. Native session machinery remains the
// authority for navigation history; renderer memory and activity are absent.
struct PageRecord {
  lifecycle::PageId id;
  lifecycle::ProfileId profile;
  lifecycle::PageOwner owner;
};
struct DurableSnapshot {
  std::uint32_t schema_version = kSchemaVersion;
  std::uint32_t contract_version = lifecycle::kContractVersion;
  std::vector<lifecycle::ProfileId> profiles;
  std::vector<std::pair<lifecycle::SpaceId, lifecycle::Space>> spaces;
  std::vector<std::pair<lifecycle::WindowId, lifecycle::Window>> windows;
  std::vector<lifecycle::Attachment> attachments;
  std::vector<std::pair<lifecycle::ItemId, lifecycle::PersistentItem>> items;
  std::vector<std::pair<lifecycle::ShortcutId, lifecycle::Shortcut>> shortcuts;
  std::vector<PageRecord> pages;
  std::vector<std::pair<lifecycle::SplitId, lifecycle::Split>> splits;
};

// Root integration binds these checks to the shared private_browsing context
// classifier. This module has no second classifier. Missing checks fail closed;
// every Profile and every actual page context must explicitly be durable.
// No native context identity or classification is serialized.
struct SnapshotAdmission {
  std::function<bool(const lifecycle::ProfileId&)> profile_may_persist;
  std::function<bool(const lifecycle::PageId&, const lifecycle::ProfileId&)>
      page_may_persist;
  // Schema 1 has no Space metadata or split presentation payload codec. The
  // caller must inspect its feature stores and attest those payloads absent.
  bool unsupported_feature_payloads_absent = false;
};
Result<DurableSnapshot> CaptureSnapshot(const lifecycle::Model& model,
                                        const SnapshotAdmission& admission);
std::optional<Failure> ValidateSnapshot(const DurableSnapshot& snapshot);
// Probe reads the fixed envelope only. It does not validate the body/checksum.
Result<EnvelopeVersion> ProbeEnvelope(std::string_view bytes);
Result<std::string> EncodeSnapshot(const DurableSnapshot& snapshot,
                                   const SnapshotAdmission& admission);
Result<DurableSnapshot> DecodeSnapshot(std::string_view bytes);

// Reports already restored, browser-policy-validated native commits. This API
// never loads a URL or submits a form. Require every page exactly once, with
// matching immutable Profile; construct a fresh model, with activity unknown.
struct RestoredNavigation {
  lifecycle::PageId page;
  lifecycle::ProfileId profile;
  std::string committed_destination;
};
Result<lifecycle::Model> RestoreSnapshot(
    const DurableSnapshot& snapshot,
    const std::vector<RestoredNavigation>& navigation,
    const SnapshotAdmission& admission);

// Byte images, not a filesystem adapter. The original is the commit authority;
// candidate presence records an unfinished write, never a newer committed
// state.
struct StoreImage {
  std::optional<std::string> original;
  std::optional<std::string> candidate;
  friend bool operator==(const StoreImage& a, const StoreImage& b) {
    return a.original == b.original && a.candidate == b.candidate;
  }
};
enum class RecoveryNotice {
  kInterruptedCandidateRetained,
  kNavigationAndRendererMemoryNotRestored,
};
struct Recovery {
  DurableSnapshot snapshot;
  std::vector<RecoveryNotice> notices;
};
Result<Recovery> Recover(const StoreImage& image);

// Preparation retains exact source bytes. Persist and verify staged_image()
// before the eventual native adapter may commit. No IO/flush/rename guarantee
// follows from these in-memory transitions. A failed operation changes nothing.
class PreparedWrite {
 public:
  const StoreImage& source() const { return source_; }
  const StoreImage& staged_image() const { return staged_; }

 private:
  PreparedWrite(StoreImage source, StoreImage staged)
      : source_(std::move(source)), staged_(std::move(staged)) {}
  StoreImage source_;
  StoreImage staged_;
  friend Result<PreparedWrite> PrepareWrite(StoreImage,
                                            const DurableSnapshot&,
                                            const SnapshotAdmission&);
};
Result<PreparedWrite> PrepareWrite(StoreImage source,
                                   const DurableSnapshot& snapshot,
                                   const SnapshotAdmission& admission);
Result<StoreImage> CommitPreparedWrite(const StoreImage& current_staged,
                                       const PreparedWrite& prepared);

}  // namespace arcified::durability

#endif  // ARCIFIED_NATIVE_FEATURES_DURABILITY_SESSION_SNAPSHOT_H_
