#include "saved_item.h"

#include <cstdlib>
#include <iostream>
#include <utility>

namespace arcified::pins {
namespace {

using namespace lifecycle;

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

template <typename T, typename... Alternatives>
T Value(std::variant<Alternatives...> result) {
  REQUIRE(std::holds_alternative<T>(result));
  return std::get<T>(std::move(result));
}

template <typename... Alternatives>
void ExpectError(const std::variant<Alternatives...>& result,
                 SavedItemError error) {
  REQUIRE(std::holds_alternative<SavedItemError>(result));
  REQUIRE(std::get<SavedItemError>(result) == error);
}

const ProfileId kWork{"work"};
const ProfileId kHome{"home"};
const SpaceId kDesk{"desk"};
const SpaceId kResearch{"research"};
const SpaceId kPersonal{"personal"};
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const Attachment kDeskFirst{kFirst, kDesk};
const Attachment kDeskSecond{kSecond, kDesk};
const Attachment kResearchSecond{kSecond, kResearch};
const Attachment kPersonalSecond{kSecond, kPersonal};
const ItemId kMail{"mail"};
const ItemId kOther{"other-mail"};
const PageId kPage{"page"};
const std::string kSaved = "https://mail.example.test/inbox";
const std::string kCurrent = "https://mail.example.test/compose";

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kHome) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kResearch, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kPersonal, kHome) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  REQUIRE(model.Attach(kDeskFirst) == Status::kOk);
  REQUIRE(model.Attach(kResearchSecond) == Status::kOk);
  REQUIRE(model.Attach(kPersonalSecond) == Status::kOk);
  return model;
}

SavedItemPresentation Describe(const Model& model,
                               const ItemId& item,
                               const SavedItemMetadata& metadata,
                               std::string default_label = "Mail") {
  return Value<SavedItemPresentation>(
      DescribeSavedItem(model, item, metadata, std::move(default_label)));
}

void CanonicalPresentationAndDistinctItems() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kMail, kDesk, kSaved) == Status::kOk);
  REQUIRE(model.AddItem(kOther, kDesk, kSaved) == Status::kOk);
  REQUIRE(model.OpenTemporary(kPage, TemporaryTabId{"temporary"}, kDeskFirst,
                              kSaved) == Status::kOk);
  const SavedItemMetadata plain{};
  const auto closed = Describe(model, kMail, plain);
  REQUIRE(closed.item == kMail);
  REQUIRE(closed.label == "Mail");
  REQUIRE(!closed.uses_custom_label);
  REQUIRE(!closed.live_page);

  const auto opened = Value<OpenedPage>(
      model.OpenItem(kMail, kDeskFirst, PageId{"saved-page"}));
  const SavedItemMetadata renamed{std::string{"Inbox"}};
  const auto live = Describe(model, kMail, renamed);
  REQUIRE(live.label == "Inbox");
  REQUIRE(live.uses_custom_label);
  REQUIRE(live.live_page == opened.page);
  const auto other = Describe(model, kOther, plain, "Other mail");
  REQUIRE(other.item == kOther);
  REQUIRE(other.label == "Other mail");
  REQUIRE(!other.live_page);
  REQUIRE(model.items().size() == 2);
  REQUIRE(model.pages().size() == 2);
  REQUIRE(!closed.live_page);  // Earlier results are owned snapshots.
}

void AliasUsesCanonicalMetadata() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kMail, kDesk, kSaved) == Status::kOk);
  const ShortcutId alias{"favorite-mail"};
  REQUIRE(model.AddShortcut(alias, kWork, kMail) == Status::kOk);
  const auto shortcuts = model.VisibleShortcuts(kResearchSecond);
  REQUIRE(shortcuts && *shortcuts == std::vector<ShortcutId>{alias});
  const auto visible = model.VisibleItems(kResearchSecond);
  REQUIRE(visible && visible->empty());
  REQUIRE(!model.attachments().count(kDeskSecond));
  REQUIRE(model.VisibleShortcuts(kPersonalSecond)->empty());

  // Favorites validates alias visibility, then shares the canonical value.
  const auto target = model.shortcuts().at(alias).target;
  const SavedItemMetadata metadata{std::string{"Work inbox"}};
  const auto via_alias = Describe(model, target, metadata);
  const auto via_space = Describe(model, kMail, metadata);
  REQUIRE(via_alias.item == via_space.item);
  REQUIRE(via_alias.label == "Work inbox");
  REQUIRE(via_alias.label == via_space.label);
  REQUIRE(!via_alias.live_page);
  REQUIRE(std::get<Status>(model.OpenShortcut(alias, kResearchSecond, kPage)) ==
          Status::kNotAttached);
  REQUIRE(model.pages().empty());

  const ItemId standalone{"standalone"};
  REQUIRE(model.AddItem(standalone, kWork, kSaved) == Status::kOk);
  REQUIRE(Describe(model, standalone, {}).label == "Mail");
}

void ReversibleLabelValuesAndConflicts() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kMail, kDesk, kSaved) == Status::kOk);
  const SavedItemMetadata original{};
  const auto rename = Value<LabelEdit>(
      PlanLabelEdit(model, kMail, original, std::string{"Postlåda"}));
  REQUIRE(rename.item == kMail);
  REQUIRE(rename.before == original);
  auto current = Value<SavedItemMetadata>(
      ApplyLabelEdit(model, rename, original, EditDirection::kForward));
  REQUIRE(current.custom_label == std::string{"Postlåda"});
  REQUIRE(Describe(model, kMail, current).label == "Postlåda");
  ExpectError(ApplyLabelEdit(model, rename, current, EditDirection::kForward),
              SavedItemError::kStaleMetadata);
  const SavedItemMetadata another_window{std::string{"Changed elsewhere"}};
  ExpectError(
      ApplyLabelEdit(model, rename, another_window, EditDirection::kReverse),
      SavedItemError::kStaleMetadata);
  REQUIRE(Value<SavedItemMetadata>(ApplyLabelEdit(
              model, rename, current, EditDirection::kReverse)) == original);

  const auto restore =
      Value<LabelEdit>(PlanLabelEdit(model, kMail, current, std::nullopt));
  const auto restored = Value<SavedItemMetadata>(
      ApplyLabelEdit(model, restore, current, EditDirection::kForward));
  REQUIRE(!restored.custom_label);
  const auto default_row = Describe(model, kMail, restored, "Updated title");
  REQUIRE(default_row.label == "Updated title");
  REQUIRE(!default_row.uses_custom_label);
  REQUIRE(Value<SavedItemMetadata>(ApplyLabelEdit(
              model, restore, restored, EditDirection::kReverse)) == current);
  const auto no_op =
      Value<LabelEdit>(PlanLabelEdit(model, kMail, original, std::nullopt));
  REQUIRE(Value<SavedItemMetadata>(ApplyLabelEdit(
              model, no_op, original, EditDirection::kForward)) == original);
  REQUIRE(model.items().at(kMail).saved_destination == kSaved);
  REQUIRE(model.pages().empty());
}

void KeepTransferCloseAndReopen() {
  auto model = Workspace();
  REQUIRE(model.Attach(kDeskSecond) == Status::kOk);
  REQUIRE(model.OpenTemporary(kPage, TemporaryTabId{"draft"}, kDeskFirst,
                              kSaved) == Status::kOk);
  REQUIRE(model.ReportActivity(kPage, Activity::kProtected) == Status::kOk);
  ExpectError(PlanLabelEdit(model, kMail, {}, std::string{"Draft"}),
              SavedItemError::kItemNotFound);
  REQUIRE(model.Keep(kPage, kMail) == Status::kOk);
  REQUIRE(model.RecordNavigation(kPage, kCurrent) == Status::kOk);
  REQUIRE(model.ReportActivity(kPage, Activity::kProtected) == Status::kOk);
  const SplitId split{"split"};
  REQUIRE(model.SetSplit(split, kDeskFirst, {kPage}) == Status::kOk);

  const auto edit = Value<LabelEdit>(
      PlanLabelEdit(model, kMail, {}, std::string{"Pinned mail"}));
  const auto metadata = Value<SavedItemMetadata>(
      ApplyLabelEdit(model, edit, {}, EditDirection::kForward));
  REQUIRE(Describe(model, kMail, metadata).live_page == kPage);
  REQUIRE(model.items().at(kMail).saved_destination == kSaved);
  REQUIRE(model.pages().at(kPage).current_navigation == kCurrent);
  REQUIRE(model.pages().at(kPage).activity == Activity::kProtected);
  REQUIRE(model.pages().at(kPage).profile == kWork);
  REQUIRE(model.pages().at(kPage).attachment() == kDeskFirst);
  REQUIRE(model.splits().at(split).pages == std::vector<PageId>{kPage});

  REQUIRE(model.TransferSavedPage(kPage, kDeskSecond) == Status::kOk);
  REQUIRE(Describe(model, kMail, metadata).live_page == kPage);
  REQUIRE(model.pages().at(kPage).current_navigation == kCurrent);
  REQUIRE(model.pages().at(kPage).activity == Activity::kProtected);
  REQUIRE(model.pages().at(kPage).attachment() == kDeskSecond);
  REQUIRE(model.splits().empty());
  REQUIRE(model.ClosePage(kPage) == Status::kWorkNotQuiescent);
  REQUIRE(Describe(model, kMail, metadata).live_page == kPage);
  REQUIRE(model.ReportActivity(kPage, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kPage) == Status::kOk);
  const auto closed = Describe(model, kMail, metadata);
  REQUIRE(!closed.live_page);
  REQUIRE(closed.label == "Pinned mail");
  REQUIRE(model.items().at(kMail).saved_destination == kSaved);
  const PageId reopened{"reopened"};
  REQUIRE(Value<OpenedPage>(model.OpenItem(kMail, kDeskFirst, reopened)).page ==
          reopened);
  REQUIRE(Describe(model, kMail, metadata).live_page == reopened);
  REQUIRE(model.pages().at(reopened).current_navigation == kSaved);
}

void InvalidAndDeletedInputs() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kMail, kDesk, kSaved) == Status::kOk);
  const SavedItemMetadata invalid{std::string{}};
  ExpectError(DescribeSavedItem(model, kMail, {}, ""),
              SavedItemError::kEmptyLabel);
  ExpectError(DescribeSavedItem(model, kMail, invalid, "Mail"),
              SavedItemError::kEmptyLabel);
  ExpectError(PlanLabelEdit(model, kMail, {}, std::string{}),
              SavedItemError::kEmptyLabel);
  ExpectError(PlanLabelEdit(model, kMail, invalid, std::nullopt),
              SavedItemError::kEmptyLabel);
  const LabelEdit invalid_edit{kMail, {}, invalid};
  ExpectError(ApplyLabelEdit(model, invalid_edit, {}, EditDirection::kForward),
              SavedItemError::kEmptyLabel);
  const auto edit = Value<LabelEdit>(
      PlanLabelEdit(model, kMail, {}, std::string{"Mail label"}));
  REQUIRE(model.RemoveItem(kMail) == Status::kOk);
  ExpectError(DescribeSavedItem(model, kMail, {}, "Mail"),
              SavedItemError::kItemNotFound);
  ExpectError(PlanLabelEdit(model, kMail, {}, std::nullopt),
              SavedItemError::kItemNotFound);
  ExpectError(ApplyLabelEdit(model, edit, {}, EditDirection::kForward),
              SavedItemError::kItemNotFound);
  ExpectError(ApplyLabelEdit(model, edit, edit.after, EditDirection::kReverse),
              SavedItemError::kItemNotFound);
  REQUIRE(model.items().empty());
}

void OwnedValues() {
  auto model = Workspace();
  REQUIRE(model.AddItem(kMail, kDesk, kSaved) == Status::kOk);
  SavedItemMetadata current{std::string{"Original"}};
  std::string desired = "Renamed";
  const auto edit = Value<LabelEdit>(
      PlanLabelEdit(model, model.items().begin()->first, current, desired));
  auto row = Describe(model, kMail, current);
  current.custom_label = "Different";
  desired = "Different again";
  REQUIRE(edit.before.custom_label == std::string{"Original"});
  REQUIRE(edit.after.custom_label == std::string{"Renamed"});
  REQUIRE(row.label == "Original");
  row.label = "Presentation only";
  REQUIRE(model.items().at(kMail).saved_destination == kSaved);
}

}  // namespace
}  // namespace arcified::pins

int main() {
  using namespace arcified::pins;
  CanonicalPresentationAndDistinctItems();
  std::cout << "PASS pins/canonical-presentation-and-distinct-items\n";
  AliasUsesCanonicalMetadata();
  std::cout << "PASS pins/alias-canonical-metadata\n";
  ReversibleLabelValuesAndConflicts();
  std::cout << "PASS pins/reversible-label-values-and-conflicts\n";
  KeepTransferCloseAndReopen();
  std::cout << "PASS pins/keep-transfer-close-reopen\n";
  InvalidAndDeletedInputs();
  std::cout << "PASS pins/invalid-and-deleted-inputs\n";
  OwnedValues();
  std::cout << "PASS pins/owned-values\n";
}
