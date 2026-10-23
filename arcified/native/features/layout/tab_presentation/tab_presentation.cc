#include "tab_presentation.h"

#include <cmath>
#include <cstdlib>

namespace arcified::tab_presentation {
namespace {

bool Valid(Extent extent) {
  return std::isfinite(extent.width) && std::isfinite(extent.height) &&
         extent.width >= 0 && extent.height >= 0;
}

bool Fits(Extent available, Extent required) {
  return available.width >= required.width &&
         available.height >= required.height;
}

Orientation Alternate(Orientation orientation) {
  switch (orientation) {
    case Orientation::kVertical:
      return Orientation::kHorizontal;
    case Orientation::kHorizontal:
      return Orientation::kVertical;
  }
  std::abort();
}

Extent Required(Orientation orientation, const FitMetrics& metrics) {
  switch (orientation) {
    case Orientation::kVertical:
      return metrics.vertical_required;
    case Orientation::kHorizontal:
      return metrics.horizontal_required;
  }
  std::abort();
}

}  // namespace

std::string_view Describe(Orientation orientation) {
  switch (orientation) {
    case Orientation::kVertical:
      return "vertical";
    case Orientation::kHorizontal:
      return "horizontal";
  }
  std::abort();
}

std::string_view Describe(Density density) {
  switch (density) {
    case Density::kCompact:
      return "compact";
    case Density::kComfortable:
      return "comfortable";
  }
  std::abort();
}

Presentation::Presentation(AuthoritativeTabs tabs,
                           Preferences requested,
                           sidebar::Side side,
                           Orientation effective)
    : tabs_(tabs), requested_(requested), side_(side), effective_(effective) {}

sidebar::PresentationOverride Presentation::sidebar_override(
    sidebar::PresentationOverride chrome_override) const {
  switch (effective_) {
    case Orientation::kVertical:
      return chrome_override;
    case Orientation::kHorizontal:
      return sidebar::PresentationOverride::kSuppress;
  }
  std::abort();
}

std::optional<RowPresentation> Presentation::RowAt(std::size_t index) const {
  const auto& order = tabs_.order.get();
  if (index >= order.size())
    return std::nullopt;
  const auto& identity = order[index];
  const auto& selected = tabs_.selected.get();
  return RowPresentation{std::cref(identity), selected && *selected == identity,
                         index + 1, order.size()};
}

ProjectionResult Project(AuthoritativeTabs tabs,
                         Preferences preferences,
                         sidebar::Side side,
                         FitMetrics metrics) {
  if (!Valid(metrics.available) || !Valid(metrics.vertical_required) ||
      !Valid(metrics.horizontal_required)) {
    return ProjectionError::kInvalidMetrics;
  }
  if (Fits(metrics.available, Required(preferences.orientation, metrics))) {
    return Presentation(tabs, preferences, side, preferences.orientation);
  }
  const Orientation alternate = Alternate(preferences.orientation);
  if (Fits(metrics.available, Required(alternate, metrics)))
    return Presentation(tabs, preferences, side, alternate);
  return ProjectionError::kNoUsableLayout;
}

}  // namespace arcified::tab_presentation
