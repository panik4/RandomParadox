#pragma once
#include "FastWorldGenerator.h"
#include "GameProvince.h"
#include "generic/Culture.h"
#include "generic/Religion.h"
#include "generic/ScenarioUtils.h"
#include <map>
namespace Scenario {
class Country;
enum class LocatorType { NONE, CITY, FARM, MINE, PORT, WOOD };
class Region : public Fwg::Areas::Region {
  std::vector<std::string> cores;

public:
  // member variables
  std::shared_ptr<Country> owner;
  std::string name;
  int superRegionID;
  // calculate this for the scenario in general
  double populationFactor;
  double worldPopulationShare;
  double developmentFactor;
  double importanceScore;
  double relativeImportance;
  // calculate this in every module
  int totalPopulation;
  // this calculated in every module, usually by taking both population and
  // development into account
  double economicActivity;
  double worldEconomicActivityShare;
  std::map<std::string, Scenario::Utils::Resource> resources;

  // other
  bool assigned;
  double snowChance, lightRainChance, heavyRainChance, blizzardChance,
      mudChance, sandStormChance;
  // containers
  std::vector<std::shared_ptr<GameProvince>> gameProvinces;
  std::vector<double> temperatureRange;
  std::vector<double> dateRange;
  std::map<std::shared_ptr<Scenario::Religion>, double> religions;
  // the sum here should ALWAYS be 1
  std::map<std::shared_ptr<Scenario::Culture>, double> cultureShares;
  std::vector<std::shared_ptr<Fwg::Civilization::Location>>
      significantLocations;

  Region();
  Region(const Fwg::Areas::Region &baseRegion);
  virtual ~Region();

  // member functions
  // average given culture and religion shares of all provinces
  // TODO: calc distinct values for religion and culture mixes
  void sumPopulations();

  void findLocator(Fwg::Civilization::LocationType locationType, int maxAmount);

  void findPortLocator(int maxAmount = 1);
  void findCityLocator(int maxAmount = 1);
  void findMineLocator(int maxAmount = 1);
  void findFarmLocator(int maxAmount = 1);
  void findWoodLocator(int maxAmount = 1);
  void findWaterLocator(int maxAmount = 1);

  void findWaterPortLocator(int maxAmount = 1);

  std::shared_ptr<Fwg::Civilization::Location>
  getLocation(Fwg::Civilization::LocationType type);
  std::shared_ptr<Scenario::Culture> getPrimaryCulture();
};
} // namespace Scenario