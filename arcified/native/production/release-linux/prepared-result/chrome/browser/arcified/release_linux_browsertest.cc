// Copyright 2026 The Arcified Authors
// SPDX-License-Identifier: GPL-3.0-only

#include <string>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/threading/thread_restrictions.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/pref_names.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "components/prefs/pref_service.h"
#include "content/public/test/browser_test.h"

namespace {

// These checks exercise a real regular Profile and its preference store. The
// launcher supplies an isolated --user-data-dir and retains it across PRE_
// stages. A passing run proves startup/restart behavior for that binary only;
// it does not exercise an installer, a different version or uninstall.
class ArcifiedReleaseLinuxBrowserTest : public InProcessBrowserTest {
 protected:
  void CheckExplicitUserDataDirectory() {
    const base::FilePath requested =
        base::CommandLine::ForCurrentProcess()->GetSwitchValuePath(
            switches::kUserDataDir);
    ASSERT_TRUE(requested.IsAbsolute());
    base::FilePath actual;
    ASSERT_TRUE(base::PathService::Get(chrome::DIR_USER_DATA, &actual));
    EXPECT_EQ(requested.StripTrailingSeparators(),
              actual.StripTrailingSeparators());
    ASSERT_TRUE(GetProfile());
    EXPECT_FALSE(GetProfile()->IsOffTheRecord());
    EXPECT_TRUE(actual.IsParent(GetProfile()->GetPath()));
  }
};

IN_PROC_BROWSER_TEST_F(ArcifiedReleaseLinuxBrowserTest,
                       UsesExplicitUserDataDirectory) {
  ASSERT_NO_FATAL_FAILURE(CheckExplicitUserDataDirectory());
}

IN_PROC_BROWSER_TEST_F(ArcifiedReleaseLinuxBrowserTest,
                       PRE_RetainsProfileDataAfterRestart) {
  ASSERT_NO_FATAL_FAILURE(CheckExplicitUserDataDirectory());
  base::ScopedAllowBlockingForTesting allow_blocking;
  base::FilePath user_data_dir;
  ASSERT_TRUE(base::PathService::Get(chrome::DIR_USER_DATA, &user_data_dir));

  // Record the real Profile location rather than assuming an upstream or a
  // future fork-owned directory name. Only test-owned data is written.
  ASSERT_TRUE(base::WriteFile(
      user_data_dir.AppendASCII("arcified-release-linux-profile-path"),
      GetProfile()->GetPath().value()));
  ASSERT_TRUE(base::WriteFile(
      GetProfile()->GetPath().AppendASCII("arcified-release-linux-sentinel"),
      "test-owned data retained across a native browser restart\n"));

  PrefService* profile_prefs = GetProfile()->GetPrefs();
  profile_prefs->SetString(prefs::kHomePage,
                           "https://arcified-release-linux.invalid/");
  profile_prefs->SetBoolean(prefs::kShowHomeButton, true);
  base::RunLoop committed;
  profile_prefs->CommitPendingWrite(committed.QuitClosure());
  committed.Run();
}

IN_PROC_BROWSER_TEST_F(ArcifiedReleaseLinuxBrowserTest,
                       RetainsProfileDataAfterRestart) {
  ASSERT_NO_FATAL_FAILURE(CheckExplicitUserDataDirectory());
  base::ScopedAllowBlockingForTesting allow_blocking;
  base::FilePath user_data_dir;
  ASSERT_TRUE(base::PathService::Get(chrome::DIR_USER_DATA, &user_data_dir));
  std::string recorded_profile_path;
  ASSERT_TRUE(base::ReadFileToString(
      user_data_dir.AppendASCII("arcified-release-linux-profile-path"),
      &recorded_profile_path));
  EXPECT_EQ(GetProfile()->GetPath().value(), recorded_profile_path);
  std::string sentinel;
  ASSERT_TRUE(base::ReadFileToString(
      GetProfile()->GetPath().AppendASCII("arcified-release-linux-sentinel"),
      &sentinel));
  EXPECT_EQ("test-owned data retained across a native browser restart\n",
            sentinel);
  EXPECT_EQ("https://arcified-release-linux.invalid/",
            GetProfile()->GetPrefs()->GetString(prefs::kHomePage));
  EXPECT_TRUE(GetProfile()->GetPrefs()->GetBoolean(prefs::kShowHomeButton));
}

}  // namespace
