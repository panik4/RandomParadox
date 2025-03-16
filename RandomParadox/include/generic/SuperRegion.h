#pragma once
#include "FastWorldGenerator.h"
#include "GameRegion.h"
#include <map>
namespace Scenario {
class SuperRegion {
  Fwg::Areas::AreaType type;

public:
  SuperRegion();

  // member variables
  int ID;
  std::string name;
  Fwg::Gfx::Colour colour;
  // containers
  std::vector<std::shared_ptr<Region>> gameRegions;
  Fwg::Position position;
  bool centerOutsidePixels = false;

  void addRegion(std::shared_ptr<Region> region);
  void removeRegion(std::shared_ptr<Region> region);
  void setType();
  void checkPosition(const std::vector<SuperRegion> &superRegions);
};
} // namespace Scenario