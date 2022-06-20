#pragma once
#include "FastWorldGenerator.h"
#include "generic/GameRegion.h"

namespace Scenario::Hoi4 {
class Region : public Scenario::Region {
public:
  // member variables
  int civilianFactories;
  int dockyards;
  int armsFactories;
  int population;
  int stateCategory;
  std::map<std::string, int> resources;
  double development;
  int stratID;
  // Constructors/Destructors
  Region();
  Region(const Scenario::Region &gameRegion);
  ~Region();
};
} // namespace Scenario::Hoi4