// SPDX-License-Identifier: GPL-3.0-only

#include <memory>
#include <optional>
#include <string>

#include "base/test/bind.h"
#include "base/version.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/webui/help/version_updater.h"
#include "chrome/browser/upgrade_detector/build_state.h"
#include "chrome/browser/upgrade_detector/installed_version_poller.h"
#include "chrome/grit/generated_resources.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "ui/base/l10n/l10n_util.h"
#include "url/gurl.h"

namespace {

class ArcifiedUpdateLinuxBrowserTest : public InProcessBrowserTest {
 protected:
  void ExpectUnavailable(content::WebContents* web_contents) {
    auto updater = VersionUpdater::Create(web_contents);
    bool received = false;
    updater->CheckForUpdate(
        base::BindLambdaForTesting(
            [&](VersionUpdater::Status status, int progress, bool rollback,
                bool powerwash, const std::string& version, int64_t size,
                const std::u16string& message) {
              EXPECT_FALSE(received);
              received = true;
              EXPECT_EQ(VersionUpdater::DISABLED, status);
              EXPECT_EQ(0, progress);
              EXPECT_FALSE(rollback);
              EXPECT_FALSE(powerwash);
              EXPECT_TRUE(version.empty());
              EXPECT_EQ(0, size);
              EXPECT_EQ(l10n_util::GetStringUTF16(
                            IDS_ARCIFIED_UPDATE_LINUX_UNAVAILABLE),
                        message);
            }),
        VersionUpdater::PromoteCallback());
    EXPECT_TRUE(received);
  }

  void ExpectVisibleUnavailable() {
    ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(),
                                             GURL("chrome://settings/help")));
    auto* web_contents = browser()->tab_strip_model()->GetActiveWebContents();
    // Follow the real settings route and its native AboutHandler callback.
    // Yield while Lit attaches/renders. EvalJs uses the browser-test harness's
    // timeout; there is no separate polling interval or time limit here.
    EXPECT_EQ(l10n_util::GetStringUTF8(IDS_ARCIFIED_UPDATE_LINUX_UNAVAILABLE),
              content::EvalJs(web_contents, R"JS(
          (async () => {
            function findAbout(root) {
              const about = root.querySelector('settings-about-page');
              if (about) return about;
              for (const element of root.querySelectorAll('*')) {
                if (element.shadowRoot) {
                  const found = findAbout(element.shadowRoot);
                  if (found) return found;
                }
              }
              return null;
            }
            while (true) {
              const about = findAbout(document);
              const status = about?.shadowRoot?.querySelector(
                  '#updateStatusMessage');
              if (status?.checkVisibility() && status.textContent.trim()) {
                const relaunch = about.shadowRoot.querySelector('#relaunch');
                if (relaunch?.checkVisibility()) {
                  throw new Error('Unavailable updater offered relaunch');
                }
                return status.textContent.trim();
              }
              await new Promise(requestAnimationFrame);
            }
          })()
        )JS"));
  }

 private:
  // Keep the real BuildState and detector, but do not poll the host executable
  // while the test delivers installed-version notifications to that service.
  InstalledVersionPoller::ScopedDisableForTesting disable_installed_polling_ =
      InstalledVersionPoller::MakeScopedDisableForTesting();
};

IN_PROC_BROWSER_TEST_F(ArcifiedUpdateLinuxBrowserTest,
                       UnavailableWithAndWithoutWebContents) {
  ExpectUnavailable(browser()->tab_strip_model()->GetActiveWebContents());
  ExpectUnavailable(nullptr);
}

IN_PROC_BROWSER_TEST_F(ArcifiedUpdateLinuxBrowserTest,
                       InstalledVersionSignalDoesNotAuthenticateAnUpdate) {
  auto* state = g_browser_process->GetBuildState();
  state->SetUpdate(BuildState::UpdateType::kNormalUpdate,
                   base::Version("156.0.0.0"), std::nullopt);
  ExpectUnavailable(browser()->tab_strip_model()->GetActiveWebContents());

  state->SetUpdate(BuildState::UpdateType::kEnterpriseRollback,
                   base::Version("154.0.0.0"), std::nullopt);
  ExpectUnavailable(browser()->tab_strip_model()->GetActiveWebContents());

  // The native poller also signals a normal update for an unreadable version.
  state->SetUpdate(BuildState::UpdateType::kNormalUpdate, base::Version(),
                   std::nullopt);
  ExpectUnavailable(browser()->tab_strip_model()->GetActiveWebContents());
  state->SetUpdate(BuildState::UpdateType::kNone, base::Version(),
                   std::nullopt);
}

IN_PROC_BROWSER_TEST_F(ArcifiedUpdateLinuxBrowserTest,
                       AboutPageExplainsUnavailableAfterReentry) {
  ExpectVisibleUnavailable();
  ASSERT_TRUE(
      ui_test_utils::NavigateToURL(browser(), GURL("chrome://version")));
  ExpectVisibleUnavailable();
}

}  // namespace
