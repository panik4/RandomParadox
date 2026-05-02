#include "UI/Hoi4UI.h"

namespace Rpx::UI::Hoi4 {

int showHoi4Configure(Fwg::Cfg &cfg, std::shared_ptr<Hoi4Gen> generator) {

  ImGui::SeparatorText("Military Starting Forces");
  ImGui::TextWrapped(
      "Configure the initial strength of armed forces for all countries.");

  {
    Fwg::UI::Elements::GridLayout grid(3, 220.0f, 12.0f);

    grid.AddInputFloat("Starting Army Factor",
                       &generator->modConfig.startingArmyStrengthFactor, 0.0f,
                       10.0f);
    grid.AddInputFloat("Starting Navy Factor",
                       &generator->modConfig.startingNavyStrengthFactor, 0.0f,
                       10.0f);
    grid.AddInputFloat("Starting Airforce Factor",
                       &generator->modConfig.startingAirforceStrengthFactor,
                       0.0f, 10.0f);
  }

  ImGui::Spacing();
  ImGui::SeparatorText("Resource Configuration");
  ImGui::TextWrapped(
      "Adjust resource prevalence and distribution across the world.");

  {
    Fwg::UI::Elements::GridLayout grid(3, 180.0f, 12.0f);

    grid.AddInputDouble("Overall Factor", &generator->ardaConfig.resourceFactor,
                        0.0, 10.0);
    grid.NextRow();

    grid.AddInputDouble("Aluminium",
                        &generator->modConfig.resources["aluminium"][0], 0.0,
                        10.0);
    grid.AddInputDouble("Coal", &generator->modConfig.resources["coal"][0], 0.0,
                        10.0);
    grid.AddInputDouble(
        "Chromium", &generator->modConfig.resources["chromium"][0], 0.0, 10.0);
    grid.AddInputDouble("Oil", &generator->modConfig.resources["oil"][0], 0.0,
                        10.0);
    grid.AddInputDouble("Rubber", &generator->modConfig.resources["rubber"][0],
                        0.0, 10.0);
    grid.AddInputDouble("Steel", &generator->modConfig.resources["steel"][0],
                        0.0, 10.0);
    grid.AddInputDouble(
        "Tungsten", &generator->modConfig.resources["tungsten"][0], 0.0, 10.0);
  }

  ImGui::Spacing();
  ImGui::SeparatorText("Weather Configuration");
  ImGui::TextWrapped("Configure base chances for various weather conditions in "
                     "strategic regions.");

  {
    Fwg::UI::Elements::GridLayout grid(3, 180.0f, 12.0f);

    grid.AddInputDouble(
        "Light Rain",
        &generator->modConfig.weatherChances["baseLightRainChance"], 0.0, 1.0);
    grid.AddInputDouble(
        "Heavy Rain",
        &generator->modConfig.weatherChances["baseHeavyRainChance"], 0.0, 1.0);
    grid.AddInputDouble(
        "Mud", &generator->modConfig.weatherChances["baseMudChance"], 0.0, 1.0);
    grid.AddInputDouble(
        "Blizzard", &generator->modConfig.weatherChances["baseBlizzardChance"],
        0.0, 1.0);
    grid.AddInputDouble(
        "Sandstorm",
        &generator->modConfig.weatherChances["baseSandstormChance"], 0.0, 1.0);
    grid.AddInputDouble("Snow",
                        &generator->modConfig.weatherChances["baseSnowChance"],
                        0.0, 1.0);
  }

  ImGui::Spacing();

  return 0;
}

} // namespace Rpx::UI::Hoi4