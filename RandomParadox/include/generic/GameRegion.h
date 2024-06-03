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
  int superRegionID;
  // calculate this for the scenario in general
  double populationFactor;
  // calculate this in every module
  int totalPopulation;
  double development;
  bool assigned;
  double snowChance, lightRainChance, heavyRainChance, blizzardChance,
      mudChance, sandStormChance;
  // containers
  std::vector<std::shared_ptr<GameProvince>> gameProvinces;
  std::vector<double> temperatureRange;
  std::vector<double> dateRange;
  std::map<std::shared_ptr<Scenario::Religion>, double> religions;
  std::map<std::shared_ptr<Scenario::Culture>, double> cultures;

  Region();
  Region(const Fwg::Region &baseRegion);
  ~Region();

  // member functions
  // average given culture and religion shares of all provinces
  // TODO: calc distinct values for religion and culture mixes
  void sumPopulations();
};
} // namespace Scenario