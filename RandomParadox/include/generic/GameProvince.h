#pragma once
#include "Culture.h"
#include "FastWorldGenerator.h"
#include "Religion.h"
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
  Fwg::Province *baseProvince;
  // containers
  std::vector<GameProvince> neighbours;
  // TODO: Remove this crap
  std::map<std::string, double> attributeDoubles;
  // TODO: Remove this crap
  std::map<std::string, std::string> attributeStrings;
  std::map<std::shared_ptr<Scenario::Religion>, double> religions;
  std::map<std::shared_ptr<Scenario::Culture>, double> cultures;
  // constructors/destructor
  GameProvince(Fwg::Province *province);
  GameProvince();
  ~GameProvince();
  // operators
  bool operator==(const GameProvince &right) const { return ID == right.ID; };
  bool operator<(const GameProvince &right) const { return ID < right.ID; };
  std::string toHexString();
};
} // namespace Scenario
