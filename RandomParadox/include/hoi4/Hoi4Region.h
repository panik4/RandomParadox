#pragma once
#include "FastWorldGenerator.h"
#include "generic/GameRegion.h"

namespace Scenario::Hoi4 {
class Region : public Scenario::Region {
public:
  // member variables
  int armsFactories;
  int civilianFactories;
  double development;
  int dockyards;
  int population;
  std::map<std::string, int> resources;
  int stateCategory;
  int stratID;
  // Constructors/Destructors
  Region();
  Region(const Scenario::Region &gameRegion);
  ~Region();
};
} // namespace Scenario::Hoi4