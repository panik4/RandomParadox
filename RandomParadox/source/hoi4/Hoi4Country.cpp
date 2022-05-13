#include "hoi4/Hoi4Country.h"

Hoi4Country::Hoi4Country() {}

Hoi4Country::Hoi4Country(PdoxCountry &c, std::vector<GameRegion> &gameRegions)
    : PdoxCountry(c), allowElections{true}, bully{0}, parties{25, 25, 25, 25},
      relativeScore{0}, strengthScore{0} {
  for (auto &region : c.ownedRegions) {
    hoi4Regions.push_back(gameRegions[region]);
  }
}

Hoi4Country::~Hoi4Country() {}
