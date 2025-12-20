#pragma once
#include "FastWorldGenerator.h"
#include "areas/ArdaRegion.h"
#include "generic/VictoryPoint.h"
#include "hoi4/Hoi4Airforce.h"
#include "utils/RpxUtils.h"

namespace Rpx::Hoi4 {

class Region : public Arda::ArdaRegion {
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
  std::string identifier = "";
  int armsFactories = 0;
  int civilianFactories = 0;
  int dockyards = 0;

  int infrastructure = 0;
  int stateCategory = 0;
  int stratID = 0;
  int airport = 0, rocketsite = 0, supplyNode = 0;
  double totalVictoryPoints = 0.0;
  Arda::Utils::WeatherPosition weatherPosition;
  // Containers
  std::vector<Arda::Utils::Building> buildings;
  std::map<int, std::shared_ptr<Arda::VictoryPoint>> victoryPointsMap;
  std::map<int, int> navalBases;
  std::shared_ptr<AirBase> airBase;
  // Constructors/Destructors
  Region();
  Region(const Arda::ArdaRegion &ardaRegion);
  ~Region();

  // member functions
  void calculateBuildingPositions(const std::vector<float> &heightmap,
                                  const Fwg::Gfx::Image &typeMap);
};
} // namespace Rpx::Hoi4