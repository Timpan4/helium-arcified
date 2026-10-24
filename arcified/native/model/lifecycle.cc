#include "lifecycle.h"

#include <algorithm>
#include <type_traits>

namespace arcified::lifecycle {

Status CheckContractVersion(std::uint32_t version) {
  return version == kContractVersion ? Status::kOk
                                     : Status::kUnsupportedVersion;
}

const Attachment& LivePage::attachment() const {
  return std::visit(
      [](const auto& owner) -> const Attachment& { return owner.attachment; },
      owner);
}

Status Model::AddProfile(ProfileId id) {
  if (id.value.empty())
    return Status::kInvalidInput;
  return profiles_.emplace(std::move(id), Profile{}).second
             ? Status::kOk
             : Status::kDuplicateId;
}

Status Model::AddSpace(SpaceId id, ProfileId profile) {
  if (id.value.empty())
    return Status::kInvalidInput;
  if (!profiles_.count(profile))
    return Status::kNotFound;
  return spaces_.emplace(std::move(id), Space{std::move(profile)}).second
             ? Status::kOk
             : Status::kDuplicateId;
}

Status Model::AddWindow(WindowId id) {
  if (id.value.empty())
    return Status::kInvalidInput;
  return windows_.emplace(std::move(id), Window{}).second
             ? Status::kOk
             : Status::kDuplicateId;
}

Status Model::Attach(Attachment attachment) {
  if (!windows_.count(attachment.window) || !spaces_.count(attachment.space))
    return Status::kNotFound;
  attachments_.insert(std::move(attachment));
  return Status::kOk;
}

Status Model::Detach(Attachment attachment) {
  if (!attachments_.count(attachment))
    return Status::kNotAttached;
  for (const auto& [id, page] : pages_)
    if (page.attachment() == attachment)
      return Status::kReferenced;
  for (const auto& [id, split] : splits_)
    if (split.attachment == attachment)
      return Status::kReferenced;
  attachments_.erase(attachment);
  auto& selected = windows_.at(attachment.window).selected_space;
  if (selected == attachment.space)
    selected.reset();
  return Status::kOk;
}

Status Model::SelectSpace(WindowId window, SpaceId space) {
  if (!windows_.count(window) || !spaces_.count(space))
    return Status::kNotFound;
  if (!attachments_.count({window, space}))
    return Status::kNotAttached;
  windows_.at(window).selected_space = std::move(space);
  return Status::kOk;
}

std::optional<ProfileId> Model::OwnerProfile(const ItemOwner& owner) const {
  return std::visit(
      [this](const auto& id) -> std::optional<ProfileId> {
        using T = std::decay_t<decltype(id)>;
        if constexpr (std::is_same_v<T, SpaceId>) {
          auto space = spaces_.find(id);
          if (space != spaces_.end())
            return space->second.profile;
        } else {
          if (profiles_.count(id))
            return id;
        }
        return std::nullopt;
      },
      owner);
}

Status Model::AddItem(ItemId id,
                      ItemOwner owner,
                      std::string saved_destination) {
  if (id.value.empty() || saved_destination.empty())
    return Status::kInvalidInput;
  if (!OwnerProfile(owner))
    return Status::kNotFound;
  return items_.emplace(std::move(id),
                        PersistentItem{std::move(owner),
                                       std::move(saved_destination)})
                 .second
             ? Status::kOk
             : Status::kDuplicateId;
}

Status Model::AddShortcut(ShortcutId id, ProfileId profile, ItemId target) {
  if (id.value.empty())
    return Status::kInvalidInput;
  auto item = items_.find(target);
  if (!profiles_.count(profile) || item == items_.end())
    return Status::kNotFound;
  if (OwnerProfile(item->second.owner) != profile)
    return Status::kProfileMismatch;
  return shortcuts_
                 .emplace(std::move(id),
                          Shortcut{std::move(profile), std::move(target)})
                 .second
             ? Status::kOk
             : Status::kDuplicateId;
}

Status Model::RemoveShortcut(ShortcutId id) {
  return shortcuts_.erase(id) ? Status::kOk : Status::kNotFound;
}

std::optional<PageId> Model::FindItemPage(const ItemId& item) const {
  for (const auto& [id, page] : pages_) {
    const auto* saved = std::get_if<SavedPage>(&page.owner);
    if (saved && saved->item == item)
      return id;
  }
  return std::nullopt;
}

OpenResult Model::OpenItem(ItemId item_id,
                           Attachment attachment,
                           PageId new_page) {
  auto item = items_.find(item_id);
  if (item == items_.end())
    return Status::kNotFound;
  if (!attachments_.count(attachment))
    return Status::kNotAttached;
  if (OwnerProfile(item->second.owner) != spaces_.at(attachment.space).profile)
    return Status::kProfileMismatch;
  // A shortcut to a Space item resolves its owning Space, never rehomes it.
  Attachment destination = attachment;
  if (const auto* space = std::get_if<SpaceId>(&item->second.owner)) {
    destination.space = *space;
    if (!attachments_.count(destination))
      return Status::kNotAttached;
  }
  if (auto existing = FindItemPage(item_id)) {
    const auto status = TransferSavedPage(*existing, destination);
    if (status != Status::kOk)
      return status;
    return OpenedPage{*existing, OpenDisposition::kExisting};
  }
  if (new_page.value.empty())
    return Status::kInvalidInput;
  if (pages_.count(new_page))
    return Status::kDuplicateId;
  pages_.emplace(new_page, LivePage{*OwnerProfile(item->second.owner),
                                    item->second.saved_destination,
                                    SavedPage{item_id, destination}});
  return OpenedPage{std::move(new_page), OpenDisposition::kCreated};
}

OpenResult Model::OpenShortcut(ShortcutId shortcut,
                               Attachment attachment,
                               PageId new_page) {
  auto alias = shortcuts_.find(shortcut);
  if (alias == shortcuts_.end())
    return Status::kNotFound;
  return OpenItem(alias->second.target, attachment, std::move(new_page));
}

Status Model::OpenTemporary(PageId page,
                            TemporaryTabId tab,
                            Attachment attachment,
                            std::string destination) {
  if (page.value.empty() || tab.value.empty() || destination.empty())
    return Status::kInvalidInput;
  if (!attachments_.count(attachment))
    return Status::kNotAttached;
  if (pages_.count(page))
    return Status::kDuplicateId;
  for (const auto& [id, live] : pages_) {
    const auto* temporary = std::get_if<TemporaryTab>(&live.owner);
    if (temporary && temporary->id == tab)
      return Status::kDuplicateId;
  }
  pages_.emplace(
      std::move(page),
      LivePage{spaces_.at(attachment.space).profile, std::move(destination),
               TemporaryTab{std::move(tab), attachment}});
  return Status::kOk;
}

Status Model::Keep(PageId page_id, ItemId item) {
  auto page = pages_.find(page_id);
  if (page == pages_.end())
    return Status::kNotFound;
  const auto* temporary = std::get_if<TemporaryTab>(&page->second.owner);
  if (!temporary)
    return Status::kInvalidInput;
  const auto attachment = temporary->attachment;
  auto status =
      AddItem(item, attachment.space, page->second.current_navigation);
  if (status != Status::kOk)
    return status;
  page->second.owner = SavedPage{std::move(item), attachment};
  return Status::kOk;
}

Status Model::TransferSavedPage(PageId page_id, Attachment destination) {
  auto page = pages_.find(page_id);
  if (page == pages_.end())
    return Status::kNotFound;
  auto* saved = std::get_if<SavedPage>(&page->second.owner);
  if (!saved)
    return Status::kInvalidInput;
  if (!attachments_.count(destination))
    return Status::kNotAttached;
  if (page->second.profile != spaces_.at(destination.space).profile)
    return Status::kProfileMismatch;
  const auto& owner = items_.at(saved->item).owner;
  if (const auto* space = std::get_if<SpaceId>(&owner))
    if (*space != destination.space)
      return Status::kInvalidInput;
  if (saved->attachment == destination)
    return Status::kOk;
  // All refusals precede mutation. No callbacks or native events run here.
  RemovePageReferences(page_id);
  saved->attachment = std::move(destination);
  return Status::kOk;
}

LinkRoutingResult Model::PlanSavedPageLink(PageId source,
                                           NavigationKind kind,
                                           SiteRelation relation,
                                           std::string destination) const {
  auto page = pages_.find(source);
  if (page == pages_.end())
    return LinkRoutingRefusal::kSourceNotFound;
  if (!std::holds_alternative<SavedPage>(page->second.owner))
    return LinkRoutingRefusal::kSourceNotSaved;
  if (destination.empty())
    return LinkRoutingRefusal::kInvalidDestination;
  if (kind != NavigationKind::kLink)
    return LinkRoutingRefusal::kOutsideApprovedScope;
  switch (relation) {
    case SiteRelation::kUnknown:
      return LinkRoutingRefusal::kClassificationRequired;
    case SiteRelation::kSameSite:
      return LinkRoutingRefusal::kSameSitePolicyOpen;
    case SiteRelation::kOutbound:
      return TemporaryBranchPlan{std::move(source), page->second.profile,
                                 page->second.attachment(),
                                 std::move(destination)};
  }
  return LinkRoutingRefusal::kClassificationRequired;
}

Status Model::RecordNavigation(PageId page_id, std::string destination) {
  if (destination.empty())
    return Status::kInvalidInput;
  auto page = pages_.find(page_id);
  if (page == pages_.end())
    return Status::kNotFound;
  page->second.current_navigation = std::move(destination);
  // A new document invalidates the previous activity report.
  page->second.activity = Activity::kUnknown;
  return Status::kOk;
}

Status Model::ReportActivity(PageId page_id, Activity activity) {
  auto page = pages_.find(page_id);
  if (page == pages_.end())
    return Status::kNotFound;
  page->second.activity = activity;
  return Status::kOk;
}

Status Model::Reset(PageId page_id) {
  auto page = pages_.find(page_id);
  if (page == pages_.end())
    return Status::kNotFound;
  const auto* saved = std::get_if<SavedPage>(&page->second.owner);
  if (!saved)
    return Status::kInvalidInput;
  if (page->second.activity != Activity::kQuiescent)
    return Status::kWorkNotQuiescent;
  const auto destination = items_.at(saved->item).saved_destination;
  return RecordNavigation(page_id, destination);
}

void Model::RemovePageReferences(PageId page) {
  for (auto it = splits_.begin(); it != splits_.end();) {
    auto& references = it->second.pages;
    references.erase(std::remove(references.begin(), references.end(), page),
                     references.end());
    if (references.empty())
      it = splits_.erase(it);
    else
      ++it;
  }
}

void Model::ErasePage(PageId page) {
  RemovePageReferences(page);
  pages_.erase(page);
}

Status Model::ClosePage(PageId page_id) {
  auto page = pages_.find(page_id);
  if (page == pages_.end())
    return Status::kNotFound;
  if (page->second.activity != Activity::kQuiescent)
    return Status::kWorkNotQuiescent;
  ErasePage(page_id);
  return Status::kOk;
}

Status Model::CloseWindow(WindowId window) {
  if (!windows_.count(window))
    return Status::kNotFound;
  std::vector<PageId> closing;
  for (const auto& [id, page] : pages_) {
    if (page.attachment().window != window)
      continue;
    if (page.activity != Activity::kQuiescent)
      return Status::kWorkNotQuiescent;
    closing.push_back(id);
  }
  for (const auto& id : closing)
    ErasePage(id);
  for (auto it = attachments_.begin(); it != attachments_.end();) {
    if (it->window == window)
      it = attachments_.erase(it);
    else
      ++it;
  }
  windows_.erase(window);
  return Status::kOk;
}

Status Model::SetSplit(SplitId id,
                       Attachment attachment,
                       std::vector<PageId> pages) {
  if (id.value.empty() || pages.empty())
    return Status::kInvalidInput;
  if (!attachments_.count(attachment))
    return Status::kNotAttached;
  std::set<PageId> distinct;
  for (const auto& page : pages) {
    auto live = pages_.find(page);
    if (live == pages_.end())
      return Status::kNotFound;
    if (!(live->second.attachment() == attachment))
      return Status::kInvalidInput;
    if (!distinct.insert(page).second)
      return Status::kInvalidInput;
  }
  splits_.insert_or_assign(std::move(id), Split{attachment, std::move(pages)});
  return Status::kOk;
}

Status Model::RemoveSplit(SplitId id) {
  return splits_.erase(id) ? Status::kOk : Status::kNotFound;
}

Status Model::MoveItem(ItemId item_id, ItemOwner owner) {
  auto item = items_.find(item_id);
  if (item == items_.end() || !OwnerProfile(owner))
    return Status::kNotFound;
  if (OwnerProfile(item->second.owner) != OwnerProfile(owner))
    return Status::kReopenRequired;
  if (FindItemPage(item_id))
    return Status::kReferenced;
  item->second.owner = std::move(owner);
  return Status::kOk;
}

Status Model::CopySavedDestination(ItemId source,
                                   ItemId copy,
                                   ItemOwner owner) {
  auto item = items_.find(source);
  if (item == items_.end())
    return Status::kNotFound;
  return AddItem(std::move(copy), std::move(owner),
                 item->second.saved_destination);
}

Status Model::RemoveItem(ItemId item) {
  if (!items_.count(item))
    return Status::kNotFound;
  if (FindItemPage(item))
    return Status::kReferenced;
  for (const auto& [id, shortcut] : shortcuts_)
    if (shortcut.target == item)
      return Status::kReferenced;
  items_.erase(item);
  return Status::kOk;
}

bool Model::SpaceReferenced(const SpaceId& space) const {
  for (const auto& [id, item] : items_)
    if (const auto* owner = std::get_if<SpaceId>(&item.owner))
      if (*owner == space)
        return true;
  for (const auto& attachment : attachments_)
    if (attachment.space == space)
      return true;
  return false;  // Every page/split also requires an attachment.
}

Status Model::ReassignEmptySpace(SpaceId space_id, ProfileId profile) {
  auto space = spaces_.find(space_id);
  if (space == spaces_.end() || !profiles_.count(profile))
    return Status::kNotFound;
  if (space->second.profile == profile)
    return Status::kOk;
  if (SpaceReferenced(space_id))
    return Status::kReferenced;
  space->second.profile = std::move(profile);
  return Status::kOk;
}

Status Model::RemoveSpace(SpaceId space) {
  if (!spaces_.count(space))
    return Status::kNotFound;
  if (SpaceReferenced(space))
    return Status::kReferenced;
  spaces_.erase(space);
  return Status::kOk;
}

Status Model::RemoveProfile(ProfileId profile) {
  if (!profiles_.count(profile))
    return Status::kNotFound;
  for (const auto& [id, space] : spaces_)
    if (space.profile == profile)
      return Status::kReferenced;
  for (const auto& [id, item] : items_)
    if (OwnerProfile(item.owner) == profile)
      return Status::kReferenced;
  for (const auto& [id, shortcut] : shortcuts_)
    if (shortcut.profile == profile)
      return Status::kReferenced;
  profiles_.erase(profile);
  return Status::kOk;
}

std::optional<std::vector<ItemId>> Model::VisibleItems(
    const Attachment& attachment) const {
  if (!attachments_.count(attachment))
    return std::nullopt;
  const auto& profile = spaces_.at(attachment.space).profile;
  std::vector<ItemId> visible;
  for (const auto& [id, item] : items_) {
    if (const auto* owner = std::get_if<SpaceId>(&item.owner)) {
      if (*owner == attachment.space)
        visible.push_back(id);
    } else if (std::get<ProfileId>(item.owner) == profile) {
      visible.push_back(id);
    }
  }
  return visible;
}

std::optional<std::vector<ShortcutId>> Model::VisibleShortcuts(
    const Attachment& attachment) const {
  if (!attachments_.count(attachment))
    return std::nullopt;
  const auto& profile = spaces_.at(attachment.space).profile;
  std::vector<ShortcutId> visible;
  for (const auto& [id, shortcut] : shortcuts_)
    if (shortcut.profile == profile)
      visible.push_back(id);
  return visible;
}

std::vector<PageId> Model::TemporaryPages(const Attachment& attachment) const {
  std::vector<PageId> local;
  for (const auto& [id, page] : pages_)
    if (std::holds_alternative<TemporaryTab>(page.owner) &&
        page.attachment() == attachment)
      local.push_back(id);
  return local;
}

}  // namespace arcified::lifecycle
