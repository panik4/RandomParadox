#pragma once
#include "FastWorldGenerator.h"
#include "generic/GameRegion.h"
#include "generic/ScenarioUtils.h"

namespace Scenario::Hoi4 {
class Region : public Scenario::Region {
  const std::vector<std::string> buildingTypes{
      "arms_factory",    "industrial_complex", "air_base",
      "bunker",          "coastal_bunker",     "dockyard",
      "naval_base",      "anti_air_building",  "synthetic_refinery",
      "nuclear_reactor", "rocket_site",        "radar_station",
      "fuel_silo",       "floating_harbor"};

public:
  // member variables
  int armsFactories;
  int civilianFactories;
  double development;
  int dockyards;
  int population;
  int stateCategory;
  int stratID;
  int airport, rocketsite;
  // Containers
  std::vector<Scenario::Utils::Building> buildings;
  std::map<std::string, int> resources;
  // Constructors/Destructors
  Region();
  Region(const Scenario::Region &gameRegion);
  ~Region();

  // member functions
  void calculateBuildingPositions(const Fwg::Gfx::Bitmap &heightmap);
};
} // namespace Scenario::Hoi4