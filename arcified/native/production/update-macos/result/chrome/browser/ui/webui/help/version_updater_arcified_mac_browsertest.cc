// Copyright 2026 The Arcified Authors
// SPDX-License-Identifier: GPL-3.0-only

#include <cstdint>
#include <string>

#include "base/run_loop.h"
#include "base/test/bind.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/webui/help/version_updater.h"
#include "chrome/common/pref_names.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"
#include "testing/gtest/include/gtest/gtest.h"

using ArcifiedMacUpdateBrowserTest = InProcessBrowserTest;

IN_PROC_BROWSER_TEST_F(ArcifiedMacUpdateBrowserTest,
                       UnavailableCheckRetainsProfileAndLivePage) {
  Profile* const profile = browser()->GetProfile();
  content::WebContents* const page =
      browser()->tab_strip_model()->GetActiveWebContents();
  ASSERT_TRUE(page);
  const auto page_url = page->GetLastCommittedURL();
  const int tab_count = browser()->tab_strip_model()->count();
  const std::string homepage = "https://update-macos.invalid/retained";
  profile->GetPrefs()->SetString(prefs::kHomePage, homepage);

  auto updater = VersionUpdater::Create(page);
  ASSERT_TRUE(updater);
  int status_calls = 0;
  int promotion_calls = 0;
  auto status = base::BindLambdaForTesting(
      [&](VersionUpdater::Status value, int progress, bool rollback,
          bool powerwash, const std::string& version, int64_t size,
          const std::u16string& message) {
        ++status_calls;
        EXPECT_EQ(value, VersionUpdater::FAILED);
        EXPECT_EQ(progress, 0);
        EXPECT_FALSE(rollback);
        EXPECT_FALSE(powerwash);
        EXPECT_TRUE(version.empty());
        EXPECT_EQ(size, 0);
        EXPECT_FALSE(message.empty());
      });
  auto promotion =
      base::BindLambdaForTesting([&](VersionUpdater::PromotionState value) {
        ++promotion_calls;
        EXPECT_EQ(value, VersionUpdater::PROMOTE_HIDDEN);
      });

  updater->CheckForUpdate(status, promotion);
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(status_calls, 1);
  EXPECT_EQ(promotion_calls, 1);
  // A direct promotion request and another check cannot enable installation or
  // turn an unavailable result into "up to date" or "relaunch".
  updater->PromoteUpdater();
  updater->CheckForUpdate(status, promotion);
  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(status_calls, 2);
  EXPECT_EQ(promotion_calls, 2);
  EXPECT_EQ(profile->GetPrefs()->GetString(prefs::kHomePage), homepage);
  EXPECT_EQ(browser()->GetProfile(), profile);
  EXPECT_EQ(page->GetBrowserContext(), profile);
  EXPECT_EQ(browser()->tab_strip_model()->GetActiveWebContents(), page);
  EXPECT_EQ(browser()->tab_strip_model()->count(), tab_count);
  EXPECT_EQ(page->GetLastCommittedURL(), page_url);
}

IN_PROC_BROWSER_TEST_F(ArcifiedMacUpdateBrowserTest,
                       PromotionCallbackCanDestroyUpdater) {
  auto updater = VersionUpdater::Create(
      browser()->tab_strip_model()->GetActiveWebContents());
  ASSERT_TRUE(updater);
  int status_calls = 0;
  updater->CheckForUpdate(
      base::BindLambdaForTesting([&](VersionUpdater::Status status, int, bool,
                                     bool, const std::string&, int64_t,
                                     const std::u16string&) {
        ++status_calls;
        EXPECT_EQ(status, VersionUpdater::FAILED);
      }),
      base::BindLambdaForTesting([&](VersionUpdater::PromotionState state) {
        EXPECT_EQ(state, VersionUpdater::PROMOTE_HIDDEN);
        updater.reset();
      }));
  base::RunLoop().RunUntilIdle();
  EXPECT_FALSE(updater);
  EXPECT_EQ(status_calls, 1);
}
