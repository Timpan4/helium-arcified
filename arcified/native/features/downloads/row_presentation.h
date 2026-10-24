#ifndef ARCIFIED_NATIVE_FEATURES_DOWNLOADS_ROW_PRESENTATION_H_
#define ARCIFIED_NATIVE_FEATURES_DOWNLOADS_ROW_PRESENTATION_H_

#include <optional>

namespace arcified::downloads {

// Transient inputs read together from the current DownloadBubbleRowViewInfo
// and its DownloadUIModel. These are render signals, not download lifecycle or
// safety state. See ../../../docs/features/downloads.md for the native mapping.
struct NativeRowSignals {
  bool main_button_enabled = false;
  bool has_security_subpage = false;
  bool progress_visible = false;
  bool progress_looping = false;
  bool paused = false;
  int percent_complete = -1;
};

enum class MainButtonRoute {
  kDisabled,
  kNativeSecuritySubpage,
  // Native OpenDownload may schedule opening on completion. This route does
  // not mean the file is complete, present, safe or successfully opened.
  kNativeOpenOrSchedule,
};

struct VisibleProgress {
  // No value means indeterminate. A value is the native percentage / 100,
  // never a byte-based estimate, completion verdict or permission to open.
  std::optional<double> fraction;
  bool paused;
};

struct RowPresentation {
  MainButtonRoute main_button;
  // No value means hidden, even if a native percentage is available.
  std::optional<VisibleProgress> progress;
};

// Pure render projection. The caller must reread native state when activating
// a control and preserve the existing native event/security/command handler.
// This function neither executes commands nor supplies their authorization.
RowPresentation DeriveRowPresentation(const NativeRowSignals& native);

}  // namespace arcified::downloads

#endif  // ARCIFIED_NATIVE_FEATURES_DOWNLOADS_ROW_PRESENTATION_H_
