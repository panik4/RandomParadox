#include "hoi4/Hoi4Country.h"

Hoi4Country::Hoi4Country() {  }

Hoi4Country::Hoi4Country(PdoxCountry &c) : PdoxCountry(c) {
  for (auto &region : ownedRegions) {
    hoi4Regions.push_back(region);
  }
}

Hoi4Country::~Hoi4Country() {}
