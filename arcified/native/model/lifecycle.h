#ifndef ARCIFIED_NATIVE_MODEL_LIFECYCLE_H_
#define ARCIFIED_NATIVE_MODEL_LIFECYCLE_H_

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

namespace arcified::lifecycle {

// Semantic contract version, not a disk/session serialization format.
inline constexpr std::uint32_t kContractVersion = 3;

template <typename Tag>
struct Id {
  explicit Id(std::string value) : value(std::move(value)) {}
  std::string value;
  friend bool operator==(const Id& a, const Id& b) {
    return a.value == b.value;
  }
  friend bool operator!=(const Id& a, const Id& b) { return !(a == b); }
  friend bool operator<(const Id& a, const Id& b) { return a.value < b.value; }
};

using ProfileId = Id<struct ProfileTag>;
using SpaceId = Id<struct SpaceTag>;
using ItemId = Id<struct ItemTag>;
using PageId = Id<struct PageTag>;
using WindowId = Id<struct WindowTag>;
using TemporaryTabId = Id<struct TemporaryTabTag>;
using ShortcutId = Id<struct ShortcutTag>;
using SplitId = Id<struct SplitTag>;

enum class Status {
  kOk,
  kNotFound,
  kDuplicateId,
  kInvalidInput,
  kNotAttached,
  kProfileMismatch,
  kReopenRequired,
  kReferenced,
  kWorkNotQuiescent,
  kUnsupportedVersion,
};

Status CheckContractVersion(std::uint32_t version);

struct Profile {};
struct Space {
  ProfileId profile;
};
struct Window {
  std::optional<SpaceId> selected_space;
};

struct Attachment {
  WindowId window;
  SpaceId space;
  friend bool operator==(const Attachment& a, const Attachment& b) {
    return a.window == b.window && a.space == b.space;
  }
  friend bool operator<(const Attachment& a, const Attachment& b) {
    return std::tie(a.window, a.space) < std::tie(b.window, b.space);
  }
};

// Space owner = Kept in this Space; Profile owner = Profile shortcut.
using ItemOwner = std::variant<SpaceId, ProfileId>;
struct PersistentItem {
  ItemOwner owner;
  std::string saved_destination;
};

// A Profile shortcut alias can expose a kept item without another item/page.
struct Shortcut {
  ProfileId profile;
  ItemId target;
};

struct TemporaryTab {
  TemporaryTabId id;
  Attachment attachment;  // Original window, immutable until kept/closed.
};
struct SavedPage {
  ItemId item;
  Attachment attachment;
};
using PageOwner = std::variant<TemporaryTab, SavedPage>;

// A report from the embedding caller, not a renderer heuristic or timer.
enum class Activity { kUnknown, kQuiescent, kProtected };
struct LivePage {
  ProfileId profile;  // Never reassigned, including after authentication.
  std::string current_navigation;
  PageOwner owner;
  Activity activity = Activity::kUnknown;
  const Attachment& attachment() const;
};

struct Split {
  Attachment attachment;
  std::vector<PageId> pages;  // Ordered references, never navigation copies.
};

enum class OpenDisposition { kCreated, kExisting };
struct OpenedPage {
  PageId page;
  OpenDisposition disposition;
};
using OpenResult = std::variant<Status, OpenedPage>;

// Authoritative browser inputs, never inferred from destination strings here.
enum class NavigationKind { kLink, kRedirect, kScript, kTyped, kOther };
// Link target relative to the source page's current committed document.
enum class SiteRelation { kUnknown, kSameSite, kOutbound };
enum class LinkRoutingRefusal {
  kSourceNotFound,
  kSourceNotSaved,
  kInvalidDestination,
  kOutsideApprovedScope,
  kClassificationRequired,
  kSameSitePolicyOpen,
};
// Owned snapshot for native preparation, not a reservation or commit token.
struct TemporaryBranchPlan {
  PageId source;
  ProfileId profile;
  Attachment attachment;
  std::string destination;
};
using LinkRoutingResult = std::variant<LinkRoutingRefusal, TemporaryBranchPlan>;

// In-memory, single-writer semantic model. See
// ../../docs/lifecycle-contract.md. Callers validate URLs, allocate non-reused
// IDs, and report actual native events. No method opens a URL, moves a
// renderer, persists state or changes credentials.
class Model {
 public:
  Status AddProfile(ProfileId id);
  Status AddSpace(SpaceId id, ProfileId profile);
  Status AddWindow(WindowId id);
  Status Attach(Attachment attachment);
  Status Detach(Attachment attachment);
  // Selection stays in this logical window, even across Space Profiles.
  Status SelectSpace(WindowId window, SpaceId space);

  Status AddItem(ItemId id, ItemOwner owner, std::string saved_destination);
  Status AddShortcut(ShortcutId id, ProfileId profile, ItemId target);
  Status RemoveShortcut(ShortcutId id);
  OpenResult OpenItem(ItemId item, Attachment attachment, PageId new_page);
  OpenResult OpenShortcut(ShortcutId shortcut,
                          Attachment attachment,
                          PageId new_page);
  Status OpenTemporary(PageId page,
                       TemporaryTabId tab,
                       Attachment attachment,
                       std::string destination);
  Status Keep(PageId page, ItemId item);
  // Moves only a saved page's attachment. Keeps its live identity and state;
  // removes old split references rather than moving or copying the layout.
  Status TransferSavedPage(PageId page, Attachment destination);

  // Pure decision only. Does not allocate a page or navigate the saved source.
  // Native caller revalidates the snapshot, acts, then reports actual success
  // with OpenTemporary using fresh IDs. Failure/cancellation records nothing.
  LinkRoutingResult PlanSavedPageLink(PageId source,
                                      NavigationKind kind,
                                      SiteRelation relation,
                                      std::string destination) const;

  // Observed navigation commits affect only current navigation, on any site.
  Status RecordNavigation(PageId page, std::string destination);
  Status ReportActivity(PageId page, Activity activity);
  Status Reset(PageId page);
  Status ClosePage(PageId page);
  Status CloseWindow(WindowId window);

  Status SetSplit(SplitId id, Attachment attachment, std::vector<PageId> pages);
  Status RemoveSplit(SplitId id);

  // Only closed items move. Cross-Profile movement always requires reopening.
  Status MoveItem(ItemId item, ItemOwner owner);
  // Explicit URL-only copy retains source and never copies current navigation.
  Status CopySavedDestination(ItemId source, ItemId copy, ItemOwner owner);
  Status RemoveItem(ItemId item);
  Status ReassignEmptySpace(SpaceId space, ProfileId profile);
  Status RemoveSpace(SpaceId space);
  Status RemoveProfile(ProfileId profile);

  std::optional<std::vector<ItemId>> VisibleItems(
      const Attachment& attachment) const;
  std::optional<std::vector<ShortcutId>> VisibleShortcuts(
      const Attachment& attachment) const;
  std::vector<PageId> TemporaryPages(const Attachment& attachment) const;

  // Views can be invalidated by mutations. Commands snapshot their arguments
  // by value, so IDs/attachments borrowed from these views are valid inputs.
  const auto& profiles() const { return profiles_; }
  const auto& spaces() const { return spaces_; }
  const auto& windows() const { return windows_; }
  const auto& attachments() const { return attachments_; }
  const auto& items() const { return items_; }
  const auto& shortcuts() const { return shortcuts_; }
  const auto& pages() const { return pages_; }
  const auto& splits() const { return splits_; }

 private:
  std::optional<ProfileId> OwnerProfile(const ItemOwner& owner) const;
  std::optional<PageId> FindItemPage(const ItemId& item) const;
  bool SpaceReferenced(const SpaceId& space) const;
  void RemovePageReferences(PageId page);
  void ErasePage(PageId page);

  std::map<ProfileId, Profile> profiles_;
  std::map<SpaceId, Space> spaces_;
  std::map<WindowId, Window> windows_;
  std::set<Attachment> attachments_;
  std::map<ItemId, PersistentItem> items_;
  std::map<ShortcutId, Shortcut> shortcuts_;
  std::map<PageId, LivePage> pages_;
  std::map<SplitId, Split> splits_;
};

}  // namespace arcified::lifecycle

#endif  // ARCIFIED_NATIVE_MODEL_LIFECYCLE_H_
