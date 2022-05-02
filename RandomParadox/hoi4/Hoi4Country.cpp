#include "Hoi4Country.h"

Hoi4Country::Hoi4Country() { Logger::logLine("AAAA"); }

Hoi4Country::Hoi4Country(Country &c) : Country(c) {
  for (auto &region : ownedRegions) {
    hoi4Regions.push_back(region);
  }
}

Hoi4Country::~Hoi4Country() {}
