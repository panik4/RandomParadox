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
void SuperRegion::setType() {
  bool foundLand = false;
  bool foundSea = false;
  std::vector<int> pixels;
  for (auto &reg : gameRegions) {
    for (auto &prov : reg->provinces) {
      if (prov->sea) {
        foundSea = true;
      } else {
        foundLand = true;
      }
    }
  }
  if (foundLand && !foundSea) {
    this->type = Fwg::Areas::AreaType::Land;
  }
  if (!foundLand && foundSea) {
    this->type = Fwg::Areas::AreaType::Sea;
  }
  if (foundLand && foundSea) {
    this->type = Fwg::Areas::AreaType::Mixed;
  }
}
// gathers all pixels from all regions in the super region, calculates the
// weighted position
void Scenario::SuperRegion::checkPosition(
    const std::vector<SuperRegion> &superRegions) {
  std::vector<int> pixels;
  for (auto &reg : gameRegions) {
    for (auto &prov : reg->provinces) {
      for (auto &pix : prov->pixels) {
        pixels.push_back(pix);
      }
    }
  }

  this->position.calcWeightedCenter(pixels);
  if (!this->position.centerPresent(pixels)) {
    Fwg::Utils::Logging::logLine("Warning: Weighted center not inside the "
                                 "pixels of the super region with ID: ",
                                 this->ID, " type is: ", (int)this->type);
    // print the position of the super region
    Fwg::Utils::Logging::logLine("Position: ", this->position);
    // now we check the type of the super region our center is actually in
    for (auto &superReg : superRegions) {
      std::set<int> othersPixels;
      for (auto &reg : superReg.gameRegions) {
        for (auto &prov : reg->provinces) {
          for (auto &pix : prov->pixels) {
            othersPixels.insert(pix);
          }
        }
      }
      if (othersPixels.find(this->position.weightedCenter) !=
          othersPixels.end()) {
        Fwg::Utils::Logging::logLine(
            "The center is in super region with ID: ", superReg.ID,
            " type is: ", (int)superReg.type);
      }
    }
  }
}
} // namespace Scenario