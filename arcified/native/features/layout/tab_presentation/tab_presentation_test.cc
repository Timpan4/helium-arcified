#include "tab_presentation.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

#include "../focus_mode/focus_mode.h"

namespace arcified::tab_presentation {
namespace {

namespace lc = lifecycle;
namespace sb = sidebar;

void Require(bool condition, const char* expression, int line) {
  if (condition)
    return;
  std::cerr << "line " << line << ": " << expression << '\n';
  std::exit(EXIT_FAILURE);
}
#define REQUIRE(expression) Require((expression), #expression, __LINE__)

// Synthetic measurements, not product defaults, limits or platform evidence.
const FitMetrics kRoom{{1200, 800}, {700, 300}, {400, 360}};
const lc::ProfileId kProfile{"work"};
const lc::Attachment kAttachment{lc::WindowId{"host"}, lc::SpaceId{"desk"}};
const lc::ItemId kSaved{"z-saved"};
const lc::ItemId kClosed{"a-closed"};
const lc::ShortcutId kAlias{"alias"};
const lc::PageId kTemporary{"temporary"};
const lc::PageId kLive{"live-saved"};

struct Fixture {
  // Deliberately different from identity/map order. Alias and item stay
  // distinct.
  std::vector<TabIdentity> order{kTemporary, kSaved, kAlias, kClosed};
  std::optional<TabIdentity> selected{TabIdentity{kAlias}};
  AuthoritativeTabs tabs() const {
    return {std::cref(kAttachment), std::cref(kProfile), std::cref(order),
            std::cref(selected)};
  }
};

Presentation Present(AuthoritativeTabs tabs,
                     Preferences preferences,
                     sb::Side side = sb::Side::kLeft,
                     FitMetrics metrics = kRoom) {
  auto result = Project(tabs, preferences, side, metrics);
  REQUIRE(std::holds_alternative<Presentation>(result));
  return std::get<Presentation>(std::move(result));
}

void BorrowedIdentityAndSelection() {
  Fixture fixture;
  for (auto orientation : {Orientation::kVertical, Orientation::kHorizontal}) {
    for (auto density : {Density::kCompact, Density::kComfortable}) {
      for (auto side : {sb::Side::kLeft, sb::Side::kRight}) {
        const auto view = Present(fixture.tabs(), {orientation, density}, side);
        REQUIRE(view.requested().orientation == orientation);
        REQUIRE(view.requested().density == density);
        REQUIRE(view.effective_orientation() == orientation);
        REQUIRE(!view.used_fallback());
        REQUIRE(view.sidebar_side() == side);
        REQUIRE(&view.tabs().order.get() == &fixture.order);
        REQUIRE(&view.tabs().selected.get() == &fixture.selected);
        REQUIRE(&view.tabs().attachment.get() == &kAttachment);
        REQUIRE(&view.tabs().profile.get() == &kProfile);
        for (std::size_t index = 0; index < fixture.order.size(); ++index) {
          const auto row = view.RowAt(index);
          REQUIRE(row.has_value());
          REQUIRE(&row->activation_target.get() == &fixture.order[index]);
          REQUIRE(row->selected == (index == 2));
          REQUIRE(row->position_in_set == index + 1);
          REQUIRE(row->set_size == 4);
        }
        REQUIRE(!view.RowAt(fixture.order.size()));
        REQUIRE(!view.RowAt(std::numeric_limits<std::size_t>::max()));
      }
    }
  }
  REQUIRE(Describe(Orientation::kVertical) == "vertical");
  REQUIRE(Describe(Orientation::kHorizontal) == "horizontal");
  REQUIRE(Describe(Density::kCompact) == "compact");
  REQUIRE(Describe(Density::kComfortable) == "comfortable");
}

void CallerReprojectsAfterChanges() {
  Fixture fixture;
  // The caller, not the component, changes authoritative order and selection.
  std::swap(fixture.order.front(), fixture.order.back());
  fixture.selected = TabIdentity{kTemporary};
  {
    const auto view =
        Present(fixture.tabs(), {Orientation::kHorizontal, Density::kCompact});
    REQUIRE(view.RowAt(3)->selected);
    REQUIRE(std::get<lc::PageId>(view.RowAt(3)->activation_target.get()) ==
            kTemporary);
    REQUIRE(std::get<lc::ItemId>(view.RowAt(0)->activation_target.get()) ==
            kClosed);
  }
  fixture.selected.reset();
  {
    const auto view = Present(fixture.tabs(),
                              {Orientation::kVertical, Density::kComfortable});
    for (std::size_t index = 0; index < fixture.order.size(); ++index)
      REQUIRE(!view.RowAt(index)->selected);
  }
  fixture.order.clear();
  const auto empty =
      Present(fixture.tabs(), {Orientation::kHorizontal, Density::kCompact});
  REQUIRE(empty.tabs().order.get().empty());
  REQUIRE(!empty.tabs().selected.get());
  REQUIRE(!empty.RowAt(0));
}

void MeasuredFallback() {
  Fixture fixture;
  const Preferences preference{Orientation::kVertical, Density::kComfortable};
  FitMetrics narrow = kRoom;
  narrow.available = {500, 400};
  const auto fallback =
      Present(fixture.tabs(), preference, sb::Side::kRight, narrow);
  REQUIRE(fallback.used_fallback());
  REQUIRE(fallback.effective_orientation() == Orientation::kHorizontal);
  REQUIRE(fallback.requested().orientation == Orientation::kVertical);
  REQUIRE(fallback.requested().density == Density::kComfortable);
  REQUIRE(fallback.sidebar_side() == sb::Side::kRight);
  REQUIRE(fallback.RowAt(2)->selected);
  REQUIRE(fallback.sidebar_override(sb::PresentationOverride::kRevealOverlay) ==
          sb::PresentationOverride::kSuppress);

  // Exact caller-measured bounds fit, including height. No hidden breakpoint.
  FitMetrics exact = kRoom;
  exact.available = exact.vertical_required;
  REQUIRE(!Present(fixture.tabs(), preference, sb::Side::kLeft, exact)
               .used_fallback());
  FitMetrics short_window = kRoom;
  short_window.available = {800, 320};
  const auto reverse =
      Present(fixture.tabs(), {Orientation::kHorizontal, Density::kCompact},
              sb::Side::kLeft, short_window);
  REQUIRE(reverse.effective_orientation() == Orientation::kVertical);
  REQUIRE(reverse.used_fallback());
  REQUIRE(reverse.sidebar_override(sb::PresentationOverride::kRevealOverlay) ==
          sb::PresentationOverride::kRevealOverlay);

  // Scaling all logical measurements consistently preserves the same decision.
  auto scale = [](Extent extent, double factor) {
    return Extent{extent.width * factor, extent.height * factor};
  };
  for (double factor : {1.25, 2.0}) {
    FitMetrics scaled{scale(narrow.available, factor),
                      scale(narrow.vertical_required, factor),
                      scale(narrow.horizontal_required, factor)};
    REQUIRE(Present(fixture.tabs(), preference, sb::Side::kRight, scaled)
                .effective_orientation() == Orientation::kHorizontal);
  }

  FitMetrics insufficient = kRoom;
  insufficient.available = {399, 299};
  REQUIRE(std::get<ProjectionError>(Project(fixture.tabs(), preference,
                                            sb::Side::kLeft, insufficient)) ==
          ProjectionError::kNoUsableLayout);
  // A later fit uses the original request. Fallback never writes preferences.
  REQUIRE(!Present(fixture.tabs(), preference).used_fallback());
}

void InvalidMeasurements() {
  Fixture fixture;
  for (double invalid : {-1.0, std::numeric_limits<double>::infinity(),
                         std::numeric_limits<double>::quiet_NaN()}) {
    // Validate every available/required width and height at the public
    // boundary.
    for (std::size_t index = 0; index < 6; ++index) {
      auto metrics = kRoom;
      double* coordinates[]{&metrics.available.width,
                            &metrics.available.height,
                            &metrics.vertical_required.width,
                            &metrics.vertical_required.height,
                            &metrics.horizontal_required.width,
                            &metrics.horizontal_required.height};
      *coordinates[index] = invalid;
      REQUIRE(std::get<ProjectionError>(Project(
                  fixture.tabs(), {Orientation::kVertical, Density::kCompact},
                  sb::Side::kLeft, metrics)) ==
              ProjectionError::kInvalidMetrics);
    }
  }
}

void SidebarAndFocusComposition() {
  Fixture fixture;
  for (auto side : {sb::Side::kLeft, sb::Side::kRight}) {
    for (auto orientation :
         {Orientation::kVertical, Orientation::kHorizontal}) {
      const auto view =
          Present(fixture.tabs(), {orientation, Density::kComfortable}, side);
      for (bool durable_visible : {false, true}) {
        sb::Context context{{10, 20, 1200, 800},    side, 180, 400, 500, 8, 6,
                            view.sidebar_override()};
        auto sidebar =
            sb::Presentation::Create({durable_visible, 240}, context);
        REQUIRE(sidebar.has_value());
        focus_mode::Presentation focus;
        focus_mode::HostFacts facts;
        auto apply = [&](sb::PresentationOverride chrome) {
          context.presentation_override = view.sidebar_override(chrome);
          REQUIRE(
              sidebar->Configure(context, sb::FocusLocation::kOutsideSidebar)
                  .status == sb::Status::kOk);
          REQUIRE(sidebar->preferences().persistent_visible == durable_visible);
          REQUIRE(sidebar->preferences().preferred_width == 240);
          return sidebar->layout();
        };
        const auto normal = apply(focus.Project(facts).chrome);
        if (orientation == Orientation::kHorizontal)
          REQUIRE(normal.mode == sb::Mode::kHidden);
        else
          REQUIRE(normal.mode ==
                  (durable_visible ? sb::Mode::kDocked : sb::Mode::kHidden));

        const auto suppressed =
            apply(focus.Dispatch(focus_mode::Action::kEnter, facts)
                      .projection.chrome);
        REQUIRE(suppressed.mode == sb::Mode::kHidden);
        REQUIRE(suppressed.content == context.available);
        const auto revealed =
            apply(focus.Dispatch(focus_mode::Action::kRevealKeyboard, facts)
                      .projection.chrome);
        REQUIRE(revealed.content == suppressed.content);
        if (orientation == Orientation::kVertical) {
          REQUIRE(revealed.mode == sb::Mode::kOverlay);
          REQUIRE(revealed.sidebar.has_value());
        } else {
          REQUIRE(revealed.mode == sb::Mode::kHidden);
          REQUIRE(!revealed.sidebar);
          REQUIRE(!revealed.reveal_target);
        }
        const auto exited = apply(
            focus.Dispatch(focus_mode::Action::kExit, facts).projection.chrome);
        REQUIRE(exited.mode == normal.mode);
        REQUIRE(exited.content == normal.content);
      }
    }
  }
}

void LifecycleV3StaysAuthoritative() {
  static_assert(lc::kContractVersion == 3);
  lc::Model model;
  REQUIRE(model.AddProfile(kProfile) == lc::Status::kOk);
  REQUIRE(model.AddSpace(kAttachment.space, kProfile) == lc::Status::kOk);
  REQUIRE(model.AddWindow(kAttachment.window) == lc::Status::kOk);
  REQUIRE(model.Attach(kAttachment) == lc::Status::kOk);
  REQUIRE(model.SelectSpace(kAttachment.window, kAttachment.space) ==
          lc::Status::kOk);
  const std::string saved = "https://example.test/saved";
  const std::string current = "https://example.test/draft";
  REQUIRE(model.AddItem(kSaved, kAttachment.space, saved) == lc::Status::kOk);
  REQUIRE(model.AddItem(kClosed, kAttachment.space, saved) == lc::Status::kOk);
  REQUIRE(model.AddShortcut(kAlias, kProfile, kSaved) == lc::Status::kOk);
  REQUIRE(std::holds_alternative<lc::OpenedPage>(
      model.OpenItem(kSaved, kAttachment, kLive)));
  REQUIRE(model.RecordNavigation(kLive, current) == lc::Status::kOk);
  REQUIRE(model.ReportActivity(kLive, lc::Activity::kProtected) ==
          lc::Status::kOk);
  REQUIRE(model.OpenTemporary(kTemporary, lc::TemporaryTabId{"temp-tab"},
                              kAttachment, "https://example.test/temporary") ==
          lc::Status::kOk);
  Fixture fixture;
  const auto* original_page = &model.pages().at(kLive);
  for (auto orientation : {Orientation::kHorizontal, Orientation::kVertical}) {
    for (auto density : {Density::kCompact, Density::kComfortable}) {
      for (auto side : {sb::Side::kRight, sb::Side::kLeft}) {
        const auto view = Present(fixture.tabs(), {orientation, density}, side);
        REQUIRE(std::get<lc::ItemId>(view.RowAt(1)->activation_target.get()) ==
                kSaved);
        REQUIRE(std::get<lc::ShortcutId>(
                    view.RowAt(2)->activation_target.get()) == kAlias);
        REQUIRE(view.RowAt(2)->selected);
        // A synthetic caller adapter dispatches the projected alias identity to
        // the existing owner. Projection neither opens nor resolves the page.
        const auto opened = model.OpenShortcut(
            std::get<lc::ShortcutId>(view.RowAt(2)->activation_target.get()),
            view.tabs().attachment.get(), lc::PageId{"unused-candidate"});
        REQUIRE(std::holds_alternative<lc::OpenedPage>(opened));
        REQUIRE(std::get<lc::OpenedPage>(opened).page == kLive);
        REQUIRE(std::get<lc::OpenedPage>(opened).disposition ==
                lc::OpenDisposition::kExisting);
      }
    }
  }
  REQUIRE(&model.pages().at(kLive) == original_page);
  REQUIRE(original_page->profile == kProfile);
  REQUIRE(original_page->current_navigation == current);
  REQUIRE(original_page->activity == lc::Activity::kProtected);
  REQUIRE(original_page->attachment() == kAttachment);
  REQUIRE(std::get<lc::SavedPage>(original_page->owner).item == kSaved);
  REQUIRE(model.items().at(kSaved).saved_destination == saved);
  REQUIRE(model.items().at(kClosed).saved_destination == saved);
  REQUIRE(model.shortcuts().at(kAlias).target == kSaved);
  REQUIRE(model.pages().size() == 2);
  REQUIRE(model.items().size() == 2);
  REQUIRE(model.TemporaryPages(kAttachment) ==
          std::vector<lc::PageId>{kTemporary});
  REQUIRE(model.windows().at(kAttachment.window).selected_space ==
          kAttachment.space);
}

}  // namespace
}  // namespace arcified::tab_presentation

int main() {
  using namespace arcified::tab_presentation;
  BorrowedIdentityAndSelection();
  CallerReprojectsAfterChanges();
  MeasuredFallback();
  InvalidMeasurements();
  SidebarAndFocusComposition();
  LifecycleV3StaysAuthoritative();
  std::cout << "tab presentation fixtures passed (standalone semantics only)\n";
}
