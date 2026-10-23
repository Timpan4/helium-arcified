#include "capture.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace arcified::capture {
namespace {

bool Valid(Scope scope) {
  switch (scope) {
    case Scope::kVisiblePane:
    case Scope::kFullPage:
      return true;
  }
  return false;
}

bool Valid(Provider provider) {
  switch (provider) {
    case Provider::kUnavailable:
    case Provider::kVisiblePane:
    case Provider::kFullPage:
      return true;
  }
  return false;
}

bool Valid(ExportAction action) {
  switch (action) {
    case ExportAction::kCopy:
    case ExportAction::kSave:
      return true;
  }
  return false;
}

bool ValidSize(CssSize size) {
  return std::isfinite(size.width) && std::isfinite(size.height) &&
         size.width > 0 && size.height > 0;
}

std::optional<std::uint32_t> PixelExtent(double css, double scale) {
  const double pixels = std::ceil(css * scale);
  if (!std::isfinite(pixels) || pixels < 1 ||
      pixels > std::numeric_limits<std::uint32_t>::max())
    return std::nullopt;
  return static_cast<std::uint32_t>(pixels);
}

std::optional<std::size_t> ByteCount(PixelSize size) {
  if (!size.width || !size.height)
    return std::nullopt;
  // RGBA8's four channels and size_t representation, not a product quota.
  const auto max = std::numeric_limits<std::size_t>::max();
  const std::size_t width = size.width;
  if (width > max / 4 || size.height > max / 4 / width)
    return std::nullopt;
  const auto bytes = width * size.height * 4;
  if (bytes > std::vector<std::uint8_t>().max_size())
    return std::nullopt;
  return bytes;
}

Error CheckPermission(Permission permission) {
  switch (permission) {
    case Permission::kAllowed:
      return Error::kOk;
    case Permission::kDenied:
      return Error::kRestricted;
    case Permission::kUnknown:
      return Error::kPolicyRequired;
  }
  return Error::kPolicyRequired;
}

Error FreshPlan(const lifecycle::Model& model,
                const HostFacts& facts,
                const Plan& plan) {
  auto fresh = Prepare(model, facts, plan.request);
  if (const auto* error = std::get_if<Error>(&fresh))
    return *error;
  const auto& current = std::get<Plan>(fresh);
  if (!(current.target == plan.target) ||
      !(current.raster_size == plan.raster_size) ||
      current.css_size.width != plan.css_size.width ||
      current.css_size.height != plan.css_size.height)
    return Error::kStaleSource;
  return Error::kOk;
}

Image Crop(Image image, PixelRect crop) {
  Image result{{crop.width, crop.height}, {}};
  result.rgba.resize(*ByteCount(result.size));
  const std::size_t row_bytes = static_cast<std::size_t>(crop.width) * 4;
  for (std::uint32_t y = 0; y < crop.height; ++y) {
    const std::size_t source =
        (static_cast<std::size_t>(crop.y + y) * image.size.width + crop.x) * 4;
    std::copy_n(image.rgba.begin() + source, row_bytes,
                result.rgba.begin() + static_cast<std::size_t>(y) * row_bytes);
  }
  return result;
}

}  // namespace

PlanResult Prepare(const lifecycle::Model& model,
                   const HostFacts& facts,
                   Request request) {
  const auto& target = facts.target;
  if (!Valid(request.scope) || !Valid(facts.provider) ||
      target.document.empty() || target.visual_revision.empty() ||
      !ValidSize(facts.viewport) || !ValidSize(facts.document) ||
      !std::isfinite(request.pixels_per_css_pixel) ||
      request.pixels_per_css_pixel <= 0)
    return Error::kInvalidInput;
  const auto page = model.pages().find(target.page);
  const auto window = model.windows().find(target.attachment.window);
  if (page == model.pages().end() || page->second.profile != target.profile ||
      !(page->second.attachment() == target.attachment) ||
      window == model.windows().end() ||
      window->second.selected_space != target.attachment.space ||
      !facts.selected_and_visible)
    return Error::kTargetUnavailable;
  if (target.split) {
    const auto split = model.splits().find(*target.split);
    if (split == model.splits().end() ||
        !(split->second.attachment == target.attachment) ||
        std::find(split->second.pages.begin(), split->second.pages.end(),
                  target.page) == split->second.pages.end())
      return Error::kTargetUnavailable;
  }
  const auto permission = CheckPermission(facts.policy.capture);
  if (permission != Error::kOk)
    return permission;
  if (!facts.policy.exact_target_content_attested)
    return Error::kRestricted;
  if (facts.provider == Provider::kUnavailable)
    return Error::kProviderUnavailable;
  if (request.scope == Scope::kFullPage &&
      facts.provider != Provider::kFullPage)
    return Error::kFullPageUnavailable;
  const CssSize css =
      request.scope == Scope::kFullPage ? facts.document : facts.viewport;
  const auto width = PixelExtent(css.width, request.pixels_per_css_pixel);
  const auto height = PixelExtent(css.height, request.pixels_per_css_pixel);
  if (!width || !height)
    return Error::kUnrepresentableImage;
  const PixelSize raster{*width, *height};
  if (!ByteCount(raster))
    return Error::kUnrepresentableImage;
  PixelSize preview = raster;
  if (request.region) {
    const auto& region = *request.region;
    const auto& crop = region.crop;
    if (request.scope != Scope::kVisiblePane || !(region.target == target) ||
        !(region.raster_size == raster) || !crop.width || !crop.height ||
        crop.x >= raster.width || crop.y >= raster.height ||
        crop.width > raster.width - crop.x ||
        crop.height > raster.height - crop.y)
      return Error::kInvalidRegion;
    preview = {crop.width, crop.height};
  }
  return Plan{target, std::move(request), css, raster, preview};
}

std::variant<Error, CaptureTicket> Session::Begin(const lifecycle::Model& model,
                                                  const HostFacts& facts,
                                                  Request request) {
  if (!std::holds_alternative<std::monostate>(state_))
    return Error::kBusy;
  auto result = Prepare(model, facts, std::move(request));
  if (const auto* error = std::get_if<Error>(&result))
    return *error;
  if (generation_ == std::numeric_limits<std::uint64_t>::max())
    return Error::kTokenExhausted;
  CaptureTicket ticket{Token{++generation_}, std::get<Plan>(std::move(result))};
  state_ = Capturing{ticket.token, ticket.plan};
  return ticket;
}

Error Session::CompleteCapture(Token token,
                               const lifecycle::Model& model,
                               const HostFacts& fresh_facts,
                               CaptureResult result) {
  const auto* capturing = std::get_if<Capturing>(&state_);
  if (!capturing || !(token == capturing->token))
    return Error::kStaleOperation;
  if (const auto* failure = std::get_if<NativeFailure>(&result)) {
    if (*failure != NativeFailure::kCancelled &&
        *failure != NativeFailure::kFailed)
      return Error::kInvalidInput;
  }
  Plan plan = capturing->plan;
  state_ = std::monostate{};
  if (const auto* failure = std::get_if<NativeFailure>(&result))
    return *failure == NativeFailure::kCancelled ? Error::kNativeCancelled
                                                 : Error::kNativeFailed;
  const auto fresh = FreshPlan(model, fresh_facts, plan);
  if (fresh != Error::kOk)
    return fresh;
  auto image = std::get<Image>(std::move(result));
  if (!(image.size == plan.raster_size) ||
      image.rgba.size() != *ByteCount(plan.raster_size))
    return Error::kInvalidImage;
  if (plan.request.region)
    image = Crop(std::move(image), plan.request.region->crop);
  state_ = Ready{std::make_shared<const Preview>(
      Preview{std::move(plan), std::move(image)})};
  return Error::kOk;
}

std::shared_ptr<const Preview> Session::preview() const {
  if (const auto* ready = std::get_if<Ready>(&state_))
    return ready->preview;
  if (const auto* exporting = std::get_if<Exporting>(&state_))
    return exporting->preview;
  return nullptr;
}

Error Session::Revalidate(const lifecycle::Model& model,
                          const HostFacts& fresh_facts) {
  if (const auto* capturing = std::get_if<Capturing>(&state_)) {
    const auto fresh = FreshPlan(model, fresh_facts, capturing->plan);
    if (fresh != Error::kOk)
      state_ = std::monostate{};
    return fresh;
  }
  const auto image = preview();
  if (!image)
    return Error::kNoPreview;
  auto fresh = FreshPlan(model, fresh_facts, image->plan);
  if (const auto* exporting = std::get_if<Exporting>(&state_)) {
    if (fresh == Error::kOk) {
      fresh = CheckPermission(exporting->action == ExportAction::kCopy
                                  ? fresh_facts.policy.copy
                                  : fresh_facts.policy.save);
    }
  }
  if (fresh != Error::kOk) {
    if (auto* exporting = std::get_if<Exporting>(&state_)) {
      // Preserve the pending native outcome; do not invent successful
      // cancellation of an external side effect. Drop local pixel ownership.
      exporting->preview.reset();
    } else {
      state_ = std::monostate{};
    }
  }
  return fresh;
}

std::variant<Error, ExportTicket> Session::BeginExport(
    ExportAction action,
    const lifecycle::Model& model,
    const HostFacts& fresh_facts) {
  if (!Valid(action))
    return Error::kInvalidInput;
  if (std::holds_alternative<Exporting>(state_) ||
      std::holds_alternative<Capturing>(state_))
    return Error::kBusy;
  if (!preview())
    return Error::kNoPreview;
  const auto fresh = Revalidate(model, fresh_facts);
  if (fresh != Error::kOk)
    return fresh;
  const auto permission =
      CheckPermission(action == ExportAction::kCopy ? fresh_facts.policy.copy
                                                    : fresh_facts.policy.save);
  if (permission != Error::kOk)
    return permission;
  if (generation_ == std::numeric_limits<std::uint64_t>::max())
    return Error::kTokenExhausted;
  ExportTicket ticket{Token{++generation_}, action, preview()};
  state_ = Exporting{ticket.token, ticket.action, ticket.preview};
  return ticket;
}

Error Session::CompleteExport(Token token, NativeExportResult result) {
  switch (result) {
    case NativeExportResult::kSucceeded:
    case NativeExportResult::kFailed:
    case NativeExportResult::kCancelled:
      break;
    default:
      return Error::kInvalidInput;
  }
  const auto* exporting = std::get_if<Exporting>(&state_);
  if (!exporting || !(token == exporting->token))
    return Error::kStaleOperation;
  const auto image = exporting->preview;
  state_ = image ? decltype(state_){Ready{image}}
                 : decltype(state_){std::monostate{}};
  switch (result) {
    case NativeExportResult::kSucceeded:
      return Error::kOk;
    case NativeExportResult::kFailed:
      return Error::kNativeFailed;
    case NativeExportResult::kCancelled:
      return Error::kNativeCancelled;
  }
  return Error::kNativeFailed;
}

Error Session::Cancel() {
  if (std::holds_alternative<Exporting>(state_))
    return Error::kBusy;
  state_ = std::monostate{};
  return Error::kOk;
}

}  // namespace arcified::capture
