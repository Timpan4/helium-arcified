// SPDX-License-Identifier: GPL-3.0-only
// Copyright 2026 The Helium Arcified Authors

#include "base/version_info/version_info.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_window/public/browser_window_interface.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

// These exercise Chromium bindings required by Arcified. They do not implement
// or prove same-window multi-Profile selection or saved-page handoff.
class ArcifiedEngineUpdateBrowserTest : public InProcessBrowserTest {};

IN_PROC_BROWSER_TEST_F(ArcifiedEngineUpdateBrowserTest,
                       DeclaredEngineRevision) {
  // Refresh deliberately with the declared update record and its source hashes.
  EXPECT_EQ("155.0.8059.79", version_info::GetVersionNumber());
}

IN_PROC_BROWSER_TEST_F(ArcifiedEngineUpdateBrowserTest,
                       NavigationRetainsLivePageAndProfile) {
  ASSERT_TRUE(embedded_test_server()->Start());
  Profile* const profile = browser()->GetProfile();
  content::WebContents* const page =
      browser()->GetTabStripModel()->GetActiveWebContents();
  ASSERT_NE(nullptr, page);
  ASSERT_EQ(profile, page->GetBrowserContext());

  const GURL first = embedded_test_server()->GetURL("/title1.html");
  const GURL second = embedded_test_server()->GetURL("/title2.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), first));
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), second));
  EXPECT_EQ(page, browser()->GetTabStripModel()->GetActiveWebContents());
  EXPECT_EQ(profile, page->GetBrowserContext());
  EXPECT_EQ(second, page->GetLastCommittedURL());
}

IN_PROC_BROWSER_TEST_F(ArcifiedEngineUpdateBrowserTest,
                       RegularAndPrivateCookiesStayIsolated) {
  ASSERT_TRUE(embedded_test_server()->Start());
  Profile* const regular = browser()->GetProfile();
  const GURL url = embedded_test_server()->GetURL("/title1.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));
  ASSERT_TRUE(content::SetCookie(regular, url, "arcified_engine=regular"));

  BrowserWindowInterface* const private_browser =
      CreateIncognitoBrowser(regular);
  ASSERT_NE(nullptr, private_browser);
  Profile* const private_profile = private_browser->GetProfile();
  ASSERT_NE(regular, private_profile);
  ASSERT_TRUE(private_profile->IsOffTheRecord());
  ASSERT_TRUE(ui_test_utils::NavigateToURL(private_browser, url));
  EXPECT_EQ("", content::GetCookies(private_profile, url));
  ASSERT_TRUE(
      content::SetCookie(private_profile, url, "arcified_engine=private"));
  EXPECT_EQ("arcified_engine=private",
            content::GetCookies(private_profile, url));
  EXPECT_EQ("arcified_engine=regular", content::GetCookies(regular, url));

  // Closing the real private browser must leave the regular store unchanged.
  CloseBrowserSynchronously(private_browser);
  EXPECT_EQ("arcified_engine=regular", content::GetCookies(regular, url));
}
