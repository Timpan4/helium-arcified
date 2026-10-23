#include "capture.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "../region_capture/region_capture.h"

namespace {
namespace c = arcified::capture;
namespace r = arcified::region_capture;
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

// This is a synthetic host conversion, not a production native adapter.
// Picker geometry is native-view DIP based; only its final raster coordinates
// cross this seam. Target/revision information is retained verbatim.
c::VisibleRegion Convert(const r::RegionSelection& selection) {
  const auto& source = selection.target;
  const auto& size = selection.mapping.raster_size;
  const auto& rect = selection.geometry.crop;
  CHECK(size.width > 0 && size.height > 0);
  CHECK(rect.left >= 0 && rect.top >= 0 && rect.right > rect.left &&
        rect.bottom > rect.top && rect.right <= size.width &&
        rect.bottom <= size.height);
  return {{source.page, source.profile, source.attachment, source.split,
           source.committed_document, source.visual_revision},
          {static_cast<std::uint32_t>(size.width),
           static_cast<std::uint32_t>(size.height)},
          {static_cast<std::uint32_t>(rect.left),
           static_cast<std::uint32_t>(rect.top),
           static_cast<std::uint32_t>(rect.right - rect.left),
           static_cast<std::uint32_t>(rect.bottom - rect.top)}};
}

void ComposedPickerCapture() {
  const l::ProfileId profile{"profile"};
  const l::Attachment attachment{l::WindowId{"window"}, l::SpaceId{"space"}};
  const l::PageId page{"pane"};
  const l::SplitId split{"split"};
  l::Model model;
  CHECK(model.AddProfile(profile) == l::Status::kOk);
  CHECK(model.AddSpace(attachment.space, profile) == l::Status::kOk);
  CHECK(model.AddWindow(attachment.window) == l::Status::kOk);
  CHECK(model.Attach(attachment) == l::Status::kOk);
  CHECK(model.SelectSpace(attachment.window, attachment.space) ==
        l::Status::kOk);
  CHECK(model.OpenTemporary(page, l::TemporaryTabId{"tab"}, attachment,
                            "https://capture.test/pane") == l::Status::kOk);
  CHECK(model.SetSplit(split, attachment, {page}) == l::Status::kOk);

  // Native view is 8x4 DIPs at an offset, raster is 12x6 pixels, while
  // CSS viewport is 6x3 at 2 pixels/CSS pixel. CSS and DIPs differ here.
  const r::PickerRequest picker_facts{
      {page, profile, attachment, split, "document", "surface"},
      {{100, 50, 108, 54}, {100, 50, 108, 54}, {12, 6}},
      r::Permission::kAllowed};
  auto picker = Value<r::Picker>(r::Picker::Start(picker_facts));
  CHECK(picker.SelectCorners({101, 50.5}, {105, 53.5}) == r::Status::kOk);
  const auto selected = Value<r::RegionSelection>(picker.Confirm(picker_facts));
  CHECK(selected.geometry.crop.left == 2);
  CHECK(selected.geometry.crop.top == 1);
  CHECK(selected.geometry.crop.right == 7);
  CHECK(selected.geometry.crop.bottom == 5);

  const auto region = Convert(selected);
  c::HostFacts capture_facts{region.target,
                             {6, 3},
                             {6, 30},
                             {c::Permission::kAllowed, c::Permission::kAllowed,
                              c::Permission::kAllowed, true},
                             c::Provider::kVisiblePane,
                             true};
  c::Request request{c::Scope::kVisiblePane, 2, region};
  c::Session session;
  const auto ticket =
      Value<c::CaptureTicket>(session.Begin(model, capture_facts, request));
  CHECK(ticket.plan.target == region.target);
  CHECK((ticket.plan.raster_size == c::PixelSize{12, 6}));
  CHECK((ticket.plan.preview_size == c::PixelSize{5, 4}));
  c::Image raster{{12, 6}, {}};
  for (std::uint8_t y = 0; y < 6; ++y) {
    for (std::uint8_t x = 0; x < 12; ++x)
      raster.rgba.insert(raster.rgba.end(), {x, y, 0, 255});
  }
  CHECK(session.CompleteCapture(ticket.token, model, capture_facts,
                                std::move(raster)) == c::Error::kOk);
  const auto preview = session.preview();
  CHECK(preview);
  const std::vector<std::uint8_t> expected{
      2, 1, 0, 255, 3, 1, 0, 255, 4, 1, 0, 255, 5, 1, 0, 255, 6, 1, 0, 255,
      2, 2, 0, 255, 3, 2, 0, 255, 4, 2, 0, 255, 5, 2, 0, 255, 6, 2, 0, 255,
      2, 3, 0, 255, 3, 3, 0, 255, 4, 3, 0, 255, 5, 3, 0, 255, 6, 3, 0, 255,
      2, 4, 0, 255, 3, 4, 0, 255, 4, 4, 0, 255, 5, 4, 0, 255, 6, 4, 0, 255};
  CHECK(preview->image.rgba == expected);

  // A previously confirmed picker cannot silently move to a new document or
  // a different raster scale, even if the host forgot to update its revision.
  auto changed = capture_facts;
  changed.target.document = "replacement";
  CHECK(Value<c::Error>(c::Prepare(model, changed, request)) ==
        c::Error::kInvalidRegion);
  auto scaled = request;
  scaled.pixels_per_css_pixel = 1;
  CHECK(Value<c::Error>(c::Prepare(model, capture_facts, scaled)) ==
        c::Error::kInvalidRegion);

  CHECK(session.Cancel() == c::Error::kOk);
  const auto pending =
      Value<c::CaptureTicket>(session.Begin(model, capture_facts, request));
  changed = capture_facts;
  changed.target.visual_revision = "new-surface";
  CHECK(session.CompleteCapture(
            pending.token, model, changed,
            c::Image{{12, 6}, std::vector<std::uint8_t>(288)}) ==
        c::Error::kInvalidRegion);
  CHECK(!session.preview());

  auto changed_picker = picker_facts;
  changed_picker.mapping.raster_size = {24, 12};
  auto stale_picker = Value<r::Picker>(r::Picker::Start(picker_facts));
  CHECK(stale_picker.SelectCorners({101, 50.5}, {105, 53.5}) == r::Status::kOk);
  CHECK(Value<r::Status>(stale_picker.Confirm(changed_picker)) ==
        r::Status::kTargetChanged);
}

}  // namespace

int main() {
  try {
    ComposedPickerCapture();
    std::cout << "capture/region seam fixture passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
