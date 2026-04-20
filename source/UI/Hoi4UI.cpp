#include "UI/Hoi4UI.h"
namespace Rpx::UI::Hoi4 {
int showHoi4Configure(Fwg::Cfg &cfg, std::shared_ptr<Hoi4Gen> generator) {
  ImGui::InputFloat("Starting Army Size Factor",
                    &generator->modConfig.startingArmyStrengthFactor, 0.1);
  ImGui::InputFloat("Starting Navy Size Factor",
                    &generator->modConfig.startingNavyStrengthFactor, 0.1);
  ImGui::InputFloat("Starting Airforce Size Factor",
                    &generator->modConfig.startingAirforceStrengthFactor, 0.1);
  ImGui::InputDouble("resourceFactor", &generator->ardaConfig.resourceFactor,
                     0.1);
  ImGui::InputDouble("aluminiumFactor",
                     &generator->modConfig.resources["aluminium"][0], 0.1);
  ImGui::InputDouble("coalFactor", &generator->modConfig.resources["coal"][0],
                     0.1);
  ImGui::InputDouble("chromiumFactor",
                     &generator->modConfig.resources["chromium"][0], 0.1);
  ImGui::InputDouble("oilFactor", &generator->modConfig.resources["oil"][0],
                     0.1);
  ImGui::InputDouble("rubberFactor",
                     &generator->modConfig.resources["rubber"][0], 0.1);
  ImGui::InputDouble("steelFactor", &generator->modConfig.resources["steel"][0],
                     0.1);
  ImGui::InputDouble("tungstenFactor",
                     &generator->modConfig.resources["tungsten"][0], 0.1);
  ImGui::InputDouble(
      "baseLightRainChance",
      &generator->modConfig.weatherChances["baseLightRainChance"], 0.1);
  ImGui::InputDouble(
      "baseHeavyRainChance",
      &generator->modConfig.weatherChances["baseHeavyRainChance"], 0.1);
  ImGui::InputDouble("baseMudChance",
                     &generator->modConfig.weatherChances["baseMudChance"],
                     0.1);
  ImGui::InputDouble("baseBlizzardChance",
                     &generator->modConfig.weatherChances["baseBlizzardChance"],
                     0.1);
  ImGui::InputDouble(
      "baseSandstormChance",
      &generator->modConfig.weatherChances["baseSandstormChance"], 0.1);
  ImGui::InputDouble("baseSnowChance",
                     &generator->modConfig.weatherChances["baseSnowChance"],
                     0.1);
  return 0;
}

} // namespace Rpx::UI::Hoi4