#ifndef ARCIFIED_NATIVE_FEATURES_SITE_CONTROLS_COMMAND_TARGET_H_
#define ARCIFIED_NATIVE_FEATURES_SITE_CONTROLS_COMMAND_TARGET_H_

#include <optional>
#include <set>
#include <variant>

#include "arcified/native/features/commands/command_target.h"

namespace arcified::site_controls {

// Host-issued, non-reused identities. These are neither URLs nor display text.
// The native host retains the corresponding objects and authoritative origins.
using DocumentId = lifecycle::Id<struct DocumentTag>;
using OriginId = lifecycle::Id<struct OriginTag>;
using SecurityStateId = lifecycle::Id<struct SecurityStateTag>;
using SurfaceId = lifecycle::Id<struct SurfaceTag>;
using SelectionId = lifecycle::Id<struct SelectionTag>;
using AnchorId = lifecycle::Id<struct AnchorTag>;

// Deliberately bounded to the native paths inspected for this slice. These
// values do not assert that the host/platform currently supports a command.
enum class Command { kPageInfo, kSiteSettings, kPrint };

// An ephemeral identity snapshot, never an owner or a stored active-page
// record.
struct Target {
  commands::PageCommandTarget page_target;
  DocumentId document;
  OriginId origin;
  SecurityStateId security_state;
  SurfaceId surface;
  SelectionId selection;
};

// Fresh authoritative native facts for the invoking window, supplied on every
// call. A host must not construct selected from Model selection or URL strings.
// Available commands reflect native policy/build/controller readiness for this
// exact selected target. An absent entry refuses rather than inventing support.
struct HostContext {
  std::optional<Target> selected;
  std::set<Command> available_commands;
  // Page Info requires a live native anchor configuration, either a view or
  // fallback rectangle plus native parent. The host owns geometry/lifetimes.
  std::optional<AnchorId> page_info_anchor;
};

enum class Refusal {
  kUnsupportedCommand,
  kNativeSelectionUnavailable,
  kInvalidNativeIdentity,
  kInvalidLifecycleIdentity,
  kPageNotFound,
  kProfileMismatch,
  kAttachmentNotFound,
  kAttachmentChanged,
  kSpaceNotSelected,
  kPageNotSelected,
  kDocumentChanged,
  kOriginChanged,
  kSecurityStateChanged,
  kSurfaceChanged,
  kSelectionChanged,
  kCommandUnavailable,
  kAnchorUnavailable,
  kAnchorChanged,
};

// Captured for a menu/action only. This is not an authorization, reservation,
// native pointer, or completion token. Never persist it or reuse it after a
// refusal. Revalidate immediately before dispatch through the native host.
struct CommandTarget {
  Command command;
  Target target;
  std::optional<AnchorId> anchor;
};
using PrepareResult = std::variant<Refusal, CommandTarget>;

PrepareResult PrepareCommand(const lifecycle::Model& model,
                             Command command,
                             const HostContext& host);

// nullopt means only that the supplied facts agree now. The caller serializes
// validation and native actuation, retaining all native objects through
// dispatch. Native success/failure and bubble lifetime remain the host's
// responsibility.
std::optional<Refusal> RevalidateCommand(const lifecycle::Model& model,
                                         const CommandTarget& request,
                                         const HostContext& host);

}  // namespace arcified::site_controls

#endif  // ARCIFIED_NATIVE_FEATURES_SITE_CONTROLS_COMMAND_TARGET_H_
