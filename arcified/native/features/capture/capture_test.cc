#include "capture.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {
namespace c = arcified::capture;
namespace l = arcified::lifecycle;

void Check(bool condition, const char* expression) {
  if (!condition)
    throw std::runtime_error(expression);
}
#define CHECK(expression) Check(static_cast<bool>(expression), #expression)

template <typename T, typename... Types>
T Value(std::variant<Types...> result) {
  CHECK(std::holds_alternative<T>(result));
  return std::get<T>(std::move(result));
}

struct Fixture {
  l::Model model;
  l::ProfileId profile{"profile"};
  l::Attachment attachment{l::WindowId{"window"}, l::SpaceId{"space"}};
  l::PageId page{"page"};
  c::HostFacts facts{
      {page, profile, attachment, std::nullopt, "document", "visual"},
      {4, 3},
      {4, 10000},
      {c::Permission::kAllowed, c::Permission::kAllowed,
       c::Permission::kAllowed, true},
      c::Provider::kFullPage,
      true};
  c::Request visible{c::Scope::kVisiblePane, 1, std::nullopt};

  Fixture() {
    CHECK(model.AddProfile(profile) == l::Status::kOk);
    CHECK(model.AddSpace(attachment.space, profile) == l::Status::kOk);
    CHECK(model.AddWindow(attachment.window) == l::Status::kOk);
    CHECK(model.Attach(attachment) == l::Status::kOk);
    CHECK(model.SelectSpace(attachment.window, attachment.space) ==
          l::Status::kOk);
    CHECK(model.OpenTemporary(page, l::TemporaryTabId{"tab"}, attachment,
                              "https://capture.test/") == l::Status::kOk);
  }

  c::Image Pixels(c::PixelSize size = {4, 3}) const {
    c::Image image{size, {}};
    for (std::uint32_t y = 0; y < size.height; ++y) {
      for (std::uint32_t x = 0; x < size.width; ++x) {
        image.rgba.push_back(static_cast<std::uint8_t>(x));
        image.rgba.push_back(static_cast<std::uint8_t>(y));
        image.rgba.push_back(17);
        image.rgba.push_back(255);
      }
    }
    return image;
  }

  c::CaptureTicket Start(c::Session& session) {
    return Value<c::CaptureTicket>(session.Begin(model, facts, visible));
  }
  void Preview(c::Session& session) {
    const auto ticket = Start(session);
    CHECK(session.CompleteCapture(ticket.token, model, facts, Pixels()) ==
          c::Error::kOk);
    CHECK(session.preview());
  }
};

void ScaleAndFullPage() {
  Fixture f;
  f.facts.viewport = {10.25, 3.5};
  auto request = f.visible;
  request.pixels_per_css_pixel = 1.5;
  const auto plan = Value<c::Plan>(c::Prepare(f.model, f.facts, request));
  CHECK((plan.raster_size == c::PixelSize{16, 6}));
  CHECK(plan.css_size.width == 10.25);
  CHECK(plan.request.pixels_per_css_pixel == 1.5);
  request.scope = c::Scope::kFullPage;
  const auto full = Value<c::Plan>(c::Prepare(f.model, f.facts, request));
  CHECK((full.raster_size == c::PixelSize{6, 15000}));
  CHECK(full.css_size.height == 10000);
  c::Session session;
  const auto ticket =
      Value<c::CaptureTicket>(session.Begin(f.model, f.facts, request));
  CHECK(session.CompleteCapture(ticket.token, f.model, f.facts,
                                f.Pixels({6, 15000})) == c::Error::kOk);
  CHECK((session.preview()->image.size == c::PixelSize{6, 15000}));
  CHECK(session.preview()->image.rgba.size() == 360000);
  f.facts.provider = c::Provider::kVisiblePane;
  CHECK(Value<c::Error>(c::Prepare(f.model, f.facts, request)) ==
        c::Error::kFullPageUnavailable);
  f.facts.provider = c::Provider::kUnavailable;
  CHECK(Value<c::Error>(c::Prepare(f.model, f.facts, f.visible)) ==
        c::Error::kProviderUnavailable);
}

void InvalidGeometry() {
  Fixture f;
  auto request = f.visible;
  for (double scale : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                       std::numeric_limits<double>::quiet_NaN()}) {
    request.pixels_per_css_pixel = scale;
    CHECK(Value<c::Error>(c::Prepare(f.model, f.facts, request)) ==
          c::Error::kInvalidInput);
  }
  request.pixels_per_css_pixel = std::numeric_limits<double>::max();
  CHECK(Value<c::Error>(c::Prepare(f.model, f.facts, request)) ==
        c::Error::kUnrepresentableImage);
  f.facts.viewport = {0, 3};
  CHECK(Value<c::Error>(c::Prepare(f.model, f.facts, f.visible)) ==
        c::Error::kInvalidInput);
  const double max = std::numeric_limits<std::uint32_t>::max();
  f.facts.viewport = {max, max};
  CHECK(Value<c::Error>(c::Prepare(f.model, f.facts, f.visible)) ==
        c::Error::kUnrepresentableImage);
}

void TargetAndPolicyBoundaries() {
  Fixture f;
  auto facts = f.facts;
  facts.selected_and_visible = false;
  CHECK(Value<c::Error>(c::Prepare(f.model, facts, f.visible)) ==
        c::Error::kTargetUnavailable);
  facts = f.facts;
  facts.target.profile = l::ProfileId{"other"};
  CHECK(Value<c::Error>(c::Prepare(f.model, facts, f.visible)) ==
        c::Error::kTargetUnavailable);
  facts = f.facts;
  facts.target.document.clear();
  CHECK(Value<c::Error>(c::Prepare(f.model, facts, f.visible)) ==
        c::Error::kInvalidInput);
  facts = f.facts;
  facts.policy.capture = c::Permission::kUnknown;
  CHECK(Value<c::Error>(c::Prepare(f.model, facts, f.visible)) ==
        c::Error::kPolicyRequired);
  facts.policy.capture = c::Permission::kDenied;
  CHECK(Value<c::Error>(c::Prepare(f.model, facts, f.visible)) ==
        c::Error::kRestricted);
  facts = f.facts;
  facts.policy.exact_target_content_attested = false;
  c::Session session;
  CHECK(Value<c::Error>(session.Begin(f.model, facts, f.visible)) ==
        c::Error::kRestricted);
  CHECK(!session.preview());
  CHECK(f.model.pages().size() == 1);
  CHECK(f.model.pages().at(f.page).current_navigation ==
        "https://capture.test/");
}

void SplitMembershipAndSpaceSelection() {
  Fixture f;
  f.facts.target.split = l::SplitId{"split"};
  CHECK(Value<c::Error>(c::Prepare(f.model, f.facts, f.visible)) ==
        c::Error::kTargetUnavailable);
  CHECK(f.model.SetSplit(*f.facts.target.split, f.attachment, {f.page}) ==
        l::Status::kOk);
  CHECK(
      std::holds_alternative<c::Plan>(c::Prepare(f.model, f.facts, f.visible)));
  CHECK(f.model.AddSpace(l::SpaceId{"other-space"}, f.profile) ==
        l::Status::kOk);
  CHECK(f.model.Attach({f.attachment.window, l::SpaceId{"other-space"}}) ==
        l::Status::kOk);
  CHECK(f.model.SelectSpace(f.attachment.window, l::SpaceId{"other-space"}) ==
        l::Status::kOk);
  CHECK(Value<c::Error>(c::Prepare(f.model, f.facts, f.visible)) ==
        c::Error::kTargetUnavailable);
}

void RegionCropAndIdentity() {
  Fixture f;
  f.visible.region = c::VisibleRegion{f.facts.target, {4, 3}, {1, 1, 2, 2}};
  c::Session session;
  const auto ticket = f.Start(session);
  CHECK((ticket.plan.raster_size == c::PixelSize{4, 3}));
  CHECK((ticket.plan.preview_size == c::PixelSize{2, 2}));
  CHECK(session.CompleteCapture(ticket.token, f.model, f.facts, f.Pixels()) ==
        c::Error::kOk);
  const auto preview = session.preview();
  CHECK((preview->image.size == c::PixelSize{2, 2}));
  CHECK((preview->image.rgba == std::vector<std::uint8_t>{1, 1, 17, 255, 2, 1,
                                                          17, 255, 1, 2, 17,
                                                          255, 2, 2, 17, 255}));
  auto bad = f.visible;
  bad.scope = c::Scope::kFullPage;
  CHECK(Value<c::Error>(c::Prepare(f.model, f.facts, bad)) ==
        c::Error::kInvalidRegion);
  bad = f.visible;
  bad.region->target.visual_revision = "old";
  CHECK(Value<c::Error>(c::Prepare(f.model, f.facts, bad)) ==
        c::Error::kInvalidRegion);
  bad = f.visible;
  bad.region->raster_size = {5, 3};
  CHECK(Value<c::Error>(c::Prepare(f.model, f.facts, bad)) ==
        c::Error::kInvalidRegion);
  for (const auto crop :
       {c::PixelRect{0, 0, 0, 1}, c::PixelRect{4, 0, 1, 1},
        c::PixelRect{1, 1, 4, 2},
        c::PixelRect{1, 0, std::numeric_limits<std::uint32_t>::max(), 1}}) {
    bad = f.visible;
    bad.region->crop = crop;
    CHECK(Value<c::Error>(c::Prepare(f.model, f.facts, bad)) ==
          c::Error::kInvalidRegion);
  }
}

void CancelAndStaleReplies() {
  Fixture f;
  c::Session session;
  const auto old = f.Start(session);
  CHECK(Value<c::Error>(session.Begin(f.model, f.facts, f.visible)) ==
        c::Error::kBusy);
  CHECK(session.Cancel() == c::Error::kOk);
  const auto current = f.Start(session);
  CHECK(session.CompleteCapture(old.token, f.model, f.facts, f.Pixels()) ==
        c::Error::kStaleOperation);
  CHECK(!session.preview());
  CHECK(session.CompleteCapture(current.token, f.model, f.facts, f.Pixels()) ==
        c::Error::kOk);
  CHECK(session.Cancel() == c::Error::kOk);
  CHECK(!session.preview());
}

void CaptureFailuresAndFreshness() {
  Fixture f;
  c::Session session;
  auto ticket = f.Start(session);
  auto malformed = f.Pixels();
  malformed.rgba.pop_back();
  CHECK(session.CompleteCapture(ticket.token, f.model, f.facts,
                                std::move(malformed)) ==
        c::Error::kInvalidImage);
  CHECK(!session.preview());
  ticket = f.Start(session);
  CHECK(session.CompleteCapture(ticket.token, f.model, f.facts,
                                c::NativeFailure::kFailed) ==
        c::Error::kNativeFailed);
  ticket = f.Start(session);
  CHECK(session.CompleteCapture(ticket.token, f.model, f.facts,
                                c::NativeFailure::kCancelled) ==
        c::Error::kNativeCancelled);
  for (const auto kind : {0, 1, 2, 3}) {
    ticket = f.Start(session);
    auto fresh = f.facts;
    if (kind == 0)
      fresh.target.document = "new-document";
    if (kind == 1)
      fresh.target.visual_revision = "scrolled";
    if (kind == 2)
      fresh.viewport.width = 5;
    if (kind == 3)
      fresh.policy.capture = c::Permission::kDenied;
    const auto expected =
        kind == 3 ? c::Error::kRestricted : c::Error::kStaleSource;
    CHECK(session.CompleteCapture(ticket.token, f.model, fresh, f.Pixels()) ==
          expected);
    CHECK(!session.preview());
  }
  ticket = f.Start(session);
  auto fresh = f.facts;
  fresh.target.visual_revision = "invalidated-before-reply";
  CHECK(session.Revalidate(f.model, fresh) == c::Error::kStaleSource);
  CHECK(session.CompleteCapture(ticket.token, f.model, f.facts, f.Pixels()) ==
        c::Error::kStaleOperation);
}

void RecoverableNativeExports() {
  Fixture f;
  c::Session session;
  f.Preview(session);
  const auto original = session.preview();
  auto restricted = f.facts;
  restricted.policy.copy = c::Permission::kDenied;
  CHECK(Value<c::Error>(
            session.BeginExport(c::ExportAction::kCopy, f.model, restricted)) ==
        c::Error::kRestricted);
  const auto first = Value<c::ExportTicket>(
      session.BeginExport(c::ExportAction::kSave, f.model, f.facts));
  CHECK(first.action == c::ExportAction::kSave);
  CHECK(first.preview == original);
  CHECK(session.Cancel() == c::Error::kBusy);
  CHECK(Value<c::Error>(session.BeginExport(c::ExportAction::kCopy, f.model,
                                            f.facts)) == c::Error::kBusy);
  CHECK(session.CompleteExport(first.token, c::NativeExportResult::kFailed) ==
        c::Error::kNativeFailed);
  CHECK(session.preview() == original);
  const auto second = Value<c::ExportTicket>(
      session.BeginExport(c::ExportAction::kCopy, f.model, f.facts));
  CHECK(
      session.CompleteExport(first.token, c::NativeExportResult::kSucceeded) ==
      c::Error::kStaleOperation);
  CHECK(
      session.CompleteExport(second.token, c::NativeExportResult::kCancelled) ==
      c::Error::kNativeCancelled);
  const auto third = Value<c::ExportTicket>(
      session.BeginExport(c::ExportAction::kCopy, f.model, f.facts));
  CHECK(session.CompleteExport(
            third.token, c::NativeExportResult::kSucceeded) == c::Error::kOk);
  CHECK(session.preview() == original);
}

void PreviewAndExportInvalidation() {
  Fixture f;
  c::Session session;
  f.Preview(session);
  auto changed = f.facts;
  changed.target.visual_revision = "resized";
  CHECK(Value<c::Error>(
            session.BeginExport(c::ExportAction::kSave, f.model, changed)) ==
        c::Error::kStaleSource);
  CHECK(!session.preview());
  f.Preview(session);
  const auto exporting = Value<c::ExportTicket>(
      session.BeginExport(c::ExportAction::kSave, f.model, f.facts));
  changed = f.facts;
  changed.policy.exact_target_content_attested = false;
  CHECK(session.Revalidate(f.model, changed) == c::Error::kRestricted);
  CHECK(!session.preview());
  CHECK(session.Cancel() == c::Error::kBusy);
  CHECK(session.CompleteExport(exporting.token,
                               c::NativeExportResult::kCancelled) ==
        c::Error::kNativeCancelled);
  CHECK(!session.preview());
  CHECK(Value<c::Error>(session.BeginExport(c::ExportAction::kSave, f.model,
                                            f.facts)) == c::Error::kNoPreview);

  f.Preview(session);
  const auto pending = Value<c::ExportTicket>(
      session.BeginExport(c::ExportAction::kCopy, f.model, f.facts));
  changed = f.facts;
  changed.policy.copy = c::Permission::kDenied;
  CHECK(session.Revalidate(f.model, changed) == c::Error::kRestricted);
  CHECK(!session.preview());
  CHECK(session.CompleteExport(pending.token,
                               c::NativeExportResult::kCancelled) ==
        c::Error::kNativeCancelled);
}

void UnrecognizedBoundaryValues() {
  Fixture f;
  auto request = f.visible;
  request.scope = static_cast<c::Scope>(-1);
  CHECK(Value<c::Error>(c::Prepare(f.model, f.facts, request)) ==
        c::Error::kInvalidInput);
  auto facts = f.facts;
  facts.provider = static_cast<c::Provider>(-1);
  CHECK(Value<c::Error>(c::Prepare(f.model, facts, f.visible)) ==
        c::Error::kInvalidInput);
  c::Session session;
  const auto ticket = f.Start(session);
  CHECK(session.CompleteCapture(ticket.token, f.model, f.facts,
                                static_cast<c::NativeFailure>(-1)) ==
        c::Error::kInvalidInput);
  CHECK(session.CompleteCapture(ticket.token, f.model, f.facts, f.Pixels()) ==
        c::Error::kOk);
  CHECK(Value<c::Error>(session.BeginExport(static_cast<c::ExportAction>(-1),
                                            f.model, f.facts)) ==
        c::Error::kInvalidInput);
  const auto exporting = Value<c::ExportTicket>(
      session.BeginExport(c::ExportAction::kSave, f.model, f.facts));
  CHECK(session.CompleteExport(exporting.token,
                               static_cast<c::NativeExportResult>(-1)) ==
        c::Error::kInvalidInput);
  CHECK(session.CompleteExport(exporting.token,
                               c::NativeExportResult::kCancelled) ==
        c::Error::kNativeCancelled);
}

}  // namespace

int main() {
  try {
    ScaleAndFullPage();
    InvalidGeometry();
    TargetAndPolicyBoundaries();
    SplitMembershipAndSpaceSelection();
    RegionCropAndIdentity();
    CancelAndStaleReplies();
    CaptureFailuresAndFreshness();
    RecoverableNativeExports();
    PreviewAndExportInvalidation();
    UnrecognizedBoundaryValues();
    std::cout << "capture fixtures passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
