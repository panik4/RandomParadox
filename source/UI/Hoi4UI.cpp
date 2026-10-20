#include "UI/Hoi4UI.h"
#include "misc/cpp/imgui_stdlib.h"
#include <array>
#include <cstdio>
#include <map>

namespace Rpx::UI::Hoi4 {
namespace {

template <typename Type>
void showCountSection(const char *title,
                      const std::map<Type, int> &counts,
                      const std::map<Type, std::string> &labels) {
  if (!ImGui::TreeNode(title))
    return;
  for (const auto &[type, count] : counts) {
    const auto label = labels.find(type);
    if (label != labels.end() && count > 0)
      ImGui::Text("%s: %d", label->second.c_str(), count);
  }
  ImGui::TreePop();
}

const std::map<Rpx::Hoi4::CombatRegimentType, std::string> combatRegimentLabels{
    {Rpx::Hoi4::CombatRegimentType::Irregulars, "Irregulars"},
    {Rpx::Hoi4::CombatRegimentType::Infantry, "Infantry"},
    {Rpx::Hoi4::CombatRegimentType::Cavalry, "Cavalry"},
    {Rpx::Hoi4::CombatRegimentType::Motorized, "Motorized"},
    {Rpx::Hoi4::CombatRegimentType::LightArmor, "Light armor"},
    {Rpx::Hoi4::CombatRegimentType::AntiTank, "Anti-tank"},
    {Rpx::Hoi4::CombatRegimentType::AntiAir, "Anti-air"},
    {Rpx::Hoi4::CombatRegimentType::Artillery, "Artillery"},
    {Rpx::Hoi4::CombatRegimentType::MotorizedAntiTank,
     "Motorized anti-tank"},
    {Rpx::Hoi4::CombatRegimentType::MotorizedAntiAir,
     "Motorized anti-air"},
    {Rpx::Hoi4::CombatRegimentType::MotorizedArtillery,
     "Motorized artillery"},
    {Rpx::Hoi4::CombatRegimentType::Paratroopers, "Paratroopers"},
    {Rpx::Hoi4::CombatRegimentType::Marines, "Marines"},
    {Rpx::Hoi4::CombatRegimentType::Mountaineers, "Mountaineers"}};

const std::map<Rpx::Hoi4::SupportRegimentType, std::string>
    supportRegimentLabels{
        {Rpx::Hoi4::SupportRegimentType::Engineer, "Engineers"},
        {Rpx::Hoi4::SupportRegimentType::Recon, "Recon"},
        {Rpx::Hoi4::SupportRegimentType::FieldHospital, "Field hospitals"},
        {Rpx::Hoi4::SupportRegimentType::Logistics, "Logistics"},
        {Rpx::Hoi4::SupportRegimentType::Maintenance, "Maintenance"},
        {Rpx::Hoi4::SupportRegimentType::AntiTank, "Anti-tank"},
        {Rpx::Hoi4::SupportRegimentType::AntiAir, "Anti-air"},
        {Rpx::Hoi4::SupportRegimentType::Artillery, "Artillery"}};

const std::map<Rpx::Hoi4::PlaneRole, std::string> planeRoleLabels{
    {Rpx::Hoi4::PlaneRole::Fighter, "Fighters"},
    {Rpx::Hoi4::PlaneRole::Cas, "Close air support"},
    {Rpx::Hoi4::PlaneRole::NavalBomber, "Naval bombers"},
    {Rpx::Hoi4::PlaneRole::TacticalBomber, "Tactical bombers"},
    {Rpx::Hoi4::PlaneRole::StrategicBomber, "Strategic bombers"},
    {Rpx::Hoi4::PlaneRole::CarrierCas, "Carrier close air support"},
    {Rpx::Hoi4::PlaneRole::CarrierFighter, "Carrier fighters"},
    {Rpx::Hoi4::PlaneRole::CarrierNavalBomber, "Carrier naval bombers"}};

std::map<Rpx::Hoi4::CombatRegimentType, int>
countCombatRegiments(const Rpx::Hoi4::Hoi4Country &country) {
  std::map<Rpx::Hoi4::CombatRegimentType, int> counts;
  for (const auto &division : country.divisions) {
    for (const auto &column : division.divisionTemplate.regiments) {
      for (const auto regiment : column)
        ++counts[regiment];
    }
  }
  return counts;
}

std::map<Rpx::Hoi4::SupportRegimentType, int>
countSupportRegiments(const Rpx::Hoi4::Hoi4Country &country) {
  std::map<Rpx::Hoi4::SupportRegimentType, int> counts;
  for (const auto &division : country.divisions) {
    for (const auto regiment : division.divisionTemplate.supportRegiments)
      ++counts[regiment];
  }
  return counts;
}

std::map<Rpx::Hoi4::PlaneRole, int>
countAirWings(const Rpx::Hoi4::Hoi4Country &country) {
  std::map<Rpx::Hoi4::PlaneRole, int> counts;
  for (const auto &wing : country.airWings)
    counts[wing.role] += wing.amount;
  return counts;
}

std::map<Rpx::Hoi4::ShipClassType, int>
countShips(const Rpx::Hoi4::Hoi4Country &country) {
  std::map<Rpx::Hoi4::ShipClassType, int> counts;
  for (const auto &ship : country.ships) {
    if (ship)
      ++counts[ship->shipClass.type];
  }
  return counts;
}

void showForceSummary(const Rpx::Hoi4::Hoi4Country &country) {
  ImGui::SeparatorText("Generated Forces");
  ImGui::Text("Divisions: %zu", country.divisions.size());
  ImGui::Text("Army brigades");
  showCountSection("Combat brigades", countCombatRegiments(country),
                   combatRegimentLabels);
  showCountSection("Support brigades", countSupportRegiments(country),
                   supportRegimentLabels);
  showCountSection("Air force by type", countAirWings(country),
                   planeRoleLabels);
  showCountSection("Navy by class", countShips(country),
                   Rpx::Hoi4::shipClassTypeMap);
}

} // namespace

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

bool MapEditorModule::showCountryEditor(
    const std::shared_ptr<Arda::Country> &country,
    Fwg::UI::Elements::GridLayout &grid) const {
  const auto hoi4Country =
      std::dynamic_pointer_cast<Rpx::Hoi4::Hoi4Country>(country);
  if (!hoi4Country) {
    return false;
  }

  grid.AddInputText("Full Name", &hoi4Country->fullName);
  grid.AddInputText("GFX Culture", &hoi4Country->gfxCulture);
  int ideology = static_cast<int>(hoi4Country->ideology);
  const char *ideologyNames[] = {"None", "Fascism", "Democratic", "Communism",
                                 "Neutrality"};
  if (grid.AddCombo("Ruling Party", &ideology, ideologyNames,
                    IM_ARRAYSIZE(ideologyNames))) {
    hoi4Country->ideology = static_cast<Arda::Utils::Ideology>(ideology);
    true;
  }

  const char *partyNames[] = {"Fascism Popularity", "Democratic Popularity",
                              "Communist Popularity", "Neutrality Popularity"};
  for (std::size_t party = 0; party < hoi4Country->parties.size(); ++party) {
    grid.AddInputInt(partyNames[party], &hoi4Country->parties[party], 0, 100);
  }
  grid.AddCheckbox("Allow Elections", &hoi4Country->allowElections);
  showForceSummary(*hoi4Country);
  return false;
}

void MapEditorModule::showCountryAggregates(
    const std::shared_ptr<Arda::Country> &country,
    Fwg::UI::Elements::GridLayout &grid) const {
  const auto hoi4Country =
      std::dynamic_pointer_cast<Rpx::Hoi4::Hoi4Country>(country);
  if (!hoi4Country) {
    return;
  }

  int armsFactories = 0;
  int civilianFactories = 0;
  int dockyards = 0;
  double population = 0.0;
  std::map<std::string, double> resources;
  for (const auto &state : hoi4Country->hoi4Regions) {
    if (!state) {
      continue;
    }
    armsFactories += state->armsFactories;
    civilianFactories += state->civilianFactories;
    dockyards += state->dockyards;
    population += state->totalPopulation;
    for (const auto &[name, resource] : state->resources) {
      resources[name] += resource.amount;
    }
  }

  grid.AddText("States", "%zu", hoi4Country->hoi4Regions.size());
  grid.AddText("Population", "%.0f", population);
  grid.AddText("Arms Factories", "%d", armsFactories);
  grid.AddText("Civilian Factories", "%d", civilianFactories);
  grid.AddText("Dockyards", "%d", dockyards);
  grid.AddText("Research Slots", "%d", hoi4Country->researchSlots);

  if (!resources.empty()) {
    for (const auto &[name, amount] : resources) {
      grid.AddText(name.c_str(), "%.0f", amount);
    }
  }
}

void MapEditorModule::showContinentAggregates(
    const std::shared_ptr<Arda::ArdaContinent> &continent,
    Fwg::UI::Elements::GridLayout &grid) const {
  if (!continent) {
    return;
  }

  int armsFactories = 0;
  int civilianFactories = 0;
  int dockyards = 0;
  for (const auto &state : continent->ardaRegions) {
    const auto hoi4State = std::dynamic_pointer_cast<Rpx::Hoi4::Region>(state);
    if (!hoi4State) {
      continue;
    }
    armsFactories += hoi4State->armsFactories;
    civilianFactories += hoi4State->civilianFactories;
    dockyards += hoi4State->dockyards;
  }

  grid.AddText("HOI4 Continent Aggregates", "%s", "");
  grid.AddText("Arms Factories", "%d", armsFactories);
  grid.AddText("Civilian Factories", "%d", civilianFactories);
  grid.AddText("Dockyards", "%d", dockyards);
}

bool MapEditorModule::showStateEditor(
    const std::shared_ptr<Arda::ArdaRegion> &state,
    Fwg::UI::Elements::GridLayout &grid) const {
  const auto hoi4State = std::dynamic_pointer_cast<Rpx::Hoi4::Region>(state);
  if (!hoi4State) {
    return false;
  }
  ImGui::SeparatorText("State Details");
  bool changed = grid.AddInputInt("Arms Factories", &hoi4State->armsFactories);
  changed |=
      grid.AddInputInt("Civilian Factories", &hoi4State->civilianFactories);
  changed |= grid.AddInputInt("Dockyards", &hoi4State->dockyards);
  changed |= grid.AddInputInt("Infrastructure", &hoi4State->infrastructure);
  changed |= grid.AddInputInt("State Category", &hoi4State->stateCategory);
  if (!hoi4State->resources.empty()) {
    ImGui::SeparatorText("State Resources");
    for (auto &[name, resource] : hoi4State->resources) {
      changed |= grid.AddInputDouble(name.c_str(), &resource.amount, 0.0,
                                     1000000000.0, 1, 10.0);
    }
  }
  return changed;
}

} // namespace Rpx::UI::Hoi4