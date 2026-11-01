#pragma once

#include <memory>

namespace Arda {
class ArdaRegion;
class Country;
class ArdaContinent;
} // namespace Arda

namespace Fwg::UI::Elements {
class GridLayout;
}

namespace Rpx::UI {

enum EditorChange : unsigned { NoChange = 0, EconomyChanged = 1, ResourcesChanged = 2, PoliticsChanged = 4, IdentityChanged = 8 };

class MapEditorModule {
public:
  virtual ~MapEditorModule() = default;

  // Optional editors return true only when they apply an edit.
  virtual unsigned
  showCountryEditor(const std::shared_ptr<Arda::Country> &country,
                    Fwg::UI::Elements::GridLayout &grid) const { return false; }
  virtual unsigned
  showStateEditor(const std::shared_ptr<Arda::ArdaRegion> &state,
                  Fwg::UI::Elements::GridLayout &grid) const { return false; }
  virtual void showCountryAggregates(
      const std::shared_ptr<Arda::Country> &country,
      Fwg::UI::Elements::GridLayout &grid) const = 0;
  virtual void showContinentAggregates(
      const std::shared_ptr<Arda::ArdaContinent> &continent,
      Fwg::UI::Elements::GridLayout &grid) const {}
};

} // namespace Rpx::UI
