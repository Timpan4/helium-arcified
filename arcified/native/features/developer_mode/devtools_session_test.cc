#include "devtools_session.h"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <type_traits>
#include <utility>
#include <vector>

namespace arcified::developer_mode {
namespace {

using namespace lifecycle;
static_assert(kContractVersion == 3);
static_assert(!std::is_convertible_v<DocumentId, NativeInstanceId>);

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
const SpaceId kHome{"home"};
const WindowId kFirst{"first"};
const WindowId kSecond{"second"};
const Attachment kHere{kFirst, kDesk};
const Attachment kThere{kSecond, kDesk};
const Attachment kHomeHere{kFirst, kHome};
const PageId kEditor{"editor"};
const PageId kPreview{"preview"};
const PageId kPersonalPage{"personal-page"};
const ItemId kSaved{"saved-editor"};
const std::string kDestination = "https://editor.example.test/";

Target Editor() {
  return {{kEditor, kWork, kHere},
          NativeInstanceId{"native-editor"},
          DocumentId{"document-editor"},
          BindingId{"binding-editor"}};
}

Target Preview() {
  return {{kPreview, kWork, kHere},
          NativeInstanceId{"native-preview"},
          DocumentId{"document-preview"},
          BindingId{"binding-preview"}};
}

Observation Visible(Target target) {
  return {std::move(target), true, true};
}

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kPersonal) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kHome, kPersonal) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  REQUIRE(model.Attach(kHere) == Status::kOk);
  REQUIRE(model.Attach(kThere) == Status::kOk);
  REQUIRE(model.Attach(kHomeHere) == Status::kOk);
  REQUIRE(model.AddItem(kSaved, kDesk, kDestination) == Status::kOk);
  REQUIRE(std::holds_alternative<OpenedPage>(
      model.OpenItem(kSaved, kHere, kEditor)));
  REQUIRE(model.OpenTemporary(kPreview, TemporaryTabId{"preview-tab"}, kHere,
                              kDestination) == Status::kOk);
  REQUIRE(model.OpenTemporary(kPersonalPage, TemporaryTabId{"personal-tab"},
                              kHomeHere, kDestination) == Status::kOk);
  REQUIRE(model.SetSplit(SplitId{"split"}, kHere, {kEditor, kPreview}) ==
          Status::kOk);
  return model;
}

struct Probe {
  int starts = 0;
  std::optional<Target> subscribed_target;
  std::optional<UpdateToken> token;
  std::vector<EndReason> cancellations;
  Cleanup acknowledgement = Cleanup::kConfirmedStopped;
  bool fail = false;
  std::function<void()> during_cancel;
};

class Subscription final : public DiagnosticSubscription {
 public:
  explicit Subscription(Probe& probe) : probe_(probe) {}
  Cleanup Cancel(EndReason reason) noexcept override {
    probe_.cancellations.push_back(reason);
    if (probe_.during_cancel)
      probe_.during_cancel();
    return probe_.acknowledgement;
  }

 private:
  Probe& probe_;
};

class Source final : public DiagnosticSource {
 public:
  explicit Source(Probe& probe) : probe_(probe) {}
  std::unique_ptr<DiagnosticSubscription> Subscribe(
      const Target& target,
      UpdateToken token) override {
    ++probe_.starts;
    probe_.subscribed_target = target;
    probe_.token = std::move(token);
    if (probe_.fail)
      return nullptr;
    return std::make_unique<Subscription>(probe_);
  }

 private:
  Probe& probe_;
};

DiagnosticEvent Event(const Probe& probe, Target target = Editor()) {
  REQUIRE(probe.token.has_value());
  return {std::move(target), *probe.token};
}

std::unique_ptr<DevToolsSession> Start(const Model& model,
                                       Source& source,
                                       Target target = Editor()) {
  auto result = DevToolsSession::Start(model, target, Visible(target), source);
  REQUIRE(std::holds_alternative<std::unique_ptr<DevToolsSession>>(result));
  return std::move(std::get<std::unique_ptr<DevToolsSession>>(result));
}

void ExactPaneAndProfile() {
  auto model = Workspace();
  Probe editor_probe;
  Probe preview_probe;
  Source editor_source(editor_probe);
  Source preview_source(preview_probe);
  auto editor = Start(model, editor_source);
  auto preview = Start(model, preview_source, Preview());
  REQUIRE(editor_probe.subscribed_target->page.page == kEditor);
  REQUIRE(preview_probe.subscribed_target->page.page == kPreview);
  REQUIRE(editor_probe.subscribed_target->page.profile == kWork);
  REQUIRE(preview_probe.subscribed_target->instance == Preview().instance);
  REQUIRE(editor->AcceptUpdate(model, Event(editor_probe), Visible(Editor())) ==
          Check::kAccepted);
  REQUIRE(preview->AcceptUpdate(model, Event(preview_probe, Preview()),
                                Visible(Preview())) == Check::kAccepted);

  // A logical Profile Space switch does not rebind either diagnostic session.
  // It also cannot stand in for a host visibility notification.
  REQUIRE(model.SelectSpace(kFirst, kHome) == Status::kOk);
  auto personal = Editor();
  personal.page.page = kPersonalPage;
  personal.page.profile = kPersonal;
  REQUIRE(editor->AcceptUpdate(model, Event(editor_probe, personal),
                               Visible(personal)) == Check::kWrongPage);
  REQUIRE(preview->AcceptUpdate(model, Event(preview_probe, Preview()),
                                Visible(Preview())) == Check::kAccepted);
  REQUIRE(editor_probe.cancellations.size() == 1);
  REQUIRE(preview_probe.cancellations.empty());
  REQUIRE(model.pages().at(kEditor).current_navigation == kDestination);
  REQUIRE(model.pages().at(kEditor).profile == kWork);
  REQUIRE(model.pages().at(kEditor).attachment() == kHere);
  REQUIRE(model.pages().size() == 3);
}

std::vector<std::pair<Check, Observation>> WrongObservations() {
  std::vector<std::pair<Check, Observation>> cases;
  auto observed = Visible(Editor());
  observed.target.page.page = kPreview;
  cases.emplace_back(Check::kWrongPage, observed);
  observed = Visible(Editor());
  observed.target.page.profile = kPersonal;
  cases.emplace_back(Check::kWrongProfile, observed);
  observed = Visible(Editor());
  observed.target.page.attachment = kThere;
  cases.emplace_back(Check::kWrongAttachment, observed);
  observed = Visible(Editor());
  observed.target.instance = NativeInstanceId{"replacement"};
  cases.emplace_back(Check::kWrongNativeInstance, observed);
  observed = Visible(Editor());
  observed.target.binding = BindingId{"rebound"};
  cases.emplace_back(Check::kWrongBinding, observed);
  observed = Visible(Editor());
  observed.target.document = DocumentId{"new-document"};
  cases.emplace_back(Check::kWrongDocument, observed);
  observed = Visible(Editor());
  observed.target.document = DocumentId{""};
  cases.emplace_back(Check::kInvalidNativeIdentity, observed);
  observed = Visible(Editor());
  observed.target.instance = NativeInstanceId{""};
  cases.emplace_back(Check::kInvalidNativeIdentity, observed);
  observed = Visible(Editor());
  observed.target.binding = BindingId{""};
  cases.emplace_back(Check::kInvalidNativeIdentity, observed);
  observed = Visible(Editor());
  observed.inspection_allowed = false;
  cases.emplace_back(Check::kInspectionDenied, observed);
  observed = Visible(Editor());
  observed.tools_visible = false;
  cases.emplace_back(Check::kToolsHidden, observed);
  return cases;
}

void RefuseBeforeSubscriptionAndEndOnStaleEvents() {
  const auto model = Workspace();
  for (const auto& [expected, observed] : WrongObservations()) {
    Probe probe;
    Source source(probe);
    auto refused = DevToolsSession::Start(model, Editor(), observed, source);
    REQUIRE(std::holds_alternative<Check>(refused));
    REQUIRE(std::get<Check>(refused) == expected);
    REQUIRE(probe.starts == 0);
    auto session = Start(model, source);
    REQUIRE(session->AcceptUpdate(model, Event(probe), observed) == expected);
    REQUIRE(session->AcceptUpdate(model, Event(probe), Visible(Editor())) ==
            Check::kSessionEnded);
    REQUIRE(session->cleanup() == Cleanup::kConfirmedStopped);
    REQUIRE(probe.cancellations ==
            std::vector<EndReason>{EndReason::kTargetInvalidated});
  }
  // Agreement between two stale/incorrect native inputs never overrides Model.
  Probe probe;
  Source source(probe);
  auto target = Editor();
  target.page.profile = kPersonal;
  auto result = DevToolsSession::Start(model, target, Visible(target), source);
  REQUIRE(std::get<Check>(result) == Check::kWrongProfile);
  REQUIRE(probe.starts == 0);
}

void ModelRejectsWrongOrGoneBindings() {
  auto model = Workspace();
  Probe probe;
  Source source(probe);
  auto target = Editor();
  target.page.attachment = kHomeHere;
  auto result = DevToolsSession::Start(model, target, Visible(target), source);
  REQUIRE(std::get<Check>(result) == Check::kWrongAttachment);
  target = Editor();
  target.page.page = PageId{"missing"};
  result = DevToolsSession::Start(model, target, Visible(target), source);
  REQUIRE(std::get<Check>(result) == Check::kPageNotFound);
  REQUIRE(probe.starts == 0);

  target = Editor();
  target.page.profile = ProfileId{""};
  result = DevToolsSession::Start(model, target, Visible(target), source);
  REQUIRE(std::get<Check>(result) == Check::kInvalidPageTarget);
  target = Editor();
  target.page.attachment = {WindowId{"missing"}, kDesk};
  result = DevToolsSession::Start(model, target, Visible(target), source);
  REQUIRE(std::get<Check>(result) == Check::kWrongAttachment);
  REQUIRE(probe.starts == 0);

  auto transferred = Start(model, source);
  REQUIRE(model.TransferSavedPage(kEditor, kThere) == Status::kOk);
  REQUIRE(transferred->AcceptUpdate(model, Event(probe), Visible(Editor())) ==
          Check::kWrongAttachment);
  REQUIRE(probe.cancellations.size() == 1);

  Probe gone_probe;
  Source gone_source(gone_probe);
  target = Editor();
  target.page.attachment = kThere;
  auto gone = Start(model, gone_source, target);
  REQUIRE(model.ReportActivity(kEditor, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kEditor) == Status::kOk);
  REQUIRE(gone->AcceptUpdate(model, Event(gone_probe, target),
                             Visible(target)) == Check::kPageNotFound);
  REQUIRE(gone_probe.cancellations.size() == 1);
  REQUIRE(model.items().count(kSaved) == 1);
  REQUIRE(model.pages().count(kPreview) == 1);
}

void CleanupAcknowledgementsAndReentry() {
  const auto model = Workspace();
  for (const auto reason : {EndReason::kClosed, EndReason::kHidden,
                            EndReason::kNativeTargetReplaced}) {
    for (const auto reply :
         {Cleanup::kConfirmedStopped, Cleanup::kUnconfirmed}) {
      Probe probe;
      probe.acknowledgement = reply;
      Source source(probe);
      auto session = Start(model, source);
      probe.during_cancel = [&] {
        REQUIRE(session->AcceptUpdate(model, Event(probe), Visible(Editor())) ==
                Check::kSessionEnded);
        REQUIRE(session->End(EndReason::kClosed) == Cleanup::kUnconfirmed);
      };
      REQUIRE(session->End(reason) == reply);
      REQUIRE(session->cleanup() == reply);
      REQUIRE(session->End(EndReason::kHidden) == reply);
      REQUIRE(session->AcceptUpdate(model, Event(probe), Visible(Editor())) ==
              Check::kSessionEnded);
      session.reset();
      REQUIRE(probe.cancellations == std::vector<EndReason>{reason});
    }
  }
  Probe probe;
  Source source(probe);
  {
    auto session = Start(model, source);
  }
  REQUIRE(probe.cancellations ==
          std::vector<EndReason>{EndReason::kSessionDestroyed});
}

void FailedSubscriptionLeavesNoSession() {
  const auto model = Workspace();
  Probe probe;
  probe.fail = true;
  Source source(probe);
  auto result =
      DevToolsSession::Start(model, Editor(), Visible(Editor()), source);
  REQUIRE(std::get<Check>(result) == Check::kSubscriptionFailed);
  REQUIRE(probe.starts == 1);
  REQUIRE(probe.cancellations.empty());
}

void NavigationInvalidationAndNewSession() {
  auto model = Workspace();
  Probe old_probe;
  Source old_source(old_probe);
  auto old = Start(model, old_source);

  // Required host ordering, including a same-URL reload: invalidate first,
  // report lifecycle commit, then discard any queued old diagnostic events.
  REQUIRE(old->End(EndReason::kNavigationCommitted) ==
          Cleanup::kConfirmedStopped);
  REQUIRE(model.RecordNavigation(kEditor, kDestination) == Status::kOk);
  REQUIRE(old->AcceptUpdate(model, Event(old_probe), Visible(Editor())) ==
          Check::kSessionEnded);
  REQUIRE(old_probe.cancellations ==
          std::vector<EndReason>{EndReason::kNavigationCommitted});

  auto new_target = Editor();
  new_target.document = DocumentId{"new-document-same-url"};
  Probe new_probe;
  Source new_source(new_probe);
  auto current = Start(model, new_source, new_target);
  REQUIRE(current->AcceptUpdate(model, Event(new_probe, new_target),
                                Visible(new_target)) == Check::kAccepted);
  REQUIRE(current->AcceptUpdate(model, Event(new_probe), Visible(new_target)) ==
          Check::kWrongDocument);
  REQUIRE(new_probe.cancellations.size() == 1);
  REQUIRE(old->AcceptUpdate(model, Event(old_probe, new_target),
                            Visible(new_target)) == Check::kSessionEnded);
}

void EventStampsAndSubscriptionProvenance() {
  const auto model = Workspace();
  for (const auto& [expected, observed] : WrongObservations()) {
    if (expected == Check::kToolsHidden || expected == Check::kInspectionDenied)
      continue;
    Probe probe;
    Source source(probe);
    auto session = Start(model, source);
    // Native facts can be fresh while the delivered event's stamp is stale.
    REQUIRE(session->AcceptUpdate(model, Event(probe, observed.target),
                                  Visible(Editor())) == expected);
    REQUIRE(probe.cancellations.size() == 1);
  }

  Probe old_probe;
  Source old_source(old_probe);
  auto old = Start(model, old_source);
  const auto queued_old_event = Event(old_probe);
  REQUIRE(old->End(EndReason::kHidden) == Cleanup::kConfirmedStopped);
  Probe current_probe;
  Source current_source(current_probe);
  auto current = Start(model, current_source);
  REQUIRE(current->AcceptUpdate(model, queued_old_event, Visible(Editor())) ==
          Check::kWrongSubscription);
  REQUIRE(current_probe.cancellations.empty());
  REQUIRE(current->AcceptUpdate(model, Event(current_probe),
                                Visible(Editor())) == Check::kAccepted);
}

void NativeBindingGenerationDetectsTransferABA() {
  auto model = Workspace();
  Probe probe;
  Source source(probe);
  auto session = Start(model, source);
  REQUIRE(model.TransferSavedPage(kEditor, kThere) == Status::kOk);
  REQUIRE(model.TransferSavedPage(kEditor, kHere) == Status::kOk);
  // Lifecycle and native instance/document are unchanged after the round trip.
  // Only the host's fresh binding generation distinguishes this observation.
  auto rebound = Editor();
  rebound.binding = BindingId{"binding-after-round-trip"};
  REQUIRE(session->AcceptUpdate(model, Event(probe), Visible(rebound)) ==
          Check::kWrongBinding);
  REQUIRE(probe.cancellations.size() == 1);

  Probe next_probe;
  Source next_source(next_probe);
  auto next = Start(model, next_source, rebound);
  REQUIRE(next->AcceptUpdate(model, Event(next_probe, rebound),
                             Visible(rebound)) == Check::kAccepted);
  // The host must also invalidate synchronously before a binding changes.
  // This rejects queued callbacks even if a caller later passes stale facts.
  REQUIRE(next->End(EndReason::kNativeTargetReplaced) ==
          Cleanup::kConfirmedStopped);
  REQUIRE(next->AcceptUpdate(model, Event(next_probe, rebound),
                             Visible(rebound)) == Check::kSessionEnded);
}

}  // namespace
}  // namespace arcified::developer_mode

int main() {
  using namespace arcified::developer_mode;
  ExactPaneAndProfile();
  RefuseBeforeSubscriptionAndEndOnStaleEvents();
  ModelRejectsWrongOrGoneBindings();
  CleanupAcknowledgementsAndReentry();
  FailedSubscriptionLeavesNoSession();
  NavigationInvalidationAndNewSession();
  EventStampsAndSubscriptionProvenance();
  NativeBindingGenerationDetectsTransferABA();
  std::cout << "developer-mode session behavioral checks passed\n";
}
