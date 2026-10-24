#include "../../features/accessibility/sidebar_accessibility.h"
#include "../../features/favorites/favorites.h"
#include "../../features/organization/folder_preview.h"
#include "../../features/organization/multi_select.h"
#include "../../features/temporary_tabs/temporary_tabs.h"
#include "../../features/window_ownership/handoff.h"
#include "../../features/workspace_organizer/inspection.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace arcified::sidebar_conformance {
namespace {

namespace ax = accessibility;
namespace lc = lifecycle;
namespace org = organization;
namespace wo = window_ownership;
namespace ws = workspace_organizer;

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

template <typename T, typename... Alternatives>
T Value(const std::variant<Alternatives...>& result) {
  REQUIRE(std::holds_alternative<T>(result));
  return std::get<T>(result);
}

const lc::ProfileId kProfile{"profile"};
const lc::SpaceId kSpace{"research"};
const lc::SpaceId kOtherSpace{"inbox"};
const lc::WindowId kSourceWindow{"source"};
const lc::WindowId kDestinationWindow{"destination"};
const lc::Attachment kSource{kSourceWindow, kSpace};
const lc::Attachment kDestination{kDestinationWindow, kSpace};
const lc::Attachment kRequesting{kDestinationWindow, kOtherSpace};
const lc::ItemId kSaved{"z-saved"};
const lc::ItemId kUnplaced{"a-unplaced"};
const lc::ShortcutId kAlias{"saved-alias"};
const lc::PageId kSavedPage{"saved-page"};
const lc::PageId kTemporaryPage{"temporary-page"};
const lc::TemporaryTabId kTemporary{"temporary"};
const lc::SplitId kSplit{"split"};
const org::FolderId kFolder{"folder"};
const org::FolderId kNested{"nested"};

// Explicit presentation identities supplied by this synthetic caller. They
// deliberately differ from lifecycle IDs. There is no production allocator.
const ax::RowId kSpaceRow{"row-space"};
const ax::RowId kAliasRow{"row-alias"};
const ax::RowId kFolderRow{"row-folder"};
const ax::RowId kNestedRow{"row-nested"};
const ax::RowId kSavedRow{"row-saved"};
const ax::RowId kUnplacedRow{"row-unplaced"};
const ax::RowId kTemporaryRow{"row-temporary"};
const ax::CommandId kOpen{"open"};

struct Fixture {
  lc::Model model;
  org::Folders folders;
  pins::SavedItemMetadata metadata{std::string{"Caf\xc3\xa9 notes"}};

  Fixture() {
    REQUIRE(model.AddProfile(kProfile) == lc::Status::kOk);
    REQUIRE(model.AddSpace(kSpace, kProfile) == lc::Status::kOk);
    REQUIRE(model.AddSpace(kOtherSpace, kProfile) == lc::Status::kOk);
    REQUIRE(model.AddWindow(kSourceWindow) == lc::Status::kOk);
    REQUIRE(model.AddWindow(kDestinationWindow) == lc::Status::kOk);
    for (const auto& attachment : {kSource, kDestination, kRequesting})
      REQUIRE(model.Attach(attachment) == lc::Status::kOk);
    REQUIRE(model.SelectSpace(kSourceWindow, kSpace) == lc::Status::kOk);
    REQUIRE(model.SelectSpace(kDestinationWindow, kOtherSpace) ==
            lc::Status::kOk);
    REQUIRE(model.AddItem(kSaved, kSpace, "https://example.test/saved") ==
            lc::Status::kOk);
    REQUIRE(model.AddItem(kUnplaced, kSpace, "https://example.test/unplaced") ==
            lc::Status::kOk);
    REQUIRE(model.AddShortcut(kAlias, kProfile, kSaved) == lc::Status::kOk);
    REQUIRE(Value<lc::OpenedPage>(model.OpenItem(kSaved, kSource, kSavedPage))
                .page == kSavedPage);
    REQUIRE(model.OpenTemporary(kTemporaryPage, kTemporary, kSource,
                                "https://example.test/temporary") ==
            lc::Status::kOk);
    REQUIRE(model.SetSplit(kSplit, kSource, {kSavedPage, kTemporaryPage}) ==
            lc::Status::kOk);
    REQUIRE(folders.CreateFolder(model, kFolder, "Research",
                                 {{kSpace, std::nullopt}, std::nullopt}) ==
            org::FolderStatus::kOk);
    REQUIRE(folders.CreateFolder(model, kNested, "Articles",
                                 {{kSpace, kFolder}, std::nullopt}) ==
            org::FolderStatus::kOk);
    REQUIRE(
        folders.PlaceItem(model, kSaved, {{kSpace, kNested}, std::nullopt}) ==
        org::FolderStatus::kOk);
  }

  pins::SavedItemPresentation Saved() const {
    return Value<pins::SavedItemPresentation>(pins::DescribeSavedItem(
        model, kSaved, metadata, "Native default title"));
  }

  org::FolderPreview Preview(std::string_view query = {},
                             std::optional<lc::PageId> active = std::nullopt,
                             lc::Attachment context = kSource) const {
    // This is the concrete UTF-8 join. A preview needs descendant labels only,
    // not the unrelated unplaced item, Favorite alias or temporary page.
    const auto saved = Saved();
    const std::map<lc::ItemId, std::string> labels{{saved.item, saved.label}};
    return Value<org::FolderPreview>(org::ProjectFolderPreview(
        model, folders, kFolder, context, labels, query, active));
  }
};

// The fixture's native caller supplies this COMPLETE visible logical preorder,
// including the offscreen unplaced item and temporary. The paired UTF-16 name
// is authoritative fixture input, not a conversion of pins' UTF-8 output.
// Neither a folder search nor Inspect/VisibleItems map order supplies it.
std::vector<ax::Row> ExpandedRows(std::u16string_view native_saved_name,
                                  bool saved_selected) {
  return {{kSpaceRow,
           u"Recherche",
           ax::RowKind::kSpace,
           std::nullopt,
           std::nullopt,
           true,
           {}},
          {kAliasRow,
           native_saved_name,
           ax::RowKind::kTab,
           kSpaceRow,
           false,
           std::nullopt,
           {{kOpen, u"Ouvrir", true}}},
          {kFolderRow,
           u"Recherche",
           ax::RowKind::kFolder,
           kSpaceRow,
           std::nullopt,
           true,
           {}},
          {kNestedRow,
           u"Articles",
           ax::RowKind::kFolder,
           kFolderRow,
           std::nullopt,
           true,
           {}},
          {kSavedRow,
           native_saved_name,
           ax::RowKind::kTab,
           kNestedRow,
           saved_selected,
           std::nullopt,
           {{kOpen, u"Ouvrir", true}}},
          {kUnplacedRow,
           u"Sans dossier",
           ax::RowKind::kTab,
           kSpaceRow,
           false,
           std::nullopt,
           {}},
          {kTemporaryRow,
           u"Temporaire",
           ax::RowKind::kTab,
           kSpaceRow,
           false,
           std::nullopt,
           {}}};
}

std::vector<ax::Row> CollapsedRows(std::u16string_view native_saved_name) {
  // A separate host observation after successful collapse. In particular the
  // active-descendant preview does not insert a hidden child into this list.
  return {{kSpaceRow,
           u"Recherche",
           ax::RowKind::kSpace,
           std::nullopt,
           std::nullopt,
           true,
           {}},
          {kAliasRow,
           native_saved_name,
           ax::RowKind::kTab,
           kSpaceRow,
           false,
           std::nullopt,
           {{kOpen, u"Ouvrir", true}}},
          {kFolderRow,
           u"Recherche",
           ax::RowKind::kFolder,
           kSpaceRow,
           std::nullopt,
           false,
           {}},
          {kUnplacedRow,
           u"Sans dossier",
           ax::RowKind::kTab,
           kSpaceRow,
           false,
           std::nullopt,
           {}},
          {kTemporaryRow,
           u"Temporaire",
           ax::RowKind::kTab,
           kSpaceRow,
           false,
           std::nullopt,
           {}}};
}

bool HasChange(const std::vector<ax::SemanticChange>& changes,
               const ax::RowId& row,
               ax::ChangeKind kind) {
  return std::any_of(changes.begin(), changes.end(), [&](const auto& change) {
    return change.row == row && change.kind == kind;
  });
}

void CanonicalAliasLabelsAndCompletePreorder() {
  Fixture f;
  const auto favorite_target = Value<favorites::FavoriteTarget>(
      favorites::ResolveFavorite(f.model, kSource, kAlias));
  const auto alias_location =
      Value<ws::SidebarLocation>(ws::ResolveSidebar(f.model, kAlias, kSource));
  REQUIRE(favorite_target.item == kSaved);
  REQUIRE(alias_location.item == favorite_target.item);
  REQUIRE(alias_location.shortcut == kAlias);
  const auto favorite =
      Value<favorites::FavoritePresentation>(favorites::DescribeFavorite(
          f.model, kSource, kAlias, f.metadata, "Native default title"));
  const auto preview = f.Preview();
  REQUIRE(preview.matches.size() == 1);
  REQUIRE(preview.matches[0].item == favorite.saved_item.item);
  REQUIRE(preview.matches[0].label == "Caf\xc3\xa9 notes");
  REQUIRE(favorite.saved_item.label == preview.matches[0].label);
  REQUIRE(favorite.saved_item.live_page == kSavedPage);
  REQUIRE(preview.matches[0].live_page == favorite.saved_item.live_page);

  const auto temporary = Value<std::vector<temporary_tabs::TemporaryIdentity>>(
      temporary_tabs::ProjectTemporaryIdentities(f.model, kSource,
                                                 {kTemporaryPage}));
  REQUIRE(temporary.size() == 1);
  REQUIRE(temporary[0].tab == kTemporary);
  const auto hierarchy =
      Value<org::Hierarchy>(f.folders.Snapshot(f.model, kSpace));
  REQUIRE(hierarchy.Locate(org::Entry{kSaved})->parent == kNested);
  REQUIRE(!hierarchy.Locate(org::Entry{kUnplaced}));

  // UTF-8/UTF-16 pairing is explicitly supplied, not inferred or encoded here.
  const auto rows = ExpandedRows(u"Caf\u00e9 notes", false);
  const auto projection = ax::Projection::Borrow(rows);
  REQUIRE(projection.has_value());
  const auto saved = *projection->Describe(kSavedRow);
  const auto alias = *projection->Describe(kAliasRow);
  REQUIRE(saved.row->name == u"Caf\u00e9 notes");
  REQUIRE(alias.row->name == saved.row->name);
  REQUIRE(saved.level == 4);
  REQUIRE(saved.position == 1 && saved.set_size == 1);
  const auto unplaced = *projection->Describe(kUnplacedRow);
  REQUIRE(unplaced.level == 2);
  REQUIRE(unplaced.position == 3 && unplaced.set_size == 4);
  REQUIRE(Value<ax::FocusSuggestion>(
              projection->OnKey(kSavedRow, ax::TreeKey::kDown,
                                ax::TextDirection::kLeftToRight))
              .row == kUnplacedRow);
  REQUIRE(Value<ax::FocusSuggestion>(
              projection->OnKey(kSpaceRow, ax::TreeKey::kEnd,
                                ax::TextDirection::kLeftToRight))
              .row == kTemporaryRow);
  REQUIRE(projection->Describe(kTemporaryRow)->row->selected == false);

  const auto edit = Value<pins::LabelEdit>(pins::PlanLabelEdit(
      f.model, alias_location.item, f.metadata, "Renamed notes"));
  f.metadata = Value<pins::SavedItemMetadata>(pins::ApplyLabelEdit(
      f.model, edit, f.metadata, pins::EditDirection::kForward));
  const auto renamed = f.Preview("Renamed");
  REQUIRE(renamed.matches.size() == 1);
  REQUIRE(renamed.matches[0].label == "Renamed notes");
  REQUIRE(Value<favorites::FavoritePresentation>(
              favorites::DescribeFavorite(f.model, kSource, kAlias, f.metadata,
                                          "Native default title"))
              .saved_item.label == renamed.matches[0].label);
  const auto new_rows = ExpandedRows(u"Renamed notes", false);
  const auto after = ax::Projection::Borrow(new_rows);
  REQUIRE(after.has_value());
  const auto changes = after->ChangesSince(*projection);
  REQUIRE(changes.size() == 2);
  REQUIRE(HasChange(changes, kSavedRow, ax::ChangeKind::kName));
  REQUIRE(HasChange(changes, kAliasRow, ax::ChangeKind::kName));
  REQUIRE(after->SuggestFocusAfterChange(*projection, kAliasRow)->row ==
          kAliasRow);
  REQUIRE(!after->SuggestFocusAfterChange(*projection, std::nullopt));
  REQUIRE(preview.matches[0].label == "Caf\xc3\xa9 notes");
  REQUIRE(f.model.pages().size() == 2);
}

void SelectionMoveIntentAndCollapsedActiveDescendant() {
  Fixture f;
  const std::vector<org::SelectionRowId> order{kFolder, kNested, kSaved,
                                               kUnplaced, kTemporary};
  const auto selection = Value<org::Selection>(
      org::ApplySelection(order, {}, kSaved, org::SelectionGesture::kReplace));
  const auto selected = Value<std::vector<org::SelectedRow>>(
      org::ProjectSelectedRows(order, selection));
  REQUIRE(selected.size() == order.size());
  REQUIRE(selected[2].id == org::SelectionRowId{kSaved});
  REQUIRE(selected[2].selected);
  REQUIRE(!selected[3].selected && !selected[4].selected);
  const auto move = Value<org::SelectionMovePlan>(
      org::PlanSelectionMove(f.model, f.folders, kSpace, order, selection,
                             {{kSpace, std::nullopt}, org::Entry{kFolder}}));
  REQUIRE(move.entries == std::vector<org::Entry>{kSaved});
  REQUIRE(
      (move.destination_children == std::vector<org::Entry>{kSaved, kFolder}));
  REQUIRE(Value<org::Hierarchy>(f.folders.Snapshot(f.model, kSpace))
              .Locate(org::Entry{kSaved})
              ->parent == kNested);

  const auto before_rows =
      ExpandedRows(u"Caf\u00e9 notes", selected[2].selected);
  const auto before = ax::Projection::Borrow(before_rows);
  REQUIRE(before.has_value());
  REQUIRE(before->Describe(kSavedRow)->row->selected == true);
  // Selection of a saved row does not choose the active page or move focus.
  REQUIRE(!f.Preview({}, kTemporaryPage).active_descendant);
  const auto collapse = Value<ax::ExpansionRequest>(before->OnKey(
      kFolderRow, ax::TreeKey::kLeft, ax::TextDirection::kLeftToRight));
  REQUIRE(collapse.row == kFolderRow && !collapse.expanded);
  REQUIRE(Value<org::Hierarchy>(f.folders.Snapshot(f.model, kSpace))
              .folders.at(kFolder)
              .expanded);
  // Only this explicit caller action records the organization change.
  REQUIRE(f.folders.SetExpanded(f.model, kFolder, collapse.expanded) ==
          org::FolderStatus::kOk);
  const auto preview = f.Preview("no matching label", kSavedPage);
  REQUIRE(preview.matches.empty());
  REQUIRE(preview.active_descendant.has_value());
  REQUIRE(preview.active_descendant->item == kSaved);
  REQUIRE(preview.active_descendant->live_page == kSavedPage);
  REQUIRE(preview.active_descendant->label == "Caf\xc3\xa9 notes");
  REQUIRE(preview.active_descendant->ancestry.size() == 2);
  REQUIRE(!preview.active_descendant->ancestry[0].expanded);

  const auto rows = CollapsedRows(u"Caf\u00e9 notes");
  const auto after = ax::Projection::Borrow(rows);
  REQUIRE(after.has_value());
  REQUIRE(!after->Describe(kSavedRow));
  REQUIRE(!after->Describe(kNestedRow));
  REQUIRE(after->Describe(kFolderRow)->row->expanded == false);
  REQUIRE(after->Describe(kUnplacedRow)->set_size == 4);
  REQUIRE(
      Value<ax::FocusSuggestion>(after->OnKey(kFolderRow, ax::TreeKey::kDown,
                                              ax::TextDirection::kLeftToRight))
          .row == kUnplacedRow);
  REQUIRE(after->SuggestFocusAfterChange(*before, kSavedRow)->row ==
          kFolderRow);
  REQUIRE(!after->SuggestFocusAfterChange(*before, std::nullopt));
  const auto changes = after->ChangesSince(*before);
  REQUIRE(HasChange(changes, kSavedRow, ax::ChangeKind::kRemoved));
  REQUIRE(HasChange(changes, kNestedRow, ax::ChangeKind::kRemoved));
  REQUIRE(HasChange(changes, kFolderRow, ax::ChangeKind::kExpansion));
  REQUIRE(f.model.pages().at(kSavedPage).attachment() == kSource);
}

void LiveAliasHandoffAndCurrentAttachmentStaleness() {
  Fixture f;
  REQUIRE(f.model.RecordNavigation(kSavedPage, "https://example.test/draft") ==
          lc::Status::kOk);
  REQUIRE(f.model.ReportActivity(kSavedPage, lc::Activity::kProtected) ==
          lc::Status::kOk);
  const auto rows = ExpandedRows(u"Caf\u00e9 notes", false);
  const auto projection = ax::Projection::Borrow(rows);
  REQUIRE(projection.has_value());
  const auto command = projection->RequestCommand(kAliasRow, kOpen);
  REQUIRE(command && command->row == kAliasRow && command->command == kOpen);
  // The caller explicitly binds this row to kAlias. The AX request carries
  // neither that binding nor the attachment and cannot authorize a transfer.
  const auto location = Value<ws::SidebarLocation>(
      ws::ResolveSidebar(f.model, kAlias, kRequesting));
  REQUIRE(location.item == kSaved && location.shortcut == kAlias);
  REQUIRE(location.location == kDestination);
  REQUIRE(location.live->page == kSavedPage);
  REQUIRE(location.live->attachment == kSource);
  const auto plan = Value<wo::HandoffPlan>(wo::PrepareOpenItemHandoff(
      f.model, location.item, kRequesting, kSavedPage));
  REQUIRE(plan.command == wo::HandoffCommand::kOpenItem);
  REQUIRE(plan.requested_destination == kRequesting);
  REQUIRE(plan.destination == location.location);
  REQUIRE(plan.source == location.live->attachment);
  REQUIRE(plan.focus.source_active == kSavedPage);
  REQUIRE(plan.focus.destination_page == kSavedPage);
  REQUIRE(plan.split_effects.size() == 1);
  REQUIRE(plan.split_effects[0].split == kSplit);
  REQUIRE(plan.split_effects[0].remaining_pages ==
          std::vector<lc::PageId>{kTemporaryPage});
  REQUIRE(f.model.pages().at(kSavedPage).attachment() == kSource);
  REQUIRE(f.model.windows().at(kDestinationWindow).selected_space ==
          kOtherSpace);
  REQUIRE(wo::RevalidateHandoff(f.model, plan, kSavedPage) ==
          wo::Revalidation::kCurrent);
  REQUIRE(wo::RevalidateHandoff(f.model, plan, kTemporaryPage) ==
          wo::Revalidation::kFocusChanged);

  REQUIRE(f.model.Detach(kRequesting) == lc::Status::kOk);
  REQUIRE(Value<lc::Status>(ws::ResolveSidebar(f.model, kAlias, kRequesting)) ==
          lc::Status::kNotAttached);
  REQUIRE(Value<wo::HandoffRefusal>(wo::PrepareOpenItemHandoff(
              f.model, location.item, kRequesting, kSavedPage)) ==
          wo::HandoffRefusal::kNotAttached);
  REQUIRE(wo::RevalidateHandoff(f.model, plan, kSavedPage) ==
          wo::Revalidation::kDestinationChanged);
  // The old semantic borrow still advertises its old capability. A fresh
  // caller observation can disable it, but cannot revoke the owned request.
  REQUIRE(projection->RequestCommand(kAliasRow, kOpen).has_value());
  auto disabled_rows = rows;
  disabled_rows[1].commands[0].enabled = false;
  const auto disabled = ax::Projection::Borrow(disabled_rows);
  REQUIRE(disabled.has_value());
  REQUIRE(!disabled->RequestCommand(kAliasRow, kOpen));
  REQUIRE(HasChange(disabled->ChangesSince(*projection), kAliasRow,
                    ax::ChangeKind::kCommands));
  REQUIRE(f.model.pages().at(kSavedPage).attachment() == kSource);
  REQUIRE(f.model.Attach(kRequesting) == lc::Status::kOk);
  REQUIRE(f.model.SelectSpace(kDestinationWindow, kOtherSpace) ==
          lc::Status::kOk);

  REQUIRE(f.model.Detach(kDestination) == lc::Status::kOk);
  REQUIRE(wo::RevalidateHandoff(f.model, plan, kSavedPage) ==
          wo::Revalidation::kDestinationChanged);
  const auto unattached = Value<ws::SidebarLocation>(
      ws::ResolveSidebar(f.model, kAlias, kRequesting));
  REQUIRE(unattached.attachment == ws::SidebarAttachment::kRequiresAttachment);
  REQUIRE(Value<wo::HandoffRefusal>(wo::PrepareOpenItemHandoff(
              f.model, unattached.item, kRequesting, kSavedPage)) ==
          wo::HandoffRefusal::kNotAttached);
  REQUIRE(f.model.Attach(kDestination) == lc::Status::kOk);
  // A new plan follows the current state; this does not claim ABA detection.
  const auto fresh = Value<wo::HandoffPlan>(wo::PrepareOpenItemHandoff(
      f.model, location.item, kRequesting, kSavedPage));
  REQUIRE(wo::RevalidateHandoff(f.model, fresh, kSavedPage) ==
          wo::Revalidation::kCurrent);
  // Synthetic model observation only. No native transfer is performed here.
  REQUIRE(f.model.TransferSavedPage(fresh.page, fresh.destination) ==
          lc::Status::kOk);
  REQUIRE(wo::RevalidateHandoff(f.model, fresh, kSavedPage) ==
          wo::Revalidation::kSourceChanged);
  const auto moved = Value<ws::SidebarLocation>(
      ws::ResolveSidebar(f.model, kAlias, kRequesting));
  REQUIRE(moved.live->page == location.live->page);
  REQUIRE(moved.live->attachment == kDestination);
  REQUIRE(location.live->attachment == kSource);  // Old snapshot is stale.
  REQUIRE(f.model.splits().at(kSplit).pages ==
          std::vector<lc::PageId>{kTemporaryPage});
  REQUIRE(f.model.pages().at(kSavedPage).current_navigation ==
          "https://example.test/draft");
  REQUIRE(f.model.pages().at(kSavedPage).activity == lc::Activity::kProtected);
  REQUIRE(f.model.items().at(kSaved).saved_destination ==
          "https://example.test/saved");
  REQUIRE(f.model.windows().at(kDestinationWindow).selected_space ==
          kOtherSpace);
  REQUIRE(f.model.pages().size() == 2);
  const auto moved_preview = f.Preview({}, kSavedPage, kDestination);
  REQUIRE(moved_preview.matches[0].live_page == kSavedPage);
  REQUIRE(moved_preview.matches[0].active);
  REQUIRE(std::get<org::PreviewError>(org::ProjectFolderPreview(
              f.model, f.folders, kFolder, kSource, {{kSaved, f.Saved().label}},
              {}, kSavedPage)) ==
          org::PreviewError::kActivePageOutsideAttachment);
  const auto inspection = ws::Inspect(f.model);
  REQUIRE(inspection.MissingTargets().empty());
  const auto incoming = inspection.ReferencesTo(kDestination);
  REQUIRE(std::any_of(incoming.begin(), incoming.end(), [](const auto& ref) {
    return ref.source == ws::Identity{kSavedPage} &&
           ref.relation == ws::Relation::kPageAttachment;
  }));
}

}  // namespace
}  // namespace arcified::sidebar_conformance

int main() {
  using namespace arcified::sidebar_conformance;
  CanonicalAliasLabelsAndCompletePreorder();
  SelectionMoveIntentAndCollapsedActiveDescendant();
  LiveAliasHandoffAndCurrentAttachmentStaleness();
  std::cout << "PASS: sidebar composed conformance scenarios\n";
}
