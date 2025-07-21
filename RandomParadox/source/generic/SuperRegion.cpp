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
      if (prov->isSea()) {
        foundSea = true;
      } else {
        foundLand = true;
      }
    }
  }
  if (foundLand && !foundSea) {
    this->areaType = Fwg::Areas::AreaType::Land;
  }
  if (!foundLand && foundSea) {
    this->areaType = Fwg::Areas::AreaType::Sea;
  }
  if (foundLand && foundSea) {
    this->areaType = Fwg::Areas::AreaType::Mixed;
  }
}
// gathers all pixels from all regions in the super region, calculates the
// weighted position
void Scenario::SuperRegion::checkPosition(
    const std::vector<SuperRegion> &superRegions) {
  if (Fwg::Cfg::Values().debugLevel > 5) {
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
                                   this->ID, " type is: ", (int)this->areaType);
      this->centerOutsidePixels = true;
      // print the position of the super region
      Fwg::Utils::Logging::logLine("Position: ", this->position);
      // now we check the type of the super region our center is actually in
      for (auto &superReg : superRegions) {
        std::unordered_set<int> othersPixels;
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
              " type is: ", (int)superReg.areaType);
        }
      }
    }
  }
}
std::vector<Cluster> Scenario::SuperRegion::getClusters(
    std::vector<std::shared_ptr<Region>> &regions) {
  std::vector<Cluster> clusters;

  // Mark all regions in this superregion as unvisited
  std::unordered_set<int> visited;
  auto allOwnRegions = this->gameRegions; // all regions in this superregion

  // Helper lambda for DFS
  auto dfs = [&](auto &&self, std::shared_ptr<Region> current,
                 std::vector<std::shared_ptr<Region>> &cluster) -> void {
    visited.insert(current->ID);
    cluster.push_back(current);

    for (int neighborID : current->neighbours) {
      if (neighborID < 0 || neighborID >= (int)regions.size())
        continue; // skip invalid IDs

      auto neighbor = regions[neighborID];
      // Only visit neighbors belonging to the same superregion
      if (neighbor && visited.find(neighbor->ID) == visited.end()) {
        if (std::find(allOwnRegions.begin(), allOwnRegions.end(), neighbor) !=
            allOwnRegions.end()) {
          self(self, neighbor, cluster);
        }
      }
    }
  };

  // Main loop: start DFS for each unvisited region
  for (auto &region : allOwnRegions) {
    if (visited.find(region->ID) == visited.end()) {
      std::vector<std::shared_ptr<Region>> cluster;
      dfs(dfs, region, cluster);
      // Create a Cluster object and add it to the result
      Cluster clusterObj;
      clusterObj.regions = std::move(cluster);
      // gather all pixels from the cluster
      for (const auto &reg : clusterObj.regions) {
        clusterObj.pixels.insert(clusterObj.pixels.end(),
                                 region->pixels.begin(), region->pixels.end());
      }

      clusters.push_back(std::move(clusterObj));
    }
  }

  return clusters;
}
} // namespace Scenario