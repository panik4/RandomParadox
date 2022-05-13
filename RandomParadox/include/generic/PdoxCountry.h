#pragma once
#include "FastWorldGenerator.h"
#include "generic/Flag.h"
#include "generic/GameRegion.h"
#include <string>
#include <vector>
class PdoxCountry {
public:
  // member variables
  const int ID;
  std::string tag;
  std::string name;
  std::string adjective;
  int capitalRegionID;
  double developmentFactor;
  Flag flag;
  Colour colour;
  // constructors/destructors
  PdoxCountry();
  PdoxCountry(std::string tag, int ID, std::string name, std::string adjective,
              Flag flag);
  ~PdoxCountry();
  // containers
  std::vector<int> ownedRegions;
  std::set<std::string> neighbours;
  // member functions
  void addRegion(GameRegion &region, std::vector<GameRegion> &gameRegions,
                 std::vector<GameProvince> &gameProvinces);
  void assignRegions(int maxRegions, std::vector<GameRegion> &gameRegions,
                     GameRegion &startRegion,
                     std::vector<GameProvince> &gameProvinces);
  // operators
  bool operator<(const PdoxCountry &right) const { return ID < right.ID; };
};
