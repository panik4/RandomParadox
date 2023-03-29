#include "generic/GameRegion.h"
namespace Scenario {
Region::Region() {}

Region::Region(const Fwg::Region &baseRegion)
    : Fwg::Region(baseRegion), assigned(false) {}

Region::~Region() {}
void Region::sumPopulations() {
  double totalShare = gameProvinces.size();
  for (const auto &prov : gameProvinces) {
    for (auto &religion : prov->religions) {
      // if we don't find the religion, add it to the map
      if (religions.find(religion.first) == religions.end()) {
        religions[religion.first] = 0.0;
      }
      // now add the share found in this province, but divide it by the amount
      // of provinces
      religions[religion.first] += religion.second / totalShare;
    }
  }

    for (const auto &prov : gameProvinces) {
    for (auto &culture : prov->cultures) {
      // if we don't find the religion, add it to the map
      if (cultures.find(culture.first) == cultures.end()) {
        cultures[culture.first] = 0.0;
      }
      // now add the share found in this province, but divide it by the amount
      // of provinces
      cultures[culture.first] += culture.second / totalShare;
    }
  }

}
} // namespace Scenario