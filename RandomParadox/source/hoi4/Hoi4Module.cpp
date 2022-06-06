#include "hoi4/Hoi4Module.h"

Hoi4Module::Hoi4Module() {}

Hoi4Module::~Hoi4Module() {}

bool Hoi4Module::createPaths() {
  // prepare folder structure
  try {
    // generic cleanup and path creation
    GenericModule::createPaths(gameModPath);
    // map
    std::filesystem::create_directory(gameModPath + "\\map\\supplyareas\\");
    std::filesystem::create_directory(gameModPath +
                                      "\\map\\strategicregions\\");
    // gfx
    std::filesystem::create_directory(gameModPath + "\\gfx\\flags\\small\\");
    std::filesystem::create_directory(gameModPath + "\\gfx\\flags\\medium\\");
    // history
    std::filesystem::create_directory(gameModPath + "\\history\\units\\");
    std::filesystem::create_directory(gameModPath + "\\history\\states\\");
    // localisation
    std::filesystem::create_directory(gameModPath +
                                      "\\localisation\\english\\");
    // common
    std::filesystem::create_directory(gameModPath +
                                      "\\common\\national_focus\\");
    return true;
  } catch (std::exception e) {
    std::string error =
        "Configured paths seem to be messed up, check Hoi4Module.json\n";
    error += "You can try fixing it yourself. Error is:\n ";
    error += e.what();
    throw(std::exception(error.c_str()));
    return false;
  }
}

void Hoi4Module::genHoi(bool useDefaultMap, bool useDefaultStates,
                        bool useDefaultProvinces, ScenarioGenerator &scenGen,
                        bool cut) {
  if (!createPaths())
    return;
  // validate options:
  if (!useDefaultProvinces) {
    useDefaultStates = false;
  }
  if (!useDefaultMap) {
    useDefaultProvinces = false;
    useDefaultStates = false;
  }
  if (useDefaultMap) {
    scenGen.hoi4Preparations(
        useDefaultStates,
        useDefaultProvinces); // load files, read states/create states
    scenGen.mapRegions();     // create gameRegions
    scenGen.generateCountries(numCountries);
    scenGen.dumpDebugCountrymap(Env::Instance().mapsPath + "countries.bmp");

    Hoi4Parser::dumpStates(gameModPath + "\\history\\states",
                           hoi4Gen.countries);
    Hoi4Parser::dumpUnitStacks(gameModPath + "\\map\\unitstacks.txt",
                               scenGen.fwg.provinceGenerator.provinces);
    Hoi4Parser::dumpRocketSites(gameModPath + "\\map\\rocketsites.txt",
                                scenGen.fwg.provinceGenerator.regions);
    Hoi4Parser::dumpStrategicRegions(gameModPath + "\\map\\strategicregions",
                                     scenGen.fwg.provinceGenerator.regions,
                                     hoi4Gen.strategicRegions);
    Hoi4Parser::dumpWeatherPositions(
        gameModPath + "\\map\\weatherpositions.txt",
        scenGen.fwg.provinceGenerator.regions, hoi4Gen.strategicRegions);
    Hoi4Parser::dumpAdjacencyRules(gameModPath + "\\map\\adjacency_rules.txt");
    Hoi4Parser::dumpAirports(gameModPath + "\\map\\airports.txt",
                             scenGen.fwg.provinceGenerator.regions);
    Hoi4Parser::dumpBuildings(gameModPath + "\\map\\buildings.txt",
                              scenGen.fwg.provinceGenerator.regions);
    if (!useDefaultStates) {
      Hoi4Parser::dumpDefinition(gameModPath + "\\map\\definition.csv",
                                 scenGen.gameProvinces);
    }
    Bitmap::SaveBMPToFile(Bitmap::findBitmapByKey("provinces"),
                          (gameModPath + ("\\map\\provinces.bmp")).c_str());
  } else {
    try {
      // start with the generic stuff in the Scenario Generator
      scenGen.mapRegions();
      scenGen.mapContinents();
      scenGen.generateCountries(numCountries);
      scenGen.evaluateNeighbours();
      scenGen.generateWorld();
      scenGen.dumpDebugCountrymap(Env::Instance().mapsPath + "countries.bmp");

      // now generate hoi4 specific stuff
      hoi4Gen.generateCountrySpecifics(scenGen, scenGen.countries);
      hoi4Gen.generateStateSpecifics(scenGen.gameRegions.size());
      hoi4Gen.generateStateResources();
      hoi4Gen.generateStrategicRegions(scenGen);
      hoi4Gen.generateWeather(scenGen);
      hoi4Gen.evaluateCountries(scenGen);
      hoi4Gen.generateLogistics(scenGen);
      NationalFocus::buildMaps();
      hoi4Gen.evaluateCountryGoals(scenGen);
      hoi4Gen.generateCountryUnits();
    } catch (std::exception e) {
      std::string error = "Error while generating the Hoi4 Module.\n";
      error += "Error is: \n";
      error += e.what();
      throw(std::exception(error.c_str()));
    }
    // now start writing game files
    try {
      // generate map files. Format must be converted and colours mapped to hoi4
      // compatible colours
      FormatConverter formatConverter(gamePath, "Hoi4");
      formatConverter.dump8BitTerrain(gameModPath + "\\map\\terrain.bmp",
                                      "terrain", cut);
      formatConverter.dump8BitCities(gameModPath + "\\map\\cities.bmp",
                                     "cities", cut);
      formatConverter.dump8BitRivers(gameModPath + "\\map\\rivers.bmp",
                                     "rivers", cut);
      formatConverter.dump8BitTrees(gameModPath + "\\map\\trees.bmp", "trees",
                                    false);
      formatConverter.dump8BitHeightmap(gameModPath + "\\map\\heightmap.bmp",
                                        "heightmap");
      formatConverter.dumpTerrainColourmap(
          gameModPath, "\\map\\terrain\\colormap_rgb_cityemissivemask_a.dds",
          DXGI_FORMAT_B8G8R8A8_UNORM,
          cut);
      formatConverter.dumpDDSFiles(
          gameModPath + "\\map\\terrain\\colormap_water_", cut, 8);
      formatConverter.dumpWorldNormal(gameModPath + "\\map\\world_normal.bmp",
                                      cut);

      Hoi4Parser::writeCompatibilityHistory(
          gameModPath + "\\history\\countries\\", gamePath,
          scenGen.fwg.provinceGenerator.regions);
      Hoi4Parser::writeHistoryCountries(gameModPath + "\\history\\countries\\",
                                        hoi4Gen.countries);
      Hoi4Parser::writeHistoryUnits(gameModPath + "\\history\\units\\",
                                    hoi4Gen.countries);
      Hoi4Parser::dumpCommonCountryTags(
          gameModPath + "\\common\\country_tags\\02_countries.txt",
          hoi4Gen.countries);
      Hoi4Parser::dumpCommonCountries(
          gameModPath + "\\common\\countries\\",
          gamePath + "\\common\\countries\\colors.txt", hoi4Gen.countries);
      Hoi4Parser::dumpAdj(gameModPath + "\\map\\adjacencies.csv");
      Hoi4Parser::dumpAirports(gameModPath + "\\map\\airports.txt",
                               scenGen.fwg.provinceGenerator.regions);
      Hoi4Parser::dumpBuildings(gameModPath + "\\map\\buildings.txt",
                                scenGen.fwg.provinceGenerator.regions);
      Hoi4Parser::dumpContinents(gameModPath + "\\map\\continents.txt",
                                 scenGen.fwg.provinceGenerator.continents);
      Hoi4Parser::dumpDefinition(gameModPath + "\\map\\definition.csv",
                                 scenGen.gameProvinces);
      Hoi4Parser::dumpUnitStacks(gameModPath + "\\map\\unitstacks.txt",
                                 scenGen.fwg.provinceGenerator.provinces);
      Hoi4Parser::dumpRocketSites(gameModPath + "\\map\\rocketsites.txt",
                                  scenGen.fwg.provinceGenerator.regions);
      Hoi4Parser::dumpStrategicRegions(gameModPath + "\\map\\strategicregions",
                                       scenGen.fwg.provinceGenerator.regions,
                                       hoi4Gen.strategicRegions);
      Hoi4Parser::dumpStates(gameModPath + "\\history\\states",
                             hoi4Gen.countries);
      Hoi4Parser::dumpFlags(gameModPath + "\\gfx\\flags\\", hoi4Gen.countries);
      Hoi4Parser::dumpWeatherPositions(
          gameModPath + "\\map\\weatherpositions.txt",
          scenGen.fwg.provinceGenerator.regions, hoi4Gen.strategicRegions);
      Hoi4Parser::dumpAdjacencyRules(gameModPath +
                                     "\\map\\adjacency_rules.txt");
      Hoi4Parser::dumpSupply(gameModPath + "\\map\\",
                             hoi4Gen.supplyNodeConnections);
      Hoi4Parser::writeStateNames(gameModPath + "\\localisation\\english\\",
                                  hoi4Gen.countries);
      Hoi4Parser::writeCountryNames(gameModPath + "\\localisation\\english\\",
                                    hoi4Gen.countries);
      Hoi4Parser::writeStrategicRegionNames(
          gameModPath + "\\localisation\\english\\", hoi4Gen.strategicRegions);
      Hoi4Parser::writeFoci(gameModPath + "\\common\\national_focus\\",
                            hoi4Gen.countries);
      Hoi4Parser::dumpCommonBookmarks(gameModPath + "\\common\\bookmarks\\",
                                      hoi4Gen.countries,
                                      hoi4Gen.strengthScores);
      Hoi4Parser::copyDescriptorFile("resources\\hoi4\\descriptor.mod",
                                     gameModPath, gameModsDirectory, modName);

      // just copy over provinces.bmp, already in a compatible format
      Bitmap::SaveBMPToFile(Bitmap::findBitmapByKey("provinces"),
                            (gameModPath + ("\\map\\provinces.bmp")).c_str());
    } catch (std::exception e) {
      std::string error = "Error while dumping and writing files.\n";
      error += "Error is: \n";
      error += e.what();
      throw(std::exception(error.c_str()));
    }
    // now if everything worked, print info about world and pause for user to
    // see
    hoi4Gen.printStatistics(scenGen);
  }
}

// reads config for Hearts of Iron IV
void Hoi4Module::readHoiConfig(std::string configSubFolder,
                               std::string username) {
  Logger::logLine("Reading Hoi4 Config");
  const auto root =
      this->readConfig(configSubFolder, username, "Hearts of Iron IV");

  // now try to locate game files
  if (!findGame(gamePath, "Hearts of Iron IV")) {
    throw(std::exception("Could not locate the game. Exiting"));
  }
  // default values taken from base game
  hoi4Gen.resources = {
      {"aluminium", {root.get<double>("hoi4.aluminiumFactor"), 1169.0, 0.3}},
      {"chromium", {root.get<double>("hoi4.chromiumFactor"), 1250.0, 0.2}},
      {"oil", {root.get<double>("hoi4.oilFactor"), 1220.0, 0.1}},
      {"rubber", {root.get<double>("hoi4.rubberFactor"), 1029.0, 0.1}},
      {"steel", {root.get<double>("hoi4.steelFactor"), 2562.0, 0.5}},
      {"tungsten", {root.get<double>("hoi4.tungstenFactor"), 1188.0, 0.2}}};
  hoi4Gen.worldPopulationFactor =
      root.get<double>("scenario.worldPopulationFactor");
  hoi4Gen.industryFactor = root.get<double>("scenario.industryFactor");
  hoi4Gen.resourceFactor = root.get<double>("hoi4.resourceFactor");
}
