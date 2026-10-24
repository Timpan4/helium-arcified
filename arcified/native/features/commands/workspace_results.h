#ifndef ARCIFIED_NATIVE_FEATURES_COMMANDS_WORKSPACE_RESULTS_H_
#define ARCIFIED_NATIVE_FEATURES_COMMANDS_WORKSPACE_RESULTS_H_

#include "command_bar.h"
#include "command_ranking.h"

namespace arcified::commands {

enum class WorkspaceResultMode { kActivate, kReveal };

// Converts an already scoped ranking row to a typed bar action. The caller
// supplies a localized consequence label including the desired provenance.
// Generated row identity includes action and semantic target, so changed
// targets cannot silently inherit the selected row during a result refresh.
// Labels and navigation text do not enter identity. No execution/revalidation,
// URL interpretation or Model mutation occurs here.
Result MakeWorkspaceResult(const RankedCommand& command,
                           WorkspaceResultMode mode,
                           std::string action_label);

}  // namespace arcified::commands

#endif  // ARCIFIED_NATIVE_FEATURES_COMMANDS_WORKSPACE_RESULTS_H_
