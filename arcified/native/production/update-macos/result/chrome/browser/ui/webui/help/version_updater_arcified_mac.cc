// Copyright 2026 The Arcified Authors
// SPDX-License-Identifier: GPL-3.0-only

#include <memory>
#include <string>

#include "chrome/browser/ui/webui/help/version_updater.h"
#include "chrome/grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"

namespace {

// Native About-page binding until issue #167 has an approved update service.
// No updater, installation, relaunch, or Profile service is invoked here.
class ArcifiedMacVersionUpdater : public VersionUpdater {
 public:
  void CheckForUpdate(StatusCallback status_callback,
                      PromoteCallback promote_callback) override {
    const std::u16string message =
        l10n_util::GetStringUTF16(IDS_ARCIFIED_UPDATE_MACOS_UNAVAILABLE);
    // Either callback may destroy this updater. Everything needed below is
    // owned by this call, with no subsequent access to the instance.
    if (promote_callback) {
      promote_callback.Run(PROMOTE_HIDDEN);
    }
    // DISABLED is hidden by the About page. Report the failed check explicitly
    // without claiming the installed version is current or ready to relaunch.
    status_callback.Run(FAILED, 0, false, false, std::string(), 0, message);
  }

  void PromoteUpdater() override {}
};

}  // namespace

std::unique_ptr<VersionUpdater> VersionUpdater::Create(
    content::WebContents* /*web_contents*/) {
  return std::make_unique<ArcifiedMacVersionUpdater>();
}
