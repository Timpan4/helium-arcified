#include "hit_regions.h"

#include <cstdlib>
#include <iostream>
#include <limits>

namespace arcified::window_chrome {
namespace {

void Require(bool condition, const char* expression, int line) {
  if (condition) {
    return;
  }
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

constexpr auto kCustom = NativeFramePolicy::kAllowCustomChrome;
constexpr auto kNative = NativeFramePolicy::kRetainNativeResult;
constexpr auto kDefer = HitRegion::kDeferToNative;
constexpr auto kClient = HitRegion::kClient;
constexpr auto kCaption = HitRegion::kCaption;
const std::vector<DipRect> kNone;

void ToolbarAndNativePrecedence() {
  const std::vector<DipRect> caption{{0, 0, 800, 40}};
  const std::vector<DipRect> interactive{{12, 8, 28, 24}, {100, 8, 400, 24}};
  const std::vector<DipRect> native{{680, 0, 120, 40}, {0, 0, 800, 4}};
  const ChromeHitRegions regions{
      {0, 0, 800, 600}, caption, interactive, native};
  REQUIRE(HitTestChrome({60, 16, 1, 1}, kCustom, regions) == kCaption);
  REQUIRE(HitTestChrome({12, 8, 1, 1}, kCustom, regions) == kClient);
  REQUIRE(HitTestChrome({200, 16, 1, 1}, kCustom, regions) == kClient);
  REQUIRE(HitTestChrome({700, 16, 1, 1}, kCustom, regions) == kDefer);
  REQUIRE(HitTestChrome({60, 1, 1, 1}, kCustom, regions) == kDefer);
  REQUIRE(HitTestChrome({60, 16, 1, 1}, kNative, regions) == kDefer);
  REQUIRE(HitTestChrome({200, 16, 1, 1}, kNative, regions) == kDefer);
  REQUIRE(HitTestChrome({200, 200, 1, 1}, kCustom, regions) == kDefer);
  // Any contact with a control excludes dragging, even if the origin misses it.
  REQUIRE(HitTestChrome({92, 4, 12, 12}, kCustom, regions) == kClient);
  // A native reservation wins even if a custom control overlaps it.
  REQUIRE(HitTestChrome({24, 2, 20, 10}, kCustom, regions) == kDefer);
}

void EdgesAndAmbiguousCoverage() {
  const std::vector<DipRect> caption{{0, 0, 50, 40}, {50, 0, 50, 40}};
  const std::vector<DipRect> interactive{{20, 10, 10, 20}};
  const ChromeHitRegions regions{{0, 0, 100, 100}, caption, interactive, kNone};
  REQUIRE(HitTestChrome({19, 10, 1, 1}, kCustom, regions) == kCaption);
  REQUIRE(HitTestChrome({20, 10, 1, 1}, kCustom, regions) == kClient);
  REQUIRE(HitTestChrome({30, 10, 1, 1}, kCustom, regions) == kCaption);
  REQUIRE(HitTestChrome({20, 30, 1, 1}, kCustom, regions) == kCaption);
  REQUIRE(HitTestChrome({99, 39, 1, 1}, kCustom, regions) == kCaption);
  REQUIRE(HitTestChrome({100, 39, 1, 1}, kCustom, regions) == kDefer);
  REQUIRE(HitTestChrome({99, 39, 2, 1}, kCustom, regions) == kDefer);
  REQUIRE(HitTestChrome({10, 39, 1, 2}, kCustom, regions) == kDefer);
  REQUIRE(HitTestChrome({49, 0, 2, 1}, kCustom, regions) == kDefer);
  // Adjacent candidates do not imply a new merged draggable surface.
  REQUIRE(HitTestChrome({50, 0, 1, 1}, kCustom, regions) == kCaption);
}

void BorrowedLayoutAndAppearanceInputs() {
  std::vector<DipRect> caption{{0, 0, 200, 40}};
  std::vector<DipRect> interactive{{20, 8, 40, 24}};
  std::vector<DipRect> native{{160, 0, 40, 40}};
  ChromeHitRegions regions{{0, 0, 200, 120}, caption, interactive, native};
  REQUIRE(HitTestChrome({30, 16, 1, 1}, kCustom, regions) == kClient);
  // Caller publishes the next mirrored layout. There is no cached old layout.
  interactive = {{140, 8, 40, 24}};
  native = {{0, 0, 40, 40}};
  REQUIRE(HitTestChrome({30, 16, 1, 1}, kCustom, regions) == kDefer);
  REQUIRE(HitTestChrome({150, 16, 1, 1}, kCustom, regions) == kClient);
  REQUIRE(HitTestChrome({100, 16, 1, 1}, kCustom, regions) == kCaption);
  // Native reservation survives a fade. Only the host may remove it.
  REQUIRE(HitTestChrome({10, 16, 1, 1}, kCustom, regions) == kDefer);
  // Small windows do not create off-window hit targets from older candidates.
  regions.bounds = {0, 0, 100, 120};
  REQUIRE(HitTestChrome({150, 16, 1, 1}, kCustom, regions) == kDefer);
  REQUIRE(HitTestChrome({99, 16, 2, 1}, kCustom, regions) == kDefer);
  // Hidden/reparented custom chrome is removed by its authoritative caller.
  caption.clear();
  interactive.clear();
  REQUIRE(HitTestChrome({60, 16, 1, 1}, kCustom, regions) == kDefer);
}

void EmptyAndInvalidInputs() {
  std::vector<DipRect> caption{{0, 0, 100, 40}};
  std::vector<DipRect> interactive{{10, 0, 0, 40}};
  std::vector<DipRect> native{{20, 0, 10, 0}};
  ChromeHitRegions regions{{0, 0, 100, 100}, caption, interactive, native};
  REQUIRE(HitTestChrome({10, 0, 1, 1}, kCustom, regions) == kCaption);
  REQUIRE(HitTestChrome({20, 0, 1, 1}, kCustom, regions) == kCaption);
  for (const auto& query : std::vector<DipRect>{{0, 0, 0, 1},
                                                {0, 0, 1, 0},
                                                {0, 0, -1, 1},
                                                {0, 0, 1, -1},
                                                {-1, 0, 1, 1}}) {
    REQUIRE(HitTestChrome(query, kCustom, regions) == kDefer);
  }
  // Invalid exclusions fail safely even when far from this query.
  interactive.push_back({500, 500, -1, 10});
  REQUIRE(HitTestChrome({60, 16, 1, 1}, kCustom, regions) == kDefer);
  interactive.clear();
  native.push_back({500, 500, 10, -1});
  REQUIRE(HitTestChrome({60, 16, 1, 1}, kCustom, regions) == kDefer);
  native.clear();
  caption.push_back({500, 500, -1, 10});
  REQUIRE(HitTestChrome({60, 16, 1, 1}, kCustom, regions) == kDefer);
  caption.pop_back();
  regions.bounds.width = -1;
  REQUIRE(HitTestChrome({60, 16, 1, 1}, kCustom, regions) == kDefer);
  regions.bounds = {0, 0, 0, 100};
  REQUIRE(HitTestChrome({0, 0, 1, 1}, kCustom, regions) == kDefer);
}

void CoordinateExtremes() {
  constexpr auto low = std::numeric_limits<std::int32_t>::min();
  constexpr auto high = std::numeric_limits<std::int32_t>::max();
  const std::vector<DipRect> caption{{low, low, high, high},
                                     {high - 1, high - 1, high, high}};
  const ChromeHitRegions negative{
      {low, low, high, high}, caption, kNone, kNone};
  REQUIRE(HitTestChrome({low, low, 1, 1}, kCustom, negative) == kCaption);
  REQUIRE(HitTestChrome({-2, -2, 1, 1}, kCustom, negative) == kCaption);
  REQUIRE(HitTestChrome({-1, -1, 1, 1}, kCustom, negative) == kDefer);
  const ChromeHitRegions positive{
      {high - 1, high - 1, high, high}, caption, kNone, kNone};
  REQUIRE(HitTestChrome({high, high, 1, 1}, kCustom, positive) == kCaption);
  const std::vector<DipRect> native{{high, high, high, high}};
  const ChromeHitRegions reserved{positive.bounds, caption, kNone, native};
  REQUIRE(HitTestChrome({high, high, 1, 1}, kCustom, reserved) == kDefer);
}

}  // namespace
}  // namespace arcified::window_chrome

int main() {
  using namespace arcified::window_chrome;
  ToolbarAndNativePrecedence();
  EdgesAndAmbiguousCoverage();
  BorrowedLayoutAndAppearanceInputs();
  EmptyAndInvalidInputs();
  CoordinateExtremes();
  std::cout << "window-chrome: all hit-region fixtures passed\n";
}
