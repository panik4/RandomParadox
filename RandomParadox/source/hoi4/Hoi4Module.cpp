#include "hoi4/Hoi4Module.h"

Hoi4Module::Hoi4Module() {}

Hoi4Module::~Hoi4Module() {}

bool Hoi4Module::createPaths() {
  // prepare folder structure
  try {
    // generic cleanup and path creation
    GenericModule::createPaths(hoi4ModPath);
    // map
    std::filesystem::create_directory(hoi4ModPath + "\\map\\supplyareas\\");
    std::filesystem::create_directory(hoi4ModPath +
                                      "\\map\\strategicregions\\");
    // gfx
    std::filesystem::create_directory(hoi4ModPath + "\\gfx\\flags\\small\\");
    std::filesystem::create_directory(hoi4ModPath + "\\gfx\\flags\\medium\\");
    // history
    std::filesystem::create_directory(hoi4ModPath + "\\history\\units\\");
    std::filesystem::create_directory(hoi4ModPath + "\\history\\states\\");
    // localisation
    std::filesystem::create_directory(hoi4ModPath +
                                      "\\localisation\\english\\");
    // common
    std::filesystem::create_directory(hoi4ModPath +
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

    Hoi4Parser::dumpStates(hoi4ModPath + "\\history\\states",
                           hoi4Gen.countries);
    Hoi4Parser::dumpUnitStacks(hoi4ModPath + "\\map\\unitstacks.txt",
                               scenGen.f.provinceGenerator.provinces);
    Hoi4Parser::dumpRocketSites(hoi4ModPath + "\\map\\rocketsites.txt",
                                scenGen.f.provinceGenerator.regions);
    Hoi4Parser::dumpStrategicRegions(hoi4ModPath + "\\map\\strategicregions",
                                     scenGen.f.provinceGenerator.regions,
                                     hoi4Gen.strategicRegions);
    Hoi4Parser::dumpWeatherPositions(
        hoi4ModPath + "\\map\\weatherpositions.txt",
        scenGen.f.provinceGenerator.regions, hoi4Gen.strategicRegions);
    Hoi4Parser::dumpAdjacencyRules(hoi4ModPath + "\\map\\adjacency_rules.txt");
    Hoi4Parser::dumpAirports(hoi4ModPath + "\\map\\airports.txt",
                             scenGen.f.provinceGenerator.regions);
    Hoi4Parser::dumpBuildings(hoi4ModPath + "\\map\\buildings.txt",
                              scenGen.f.provinceGenerator.regions);
    if (!useDefaultStates) {
      Hoi4Parser::dumpDefinition(hoi4ModPath + "\\map\\definition.csv",
                                 scenGen.gameProvinces);
    }
    Bitmap::SaveBMPToFile(Bitmap::findBitmapByKey("provinces"),
                          (hoi4ModPath + ("\\map\\provinces.bmp")).c_str());
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
      FormatConverter formatConverter(hoi4Path);
      formatConverter.dump8BitTerrain(hoi4ModPath + "\\map\\terrain.bmp",
                                      "terrainHoi4", cut);
      formatConverter.dump8BitCities(hoi4ModPath + "\\map\\cities.bmp",
                                     "citiesHoi4", cut);
      formatConverter.dump8BitRivers(hoi4ModPath + "\\map\\rivers.bmp",
                                     "riversHoi4", cut);
      formatConverter.dump8BitTrees(hoi4ModPath + "\\map\\trees.bmp",
                                    "treesHoi4", false);
      formatConverter.dump8BitHeightmap(hoi4ModPath + "\\map\\heightmap.bmp",
                                        "heightmapHoi4");
      formatConverter.dumpTerrainColourmap(
          hoi4ModPath + "\\map\\terrain\\colormap_rgb_cityemissivemask_a.dds",
          cut);
      formatConverter.dumpDDSFiles(
          hoi4ModPath + "\\map\\terrain\\colormap_water_", cut);
      formatConverter.dumpWorldNormal(hoi4ModPath + "\\map\\world_normal.bmp",
                                      cut);

      Hoi4Parser::writeCompatibilityHistory(
          hoi4ModPath + "\\history\\countries\\", hoi4Path,
          scenGen.f.provinceGenerator.regions);
      Hoi4Parser::writeHistoryCountries(hoi4ModPath + "\\history\\countries\\",
                                        hoi4Gen.countries);
      Hoi4Parser::writeHistoryUnits(hoi4ModPath + "\\history\\units\\",
                                    hoi4Gen.countries);
      Hoi4Parser::dumpCommonCountryTags(
          hoi4ModPath + "\\common\\country_tags\\02_countries.txt",
          hoi4Gen.countries);
      Hoi4Parser::dumpCommonCountries(
          hoi4ModPath + "\\common\\countries\\",
          hoi4Path + "\\common\\countries\\colors.txt", hoi4Gen.countries);
      Hoi4Parser::dumpAdj(hoi4ModPath + "\\map\\adjacencies.csv");
      Hoi4Parser::dumpAirports(hoi4ModPath + "\\map\\airports.txt",
                               scenGen.f.provinceGenerator.regions);
      Hoi4Parser::dumpBuildings(hoi4ModPath + "\\map\\buildings.txt",
                                scenGen.f.provinceGenerator.regions);
      Hoi4Parser::dumpContinents(hoi4ModPath + "\\map\\continents.txt",
                                 scenGen.f.provinceGenerator.continents);
      Hoi4Parser::dumpDefinition(hoi4ModPath + "\\map\\definition.csv",
                                 scenGen.gameProvinces);
      Hoi4Parser::dumpUnitStacks(hoi4ModPath + "\\map\\unitstacks.txt",
                                 scenGen.f.provinceGenerator.provinces);
      Hoi4Parser::dumpRocketSites(hoi4ModPath + "\\map\\rocketsites.txt",
                                  scenGen.f.provinceGenerator.regions);
      Hoi4Parser::dumpStrategicRegions(hoi4ModPath + "\\map\\strategicregions",
                                       scenGen.f.provinceGenerator.regions,
                                       hoi4Gen.strategicRegions);
      Hoi4Parser::dumpStates(hoi4ModPath + "\\history\\states",
                             hoi4Gen.countries);
      Hoi4Parser::dumpFlags(hoi4ModPath + "\\gfx\\flags\\", hoi4Gen.countries);
      Hoi4Parser::dumpWeatherPositions(
          hoi4ModPath + "\\map\\weatherpositions.txt",
          scenGen.f.provinceGenerator.regions, hoi4Gen.strategicRegions);
      Hoi4Parser::dumpAdjacencyRules(hoi4ModPath +
                                     "\\map\\adjacency_rules.txt");
      Hoi4Parser::dumpSupply(hoi4ModPath + "\\map\\",
                             hoi4Gen.supplyNodeConnections);
      Hoi4Parser::writeStateNames(hoi4ModPath + "\\localisation\\english\\",
                                  hoi4Gen.countries);
      Hoi4Parser::writeCountryNames(hoi4ModPath + "\\localisation\\english\\",
                                    hoi4Gen.countries);
      Hoi4Parser::writeStrategicRegionNames(
          hoi4ModPath + "\\localisation\\english\\", hoi4Gen.strategicRegions);
      Hoi4Parser::writeFoci(hoi4ModPath + "\\common\\national_focus\\",
                            hoi4Gen.countries);
      Hoi4Parser::dumpCommonBookmarks(hoi4ModPath + "\\common\\bookmarks\\",
                                      hoi4Gen.countries,
                                      hoi4Gen.strengthScores);
      Hoi4Parser::copyDescriptorFile("resources\\hoi4\\descriptor.mod",
                                     hoi4ModPath, hoi4ModsDirectory, modName);

      // just copy over provinces.bmp, already in a compatible format
      Bitmap::SaveBMPToFile(Bitmap::findBitmapByKey("provinces"),
                            (hoi4ModPath + ("\\map\\provinces.bmp")).c_str());
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
// a method to search for the original game files on the hard drive(s)
bool Hoi4Module::findHoi4() {
  std::vector<std::string> drives{"C:\\", "D:\\", "E:\\",
                                  "F:\\", "G:\\", "H:\\"};
  // first try to find hoi4 at the configured location
  if (std::filesystem::exists(hoi4Path)) {
    hoi4Path = hoi4Path;
    return true;
  } else {
    Logger::logLine("Could not find game under configured path ", hoi4Path,
                    " it doesn't exist or is malformed. Auto search will now "
                    "try to locate the game, but may not succeed. It is "
                    "recommended to correctly configure the path");
    system("pause");
  }
  for (const auto &drive : drives) {
    if (std::filesystem::exists(
            drive + "Program Files (x86)\\Steam\\steamapps\\common\\Hearts of "
                    "Iron IV")) {
      hoi4Path =
          drive +
          "Program Files (x86)\\Steam\\steamapps\\common\\Hearts of Iron IV";
      Logger::logLine("Located game under ", hoi4Path);
      return true;
    } else if (std::filesystem::exists(
                   drive + "Program Files\\Steam\\steamapps\\common\\Hearts of "
                           "Iron IV")) {
      hoi4Path =
          drive + "Program Files\\Steam\\steamapps\\common\\Hearts of Iron IV";
      Logger::logLine("Located game under ", hoi4Path);
      return true;
    } else if (std::filesystem::exists(
                   drive + "Steam\\steamapps\\common\\Hearts of Iron IV")) {
      hoi4Path = drive + "Steam\\steamapps\\common\\Hearts of Iron IV";
      Logger::logLine("Located game under ", hoi4Path);
      return true;
    }
  }
  Logger::logLine("Could not find the game anywhere. Make sure the hoi4Path is "
                  "configured correctly in the config files");
  return false;
}
// reads config for Hearts of Iron IV
void Hoi4Module::readConfig(std::string configSubFolder, std::string username) {
  // Short alias for this namespace
  namespace pt = boost::property_tree;
  // Create a root
  pt::ptree root;
  std::ifstream f(configSubFolder + "Hoi4Module.json");
  std::stringstream buffer;
  if (!f.good()) {
    Logger::logLine("Config could not be loaded");
  }
  buffer << f.rdbuf();
  try {
    pt::read_json(buffer, root);
  } catch (std::exception e) {
    std::string error =
        "Incorrect config " + configSubFolder + " \"Hoi4Module.json\"\n";
    error += "You can try fixing it yourself. Error is: \n";
    error += e.what();
    error += "\n";
    error += "Otherwise try running it through a json validator, e.g. "
             "\"https://jsonlint.com/\" or search for \"json validator\"\n";
    throw(std::exception(error.c_str()));
  }
  // now read the paths
  modName = root.get<std::string>("module.modName");
  hoi4Path = root.get<std::string>("module.hoi4Path");
  hoi4ModPath = root.get<std::string>("module.hoi4ModPath") + modName;
  ParserUtils::replaceOccurences(hoi4ModPath, "<username>", username);
  hoi4ModsDirectory = root.get<std::string>("module.hoi4ModsDirectory");
  ParserUtils::replaceOccurences(hoi4ModsDirectory, "<username>", username);

  // now try to locate game files
  if (!findHoi4()) {
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
  // passed to generic ScenarioGenerator
  numCountries = root.get<int>("scenario.numCountries");
}
