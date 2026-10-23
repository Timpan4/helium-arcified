#include "command_target.h"

#include <cstdlib>
#include <iostream>

namespace arcified::commands {
namespace {

using namespace lifecycle;

void Require(bool condition, const char* expression, int line) {
  if (!condition) {
    std::cerr << "line " << line << ": " << expression << '\n';
    std::exit(EXIT_FAILURE);
  }
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
const Attachment kDeskSecond{kSecond, kDesk};
const Attachment kResearchFirst{kFirst, kResearch};
const Attachment kHomeFirst{kFirst, kHome};
const PageId kPage{"saved-page"};
const PageId kTemporary{"temporary-page"};
const ItemId kItem{"saved"};

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
  REQUIRE(model.Attach(kDeskSecond) == Status::kOk);
  REQUIRE(model.Attach(kResearchFirst) == Status::kOk);
  REQUIRE(model.Attach(kHomeFirst) == Status::kOk);
  REQUIRE(model.AddItem(kItem, kDesk, "https://saved.example.test/") ==
          Status::kOk);
  REQUIRE(std::holds_alternative<OpenedPage>(
      model.OpenItem(kItem, kDeskFirst, kPage)));
  REQUIRE(model.OpenTemporary(kTemporary, TemporaryTabId{"temporary"},
                              kResearchFirst,
                              "https://draft.example.test/") == Status::kOk);
  return model;
}

PageCommandTarget Resolve(const Model& model,
                          const PageId& page,
                          const Attachment& attachment) {
  auto result = ResolvePageCommandTarget(model, page, attachment);
  REQUIRE(std::holds_alternative<PageCommandTarget>(result));
  return std::get<PageCommandTarget>(result);
}

void ExplicitFocusAndReadOnlyResolution() {
  auto model = Workspace();
  REQUIRE(model.SelectSpace(kFirst, kHome) == Status::kOk);
  // Focus can refer to a caller-owned transient or split surface. The resolver
  // must never substitute a page from the selected Space, even across Profiles.
  const auto target = Resolve(model, kPage, kDeskFirst);
  REQUIRE(target.page == kPage);
  REQUIRE(target.profile == kWork);
  REQUIRE(target.attachment == kDeskFirst);
  const auto temporary = Resolve(model, kTemporary, kResearchFirst);
  REQUIRE(temporary.page == kTemporary);
  REQUIRE(temporary.profile == kWork);
  REQUIRE(model.windows().at(kFirst).selected_space == kHome);
  REQUIRE(model.pages().size() == 2);
  REQUIRE(model.items().size() == 1);
  REQUIRE(model.pages().at(kPage).current_navigation ==
          "https://saved.example.test/");
  REQUIRE(model.pages().at(kPage).attachment() == kDeskFirst);
}

void InvalidAndStaleTargets() {
  auto model = Workspace();
  const auto target = Resolve(model, kPage, kDeskFirst);
  REQUIRE(RevalidatePageCommandTarget(model, target) == TargetError::kOk);
  REQUIRE(std::get<TargetError>(ResolvePageCommandTarget(
              model, PageId{""}, kDeskFirst)) == TargetError::kInvalidInput);
  REQUIRE(std::get<TargetError>(
              ResolvePageCommandTarget(model, PageId{"missing"}, kDeskFirst)) ==
          TargetError::kPageNotFound);
  REQUIRE(std::get<TargetError>(ResolvePageCommandTarget(
              model, kPage, {WindowId{"missing"}, kDesk})) ==
          TargetError::kAttachmentNotFound);
  REQUIRE(std::get<TargetError>(ResolvePageCommandTarget(
              model, kPage, kHomeFirst)) == TargetError::kAttachmentMismatch);
  auto wrong_profile = target;
  wrong_profile.profile = kPersonal;
  REQUIRE(RevalidatePageCommandTarget(model, wrong_profile) ==
          TargetError::kProfileMismatch);
  wrong_profile.profile = ProfileId{""};
  REQUIRE(RevalidatePageCommandTarget(model, wrong_profile) ==
          TargetError::kInvalidInput);

  REQUIRE(model.TransferSavedPage(kPage, kDeskSecond) == Status::kOk);
  REQUIRE(RevalidatePageCommandTarget(model, target) ==
          TargetError::kAttachmentMismatch);
  const auto moved = Resolve(model, kPage, kDeskSecond);
  REQUIRE(moved.profile == target.profile);
  REQUIRE(moved.page == target.page);
  REQUIRE(model.TransferSavedPage(kPage, kDeskFirst) == Status::kOk);
  // Native generation is intentionally a separate required gate. A semantic
  // snapshot alone cannot detect this away-and-back or WebContents replacement.
  REQUIRE(RevalidatePageCommandTarget(model, target) == TargetError::kOk);
  REQUIRE(model.ReportActivity(kPage, Activity::kQuiescent) == Status::kOk);
  REQUIRE(model.ClosePage(kPage) == Status::kOk);
  REQUIRE(RevalidatePageCommandTarget(model, target) ==
          TargetError::kPageNotFound);
}

}  // namespace
}  // namespace arcified::commands

int main() {
  arcified::commands::ExplicitFocusAndReadOnlyResolution();
  arcified::commands::InvalidAndStaleTargets();
  std::cout << "command target fixtures passed\n";
}
