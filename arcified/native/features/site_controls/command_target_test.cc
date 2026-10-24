#include "command_target.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace arcified::site_controls {
namespace {

namespace lc = lifecycle;

static_assert(lc::kContractVersion == 3);
static_assert(!std::is_convertible_v<DocumentId, OriginId>);
static_assert(!std::is_convertible_v<SurfaceId, AnchorId>);

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const lc::ProfileId kWork{"work"};
const lc::ProfileId kPersonal{"personal"};
const lc::SpaceId kDesk{"desk"};
const lc::SpaceId kOtherDesk{"other-desk"};
const lc::SpaceId kHome{"home"};
const lc::WindowId kFirst{"first"};
const lc::WindowId kSecond{"second"};
const lc::Attachment kHere{kFirst, kDesk};
const lc::Attachment kThere{kSecond, kDesk};
const lc::Attachment kOtherHere{kFirst, kOtherDesk};
const lc::Attachment kHomeHere{kFirst, kHome};
const lc::ItemId kItem{"saved-item"};
const lc::PageId kPage{"saved-page"};
const lc::PageId kSibling{"same-url-split-page"};
const lc::PageId kHomePage{"personal-page"};
const lc::SplitId kSplit{"split"};
const char kDestination[] = "https://synthetic.example.test/document";

lc::Model Workspace() {
  lc::Model model;
  REQUIRE(model.AddProfile(kWork) == lc::Status::kOk);
  REQUIRE(model.AddProfile(kPersonal) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kOtherDesk, kWork) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kHome, kPersonal) == lc::Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == lc::Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == lc::Status::kOk);
  REQUIRE(model.Attach(kHere) == lc::Status::kOk);
  REQUIRE(model.Attach(kThere) == lc::Status::kOk);
  REQUIRE(model.Attach(kOtherHere) == lc::Status::kOk);
  REQUIRE(model.Attach(kHomeHere) == lc::Status::kOk);
  REQUIRE(model.SelectSpace(kFirst, kDesk) == lc::Status::kOk);
  REQUIRE(model.SelectSpace(kSecond, kDesk) == lc::Status::kOk);
  REQUIRE(model.AddItem(kItem, kDesk, kDestination) == lc::Status::kOk);
  REQUIRE(std::holds_alternative<lc::OpenedPage>(
      model.OpenItem(kItem, kHere, kPage)));
  REQUIRE(model.OpenTemporary(kSibling, lc::TemporaryTabId{"sibling-tab"},
                              kHere, kDestination) == lc::Status::kOk);
  REQUIRE(model.OpenTemporary(kHomePage, lc::TemporaryTabId{"home-tab"},
                              kHomeHere, kDestination) == lc::Status::kOk);
  REQUIRE(model.SetSplit(kSplit, kHere, {kPage, kSibling}) == lc::Status::kOk);
  return model;
}

HostContext NativeContext() {
  return {Target{{kPage, kWork, kHere},
                 DocumentId{"document"},
                 OriginId{"origin"},
                 SecurityStateId{"security"},
                 SurfaceId{"pane"},
                 SelectionId{"selection"}},
          {Command::kPageInfo, Command::kSiteSettings, Command::kPrint},
          AnchorId{"native-anchor"}};
}

CommandTarget Prepare(const lc::Model& model,
                      const HostContext& host,
                      Command command = Command::kPageInfo) {
  const auto result = PrepareCommand(model, command, host);
  REQUIRE(std::holds_alternative<CommandTarget>(result));
  return std::get<CommandTarget>(result);
}

void ExpectRefusal(const PrepareResult& result, Refusal refusal) {
  REQUIRE(std::holds_alternative<Refusal>(result));
  REQUIRE(std::get<Refusal>(result) == refusal);
}

void ReadyCommandsDoNotMutateOwnership() {
  auto model = Workspace();
  auto host = NativeContext();
  REQUIRE(model.ReportActivity(kPage, lc::Activity::kProtected) ==
          lc::Status::kOk);
  for (const auto command :
       {Command::kPageInfo, Command::kSiteSettings, Command::kPrint}) {
    const auto request = Prepare(model, host, command);
    REQUIRE(request.command == command);
    REQUIRE(request.target.page_target.page == kPage);
    REQUIRE(request.target.page_target.profile == kWork);
    REQUIRE(!RevalidateCommand(model, request, host));
    REQUIRE(request.anchor.has_value() == (command == Command::kPageInfo));
  }
  REQUIRE(model.pages().size() == 3);
  REQUIRE(model.items().size() == 1);
  REQUIRE(model.pages().at(kPage).current_navigation == kDestination);
  REQUIRE(model.pages().at(kPage).profile == kWork);
  REQUIRE(model.pages().at(kPage).attachment() == kHere);
  REQUIRE(model.pages().at(kPage).activity == lc::Activity::kProtected);
  REQUIRE(model.items().at(kItem).saved_destination == kDestination);
  REQUIRE(model.splits().at(kSplit).pages ==
          std::vector<lc::PageId>({kPage, kSibling}));
  REQUIRE(model.windows().at(kFirst).selected_space == kDesk);
}

void NativeSelectionAndIdentityRequired() {
  const auto model = Workspace();
  auto host = NativeContext();
  const auto request = Prepare(model, host);
  host.selected.reset();
  ExpectRefusal(PrepareCommand(model, Command::kPageInfo, host),
                Refusal::kNativeSelectionUnavailable);
  REQUIRE(RevalidateCommand(model, request, host) ==
          Refusal::kNativeSelectionUnavailable);

  // Model selection still names the Space. It supplies no native page/pane.
  REQUIRE(model.windows().at(kFirst).selected_space == kDesk);
  const auto invalidate = [](Target& target, int field) {
    switch (field) {
      case 0:
        target.document = DocumentId{""};
        break;
      case 1:
        target.origin = OriginId{""};
        break;
      case 2:
        target.security_state = SecurityStateId{""};
        break;
      case 3:
        target.surface = SurfaceId{""};
        break;
      case 4:
        target.selection = SelectionId{""};
        break;
    }
  };
  for (const int field : {0, 1, 2, 3, 4}) {
    host = NativeContext();
    invalidate(*host.selected, field);
    ExpectRefusal(PrepareCommand(model, Command::kPageInfo, host),
                  Refusal::kInvalidNativeIdentity);
    REQUIRE(RevalidateCommand(model, request, host) ==
            Refusal::kInvalidNativeIdentity);
    auto malformed_request = request;
    invalidate(malformed_request.target, field);
    REQUIRE(RevalidateCommand(model, malformed_request, NativeContext()) ==
            Refusal::kInvalidNativeIdentity);
  }
}

void WrongLifecycleIdentityRefused() {
  auto model = Workspace();
  auto host = NativeContext();
  const auto request = Prepare(model, host);
  host.selected->page_target.profile = lc::ProfileId{""};
  ExpectRefusal(PrepareCommand(model, Command::kSiteSettings, host),
                Refusal::kInvalidLifecycleIdentity);
  auto malformed_request = request;
  malformed_request.target.page_target.profile = lc::ProfileId{""};
  REQUIRE(RevalidateCommand(model, malformed_request, NativeContext()) ==
          Refusal::kInvalidLifecycleIdentity);
  host = NativeContext();
  host.selected->page_target.page = lc::PageId{"missing"};
  ExpectRefusal(PrepareCommand(model, Command::kPageInfo, host),
                Refusal::kPageNotFound);
  host = NativeContext();
  host.selected->page_target.profile = kPersonal;
  ExpectRefusal(PrepareCommand(model, Command::kSiteSettings, host),
                Refusal::kProfileMismatch);
  host = NativeContext();
  host.selected->page_target.attachment = kThere;
  ExpectRefusal(PrepareCommand(model, Command::kPrint, host),
                Refusal::kAttachmentChanged);
  host.selected->page_target.attachment =
      lc::Attachment{kFirst, lc::SpaceId{"missing"}};
  ExpectRefusal(PrepareCommand(model, Command::kPrint, host),
                Refusal::kAttachmentNotFound);
  host = NativeContext();
  REQUIRE(model.SelectSpace(kFirst, kOtherDesk) == lc::Status::kOk);
  ExpectRefusal(PrepareCommand(model, Command::kPageInfo, host),
                Refusal::kSpaceNotSelected);
  REQUIRE(model.Detach(kOtherHere) == lc::Status::kOk);
  REQUIRE(!model.windows().at(kFirst).selected_space);
  ExpectRefusal(PrepareCommand(model, Command::kPageInfo, host),
                Refusal::kSpaceNotSelected);
}

void AvailabilityAndAnchorFailures() {
  const auto model = Workspace();
  for (const auto command :
       {Command::kPageInfo, Command::kSiteSettings, Command::kPrint}) {
    auto host = NativeContext();
    const auto request = Prepare(model, host, command);
    host.available_commands.erase(command);
    ExpectRefusal(PrepareCommand(model, command, host),
                  Refusal::kCommandUnavailable);
    REQUIRE(RevalidateCommand(model, request, host) ==
            Refusal::kCommandUnavailable);
  }
  auto host = NativeContext();
  const auto request = Prepare(model, host);
  host.page_info_anchor.reset();
  ExpectRefusal(PrepareCommand(model, Command::kPageInfo, host),
                Refusal::kAnchorUnavailable);
  REQUIRE(RevalidateCommand(model, request, host) ==
          Refusal::kAnchorUnavailable);
  // Site settings and print have native ownership, but no Page Info anchor.
  REQUIRE(!RevalidateCommand(
      model, Prepare(model, host, Command::kSiteSettings), host));
  REQUIRE(
      !RevalidateCommand(model, Prepare(model, host, Command::kPrint), host));
  host.page_info_anchor = AnchorId{""};
  ExpectRefusal(PrepareCommand(model, Command::kPageInfo, host),
                Refusal::kAnchorUnavailable);
  host.page_info_anchor = AnchorId{"recreated-native-anchor"};
  REQUIRE(RevalidateCommand(model, request, host) == Refusal::kAnchorChanged);
  REQUIRE(!RevalidateCommand(model, Prepare(model, host), host));
  const auto unsupported = static_cast<Command>(-1);
  host.available_commands.insert(unsupported);
  ExpectRefusal(PrepareCommand(model, unsupported, host),
                Refusal::kUnsupportedCommand);
  auto invalid_request = request;
  invalid_request.command = unsupported;
  REQUIRE(RevalidateCommand(model, invalid_request, host) ==
          Refusal::kUnsupportedCommand);
}

void NavigationSecurityAndSurfaceChanges() {
  auto model = Workspace();
  auto host = NativeContext();
  const auto request = Prepare(model, host);
  // Same-URL reload is a different document, without a destination difference.
  REQUIRE(model.RecordNavigation(kPage, kDestination) == lc::Status::kOk);
  host.selected->document = DocumentId{"reloaded-document"};
  REQUIRE(RevalidateCommand(model, request, host) == Refusal::kDocumentChanged);
  REQUIRE(!RevalidateCommand(model, Prepare(model, host), host));
  host = NativeContext();
  host.selected->origin = OriginId{"new-native-origin-subject"};
  REQUIRE(RevalidateCommand(model, request, host) == Refusal::kOriginChanged);
  host = NativeContext();
  host.selected->security_state = SecurityStateId{"updated-security"};
  REQUIRE(RevalidateCommand(model, request, host) ==
          Refusal::kSecurityStateChanged);
  host = NativeContext();
  host.selected->surface = SurfaceId{"replacement-pane"};
  REQUIRE(RevalidateCommand(model, request, host) == Refusal::kSurfaceChanged);
  host = NativeContext();
  host.selected->selection = SelectionId{"returned-after-switch"};
  REQUIRE(RevalidateCommand(model, request, host) ==
          Refusal::kSelectionChanged);
}

void SameUrlSplitFocusIsNotInferred() {
  const auto model = Workspace();
  auto host = NativeContext();
  const auto request = Prepare(model, host);
  host.selected->page_target.page = kSibling;
  REQUIRE(model.pages().at(kPage).current_navigation ==
          model.pages().at(kSibling).current_navigation);
  REQUIRE(RevalidateCommand(model, request, host) == Refusal::kPageNotSelected);
  host.selected->document = DocumentId{"sibling-document"};
  host.selected->surface = SurfaceId{"sibling-pane"};
  host.selected->selection = SelectionId{"sibling-selection"};
  const auto sibling_request = Prepare(model, host);
  REQUIRE(sibling_request.target.page_target.page == kSibling);
  REQUIRE(!RevalidateCommand(model, sibling_request, host));
}

void SavedTransferInvalidatesCapturedCommand() {
  auto model = Workspace();
  auto host = NativeContext();
  const auto request = Prepare(model, host);
  REQUIRE(model.TransferSavedPage(kPage, kThere) == lc::Status::kOk);
  REQUIRE(RevalidateCommand(model, request, host) ==
          Refusal::kAttachmentChanged);
  host.selected->page_target.attachment = kThere;
  host.selected->surface = SurfaceId{"destination-native-pane"};
  host.selected->selection = SelectionId{"destination-selection"};
  host.page_info_anchor = AnchorId{"destination-native-anchor"};
  REQUIRE(RevalidateCommand(model, request, host) ==
          Refusal::kAttachmentChanged);
  REQUIRE(!RevalidateCommand(model, Prepare(model, host), host));
  REQUIRE(model.pages().at(kPage).current_navigation == kDestination);
  REQUIRE(model.pages().at(kPage).profile == kWork);
  // Lifecycle identities alone cannot detect moving away and back (ABA).
  REQUIRE(model.TransferSavedPage(kPage, kHere) == lc::Status::kOk);
  host = NativeContext();
  host.selected->selection = SelectionId{"after-transfer-away-and-back"};
  REQUIRE(RevalidateCommand(model, request, host) ==
          Refusal::kSelectionChanged);
  REQUIRE(!RevalidateCommand(model, Prepare(model, host), host));
}

void ProfileSwitchCannotRetargetOldAction() {
  auto model = Workspace();
  auto host = NativeContext();
  const auto request = Prepare(model, host, Command::kSiteSettings);
  REQUIRE(model.SelectSpace(kFirst, kHome) == lc::Status::kOk);
  REQUIRE(RevalidateCommand(model, request, host) ==
          Refusal::kSpaceNotSelected);
  host.selected->page_target.page = kHomePage;
  host.selected->page_target.profile = kPersonal;
  host.selected->page_target.attachment = kHomeHere;
  host.selected->document = DocumentId{"personal-document"};
  host.selected->origin = OriginId{"personal-origin"};
  host.selected->surface = SurfaceId{"personal-pane"};
  host.selected->selection = SelectionId{"personal-selection"};
  const auto personal_request = Prepare(model, host, Command::kSiteSettings);
  REQUIRE(personal_request.target.page_target.profile == kPersonal);
  REQUIRE(!RevalidateCommand(model, personal_request, host));
  REQUIRE(RevalidateCommand(model, request, host) ==
          Refusal::kSpaceNotSelected);
  REQUIRE(model.pages().at(kPage).profile == kWork);
  REQUIRE(model.windows().size() == 2);
}

void CloseInvalidatesCommandWithoutRemovingSavedItem() {
  auto model = Workspace();
  const auto host = NativeContext();
  const auto request = Prepare(model, host, Command::kPrint);
  REQUIRE(model.ReportActivity(kPage, lc::Activity::kQuiescent) ==
          lc::Status::kOk);
  REQUIRE(model.ClosePage(kPage) == lc::Status::kOk);
  REQUIRE(RevalidateCommand(model, request, host) == Refusal::kPageNotFound);
  REQUIRE(model.items().count(kItem) == 1);
  REQUIRE(model.pages().count(kSibling) == 1);
}

}  // namespace
}  // namespace arcified::site_controls

int main() {
  using namespace arcified::site_controls;
  ReadyCommandsDoNotMutateOwnership();
  NativeSelectionAndIdentityRequired();
  WrongLifecycleIdentityRefused();
  AvailabilityAndAnchorFailures();
  NavigationSecurityAndSurfaceChanges();
  SameUrlSplitFocusIsNotInferred();
  SavedTransferInvalidatesCapturedCommand();
  ProfileSwitchCannotRetargetOldAction();
  CloseInvalidatesCommandWithoutRemovingSavedItem();
  std::cout << "site-controls: 9 behavioral fixtures passed\n";
}
