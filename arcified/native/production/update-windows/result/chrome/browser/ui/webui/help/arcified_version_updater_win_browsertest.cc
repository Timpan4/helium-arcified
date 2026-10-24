// Copyright 2026 The Helium Arcified Authors
// SPDX-License-Identifier: GPL-3.0-only

#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/grit/generated_resources.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "ui/base/l10n/l10n_util.h"
#include "url/gurl.h"

namespace {

using ArcifiedUpdateWindowsBrowserTest = InProcessBrowserTest;

// Exercise the actual Settings WebUI -> AboutHandler -> VersionUpdater factory
// -> WebUI event binding. No update service or VersionUpdater is substituted.
IN_PROC_BROWSER_TEST_F(ArcifiedUpdateWindowsBrowserTest,
                       AboutReportsUnavailableThroughNativeBinding) {
  ASSERT_TRUE(
      ui_test_utils::NavigateToURL(browser(), GURL("chrome://settings/help")));
  auto* contents = browser()->tab_strip_model()->GetActiveWebContents();
  EXPECT_EQ(l10n_util::GetStringUTF8(IDS_ARCIFIED_UPDATE_WINDOWS_UNAVAILABLE),
            content::EvalJs(contents, R"JS(
              (async () => {
                const {addWebUiListener, removeWebUiListener} =
                    await import('chrome://resources/js/cr.js');
                return new Promise(resolve => {
                  const listener = addWebUiListener(
                      'update-status-changed', event => {
                        removeWebUiListener(listener);
                        resolve(event.status === 'disabled' &&
                                        event.progress === 0 &&
                                        !event.rollback && !event.powerwash &&
                                        event.version === '' &&
                                        event.size === '0' ?
                                    event.message :
                                    'Unexpected update status');
                      });
                  chrome.send('refreshUpdateStatus');
                });
              })()
            )JS"));
}

}  // namespace
