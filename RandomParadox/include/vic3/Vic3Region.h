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
  std::map<ResourceType, double> resources;
  double arableLand = 100.0;
  
};
} // namespace Scenario::Vic3