#include "space_themes.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <utility>

namespace arcified::space_themes {
namespace {

constexpr Rgb kBlack{0, 0, 0};
constexpr Rgb kWhite{255, 255, 255};

// WCAG 2.2 relative-luminance definition, using opaque eight-bit sRGB.
double LinearChannel(std::uint8_t channel) {
  const double srgb = channel / 255.0;
  return srgb <= 0.04045 ? srgb / 12.92 : std::pow((srgb + 0.055) / 1.055, 2.4);
}

double Luminance(Rgb color) {
  return 0.2126 * LinearChannel(color.red) +
         0.7152 * LinearChannel(color.green) +
         0.0722 * LinearChannel(color.blue);
}

Rgb ContrastingEndpoint(Rgb surface) {
  return ContrastRatio(kBlack, surface) >= ContrastRatio(kWhite, surface)
             ? kBlack
             : kWhite;
}

ColorScheme ResolveScheme(SchemePreference preference, ColorScheme system) {
  switch (preference) {
    case SchemePreference::kSystem:
      return system;
    case SchemePreference::kLight:
      return ColorScheme::kLight;
    case SchemePreference::kDark:
      return ColorScheme::kDark;
  }
  // Invalid enum representations must be rejected by the external data parser.
  std::abort();
}

}  // namespace

double ContrastRatio(Rgb first, Rgb second) {
  const double a = Luminance(first);
  const double b = Luminance(second);
  return (std::max(a, b) + 0.05) / (std::min(a, b) + 0.05);
}

Palette ResolvePalette(const Theme& theme, const Environment& environment) {
  if (environment.enforced_palette)
    return *environment.enforced_palette;

  const Rgb surface = ResolveScheme(theme.scheme, environment.system_scheme) ==
                              ColorScheme::kDark
                          ? theme.dark_surface
                          : theme.light_surface;
  const Rgb foreground = ContrastingEndpoint(surface);
  // Keep an already visible accent intact. Otherwise choose the endpoint that
  // contrasts most with the surface. Each surface gets its own readable text.
  // For opaque sRGB, max(black contrast, white contrast) >= sqrt(21) > 4.5.
  const Rgb selected =
      ContrastRatio(theme.accent, surface) >= kMinimumNonTextContrast
          ? theme.accent
          : foreground;
  return {surface, foreground, selected, ContrastingEndpoint(selected),
          foreground};
}

Preview::Preview(lifecycle::SpaceId space) : space_(std::move(space)) {}

void Preview::Set(Theme draft) {
  draft_ = draft;
}

void Preview::Revert() {
  draft_.reset();
}

std::optional<ThemeChange> Preview::proposal() const {
  if (!draft_)
    return std::nullopt;
  return ThemeChange{space_, *draft_};
}

Palette Preview::Resolve(const lifecycle::SpaceId& live_space,
                         const Theme& current_theme,
                         const Environment& environment) const {
  return ResolvePalette(
      live_space == space_ && draft_ ? *draft_ : current_theme, environment);
}

}  // namespace arcified::space_themes
