#ifndef ARCIFIED_NATIVE_FEATURES_DEVELOPER_MODE_DEVTOOLS_SESSION_H_
#define ARCIFIED_NATIVE_FEATURES_DEVELOPER_MODE_DEVTOOLS_SESSION_H_

#include <memory>
#include <optional>
#include <utility>
#include <variant>

#include "arcified/native/features/commands/command_target.h"

namespace arcified::developer_mode {

// Opaque, nonempty, non-reused identities supplied by the native host. These
// are neither addresses nor URLs. Replacement WebContents gets a new instance;
// each committed main-frame document gets a new document identity.
using NativeInstanceId = lifecycle::Id<struct NativeInstanceTag>;
using DocumentId = lifecycle::Id<struct DocumentTag>;
// Host generation for the page's native attachment binding. Rotate on every
// rebind/transfer, including transfer away and back without replacing contents.
using BindingId = lifecycle::Id<struct BindingTag>;

// A DevTools binding snapshot, never another page/attachment owner. In a split,
// the caller supplies the exact inspected pane's page, not the selected tab.
struct Target {
  commands::PageCommandTarget page;
  NativeInstanceId instance;
  DocumentId document;
  BindingId binding;
};

// Subscription provenance, NOT a native binding generation. Copy the token
// supplied to Subscribe into every event. Only a new session can mint one.
class UpdateToken {
 public:
  friend bool operator==(const UpdateToken& a, const UpdateToken& b) {
    return a.identity_ == b.identity_;
  }

 private:
  friend class DevToolsSession;
  struct Identity {};
  explicit UpdateToken(std::shared_ptr<const Identity> identity)
      : identity_(std::move(identity)) {}
  std::shared_ptr<const Identity> identity_;
};

struct DiagnosticEvent {
  Target target;
  UpdateToken subscription;
};

// Fresh authoritative native facts for this operation. Model selection and URL
// text cannot supply native focus, document identity or inspection permission.
struct Observation {
  Target target;
  bool inspection_allowed;
  bool tools_visible;
};

enum class Check {
  kAccepted,
  kInvalidNativeIdentity,
  kInvalidPageTarget,
  kPageNotFound,
  kWrongPage,
  kWrongProfile,
  kWrongAttachment,
  kWrongNativeInstance,
  kWrongDocument,
  kWrongBinding,
  kWrongSubscription,
  kInspectionDenied,
  kToolsHidden,
  kSubscriptionFailed,
  kSessionEnded,
};

Check ValidateTarget(const lifecycle::Model& model,
                     const Target& requested,
                     const Observation& observed);

enum class EndReason {
  kClosed,
  kHidden,
  kNavigationCommitted,
  kNativeTargetReplaced,
  kTargetInvalidated,
  kSessionDestroyed,
};

enum class Cleanup { kConfirmedStopped, kUnconfirmed };

// Area-specific diagnostic update work, not the DevTools frontend, target
// registry or page lifetime. Cancel must not throw. kConfirmedStopped means the
// host has stopped this subscription's work; queued events can still arrive.
// On kUnconfirmed the host retains the native cleanup obligation independently
// of this handle's lifetime. Destroying a handle is not a cancellation proof.
class DiagnosticSubscription {
 public:
  virtual ~DiagnosticSubscription() = default;
  virtual Cleanup Cancel(EndReason reason) noexcept = 0;
};

class DiagnosticSource {
 public:
  virtual ~DiagnosticSource() = default;
  // Subscribe only to this exact target. Return null on failure, leaving no
  // subscription work behind. On the UI sequence, do not reenter lifecycle or
  // target mutations or deliver callbacks before this method returns.
  virtual std::unique_ptr<DiagnosticSubscription> Subscribe(
      const Target& target,
      UpdateToken token) = 0;
};

// Single-UI-sequence, terminal session for one exact target. Reopening requires
// a new session. No native frontend is opened, closed, detached or focused
// here. The host MUST End the old session before reporting every main-frame
// commit (including same-URL/same-document commits), binding invalidation or
// tools hide/close. Model cannot detect navigation or binding ABA if the host
// omits that notification and supplies stale native facts. See README.md.
class DevToolsSession {
 public:
  using StartResult = std::variant<Check, std::unique_ptr<DevToolsSession>>;
  static StartResult Start(const lifecycle::Model& model,
                           Target target,
                           const Observation& observed,
                           DiagnosticSource& source);
  ~DevToolsSession();
  DevToolsSession(const DevToolsSession&) = delete;
  DevToolsSession& operator=(const DevToolsSession&) = delete;

  // Call before consuming each diagnostic event, with its immutable stamped
  // target/token AND separately obtained current native facts. Never relabel an
  // old event with the current target's identities. A binding, capability or
  // visibility mismatch ends this session and requests
  // cancellation exactly once. An unrelated subscription's event is rejected
  // without ending this session.
  Check AcceptUpdate(const lifecycle::Model& model,
                     const DiagnosticEvent& event,
                     const Observation& observed);

  // Invalidates acceptance BEFORE invoking host cancellation. Idempotent,
  // including synchronous reentry from Cancel. This acknowledges only the
  // diagnostic subscription, not native DevTools before-unload/close success.
  Cleanup End(EndReason reason);
  std::optional<Cleanup> cleanup() const { return cleanup_; }

 private:
  explicit DevToolsSession(Target target);

  const Target target_;
  const UpdateToken token_;
  std::unique_ptr<DiagnosticSubscription> subscription_;
  std::optional<Cleanup> cleanup_;
};

}  // namespace arcified::developer_mode

#endif  // ARCIFIED_NATIVE_FEATURES_DEVELOPER_MODE_DEVTOOLS_SESSION_H_
