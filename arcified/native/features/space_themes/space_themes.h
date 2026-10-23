#ifndef ARCIFIED_NATIVE_FEATURES_SPACE_THEMES_SPACE_THEMES_H_
#define ARCIFIED_NATIVE_FEATURES_SPACE_THEMES_SPACE_THEMES_H_

#include <cstdint>
#include <optional>

#include "../../model/lifecycle.h"

namespace arcified::space_themes {

// Opaque, eight-bit sRGB. No alpha, material, CSS, or native resource handles.
struct Rgb {
  std::uint8_t red;
  std::uint8_t green;
  std::uint8_t blue;
  friend bool operator==(const Rgb& a, const Rgb& b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue;
  }
};

enum class ColorScheme { kLight, kDark };
enum class SchemePreference { kSystem, kLight, kDark };

// Caller-owned theme input, not a persisted Space record or schema.
struct Theme {
  SchemePreference scheme;
  Rgb light_surface;
  Rgb dark_surface;
  Rgb accent;
};

// Presentation roles for opaque Space chrome. Native controls, focus behavior,
// layout, and web content remain outside this component.
struct Palette {
  Rgb surface;
  Rgb on_surface;
  Rgb selected_surface;
  Rgb on_selected;
  Rgb separator;  // Contrasts with surface, not arbitrary web content.
  friend bool operator==(const Palette& a, const Palette& b) {
    return a.surface == b.surface && a.on_surface == b.on_surface &&
           a.selected_surface == b.selected_surface &&
           a.on_selected == b.on_selected && a.separator == b.separator;
  }
};

struct Environment {
  ColorScheme system_scheme;
  // Supply the host UI tree's authoritative palette when forced/high contrast
  // or policy must override custom colors. It is trusted and returned verbatim;
  // this component neither validates nor repairs native system colors.
  std::optional<Palette> enforced_palette;
};

// WCAG 2.2 1.4.3 and 1.4.11, also pinned Chromium ui/gfx/color_utils.h.
// See ../../../docs/features/space-themes.md for sources and scope.
inline constexpr double kMinimumTextContrast = 4.5;
inline constexpr double kMinimumNonTextContrast = 3.0;

// WCAG 2.2 sRGB relative luminance and contrast ratio, without rounding.
double ContrastRatio(Rgb first, Rgb second);
Palette ResolvePalette(const Theme& theme, const Environment& environment);

// An owned proposal for the caller to authorize and save, never a commit token.
struct ThemeChange {
  lifecycle::SpaceId space;
  Theme theme;
};

// An editor-local draft bound to an existing caller-validated SpaceId. No Space
// registry, durable baseline, lifecycle mutation, or global preview singleton.
// The caller serializes edits and confirms live identity before rendering or
// saving. A proposal does not grant permission to overwrite concurrent edits.
class Preview {
 public:
  explicit Preview(lifecycle::SpaceId space);

  void Set(Theme draft);
  void Revert();
  std::optional<ThemeChange> proposal() const;

  // current_theme belongs to live_space. A different Space never sees this
  // editor's draft. Revert exposes current caller state, not a saved snapshot.
  Palette Resolve(const lifecycle::SpaceId& live_space,
                  const Theme& current_theme,
                  const Environment& environment) const;

 private:
  lifecycle::SpaceId space_;
  std::optional<Theme> draft_;
};

}  // namespace arcified::space_themes

#endif  // ARCIFIED_NATIVE_FEATURES_SPACE_THEMES_SPACE_THEMES_H_
