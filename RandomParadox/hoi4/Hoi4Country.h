#pragma once
#include "../generic/countries/Country.h"
#include "Hoi4GameRegion.h"
#include <array>
#include <string>
#include <vector>
class Hoi4Country : public Country {
public:
  Hoi4Country();
  Hoi4Country(Country &country);
  ~Hoi4Country();
  std::string fullName;
  std::string gfxCulture;
  std::string rank;
  std::string rulingParty;
  bool allowElections;
  double bully;
  double strengthScore;
  double relativeScore;
  int stateCategory;
  std::vector<int> doctrines;
  std::vector<int> units;
  std::vector<int> unitCount;
  std::array<int, 4> parties;
  std::vector<Hoi4GameRegion> hoi4Regions;
};
