#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "GameProvince.h"
#include <map>
class GameRegion {
  std::vector<std::string> cores;

public:
  // member variables
  std::string owner;
  std::string name;
  int ID;
  bool sea;
  const Region baseRegion;
  Position position;
  bool assigned;
  // containers
  std::vector<int> neighbours;
  std::vector<GameProvince> gameProvinces;
  std::map<std::string, std::string> attributeStrings;
  std::map<std::string, double> attributeDoubles;

  std::vector<double> temperatureRange;
  std::vector<double> dateRange;
  double snowChance, lightRainChance, heavyRainChance, blizzardChance,
      mudChance, sandStormChance;
  GameRegion();
  GameRegion(const Region &baseRegion);
  ~GameRegion();
};
