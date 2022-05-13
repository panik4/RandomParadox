#include "generic/ScenarioGenerator.h"

ScenarioGenerator::ScenarioGenerator(FastWorldGenerator &f) : f(f) {
  gamePaths["hoi4"] = "D:\\Steam\\steamapps\\common\\Hearts of Iron IV\\";
  Flag::readColourGroups();
  Flag::readFlagTypes();
  Flag::readFlagTemplates();
  Flag::readSymbolTemplates();
}

ScenarioGenerator::~ScenarioGenerator() {}

void ScenarioGenerator::loadRequiredResources(std::string gamePath) {
  bitmaps["provinces"] = rLoader.loadProvinceMap(gamePath);
  bitmaps["heightmap"] = rLoader.loadHeightMap(gamePath);
  Bitmap::bufferBitmap("provinces", bitmaps["provinces"]);
}

void ScenarioGenerator::hoi4Preparations(bool useDefaultStates,
                                         bool useDefaultProvinces) {
  loadRequiredResources(gamePaths["hoi4"]);
  auto heightMap = bitmaps["heightmap"].get24BitRepresentation();
  Bitmap::bufferBitmap("heightmap", heightMap);
  Env::Instance().width = bitmaps["heightmap"].bInfoHeader.biWidth;
  Env::Instance().height = bitmaps["heightmap"].bInfoHeader.biHeight;
  Env::Instance().bitmapSize = Env::Instance().width * Env::Instance().height;
  Env::Instance().seaLevel = 94; // hardcoded for hoi4
  Env::Instance().mapsPath = "Maps//";

  Bitmap terrainBMP = Bitmap(Env::Instance().width, Env::Instance().height, 24);
  TerrainGenerator tG;

  std::map<int, Province *> provinces;
  if (useDefaultProvinces) {
    // get province map
    auto &provinceMap = bitmaps["provinces"];
    // write info to Data that is needed by FastWorldGenerator

    // now get info on provinces: who neighbours who, who is coastal...
    auto provinceDefinition = rLoader.loadDefinition(gamePaths["hoi4"]);
    provinceDefinition.erase(provinceDefinition.begin());
    const std::set<int> tokensToConvert{0, 1, 2, 3, 7};
    for (const auto &def : provinceDefinition) {
      auto numbers = ParserUtils::getNumbers(def, ';', tokensToConvert);
      if (!numbers.size() || numbers[0] == 0)
        continue;
      Colour colour{(unsigned char)numbers[1], (unsigned char)numbers[2],
                    (unsigned char)numbers[3]};
      Province *province = new Province();
      province->isLake = false;
      if (def.find("sea") != std::string::npos)
        province->sea = true;
      else if (def.find("lake") != std::string::npos) {
        province->isLake = true;
      } else
        province->sea = false;
      f.provinceGenerator.provinceMap.setValue(colour, province);
      f.provinceGenerator.provinceMap[colour]->ID = numbers[0] - 1;
      province->colour = colour;
      provinces[province->ID] = province;
      f.provinceGenerator.provinces.push_back(province);
    }
    f.provinceGenerator.provPixels(provinceMap);
    f.provinceGenerator.evaluateNeighbours(provinceMap);
    for (auto prov : f.provinceGenerator.provinces)
      prov->position.calcWeightedCenter(prov->pixels);
  } else {
    Env::Instance().calcParameters();
    Bitmap riverMap(Env::Instance().width, Env::Instance().height, 24);
    Bitmap humidityBMP(Env::Instance().width, Env::Instance().height, 24);
    Bitmap climateMap(Env::Instance().width, Env::Instance().height, 24);
    tG.createTerrain(terrainBMP, heightMap);
    ClimateGenerator climateGenerator;
    climateGenerator.humidityMap(f.provinceGenerator.provinces, heightMap,
                                 humidityBMP, riverMap,
                                 Env::Instance().seaLevel);
    Bitmap::SaveBMPToFile(
        humidityBMP, (Env::Instance().mapsPath + ("humidity.bmp")).c_str());
    climateGenerator.climateMap(climateMap, humidityBMP, heightMap,
                                Env::Instance().seaLevel);
    Bitmap::SaveBMPToFile(climateMap,
                          (Env::Instance().mapsPath + ("climate.bmp")).c_str());

    Bitmap::SaveBMPToFile(terrainBMP,
                          (Env::Instance().mapsPath + ("terrain.bmp")).c_str());
    Bitmap provinceMap(Env::Instance().width, Env::Instance().height, 24);
    f.provinceGenerator.generateProvinces(terrainBMP, provinceMap, riverMap,
                                          tG.landBodies);
    Bitmap::SaveBMPToFile(
        provinceMap, (Env::Instance().mapsPath + ("provinces.bmp")).c_str());
    bitmaps["provinces"] = provinceMap;
    f.provinceGenerator.createProvinceMap();
    f.provinceGenerator.beautifyProvinces(provinceMap, riverMap);
    f.provinceGenerator.evaluateNeighbours(provinceMap);
    tG.detectContinents(terrainBMP);
    f.provinceGenerator.generateRegions(3);
    f.provinceGenerator.evaluateContinents(Env::Instance().width,
                                           Env::Instance().height,
                                           tG.continents, tG.landBodies);
    // genericParser.writeAdjacency((Env::Instance().debugMapsPath +
    // ("adjacency.csv")).c_str(), provinceGenerator.provinces);
    // genericParser.writeDefinition((Env::Instance().debugMapsPath +
    // ("definition.csv")).c_str(), provinceGenerator.provinces);
    Visualizer::provinceInfoMapNeighbours(provinceMap,
                                          f.provinceGenerator.provinces);
    Visualizer::provinceInfoMapCoasts(provinceMap,
                                      f.provinceGenerator.provinces);
    Visualizer::provinceInfoMapBorders(provinceMap,
                                       f.provinceGenerator.provinces);
  }
  if (useDefaultStates) {
    auto textRegions = rLoader.loadStates(gamePaths["hoi4"]);
    for (const auto &textRegion : textRegions) {
      Region R;
      auto ID = ParserUtils::getLineValue(textRegion, "id", "=");
      R.ID = stoi(ID) - 1;
      // R.provinces.push_back(f.provinceGenerator.provinceMap[colour]);
      auto stateProvinces =
          ParserUtils::getNumberBlock(textRegion, "provinces");
      for (auto stateProvince : stateProvinces) {
        stateProvince -= 1; // we count from 0, the game starts at 1
        provinces[stateProvince]->regionID = R.ID;
        if (provinces[stateProvince]->sea)
          R.sea = true;
        R.provinces.push_back(provinces[stateProvince]);
      }
      f.provinceGenerator.regions.push_back(R);
    }
  } else {
    // evaluate landmasses

    // tG.detectContinents(terrainBMP);
    f.provinceGenerator.generateRegions(3);
  }

  const auto &provinceMap = bitmaps["provinces"];
  f.provinceGenerator.sortRegions();
  f.provinceGenerator.evaluateRegionNeighbours();
  Visualizer::provinceInfoMapBorders(provinceMap,
                                     f.provinceGenerator.provinces);
  Visualizer::provinceInfoMapClassification(provinceMap,
                                            f.provinceGenerator.provinces);
}

void ScenarioGenerator::generateWorld() {
  mapTerrain();
  generatePopulations();
  generateDevelopment();
}

void ScenarioGenerator::mapContinents() {
  Logger::logLine("Mapping Continents");
  for (const auto &continent : f.provinceGenerator.continents) {
    // we copy the fwg continents by choice, to leave them untouched
    pdoxContinents.push_back(PdoxContinent(continent));
  }
}

void ScenarioGenerator::mapRegions() {
  Logger::logLine("Mapping Regions");
  for (auto &region : f.provinceGenerator.regions) {
    std::sort(region.provinces.begin(), region.provinces.end(),
              [](const Province *a, const Province *b) { return (*a < *b); });
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
  if (gameProvinces.size() != f.provinceGenerator.provinces.size())
    throw(std::exception("Fatal: Lost provinces, terminating"));
  if (gameRegions.size() != f.provinceGenerator.regions.size())
    throw(std::exception("Fatal: Lost regions, terminating"));
  // sort by gameprovince ID
  std::sort(gameProvinces.begin(), gameProvinces.end());
}

void ScenarioGenerator::generatePopulations() {
  Logger::logLine("Generating Population");
  const auto &popMap = Bitmap::findBitmapByKey("population");
  const auto &cityMap = Bitmap::findBitmapByKey("cities");
  for (auto &c : countries)
    for (auto &gR : c.second.ownedRegions)
      for (auto &gProv : gameRegions[gR].gameProvinces) {
        // calculate the population factor
        gProv.popFactor =
            0.1 + popMap.getColourAtIndex(
                      gProv.baseProvince->position.weightedCenter) /
                      Env::Instance().namedColours["population"];
        int cityPixels = 0;
        // calculate share of province that is a city
        for (auto pix : gProv.baseProvince->pixels)
          if (cityMap.getColourAtIndex(pix).isShadeOf(
                  Env::Instance().namedColours["cities"]))
            cityPixels++;
        gProv.cityShare =
            (double)cityPixels / gProv.baseProvince->pixels.size();
      }
}

void ScenarioGenerator::generateDevelopment() {
  // high pop-> high development
  // high city share->high dev
  // terrain type?
  // .....
  Logger::logLine("Generating State Development");
  const auto &cityBMP = Bitmap::findBitmapByKey("cities");
  for (auto &c : countries)
    for (auto &gR : c.second.ownedRegions)
      for (auto &gameProv : gameRegions[gR].gameProvinces) {
        auto cityDensity = 0.0;
        // calculate development with density of city and population factor
        if (gameProv.baseProvince->cityPixels.size())
          cityDensity =
              cityBMP.getColourAtIndex(gameProv.baseProvince->cityPixels[0]) /
              Env::Instance().namedColours["cities"];
        gameProv.devFactor =
            std::clamp(0.2 + 0.5 * gameProv.popFactor +
                           1.0 * gameProv.cityShare * cityDensity,
                       0.0, 1.0);
      }
}

void ScenarioGenerator::mapTerrain() {
  const auto &namedColours = Env::Instance().namedColours;
  const auto &climateMap = Bitmap::findBitmapByKey("climate");
  Bitmap typeMap(climateMap.bInfoHeader.biWidth,
                 climateMap.bInfoHeader.biHeight, 24);
  Logger::logLine("Mapping Terrain");
  std::vector<std::string> targetTypes{"plains",   "forest", "marsh", "hills",
                                       "mountain", "desert", "urban", "jungle"};

  for (auto &c : countries)
    for (auto &gameRegion : c.second.ownedRegions)
      for (auto &gameProv : gameRegions[gameRegion].gameProvinces) {
        std::map<Colour, int> colourPrevalence;
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
            typeMap.setColourAtIndex(pix, Colour{255, 255, 0});
          else if (pr->first == namedColours.at("forest"))
            typeMap.setColourAtIndex(pix, Colour{0, 255, 0});
          else if (pr->first == namedColours.at("hills"))
            typeMap.setColourAtIndex(pix, Colour{128, 128, 128});
          else if (pr->first == namedColours.at("mountains") ||
                   pr->first == namedColours.at("peaks"))
            typeMap.setColourAtIndex(pix, Colour{255, 255, 255});
          else if (pr->first == namedColours.at("grassland") ||
                   pr->first == namedColours.at("savannah"))
            typeMap.setColourAtIndex(pix, Colour{0, 255, 128});
          else if (pr->first == namedColours.at("desert"))
            typeMap.setColourAtIndex(pix, Colour{0, 255, 255});
          else
            typeMap.setColourAtIndex(pix, Colour{255, 0, 0});
        }
      }
  Bitmap::SaveBMPToFile(typeMap, "Maps/typeMap.bmp");
}

GameRegion &ScenarioGenerator::findStartRegion() {
  std::vector<GameRegion> freeRegions;
  for (const auto &gameRegion : gameRegions)
    if (!gameRegion.assigned && !gameRegion.sea)
      freeRegions.push_back(gameRegion);

  if (freeRegions.size() == 0)
    return gameRegions[0];

  const auto &startRegion = UtilLib::selectRandom(freeRegions);
  return gameRegions[startRegion.ID];
}

// generate countries according to given ruleset for each game
// TODO: rulesets, e.g. naming schemes? tags? country size?
void ScenarioGenerator::generateCountries(int numCountries) {
  this->numCountries = numCountries;
  Logger::logLine("Generating Countries");
  // load tags from hoi4 that are used by the base game
  // do not use those to avoid conflicts
  const auto forbiddenTags = rLoader.loadForbiddenTags(gamePaths["hoi4"]);
  for (const auto &tag : forbiddenTags)
    tags.insert(tag);

  for (int i = 0; i < numCountries; i++) {
    // Get Name
    auto name = nG.generateName();
    // Tag from Name
    ;
    PdoxCountry pdoxC(nG.generateTag(name, tags), i, name,
                      nG.generateAdjective(name), Flag(82, 52));
    // randomly set development of countries
    pdoxC.developmentFactor = Env::Instance().getRandomDouble(0.1, 1.0);
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
      auto &x = UtilLib::getNearestAssignedLand(gameRegions, gameRegion,
                                                Env::Instance().width,
                                                Env::Instance().height);
      countries.at(x.owner).addRegion(gameRegion, gameRegions, gameProvinces);
    }
  }
}

void ScenarioGenerator::evaluateNeighbours() {
  Logger::logLine("Evaluating Country Neighbours");
  for (auto &c : countries)
    for (const auto &gR : c.second.ownedRegions)
      for (const auto &neighbourRegion : gameRegions[gR].neighbours)
        if (gameRegions[neighbourRegion].owner != c.first)
          c.second.neighbours.insert(gameRegions[neighbourRegion].owner);
}

void ScenarioGenerator::dumpDebugCountrymap(std::string path) {
  Logger::logLine("Mapping Continents");
  Bitmap countryBMP(Env::Instance().width, Env::Instance().height, 24);
  for (const auto &country : countries)
    for (const auto &region : country.second.ownedRegions)
      for (const auto &prov : gameRegions[region].provinces)
        for (const auto &pix : prov->pixels)
          countryBMP.setColourAtIndex(pix, country.second.colour);

  Bitmap::bufferBitmap("countries", countryBMP);
  Bitmap::SaveBMPToFile(countryBMP, (path).c_str());
}
