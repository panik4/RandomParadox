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
}

Generator::~Generator() {}

void Generator::loadRequiredResources(const std::string &gamePath) {}

void Generator::generateWorldCivilizations() {
  typeMap = mapTerrain();
  generatePopulations();
  generateDevelopment();
  generateReligions();
  generateCultures();
  for (auto &region : gameRegions) {
    region->sumPopulations();
  }
}

void Generator::mapContinents() {
  Logging::logLine("Mapping Continents");
  scenContinents.clear();
  for (const auto &continent : this->areas.continents) {
    // we copy the fwg continents by choice, to leave them untouched
    scenContinents.push_back(ScenarioContinent(continent));
  }
}

void Generator::mapRegions() {
  Logging::logLine("Mapping Regions");
  gameRegions.clear();

  for (auto &region : this->areas.regions) {
    std::sort(region.provinces.begin(), region.provinces.end(),
              [](const Fwg::Province *a, const Fwg::Province *b) {
                return (*a < *b);
              });
    auto gameRegion = std::make_shared<Region>(region);
    // generate random name for region
    gameRegion->name = NameGeneration::generateName(nData);

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
  if (gameProvinces.size() != this->areas.provinces.size())
    throw(std::exception("Fatal: Lost provinces, terminating"));
  if (gameRegions.size() != this->areas.regions.size())
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
}

void Generator::mapProvinces() {
  gameProvinces.clear();
  for (auto &prov : this->areas.provinces) {
    // edit coastal status: lakes are not coasts!
    if (prov->coastal && prov->isLake)
      prov->coastal = false;
    // if it is a land province, check that a neighbour is an ocean, otherwise
    // this isn't coastal in this scenario definition
    else if (prov->coastal) {
      bool foundTrueCoast = false;
      for (auto &neighbour : prov->neighbours) {
        if (neighbour->sea) {
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
    // give name to province
    gP->name = NameGeneration::generateName(nData);
    gameProvinces.push_back(gP);
  }
  // sort by gameprovince ID
  std::sort(gameProvinces.begin(), gameProvinces.end(),
            [](auto l, auto r) { return *l < *r; });
}

void Generator::generatePopulations() {
  Logging::logLine("Generating Population");
  for (auto &gR : gameRegions) {
    gR->populationFactor = 0.0;
    for (auto &gProv : gR->gameProvinces) {
      // calculate the population factor
      gProv->popFactor = gProv->baseProvince->populationDensity;
      gR->populationFactor += gProv->popFactor;
    }
  }
}
void Generator::generateDevelopment() {
  Logging::logLine("Generating State Development");
  auto &config = Fwg::Cfg::Values();
  Bitmap development(config.width, config.height, 24);
  for (auto &region : gameRegions) {
    region->development = 0.0;
    for (auto &gameProv : region->gameProvinces) {
      gameProv->devFactor = gameProv->baseProvince->development;
      // weigh more populated provinces more
      region->development += gameProv->devFactor *
                             (gameProv->popFactor * region->populationFactor);
      for (auto pix : gameProv->baseProvince->pixels) {
        development.setColourAtIndex(
            pix, static_cast<unsigned char>(gameProv->devFactor * 255.0));
      }
    }
  }
  // this initializes pop and dev factors for countries
  for (auto &cEntry : countries) {
    auto &c = cEntry.second;
    for (auto &state : c->ownedRegions) {
      // to count total pop
      c->populationFactor += state->populationFactor;
    }
    for (auto &state : c->ownedRegions) {
      // development should be weighed by the pop in the state
      c->developmentFactor +=
          state->development *
          ((double)state->populationFactor / (double)c->populationFactor);
    }
  }
  if (config.debugLevel > 5)
    Png::save(development, config.mapsPath + "world/developmentScenario.png");
}
void Generator::generateReligions() {
  auto &config = Fwg::Cfg::Values();
  religions.clear();
  Bitmap religionMap(config.width, config.height, 24);
  for (int i = 0; i < 8; i++) {
    Religion r;
    r.name = NameGeneration::generateName(this->nData);
    do {
      r.centerOfReligion = Fwg::Utils::selectRandom(gameProvinces)->ID;
    } while (!gameProvinces[r.centerOfReligion]->baseProvince->isLand());
    r.colour.randomize();
    religions.push_back(std::make_shared<Religion>(r));
  }

  for (auto &gameProvince : gameProvinces) {
    if (!gameProvince->baseProvince->isLand())
      continue;
    auto closestReligion = 0;
    auto distance = 100000000.0;
    for (auto x = 0; x < religions.size(); x++) {
      auto &religion = religions[x];
      auto religionCenter = gameProvinces[religion->centerOfReligion];
      auto nDistance = Fwg::getPositionDistance(
          religionCenter->baseProvince->position,
          gameProvince->baseProvince->position, config.width);
      if (Fwg::Utils::switchIfComparator(nDistance, distance, std::less())) {
        closestReligion = x;
      }
    }
    // add only the main religion at this time
    gameProvince->religions[religions[closestReligion]] = 1.0;
    for (auto pix : gameProvince->baseProvince->pixels) {
      religionMap.setColourAtIndex(pix, religions[closestReligion]->colour);
    }
  }
  Png::save(religionMap, "Maps/world/religions.png");
}

void Generator::generateCultures() {
  cultures.clear();
  auto &config = Fwg::Cfg::Values();
  Bitmap cultureMap(config.width, config.height, 24);
  for (int i = 0; i < 200; i++) {
    Culture r;
    r.name = NameGeneration::generateName(this->nData);
    do {
      r.centerOfCulture = Fwg::Utils::selectRandom(gameProvinces)->ID;
    } while (!gameProvinces[r.centerOfCulture]->baseProvince->isLand());
    auto presentReligions = gameProvinces[r.centerOfCulture]->religions;

    using pair_type = decltype(presentReligions)::value_type;

    auto pr = std::max_element(std::begin(presentReligions),
                               std::end(presentReligions),
                               [](const pair_type &p1, const pair_type &p2) {
                                 return p1.second < p2.second;
                               });

    r.primaryReligion = pr->first;

    r.colour.randomize();
    cultures.push_back(std::make_shared<Culture>(r));
  }

  for (auto &gameProvince : gameProvinces) {
    if (gameProvince->baseProvince->sea || gameProvince->baseProvince->isLake)
      continue;
    auto closestCulture = 0;
    auto distance = 100000000.0;
    for (auto x = 0; x < cultures.size(); x++) {
      auto &culture = cultures[x];
      auto cultureCenter = gameProvinces[culture->centerOfCulture];
      auto nDistance = Fwg::getPositionDistance(
          cultureCenter->baseProvince->position,
          gameProvince->baseProvince->position, config.width);
      if (Fwg::Utils::switchIfComparator(nDistance, distance, std::less())) {
        closestCulture = x;
      }
    }
    // add only the main culture at this time
    gameProvince->cultures[cultures[closestCulture]] = 1.0;
    for (auto pix : gameProvince->baseProvince->pixels) {
      cultureMap.setColourAtIndex(pix, cultures[closestCulture]->colour);
    }
  }
  Png::save(cultureMap, "Maps/world/cultures.png");
}
void Generator::cutFromFiles(const std::string &gamePath) {
  Fwg::Utils::Logging::logLine("Unimplemented cutting");
}
// initialize states
void Generator::initializeStates() {}
// initialize states
void Generator::initializeCountries() {}

Fwg::Gfx::Bitmap Generator::mapTerrain() {
  const auto &climateMap = this->climateMap;
  Bitmap typeMap(climateMap.width(), climateMap.height(), 24);
  auto &colours = Fwg::Cfg::Values().colours;
  typeMap.fill(colours.at("sea"));
  Logging::logLine("Mapping Terrain");
  for (auto &gameRegion : gameRegions) {
    for (auto &gameProv : gameRegion->gameProvinces) {
      gameProv->terrainType =
          terrainTypeToString.at(gameProv->baseProvince->terrainType);
      if (climateMap.imageData.size()) {
        auto tType = gameProv->terrainType;
        for (auto pix : gameProv->baseProvince->pixels) {
          if (tType == "jungle")
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{255, 255, 0});
          else if (tType == "forest")
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{0, 255, 0});
          else if (tType == "hills")
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{128, 128, 128});
          else if (tType == "plains")
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{0, 255, 128});
          else if (tType == "marsh")
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{0, 100, 128});
          else if (tType == "desert")
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{0, 255, 255});
          else if (tType == "mountain")
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{255, 255, 255});
          else if (tType == "snowyPeaks")
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{255, 255, 255});
          else if (tType == "ice")
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{255, 255, 255});
          else if (tType == "lakes")
            typeMap.setColourAtIndex(pix, colours.at("lake"));
          else if (tType == "urban")
            typeMap.setColourAtIndex(pix, colours.at("urban"));
          else
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour{255, 0, 0});
        }
      }
    }
  }
  Png::save(typeMap, "Maps/typeMap.png");
  return typeMap;
}

std::shared_ptr<Region> &Generator::findStartRegion() {
  std::vector<std::shared_ptr<Region>> freeRegions;
  for (const auto &gameRegion : gameRegions)
    if (!gameRegion->assigned && !gameRegion->sea)
      freeRegions.push_back(gameRegion);

  if (freeRegions.size() == 0)
    return gameRegions[0];

  const auto &startRegion = Fwg::Utils::selectRandom(freeRegions);
  return gameRegions[startRegion->ID];
}

void Generator::loadCountries(const std::string &countryMapPath,
                              const std::string &mappingPath) {
  int counter = 0;
  std::vector<std::string> mappingFileLines;
  Fwg::Utils::ColourTMap<std::vector<std::string>> mapOfCountries;
  try {
    mappingFileLines = Fwg::Parsing::getLines(mappingPath);
    for (auto &line : mappingFileLines) {
      auto tokens = Fwg::Parsing::getTokens(line, ';');
      auto colour = Fwg::Gfx::Colour(std::stoi(tokens[0]), std::stoi(tokens[1]),
                                     std::stoi(tokens[2]));
      mapOfCountries.setValue(colour, tokens);
    }
  } catch (std::exception e) {
    Fwg::Utils::Logging::logLine(
        "Exception while parsing country input, ", e.what(),
        " continuing with randomly generated countries");
  }
  countryMap =
      Fwg::IO::Reader::loadAnySupported(countryMapPath, Fwg::Cfg::Values());
  Fwg::Utils::ColourTMap<std::vector<std::shared_ptr<Scenario::Region>>>
      mapOfRegions;
  for (auto &region : gameRegions) {
    if (region->sea)
      continue;

    Fwg::Utils::ColourTMap<int> likeliestOwner;
    Fwg::Gfx::Colour selectedCol;

    for (auto province : region->gameProvinces) {
      if (!province->baseProvince->sea) {
        //  we have the colour already
        auto colour = countryMap[province->baseProvince->pixels[0]];

        if (likeliestOwner.find(colour)) {
          likeliestOwner[colour] += province->baseProvince->pixels.size();

        } else {
          likeliestOwner.setValue(colour,
                                  province->baseProvince->pixels.size());
        }
        int max = 0;

        for (auto &potOwner : likeliestOwner.getMap()) {
          if (potOwner.second > max) {
            max = potOwner.second;
            selectedCol = likeliestOwner.getKeyColour(potOwner.first);
          }
        }
      }
    }
    if (mapOfRegions.find(selectedCol)) {
      mapOfRegions[selectedCol].push_back(region);
    } else {
      mapOfRegions.setValue(selectedCol, {region});
    }
  }
  for (auto &entry : mapOfRegions.getMap()) {
    auto entryCol = mapOfRegions.getKeyColour(entry.first);
    if (mapOfCountries.find(entryCol)) {
      auto tokens = mapOfCountries[entryCol];
      auto colour = Fwg::Gfx::Colour(std::stoi(tokens[0]), std::stoi(tokens[1]),
                                     std::stoi(tokens[2]));
      Country pdoxC(tokens[3], counter++, tokens[4], tokens[5],
                    Gfx::Flag(82, 52));
      pdoxC.colour = colour;
      for (auto &region : entry.second) {
        pdoxC.addRegion(region);
      }
      countries.emplace(pdoxC.tag, std::make_shared<Country>(pdoxC));
      nData.tags.insert(pdoxC.tag);
    } else {

      auto name{NameGeneration::generateName(nData)};
      Country pdoxC(NameGeneration::generateTag(name, nData), counter++, name,
                    NameGeneration::generateAdjective(name, nData),
                    Gfx::Flag(82, 52));
      pdoxC.colour = mapOfRegions.getKeyColour(entry.first);
      for (auto &region : entry.second) {
        pdoxC.addRegion(region);
      }
      countries.emplace(pdoxC.tag, std::make_shared<Country>(pdoxC));
      nData.tags.insert(pdoxC.tag);
    }
  }
}

// generate countries according to given ruleset for each game
// TODO: rulesets, e.g. naming schemes? tags? country size?

void Generator::generateStrategicRegions() {
  Fwg::Utils::Logging::logLine(
      "Scenario: Dividing world into strategic regions");
  strategicRegions.clear();
  std::set<int> assignedIDs;
  auto idcounter = 0;
  for (auto &region : gameRegions) {
    if (assignedIDs.find(region->ID) == assignedIDs.end()) {
      StrategicRegion stratRegion;
      stratRegion.ID = idcounter++;
      stratRegion.addRegion(region);
      assignedIDs.insert(region->ID);
      for (auto &neighbourID : region->neighbours) {
        // should be equal in sea/land
        if (neighbourID > gameRegions.size())
          continue;
        auto neighbourRegion = gameRegions[neighbourID];
        if (gameRegions[neighbourID]->sea == region->sea &&
            assignedIDs.find(neighbourID) == assignedIDs.end()) {
          stratRegion.addRegion(neighbourRegion);
          assignedIDs.insert(neighbourID);
        }
      }
      stratRegion.name = NameGeneration::generateName(nData);
      Colour c{static_cast<unsigned char>(RandNum::getRandom(255)),
               static_cast<unsigned char>(RandNum::getRandom(255)),
               static_cast<unsigned char>(region->sea ? 255 : 0)};
      stratRegion.colour = c;
      strategicRegions.push_back(stratRegion);
    }
  }
}

Fwg::Gfx::Bitmap Generator::visualiseStrategicRegions() {
  Bitmap stratRegionBMP(Fwg::Cfg::Values().width, Fwg::Cfg::Values().height,
                        24);
  for (auto &strat : strategicRegions) {
    for (auto &reg : strat.gameRegions) {
      for (auto &prov : reg->gameProvinces) {
        for (auto &pix : prov->baseProvince->pixels) {
          stratRegionBMP.setColourAtIndex(pix, reg->colour);
        }
      }
      for (auto &pix : reg->borderPixels) {
        stratRegionBMP.setColourAtIndex(pix, strat.colour * 0.5);
      }
    }
  }
  Bmp::save(stratRegionBMP, Fwg::Cfg::Values().mapsPath + "stratRegions.bmp");
  return stratRegionBMP;
}

void Generator::evaluateNeighbours() {
  Logging::logLine("Evaluating Country Neighbours");
  for (auto &c : countries)
    for (const auto &gR : c.second->ownedRegions)
      for (const auto &neighbourRegion : gR->neighbours)
        // TO DO: Investigate rare crash issue with index being out of range
        if (neighbourRegion < gameRegions.size() &&
            gameRegions[neighbourRegion]->owner != c.first)
          c.second->neighbours.insert(gameRegions[neighbourRegion]->owner);
}

Bitmap Generator::dumpDebugCountrymap(const std::string &path,
                                      Fwg::Gfx::Bitmap &countryBmp,
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
      if (region->owner.size()) {
        countryColour = countries.at(region->owner)->colour;
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
    for (const auto &region : gameRegions) {
      auto countryColour = Fwg::Gfx::Colour(0, 0, 0);
      // if this tag is assigned, use the colour
      if (region->owner.size()) {
        countryColour = countries.at(region->owner)->colour;
      }
      for (const auto &prov : region->provinces) {
        for (const auto &pix : prov->pixels) {
          countryBmp.setColourAtIndex(pix,
                                      countryColour * 0.9 + prov->colour * 0.1);
        }
      }
      for (auto &pix : region->borderPixels) {
        countryBmp.setColourAtIndex(pix, countryColour * 0.0);
      }
    }
  }
  return countryBmp;
}
} // namespace Scenario