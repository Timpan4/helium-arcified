// Copyright 2026 The Helium Arcified Authors
// SPDX-License-Identifier: GPL-3.0-only

#include "chrome/browser/ui/webui/help/version_updater.h"

#include <cstdint>
#include <string>

#include "base/test/bind.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

TEST(ArcifiedVersionUpdaterWinTest, ReportsUnavailableWithoutOfferingMutation) {
  auto updater = VersionUpdater::Create(nullptr);
  int replies = 0;
  bool promoted = false;
  updater->CheckForUpdate(
      base::BindLambdaForTesting(
          [&](VersionUpdater::Status status, int progress, bool rollback,
              bool powerwash, const std::string& version, int64_t size,
              const std::u16string& message) {
            ++replies;
            EXPECT_EQ(VersionUpdater::DISABLED, status);
            EXPECT_EQ(0, progress);
            EXPECT_FALSE(rollback);
            EXPECT_FALSE(powerwash);
            EXPECT_TRUE(version.empty());
            EXPECT_EQ(0, size);
            EXPECT_FALSE(message.empty());
          }),
      base::BindLambdaForTesting(
          [&](VersionUpdater::PromotionState) { promoted = true; }));
  EXPECT_EQ(1, replies);
  EXPECT_FALSE(promoted);
}

TEST(ArcifiedVersionUpdaterWinTest, CallbackCanDestroyUpdater) {
  auto updater = VersionUpdater::Create(nullptr);
  bool replied = false;
  updater->CheckForUpdate(
      base::BindLambdaForTesting([&](VersionUpdater::Status status, int, bool,
                                     bool, const std::string&, int64_t,
                                     const std::u16string&) {
        EXPECT_EQ(VersionUpdater::DISABLED, status);
        replied = true;
        updater.reset();
      }),
      {});
  EXPECT_TRUE(replied);
  EXPECT_FALSE(updater);
}

TEST(ArcifiedVersionUpdaterWinTest, RepeatedRequestsStayUnavailable) {
  auto updater = VersionUpdater::Create(nullptr);
  int replies = 0;
  auto callback = base::BindLambdaForTesting(
      [&](VersionUpdater::Status status, int, bool, bool, const std::string&,
          int64_t, const std::u16string&) {
        EXPECT_EQ(VersionUpdater::DISABLED, status);
        ++replies;
      });
  updater->CheckForUpdate(callback, {});
  updater->CheckForUpdate(callback, {});
  EXPECT_EQ(2, replies);
}

}  // namespace
