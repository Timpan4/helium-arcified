#include "space_themes.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <type_traits>

namespace arcified::space_themes {
namespace {

static_assert(lifecycle::kContractVersion == 3);
static_assert(!std::is_constructible_v<Preview, lifecycle::ProfileId>);

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

constexpr Rgb kBlack{0, 0, 0};
constexpr Rgb kWhite{255, 255, 255};
constexpr Rgb kRed{255, 0, 0};
constexpr Rgb kBlue{0, 0, 255};
constexpr Rgb kGreen{0, 255, 0};
constexpr Theme kCurrent{SchemePreference::kSystem, kWhite, kBlack, kBlue};
constexpr Theme kDraft{SchemePreference::kSystem, kRed, kBlue, kGreen};
constexpr Environment kLight{ColorScheme::kLight, std::nullopt};
constexpr Environment kDark{ColorScheme::kDark, std::nullopt};
const lifecycle::SpaceId kDesk{"desk"};
const lifecycle::SpaceId kResearch{"research"};
const lifecycle::SpaceId kHome{"home"};

bool SameTheme(const Theme& a, const Theme& b) {
  return a.scheme == b.scheme && a.light_surface == b.light_surface &&
         a.dark_surface == b.dark_surface && a.accent == b.accent;
}

void ExpectLegible(const Palette& palette) {
  REQUIRE(ContrastRatio(palette.on_surface, palette.surface) >=
          kMinimumTextContrast);
  REQUIRE(ContrastRatio(palette.on_selected, palette.selected_surface) >=
          kMinimumTextContrast);
  REQUIRE(ContrastRatio(palette.selected_surface, palette.surface) >=
          kMinimumNonTextContrast);
  REQUIRE(ContrastRatio(palette.separator, palette.surface) >=
          kMinimumNonTextContrast);
}

void ContrastMathAndAccentPreservation() {
  REQUIRE(ContrastRatio(kBlack, kWhite) == 21.0);
  REQUIRE(ContrastRatio(kWhite, kBlack) == 21.0);
  REQUIRE(ContrastRatio(kRed, kRed) == 1.0);
  // Primary red is readable on black but not on white at the text threshold.
  REQUIRE(ContrastRatio(kRed, kBlack) > kMinimumTextContrast);
  REQUIRE(ContrastRatio(kRed, kWhite) < kMinimumTextContrast);
  REQUIRE(ContrastRatio(kRed, kGreen) < kMinimumNonTextContrast);

  // WCAG sRGB math places the maximum-contrast endpoint crossover between
  // these encoded grays. Treating encoded channels as linear would choose
  // black for both and fail the first expectation.
  const Theme below_crossover{
      SchemePreference::kLight, {117, 117, 117}, kBlack, kBlue};
  const Theme above_crossover{
      SchemePreference::kLight, {118, 118, 118}, kBlack, kBlue};
  REQUIRE(ResolvePalette(below_crossover, kLight).on_surface == kWhite);
  REQUIRE(ResolvePalette(above_crossover, kLight).on_surface == kBlack);

  // Adjacent eight-bit grays straddle 3:1 against white. The second must not
  // pass by rounding its ratio up to the threshold.
  const Rgb visible_gray{148, 148, 148};
  const Rgb insufficient_gray{149, 149, 149};
  const Theme visible{SchemePreference::kLight, kWhite, kBlack, visible_gray};
  const Theme insufficient{SchemePreference::kLight, kWhite, kBlack,
                           insufficient_gray};
  REQUIRE(ResolvePalette(visible, kLight).selected_surface == visible_gray);
  REQUIRE(ResolvePalette(insufficient, kLight).selected_surface == kBlack);

  const Rgb colors[] = {kBlack,         kWhite,        kRed,
                        kGreen,         kBlue,         {255, 255, 0},
                        {0, 255, 255},  {255, 0, 255}, {117, 117, 117},
                        {118, 118, 118}};
  for (Rgb surface : colors) {
    for (Rgb accent : colors) {
      const Theme theme{SchemePreference::kSystem, surface, surface, accent};
      const Palette palette = ResolvePalette(theme, kLight);
      REQUIRE(palette.surface == surface);
      ExpectLegible(palette);
      if (ContrastRatio(accent, surface) >= kMinimumNonTextContrast)
        REQUIRE(palette.selected_surface == accent);
    }
  }

  // Exercise every encoded gray, including both sides of the sRGB linear
  // segment and the black/white foreground crossover. Identical surface and
  // accent must be repaired even though the user's requested color is retained
  // in the draft itself.
  for (int value = 0; value <= std::numeric_limits<std::uint8_t>::max();
       ++value) {
    const auto gray = static_cast<std::uint8_t>(value);
    const Rgb surface{gray, gray, gray};
    const Theme theme{SchemePreference::kSystem, surface, surface, surface};
    ExpectLegible(ResolvePalette(theme, kLight));
  }

  const Theme light{SchemePreference::kLight, kWhite, kBlack, kWhite};
  const Palette repaired = ResolvePalette(light, kLight);
  REQUIRE(repaired.selected_surface == kBlack);
  REQUIRE(repaired.on_selected == kWhite);
  REQUIRE(light.accent == kWhite);
}

void SystemAndExplicitSchemes() {
  REQUIRE(ResolvePalette(kCurrent, kLight).surface == kWhite);
  REQUIRE(ResolvePalette(kCurrent, kDark).surface == kBlack);
  Theme theme = kCurrent;
  theme.scheme = SchemePreference::kLight;
  REQUIRE(ResolvePalette(theme, kDark).surface == kWhite);
  theme.scheme = SchemePreference::kDark;
  REQUIRE(ResolvePalette(theme, kLight).surface == kBlack);

  Preview preview{kDesk};
  preview.Set(kDraft);
  REQUIRE(preview.Resolve(kDesk, kCurrent, kLight).surface == kRed);
  REQUIRE(preview.Resolve(kDesk, kCurrent, kDark).surface == kBlue);
  REQUIRE(SameTheme(preview.proposal()->theme, kDraft));
}

void NativePaletteIsAuthoritative() {
  // Deliberately unusual colors prove byte-for-byte forwarding. Their native
  // accessibility and contrast are the caller's responsibility, not repaired
  // or certified by the normal-contrast algorithm.
  const Palette native{
      {9, 10, 11}, {9, 10, 11}, {80, 20, 90}, {1, 2, 3}, {44, 55, 66}};
  const Environment forced{ColorScheme::kDark, native};
  REQUIRE(ResolvePalette(kCurrent, forced) == native);
  Preview preview{kDesk};
  Theme theme = kDraft;
  theme.scheme = SchemePreference::kLight;
  preview.Set(theme);
  REQUIRE(preview.Resolve(kDesk, kCurrent, forced) == native);
  REQUIRE(preview.Resolve(kHome, kCurrent, forced) == native);
  REQUIRE(SameTheme(preview.proposal()->theme, theme));
  REQUIRE(preview.Resolve(kDesk, kCurrent, kLight) ==
          ResolvePalette(theme, kLight));
  preview.Revert();
  REQUIRE(preview.Resolve(kDesk, kCurrent, forced) == native);
}

void PreviewDoesNotOwnDurableState() {
  Preview preview{kDesk};
  REQUIRE(!preview.proposal());
  REQUIRE(preview.Resolve(kDesk, kCurrent, kLight) ==
          ResolvePalette(kCurrent, kLight));
  preview.Set(kDraft);
  auto proposal = preview.proposal();
  REQUIRE(proposal.has_value());
  REQUIRE(proposal->space == kDesk);
  REQUIRE(SameTheme(proposal->theme, kDraft));
  REQUIRE(preview.Resolve(kDesk, kCurrent, kLight) ==
          ResolvePalette(kDraft, kLight));
  REQUIRE(preview.Resolve(kResearch, kCurrent, kLight) ==
          ResolvePalette(kCurrent, kLight));
  REQUIRE(preview.Resolve(kHome, kCurrent, kLight) ==
          ResolvePalette(kCurrent, kLight));

  // A proposal is a copy. Reading or altering it cannot commit or consume the
  // draft; a failed caller save leaves the editor available for correction.
  proposal->theme = kCurrent;
  REQUIRE(SameTheme(preview.proposal()->theme, kDraft));
  REQUIRE(preview.Resolve(kDesk, kCurrent, kLight) ==
          ResolvePalette(kDraft, kLight));

  Theme external_change = kCurrent;
  external_change.light_surface = kGreen;
  preview.Revert();
  REQUIRE(!preview.proposal());
  REQUIRE(preview.Resolve(kDesk, external_change, kLight).surface == kGreen);
  REQUIRE(preview.Resolve(kDesk, external_change, kDark).surface == kBlack);
  preview.Revert();
  REQUIRE(!preview.proposal());
  preview.Set(kDraft);
  preview.Set(external_change);
  REQUIRE(SameTheme(preview.proposal()->theme, external_change));
  REQUIRE(preview.Resolve(kDesk, kCurrent, kLight).surface == kGreen);
}

void SpaceIdentityDoesNotBecomeProfileIdentity() {
  lifecycle::Model model;
  const lifecycle::ProfileId work{"work"};
  const lifecycle::ProfileId personal{"personal"};
  REQUIRE(model.AddProfile(work) == lifecycle::Status::kOk);
  REQUIRE(model.AddProfile(personal) == lifecycle::Status::kOk);
  REQUIRE(model.AddSpace(kDesk, work) == lifecycle::Status::kOk);
  REQUIRE(model.AddSpace(kResearch, work) == lifecycle::Status::kOk);
  REQUIRE(model.AddSpace(kHome, personal) == lifecycle::Status::kOk);
  // The caller confirms these identities in lifecycle, then owns its editors.
  Preview desk{model.spaces().find(kDesk)->first};
  Preview research{model.spaces().find(kResearch)->first};
  Preview home{model.spaces().find(kHome)->first};
  desk.Set(kDraft);
  research.Set(kCurrent);
  home.Set(kDraft);
  desk.Revert();
  REQUIRE(!desk.proposal());
  REQUIRE(research.proposal()->space == kResearch);
  REQUIRE(home.proposal()->space == kHome);
  REQUIRE(SameTheme(research.proposal()->theme, kCurrent));
  REQUIRE(SameTheme(home.proposal()->theme, kDraft));
  REQUIRE(model.spaces().at(kDesk).profile == work);
  REQUIRE(model.spaces().at(kResearch).profile == work);
  REQUIRE(model.spaces().at(kHome).profile == personal);
}

}  // namespace
}  // namespace arcified::space_themes

int main() {
  using namespace arcified::space_themes;
  ContrastMathAndAccentPreservation();
  SystemAndExplicitSchemes();
  NativePaletteIsAuthoritative();
  PreviewDoesNotOwnDurableState();
  SpaceIdentityDoesNotBecomeProfileIdentity();
  std::cout << "Space theme behavior checks passed\n";
}
