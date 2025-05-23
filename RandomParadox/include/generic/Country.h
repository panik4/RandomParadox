#pragma once
#include "FastWorldGenerator.h"
#include "RandNum.h"
#include "generic/Character.h"
#include "generic/Culture.h"
#include "generic/Flag.h"
#include "generic/GameRegion.h"
#include <string>
#include <vector>
namespace Scenario {

enum class Rank {
  GreatPower,
  SecondaryPower,
  RegionalPower,
  LocalPower,
  MinorPower,
  Unranked
};
class Country {

public:
  // member variables
  const int ID;
  std::string tag;
  std::string name;
  std::string adjective;
  int capitalRegionID = 0;
  int capitalProvinceID;
  double populationFactor;
  double averageDevelopment;
  double worldPopulationShare;
  double worldEconomicActivityShare;
  bool landlocked = true;

  Rank rank = Rank::Unranked;
  // total importance
  double importanceScore;
  // relative importance to most important country
  double relativeScore;
  // the gamemodule calculates the total population
  int pop;
  // the cultures and their populationFactor
  std::map<std::shared_ptr<Culture>, double> cultures;
  Gfx::Flag flag;
  Fwg::Gfx::Colour colour;
  std::vector<Scenario::Character> characters;
  // constructors/destructors
  Country();
  Country(std::string tag, int ID, std::string name, std::string adjective,
          Gfx::Flag flag);
  virtual ~Country() = default;
  // containers
  std::vector<std::shared_ptr<Region>> ownedRegions;
  std::vector<std::shared_ptr<GameProvince>> ownedProvinces;

  std::set<std::shared_ptr<Country>> neighbours;
  // member functions
  void assignRegions(int maxRegions,
                     std::vector<std::shared_ptr<Region>> &gameRegions,
                     std::shared_ptr<Region> startRegion,
                     std::vector<std::shared_ptr<GameProvince>> &gameProvinces);
  void addRegion(std::shared_ptr<Region> region);
  void removeRegion(std::shared_ptr<Region> region);
  void selectCapital();
  // operators
  bool operator<(const Country &right) const { return ID < right.ID; };
  void evaluateProvinces();
  void evaluatePopulations(const double worldPopulationFactor);
  void evaluateDevelopment();
  void evaluateEconomicActivity(const double worldEconomicActivity);
  void evaluateProperties();

  void gatherCultureShares();
  virtual std::shared_ptr<Culture> getPrimaryCulture() const;
};
} // namespace Scenario