#include "generic/SuperRegion.h"
namespace Scenario {
SuperRegion::SuperRegion() {}
void SuperRegion::addRegion(std::shared_ptr<Region> region) {
  this->gameRegions.push_back(region);
  region->superRegionID = this->ID;
}
void SuperRegion::removeRegion(std::shared_ptr<Region> region) {
  for (auto &reg : gameRegions) {
    if (reg->ID == region->ID) {
      gameRegions.erase(
          std::remove(gameRegions.begin(), gameRegions.end(), reg),
          gameRegions.end());
      break;
    }
  }
}
} // namespace Scenario