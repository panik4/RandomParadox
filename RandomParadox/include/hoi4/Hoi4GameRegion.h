#pragma once
#include "FastWorldGenerator.h"
#include "generic/GameRegion.h"

class Hoi4GameRegion : public GameRegion {
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
  Hoi4GameRegion();
  Hoi4GameRegion(const GameRegion &gameRegion);
  ~Hoi4GameRegion();
};
