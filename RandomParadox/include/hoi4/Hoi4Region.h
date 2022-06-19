#pragma once
#include "FastWorldGenerator.h"
#include "generic/GameRegion.h"

namespace Scenario::Hoi4 {
class Hoi4Region : public GameRegion {
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
  Hoi4Region();
  Hoi4Region(const GameRegion &gameRegion);
  ~Hoi4Region();
};
} // namespace Scenario::Hoi4