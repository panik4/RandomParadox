#pragma once
#include <generic/ScenarioGenerator.h>

namespace Scenario::Vic3 {
// eu4 regions consist of multiple areas, which are collections of provinces
struct Vic3Region {
  std::set<int> areaIDs;
  std::string name;
};

class Generator : public Scenario::Generator {
  std::vector<Vic3Region> vic3regions;

public:
  Generator();
  Generator(Fwg::FastWorldGenerator &fwg);
  void generateRegions(std::vector<std::shared_ptr<Region>> &regions);
  std::vector<Vic3Region> getVic3Regions() { return vic3regions; };
};
} // namespace Scenario::Vic3