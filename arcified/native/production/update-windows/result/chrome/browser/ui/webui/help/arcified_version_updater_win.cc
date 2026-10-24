// Copyright 2026 The Helium Arcified Authors
// SPDX-License-Identifier: GPL-3.0-only

#include "chrome/browser/ui/webui/help/version_updater.h"

#include <memory>
#include <string>

#include "chrome/grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"

namespace {

// The unbranded Windows build has no approved authenticated update transport.
// Keep the native About-page entry point available without interpreting an
// installed-version notification as proof of an authenticated Arcified update.
// This class does not fetch, stage, install, roll back, or request a restart.
class ArcifiedVersionUpdaterWin final : public VersionUpdater {
 public:
  void CheckForUpdate(StatusCallback callback, PromoteCallback) override {
    callback.Run(
        DISABLED, 0, false, false, std::string(), 0,
        l10n_util::GetStringUTF16(IDS_ARCIFIED_UPDATE_WINDOWS_UNAVAILABLE));
    // The callback can destroy this updater. Do not access members afterward.
  }
};

}  // namespace

std::unique_ptr<VersionUpdater> VersionUpdater::Create(content::WebContents*) {
  return std::make_unique<ArcifiedVersionUpdaterWin>();
}
