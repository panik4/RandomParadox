#pragma once
#include "FastWorldGenerator.h"
#include "GameProvince.h"
#include <map>
namespace Scenario {
class Region : public Fwg::Region {
  std::vector<std::string> cores;

public:
  // member variables
  std::string owner;
  std::string name;
  bool assigned;
  // containers
  std::vector<GameProvince> gameProvinces;
  std::vector<double> temperatureRange;
  std::vector<double> dateRange;
  double snowChance, lightRainChance, heavyRainChance, blizzardChance,
      mudChance, sandStormChance;
  Region();
  Region(const Fwg::Region &baseRegion);
  ~Region();
};
} // namespace Scenario