#include "favorites.h"

#include <cstdlib>
#include <iostream>
#include <set>

namespace arcified::favorites {
namespace {

using namespace lifecycle;

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const ProfileId kWork{"work"};
const ProfileId kPersonal{"personal"};
const SpaceId kDesk{"desk"};
const SpaceId kResearch{"research"};
const SpaceId kHome{"home"};
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const Attachment kDeskFirst{kFirst, kDesk};
const Attachment kResearchFirst{kFirst, kResearch};
const Attachment kResearchSecond{kSecond, kResearch};
const Attachment kHomeFirst{kFirst, kHome};
const ItemId kMail{"mail"};
const ItemId kKept{"kept"};
const ShortcutId kAlias{"alias"};
const PageId kPage{"page"};
const std::string kSaved = "https://mail.example.test/inbox";
const std::string kCurrent = "https://mail.example.test/draft";

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kPersonal) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kResearch, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kHome, kPersonal) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  REQUIRE(model.Attach(kDeskFirst) == Status::kOk);
  REQUIRE(model.Attach(kResearchFirst) == Status::kOk);
  REQUIRE(model.Attach(kResearchSecond) == Status::kOk);
  REQUIRE(model.Attach(kHomeFirst) == Status::kOk);
  return model;
}

void ExpectOpen(const OpenResult& result,
                const PageId& page,
                OpenDisposition disposition) {
  REQUIRE(std::holds_alternative<OpenedPage>(result));
  REQUIRE(std::get<OpenedPage>(result).page == page);
  REQUIRE(std::get<OpenedPage>(result).disposition == disposition);
}

FavoriteTarget Target(const Model& model,
                      const Attachment& requesting,
                      FavoriteReference reference) {
  const auto result = ResolveFavorite(model, requesting, std::move(reference));
  REQUIRE(std::holds_alternative<FavoriteTarget>(result));
  return std::get<FavoriteTarget>(result);
}

FavoriteList List(const ListResult& result) {
  REQUIRE(std::holds_alternative<FavoriteList>(result));
  return std::get<FavoriteList>(result);
}

std::vector<FavoriteReference> References(const FavoriteList& list) {
  std::vector<FavoriteReference> result;
  for (const auto& entry : list.entries)
    result.push_back(entry.reference);
  return result;
}

void ProjectionKeepsProfileAndReferenceIdentity() {
  auto model = Workspace();
  const ItemId personal{"personal-item"};
  const ItemId same_url{"same-url"};
  const ShortcutId other_alias{"other-alias"};
  const ShortcutId personal_alias{"personal-alias"};
  REQUIRE(model.AddItem(kMail, kWork, kSaved) == Status::kOk);
  REQUIRE(model.AddItem(same_url, kWork, kSaved) == Status::kOk);
  REQUIRE(model.AddItem(kKept, kDesk, kSaved) == Status::kOk);
  REQUIRE(model.AddItem(personal, kPersonal, kSaved) == Status::kOk);
  REQUIRE(model.AddShortcut(kAlias, kWork, kKept) == Status::kOk);
  REQUIRE(model.AddShortcut(other_alias, kWork, kKept) == Status::kOk);
  REQUIRE(model.AddShortcut(personal_alias, kPersonal, personal) ==
          Status::kOk);
  const std::set<FavoriteReference> expected{kMail, same_url, kAlias,
                                             other_alias};
  for (const auto& attachment : {kDeskFirst, kResearchFirst, kResearchSecond}) {
    const auto list = List(EnumerateFavorites(model, attachment));
    REQUIRE(list.profile == kWork);
    const auto references = References(list);
    REQUIRE(std::set<FavoriteReference>(references.begin(), references.end()) ==
            expected);
    REQUIRE(list.entries.size() == expected.size());
    REQUIRE(Target(model, attachment, kMail).item == kMail);
    REQUIRE(Target(model, attachment, same_url).item == same_url);
    REQUIRE(Target(model, attachment, kAlias).item == kKept);
    REQUIRE(Target(model, attachment, other_alias).item == kKept);
  }
  const auto home = List(EnumerateFavorites(model, kHomeFirst));
  REQUIRE(home.profile == kPersonal);
  REQUIRE(References(home) ==
          (std::vector<FavoriteReference>{personal, personal_alias}));
  REQUIRE(std::get<Refusal>(ResolveFavorite(model, kHomeFirst, kMail)) ==
          Refusal::kProfileMismatch);
  REQUIRE(std::get<Refusal>(ResolveFavorite(model, kHomeFirst, kAlias)) ==
          Refusal::kProfileMismatch);
  REQUIRE(std::get<Refusal>(ResolveFavorite(model, kDeskFirst, kKept)) ==
          Refusal::kNotFavorite);
  REQUIRE(model.pages().empty());
  REQUIRE(!model.windows().at(kFirst).selected_space);
}

void ProjectionExposesAttachmentWithoutChangingIt() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kKept, kDesk, kSaved) == Status::kOk);
  REQUIRE(model.AddShortcut(kAlias, kWork, kKept) == Status::kOk);
  const auto first = Target(model, kResearchFirst, kAlias);
  REQUIRE(first.destination == kDeskFirst);
  REQUIRE(first.destination_attached);
  const auto second = Target(model, kResearchSecond, kAlias);
  const Attachment desk_second{kSecond, kDesk};
  REQUIRE(second.destination == desk_second);
  REQUIRE(!second.destination_attached);
  REQUIRE(List(EnumerateFavorites(model, kResearchSecond)).entries.size() == 1);
  REQUIRE(model.attachments().count(desk_second) == 0);
  REQUIRE(model.Attach(desk_second) == Status::kOk);
  REQUIRE(!second.destination_attached);  // Owned snapshot, never live state.
  REQUIRE(Target(model, kResearchSecond, kAlias).destination_attached);
  REQUIRE(model.MoveItem(kKept, kResearch) == Status::kOk);
  REQUIRE(Target(model, kResearchSecond, kAlias).destination ==
          kResearchSecond);
  REQUIRE(model.RemoveShortcut(kAlias) == Status::kOk);
  REQUIRE(std::get<Refusal>(ResolveFavorite(model, kResearchSecond, kAlias)) ==
          Refusal::kNotFound);
  REQUIRE(List(EnumerateFavorites(model, kResearchSecond)).entries.empty());
  REQUIRE(model.items().count(kKept) == 1);
}

void ExactCallerOrderRejectsStaleAndPartialInput() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kMail, kWork, kSaved) == Status::kOk);
  REQUIRE(model.AddItem(kKept, kDesk, kSaved) == Status::kOk);
  REQUIRE(model.AddShortcut(kAlias, kWork, kKept) == Status::kOk);
  const std::vector<FavoriteReference> order{kAlias, kMail};
  REQUIRE(References(List(OrderFavorites(model, kResearchSecond, order))) ==
          order);
  REQUIRE(std::get<Refusal>(OrderFavorites(model, kResearchSecond, {})) ==
          Refusal::kIncompleteOrder);
  REQUIRE(std::get<Refusal>(OrderFavorites(model, kResearchSecond, {kMail})) ==
          Refusal::kIncompleteOrder);
  REQUIRE(std::get<Refusal>(
              OrderFavorites(model, kResearchSecond, {kMail, kAlias, kMail})) ==
          Refusal::kDuplicateReference);
  REQUIRE(std::get<Refusal>(
              OrderFavorites(model, kResearchSecond, {kMail, kAlias, kKept})) ==
          Refusal::kNotFavorite);
  REQUIRE(std::get<Refusal>(OrderFavorites(model, kResearchSecond,
                                           {kMail, ShortcutId{"missing"}})) ==
          Refusal::kNotFound);
  const ItemId personal{"personal-item"};
  REQUIRE(model.AddItem(personal, kPersonal, kSaved) == Status::kOk);
  REQUIRE(std::get<Refusal>(
              OrderFavorites(model, kResearchSecond, {kMail, personal})) ==
          Refusal::kProfileMismatch);
  REQUIRE(model.RemoveShortcut(kAlias) == Status::kOk);
  REQUIRE(std::get<Refusal>(OrderFavorites(model, kResearchSecond, order)) ==
          Refusal::kNotFound);
  REQUIRE(model.RemoveItem(kMail) == Status::kOk);
  REQUIRE(List(OrderFavorites(model, kResearchSecond, {})).entries.empty());
}

void MissingAttachmentAndEmptyProfileStayDistinct() {
  auto model = Workspace();
  REQUIRE(List(EnumerateFavorites(model, kDeskFirst)).entries.empty());
  const Attachment absent{WindowId{"absent"}, kDesk};
  REQUIRE(std::get<Refusal>(EnumerateFavorites(model, absent)) ==
          Refusal::kNotAttached);
  REQUIRE(std::get<Refusal>(ResolveFavorite(model, absent, kMail)) ==
          Refusal::kNotAttached);
  REQUIRE(std::get<Refusal>(OrderFavorites(model, absent, {})) ==
          Refusal::kNotAttached);
  REQUIRE(std::get<Refusal>(ResolveFavorite(model, kDeskFirst, kMail)) ==
          Refusal::kNotFound);
  REQUIRE(model.Detach(kDeskFirst) == Status::kOk);
  REQUIRE(std::get<Refusal>(EnumerateFavorites(model, kDeskFirst)) ==
          Refusal::kNotAttached);
}

void ItemAndAliasWithSameTextRemainDistinct() {
  auto model = Workspace();
  const ItemId direct{"same-text"};
  const ShortcutId alias{"same-text"};
  REQUIRE(model.AddItem(direct, kWork, kSaved) == Status::kOk);
  REQUIRE(model.AddShortcut(alias, kWork, direct) == Status::kOk);
  const std::vector<FavoriteReference> order{alias, direct};
  const auto list = List(OrderFavorites(model, kResearchSecond, order));
  REQUIRE(References(list) == order);
  REQUIRE(list.entries.size() == 2);
  for (const auto& target : list.entries) {
    REQUIRE(target.item == direct);
    REQUIRE(target.destination == kResearchSecond);
    REQUIRE(target.destination_attached);
  }
  REQUIRE(model.MoveItem(direct, kDesk) == Status::kOk);
  REQUIRE(std::get<Refusal>(OrderFavorites(model, kResearchSecond, order)) ==
          Refusal::kNotFavorite);
  REQUIRE(References(List(EnumerateFavorites(model, kResearchSecond))) ==
          std::vector<FavoriteReference>{alias});
  REQUIRE(Target(model, kResearchSecond, alias).destination ==
          (Attachment{kSecond, kDesk}));
}

FavoritePresentation Describe(const Model& model,
                              const Attachment& requesting,
                              FavoriteReference reference,
                              const pins::SavedItemMetadata& metadata) {
  const auto result = DescribeFavorite(model, requesting, std::move(reference),
                                       metadata, "Native label");
  REQUIRE(std::holds_alternative<FavoritePresentation>(result));
  return std::get<FavoritePresentation>(result);
}

void AliasesReusePinsMetadataAndLivePageIdentity() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kKept, kDesk, kSaved) == Status::kOk);
  REQUIRE(model.AddShortcut(kAlias, kWork, kKept) == Status::kOk);
  pins::SavedItemMetadata metadata;
  const auto initial = Describe(model, kResearchSecond, kAlias, metadata);
  REQUIRE(initial.saved_item.item == kKept);
  REQUIRE(initial.saved_item.label == "Native label");
  REQUIRE(!initial.saved_item.uses_custom_label);
  REQUIRE(!initial.saved_item.live_page);
  REQUIRE(!initial.target.destination_attached);
  // One canonical metadata edit is observed by pins and every Favorite alias.
  const auto planned = pins::PlanLabelEdit(model, initial.target.item, metadata,
                                           std::string{"Project mail"});
  REQUIRE(std::holds_alternative<pins::LabelEdit>(planned));
  const auto edit = std::get<pins::LabelEdit>(planned);
  const auto applied = pins::ApplyLabelEdit(model, edit, metadata,
                                            pins::EditDirection::kForward);
  REQUIRE(std::holds_alternative<pins::SavedItemMetadata>(applied));
  metadata = std::get<pins::SavedItemMetadata>(applied);
  REQUIRE(Describe(model, kResearchFirst, kAlias, metadata).saved_item.label ==
          "Project mail");
  const auto pin = pins::DescribeSavedItem(model, kKept, metadata, "Pin label");
  REQUIRE(std::get<pins::SavedItemPresentation>(pin).label == "Project mail");
  ExpectOpen(model.OpenShortcut(kAlias, kResearchFirst, kPage), kPage,
             OpenDisposition::kCreated);
  REQUIRE(model.RecordNavigation(kPage, kCurrent) == Status::kOk);
  const auto live = Describe(model, kResearchFirst, kAlias, metadata);
  REQUIRE(live.saved_item.live_page == kPage);
  REQUIRE(live.saved_item.uses_custom_label);
  REQUIRE(live.saved_item.label == "Project mail");
  REQUIRE(model.items().at(kKept).saved_destination == kSaved);
  REQUIRE(model.pages().at(kPage).current_navigation == kCurrent);
  REQUIRE(std::get<Refusal>(DescribeFavorite(
              model, kHomeFirst, kAlias, metadata, "Other Profile label")) ==
          Refusal::kProfileMismatch);
  REQUIRE(std::get<pins::SavedItemError>(
              DescribeFavorite(model, kResearchFirst, kAlias, metadata, "")) ==
          pins::SavedItemError::kEmptyLabel);
  const auto reverted = pins::ApplyLabelEdit(model, edit, metadata,
                                             pins::EditDirection::kReverse);
  metadata = std::get<pins::SavedItemMetadata>(reverted);
  REQUIRE(Describe(model, kResearchFirst, kAlias, metadata).saved_item.label ==
          "Native label");
  REQUIRE(model.RemoveShortcut(kAlias) == Status::kOk);
  REQUIRE(std::get<Refusal>(DescribeFavorite(model, kResearchFirst, kAlias,
                                             metadata, "Native label")) ==
          Refusal::kNotFound);
  REQUIRE(model.items().count(kKept) == 1);
  REQUIRE(model.pages().count(kPage) == 1);
}

void FavoriteLifecycleUsesExistingModel() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kMail, kWork, kSaved) == Status::kOk);
  ExpectOpen(model.OpenItem(kMail, kDeskFirst, kPage), kPage,
             OpenDisposition::kCreated);
  REQUIRE(model.RecordNavigation(kPage, kCurrent) == Status::kOk);
  REQUIRE(model.ReportActivity(kPage, Activity::kProtected) == Status::kOk);
  ExpectOpen(model.OpenItem(kMail, kResearchSecond, PageId{"unused"}), kPage,
             OpenDisposition::kExisting);
  REQUIRE(model.pages().size() == 1);
  REQUIRE(model.pages().at(kPage).attachment() == kResearchSecond);
  REQUIRE(model.pages().at(kPage).profile == kWork);
  REQUIRE(model.pages().at(kPage).current_navigation == kCurrent);
  REQUIRE(model.pages().at(kPage).activity == Activity::kProtected);
  const auto transferred = Describe(model, kResearchSecond, kMail, {});
  REQUIRE(transferred.target.destination == kResearchSecond);
  REQUIRE(transferred.saved_item.live_page == kPage);
  REQUIRE(model.Reset(kPage) == Status::kWorkNotQuiescent);
  REQUIRE(model.ClosePage(kPage) == Status::kWorkNotQuiescent);
  REQUIRE(model.ReportActivity(kPage, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.Reset(kPage) == Status::kOk);
  REQUIRE(model.pages().at(kPage).current_navigation == kSaved);
  REQUIRE(model.items().at(kMail).saved_destination == kSaved);
  REQUIRE(model.ReportActivity(kPage, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kPage) == Status::kOk);
  REQUIRE(model.items().at(kMail).saved_destination == kSaved);
  REQUIRE(!Describe(model, kResearchFirst, kMail, {}).saved_item.live_page);
  ExpectOpen(model.OpenItem(kMail, kResearchFirst, PageId{"reopened"}),
             PageId{"reopened"}, OpenDisposition::kCreated);
  REQUIRE(model.pages().at(PageId{"reopened"}).current_navigation == kSaved);
  REQUIRE(Describe(model, kResearchFirst, kMail, {}).saved_item.live_page ==
          PageId{"reopened"});
}

void AliasVisibilityDoesNotRehomeTarget() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kKept, kDesk, kSaved) == Status::kOk);
  REQUIRE(model.AddShortcut(kAlias, kWork, kKept) == Status::kOk);
  REQUIRE(model.VisibleShortcuts(kResearchSecond) ==
          std::optional<std::vector<ShortcutId>>{{kAlias}});
  REQUIRE(!Target(model, kResearchSecond, kAlias).destination_attached);
  const auto failed = model.OpenShortcut(kAlias, kResearchSecond, kPage);
  REQUIRE(std::get<Status>(failed) == Status::kNotAttached);
  REQUIRE(model.pages().empty());
  const Attachment desk_second{kSecond, kDesk};
  REQUIRE(model.Attach(desk_second) == Status::kOk);
  REQUIRE(Target(model, kResearchSecond, kAlias).destination_attached);
  ExpectOpen(model.OpenShortcut(kAlias, kResearchSecond, kPage), kPage,
             OpenDisposition::kCreated);
  REQUIRE(model.pages().at(kPage).attachment() == desk_second);
  REQUIRE(std::get<SpaceId>(model.items().at(kKept).owner) == kDesk);
  REQUIRE(model.RemoveShortcut(kAlias) == Status::kOk);
  REQUIRE(List(EnumerateFavorites(model, kResearchSecond)).entries.empty());
  REQUIRE(model.items().count(kKept) == 1);
  REQUIRE(model.pages().count(kPage) == 1);
}

}  // namespace
}  // namespace arcified::favorites

int main() {
  using namespace arcified::favorites;
  const std::pair<const char*, void (*)()> fixtures[] = {
      {"profile-reference-projection",
       ProjectionKeepsProfileAndReferenceIdentity},
      {"attachment-projection", ProjectionExposesAttachmentWithoutChangingIt},
      {"exact-caller-order", ExactCallerOrderRejectsStaleAndPartialInput},
      {"missing-attachment", MissingAttachmentAndEmptyProfileStayDistinct},
      {"typed-reference-identity", ItemAndAliasWithSameTextRemainDistinct},
      {"shared-pins-metadata", AliasesReusePinsMetadataAndLivePageIdentity},
      {"favorite-model-lifecycle", FavoriteLifecycleUsesExistingModel},
      {"alias-owner-attachment", AliasVisibilityDoesNotRehomeTarget},
  };
  for (const auto& [name, run] : fixtures) {
    run();
    std::cout << "PASS " << name << '\n';
  }
}
