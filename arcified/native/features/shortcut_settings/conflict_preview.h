// Copyright 2026 The Helium Arcified Authors
// Use, redistribution and modification are governed by the GPL-3.0 license
// that can be found in the repository LICENSE file.

#ifndef CHROME_BROWSER_UI_BROWSER_SHORTCUTS_CONFLICT_PREVIEW_H_
#define CHROME_BROWSER_UI_BROWSER_SHORTCUTS_CONFLICT_PREVIEW_H_

#include <algorithm>
#include <vector>

namespace browser_shortcuts {

struct ShortcutConflict {
  int command_id;
  bool locked;
};

// Assignment reports locked matches too. Reset removes only modifiable ones.
enum class ConflictScope { kAssignment, kReset };

inline void AddShortcutConflict(std::vector<ShortcutConflict>& conflicts,
                                int command_id,
                                bool locked) {
  auto it = std::lower_bound(conflicts.begin(), conflicts.end(), command_id,
                             [](const ShortcutConflict& conflict, int id) {
                               return conflict.command_id < id;
                             });
  if (it != conflicts.end() && it->command_id == command_id) {
    it->locked = it->locked || locked;
  } else {
    conflicts.insert(it, {command_id, locked});
  }
}

// Borrow the service's current map and proposed AcceleratorEntry range. No
// command catalog, binding store, parsing or accelerator equality lives here.
// Production instantiates this with ui::Accelerator through AcceleratorEntry.
template <typename CommandAccelerators, typename AcceleratorEntries>
std::vector<ShortcutConflict> FindShortcutConflicts(
    int command_id_to_ignore,
    const CommandAccelerators& current,
    const AcceleratorEntries& proposed,
    ConflictScope scope) {
  std::vector<ShortcutConflict> conflicts;
  for (const auto& [command_id, entries] : current) {
    if (command_id == command_id_to_ignore) {
      continue;
    }
    for (const auto& entry : entries) {
      if (scope == ConflictScope::kReset && !entry.user_modifiable) {
        continue;
      }
      for (const auto& candidate : proposed) {
        if (entry.accelerator == candidate.accelerator) {
          AddShortcutConflict(conflicts, command_id, !entry.user_modifiable);
        }
      }
    }
  }
  return conflicts;
}

}  // namespace browser_shortcuts

#endif  // CHROME_BROWSER_UI_BROWSER_SHORTCUTS_CONFLICT_PREVIEW_H_
