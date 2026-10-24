#ifndef ARCIFIED_NATIVE_FEATURES_WINDOW_CHROME_HIT_REGIONS_H_
#define ARCIFIED_NATIVE_FEATURES_WINDOW_CHROME_HIT_REGIONS_H_

#include <cstdint>
#include <vector>

namespace arcified::window_chrome {

// Integer, window-local device-independent coordinates after native layout,
// mirroring, clipping and enclosing conversion. Negative origins are valid.
// Extents must be nonnegative. Right and bottom edges are exclusive.
struct DipRect {
  std::int32_t x;
  std::int32_t y;
  std::int32_t width;
  std::int32_t height;
};

// The host decides eligibility using its actual native frame and view state.
// Keep resize edges, OS caption controls, overlays, fullscreen routing and
// teardown on the native path. This is not inferred from rectangle placement.
enum class NativeFramePolicy { kRetainNativeResult, kAllowCustomChrome };

enum class HitRegion { kDeferToNative, kClient, kCaption };

// Borrowed current-layout geometry, never cached by this component. All rects
// use the same coordinate space. Empty region rects are ignored; negative
// extents anywhere invalidate the input. Native exclusions include reserved
// caption-button space even when appearance makes buttons transparent.
struct ChromeHitRegions {
  DipRect bounds;
  const std::vector<DipRect>& caption;
  const std::vector<DipRect>& interactive;
  const std::vector<DipRect>& native;
};

// Pure region decision, not a native hit-test code or permission to move a
// window. The host retains its native result on kDeferToNative. A query must be
// nonempty and fully inside bounds. Native intersections defer; interactive
// intersections return client; otherwise full containment in a single caption
// candidate returns caption. Uncovered/ambiguous queries defer.
//
// Use the native view's actual hit-test geometry, not paint bounds, to supply
// exclusions. For nonrectangular controls the adapter must conservatively
// enclose the hit region or retain native hit testing. The host must prevent
// stale layouts, coordinate mismatches and concurrent mutation; those cannot
// be detected from rectangles. No input/reference survives this call.
HitRegion HitTestChrome(const DipRect& query,
                        NativeFramePolicy policy,
                        const ChromeHitRegions& regions);

}  // namespace arcified::window_chrome

#endif  // ARCIFIED_NATIVE_FEATURES_WINDOW_CHROME_HIT_REGIONS_H_
