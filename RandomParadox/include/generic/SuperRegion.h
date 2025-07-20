#pragma once
#include "FastWorldGenerator.h"
#include "GameRegion.h"
#include <map>
namespace Scenario {
struct Cluster : Fwg::Areas::Area {
  std::vector<std::shared_ptr<Region>> regions;
};
class SuperRegion : public Fwg::Areas::Area {

public:
  SuperRegion();

  // member variables
  int ID;
  std::string name;
  Fwg::Gfx::Colour colour;
  // containers
  std::vector<std::shared_ptr<Region>> gameRegions;
  std::vector<Cluster> gameRegionClusters;

  bool centerOutsidePixels = false;

  void addRegion(std::shared_ptr<Region> region);
  void removeRegion(std::shared_ptr<Region> region);
  void setType();
  void checkPosition(const std::vector<SuperRegion> &superRegions);
  std::vector<Cluster>
  getClusters(std::vector<std::shared_ptr<Region>> &regions);
};
} // namespace Scenario