#ifndef ARCIFIED_NATIVE_INTEGRATION_PERSISTENCE_CONTEXT_ADMISSION_H_
#define ARCIFIED_NATIVE_INTEGRATION_PERSISTENCE_CONTEXT_ADMISSION_H_

#include <optional>
#include <vector>

#include "../../features/durability/session_snapshot.h"
#include "../../features/profiles/preparation.h"

namespace arcified::persistence {

struct ContextObservation {
  profiles::ProfileBinding binding;
  private_browsing::NativeContextFacts facts;
};

struct PageObservation {
  lifecycle::Attachment attachment;
  ContextObservation context;
};

enum class PayloadPresence { kUnknown, kAbsent, kPresent };

// Reports from the actual peer feature stores, over the whole operation scope.
// Model inspection cannot establish any of these absences.
struct UnsupportedPeerPayloads {
  PayloadPresence folders = PayloadPresence::kUnknown;
  // Custom theme/palette payload beyond Space DisplayIdentity::theme_key.
  PayloadPresence themes = PayloadPresence::kUnknown;
  PayloadPresence split_presentation = PayloadPresence::kUnknown;
  PayloadPresence other = PayloadPresence::kUnknown;
};

// Implemented by the native host's existing live binding/freshness authority.
// Reads must describe one coherent observation on its serialized UI sequence.
// This interface owns no registry, native identity, keepalive or classifier.
class NativeContextSource {
 public:
  virtual ~NativeContextSource() = default;
  virtual std::optional<ContextObservation> ReadProfile(
      const lifecycle::ProfileId& profile) const = 0;
  virtual std::optional<PageObservation> ReadPage(
      const lifecycle::PageId& page) const = 0;
  // Report ALL exact native contexts bound to this logical window, including
  // contexts outside the selected Space. An empty or unknown report refuses.
  // Multi-Profile windows must report each context; no selected Browser Profile
  // or original-Profile substitution is permitted.
  virtual std::optional<std::vector<ContextObservation>> ReadWindow(
      const lifecycle::WindowId& window) const = 0;
  virtual UnsupportedPeerPayloads ReadPeerPayloads() const = 0;
};

// Stateless composition of profiles preparation (and its shared classifier)
// with durability admission. Every registered Profile, Page and Window is
// checked, including Profiles without attachments and Windows without pages.
// Distinct lifecycle Profiles cannot claim one exact native context identity.
// No runtime identity is added to the snapshot or codec.
//
// The returned callbacks BORROW model, metadata and source. Use immediately
// within the same serialized native operation, keeping all three alive. They
// re-read facts and never cache a permit. Whole-snapshot admission compares the
// complete durable graph and exact ordered metadata, then current native/peer
// stores, even for empty or Window-only graphs. Bind the operation's graph,
// retain native objects and prevent changes/reentrancy until use completes. The
// result is not a freshness token or native authorization. Recompose for each
// operation and supply current admission to CommitPreparedWrite before native
// IO commit.
durability::Result<durability::SnapshotAdmission> ComposeSnapshotAdmission(
    const lifecycle::Model& model,
    const space_management::Metadata& metadata,
    const NativeContextSource& source);

}  // namespace arcified::persistence

#endif  // ARCIFIED_NATIVE_INTEGRATION_PERSISTENCE_CONTEXT_ADMISSION_H_
