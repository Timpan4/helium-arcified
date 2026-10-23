#ifndef ARCIFIED_NATIVE_FEATURES_ACCESSIBILITY_SIDEBAR_ACCESSIBILITY_H_
#define ARCIFIED_NATIVE_FEATURES_ACCESSIBILITY_SIDEBAR_ACCESSIBILITY_H_

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace arcified::accessibility {

// Presentation identities supplied by the host, not lifecycle IDs or indices.
struct RowId {
  std::string value;
  friend bool operator==(const RowId& a, const RowId& b) {
    return a.value == b.value;
  }
  friend bool operator!=(const RowId& a, const RowId& b) { return !(a == b); }
  friend bool operator<(const RowId& a, const RowId& b) {
    return a.value < b.value;
  }
};
struct CommandId {
  std::string value;
  friend bool operator==(const CommandId& a, const CommandId& b) {
    return a.value == b.value;
  }
  friend bool operator<(const CommandId& a, const CommandId& b) {
    return a.value < b.value;
  }
};

enum class RowKind { kSpace, kTab, kFolder };
struct Command {
  CommandId id;
  std::u16string_view name;  // Host-localized menu/accessibility label.
  bool enabled;
};
struct Row {
  RowId id;
  std::u16string_view name;
  RowKind kind;
  std::optional<RowId> parent;
  std::optional<bool> selected;  // Absent means not selectable.
  // Absent means a leaf, including empty folders.
  std::optional<bool> expanded;
  std::vector<Command> commands;
};

// All rows map to tree items in a host-labeled tree. Commands map to menu items
// in a host-labeled menu. This module does not create native AX objects/events.
struct RowSemantics {
  const Row* row;
  size_t level;  // One-based, as are position and set_size.
  size_t position;
  size_t set_size;
};

enum class TreeKey { kUp, kDown, kHome, kEnd, kLeft, kRight };
enum class TextDirection { kLeftToRight, kRightToLeft };
struct FocusSuggestion {
  RowId row;
};
struct ExpansionRequest {
  RowId row;
  bool expanded;
};
using KeyResult =
    std::variant<std::monostate, FocusSuggestion, ExpansionRequest>;
struct CommandRequest {
  RowId row;
  CommandId command;
};

enum class ChangeKind {
  kAdded,
  kRemoved,
  kName,
  kKind,
  kHierarchy,
  kSelection,
  kExpansion,
  kCommands,
};
struct SemanticChange {
  RowId row;
  ChangeKind kind;
};

// A synchronous, read-only borrow of a complete visible logical preorder,
// including offscreen rows but excluding collapsed descendants. Rows, strings
// and commands must remain alive and unchanged until this borrow is discarded.
// No folder/order/selection state is owned here. See README.md for host duties.
class Projection {
 public:
  // Empty projections are valid. Malformed identities/names, duplicate row or
  // per-row command IDs, and inconsistent parent/preorder/expansion facts fail.
  static std::optional<Projection> Borrow(const std::vector<Row>& rows);
  static std::optional<Projection> Borrow(std::vector<Row>&& rows) = delete;
  static std::optional<Projection> Borrow(const std::vector<Row>&& rows) =
      delete;

  std::optional<RowSemantics> Describe(const RowId& row) const;
  // Direction is the native UI text direction, not the sidebar's screen edge.
  // Missing/stale focus and boundary keys return monostate. Never selects.
  KeyResult OnKey(const RowId& focused,
                  TreeKey key,
                  TextDirection direction) const;
  // Menu, keyboard and assistive actions use this same advertised capability
  // gate. Missing or disabled commands return nullopt. Results own their IDs;
  // they are suggestions, not authorization tokens for delayed native dispatch.
  std::optional<CommandRequest> RequestCommand(const RowId& row,
                                               const CommandId& command) const;

  // Call only after the host reports an actual projection change. nullopt focus
  // means focus is outside the tree and is never pulled inside. Keep surviving
  // focus, otherwise suggest next sibling, previous sibling, then parent;
  // repeat up the old ancestry when an entire subtree disappeared. Unknown old
  // focus or no surviving candidate yields nullopt for the host's own fallback.
  std::optional<FocusSuggestion> SuggestFocusAfterChange(
      const Projection& before,
      const std::optional<RowId>& focused) const;

  // Facts that changed, not platform events or spoken text. Host chooses event
  // ordering/coalescing and publishes only after native success. No focus event
  // is inferred from selection, expansion, insertion or undo.
  std::vector<SemanticChange> ChangesSince(const Projection& before) const;

 private:
  struct Position {
    std::optional<size_t> parent;
    size_t level;
    size_t position;
    size_t set_size;
  };
  Projection(const std::vector<Row>& rows,
             std::vector<Position> positions,
             std::map<RowId, size_t> indices);
  std::optional<size_t> Find(const RowId& row) const;

  const std::vector<Row>* rows_;
  std::vector<Position> positions_;  // Derived metadata for this borrow only.
  std::map<RowId, size_t> indices_;
};

}  // namespace arcified::accessibility

#endif  // ARCIFIED_NATIVE_FEATURES_ACCESSIBILITY_SIDEBAR_ACCESSIBILITY_H_
