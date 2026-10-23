#ifndef ARCIFIED_NATIVE_FEATURES_WORKSPACE_ORGANIZER_INSPECTION_H_
#define ARCIFIED_NATIVE_FEATURES_WORKSPACE_ORGANIZER_INSPECTION_H_

#include <optional>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::workspace_organizer {

namespace lc = lifecycle;

// Only opaque identities leave this boundary. In particular, do not add whole
// PersistentItem/LivePage objects, destinations, credentials or display labels.
using Identity = std::variant<lc::ProfileId,
                              lc::SpaceId,
                              lc::WindowId,
                              lc::Attachment,
                              lc::ItemId,
                              lc::ShortcutId,
                              lc::PageId,
                              lc::TemporaryTabId,
                              lc::SplitId>;

enum class Relation {
  kSpaceProfile,
  kAttachmentWindow,
  kAttachmentSpace,
  kSelectedSpace,
  kItemOwner,
  kShortcutProfile,
  kShortcutTarget,
  kPageProfile,
  kPageOwner,
  kPageAttachment,
  kTemporaryOrigin,
  kSplitAttachment,
  kSplitPage,
};

struct Reference {
  Identity source;
  Relation relation;
  Identity target;
};

// Disposable advisory projection, not a persistence schema, repair authority or
// reservation. Reinspect the Model after mutations, on its single-writer
// sequence. Iteration order is not user sidebar order. Missing targets diagnose
// identity references only, not arbitrary corruption or native registry state.
struct Inspection {
  std::vector<Identity> identities;
  std::vector<Reference> references;

  bool Contains(const Identity& identity) const;
  std::vector<Reference> ReferencesTo(const Identity& identity) const;
  std::vector<Reference> MissingTargets() const;
  // No window attachment, not a lost/deleted Space or an orphaned item.
  std::vector<lc::SpaceId> UnattachedSpaces() const;
  // No incoming Model references, not permission to delete Profile data.
  std::vector<lc::ProfileId> UnreferencedProfiles() const;
};

Inspection Inspect(const lc::Model& model);

// Absence of modeled references says nothing about folders, routing, on-disk
// Profile data or other external users. These are always uninspected here.
enum class ExternalReferences { kUninspected };
enum class ReferenceState {
  kIdentityNotFound,
  kReferenced,
  kNoModelReferences
};
struct ProfileDeletionPreview {
  ReferenceState state;
  std::vector<Reference> references;
  ExternalReferences external = ExternalReferences::kUninspected;
};
ProfileDeletionPreview PreviewProfileDeletion(const lc::Model& model,
                                              const lc::ProfileId& profile);

enum class ReassignmentState {
  kIdentityNotFound,
  kSameProfile,
  kBlockedByModelReferences,
  kEmptyUnattached,
};
enum class PopulatedMigrationPolicy { kNotApplicable, kOpen };
struct SpaceReassignmentPreview {
  ReassignmentState state;
  PopulatedMigrationPolicy populated_migration;
  std::vector<Reference> references;
  ExternalReferences external = ExternalReferences::kUninspected;
};
// Identity-derived evidence for the existing empty/unattached Model guard.
// This is not the Profile safety peer's transaction or a populated move plan.
SpaceReassignmentPreview PreviewSpaceReassignment(const lc::Model& model,
                                                  const lc::SpaceId& space,
                                                  const lc::ProfileId& target);

using SidebarReference = std::variant<lc::ItemId, lc::ShortcutId>;
enum class SidebarAttachment { kAttached, kRequiresAttachment };
struct LiveReference {
  lc::PageId page;
  lc::Attachment attachment;
};
struct SidebarLocation {
  lc::ProfileId profile;
  lc::ItemId item;
  std::optional<lc::ShortcutId> shortcut;
  lc::ItemOwner owner;
  lc::Attachment location;
  SidebarAttachment attachment;
  std::optional<LiveReference> live;
};
using SidebarResult = std::variant<lc::Status, SidebarLocation>;
// Resolve ordinary item/alias identities, including a missing owning-Space
// attachment. Does not attach, select, transfer, open or focus anything.
SidebarResult ResolveSidebar(const lc::Model& model,
                             const SidebarReference& reference,
                             const lc::Attachment& requesting);

}  // namespace arcified::workspace_organizer

#endif  // ARCIFIED_NATIVE_FEATURES_WORKSPACE_ORGANIZER_INSPECTION_H_
