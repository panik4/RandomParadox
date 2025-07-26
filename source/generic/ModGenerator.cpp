#include "generic/ModGenerator.h"
namespace Logging = Fwg::Utils::Logging;
namespace Scenario {
using namespace Fwg::Gfx;
ModGenerator::ModGenerator(){}

ModGenerator::ModGenerator(const std::string &configSubFolder)
    : Scenario::Generator(configSubFolder) {
  Gfx::Flag::readColourGroups();
  Gfx::Flag::readFlagTypes();
  Gfx::Flag::readFlagTemplates();
  Gfx::Flag::readSymbolTemplates();
  stratRegionMap = Bitmap(0, 0, 24);
}

ModGenerator::ModGenerator(Scenario::Generator &scenGen)
    : Scenario::Generator(scenGen) {}

ModGenerator::~ModGenerator() {}

void ModGenerator::generateStrategicRegions() {
  Fwg::Utils::Logging::logLine(
      "Scenario: Dividing world into strategic regions");
  strategicRegions.clear();
  stratRegionMap.clear();
  const auto &config = Fwg::Cfg::Values();

  std::vector<int> waterAreaPixels;
  std::vector<int> landAreaPixels;
  for (auto &region : this->gameRegions) {
    // as per types, group the regions. Land and lake together, while ocean and
    // islands together. MixedLand is landArea
    // sum up all the province pixels of the region in one vector
    region->pixels = region->gatherPixels();
    if (region->type == Region::RegionType::Ocean ||
        region->type == Region::RegionType::OceanCoastal ||
        region->type == Region::RegionType::OceanIslandCoastal ||
        region->type == Region::RegionType::OceanMixedCoastal ||
        region->type == Region::RegionType::CoastalIsland ||
        region->type == Region::RegionType::Island ||
        region->type == Region::RegionType::IslandLake) {
      for (auto &province : region->provinces) {
        waterAreaPixels.insert(waterAreaPixels.end(), region->pixels.begin(),
                               region->pixels.end());
      }
    } else {
      for (auto &province : region->provinces) {
        landAreaPixels.insert(landAreaPixels.end(), region->pixels.begin(),
                              region->pixels.end());
      }
    }
  }
  auto landShare = static_cast<double>(landAreaPixels.size()) /
                   (landAreaPixels.size() + waterAreaPixels.size());
  Fwg::Utils::Logging::logLine("Land share: ", landShare);
  auto waterShare = static_cast<double>(waterAreaPixels.size()) /
                    (landAreaPixels.size() + waterAreaPixels.size());
  Fwg::Utils::Logging::logLine("Water share: ", waterShare);
  if (landShare + waterShare != 1.0) {
    Fwg::Utils::Logging::logLine(
        "Error: Land and water share do not add up to 1.0");
  }
  // calculate the amount of strategic regions we want to have
  int landStratRegions =
      static_cast<int>(landShare * 110.0 * 2.0 * this->strategicRegionFactor);
  int waterStratRegions =
      static_cast<int>(waterShare * 110.0 * 1.0 * this->strategicRegionFactor);
  if (config.debugLevel > 5) {
    // lets debug visualise the water and land areas in the same image
    Bitmap areaBmp(config.width, config.height, 24);
    for (const auto &pix : landAreaPixels) {
      areaBmp.setColourAtIndex(pix, config.colours.at("land"));
    }
    for (const auto &pix : waterAreaPixels) {
      areaBmp.setColourAtIndex(pix, config.colours.at("sea"));
    }
    Png::save(areaBmp, config.mapsPath + "debug//waterLandAreas.png", false);
  }
  int landMinDist = Fwg::Utils::computePoissonMinDistFromArea(
      landAreaPixels.size(), landStratRegions, config.width, 8.0);
  int waterMinDist = Fwg::Utils::computePoissonMinDistFromArea(
      waterAreaPixels.size(), waterStratRegions, config.width, 8.0);

  auto waterPoints = Fwg::Utils::generatePoissonDiskPoints(
      waterAreaPixels, config.width, waterStratRegions, waterMinDist);
  auto landPoints = Fwg::Utils::generatePoissonDiskPoints(
      landAreaPixels, config.width, landStratRegions, landMinDist);

  std::vector<int> validLandSeeds;
  auto landVoronois = Fwg::Utils::growRegionsMultiSourceClusters(
      landAreaPixels, landPoints, config.width, config.height,
      /*wrapX=*/false,
      /*fillIslands=*/true, &validLandSeeds);

  std::vector<int> validWaterSeeds;
  auto waterVoronois = Fwg::Utils::growRegionsMultiSourceClusters(
      waterAreaPixels, waterPoints, config.width, config.height,
      /*wrapX=*/false,
      /*fillIslands=*/true, &validWaterSeeds);

  if (config.debugLevel > 5) {
    // debug visualise landVoronoi
    Bitmap landVoronoiBmp(config.width, config.height, 24);
    for (auto &landvor : landVoronois) {
      Fwg::Gfx::Colour c;
      c.randomize();
      for (const auto &pix : landvor) {
        landVoronoiBmp.setColourAtIndex(pix, c);
      }
    }
    Png::save(landVoronoiBmp, config.mapsPath + "debug//landVoronoi.png",
              false);
    //  debug visualise waterVoronoi
    Bitmap waterVoronoiBmp(config.width, config.height, 24);
    for (auto &watervor : waterVoronois) {
      Fwg::Gfx::Colour c;
      c.randomize();
      for (const auto &pix : watervor) {
        waterVoronoiBmp.setColourAtIndex(pix, c);
      }
    }
    Png::save(waterVoronoiBmp, config.mapsPath + "debug//waterVoronoi.png",
              false);
  }

  std::vector<int> indexToVoronoiID(config.bitmapSize);
  for (int i = 0; i < landVoronois.size(); ++i) {
    for (const auto &pix : landVoronois[i]) {
      indexToVoronoiID[pix] = i;
    }
  }
  for (int i = 0; i < waterVoronois.size(); ++i) {
    for (const auto &pix : waterVoronois[i]) {
      indexToVoronoiID[pix] = i + landVoronois.size();
    }
  }
  for (auto &landVor : landVoronois) {
    StrategicRegion stratRegion;
    stratRegion.ID = strategicRegions.size();
    stratRegion.areaType = Fwg::Areas::AreaType::Land;
    strategicRegions.push_back(stratRegion);
  }
  for (auto &waterVor : waterVoronois) {
    StrategicRegion stratRegion;
    stratRegion.ID = strategicRegions.size();
    stratRegion.areaType = Fwg::Areas::AreaType::Sea;
    strategicRegions.push_back(stratRegion);
  }
  // just add a map to track which region belongs to which type of voronoiArea
  std::map<int, Fwg::Areas::AreaType> regionAreaTypeMap;
  // now we match the regions to the voronoi areas, and create the strategic
  // regions by a best fit
  for (auto &region : this->gameRegions) {
    std::unordered_map<int, int> voronoiOverlap;
    auto &regionPixels = region->pixels;
    for (const auto &pix : regionPixels) {
      auto voronoiID = indexToVoronoiID[pix];
      if (voronoiOverlap.find(voronoiID) == voronoiOverlap.end()) {
        voronoiOverlap[voronoiID] = 1;
      } else {
        voronoiOverlap[voronoiID]++;
      }
    }
    // now find the voronoi area with the most overlap
    int maxOverlap = 0;
    int bestVoronoiID = -1;
    for (const auto &voronoi : voronoiOverlap) {
      if (voronoi.second > maxOverlap) {
        maxOverlap = voronoi.second;
        bestVoronoiID = voronoi.first;
      }
    }
    strategicRegions[bestVoronoiID].addRegion(region);
    regionAreaTypeMap[region->ID] = strategicRegions[bestVoronoiID].areaType;
  }
  // to track which regions should be reassigned later after evaluation of some
  // metrics
  std::queue<std::shared_ptr<Region>> regionsToBeReassigned;

  // postprocess stratregions
  for (auto &stratRegion : strategicRegions) {
    stratRegion.colour.randomize();
    // lets sum up all the pixels of the strategic region
    for (auto &gameRegion : stratRegion.gameRegions) {
      stratRegion.pixels.insert(stratRegion.pixels.end(),
                                gameRegion->pixels.begin(),
                                gameRegion->pixels.end());
    }
    // let's find the weighted centre of the strat region
    stratRegion.position.calcWeightedCenter(stratRegion.pixels);

    // now get all clusters
    stratRegion.gameRegionClusters = stratRegion.getClusters(gameRegions);
    if (stratRegion.gameRegionClusters.size() > 1) {
      Fwg::Utils::Logging::logLineLevel(
          9, "Strategic region with ID: ", stratRegion.ID,
          " has multiple clusters: ", stratRegion.gameRegionClusters.size());
    }

    // now if the strategic region is of AreaType sea, free the smaller
    // clusters, add their regions to the regionsToBeReassigned vector
    if (stratRegion.gameRegionClusters.size() > 1 &&
        stratRegion.areaType == Fwg::Areas::AreaType::Sea) {
      Fwg::Utils::Logging::logLineLevel(
          9, "Strategic region with ID: ", stratRegion.ID,
          " has multiple clusters, trying to free smaller clusters");
      // the biggest cluster by pixels size remains
      auto biggestCluster =
          std::max_element(stratRegion.gameRegionClusters.begin(),
                           stratRegion.gameRegionClusters.end(),
                           [](const Cluster &a, const Cluster &b) {
                             return a.size() < b.size();
                           });
      // free the others
      for (auto &cluster : stratRegion.gameRegionClusters) {
        if (&cluster != &(*biggestCluster)) {
          Fwg::Utils::Logging::logLine("Freeing cluster with size: ",
                                       cluster.size());
          // add the regions of the cluster to the regionsToBeReassigned vector
          for (auto &region : cluster.regions) {
            regionsToBeReassigned.push(region);
            // remove the region from the stratregion gameRegions vector
            auto it = std::find(stratRegion.gameRegions.begin(),
                                stratRegion.gameRegions.end(), region);
            if (it != stratRegion.gameRegions.end()) {
              Fwg::Utils::Logging::logLine(
                  "Removing region with ID: ", region->ID,
                  " from strategic region with ID: ", stratRegion.ID);
              stratRegion.gameRegions.erase(it);

            } else {
              Fwg::Utils::Logging::logLine(
                  "Warning: Region not found in strategic region gameRegions");
            }
          }
          // clear the cluster regions
          cluster.regions.clear();
        }
      }

      // remove empty clusters
      stratRegion.gameRegionClusters.erase(
          std::remove_if(
              stratRegion.gameRegionClusters.begin(),
              stratRegion.gameRegionClusters.end(),
              [](const Cluster &cluster) { return cluster.regions.empty(); }),
          stratRegion.gameRegionClusters.end());
    }
  }

  // create a buffer to map which region is assigned to which strategic region
  std::map<int, int> assignedToIDs;
  for (auto &stratRegion : strategicRegions) {
    // assign the regions to the strategic region
    for (auto &region : stratRegion.gameRegions) {
      // check for duplicate regions
      if (assignedToIDs.count(region->ID)) {
        Fwg::Utils::Logging::logLine(
            "Warning: Region with ID: ", region->ID,
            " is already assigned to strategic region with ID: ",
            assignedToIDs[region->ID]);
        continue; // skip this region, it is already assigned
      }

      assignedToIDs[region->ID] = stratRegion.ID;
    }
  }

  // now we reassign the regions to the strategic regions
  // take from queue regionsToBeReassigned, and assign all of them until none
  // are left. We assign by taking all neighbours of the same AreaType, and
  // determining which one of those is closest to us using the position
  while (!regionsToBeReassigned.empty()) {
    auto region = regionsToBeReassigned.front();
    regionsToBeReassigned.pop();
    std::map<int, int> regionDistances;
    for (auto &neighbourId : region->neighbours) {
      auto &neighbourRegion = gameRegions[neighbourId];
      // if the neighbour region is of the same area type, we can consider that
      // ones distance, and it must already be assigned
      if (regionAreaTypeMap[neighbourRegion->ID] ==
              regionAreaTypeMap[region->ID] &&
          assignedToIDs.count(neighbourRegion->ID)) {
        // calculate the distance between the two regions
        auto distance = Fwg::Utils::getDistance(
            region->position.weightedCenter,
            neighbourRegion->position.weightedCenter, config.width);
        regionDistances[neighbourId] = distance;
      }
    }
    // now find the closest neighbour region
    if (regionDistances.empty()) {
      Fwg::Utils::Logging::logLine(
          "Warning: No neighbours of the same area type found for region with "
          "ID: ",
          region->ID);
      continue; // no neighbours of the same area type, skip
    }
    auto closestNeighbourIt = std::min_element(
        regionDistances.begin(), regionDistances.end(),
        [](const std::pair<int, int> &a, const std::pair<int, int> &b) {
          return a.second < b.second;
        });
    auto closestNeighbourID = closestNeighbourIt->first;
    auto closestNeighbourRegion = gameRegions[closestNeighbourID];
    // check if the closest neighbour region is already assigned to a strategic
    // region
    if (assignedToIDs.find(closestNeighbourID) != assignedToIDs.end()) {
      // if it is, assign the region to the same strategic region
      auto stratRegionID = assignedToIDs[closestNeighbourID];
      Fwg::Utils::Logging::logLine(
          "Reassigning region with ID: ", region->ID,
          " to strategic region with ID: ", stratRegionID);
      strategicRegions[stratRegionID].addRegion(region);
      assignedToIDs[region->ID] = stratRegionID;
    } else {
      // if it is not, put it back on the queue to be reassigned
      Fwg::Utils::Logging::logLine(
          "No strategic region found for region with ID: ", region->ID,
          " closest neighbour is: ", closestNeighbourID);
      regionsToBeReassigned.push(region);
    }
  }

  // safety check to see if all regions are assigned to some region
  for (const auto &region : gameRegions) {
    if (assignedToIDs.find(region->ID) == assignedToIDs.end()) {
      Fwg::Utils::Logging::logLine("Warning: Region with ID: ", region->ID,
                                   " is not assigned to any strategic region");
      // assign it to the first strategic region
      if (!strategicRegions.empty()) {
        strategicRegions[0].addRegion(region);
        assignedToIDs[region->ID] = strategicRegions[0].ID;
      } else {
        Fwg::Utils::Logging::logLine(
            "Error: No strategic regions available, cannot assign region with "
            "ID: ",
            region->ID);
      }
    }
  }
  // delete all empty strategic regions
  strategicRegions.erase(
      std::remove_if(strategicRegions.begin(), strategicRegions.end(),
                     [](const StrategicRegion &stratRegion) {
                       return stratRegion.gameRegions.empty();
                     }),
      strategicRegions.end());
  // fix IDs of strategic regions
  for (size_t i = 0; i < strategicRegions.size(); ++i) {
    strategicRegions[i].ID = i;
    // also set the name of the strategic region
    strategicRegions[i].name = std::to_string(i + 1);
  }

  //  build a vector of superregions from all the strategic regions
  std::vector<SuperRegion> superRegions;
  for (auto &stratRegion : strategicRegions) {
    SuperRegion superRegion;
    superRegion.ID = stratRegion.ID;
    superRegion.gameRegions = stratRegion.gameRegions;
    superRegion.setType();
    superRegions.push_back(superRegion);
  }
  Fwg::Utils::Logging::logLine(
      "Scenario: Done Dividing world into strategic regions");
  for (auto &stratRegion : strategicRegions) {
    stratRegion.checkPosition(superRegions);
    stratRegion.name = std::to_string(stratRegion.ID + 1);
  }
  visualiseStrategicRegions();
  Fwg::Utils::Logging::logLine(
      "Scenario: Done visualising and checking positions of strategic regions");

  return;
}

Fwg::Gfx::Bitmap ModGenerator::visualiseStrategicRegions(const int ID) {
  if (!stratRegionMap.size()) {
    stratRegionMap =
        Bitmap(Fwg::Cfg::Values().width, Fwg::Cfg::Values().height, 24);
  }
  if (ID > -1) {
    auto &strat = strategicRegions[ID];
    for (auto &reg : strat.gameRegions) {
      for (auto &prov : reg->gameProvinces) {
        for (auto &pix : prov->baseProvince->pixels) {
          stratRegionMap.setColourAtIndex(pix, strat.colour);
        }
      }
      for (auto &pix : reg->borderPixels) {
        stratRegionMap.setColourAtIndex(pix, strat.colour * 0.5);
      }
    }
  } else {
    auto noBorderMap = Fwg::Gfx::Bitmap(Fwg::Cfg::Values().width,
                                        Fwg::Cfg::Values().height, 24);
    for (auto &strat : strategicRegions) {

      for (auto &reg : strat.gameRegions) {
        for (auto &prov : reg->gameProvinces) {
          for (auto &pix : prov->baseProvince->pixels) {
            stratRegionMap.setColourAtIndex(pix, strat.colour);
            if (ID == -1) {
              noBorderMap.setColourAtIndex(pix, strat.colour);
            }
          }
        }
        for (auto &pix : reg->borderPixels) {
          if (strat.centerOutsidePixels) {
            stratRegionMap.setColourAtIndex(pix,
                                            Fwg::Gfx::Colour(255, 255, 255));
          } else {
            stratRegionMap.setColourAtIndex(pix, strat.colour * 0.5);
          }
        }
      }
    }
    Png::save(noBorderMap,
              Fwg::Cfg::Values().mapsPath + "stratRegions_no_borders.png");
    Png::save(stratRegionMap, Fwg::Cfg::Values().mapsPath + "stratRegions.png");
  }
  return stratRegionMap;
}

} // namespace Scenario