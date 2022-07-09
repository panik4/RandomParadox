#pragma once
#include <generic/ScenarioGenerator.h>

namespace Scenario::Eu4 {
// eu4 regions consist of multiple areas, which are collections of provinces
struct eu4Region {
  std::set<int> areaIDs;
  std::string name;
};

class Generator : public Scenario::Generator {
  std::vector<eu4Region> eu4regions;

public:
  Generator();
  Generator(Fwg::FastWorldGenerator &fwg);
  void generateRegions(std::vector<Region> &regions);
  std::vector<eu4Region> getEu4Regions() { return eu4regions; };
};
} // namespace Scenario::Eu4