#pragma once
#include <generic/ScenarioGenerator.h>

// eu4 regions consist of multiple areas, which are collections of provinces
struct eu4Region {
  std::set<int> areaIDs;
  std::string name;
};

class Eu4ScenarioGenerator {

  std::vector<eu4Region> eu4regions;

public:
  void generateRegions(std::vector<GameRegion> &regions);
  std::vector<eu4Region> getEu4Regions() { return eu4regions; };
};
