#pragma once
#include "FastWorldGenerator.h"
#include "areas/GameRegion.h"
#include "utils/ScenarioUtils.h"
#include "hoi4/Hoi4Airforce.h"
#include "generic/VictoryPoint.h"

namespace Scenario::Hoi4 {


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
  // the identifier the game uses for this state, referred to by localisation,
  // history, foci, etc
  std::string identifier;
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
  std::map<int, std::shared_ptr<VictoryPoint>> victoryPointsMap;
  std::map<int, int> navalBases;
  std::shared_ptr<AirBase> airBase;
  // Constructors/Destructors
  Region();
  Region(const Scenario::Region &gameRegion);
  ~Region();

  // member functions
  void calculateBuildingPositions(const std::vector<float> &heightmap,
                                  const Fwg::Gfx::Bitmap &typeMap);
};
} // namespace Scenario::Hoi4