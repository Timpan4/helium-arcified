// SPDX-License-Identifier: GPL-3.0-only

#include <memory>
#include <string>

#include "chrome/browser/ui/webui/help/version_updater.h"
#include "chrome/grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"

namespace {

// No authenticated application-update channel is configured for this build.
// Installed-version polling is a separate external-install notification. Its
// result cannot establish the replacement artifact's identity or signature.
class VersionUpdaterArcifiedLinux final : public VersionUpdater {
 public:
  void CheckForUpdate(StatusCallback callback, PromoteCallback) override {
    callback.Run(
        DISABLED, 0, false, false, std::string(), 0,
        l10n_util::GetStringUTF16(IDS_ARCIFIED_UPDATE_LINUX_UNAVAILABLE));
  }
};

}  // namespace

std::unique_ptr<VersionUpdater> VersionUpdater::Create(
    content::WebContents* web_contents) {
  return std::make_unique<VersionUpdaterArcifiedLinux>();
}
