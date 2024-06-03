#pragma once
#include "FastWorldGenerator.h"
#include "GameRegion.h"
#include <map>
namespace Scenario {
class SuperRegion {

public:
  SuperRegion();

  // member variables
  int ID;
  std::string name;
  Fwg::Gfx::Colour colour;
  // containers
  std::vector<std::shared_ptr<Region>> gameRegions;


  void addRegion(std::shared_ptr<Region> region);
  void removeRegion(std::shared_ptr<Region> region);
};
} // namespace Scenario