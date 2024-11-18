#pragma once
#include "FastWorldGenerator.h"
#include "generic/GameRegion.h"
#include "generic/ScenarioUtils.h"

namespace Scenario::Hoi4 {
struct VictoryPoint {
  int amount;
  Fwg::Position position;
  std::string name;
};

class Region : public Scenario::Region {
  const std::vector<std::string> buildingTypes{
      "arms_factory",       "industrial_complex",
      "air_base",           "bunker",
      "coastal_bunker",     "dockyard",
      "naval_base_spawn",   "anti_air_building",
      "synthetic_refinery", "nuclear_reactor_spawn",
      "rocket_site_spawn",  "radar_station",
      "fuel_silo",          "floating_harbor",
      "stronghold_network", "special_project_facility_spawn"};

public:
  // member variables
  int armsFactories;
  int civilianFactories;
  int dockyards;

  int infrastructure;
  int stateCategory;
  int stratID;
  int airport, rocketsite, supplyNode;
  double totalVictoryPoints;
  Scenario::Utils::WeatherPosition weatherPosition;
  // Containers
  std::vector<Scenario::Utils::Building> buildings;
  std::map<std::string, int> resources;
  std::map<int, VictoryPoint> victoryPointsMap;
  std::map<int, int> navalBases;
  // Constructors/Destructors
  Region();
  Region(const Scenario::Region &gameRegion);
  ~Region();

  // member functions
  void calculateBuildingPositions(const Fwg::Gfx::Bitmap &heightmap,
                                  const Fwg::Gfx::Bitmap &typeMap);
};
} // namespace Scenario::Hoi4