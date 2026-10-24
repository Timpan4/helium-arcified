#include "sidebar.h"

#include <algorithm>
#include <cmath>

namespace arcified::sidebar {
namespace {

bool Nonnegative(double value) {
  return std::isfinite(value) && value >= 0;
}

bool Positive(double value) {
  return std::isfinite(value) && value > 0;
}

bool ValidContext(const Context& context) {
  const auto& bounds = context.available;
  return std::isfinite(bounds.x) && std::isfinite(bounds.y) &&
         Nonnegative(bounds.width) && Nonnegative(bounds.height) &&
         std::isfinite(bounds.x + bounds.width) &&
         std::isfinite(bounds.y + bounds.height) &&
         Positive(context.minimum_width) && Positive(context.maximum_width) &&
         context.maximum_width >= context.minimum_width &&
         Nonnegative(context.minimum_content_width) &&
         Positive(context.reveal_hit_width) &&
         Positive(context.resize_hit_width);
}

std::optional<Rect> HitTarget(const Rect& bounds, double center, double width) {
  if (width > bounds.width)
    return std::nullopt;
  const double x =
      std::clamp(center - width / 2, bounds.x, bounds.x + bounds.width - width);
  return Rect{x, bounds.y, width, bounds.height};
}

}  // namespace

bool Rect::Contains(double point_x, double point_y) const {
  return point_x >= x && point_y >= y && point_x < x + width &&
         point_y < y + height;
}

bool operator==(const Rect& a, const Rect& b) {
  return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}

bool operator==(const Context& a, const Context& b) {
  return a.available == b.available && a.side == b.side &&
         a.minimum_width == b.minimum_width &&
         a.maximum_width == b.maximum_width &&
         a.minimum_content_width == b.minimum_content_width &&
         a.reveal_hit_width == b.reveal_hit_width &&
         a.resize_hit_width == b.resize_hit_width &&
         a.presentation_override == b.presentation_override;
}

std::optional<Presentation> Presentation::Create(Preferences preferences,
                                                 Context context) {
  if (!Positive(preferences.preferred_width) || !ValidContext(context))
    return std::nullopt;
  return Presentation(preferences, context);
}

Presentation::Presentation(Preferences preferences, Context context)
    : preferences_(preferences), context_(context) {}

double Presentation::ConstrainWidth(double width) const {
  const double maximum = std::min(
      context_.maximum_width,
      std::max(0.0, context_.available.width - context_.minimum_content_width));
  if (maximum < context_.minimum_width)
    return 0;
  return std::clamp(width, context_.minimum_width, maximum);
}

bool Presentation::HasRoom() const {
  return context_.available.height > 0 &&
         ConstrainWidth(preferences_.preferred_width) > 0;
}

bool Presentation::CanPresent() const {
  return context_.presentation_override != PresentationOverride::kSuppress &&
         HasRoom();
}

Layout Presentation::layout() const {
  Layout result{Mode::kHidden,
                HasRoom() ? WidthFit::kFits : WidthFit::kInsufficientSpace,
                context_.available,
                std::nullopt,
                std::nullopt,
                std::nullopt,
                resize_.has_value()};
  if (!CanPresent())
    return result;
  const bool left = context_.side == Side::kLeft;
  const auto& bounds = context_.available;
  const bool forced_overlay =
      context_.presentation_override == PresentationOverride::kRevealOverlay;
  if (!forced_overlay && !preferences_.persistent_visible &&
      reveal_ == Reveal::kNone) {
    result.reveal_target =
        HitTarget(bounds, left ? bounds.x : bounds.x + bounds.width,
                  context_.reveal_hit_width);
    return result;
  }

  result.mode = !forced_overlay && preferences_.persistent_visible
                    ? Mode::kDocked
                    : Mode::kOverlay;
  const double width = ConstrainWidth(resize_ ? resize_->draft_width
                                              : preferences_.preferred_width);
  const double sidebar_x = left ? bounds.x : bounds.x + bounds.width - width;
  result.sidebar = Rect{sidebar_x, bounds.y, width, bounds.height};
  if (result.mode == Mode::kDocked) {
    result.content.width -= width;
    if (left)
      result.content.x += width;
  }
  // Preserve the caller's hit width by shifting inward near an edge. A metric
  // wider than available space cannot be honored and has no pointer target.
  const double boundary = left ? sidebar_x + width : sidebar_x;
  result.resize_target = HitTarget(bounds, boundary, context_.resize_hit_width);
  return result;
}

Effect Presentation::FinishChange(bool was_visible,
                                  FocusLocation focus,
                                  bool release_capture,
                                  bool preferences_changed) {
  const bool hidden = layout().mode == Mode::kHidden;
  return {Status::kOk,
          was_visible && hidden && focus == FocusLocation::kInsideSidebar
              ? FocusRequest::kPageContent
              : FocusRequest::kNone,
          release_capture ? CaptureRequest::kRelease : CaptureRequest::kNone,
          preferences_changed};
}

Effect Presentation::Configure(Context context, FocusLocation focus) {
  if (!ValidContext(context))
    return {Status::kInvalidInput};
  if (context == context_)
    return {};
  const bool was_visible = layout().mode != Mode::kHidden;
  const bool release = resize_.has_value();
  resize_.reset();
  // Never resurrect stale hover/keyboard reveal after a host mode change.
  if (context.presentation_override != context_.presentation_override)
    reveal_ = Reveal::kNone;
  context_ = context;
  if (!CanPresent())
    reveal_ = Reveal::kNone;
  return FinishChange(was_visible, focus, release, false);
}

Effect Presentation::Dispatch(Action action, FocusLocation focus) {
  const bool was_visible = layout().mode != Mode::kHidden;
  const bool was_resizing = resize_.has_value();
  bool changed = false;
  switch (action) {
    case Action::kTogglePersistent:
      preferences_.persistent_visible = !preferences_.persistent_visible;
      changed = true;
      reveal_ = Reveal::kNone;
      resize_.reset();
      break;
    case Action::kPointerEnter:
      if (CanPresent() && reveal_ != Reveal::kEngaged)
        reveal_ = Reveal::kPointer;
      break;
    case Action::kPointerLeave:
      if (reveal_ == Reveal::kPointer && !resize_) {
        reveal_ = focus == FocusLocation::kInsideSidebar ? Reveal::kEngaged
                                                         : Reveal::kNone;
      }
      break;
    case Action::kKeyboardReveal:
      if (!CanPresent())
        return {Status::kUnavailable};
      reveal_ = Reveal::kEngaged;
      return {Status::kOk, FocusRequest::kSidebar};
    case Action::kLeave:
      reveal_ = Reveal::kNone;
      resize_.reset();
      break;
    case Action::kFocusLeft:
      if (focus == FocusLocation::kInsideSidebar)
        return {Status::kInvalidInput};
      reveal_ = Reveal::kNone;
      resize_.reset();
      break;
    case Action::kEscape:
      if (resize_)
        return CancelResize();
      reveal_ = Reveal::kNone;
      break;
    case Action::kCaptureLost:
      // Also drop temporary reveal, since the pointer may now be elsewhere.
      reveal_ = Reveal::kNone;
      resize_.reset();
      break;
  }
  auto effect =
      FinishChange(was_visible, focus, was_resizing && !resize_, changed);
  if ((action == Action::kLeave || action == Action::kEscape) && was_visible &&
      focus == FocusLocation::kInsideSidebar) {
    effect.focus = FocusRequest::kPageContent;
  }
  return effect;
}

Effect Presentation::BeginResize(double pointer_x, double pointer_y) {
  if (!std::isfinite(pointer_x) || !std::isfinite(pointer_y))
    return {Status::kInvalidInput};
  if (resize_)
    return {Status::kResizeActive};
  const auto current = layout();
  if (!current.sidebar || !current.resize_target ||
      !current.resize_target->Contains(pointer_x, pointer_y)) {
    return {Status::kUnavailable};
  }
  const double width = current.sidebar->width;
  resize_ = Resize{pointer_x, width, width};
  return {Status::kOk, FocusRequest::kNone, CaptureRequest::kAcquire};
}

Effect Presentation::UpdateResize(double pointer_x) {
  if (!std::isfinite(pointer_x))
    return {Status::kInvalidInput};
  if (!resize_)
    return {Status::kNoResize};
  const double delta = context_.side == Side::kLeft
                           ? pointer_x - resize_->start_pointer_x
                           : resize_->start_pointer_x - pointer_x;
  resize_->draft_width = ConstrainWidth(resize_->start_width + delta);
  return {};
}

Effect Presentation::CommitResize() {
  if (!resize_)
    return {Status::kNoResize};
  // A no-movement click must not overwrite a width temporarily constrained by
  // a small window. Only a changed visible draft is a preference edit.
  const bool changed = resize_->draft_width != resize_->start_width &&
                       resize_->draft_width != preferences_.preferred_width;
  if (changed)
    preferences_.preferred_width = resize_->draft_width;
  resize_.reset();
  return {Status::kOk, FocusRequest::kNone, CaptureRequest::kRelease, changed};
}

Effect Presentation::CancelResize() {
  if (!resize_)
    return {Status::kNoResize};
  resize_.reset();
  return {Status::kOk, FocusRequest::kNone, CaptureRequest::kRelease};
}

}  // namespace arcified::sidebar
