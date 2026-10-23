#ifndef ARCIFIED_NATIVE_FEATURES_MEDIA_TAB_STATUS_H_
#define ARCIFIED_NATIVE_FEATURES_MEDIA_TAB_STATUS_H_

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::media {

// Unknown is never evidence that an activity is absent or work is quiescent.
enum class Signal { kUnknown, kOff, kOn };

// Complete browser observations, not lifecycle policy or native ownership.
// An unsupported/unavailable observation stays unknown. No active selection,
// navigation, document token, native handle or ownership is cached here.
struct TabSignals {
  Signal loading = Signal::kUnknown;
  Signal crashed = Signal::kUnknown;
  Signal sleeping = Signal::kUnknown;
  Signal audible = Signal::kUnknown;
  Signal muted = Signal::kUnknown;
  Signal microphone = Signal::kUnknown;
  Signal camera = Signal::kUnknown;
  Signal tab_capture = Signal::kUnknown;
  Signal display_capture = Signal::kUnknown;
  Signal attention = Signal::kUnknown;

  friend bool operator==(const TabSignals& a, const TabSignals& b);
};

// Owned read snapshot. Profile and attachment always come from a fresh Model
// lookup; holding this value does not reserve the page or native document.
struct PageStatus {
  lifecycle::PageId page;
  lifecycle::ProfileId profile;
  lifecycle::Attachment attachment;
  TabSignals signals;

  friend bool operator==(const PageStatus& a, const PageStatus& b);
};

enum class ObservationResult { kChanged, kUnchanged, kPageNotFound };

enum class IndicatorKind {
  kActive,
  kLoading,
  kCrashed,
  kSleeping,
  kAudible,
  kMuted,
  kMicrophone,
  kCamera,
  kTabCapture,
  kDisplayCapture,
  kAttention,
};

// Kind supplies a non-color semantic icon key; the text also works without
// icons. English names are reference strings, not Chromium localized resources.
struct StatusIndicator {
  IndicatorKind kind;
  Signal state;
  std::string accessible_name;
};

// Active selection is a fresh caller-owned native fact, never inferred from
// selected Space or cached as another focus authority. Includes off/unknown
// states so callers can offer a complete textual status inspection.
std::vector<StatusIndicator> DescribeStatus(const TabSignals& signals,
                                            Signal active);

enum class PageActionKind {
  kMute,
  kUnmute,
  kStopLoading,
  kRetryCrash,
  kLocate
};
std::string_view ActionAccessibleName(PageActionKind kind);

struct PageAction {
  PageStatus expected;  // The exact snapshot presented by the requesting UI.
  PageActionKind kind;
};

enum class ActionRefusal {
  kPageNotFound,
  kStaleSnapshot,
  kSignalUnavailable,
  kSleeping,
  kNotApplicable,
};

// Route planning only. Not native success, permission to reload, or a token
// that can be queued and later dispatched without fresh resolution.
struct ResolvedPageAction {
  lifecycle::PageId page;
  lifecycle::ProfileId profile;
  lifecycle::Attachment attachment;
  PageActionKind kind;
};
using PageActionResult = std::variant<ActionRefusal, ResolvedPageAction>;

// Single-sequence projection cache. The host's one native page registry owns
// document freshness and must reject obsolete callbacks BEFORE Observe. It
// must Invalidate before navigation/replacement and publish fresh observations
// for the new document. This component cannot detect an obsolete callback for
// a still-live PageId and deliberately generates no competing registry tokens.
class TabStatus {
 public:
  ObservationResult Observe(const lifecycle::Model& model,
                            lifecycle::PageId page,
                            TabSignals signals);
  bool Invalidate(const lifecycle::PageId& page);
  std::size_t PruneClosed(const lifecycle::Model& model);
  std::optional<PageStatus> Read(const lifecycle::Model& model,
                                 const lifecycle::PageId& page) const;

  // Revalidates the UI's expected Profile, attachment and all signals against
  // a fresh model/status read. Native host must additionally match its current
  // registry/document facts to the originating UI request, then resolve again
  // immediately before actuation on the same serialized UI sequence. Retained
  // plans are not dispatch authority. No native operation or Model mutation.
  PageActionResult ResolveAction(const lifecycle::Model& model,
                                 const PageAction& action) const;

 private:
  std::map<lifecycle::PageId, TabSignals> signals_;
};

}  // namespace arcified::media

#endif  // ARCIFIED_NATIVE_FEATURES_MEDIA_TAB_STATUS_H_
