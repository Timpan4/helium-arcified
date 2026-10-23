#include "sidebar_accessibility.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace arcified::accessibility {
namespace {

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

const RowId kSpace{"space"};
const RowId kFolder{"folder"};
const RowId kFirst{"first"};
const RowId kSecond{"second"};
const RowId kLast{"last"};
const RowId kOther{"other-space"};
const RowId kMissing{"missing"};
constexpr auto kLtr = TextDirection::kLeftToRight;
constexpr auto kRtl = TextDirection::kRightToLeft;

std::vector<Row> Rows() {
  // Duplicate names deliberately identify distinct synthetic rows.
  return {{kSpace, u"Research", RowKind::kSpace, {}, true, true, {}},
          {kFolder, u"Reading", RowKind::kFolder, kSpace, {}, true, {}},
          {kFirst,
           u"Page",
           RowKind::kTab,
           kFolder,
           false,
           {},
           {{{"move-up"}, u"Move up", true},
            {{"move-to-folder"}, u"Move to folder", true},
            {{"keep"}, u"Keep in this Space", false}}},
          {kSecond, u"Page", RowKind::kTab, kFolder, true, {}, {}},
          {kLast, u"Notes", RowKind::kTab, kSpace, false, {}, {}},
          {kOther, u"Personal", RowKind::kSpace, {}, false, {}, {}}};
}

Projection Borrow(const std::vector<Row>& rows) {
  const auto result = Projection::Borrow(rows);
  REQUIRE(result.has_value());
  return *result;
}

void ExpectFocus(const KeyResult& result, const RowId& row) {
  REQUIRE(std::holds_alternative<FocusSuggestion>(result));
  REQUIRE(std::get<FocusSuggestion>(result).row == row);
}

void ExpectExpansion(const KeyResult& result, const RowId& row, bool expanded) {
  REQUIRE(std::holds_alternative<ExpansionRequest>(result));
  REQUIRE(std::get<ExpansionRequest>(result).row == row);
  REQUIRE(std::get<ExpansionRequest>(result).expanded == expanded);
}

void Remove(std::vector<Row>& rows, const RowId& row) {
  rows.erase(std::remove_if(
                 rows.begin(), rows.end(),
                 [&row](const Row& candidate) { return candidate.id == row; }),
             rows.end());
}

void SemanticsAndKeyboard() {
  const auto rows = Rows();
  const auto projection = Borrow(rows);
  const auto space = projection.Describe(kSpace);
  REQUIRE(space->level == 1 && space->position == 1 && space->set_size == 2);
  const auto folder = projection.Describe(kFolder);
  REQUIRE(folder->level == 2 && folder->position == 1 && folder->set_size == 2);
  REQUIRE(!folder->row->selected.has_value());
  const auto second = projection.Describe(kSecond);
  REQUIRE(second->level == 3 && second->position == 2 && second->set_size == 2);
  REQUIRE(second->row->selected == true);
  REQUIRE(!second->row->expanded.has_value());
  REQUIRE(!projection.Describe(kMissing));

  ExpectFocus(projection.OnKey(kFirst, TreeKey::kDown, kLtr), kSecond);
  ExpectFocus(projection.OnKey(kLast, TreeKey::kUp, kLtr), kSecond);
  ExpectFocus(projection.OnKey(kSecond, TreeKey::kHome, kLtr), kSpace);
  ExpectFocus(projection.OnKey(kFirst, TreeKey::kEnd, kLtr), kOther);
  ExpectFocus(projection.OnKey(kSpace, TreeKey::kRight, kLtr), kFolder);
  ExpectFocus(projection.OnKey(kFolder, TreeKey::kRight, kLtr), kFirst);
  ExpectFocus(projection.OnKey(kFirst, TreeKey::kLeft, kLtr), kFolder);
  ExpectExpansion(projection.OnKey(kFolder, TreeKey::kLeft, kLtr), kFolder,
                  false);
  ExpectExpansion(projection.OnKey(kSpace, TreeKey::kRight, kRtl), kSpace,
                  false);
  ExpectFocus(projection.OnKey(kSpace, TreeKey::kLeft, kRtl), kFolder);
  ExpectFocus(projection.OnKey(kSecond, TreeKey::kRight, kRtl), kFolder);
  REQUIRE(std::holds_alternative<std::monostate>(
      projection.OnKey(kSpace, TreeKey::kUp, kLtr)));
  REQUIRE(std::holds_alternative<std::monostate>(
      projection.OnKey(kOther, TreeKey::kDown, kLtr)));
  REQUIRE(std::holds_alternative<std::monostate>(
      projection.OnKey(kFirst, TreeKey::kRight, kLtr)));
  REQUIRE(std::holds_alternative<std::monostate>(
      projection.OnKey(kMissing, TreeKey::kHome, kLtr)));
  REQUIRE(projection.Describe(kSecond)->row->selected == true);
  REQUIRE(projection.Describe(kFirst)->row->selected == false);

  auto collapsed = Rows();
  Remove(collapsed, kFirst);
  Remove(collapsed, kSecond);
  collapsed[1].expanded = false;
  const auto closed = Borrow(collapsed);
  ExpectExpansion(closed.OnKey(kFolder, TreeKey::kRight, kLtr), kFolder, true);
  ExpectFocus(closed.OnKey(kFolder, TreeKey::kDown, kLtr), kLast);
  ExpectFocus(closed.OnKey(kFolder, TreeKey::kLeft, kLtr), kSpace);
  ExpectExpansion(closed.OnKey(kFolder, TreeKey::kLeft, kRtl), kFolder, true);
}

void CommandCapabilityGate() {
  std::optional<CommandRequest> request;
  {
    const auto rows = Rows();
    const auto projection = Borrow(rows);
    request = projection.RequestCommand(kFirst, {"move-up"});
    REQUIRE(request.has_value());
    REQUIRE(projection.RequestCommand(kFirst, {"move-to-folder"}).has_value());
    REQUIRE(!projection.RequestCommand(kFirst, {"keep"}));
    REQUIRE(!projection.RequestCommand(kFirst, {"unadvertised"}));
    REQUIRE(!projection.RequestCommand(kSecond, {"move-up"}));
    REQUIRE(!projection.RequestCommand(kMissing, {"move-up"}));
  }
  // Native asynchronous routing can retain owned identities, never row
  // pointers.
  REQUIRE(request->row == kFirst);
  REQUIRE(request->command == CommandId{"move-up"});
  auto fresh = Rows();
  fresh[2].commands[0].enabled = false;
  REQUIRE(!Borrow(fresh).RequestCommand(request->row, request->command));
}

void FocusSuggestions() {
  const auto before_rows = Rows();
  const auto before = Borrow(before_rows);
  auto after_rows = Rows();
  Remove(after_rows, kFirst);
  REQUIRE(Borrow(after_rows).SuggestFocusAfterChange(before, kFirst)->row ==
          kSecond);
  REQUIRE(Borrow(after_rows).SuggestFocusAfterChange(before, kLast)->row ==
          kLast);
  REQUIRE(!Borrow(after_rows).SuggestFocusAfterChange(before, std::nullopt));
  REQUIRE(!Borrow(after_rows).SuggestFocusAfterChange(before, kMissing));

  after_rows = Rows();
  Remove(after_rows, kSecond);
  REQUIRE(Borrow(after_rows).SuggestFocusAfterChange(before, kSecond)->row ==
          kFirst);

  after_rows = Rows();
  Remove(after_rows, kFirst);
  Remove(after_rows, kSecond);
  after_rows[1].expanded = false;
  REQUIRE(Borrow(after_rows).SuggestFocusAfterChange(before, kSecond)->row ==
          kFolder);

  Remove(after_rows, kFolder);
  REQUIRE(Borrow(after_rows).SuggestFocusAfterChange(before, kFirst)->row ==
          kLast);

  Remove(after_rows, kLast);
  after_rows[0].expanded = false;
  REQUIRE(Borrow(after_rows).SuggestFocusAfterChange(before, kFirst)->row ==
          kSpace);

  // Undo/reveal inserts rows but cannot steal surviving or outside focus.
  REQUIRE(before.SuggestFocusAfterChange(Borrow(after_rows), kSpace)->row ==
          kSpace);
  REQUIRE(!before.SuggestFocusAfterChange(Borrow(after_rows), std::nullopt));
  const std::vector<Row> empty;
  REQUIRE(!Borrow(empty).SuggestFocusAfterChange(before, kFirst));
  REQUIRE(std::holds_alternative<std::monostate>(
      Borrow(empty).OnKey(kMissing, TreeKey::kEnd, kLtr)));
}

bool HasChange(const std::vector<SemanticChange>& changes,
               const RowId& row,
               ChangeKind kind) {
  return std::any_of(changes.begin(), changes.end(),
                     [&](const SemanticChange& change) {
                       return change.row == row && change.kind == kind;
                     });
}

void ChangeMetadata() {
  const auto before_rows = Rows();
  const auto before = Borrow(before_rows);
  REQUIRE(before.ChangesSince(before).empty());
  auto after_rows = Rows();
  Remove(after_rows, kSecond);
  after_rows[2].name = u"Renamed";
  after_rows[2].selected = true;
  after_rows[2].commands[0].enabled = false;
  const auto changes = Borrow(after_rows).ChangesSince(before);
  REQUIRE(changes.size() == 5);
  REQUIRE(HasChange(changes, kSecond, ChangeKind::kRemoved));
  REQUIRE(HasChange(changes, kFirst, ChangeKind::kName));
  REQUIRE(HasChange(changes, kFirst, ChangeKind::kSelection));
  REQUIRE(HasChange(changes, kFirst, ChangeKind::kCommands));
  REQUIRE(HasChange(changes, kFirst, ChangeKind::kHierarchy));
  REQUIRE(HasChange(before.ChangesSince(Borrow(after_rows)), kSecond,
                    ChangeKind::kAdded));

  auto moved_rows = Rows();
  std::swap(moved_rows[2], moved_rows[3]);
  const auto reordered = Borrow(moved_rows).ChangesSince(before);
  REQUIRE(reordered.size() == 2);
  REQUIRE(HasChange(reordered, kFirst, ChangeKind::kHierarchy));
  REQUIRE(HasChange(reordered, kSecond, ChangeKind::kHierarchy));

  auto collapsed = Rows();
  Remove(collapsed, kFirst);
  Remove(collapsed, kSecond);
  collapsed[1].expanded = false;
  REQUIRE(HasChange(Borrow(collapsed).ChangesSince(before), kFolder,
                    ChangeKind::kExpansion));

  auto reparented = Rows();
  reparented[3].parent = kSpace;
  REQUIRE(HasChange(Borrow(reparented).ChangesSince(before), kSecond,
                    ChangeKind::kHierarchy));
  auto kind = Rows();
  kind[4].kind = RowKind::kFolder;
  REQUIRE(
      HasChange(Borrow(kind).ChangesSince(before), kLast, ChangeKind::kKind));
}

void InvalidBoundaryInputs() {
  auto rows = Rows();
  rows[1].id = kSpace;
  REQUIRE(!Projection::Borrow(rows));
  rows = Rows();
  rows[0].id.value.clear();
  REQUIRE(!Projection::Borrow(rows));
  rows = Rows();
  rows[0].name = u"";
  REQUIRE(!Projection::Borrow(rows));
  rows = Rows();
  rows[2].parent = kMissing;
  REQUIRE(!Projection::Borrow(rows));
  rows = Rows();
  rows[1].parent = kFirst;  // Parent cannot be a future row or cycle.
  REQUIRE(!Projection::Borrow(rows));
  rows = Rows();
  rows[1].parent = kFolder;
  REQUIRE(!Projection::Borrow(rows));
  rows = Rows();
  rows[1].expanded =
      false;  // Hidden children cannot remain in this projection.
  REQUIRE(!Projection::Borrow(rows));
  rows = Rows();
  rows[2].expanded = false;  // Tabs are leaves, not collapsed containers.
  REQUIRE(!Projection::Borrow(rows));
  rows = Rows();
  std::swap(rows[3], rows[4]);  // A subtree cannot resume after its sibling.
  REQUIRE(!Projection::Borrow(rows));
  rows = Rows();
  rows[2].commands.push_back(rows[2].commands[0]);
  REQUIRE(!Projection::Borrow(rows));
  rows = Rows();
  rows[2].commands[0].name = u"";
  REQUIRE(!Projection::Borrow(rows));
  rows = Rows();
  rows[2].commands[0].id.value.clear();
  REQUIRE(!Projection::Borrow(rows));
}

}  // namespace
}  // namespace arcified::accessibility

int main() {
  using namespace arcified::accessibility;
  static_assert(!std::is_convertible_v<RowId, CommandId>);
  SemanticsAndKeyboard();
  CommandCapabilityGate();
  FocusSuggestions();
  ChangeMetadata();
  InvalidBoundaryInputs();
  std::cout << "sidebar accessibility fixtures passed\n";
}
