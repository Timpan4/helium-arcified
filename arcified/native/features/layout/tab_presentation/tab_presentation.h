#ifndef ARCIFIED_NATIVE_FEATURES_LAYOUT_TAB_PRESENTATION_TAB_PRESENTATION_H_
#define ARCIFIED_NATIVE_FEATURES_LAYOUT_TAB_PRESENTATION_TAB_PRESENTATION_H_

#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

#include "../../../model/lifecycle.h"
#include "../sidebar/sidebar.h"

namespace arcified::tab_presentation {

enum class Orientation { kVertical, kHorizontal };
enum class Density { kCompact, kComfortable };

// Caller chooses values and owns durability. Sidebar preferences stay in #51.
struct Preferences {
  Orientation orientation;
  Density density;
};

// Semantic descriptors for UI adapters, not a persistence schema or codec.
std::string_view Describe(Orientation orientation);
std::string_view Describe(Density density);

// All extents use the same caller-selected logical coordinate system. The host
// measures requirements at the requested density and current display scale,
// including captions, controls and minimum usable content. No pixel defaults.
struct Extent {
  double width;
  double height;
};
struct FitMetrics {
  Extent available;
  Extent vertical_required;
  Extent horizontal_required;
};

// Saved items and aliases stay distinct rows even when they resolve to the same
// live page. PageId identifies a caller-projected temporary row. No URL/title
// or native tab index becomes an identity here.
using TabIdentity =
    std::variant<lifecycle::ItemId, lifecycle::ShortcutId, lifecycle::PageId>;

// Already scoped/validated by the authoritative workspace/native adapter. This
// component neither builds the row list nor infers Profile, order or selection.
// Borrowed objects must outlive the projection. Serialize changes and reproject
// after changes to these objects, model/native identity or measurements.
struct AuthoritativeTabs {
  std::reference_wrapper<const lifecycle::Attachment> attachment;
  std::reference_wrapper<const lifecycle::ProfileId> profile;
  std::reference_wrapper<const std::vector<TabIdentity>> order;
  std::reference_wrapper<const std::optional<TabIdentity>> selected;
};

struct RowPresentation {
  // Pointer, keyboard and accessibility activation all use this same identity.
  // Host resolves and acts through the native command path. Never an open plan.
  std::reference_wrapper<const TabIdentity> activation_target;
  bool selected;
  std::size_t position_in_set;  // One-based accessibility ordinal.
  std::size_t set_size;
};

enum class ProjectionError { kInvalidMetrics, kNoUsableLayout };

class Presentation {
 public:
  Preferences requested() const { return requested_; }
  Orientation effective_orientation() const { return effective_; }
  bool used_fallback() const { return effective_ != requested_.orientation; }
  // Retains the caller's #51 side even when the effective strip is horizontal.
  sidebar::Side sidebar_side() const { return side_; }
  // Compose the host's Focus-mode chrome request before feeding #51 Context.
  // Horizontal tabs always suppress the vertical sidebar, including on reveal;
  // the host applies chrome_override to its horizontal toolbar/tabstrip
  // instead. #51 owns visibility, bounds and resize cancellation; preferences
  // stay intact.
  sidebar::PresentationOverride sidebar_override(
      sidebar::PresentationOverride chrome_override =
          sidebar::PresentationOverride::kNone) const;

  const AuthoritativeTabs& tabs() const { return tabs_; }
  std::optional<RowPresentation> RowAt(std::size_t index) const;

 private:
  friend std::variant<ProjectionError, Presentation> Project(AuthoritativeTabs,
                                                             Preferences,
                                                             sidebar::Side,
                                                             FitMetrics);
  Presentation(AuthoritativeTabs tabs,
               Preferences requested,
               sidebar::Side side,
               Orientation effective);

  AuthoritativeTabs tabs_;
  Preferences requested_;
  sidebar::Side side_;
  Orientation effective_;
};

using ProjectionResult = std::variant<ProjectionError, Presentation>;

// Prefer the requested orientation, then the other orientation if it fits.
// A failed fit never changes preference, hides rows, selects a substitute page
// or invents clipped controls. Host must retain a usable surface/report failure
// and remeasure; neither this result nor a row reference authorizes actuation.
ProjectionResult Project(AuthoritativeTabs tabs,
                         Preferences preferences,
                         sidebar::Side side,
                         FitMetrics metrics);

}  // namespace arcified::tab_presentation

#endif  // ARCIFIED_NATIVE_FEATURES_LAYOUT_TAB_PRESENTATION_TAB_PRESENTATION_H_
