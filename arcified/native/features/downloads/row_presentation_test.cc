#include "row_presentation.h"

#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <limits>

namespace arcified::downloads {
namespace {

void Check(bool condition, const char* requirement) {
  if (!condition) {
    std::cerr << requirement << '\n';
    std::exit(EXIT_FAILURE);
  }
}

void MainButtonPreservesNativeRouting() {
  NativeRowSignals signals;
  Check(
      DeriveRowPresentation(signals).main_button == MainButtonRoute::kDisabled,
      "Default signals must leave activation disabled");
  for (bool enabled : {false, true}) {
    for (bool security : {false, true}) {
      signals.main_button_enabled = enabled;
      signals.has_security_subpage = security;
      const auto route = DeriveRowPresentation(signals).main_button;
      if (!enabled) {
        Check(route == MainButtonRoute::kDisabled,
              "A security subpage must not override a disabled native button");
      } else if (security) {
        Check(route == MainButtonRoute::kNativeSecuritySubpage,
              "Warning activation must retain the native security route");
      } else {
        Check(route == MainButtonRoute::kNativeOpenOrSchedule,
              "Ordinary activation must retain native open-or-schedule");
      }
    }
  }
}

void HiddenProgressIgnoresPercentage() {
  NativeRowSignals signals;
  signals.percent_complete = 100;
  for (bool looping : {false, true}) {
    signals.progress_looping = looping;
    Check(!DeriveRowPresentation(signals).progress,
          "A hidden native bar must stay hidden even at 100 percent");
  }
}

void UnknownAndInvalidProgressStayIndeterminate() {
  NativeRowSignals signals;
  signals.progress_visible = true;
  for (int percent : {-1, -2, 101, std::numeric_limits<int>::min(),
                      std::numeric_limits<int>::max()}) {
    signals.percent_complete = percent;
    const auto presentation = DeriveRowPresentation(signals);
    Check(presentation.progress && !presentation.progress->fraction,
          "Unknown/invalid native percentages must not invent progress");
  }
}

void NativePercentageIsUsedWithoutRecalculation() {
  NativeRowSignals signals;
  signals.progress_visible = true;
  for (int percent = 0; percent <= 100; ++percent) {
    signals.percent_complete = percent;
    const auto presentation = DeriveRowPresentation(signals);
    if (!presentation.progress || !presentation.progress->fraction) {
      Check(false, "Every native percentage must yield determinate progress");
      return;
    }
    Check(*presentation.progress->fraction == percent / 100.0,
          "Determinate progress must preserve the native percentage");
    Check(presentation.main_button == MainButtonRoute::kDisabled,
          "Progress, including 100 percent, must not enable activation");
  }
}

void ScanningAndPauseRemainNativeDecisions() {
  NativeRowSignals signals;
  signals.main_button_enabled = true;
  signals.has_security_subpage = true;
  signals.progress_visible = true;
  signals.percent_complete = 100;
  for (bool paused : {false, true}) {
    for (bool looping : {false, true}) {
      signals.paused = paused;
      signals.progress_looping = looping;
      const auto presentation = DeriveRowPresentation(signals);
      if (!presentation.progress) {
        Check(false, "Native scanning and pause must retain a visible bar");
        return;
      }
      Check(presentation.progress->paused == paused,
            "The native pause signal must survive for every visible bar");
      Check(presentation.progress->fraction.has_value() == !looping,
            "A native scanning loop must remain indeterminate at 100 percent");
      Check(presentation.main_button == MainButtonRoute::kNativeSecuritySubpage,
            "Progress must never replace warning activation with file open");
    }
  }
}

void ReprojectionDoesNotRetainOldControls() {
  const auto ordinary =
      DeriveRowPresentation({true, false, true, false, true, 25});
  const auto warning =
      DeriveRowPresentation({true, true, false, false, false, 100});
  const auto unavailable = DeriveRowPresentation({});
  Check(ordinary.progress && ordinary.progress->fraction == 0.25 &&
            ordinary.progress->paused,
        "A paused ordinary projection must retain its own native inputs");
  Check(warning.main_button == MainButtonRoute::kNativeSecuritySubpage &&
            !warning.progress,
        "A new warning projection must hide the old progress bar");
  Check(unavailable.main_button == MainButtonRoute::kDisabled &&
            !unavailable.progress,
        "Unavailable inputs must not retain an old action or progress bar");
}

}  // namespace
}  // namespace arcified::downloads

int main() {
  using namespace arcified::downloads;
  MainButtonPreservesNativeRouting();
  HiddenProgressIgnoresPercentage();
  UnknownAndInvalidProgressStayIndeterminate();
  NativePercentageIsUsedWithoutRecalculation();
  ScanningAndPauseRemainNativeDecisions();
  ReprojectionDoesNotRetainOldControls();
  std::cout << "Download row presentation fixtures passed\n";
}
