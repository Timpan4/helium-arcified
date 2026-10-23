#include "peek.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <vector>

namespace arcified::peek {
namespace {

using lifecycle::Activity;
using lifecycle::Attachment;
using lifecycle::ItemId;
using lifecycle::LinkRoutingRefusal;
using lifecycle::Model;
using lifecycle::NavigationKind;
using lifecycle::PageId;
using lifecycle::ProfileId;
using lifecycle::ShortcutId;
using lifecycle::SiteRelation;
using lifecycle::SpaceId;
using lifecycle::Status;
using lifecycle::TemporaryTabId;
using lifecycle::WindowId;

static_assert(lifecycle::kContractVersion == 3);
static_assert(!std::is_default_constructible_v<Settings>);

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
const Attachment kHere{kFirst, kDesk};
const Attachment kElsewhere{kSecond, kDesk};
const Attachment kOtherSpace{kFirst, kResearch};
const Attachment kOtherProfile{kFirst, kHome};
const ItemId kSaved{"saved"};
const PageId kParent{"parent"};
const PageId kPreview{"preview"};
const PresentationReferences kReferences{kParent, kPreview};
const std::string kSavedDestination = "https://parent.example.test/start";
const std::string kCurrentNavigation = "https://parent.example.test/draft";
const std::string kDestination = "https://destination.example.test/form";
const Settings kDisabled{false, false, false};
const Settings kAllEnabled{true, true, true};

void ExpectOpen(const lifecycle::OpenResult& result, const PageId& expected) {
  REQUIRE(std::holds_alternative<lifecycle::OpenedPage>(result));
  REQUIRE(std::get<lifecycle::OpenedPage>(result).page == expected);
}

Model Workspace() {
  Model model;
  REQUIRE(model.AddProfile(kWork) == Status::kOk);
  REQUIRE(model.AddProfile(kPersonal) == Status::kOk);
  REQUIRE(model.AddSpace(kDesk, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kResearch, kWork) == Status::kOk);
  REQUIRE(model.AddSpace(kHome, kPersonal) == Status::kOk);
  REQUIRE(model.AddWindow(kFirst) == Status::kOk);
  REQUIRE(model.AddWindow(kSecond) == Status::kOk);
  REQUIRE(model.Attach(kHere) == Status::kOk);
  REQUIRE(model.Attach(kElsewhere) == Status::kOk);
  REQUIRE(model.Attach(kOtherSpace) == Status::kOk);
  REQUIRE(model.Attach(kOtherProfile) == Status::kOk);
  REQUIRE(model.AddItem(kSaved, kDesk, kSavedDestination) == Status::kOk);
  ExpectOpen(model.OpenItem(kSaved, kHere, kParent), kParent);
  REQUIRE(model.RecordNavigation(kParent, kCurrentNavigation) == Status::kOk);
  REQUIRE(model.ReportActivity(kParent, Activity::kProtected) == Status::kOk);
  return model;
}

EligibilityResult Evaluate(const Model& model,
                           const Settings& settings,
                           Invocation invocation = Invocation::kAutomatic) {
  return EvaluateLink(model, kParent, NavigationKind::kLink,
                      SiteRelation::kOutbound, kDestination, invocation,
                      settings);
}

void ExpectPolicy(const EligibilityResult& result, PolicyRefusal expected) {
  REQUIRE(std::holds_alternative<PolicyRefusal>(result));
  REQUIRE(std::get<PolicyRefusal>(result) == expected);
}

void ExpectRouting(const EligibilityResult& result,
                   LinkRoutingRefusal expected) {
  REQUIRE(std::holds_alternative<LinkRoutingRefusal>(result));
  REQUIRE(std::get<LinkRoutingRefusal>(result) == expected);
}

void SettingsAndExistingFavoriteAliases() {
  Model model = Workspace();
  ExpectPolicy(Evaluate(model, kDisabled),
               PolicyRefusal::kAutomaticDisabledForSource);
  ExpectPolicy(Evaluate(model, kDisabled, Invocation::kExplicitModifier),
               PolicyRefusal::kExplicitDisabled);
  REQUIRE(std::holds_alternative<Eligible>(
      Evaluate(model, Settings{true, false, false})));
  ExpectPolicy(Evaluate(model, Settings{false, true, false}),
               PolicyRefusal::kAutomaticDisabledForSource);
  ExpectPolicy(Evaluate(model, Settings{true, true, false},
                        Invocation::kExplicitModifier),
               PolicyRefusal::kExplicitDisabled);
  REQUIRE(std::holds_alternative<Eligible>(Evaluate(
      model, Settings{false, false, true}, Invocation::kExplicitModifier)));
  ExpectPolicy(Evaluate(model, Settings{false, false, true}),
               PolicyRefusal::kAutomaticDisabledForSource);

  // An alias to another item must not classify this parent as a Favorite.
  const ItemId other{"other"};
  const ShortcutId unrelated{"unrelated"};
  REQUIRE(model.AddItem(other, kDesk, kSavedDestination) == Status::kOk);
  REQUIRE(model.AddShortcut(unrelated, kWork, other) == Status::kOk);
  ExpectPolicy(Evaluate(model, Settings{false, true, false}),
               PolicyRefusal::kAutomaticDisabledForSource);

  const ShortcutId alias{"alias"};
  const ShortcutId second_alias{"second-alias"};
  REQUIRE(model.AddShortcut(alias, kWork, kSaved) == Status::kOk);
  REQUIRE(model.AddShortcut(second_alias, kWork, kSaved) == Status::kOk);
  REQUIRE(std::holds_alternative<Eligible>(
      Evaluate(model, Settings{false, true, false})));
  REQUIRE(model.RemoveShortcut(alias) == Status::kOk);
  REQUIRE(std::holds_alternative<Eligible>(
      Evaluate(model, Settings{false, true, false})));
  REQUIRE(model.RemoveShortcut(second_alias) == Status::kOk);
  ExpectPolicy(Evaluate(model, Settings{false, true, false}),
               PolicyRefusal::kAutomaticDisabledForSource);

  // A standalone Profile shortcut has the same eligibility without an alias.
  const ItemId favorite{"favorite"};
  const PageId favorite_page{"favorite-page"};
  REQUIRE(model.AddItem(favorite, kWork, kSavedDestination) == Status::kOk);
  ExpectOpen(model.OpenItem(favorite, kOtherSpace, favorite_page),
             favorite_page);
  REQUIRE(std::holds_alternative<Eligible>(EvaluateLink(
      model, favorite_page, NavigationKind::kLink, SiteRelation::kOutbound,
      kDestination, Invocation::kAutomatic, Settings{false, true, false})));
  ExpectPolicy(
      EvaluateLink(model, favorite_page, NavigationKind::kLink,
                   SiteRelation::kOutbound, kDestination,
                   Invocation::kAutomatic, Settings{true, false, false}),
      PolicyRefusal::kAutomaticDisabledForSource);
  REQUIRE(model.pages().size() == 2);
  REQUIRE(model.items().size() == 3);
  REQUIRE(model.pages().at(kParent).current_navigation == kCurrentNavigation);
  REQUIRE(model.pages().at(kParent).activity == Activity::kProtected);
}

void RoutingScopeCannotBeOverridden() {
  Model model = Workspace();
  REQUIRE(model.OpenTemporary(kPreview, TemporaryTabId{"preview-tab"}, kHere,
                              kDestination) == Status::kOk);
  for (const auto invocation :
       {Invocation::kAutomatic, Invocation::kExplicitModifier}) {
    ExpectRouting(EvaluateLink(model, PageId{"missing"}, NavigationKind::kLink,
                               SiteRelation::kOutbound, kDestination,
                               invocation, kAllEnabled),
                  LinkRoutingRefusal::kSourceNotFound);
    ExpectRouting(EvaluateLink(model, kPreview, NavigationKind::kLink,
                               SiteRelation::kOutbound, kDestination,
                               invocation, kAllEnabled),
                  LinkRoutingRefusal::kSourceNotSaved);
    ExpectRouting(
        EvaluateLink(model, kParent, NavigationKind::kLink,
                     SiteRelation::kOutbound, "", invocation, kAllEnabled),
        LinkRoutingRefusal::kInvalidDestination);
    ExpectRouting(EvaluateLink(model, kParent, NavigationKind::kLink,
                               SiteRelation::kSameSite, kDestination,
                               invocation, kAllEnabled),
                  LinkRoutingRefusal::kSameSitePolicyOpen);
    ExpectRouting(EvaluateLink(model, kParent, NavigationKind::kLink,
                               SiteRelation::kUnknown, kDestination, invocation,
                               kAllEnabled),
                  LinkRoutingRefusal::kClassificationRequired);
    for (const auto kind : {NavigationKind::kRedirect, NavigationKind::kScript,
                            NavigationKind::kTyped, NavigationKind::kOther}) {
      ExpectRouting(EvaluateLink(model, kParent, kind, SiteRelation::kOutbound,
                                 kDestination, invocation, kAllEnabled),
                    LinkRoutingRefusal::kOutsideApprovedScope);
    }
    // URL equality cannot replace the authoritative browser classification.
    REQUIRE(std::holds_alternative<Eligible>(EvaluateLink(
        model, kParent, NavigationKind::kLink, SiteRelation::kOutbound,
        kCurrentNavigation, invocation, kAllEnabled)));
  }
  REQUIRE(model.pages().size() == 2);
  REQUIRE(model.pages().at(kParent).current_navigation == kCurrentNavigation);
  REQUIRE(model.pages().at(kParent).profile == kWork);
  REQUIRE(model.pages().at(kParent).activity == Activity::kProtected);
  REQUIRE(model.items().at(kSaved).saved_destination == kSavedDestination);
}

void ReferenceBoundaries() {
  Model model = Workspace();
  REQUIRE(model.OpenTemporary(kPreview, TemporaryTabId{"preview-tab"}, kHere,
                              kDestination) == Status::kOk);
  REQUIRE(CheckReferences(model, kReferences, kHere) ==
          ReferenceStatus::kCompatible);
  REQUIRE(CheckReferences(model, {kParent, kParent}, kHere) ==
          ReferenceStatus::kSamePage);
  REQUIRE(CheckReferences(model, kReferences, {WindowId{"missing"}, kDesk}) ==
          ReferenceStatus::kAttachmentMissing);
  REQUIRE(CheckReferences(model, {PageId{"missing"}, kPreview}, kHere) ==
          ReferenceStatus::kParentMissing);
  REQUIRE(CheckReferences(model, {kParent, PageId{"missing"}}, kHere) ==
          ReferenceStatus::kPreviewMissing);
  REQUIRE(CheckReferences(model, {kPreview, kParent}, kHere) ==
          ReferenceStatus::kParentNotSaved);

  const ItemId another_saved{"another-saved"};
  const PageId another_page{"another-page"};
  REQUIRE(model.AddItem(another_saved, kDesk, kDestination) == Status::kOk);
  ExpectOpen(model.OpenItem(another_saved, kHere, another_page), another_page);
  REQUIRE(CheckReferences(model, {kParent, another_page}, kHere) ==
          ReferenceStatus::kPreviewNotTemporary);

  const PageId personal_preview{"personal-preview"};
  REQUIRE(model.OpenTemporary(personal_preview, TemporaryTabId{"personal-tab"},
                              kOtherProfile, kDestination) == Status::kOk);
  REQUIRE(CheckReferences(model, {kParent, personal_preview}, kHere) ==
          ReferenceStatus::kProfileMismatch);

  const PageId remote_preview{"remote-preview"};
  REQUIRE(model.OpenTemporary(remote_preview, TemporaryTabId{"remote-tab"},
                              kElsewhere, kDestination) == Status::kOk);
  REQUIRE(CheckReferences(model, {kParent, remote_preview}, kHere) ==
          ReferenceStatus::kAttachmentMismatch);
  const PageId other_space_preview{"other-space-preview"};
  REQUIRE(model.OpenTemporary(other_space_preview, TemporaryTabId{"space-tab"},
                              kOtherSpace, kDestination) == Status::kOk);
  REQUIRE(CheckReferences(model, {kParent, other_space_preview}, kHere) ==
          ReferenceStatus::kAttachmentMismatch);
  REQUIRE(CheckReferences(model, kReferences, kOtherSpace) ==
          ReferenceStatus::kAttachmentMismatch);

  // Keeping the preview is an external lifecycle action. It invalidates this
  // temporary-only presentation instead of creating another page or item.
  REQUIRE(model.Keep(kPreview, ItemId{"kept-preview"}) == Status::kOk);
  REQUIRE(CheckReferences(model, kReferences, kHere) ==
          ReferenceStatus::kPreviewNotTemporary);
}

void TransferCloseAndNavigationRequireRechecks() {
  Model model = Workspace();
  REQUIRE(model.OpenTemporary(kPreview, TemporaryTabId{"preview-tab"}, kHere,
                              kDestination) == Status::kOk);
  const lifecycle::SplitId split{"existing-layout"};
  REQUIRE(model.SetSplit(split, kHere, {kParent, kPreview}) == Status::kOk);
  REQUIRE(CheckReferences(model, kReferences, kHere) ==
          ReferenceStatus::kCompatible);
  REQUIRE(std::holds_alternative<Eligible>(Evaluate(model, kAllEnabled)));
  REQUIRE(model.splits().at(split).pages ==
          std::vector<PageId>({kParent, kPreview}));

  // Unknown/protected activity is retained, never interpreted as permission
  // to dismiss. Even quiescent graph compatibility grants no close authority.
  for (const auto activity :
       {Activity::kUnknown, Activity::kProtected, Activity::kQuiescent}) {
    REQUIRE(model.ReportActivity(kPreview, activity) == Status::kOk);
    REQUIRE(CheckReferences(model, kReferences, kHere) ==
            ReferenceStatus::kCompatible);
    REQUIRE(model.pages().at(kPreview).activity == activity);
    REQUIRE(model.pages().at(kPreview).current_navigation == kDestination);
    REQUIRE(model.pages().at(kPreview).profile == kWork);
    REQUIRE(model.pages().at(kPreview).attachment() == kHere);
  }
  REQUIRE(model.pages().at(kParent).activity == Activity::kProtected);
  REQUIRE(model.items().at(kSaved).saved_destination == kSavedDestination);
  REQUIRE(model.pages().size() == 2);

  // A graph check deliberately cannot detect a document change, establish
  // native focus or certify a native TabModel/WebContents generation.
  REQUIRE(model.RecordNavigation(kPreview, kCurrentNavigation) == Status::kOk);
  REQUIRE(CheckReferences(model, kReferences, kHere) ==
          ReferenceStatus::kCompatible);
  REQUIRE(model.pages().at(kPreview).activity == Activity::kUnknown);

  REQUIRE(model.TransferSavedPage(kParent, kElsewhere) == Status::kOk);
  REQUIRE(CheckReferences(model, kReferences, kHere) ==
          ReferenceStatus::kAttachmentMismatch);
  REQUIRE(CheckReferences(model, kReferences, kElsewhere) ==
          ReferenceStatus::kAttachmentMismatch);
  REQUIRE(std::holds_alternative<Eligible>(Evaluate(model, kAllEnabled)));
  REQUIRE(model.pages().at(kParent).current_navigation == kCurrentNavigation);
  REQUIRE(model.pages().at(kPreview).attachment() == kHere);
  REQUIRE(model.splits().at(split).pages == std::vector<PageId>({kPreview}));

  // Model close commands below are test stimuli, not Peek dismiss behavior.
  REQUIRE(model.ReportActivity(kPreview, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kPreview) == Status::kOk);
  REQUIRE(CheckReferences(model, kReferences, kHere) ==
          ReferenceStatus::kPreviewMissing);
  REQUIRE(model.pages().count(kParent) == 1);
  REQUIRE(model.ReportActivity(kParent, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kParent) == Status::kOk);
  REQUIRE(CheckReferences(model, kReferences, kHere) ==
          ReferenceStatus::kParentMissing);
}

}  // namespace
}  // namespace arcified::peek

int main() {
  arcified::peek::SettingsAndExistingFavoriteAliases();
  arcified::peek::RoutingScopeCannotBeOverridden();
  arcified::peek::ReferenceBoundaries();
  arcified::peek::TransferCloseAndNavigationRequireRechecks();
  std::cout << "PASS: Peek eligibility and reference fixtures\n";
}
