#include "generic/ScenarioGenerator.h"
namespace Scenario {
Generator::Generator(Fwg::FastWorldGenerator &fwg) : fwg(fwg) {
  gamePaths["hoi4"] = "D:\\Steam\\steamapps\\common\\Hearts of Iron IV\\";

  Graphics::Flag::readColourGroups();
  Graphics::Flag::readFlagTypes();
  Graphics::Flag::readFlagTemplates();
  Graphics::Flag::readSymbolTemplates();
}

Generator::~Generator() {}

void Generator::loadRequiredResources(std::string gamePath) {
  bitmaps["provinces"] = ResourceLoading::loadProvinceMap(gamePath);
  bitmaps["heightmap"] = ResourceLoading::loadHeightMap(gamePath);
  Fwg::Bitmap::bufferBitmap("provinces", bitmaps["provinces"]);
}

void Generator::hoi4Preparations(bool useDefaultStates,
                                 bool useDefaultProvinces) {
  loadRequiredResources(gamePaths["hoi4"]);
  auto heightMap = bitmaps["heightmap"].get24BitRepresentation();
  auto &config = Fwg::Env::Instance();
  Fwg::Bitmap::bufferBitmap("heightmap", heightMap);
  config.width = bitmaps["heightmap"].bInfoHeader.biWidth;
  config.height = bitmaps["heightmap"].bInfoHeader.biHeight;
  config.bitmapSize = config.width * config.height;
  config.seaLevel = 94; // hardcoded for hoi4
  config.mapsPath = "Maps//";

  Fwg::Bitmap terrainBMP = Fwg::Bitmap(config.width, config.height, 24);
  Fwg::TerrainGenerator tG;

  std::map<int, Fwg::Province *> provinces;
  if (useDefaultProvinces) {
    // get province map
    auto &provinceMap = bitmaps["provinces"];
    // write info to Data that is needed by FastWorldGenerator

    // now get info on provinces: who neighbours who, who is coastal...
    auto provinceDefinition =
        ResourceLoading::loadDefinition(gamePaths["hoi4"]);
    provinceDefinition.erase(provinceDefinition.begin());
    const std::set<int> tokensToConvert{0, 1, 2, 3, 7};
    for (const auto &def : provinceDefinition) {
      auto numbers = ParserUtils::getNumbers(def, ';', tokensToConvert);
      if (!numbers.size() || numbers[0] == 0)
        continue;
      Fwg::Colour colour{(unsigned char)numbers[1], (unsigned char)numbers[2],
                         (unsigned char)numbers[3]};
      Fwg::Province *province = new Fwg::Province();
      province->isLake = false;
      if (def.find("sea") != std::string::npos)
        province->sea = true;
      else if (def.find("lake") != std::string::npos) {
        province->isLake = true;
      } else
        province->sea = false;
      fwg.provinceGenerator.provinceMap.setValue(colour, province);
      fwg.provinceGenerator.provinceMap[colour]->ID = numbers[0] - 1;
      province->colour = colour;
      provinces[province->ID] = province;
      fwg.provinceGenerator.provinces.push_back(province);
    }
    fwg.provinceGenerator.provPixels(provinceMap);
    fwg.provinceGenerator.evaluateNeighbours(provinceMap);
    for (auto prov : fwg.provinceGenerator.provinces)
      prov->position.calcWeightedCenter(prov->pixels);
  } else {
    config.calcParameters();
    Fwg::Bitmap riverMap(config.width, config.height, 24);
    Fwg::Bitmap humidityBMP(config.width, config.height, 24);
    Fwg::Bitmap climateMap(config.width, config.height, 24);
    tG.createTerrain(terrainBMP, heightMap);
    Fwg::ClimateGenerator climateGenerator;
    climateGenerator.humidityMap(fwg.provinceGenerator.provinces, heightMap,
                                 humidityBMP, riverMap, config.seaLevel);
    Fwg::Bitmap::SaveBMPToFile(humidityBMP, config.mapsPath + "humidity.bmp");
    climateGenerator.climateMap(climateMap, humidityBMP, heightMap,
                                config.seaLevel);
    Fwg::Bitmap::SaveBMPToFile(climateMap, config.mapsPath + "climate.bmp");

    Fwg::Bitmap::SaveBMPToFile(terrainBMP, config.mapsPath + "terrain.bmp");
    Fwg::Bitmap provinceMap(config.width, config.height, 24);
    fwg.provinceGenerator.generateProvinces(terrainBMP, provinceMap, riverMap,
                                            tG.landBodies);
    Fwg::Bitmap::SaveBMPToFile(provinceMap,
                               (config.mapsPath + ("provinces.bmp")).c_str());
    bitmaps["provinces"] = provinceMap;
    fwg.provinceGenerator.createProvinceMap();
    fwg.provinceGenerator.beautifyProvinces(provinceMap, riverMap);
    fwg.provinceGenerator.evaluateNeighbours(provinceMap);
    tG.detectContinents(terrainBMP);
    fwg.provinceGenerator.generateRegions(3);
    fwg.provinceGenerator.evaluateContinents(config.width, config.height,
                                             tG.continents, tG.landBodies);
    // genericParser.writeAdjacency((config.debugMapsPath +
    // ("adjacency.csv")).c_str(), provinceGenerator.provinces);
    // genericParser.writeDefinition((config.debugMapsPath +
    // ("definition.csv")).c_str(), provinceGenerator.provinces);
    using namespace Fwg::Visualize::Provinces;
    neighboursMap(provinceMap, fwg.provinceGenerator.provinces);
    coastsMap(provinceMap, fwg.provinceGenerator.provinces);
    bordersMap(provinceMap, fwg.provinceGenerator.provinces);
  }
  if (useDefaultStates) {
    auto textRegions = ResourceLoading::loadStates(gamePaths["hoi4"]);
    for (const auto &textRegion : textRegions) {
      Fwg::Region R;
      auto ID = ParserUtils::getLineValue(textRegion, "id", "=");
      R.ID = stoi(ID) - 1;
      // R.provinces.push_back(fwg.provinceGenerator.provinceMap[colour]);
      auto stateProvinces =
          ParserUtils::getNumberBlock(textRegion, "provinces");
      for (auto stateProvince : stateProvinces) {
        stateProvince -= 1; // we count from 0, the game starts at 1
        provinces[stateProvince]->regionID = R.ID;
        if (provinces[stateProvince]->sea)
          R.sea = true;
        R.provinces.push_back(provinces[stateProvince]);
      }
      fwg.provinceGenerator.regions.push_back(R);
    }
  } else {
    // evaluate landmasses

    // tG.detectContinents(terrainBMP);
    fwg.provinceGenerator.generateRegions(3);
  }

  const auto &provinceMap = bitmaps["provinces"];
  fwg.provinceGenerator.sortRegions();
  fwg.provinceGenerator.evaluateRegionNeighbours();
  using namespace Fwg::Visualize::Provinces;
  bordersMap(provinceMap, fwg.provinceGenerator.provinces);
  classificationMap(provinceMap, fwg.provinceGenerator.provinces);
}

void Generator::generateWorld() {
  mapTerrain();
  generatePopulations();
  generateDevelopment();
}

void Generator::mapContinents() {
  Logger::logLine("Mapping Continents");
  for (const auto &continent : fwg.provinceGenerator.continents) {
    // we copy the fwg continents by choice, to leave them untouched
    pdoxContinents.push_back(PdoxContinent(continent));
  }
}

void Generator::mapRegions() {
  Logger::logLine("Mapping Regions");
  for (auto &region : fwg.provinceGenerator.regions) {
    std::sort(region.provinces.begin(), region.provinces.end(),
              [](const Fwg::Province *a, const Fwg::Province *b) {
                return (*a < *b);
              });
    GameRegion gR(region);
    for (auto &baseRegion : gR.neighbours)
      gR.neighbours.push_back(baseRegion);
    // generate random name for region
    gR.name = nG.generateName();
    // now create gameprovinces from FastWorldGen provinces
    for (auto &province : gR.provinces) {
      GameProvince gP(province);
      // also copy neighbours
      for (auto &baseProvinceNeighbour : gP.baseProvince->neighbours)
        gP.neighbours.push_back(baseProvinceNeighbour);
      // give name to province
      gP.name = nG.generateName();
      gR.gameProvinces.push_back(gP);
      gameProvinces.push_back(gP);
    }
    // save game region
    gameRegions.push_back(gR);
  }
  // check if we have the same amount of gameProvinces as FastWorldGen provinces
  if (gameProvinces.size() != fwg.provinceGenerator.provinces.size())
    throw(std::exception("Fatal: Lost provinces, terminating"));
  if (gameRegions.size() != fwg.provinceGenerator.regions.size())
    throw(std::exception("Fatal: Lost regions, terminating"));
  // sort by gameprovince ID
  std::sort(gameProvinces.begin(), gameProvinces.end());
}

void Generator::generatePopulations() {
  Logger::logLine("Generating Population");
  auto &config = Fwg::Env::Instance();
  const auto &popMap = Fwg::Bitmap::findBitmapByKey("population");
  const auto &cityMap = Fwg::Bitmap::findBitmapByKey("cities");
  for (auto &c : countries)
    for (auto &gR : c.second.ownedRegions)
      for (auto &gProv : gameRegions[gR].gameProvinces) {
        // calculate the population factor
        gProv.popFactor =
            0.1 + popMap.getColourAtIndex(
                      gProv.baseProvince->position.weightedCenter) /
                      config.namedColours["population"];
        int cityPixels = 0;
        // calculate share of province that is a city
        for (auto pix : gProv.baseProvince->pixels)
          if (cityMap.getColourAtIndex(pix).isShadeOf(
                  config.namedColours["cities"]))
            cityPixels++;
        gProv.cityShare =
            (double)cityPixels / gProv.baseProvince->pixels.size();
      }
}

void Generator::generateDevelopment() {
  // high pop-> high development
  // high city share->high dev
  // terrain type?
  // .....
  Logger::logLine("Generating State Development");
  auto &config = Fwg::Env::Instance();
  const auto &cityBMP = Fwg::Bitmap::findBitmapByKey("cities");
  for (auto &c : countries)
    for (auto &gR : c.second.ownedRegions)
      for (auto &gameProv : gameRegions[gR].gameProvinces) {
        auto cityDensity = 0.0;
        // calculate development with density of city and population factor
        if (gameProv.baseProvince->cityPixels.size())
          cityDensity =
              cityBMP.getColourAtIndex(gameProv.baseProvince->cityPixels[0]) /
              config.namedColours["cities"];
        gameProv.devFactor =
            std::clamp(0.2 + 0.5 * gameProv.popFactor +
                           1.0 * gameProv.cityShare * cityDensity,
                       0.0, 1.0);
      }
}

void Generator::mapTerrain() {
  auto &config = Fwg::Env::Instance();
  const auto &namedColours = config.namedColours;
  const auto &climateMap = Fwg::Bitmap::findBitmapByKey("climate");
  Fwg::Bitmap typeMap(climateMap.bInfoHeader.biWidth,
                      climateMap.bInfoHeader.biHeight, 24);
  Logger::logLine("Mapping Terrain");
  std::vector<std::string> targetTypes{"plains",   "forest", "marsh", "hills",
                                       "mountain", "desert", "urban", "jungle"};

  for (auto &c : countries)
    for (auto &gameRegion : c.second.ownedRegions)
      for (auto &gameProv : gameRegions[gameRegion].gameProvinces) {
        std::map<Fwg::Colour, int> colourPrevalence;
        for (auto &pix : gameProv.baseProvince->pixels) {
          if (colourPrevalence[climateMap.getColourAtIndex(pix)])
            colourPrevalence[climateMap.getColourAtIndex(pix)]++;
          else
            colourPrevalence[climateMap.getColourAtIndex(pix)] = 1;
        }
        // find the most prevalent colour in province, which sets the terrain
        // type
        using pair_type = decltype(colourPrevalence)::value_type;
        auto pr = std::max_element(
            std::begin(colourPrevalence), std::end(colourPrevalence),
            [](const pair_type &p1, const pair_type &p2) {
              return p1.second < p2.second;
            });
        // now check which it is and set the terrain type
        if (pr->first == namedColours.at("jungle"))
          gameProv.terrainType = "jungle";
        else if (pr->first == namedColours.at("forest"))
          gameProv.terrainType = "forest";
        else if (pr->first == namedColours.at("hills"))
          gameProv.terrainType = "hills";
        else if (pr->first == namedColours.at("mountains") ||
                 pr->first == namedColours.at("peaks"))
          gameProv.terrainType = "mountain";
        else if (pr->first == namedColours.at("grassland") ||
                 pr->first == namedColours.at("savannah"))
          gameProv.terrainType = "plains";
        else if (pr->first == namedColours.at("desert"))
          gameProv.terrainType = "desert";
        else
          gameProv.terrainType = "plains";
        gameProvinces[gameProv.ID].terrainType = gameProv.terrainType;
        for (auto pix : gameProv.baseProvince->pixels) {
          if (pr->first == namedColours.at("jungle"))
            typeMap.setColourAtIndex(pix, Fwg::Colour{255, 255, 0});
          else if (pr->first == namedColours.at("forest"))
            typeMap.setColourAtIndex(pix, Fwg::Colour{0, 255, 0});
          else if (pr->first == namedColours.at("hills"))
            typeMap.setColourAtIndex(pix, Fwg::Colour{128, 128, 128});
          else if (pr->first == namedColours.at("mountains") ||
                   pr->first == namedColours.at("peaks"))
            typeMap.setColourAtIndex(pix, Fwg::Colour{255, 255, 255});
          else if (pr->first == namedColours.at("grassland") ||
                   pr->first == namedColours.at("savannah"))
            typeMap.setColourAtIndex(pix, Fwg::Colour{0, 255, 128});
          else if (pr->first == namedColours.at("desert"))
            typeMap.setColourAtIndex(pix, Fwg::Colour{0, 255, 255});
          else
            typeMap.setColourAtIndex(pix, Fwg::Colour{255, 0, 0});
        }
      }
  Fwg::Bitmap::SaveBMPToFile(typeMap, "Maps/typeMap.bmp");
}

GameRegion &Generator::findStartRegion() {
  std::vector<GameRegion> freeRegions;
  for (const auto &gameRegion : gameRegions)
    if (!gameRegion.assigned && !gameRegion.sea)
      freeRegions.push_back(gameRegion);

  if (freeRegions.size() == 0)
    return gameRegions[0];

  const auto &startRegion = Fwg::Utils::selectRandom(freeRegions);
  return gameRegions[startRegion.ID];
}

// generate countries according to given ruleset for each game
// TODO: rulesets, e.g. naming schemes? tags? country size?
void Generator::generateCountries(int numCountries) {
  auto &config = Fwg::Env::Instance();
  this->numCountries = numCountries;
  Logger::logLine("Generating Countries");
  // load tags from hoi4 that are used by the base game
  // do not use those to avoid conflicts
  const auto forbiddenTags =
      ResourceLoading::loadForbiddenTags(gamePaths["hoi4"]);
  for (const auto &tag : forbiddenTags)
    tags.insert(tag);

  for (int i = 0; i < numCountries; i++) {
    auto name{nG.generateName()};
    PdoxCountry pdoxC(nG.generateTag(name, tags), i, name,
                      nG.generateAdjective(name), Graphics::Flag(82, 52));
    // randomly set development of countries
    pdoxC.developmentFactor = FastWorldGen::RandNum::getRandomDouble(0.1, 1.0);
    countries.emplace(pdoxC.tag, pdoxC);
  }
  for (auto &pdoxCountry : countries) {
    auto startRegion(findStartRegion());
    if (startRegion.assigned || startRegion.sea)
      continue;
    pdoxCountry.second.assignRegions(6, gameRegions, startRegion,
                                     gameProvinces);
  }
  for (auto &gameRegion : gameRegions) {
    if (!gameRegion.sea && !gameRegion.assigned) {
      auto &gR = Fwg::Utils::getNearestAssignedLand(
          gameRegions, gameRegion, config.width, config.height);
      countries.at(gR.owner).addRegion(gameRegion, gameRegions, gameProvinces);
    }
  }
}

void Generator::evaluateNeighbours() {
  Logger::logLine("Evaluating Country Neighbours");
  for (auto &c : countries)
    for (const auto &gR : c.second.ownedRegions)
      for (const auto &neighbourRegion : gameRegions[gR].neighbours)
        if (gameRegions[neighbourRegion].owner != c.first)
          c.second.neighbours.insert(gameRegions[neighbourRegion].owner);
}

void Generator::dumpDebugCountrymap(std::string path) {
  Logger::logLine("Mapping Continents");
  auto &config = Fwg::Env::Instance();
  Fwg::Bitmap countryBMP(config.width, config.height, 24);
  for (const auto &country : countries)
    for (const auto &region : country.second.ownedRegions)
      for (const auto &prov : gameRegions[region].provinces)
        for (const auto &pix : prov->pixels)
          countryBMP.setColourAtIndex(pix, country.second.colour);

  Fwg::Bitmap::bufferBitmap("countries", countryBMP);
  Fwg::Bitmap::SaveBMPToFile(countryBMP, (path).c_str());
}
} // namespace Scenario