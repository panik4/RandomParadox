#pragma once
#include "FastWorldGenerator.h"
#include "generic/GameRegion.h"
#include "generic/ScenarioUtils.h"

namespace Scenario::Vic3 {
enum class ResourceType : int {
  COAL,
  FISH,
  GOLDFIELDS,
  GOLDMINES,
  IRON,
  LEAD,
  LOGGING,
  OIL,
  RUBBER,
  SULFUR,
  WHALING,
  LIVESTOCK,
  BANANA,
  COFFEE,
  COTTON,
  DYE,
  MAIZE,
  MILLET,
  OPIUM,
  RICE,
  RYE,
  SILK,
  SUGAR,
  TEA,
  TOBACCO,
  VINYARDS,
  WHEAT
};
class Region : public Scenario::Region {



public:
  Region();
  Region(const Scenario::Region &gameRegion);
  ~Region();
  double development;
  std::map<ResourceType, double> resources;
  double arableLand = 100.0;
  
};
} // namespace Scenario::Vic3