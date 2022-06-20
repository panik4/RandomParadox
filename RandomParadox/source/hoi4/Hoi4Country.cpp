#include "hoi4/Hoi4Country.h"

namespace Scenario::Hoi4 {
Hoi4Country::Hoi4Country() {}

Hoi4Country::Hoi4Country(PdoxCountry &c,
                         std::vector<Scenario::Region> &gameRegions)
    : PdoxCountry(c), allowElections{true}, bully{0.0}, parties{25, 25, 25, 25},
      relativeScore{0.0}, strengthScore{0} {
  for (auto &region : c.ownedRegions) {
    hoi4Regions.push_back(gameRegions[region]);
  }
}

Hoi4Country::~Hoi4Country() {}

} // namespace Scenario::Hoi4