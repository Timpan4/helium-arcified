#ifndef ARCIFIED_NATIVE_FEATURES_ORGANIZATION_FOLDERS_H_
#define ARCIFIED_NATIVE_FEATURES_ORGANIZATION_FOLDERS_H_

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::organization {

using FolderId = lifecycle::Id<struct FolderTag>;
using Entry = std::variant<FolderId, lifecycle::ItemId>;

enum class FolderStatus {
  kOk,
  kNotFound,
  kDuplicateId,
  kInvalidInput,
  kSpaceNotFound,
  kItemNotFound,
  kItemOwnerMismatch,
  kCycle,
  kCrossSpaceUnsupported,
};

struct Folder {
  std::string label;
  bool expanded = true;
  std::vector<Entry> children;
};

struct Location {
  std::optional<FolderId> parent;  // nullopt means this Space's root.
  std::size_t index;
};

// Owned, validated read result. Contains only explicitly placed items.
// Consumers rendering all saved items must append unrepresented Space-owned
// ItemIds from the SAME lifecycle model in caller-supplied canonical order.
// Never treat omitted items as deleted or infer user order from map key order.
struct Hierarchy {
  lifecycle::SpaceId space;
  std::vector<Entry> roots;
  std::map<FolderId, Folder> folders;

  std::optional<Location> Locate(const Entry& entry) const;
  // Root-to-parent folder IDs, excluding entry itself; nullopt if absent.
  std::optional<std::vector<FolderId>> Ancestors(const Entry& entry) const;
};

using SnapshotResult = std::variant<FolderStatus, Hierarchy>;

struct Container {
  lifecycle::SpaceId space;
  std::optional<FolderId> folder;  // nullopt selects the Space root.
};

struct Placement {
  Container container;
  // Insert before an existing direct sibling, or append for nullopt.
  // Moving before oneself in the same container is a successful no-op.
  std::optional<Entry> before;
};

// Single-writer, in-memory organization metadata. The embedding caller
// serializes operations with lifecycle changes and allocates non-reused IDs.
// Commands own their arguments, including values borrowed from a snapshot.
// No page, URL, item label, window selection or lifecycle owner is stored here.
// No disk format, native adapter, deletion/undo or cross-Space move is
// provided.
class Folders {
 public:
  FolderStatus CreateFolder(const lifecycle::Model& model,
                            FolderId id,
                            std::string label,
                            Placement placement);
  FolderStatus Rename(const lifecycle::Model& model,
                      FolderId id,
                      std::string label);
  FolderStatus SetExpanded(const lifecycle::Model& model,
                           FolderId id,
                           bool expanded);
  FolderStatus SetAllExpanded(const lifecycle::Model& model,
                              lifecycle::SpaceId space,
                              bool expanded);
  // Places a Space-kept item for the first time, or moves its existing entry.
  // Profile shortcuts and items belonging to another Space are refused.
  FolderStatus PlaceItem(const lifecycle::Model& model,
                         lifecycle::ItemId item,
                         Placement placement);
  FolderStatus MoveFolder(const lifecycle::Model& model,
                          FolderId id,
                          Placement placement);

  // Revalidate all represented references in this Space on every read and
  // mutation. Missing/moved lifecycle items produce an error, never pruning.
  SnapshotResult Snapshot(const lifecycle::Model& model,
                          lifecycle::SpaceId space) const;
  SnapshotResult ReadFolder(const lifecycle::Model& model, FolderId id) const;

 private:
  using Trees = std::map<lifecycle::SpaceId, Hierarchy>;
  Trees::iterator FindFolder(const FolderId& id);
  Trees::const_iterator FindFolder(const FolderId& id) const;
  FolderStatus Validate(const lifecycle::Model& model,
                        const lifecycle::SpaceId& space) const;
  FolderStatus ValidatePlacement(const Placement& placement) const;
  Hierarchy& EnsureTree(const lifecycle::SpaceId& space);
  std::vector<Entry>& Children(Hierarchy& tree,
                               const std::optional<FolderId>& folder);
  void Relocate(Hierarchy& tree, Entry entry, const Placement& placement);

  Trees trees_;
};

}  // namespace arcified::organization

#endif  // ARCIFIED_NATIVE_FEATURES_ORGANIZATION_FOLDERS_H_
