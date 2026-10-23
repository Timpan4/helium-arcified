#include "hit_regions.h"

#include <algorithm>

namespace arcified::window_chrome {
namespace {

bool Valid(const DipRect& rect) {
  return rect.width >= 0 && rect.height >= 0;
}

bool Empty(const DipRect& rect) {
  return rect.width == 0 || rect.height == 0;
}

// Widen before addition so even the full int32 input domain is well-defined.
std::int64_t Right(const DipRect& rect) {
  return std::int64_t{rect.x} + rect.width;
}

std::int64_t Bottom(const DipRect& rect) {
  return std::int64_t{rect.y} + rect.height;
}

bool Contains(const DipRect& outer, const DipRect& inner) {
  return outer.x <= inner.x && outer.y <= inner.y &&
         Right(outer) >= Right(inner) && Bottom(outer) >= Bottom(inner);
}

bool Intersects(const DipRect& a, const DipRect& b) {
  return !Empty(a) && !Empty(b) && a.x < Right(b) && b.x < Right(a) &&
         a.y < Bottom(b) && b.y < Bottom(a);
}

bool Valid(const std::vector<DipRect>& rects) {
  return std::all_of(rects.begin(), rects.end(),
                     [](const DipRect& rect) { return Valid(rect); });
}

bool IntersectsAny(const DipRect& query, const std::vector<DipRect>& rects) {
  return std::any_of(rects.begin(), rects.end(), [&](const DipRect& rect) {
    return Intersects(query, rect);
  });
}

}  // namespace

HitRegion HitTestChrome(const DipRect& query,
                        NativeFramePolicy policy,
                        const ChromeHitRegions& regions) {
  switch (policy) {
    case NativeFramePolicy::kRetainNativeResult:
      return HitRegion::kDeferToNative;
    case NativeFramePolicy::kAllowCustomChrome:
      break;
  }

  if (!Valid(query) || Empty(query) || !Valid(regions.bounds) ||
      !Valid(regions.caption) || !Valid(regions.interactive) ||
      !Valid(regions.native) || !Contains(regions.bounds, query)) {
    return HitRegion::kDeferToNative;
  }
  if (IntersectsAny(query, regions.native)) {
    return HitRegion::kDeferToNative;
  }
  if (IntersectsAny(query, regions.interactive)) {
    return HitRegion::kClient;
  }
  for (const auto& caption : regions.caption) {
    if (!Empty(caption) && Contains(caption, query)) {
      return HitRegion::kCaption;
    }
  }
  return HitRegion::kDeferToNative;
}

}  // namespace arcified::window_chrome
