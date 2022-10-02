#include "hoi4/Hoi4Country.h"

namespace Scenario::Hoi4 {
Hoi4Country::Hoi4Country() {}

Hoi4Country::Hoi4Country(
    PdoxCountry &c, std::vector<std::shared_ptr<Scenario::Region>> &gameRegions)
    : PdoxCountry(c), allowElections{true}, bully{0.0},
      relativeScore{0.0}, parties{25, 25, 25, 25}, strengthScore{0} {
  for (auto &region : c.ownedRegions) {
      // we want a copy of the region, that's why we create a new region
    hoi4Regions.push_back(std::make_shared<Region>(*gameRegions[region]));
  }
}

Hoi4Country::~Hoi4Country() {}

} // namespace Scenario::Hoi4