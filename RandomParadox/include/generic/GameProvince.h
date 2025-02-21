#pragma once
#include "FastWorldGenerator.h"
namespace Scenario {
class GameProvince {
public:
  int ID;
  std::string name;
  std::string owner;
  std::string terrainType;
  double popFactor;
  double devFactor;
  double cityShare;
  std::shared_ptr<Fwg::Province> baseProvince;
  // containers
  std::vector<GameProvince> neighbours;
  // TODO: Remove this crap
  std::map<std::string, double> attributeDoubles;
  // TODO: Remove this crap
  std::map<std::string, std::string> attributeStrings;
  // constructors/destructor
  GameProvince(std::shared_ptr<Fwg::Province> province);
  GameProvince();
  ~GameProvince();
  // operators
  bool operator==(const GameProvince &right) const { return ID == right.ID; };
  bool operator<(const GameProvince &right) const { return ID < right.ID; };
  std::string toHexString();
};
} // namespace Scenario
