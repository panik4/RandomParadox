#pragma once
#include "FastWorldGenerator.h"
#include "generic/GameRegion.h"
#include "generic/ScenarioUtils.h"
#include "vic3/Vic3Utils.h"
namespace Scenario::Vic3 {

class Region : public Scenario::Region {

public:
  Region();
  Region(const Scenario::Region &gameRegion);
  ~Region();
  std::map<std::string, Resource> resources;
  double arableLand = 0.0;
  std::map<std::string, Building> buildings;
  int supportsBuilding(const BuildingType &buildingType);
  int navalExit = -1;
};
} // namespace Scenario::Vic3