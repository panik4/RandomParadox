#include "generic/ScenarioGenerator.h"
namespace Logging = Fwg::Utils::Logging;
namespace Scenario {
using namespace Fwg::Gfx;
Generator::Generator() {}

Generator::Generator(const std::string &configSubFolder)
    : FastWorldGenerator(configSubFolder) {
  Gfx::Flag::readColourGroups();
  Gfx::Flag::readFlagTypes();
  Gfx::Flag::readFlagTemplates();
  Gfx::Flag::readSymbolTemplates();
  stratRegionMap = Bitmap(0, 0, 24);
}

Generator::~Generator() {}

void Generator::loadRequiredResources(const std::string &gamePath) {}

void Generator::mapContinents() {
  Logging::logLine("Mapping Continents");
  scenContinents.clear();
  for (const auto &continent : this->areaData.continents) {
    // we copy the fwg continents by choice, to leave them untouched
    scenContinents.push_back(ScenarioContinent(continent));
  }
}

void Generator::mapRegions() {
  Logging::logLine("Mapping Regions");
  gameRegions.clear();

  for (auto &region : this->areaData.regions) {
    std::sort(region.provinces.begin(), region.provinces.end(),
              [](const std::shared_ptr<Fwg::Areas::Province> a,
                 const std::shared_ptr<Fwg::Areas::Province> b) {
                return (*a < *b);
              });
    auto gameRegion = std::make_shared<Region>(region);

    for (auto &province : gameRegion->provinces) {
      gameRegion->gameProvinces.push_back(gameProvinces[province->ID]);
    }
    // save game region
    gameRegions.push_back(gameRegion);
  }
  // sort by gameprovince ID
  std::sort(gameRegions.begin(), gameRegions.end(),
            [](auto l, auto r) { return *l < *r; });
  // check if we have the same amount of gameProvinces as FastWorldGen provinces
  if (gameProvinces.size() != this->areaData.provinces.size())
    throw(std::exception("Fatal: Lost provinces, terminating"));
  if (gameRegions.size() != this->areaData.regions.size())
    throw(std::exception("Fatal: Lost regions, terminating"));
  for (const auto &gameRegion : gameRegions) {
    if (gameRegion->ID > gameRegions.size()) {
      throw(std::exception("Fatal: Invalid region IDs, terminating"));
    }
  }
  applyRegionInput();
}

void Generator::applyRegionInput() {
  Fwg::Utils::ColourTMap<std::vector<std::string>> regionInputMap;
  if (regionMappingPath.size() && std::filesystem::exists(regionMappingPath)) {
    auto mappingFileLines = Fwg::Parsing::getLines(regionMappingPath);
    for (auto &line : mappingFileLines) {
      auto tokens = Fwg::Parsing::getTokens(line, ';');
      auto colour = Fwg::Gfx::Colour(std::stoi(tokens[0]), std::stoi(tokens[1]),
                                     std::stoi(tokens[2]));
      regionInputMap.setValue(colour, tokens);
    }
  }
  for (auto &gameRegion : this->gameRegions) {
    if (regionInputMap.find(gameRegion->colour)) {
      if (regionInputMap[gameRegion->colour].size() > 3 &&
          regionInputMap[gameRegion->colour][3].size()) {
        // get the predefined name
        gameRegion->name = regionInputMap[gameRegion->colour][3];
      }
      if (regionInputMap[gameRegion->colour].size() > 4 &&
          regionInputMap[gameRegion->colour][4].size()) {
        try {

          // get the predefined population
          gameRegion->totalPopulation =
              stoi(regionInputMap[gameRegion->colour][4]);
        } catch (std::exception e) {
          Fwg::Utils::Logging::logLine(
              "ERROR: Some of the tokens can't be turned into a population "
              "number. The faulty token is ",
              regionInputMap[gameRegion->colour][4]);
        }
      }
    }
  }
  // debug visualisation of all regions, if coastal they are yellow, if sea they
  // are blue, if non-coastal they are green
  Bitmap regionMap(Fwg::Cfg::Values().width, Fwg::Cfg::Values().height, 24);
  for (auto &gameRegion : this->gameRegions) {
    for (auto &gameProv : gameRegion->gameProvinces) {
      for (auto &pix : gameProv->baseProvince->pixels) {
        if (gameRegion->isSea()) {
          regionMap.setColourAtIndex(pix, Fwg::Cfg::Values().colours.at("sea"));

        } else if (gameRegion->coastal && !gameRegion->isSea()) {
          regionMap.setColourAtIndex(pix,
                                     Fwg::Cfg::Values().colours.at("ores"));

        } else if (gameRegion->isLake()) {
          regionMap.setColourAtIndex(pix,
                                     Fwg::Cfg::Values().colours.at("lake"));
        }

        else {
          regionMap.setColourAtIndex(pix,
                                     Fwg::Cfg::Values().colours.at("land"));
          if (gameProv->baseProvince->coastal) {
            regionMap.setColourAtIndex(
                pix, Fwg::Cfg::Values().colours.at("autumnForest"));
          }
        }
      }
    }
  }
  Png::save(regionMap, Fwg::Cfg::Values().mapsPath + "debug//regionTypes.png",
            false);
}

void Generator::mapProvinces() {
  gameProvinces.clear();
  for (auto &prov : this->areaData.provinces) {
    // edit coastal status: lakes are not coasts!
    if (prov->coastal && prov->isLake())
      prov->coastal = false;
    // if it is a land province, check that a neighbour is an ocean, otherwise
    // this isn't coastal in this scenario definition
    else if (prov->coastal) {
      bool foundTrueCoast = false;
      for (auto &neighbour : prov->neighbours) {
        if (neighbour->isSea()) {
          foundTrueCoast = true;
        }
      }
      prov->coastal = foundTrueCoast;
    }

    // now create gameprovinces from FastWorldGen provinces
    auto gP = std::make_shared<GameProvince>(prov);
    // also copy neighbours
    for (auto &baseProvinceNeighbour : gP->baseProvince->neighbours)
      gP->neighbours.push_back(baseProvinceNeighbour);
    gameProvinces.push_back(gP);
  }

  // sort by gameprovince ID
  std::sort(gameProvinces.begin(), gameProvinces.end(),
            [](auto l, auto r) { return *l < *r; });
}

void Generator::cutFromFiles(const std::string &gamePath) {
  Fwg::Utils::Logging::logLine("Unimplemented cutting");
}
// initialize states
void Generator::initializeStates() {}
// initialize states
void Generator::mapCountries() {}

Fwg::Gfx::Bitmap Generator::mapTerrain() {
  Bitmap typeMap(climateMap.width(), climateMap.height(), 24);
  auto &colours = Fwg::Cfg::Values().colours;
  typeMap.fill(colours.at("sea"));
  Logging::logLine("Mapping Terrain");
  for (auto &gameRegion : gameRegions) {
    for (auto &gameProv : gameRegion->gameProvinces) {
    }
  }
  Png::save(typeMap, Fwg::Cfg::Values().mapsPath + "/typeMap.png");
  return typeMap;
}

std::shared_ptr<Region> &Generator::findStartRegion() {
  std::vector<std::shared_ptr<Region>> freeRegions;
  for (const auto &gameRegion : gameRegions)
    if (!gameRegion->assigned && !gameRegion->isSea() && !gameRegion->isLake())
      freeRegions.push_back(gameRegion);

  if (freeRegions.size() == 0)
    return gameRegions[0];

  const auto &startRegion = Fwg::Utils::selectRandom(freeRegions);
  return gameRegions[startRegion->ID];
}

// generate countries according to given ruleset for each game
// TODO: rulesets, e.g. naming schemes? tags? country size?

void Generator::generateStrategicRegions() {
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
  int landStratRegions = static_cast<int>(landShare * 110.0 * 2.0 *
                                          this->strategicRegionFactor);
  int waterStratRegions = static_cast<int>(waterShare * 110.0 *
                                           1.0 * this->strategicRegionFactor);
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
      landAreaPixels.size(), landStratRegions, config.width, 4.0);
  int waterMinDist = Fwg::Utils::computePoissonMinDistFromArea(
      waterAreaPixels.size(), waterStratRegions, config.width, 4.0);

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
      std::cout << "Split region with clusters: "
                << stratRegion.gameRegionClusters.size() << std::endl;
    }

    // now if the strategic region is of AreaType sea, free the smaller
    // clusters, add their regions to the regionsToBeReassigned vector
    if (stratRegion.gameRegionClusters.size() > 1 &&
        stratRegion.areaType == Fwg::Areas::AreaType::Sea) {
      std::cout << "Split sea region with ID: " << stratRegion.ID
                << " with clusters: " << stratRegion.gameRegionClusters.size()
                << std::endl;
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
          std::cout << "Freeing cluster with size: " << cluster.size()
                    << std::endl;
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

Fwg::Gfx::Bitmap Generator::visualiseStrategicRegions(const int ID) {
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

Bitmap Generator::visualiseCountries(Fwg::Gfx::Bitmap &countryBmp,
                                     const int ID) {
  Logging::logLine("Drawing borders");
  auto &config = Fwg::Cfg::Values();
  if (!countryBmp.initialised()) {
    countryBmp = Bitmap(config.width, config.height, 24);
  }
  if (ID > -1) {
    for (const auto &prov : gameRegions[ID]->provinces) {
      auto countryColour = Fwg::Gfx::Colour(0, 0, 0);
      const auto &region = gameRegions[ID];

      if (region->owner) {
        countryColour = region->owner->colour;
      }
      for (const auto &pix : prov->pixels) {
        countryBmp.setColourAtIndex(pix,
                                    countryColour * 0.9 + prov->colour * 0.1);
      }
      for (auto &pix : region->borderPixels) {
        countryBmp.setColourAtIndex(pix, countryColour * 0.0);
      }
    }
  } else {
    Fwg::Gfx::Bitmap noBorderCountries(config.width, config.height, 24);
    for (const auto &region : gameRegions) {
      auto countryColour = Fwg::Gfx::Colour(0, 0, 0);
      // if this tag is assigned, use the colour
      if (region->owner) {
        countryColour = region->owner->colour;
      }
      for (const auto &prov : region->provinces) {
        for (const auto &pix : prov->pixels) {
          countryBmp.setColourAtIndex(pix,
                                      countryColour * 0.9 + prov->colour * 0.1);
          // clean export, for editing outside of the tool, for later loading
          noBorderCountries.setColourAtIndex(pix, countryColour * 1.0);
        }
      }
      for (auto &pix : region->borderPixels) {
        countryBmp.setColourAtIndex(pix, countryColour * 0.0);
      }
    }
    Png::save(noBorderCountries,
              Fwg::Cfg::Values().mapsPath + "countries_no_borders.png");
  }
  return countryBmp;
}

void Generator::distributeCountries() {

  auto &config = Fwg::Cfg::Values();

  Fwg::Utils::Logging::logLine("Distributing Countries");
  for (auto &countryEntry : countries) {
    auto &country = countryEntry.second;
    country->ownedRegions.clear();
    auto startRegion(findStartRegion());
    if (startRegion->assigned || startRegion->isSea() || startRegion->isLake())
      continue;
    country->assignRegions(6, gameRegions, startRegion, gameProvinces);
    if (!country->ownedRegions.size())
      continue;
    // get the dominant culture in the country by iterating over all regions
    // and counting the number of provinces with the same culture
    country->gatherCultureShares();
    auto culture = country->getPrimaryCulture();
    auto language = culture->language;
    country->name = language->generateGenericCapitalizedWord();
    country->adjective = language->getAdjectiveForm(country->name);
    country->tag = NameGeneration::generateTag(country->name, nData);
    for (auto &region : country->ownedRegions) {
      region->owner = country;
    }
  }
  Fwg::Utils::Logging::logLine("Distributing Countries::Assigning Regions");

  if (countries.size()) {
    for (auto &gameRegion : gameRegions) {
      if (!gameRegion->isSea() && !gameRegion->assigned &&
          !gameRegion->isLake()) {
        auto gR = Fwg::Utils::getNearestAssignedLand(
            gameRegions, gameRegion, config.width, config.height);
        gR->owner->addRegion(gameRegion);
        gameRegion->owner = gR->owner;
      }
    }
  }
  Fwg::Utils::Logging::logLine(
      "Distributing Countries::Evaluating Populations");
  for (auto &country : countries) {
    country.second->evaluatePopulations(civData.worldPopulationFactorSum);
    country.second->gatherCultureShares();
  }
  Fwg::Utils::Logging::logLine("Distributing Countries::Visualising Countries");
  visualiseCountries(countryMap);
  Fwg::Gfx::Png::save(countryMap,
                      Fwg::Cfg::Values().mapsPath + "countries.png");
}

void Generator::evaluateCountryNeighbours() {
  Logging::logLine("Evaluating Country Neighbours");
  Fwg::Areas::Regions::evaluateRegionNeighbours(areaData.regions);

  for (auto &c : countries) {
    for (const auto &gR : c.second->ownedRegions) {
      if (gR->neighbours.size() != areaData.regions[gR->ID].neighbours.size())
        throw(std::exception("Fatal: Neighbour count mismatch, terminating"));
      // now compare if all IDs in those neighbour vectors match
      for (int i = 0; i < gR->neighbours.size(); i++) {
        if (gR->neighbours[i] != areaData.regions[gR->ID].neighbours[i])
          throw(std::exception("Fatal: Neighbour mismatch, terminating"));
      }

      for (const auto &neighbourRegion : gR->neighbours) {
        // TO DO: Investigate rare crash issue with index being out of range
        if (gameRegions[neighbourRegion]->owner == nullptr)
          continue;
        if (neighbourRegion < gameRegions.size() &&
            gameRegions[neighbourRegion]->owner->tag != c.second->tag) {
          c.second->neighbours.insert(gameRegions[neighbourRegion]->owner);
        }
      }
    }
  }
}
void Generator::totalResourceVal(
    const std::vector<float> &resPrev, float resourceModifier,
    const Scenario::Utils::ResConfig &resourceConfig) {
  const auto baseResourceAmount = resourceModifier;
  auto totalRes = 0.0;
  for (auto &val : resPrev) {
    totalRes += val;
  }
  for (auto &reg : gameRegions) {
    auto resShare = 0.0;
    for (const auto &prov : reg->provinces) {
      for (const auto &pix : prov->pixels) {
        resShare += resPrev[pix];
      }
    }
    // basically fictive value from given input of how often this resource
    // appears
    auto stateRes = baseResourceAmount * (resShare / totalRes);
    // round to whole number
    stateRes = std::round(stateRes);
    reg->resources.insert(
        {resourceConfig.name,
         {resourceConfig.name, resourceConfig.capped, stateRes}});
  }
}
void Generator::evaluateCountries() {}
void Generator::generateCountrySpecifics() {};
void Generator::printStatistics() {
  Logging::logLine("Printing Statistics");
  std::map<std::string, int> countryPop;
  for (auto &c : countries) {
    countryPop[c.first] = 0;
    for (auto &gR : c.second->ownedRegions) {
      countryPop[c.first] += gR->totalPopulation;
    }
  }
  for (auto &c : countries) {
    Logging::logLine("Country: ", c.first,
                     " Population: ", countryPop[c.first]);
  }
}
void Generator::writeTextFiles() {}
void Generator::writeLocalisation() {}
void Generator::writeImages() {}
} // namespace Scenario