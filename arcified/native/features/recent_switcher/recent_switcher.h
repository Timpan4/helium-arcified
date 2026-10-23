#ifndef ARCIFIED_NATIVE_FEATURES_RECENT_SWITCHER_RECENT_SWITCHER_H_
#define ARCIFIED_NATIVE_FEATURES_RECENT_SWITCHER_RECENT_SWITCHER_H_

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../../model/lifecycle.h"

namespace arcified::recent_switcher {

enum class Direction { kForward, kReverse };

// Recent-first, supplied by the activation-history owner. Aliases resolve to
// the existing PageId before entering this list. The first occurrence wins.
struct HistoryEntry {
  lifecycle::PageId page;
  lifecycle::SpaceId requested_space;
};

// Display data only. Never an identity, activity policy or navigation snapshot.
struct Preview {
  std::string title;
  std::string space_label;
  std::string status;
};

struct Snapshot {
  lifecycle::WindowId window;
  // Caller changes this on external focus/selection/context changes and never
  // reuses it during a session. This is not a model revision or native handle.
  std::uint64_t context_generation;
  std::optional<lifecycle::PageId> focused_page;
  // Explicit caller policy. No implicit all-window or all-Space MRU scope.
  std::vector<lifecycle::SpaceId> eligible_spaces;
  std::vector<HistoryEntry> history;
  std::map<lifecycle::PageId, Preview> previews;
};

// A refreshed UI projection, not a command target or permission to transfer.
struct Candidate {
  HistoryEntry entry;
  lifecycle::ProfileId profile;
  lifecycle::Attachment source;
  lifecycle::Attachment destination;
  std::optional<Preview> preview;
};

enum class Update {
  kHolding,
  kIdle,
  kAlreadyHolding,
  kContextChanged,
  kNoCandidates,
};

enum class EndReason {
  kIdle,
  kContextChanged,
  kNoCandidates,
  kSelectionChanged,
  kAlreadyFocused,
};

// A plan-specific request for the external command/transfer coordinator.
// It is not a shared PageCommandTarget, reservation or native commit token.
// Re-resolve the page with the shared command resolver, recheck destination,
// caller generation and native identity, then execute through lifecycle owners.
struct ActivationIntent {
  HistoryEntry entry;
  lifecycle::Attachment expected_source;
  lifecycle::Attachment destination;
  std::uint64_t context_generation;
};
using ReleaseResult = std::variant<EndReason, ActivationIntent>;

// Ephemeral state for one hold. The caller serializes access and supplies an
// authoritative snapshot paired with the read-only model. No method selects a
// Space, transfers a page, changes focus, records activation history, persists
// data or opens a page.
class Session {
 public:
  Update Begin(const lifecycle::Model& model,
               const Snapshot& snapshot,
               Direction direction);
  Update Cycle(const lifecycle::Model& model,
               const Snapshot& snapshot,
               Direction direction);
  Update Reconcile(const lifecycle::Model& model, const Snapshot& snapshot);
  ReleaseResult Release(const lifecycle::Model& model,
                        const Snapshot& snapshot);
  void Cancel();

  bool holding() const { return held_.has_value(); }
  // Views are invalidated by the next non-const call. Reconcile before drawing.
  const std::vector<Candidate>& candidates() const;
  const Candidate* selected() const;

 private:
  struct Held {
    lifecycle::WindowId window;
    std::uint64_t context_generation;
    Direction direction;
    std::vector<Candidate> candidates;
    std::size_t selection;
  };

  std::optional<Held> held_;
};

}  // namespace arcified::recent_switcher

#endif  // ARCIFIED_NATIVE_FEATURES_RECENT_SWITCHER_RECENT_SWITCHER_H_
