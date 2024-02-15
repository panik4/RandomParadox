#pragma once
#include "FastWorldGenerator.h"
#include "RandNum/RandNum.h"
#include "generic/Flag.h"
#include "generic/GameRegion.h"
#include <string>
#include <vector>
namespace Scenario {
class Country {

public:
  // member variables
  const int ID;
  std::string tag;
  std::string name;
  std::string adjective;
  int capitalRegionID;
  double populationFactor;
  double developmentFactor;
  // the gamemodule calculate the total population
  int pop;
  Gfx::Flag flag;
  Fwg::Gfx::Colour colour;
  // constructors/destructors
  Country();
  Country(std::string tag, int ID, std::string name, std::string adjective,
              Gfx::Flag flag);
  ~Country();
  // containers
  std::vector<std::shared_ptr<Region>> ownedRegions;
  std::set<std::string> neighbours;
  // member functions
  void assignRegions(int maxRegions,
                     std::vector<std::shared_ptr<Region>> &gameRegions,
                     std::shared_ptr<Region> startRegion,
                     std::vector<std::shared_ptr<GameProvince>> &gameProvinces);
  void addRegion(std::shared_ptr<Region> region,
                 std::vector<std::shared_ptr<Region>> &gameRegions,
                 std::vector<std::shared_ptr<GameProvince>> &gameProvinces);
  // operators
  bool operator<(const Country &right) const { return ID < right.ID; };
};
} // namespace Scenario