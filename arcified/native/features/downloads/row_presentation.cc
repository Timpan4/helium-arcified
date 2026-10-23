#include "row_presentation.h"

namespace arcified::downloads {

RowPresentation DeriveRowPresentation(const NativeRowSignals& native) {
  RowPresentation presentation{MainButtonRoute::kDisabled, std::nullopt};
  if (native.main_button_enabled) {
    presentation.main_button = native.has_security_subpage
                                   ? MainButtonRoute::kNativeSecuritySubpage
                                   : MainButtonRoute::kNativeOpenOrSchedule;
  }

  if (native.progress_visible) {
    VisibleProgress progress{std::nullopt, native.paused};
    // DownloadItem::PercentComplete documents -1 for unknown and 0..100 for
    // percentages. Do not clamp an invalid value into apparent completion.
    // Native looping (for example, scanning) takes precedence over bytes.
    if (!native.progress_looping && native.percent_complete >= 0 &&
        native.percent_complete <= 100) {
      progress.fraction = static_cast<double>(native.percent_complete) / 100;
    }
    presentation.progress = progress;
  }
  return presentation;
}

}  // namespace arcified::downloads
