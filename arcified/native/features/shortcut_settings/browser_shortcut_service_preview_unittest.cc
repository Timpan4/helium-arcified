// Copyright 2026 The Helium Arcified Authors
// Use, redistribution and modification are governed by the GPL-3.0 license
// that can be found in the repository LICENSE file.

#include "chrome/browser/ui/browser_shortcuts/browser_shortcut_service.h"

#include <algorithm>
#include <memory>

#include "build/build_config.h"
#include "chrome/app/chrome_command_ids.h"
#include "chrome/browser/ui/browser_shortcuts/browser_shortcut_prefs.h"
#include "chrome/test/base/testing_profile.h"
#include "components/prefs/pref_service.h"
#include "content/public/test/browser_task_environment.h"
#include "testing/gtest/include/gtest/gtest.h"

#if BUILDFLAG(IS_MAC)
#include "chrome/browser/ui/browser_shortcuts/browser_shortcut_platform_mac.h"
#include "chrome/browser/ui/browser_shortcuts/browser_shortcut_serialization.h"
#endif

namespace browser_shortcuts {
namespace {

class ChangeCounter : public BrowserShortcutService::Observer {
 public:
  void OnBrowserShortcutsChanged() override { ++changes; }
  int changes = 0;
};

class BrowserShortcutPreviewTest : public testing::Test {
 protected:
  void SetUp() override {
    service_ = std::make_unique<BrowserShortcutService>(profile_.GetPrefs());
    service_->AddObserver(&counter_);
  }
  void TearDown() override { service_->RemoveObserver(&counter_); }

  content::BrowserTaskEnvironment task_environment_;
  TestingProfile profile_;
  ChangeCounter counter_;
  std::unique_ptr<BrowserShortcutService> service_;
};

TEST_F(BrowserShortcutPreviewTest, PreviewIsReadOnlyAndUsesNativeEquality) {
  ASSERT_EQ(AssignmentResult::kSuccess,
            service_->AssignAcceleratorToCommand(IDC_NEW_WINDOW,
                                                 "Control+Shift+KeyY"));
  const auto before = service_->GetAcceleratorMap();
  const auto pref =
      profile_.GetPrefs()->GetDict(prefs::kCustomAccelerators).Clone();
  const int changes = counter_.changes;
  // The recorder spelling differs from canonical serialization. Both parse to
  // the same native accelerator, unlike frontend string equality.
  const auto preview =
      service_->PreviewAcceleratorAssignment(IDC_NEW_TAB, "Shift+Control+KeyY");
  ASSERT_EQ(AssignmentResult::kSuccess, preview.result);
  ASSERT_EQ(1u, preview.conflicts.size());
  EXPECT_EQ(IDC_NEW_WINDOW, preview.conflicts[0].command_id);
  EXPECT_FALSE(preview.conflicts[0].locked);
  EXPECT_EQ(before, service_->GetAcceleratorMap());
  EXPECT_EQ(pref, profile_.GetPrefs()->GetDict(prefs::kCustomAccelerators));
  EXPECT_EQ(changes, counter_.changes);

  EXPECT_EQ(preview.result, service_->AssignAcceleratorToCommand(
                                IDC_NEW_TAB, "Shift+Control+KeyY"));
  EXPECT_EQ(IDC_NEW_TAB,
            service_->FindCommandForAccelerator("Control+Shift+KeyY"));
}

TEST_F(BrowserShortcutPreviewTest,
       ValidationOrderAndSelfAssignmentMatchMutation) {
  EXPECT_EQ(AssignmentResult::kUnknownCommand,
            service_->PreviewAcceleratorAssignment(-1, "Control+").result);
  EXPECT_EQ(AssignmentResult::kUnknownCommand,
            service_->AssignAcceleratorToCommand(-1, "Control+"));
  EXPECT_EQ(
      AssignmentResult::kInvalidAccelerator,
      service_->PreviewAcceleratorAssignment(IDC_NEW_TAB, "Control+").result);
  EXPECT_EQ(AssignmentResult::kInvalidAccelerator,
            service_->AssignAcceleratorToCommand(IDC_NEW_TAB, "Control+"));
  ASSERT_EQ(AssignmentResult::kSuccess, service_->AssignAcceleratorToCommand(
                                            IDC_NEW_TAB, "Control+Shift+KeyY"));
  const auto preview =
      service_->PreviewAcceleratorAssignment(IDC_NEW_TAB, "Control+Shift+KeyY");
  EXPECT_EQ(AssignmentResult::kSuccess, preview.result);
  EXPECT_TRUE(preview.conflicts.empty());
}

TEST_F(BrowserShortcutPreviewTest,
       ResetPreviewMatchesDisplacementWithoutWrites) {
  const auto& commands = service_->GetCommands();
  // A macOS unit test may have no main menu, making Cocoa defaults locked.
  // Select from the service's actual modifiable defaults instead of assuming
  // that a particular menu-owned command can be reassigned.
  const auto command = std::ranges::find_if(commands, [](const auto& entry) {
    return entry.command_id != IDC_NEW_WINDOW && !entry.accelerators.empty() &&
           std::ranges::all_of(entry.accelerators, [](const auto& shortcut) {
             return shortcut.user_modifiable;
           });
  });
  ASSERT_NE(commands.end(), command);
  const int command_id = command->command_id;
  const auto defaults = command->accelerators;
  ASSERT_FALSE(defaults.empty());
  for (const auto& shortcut : defaults) {
    ASSERT_TRUE(shortcut.user_modifiable);
    ASSERT_EQ(AssignmentResult::kSuccess,
              service_->AssignAcceleratorToCommand(IDC_NEW_WINDOW,
                                                   shortcut.serialized));
  }
  const auto before = service_->GetAcceleratorMap();
  const auto pref =
      profile_.GetPrefs()->GetDict(prefs::kCustomAccelerators).Clone();
  const int changes = counter_.changes;
  const auto preview = service_->PreviewAcceleratorReset(command_id);
  ASSERT_EQ(AssignmentResult::kSuccess, preview.result);
  ASSERT_EQ(1u, preview.conflicts.size());
  EXPECT_EQ(IDC_NEW_WINDOW, preview.conflicts[0].command_id);
  EXPECT_FALSE(preview.conflicts[0].locked);
  EXPECT_EQ(before, service_->GetAcceleratorMap());
  EXPECT_EQ(pref, profile_.GetPrefs()->GetDict(prefs::kCustomAccelerators));
  EXPECT_EQ(changes, counter_.changes);
  ASSERT_TRUE(service_->ResetAcceleratorsForCommand(command_id));
  for (const auto& shortcut : defaults) {
    EXPECT_EQ(command_id,
              service_->FindCommandForAccelerator(shortcut.serialized));
  }
  EXPECT_TRUE(service_->PreviewAcceleratorReset(command_id).conflicts.empty());
  EXPECT_EQ(AssignmentResult::kUnknownCommand,
            service_->PreviewAcceleratorReset(-1).result);
  EXPECT_FALSE(service_->ResetAcceleratorsForCommand(-1));
}

TEST_F(BrowserShortcutPreviewTest, CommandWithoutDefaultsResetsToEmpty) {
  const auto& commands = service_->GetCommands();
  const auto command = std::ranges::find_if(
      commands, [](const auto& entry) { return entry.accelerators.empty(); });
  ASSERT_NE(commands.end(), command);
  const int command_id = command->command_id;
  ASSERT_EQ(AssignmentResult::kSuccess, service_->AssignAcceleratorToCommand(
                                            command_id, "Control+Shift+KeyY"));
  const auto preview = service_->PreviewAcceleratorReset(command_id);
  EXPECT_EQ(AssignmentResult::kSuccess, preview.result);
  EXPECT_TRUE(preview.conflicts.empty());
  ASSERT_TRUE(service_->ResetAcceleratorsForCommand(command_id));
  EXPECT_FALSE(service_->FindCommandForAccelerator("Control+Shift+KeyY"));
}

TEST_F(BrowserShortcutPreviewTest, PreviewDoesNotReserveAnAccelerator) {
  ASSERT_EQ(AssignmentResult::kSuccess, service_->AssignAcceleratorToCommand(
                                            IDC_NEW_TAB, "Control+Shift+KeyY"));
  const auto old_preview =
      service_->PreviewAcceleratorAssignment(IDC_NEW_TAB, "Control+Shift+KeyY");
  EXPECT_TRUE(old_preview.conflicts.empty());
  ASSERT_EQ(AssignmentResult::kSuccess,
            service_->AssignAcceleratorToCommand(IDC_NEW_WINDOW,
                                                 "Control+Shift+KeyY"));
  const auto refreshed =
      service_->PreviewAcceleratorAssignment(IDC_NEW_TAB, "Control+Shift+KeyY");
  ASSERT_EQ(1u, refreshed.conflicts.size());
  EXPECT_EQ(IDC_NEW_WINDOW, refreshed.conflicts[0].command_id);
  EXPECT_TRUE(old_preview.conflicts.empty());
}

#if BUILDFLAG(IS_MAC)
TEST_F(BrowserShortcutPreviewTest, LockedPlatformDefaultsRefuseWithoutWrites) {
  const auto defaults = GetPlatformDefaultAccelerators();
  const auto locked = std::ranges::find_if(defaults, [](const auto& entry) {
    return !entry.user_modifiable && entry.command_id != IDC_NEW_TAB;
  });
  ASSERT_NE(defaults.end(), locked);
  const std::string serialized = AcceleratorToString(locked->accelerator);
  ASSERT_FALSE(serialized.empty());
  const auto before = service_->GetAcceleratorMap();
  const auto pref =
      profile_.GetPrefs()->GetDict(prefs::kCustomAccelerators).Clone();
  const int changes = counter_.changes;
  const auto preview =
      service_->PreviewAcceleratorAssignment(IDC_NEW_TAB, serialized);
  ASSERT_EQ(AssignmentResult::kShortcutLocked, preview.result);
  const auto conflict = std::ranges::find(preview.conflicts, locked->command_id,
                                          &ShortcutConflict::command_id);
  ASSERT_NE(preview.conflicts.end(), conflict);
  EXPECT_TRUE(conflict->locked);
  EXPECT_EQ(preview.result,
            service_->AssignAcceleratorToCommand(IDC_NEW_TAB, serialized));
  EXPECT_EQ(before, service_->GetAcceleratorMap());
  EXPECT_EQ(pref, profile_.GetPrefs()->GetDict(prefs::kCustomAccelerators));
  EXPECT_EQ(changes, counter_.changes);
}
#endif

}  // namespace
}  // namespace browser_shortcuts
