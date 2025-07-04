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
                 const std::shared_ptr<Fwg::Areas::Province> b) { return (*a < *b); });
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
  Png::save(regionMap, Fwg::Cfg::Values().mapsPath + "regionMap.png", false);
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
  // now print all colours of the provinces
  for (auto &gameProv : gameProvinces) {

    Fwg::Utils::Logging::logLine(
        "GameProvince ID: " + std::to_string(gameProv->ID) +
        " Colour: " + gameProv->baseProvince->colour.toString());
  }
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
  std::map<int, int> assignedToIDs;
  // gather all non contiguous regions
  std::vector<std::shared_ptr<Region>> nonContiguousRegions;
  for (auto &region : gameRegions) {
    if (!region->isContiguous()) {
      nonContiguousRegions.push_back(region);
    }
  }
  //// assign each non contiguous region to a strategic region
  // for (auto &region : nonContiguousRegions) {
  //   StrategicRegion stratRegion;
  //   stratRegion.ID = strategicRegions.size();
  //   stratRegion.addRegion(region);
  //   assignedIDs.insert(region->ID);
  //   for (auto &neighbourID : region->neighbours) {
  //     // should be equal in sea/land
  //     if (neighbourID > gameRegions.size())
  //       continue;
  //     auto neighbourRegion = gameRegions[neighbourID];
  //     if (assignedIDs.find(neighbourID) == assignedIDs.end()) {
  //       stratRegion.addRegion(neighbourRegion);
  //       assignedIDs.insert(neighbourID);
  //     }
  //   }
  //   stratRegion.name = NameGeneration::generateName(nData);
  //   Colour c{static_cast<unsigned char>(RandNum::getRandom(255)),
  //            static_cast<unsigned char>(RandNum::getRandom(255)),
  //            static_cast<unsigned char>(region->sea ? 255 : 0)};
  //   stratRegion.colour = c;
  //   strategicRegions.push_back(stratRegion);
  // }

  // std::vector<std::shared_ptr<Region>> coastalOceanRegions;
  // for (auto region : gameRegions) {
  //   if (region->type == Region::RegionType::OceanMixedCoastal) {
  //     coastalOceanRegions.push_back(region);
  //   }
  // }
  //// sort these regions by the amount of neighbours of type
  //// Region::RegionType::OceanMixedCoastal
  // std::sort(
  //     coastalOceanRegions.begin(), coastalOceanRegions.end(),
  //     [](const std::shared_ptr<Region> &a, const std::shared_ptr<Region> &b)
  //     {
  //       return a->neighbourTypes.at(Region::RegionType::OceanMixedCoastal) >
  //              b->neighbourTypes.at(Region::RegionType::OceanMixedCoastal);
  //     });

  // int targetRegionAmount = 6;
  //// the goal: assign each region to a strategic region. Try not to mix water
  //// and land too much, preferrably only in the case of island regions. try to
  //// get most regions to a certain size, and reallocate too small regions to
  //// larger ones start
  auto idcounter = 0;
  //  for (auto coastalRegion : coastalOceanRegions) {
  //    std::cout << coastalRegion->neighbourTypes.at(
  //                     Region::RegionType::OceanMixedCoastal)
  //              << std::endl;
  //    if (assignedToIDs.contains(coastalRegion->ID))
  //      continue;
  //    StrategicRegion stratRegion;
  //    stratRegion.ID = idcounter++;
  //    stratRegion.addRegion(coastalRegion);
  //    assignedToIDs.insert({coastalRegion->ID, stratRegion.ID});
  //    for (const auto neighbourID : coastalRegion->neighbours) {
  //      auto neighbourRegion = gameRegions[neighbourID];
  //      if (gameRegions[neighbourID]->type ==
  //              Region::RegionType::OceanMixedCoastal &&
  //          assignedToIDs.find(neighbourID) == assignedToIDs.end()) {
  //        stratRegion.addRegion(neighbourRegion);
  //        assignedToIDs.insert({neighbourID, stratRegion.ID});
  //      }
  //    }
  //    Colour c{static_cast<unsigned char>(RandNum::getRandom(255)),
  //             static_cast<unsigned char>(RandNum::getRandom(255)),
  //             static_cast<unsigned char>(coastalRegion->sea ? 255 : 0)};
  //    stratRegion.colour = c;
  //    stratRegion.setType();
  //    stratRegion.name = std::to_string(stratRegion.ID + 1);
  //    strategicRegions.push_back(stratRegion);
  //  }
  //  now for all these existing strat regions, add all of their neighbours if
  //  they are some type of ocean as well

  // for (auto &stratRegion : strategicRegions) {
  //   std::cout << "0B: " << stratRegion.gameRegions.size() << std::endl;
  //   for (auto &region : stratRegion.gameRegions) {
  //     if (region == nullptr)
  //       continue;
  //     std::cout << "01: " << region->ID << std::endl;
  //     std::cout << "0A: " << region->neighbours.size() << std::endl;
  //     for (const auto neighbourID : region->neighbours) {
  //               if (neighbourID > gameRegions.size())
  //                 continue;
  //       std::cout << "A: " << neighbourID << std::endl;
  //       const auto neighbourRegion = gameRegions[neighbourID];
  //       if (assignedToIDs.find(neighbourID) == assignedToIDs.end()) {
  //         if (neighbourRegion->type == Region::RegionType::OceanCoastal ||
  //             neighbourRegion->type == Region::RegionType::Ocean ||
  //             neighbourRegion->type ==
  //             Region::RegionType::OceanIslandCoastal) {

  //          stratRegion.addRegion(neighbourRegion);
  //          assignedToIDs.insert({neighbourID, stratRegion.ID});
  //        }
  //      }
  //    }
  //  }
  //}

  for (auto &region : gameRegions) {
    if (assignedToIDs.find(region->ID) == assignedToIDs.end()) {
      StrategicRegion stratRegion;
      stratRegion.ID = idcounter++;
      stratRegion.addRegion(region);
      assignedToIDs.insert({region->ID, stratRegion.ID});
      for (auto &neighbourID : region->neighbours) {
        // should be equal in sea/land
        if (neighbourID > gameRegions.size())
          continue;
        auto neighbourRegion = gameRegions[neighbourID];
        if (gameRegions[neighbourID]->isSea() == region->isSea() &&
            assignedToIDs.find(neighbourID) == assignedToIDs.end()) {
          stratRegion.addRegion(neighbourRegion);
          assignedToIDs.insert({neighbourID, stratRegion.ID});
        }
      }
      Colour c{static_cast<unsigned char>(RandNum::getRandom(255)),
               static_cast<unsigned char>(RandNum::getRandom(255)),
               static_cast<unsigned char>(region->isSea() ? 255 : 0)};
      stratRegion.colour = c;
      stratRegion.setType();
      strategicRegions.push_back(stratRegion);
    }
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
  for (auto &stratRegion : strategicRegions) {
    stratRegion.checkPosition(superRegions);
    stratRegion.name = std::to_string(stratRegion.ID + 1);
  }
  visualiseStrategicRegions();
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
      if (!gameRegion->isSea() && !gameRegion->assigned && !gameRegion->isLake()) {
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
void Generator::generateCountrySpecifics(){};
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