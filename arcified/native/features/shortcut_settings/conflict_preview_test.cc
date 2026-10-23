// Copyright 2026 The Helium Arcified Authors
// Use, redistribution and modification are governed by the GPL-3.0 license
// that can be found in the repository LICENSE file.

#include "conflict_preview.h"

#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

namespace {

// Synthetic equality token only. Native gtests use actual ui::Accelerator.
struct Entry {
  int accelerator;
  bool user_modifiable;
};
using Bindings = std::map<int, std::vector<Entry>>;
using browser_shortcuts::ConflictScope;
using browser_shortcuts::FindShortcutConflicts;
using browser_shortcuts::ShortcutConflict;

void Expect(const std::vector<ShortcutConflict>& actual,
            const std::vector<ShortcutConflict>& expected) {
  if (actual.size() != expected.size()) {
    std::abort();
  }
  for (size_t i = 0; i < expected.size(); ++i) {
    if (actual[i].command_id != expected[i].command_id ||
        actual[i].locked != expected[i].locked) {
      std::abort();
    }
  }
}

void AssignmentReportsAllCommandsOnce() {
  const Bindings current{{10, {{1, true}, {2, true}}},
                         {20, {{1, true}, {2, false}}},
                         {30, {{3, true}}},
                         {40, {{1, true}, {2, true}}}};
  const std::vector<Entry> proposed{{1, true}, {2, false}, {1, true}};
  Expect(
      FindShortcutConflicts(10, current, proposed, ConflictScope::kAssignment),
      {{20, true}, {40, false}});
  // Reset does not remove locked bindings, even when that command also loses
  // another modifiable binding. Proposed entry mutability is irrelevant.
  Expect(FindShortcutConflicts(10, current, proposed, ConflictScope::kReset),
         {{20, false}, {40, false}});
}

void EmptyAndSelfOnlyHaveNoConflicts() {
  const Bindings current{{10, {{1, false}}}, {20, {{2, true}}}};
  Expect(FindShortcutConflicts(10, current, std::vector<Entry>{},
                               ConflictScope::kAssignment),
         {});
  Expect(FindShortcutConflicts(10, current, std::vector<Entry>{{1, true}},
                               ConflictScope::kAssignment),
         {});
  Expect(FindShortcutConflicts(20, current, std::vector<Entry>{{1, true}},
                               ConflictScope::kReset),
         {});
}

void PlatformConflictsMergeWithoutLosingLocks() {
  std::vector<ShortcutConflict> conflicts;
  browser_shortcuts::AddShortcutConflict(conflicts, 30, false);
  browser_shortcuts::AddShortcutConflict(conflicts, 10, true);
  browser_shortcuts::AddShortcutConflict(conflicts, 30, true);
  browser_shortcuts::AddShortcutConflict(conflicts, 10, false);
  Expect(conflicts, {{10, true}, {30, true}});
}

}  // namespace

int main() {
  AssignmentReportsAllCommandsOnce();
  EmptyAndSelfOnlyHaveNoConflicts();
  PlatformConflictsMergeWithoutLosingLocks();
  std::cout << "shortcut conflict collector fixtures passed\n";
}
