#pragma once
#include "ArdaUI/ArdaUI.h"
#include "FastWorldGenerator.h"
#include "MapEditorModule.h"
#include "hoi4/Hoi4Generator.h"

using Hoi4Gen = Rpx::Hoi4::Generator;
namespace Rpx::UI::Hoi4 {
int showHoi4Configure(Fwg::Cfg &cfg, std::shared_ptr<Hoi4Gen> generator);

class MapEditorModule final : public Rpx::UI::MapEditorModule {
public:
  bool showCountryEditor(
      const std::shared_ptr<Arda::Country> &country,
      Fwg::UI::Elements::GridLayout &grid) const override;
  bool showStateEditor(
      const std::shared_ptr<Arda::ArdaRegion> &state,
      Fwg::UI::Elements::GridLayout &grid) const override;
  void showCountryAggregates(
      const std::shared_ptr<Arda::Country> &country,
      Fwg::UI::Elements::GridLayout &grid) const override;
  void showContinentAggregates(
      const std::shared_ptr<Arda::ArdaContinent> &continent,
      Fwg::UI::Elements::GridLayout &grid) const override;
};
}; // namespace Rpx::UI::Hoi4