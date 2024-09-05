#include "hoi4/Hoi4Module.h"
using namespace Fwg;
namespace Scenario::Hoi4 {
Hoi4Module::Hoi4Module(const boost::property_tree::ptree &gamesConf,
                       const std::string &configSubFolder,
                       const std::string &username, const bool editMode) {
  generator = std::make_shared<Scenario::Hoi4::Generator>((configSubFolder));
  hoi4Gen = std::reinterpret_pointer_cast<Scenario::Hoi4::Generator,
                                          Scenario::Generator>(generator);
  const auto &config = Fwg::Cfg::Values();
  // set the executable subpath
  this->executableSubPath = "hoi4.exe";

  // read hoi configs and potentially overwrite settings for fwg
  readHoiConfig(configSubFolder, username, gamesConf);
}

Hoi4Module::~Hoi4Module() {}

bool Hoi4Module::createPaths() {
  // prepare folder structure
  try {
    // generic cleanup and path creation
    using namespace std::filesystem;
    // GenericModule::createPaths(pathcfg.gameModPath);
    create_directory(pathcfg.gameModPath);
    // map
    remove_all(pathcfg.gameModPath + "//map//");
    remove_all(pathcfg.gameModPath + "//gfx");
    remove_all(pathcfg.gameModPath + "//history");
    remove_all(pathcfg.gameModPath + "//common//");
    remove_all(pathcfg.gameModPath + "//localisation//");
    create_directory(pathcfg.gameModPath + "//map//");
    create_directory(pathcfg.gameModPath + "//map//terrain//");
    // gfx
    create_directory(pathcfg.gameModPath + "//gfx//");
    create_directory(pathcfg.gameModPath + "//gfx//flags//");
    // history
    create_directory(pathcfg.gameModPath + "//history//");
    // localisation
    create_directory(pathcfg.gameModPath + "//localisation//");
    // common
    create_directory(pathcfg.gameModPath + "//common//");
    // map
    create_directory(pathcfg.gameModPath + "//map//strategicregions//");
    // gfx
    create_directory(pathcfg.gameModPath + "//gfx//flags//small//");
    create_directory(pathcfg.gameModPath + "//gfx//flags//medium//");
    // history
    create_directory(pathcfg.gameModPath + "//history//units//");
    create_directory(pathcfg.gameModPath + "//history//states//");
    create_directory(pathcfg.gameModPath + "//history//countries//");
    // localisation
    create_directory(pathcfg.gameModPath + "//localisation//english//");
    // common
    create_directory(pathcfg.gameModPath + "//common//national_focus//");
    create_directory(pathcfg.gameModPath + "//common//countries//");
    create_directory(pathcfg.gameModPath + "//common//bookmarks//");
    create_directory(pathcfg.gameModPath + "//common//country_tags//");
    //
    create_directory(pathcfg.gameModPath + "//tutorial//");
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

// reads config for Hearts of Iron IV
void Hoi4Module::readHoiConfig(const std::string &configSubFolder,
                               const std::string &username,
                               const boost::property_tree::ptree &rpdConf) {
  Fwg::Utils::Logging::logLine("Reading Hoi4 Config");
  this->configurePaths(username, "Hearts of Iron IV", rpdConf);

  auto &config = Cfg::Values();
  namespace pt = boost::property_tree;
  pt::ptree hoi4Conf;
  try {
    // Read the basic settings
    std::ifstream f(configSubFolder + "//Hearts of Iron IVModule.json");
    std::stringstream buffer;
    if (!f.good())
      Fwg::Utils::Logging::logLine("Config could not be loaded");
    buffer << f.rdbuf();
    Fwg::Parsing::replaceInStringStream(buffer, "//", "//");

    pt::read_json(buffer, hoi4Conf);
  } catch (std::exception e) {
    Fwg::Utils::Logging::logLine("Incorrect config \"RandomParadox.json\"");
    Fwg::Utils::Logging::logLine("You can try fixing it yourself. Error is: ",
                                 e.what());
    Fwg::Utils::Logging::logLine(
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or search for \"json validator\"");
    system("pause");
  }
  // default values taken from base game
  hoi4Gen->resources = {
      {"aluminium",
       {hoi4Conf.get<double>("hoi4.aluminiumFactor"), 1169.0, 0.3}},
      {"chromium", {hoi4Conf.get<double>("hoi4.chromiumFactor"), 1250.0, 0.2}},
      {"oil", {hoi4Conf.get<double>("hoi4.oilFactor"), 1220.0, 0.1}},
      {"rubber", {hoi4Conf.get<double>("hoi4.rubberFactor"), 1029.0, 0.1}},
      {"steel", {hoi4Conf.get<double>("hoi4.steelFactor"), 2562.0, 0.5}},
      {"tungsten", {hoi4Conf.get<double>("hoi4.tungstenFactor"), 1188.0, 0.2}}};
  hoi4Gen->weatherChances = {
      {"baseLightRainChance", hoi4Conf.get<double>("hoi4.baseLightRainChance")},
      {"baseHeavyRainChance", hoi4Conf.get<double>("hoi4.baseHeavyRainChance")},
      {"baseMudChance", hoi4Conf.get<double>("hoi4.baseMudChance")},
      {"baseBlizzardChance", hoi4Conf.get<double>("hoi4.baseBlizzardChance")},
      {"baseSandstormChance", hoi4Conf.get<double>("hoi4.baseSandstormChance")},
      {"baseSnowChance", hoi4Conf.get<double>("hoi4.baseSnowChance")}};
  hoi4Gen->worldPopulationFactor =
      hoi4Conf.get<double>("scenario.worldPopulationFactor");
  hoi4Gen->worldIndustryFactor =
      hoi4Conf.get<double>("scenario.industryFactor");
  hoi4Gen->resourceFactor = hoi4Conf.get<double>("hoi4.resourceFactor");

  // settings for scenGen
  hoi4Gen->countryMappingPath =
      rpdConf.get<std::string>("randomScenario.countryColourMap");
  Fwg::Parsing::attachTrailing(hoi4Gen->countryMappingPath);

  //  passed to generic Scenariohoi4Gen
  hoi4Gen->numCountries = hoi4Conf.get<int>("scenario.numCountries");
  // force defaults for the game, if not set otherwise
  if (config.targetLandRegionAmount == 0 && config.autoLandRegionParams)
    config.targetLandRegionAmount = 640;
  // force defaults for the game, if not set otherwise
  if (config.targetSeaRegionAmount == 0 && config.autoSeaRegionParams)
    config.targetSeaRegionAmount = 160;
  cut = config.cut;
  config.forceResolutionBase = true;
  config.resolutionBase = 64;
  config.autoSplitProvinces = true;
  // check if config settings are fine
  config.sanityCheck();
}

void Hoi4Module::prepareData() {}

// void Hoi4Module::modEdit(std::string &path) {
//   auto &config = Fwg::Cfg::Values();
//   path.append("//");
//   hoi4Gen->heightMap = Fwg::IO::Reader::readGenericImage(
//       path + "map//heightmap.bmp", Fwg::Cfg::Values());
//   if (hoi4Gen->heightMap.initialised()) {
//     hoi4Gen->genSobelMap(config);
//     hoi4Gen->genLand();
//     hoi4Gen->genLand();
//     hoi4Gen->genHumidity(config);
//     hoi4Gen->loadClimate(path + "map//terrain.bmp");
//   }
// }

void Hoi4Module::writeTextFiles() {
  using namespace Parsing::Writing;
  Fwg::Utils::Logging::logLine("Writing Hoi4 mod text files to path: ",
                               pathcfg.gameModPath);
  ambientObjects(pathcfg.gameModPath + "//map//ambient_object.txt",
                 hoi4Gen->heightMap);
  compatibilityHistory(pathcfg.gameModPath + "//history//countries//",
                       pathcfg.gamePath, hoi4Gen->areas.regions);
  historyCountries(pathcfg.gameModPath + "//history//countries//",
                   hoi4Gen->hoi4Countries);
  historyUnits(pathcfg.gameModPath + "//history//units//",
               hoi4Gen->hoi4Countries);
  commonCountryTags(pathcfg.gameModPath +
                        "//common//country_tags//02_countries.txt",
                    hoi4Gen->hoi4Countries);
  commonCountries(pathcfg.gameModPath + "//common//countries//",
                  pathcfg.gamePath + "//common//countries//colors.txt",
                  hoi4Gen->hoi4Countries);
  adj(pathcfg.gameModPath + "//map//adjacencies.csv");
  airports(pathcfg.gameModPath + "//map//airports.txt", hoi4Gen->areas.regions);
  buildings(pathcfg.gameModPath + "//map//buildings.txt", hoi4Gen->hoi4States,
            hoi4Gen->heightMap);
  continents(pathcfg.gameModPath + "//map//continents.txt",
             hoi4Gen->areas.continents);
  definition(pathcfg.gameModPath + "//map//definition.csv",
             hoi4Gen->gameProvinces);
  unitStacks(pathcfg.gameModPath + "//map//unitstacks.txt",
             hoi4Gen->areas.provinces, hoi4Gen->hoi4States, hoi4Gen->heightMap);
  rocketSites(pathcfg.gameModPath + "//map//rocketsites.txt",
              hoi4Gen->areas.regions);
  strategicRegions(pathcfg.gameModPath + "//map//strategicregions",
                   hoi4Gen->areas.regions, hoi4Gen->strategicRegions);
  states(pathcfg.gameModPath + "//history//states", hoi4Gen->hoi4States);
  flags(pathcfg.gameModPath + "//gfx//flags//", hoi4Gen->hoi4Countries);
  weatherPositions(pathcfg.gameModPath + "//map//weatherpositions.txt",
                   hoi4Gen->areas.regions, hoi4Gen->strategicRegions);
  adjacencyRules(pathcfg.gameModPath + "//map//adjacency_rules.txt");
  supply(pathcfg.gameModPath + "//map//", hoi4Gen->supplyNodeConnections);
  stateNames(pathcfg.gameModPath + "//localisation//english//",
             hoi4Gen->hoi4Countries);
  countryNames(pathcfg.gameModPath + "//localisation//english//",
               hoi4Gen->hoi4Countries, hoi4Gen->nData);
  strategicRegionNames(pathcfg.gameModPath + "//localisation//english//",
                       hoi4Gen->strategicRegions);
  victoryPointNames(pathcfg.gameModPath + "//localisation//english//",
                    hoi4Gen->hoi4States);
  foci(pathcfg.gameModPath + "//common//national_focus//",
       hoi4Gen->hoi4Countries, hoi4Gen->nData);
  commonBookmarks(pathcfg.gameModPath + "//common//bookmarks//",
                  hoi4Gen->hoi4Countries, hoi4Gen->countryImportanceScores);
  tutorials(pathcfg.gameModPath + "//tutorial//tutorial.txt");
  Parsing::copyDescriptorFile("resources//hoi4//descriptor.mod",
                              pathcfg.gameModPath, pathcfg.gameModsDirectory,
                              pathcfg.modName);
}
void Hoi4Module::writeImages() {
  Fwg::Utils::Logging::logLine("Writing Hoi4 mod image files to path: ",
                               pathcfg.gameModPath);
  // generate map files. Format must be converted and colours mapped to hoi4
  // compatible colours
  Gfx::FormatConverter formatConverter(pathcfg.gamePath, "Hoi4");
  formatConverter.dump8BitTerrain(hoi4Gen->climateData, hoi4Gen->civLayer,
                                  pathcfg.gameModPath + "//map//terrain.bmp",
                                  "terrain", cut);
  formatConverter.dump8BitCities(hoi4Gen->climateMap,
                                 pathcfg.gameModPath + "//map//cities.bmp",
                                 "cities", cut);
  formatConverter.dump8BitRivers(hoi4Gen->climateData,
                                 pathcfg.gameModPath + "//map//rivers",
                                 "rivers", cut);
  formatConverter.dump8BitTrees(hoi4Gen->climateData,
                                pathcfg.gameModPath + "//map//trees.bmp",
                                "trees", false);
  formatConverter.dump8BitHeightmap(hoi4Gen->heightMap,
                                    pathcfg.gameModPath + "//map//heightmap",
                                    "heightmap");
  formatConverter.dumpTerrainColourmap(
      Fwg::Gfx::MapMerging::mergeTerrain(
          hoi4Gen->heightMap, hoi4Gen->climateMap, hoi4Gen->sobelMap),
      hoi4Gen->civLayer, pathcfg.gameModPath,
      "//map//terrain//colormap_rgb_cityemissivemask_a.dds",
      DXGI_FORMAT_B8G8R8A8_UNORM, 2, cut);
  formatConverter.dumpDDSFiles(
      hoi4Gen->riverMap, hoi4Gen->heightMap,
      pathcfg.gameModPath + "//map//terrain//colormap_water_", cut, 8);
  formatConverter.dumpWorldNormal(
      hoi4Gen->sobelMap, pathcfg.gameModPath + "//map//world_normal.bmp", cut);

  // just copy over provinces.bmp, already in a compatible format
  Fwg::Gfx::Bmp::save(hoi4Gen->provinceMap,
                      (pathcfg.gameModPath + ("//map//provinces.bmp")).c_str());
}

void Hoi4Module::readHoi(std::string &gamePath) {
  initNameData("resources//names", gamePath);
  auto &config = Fwg::Cfg::Values();
  bool bufferedCut = config.cut;
  config.cut = false;
  hoi4Gen->provinceMap = Fwg::IO::Reader::readProvinceImage(
      gamePath + "map//provinces.bmp", config);
  hoi4Gen->heightMap = Fwg::IO::Reader::readGenericImage(
      gamePath + "map//heightmap.bmp", config);
  // read in game or mod files
  hoi4Gen->climateData.habitabilities.resize(hoi4Gen->provinceMap.size());
  Hoi4::Parsing::Reading::readProvinces(hoi4Gen->climateData, gamePath,
                                        "provinces.bmp", hoi4Gen->areas);
  // get the provinces into GameProvinces
  hoi4Gen->mapProvinces();
  // get the states from files to initialize gameRegions
  Hoi4::Parsing::Reading::readStates(gamePath, *hoi4Gen);
  try {
    hoi4Gen->mapRegions();
  } catch (std::exception e) {
  };
  // read the colour codes from the game/mod files
  hoi4Gen->countryColourMap =
      Hoi4::Parsing::Reading::readColourMapping(pathcfg.gamePath);
  // now initialize hoi4 states from the gameRegions
  hoi4Gen->mapTerrain();
  for (auto &c : hoi4Gen->countries) {
    auto fCol = hoi4Gen->countryColourMap.valueSearch(c.first);
    if (fCol != Fwg::Gfx::Colour{0, 0, 0}) {
      c.second->colour = fCol;
    } else {
      do {
        // generate random colour as long as we have a duplicate
        c.second->colour = Fwg::Gfx::Colour(RandNum::getRandom(1, 254),
                                            RandNum::getRandom(1, 254),
                                            RandNum::getRandom(1, 254));
      } while (hoi4Gen->countryColourMap.find(c.second->colour));
      hoi4Gen->countryColourMap.setValue(c.second->colour, c.first);
    }
  }
  hoi4Gen->mapCountries();
  // read in further state details from map files
  Hoi4::Parsing::Reading::readAirports(pathcfg.gamePath, hoi4Gen->hoi4States);
  Hoi4::Parsing::Reading::readRocketSites(pathcfg.gamePath,
                                          hoi4Gen->hoi4States);
  Hoi4::Parsing::Reading::readBuildings(pathcfg.gamePath, hoi4Gen->hoi4States);
  Hoi4::Parsing::Reading::readSupplyNodes(pathcfg.gamePath,
                                          hoi4Gen->hoi4States);
  Hoi4::Parsing::Reading::readWeatherPositions(pathcfg.gamePath,
                                               hoi4Gen->hoi4States);
  config.cut = bufferedCut;
}

void Hoi4Module::generate() {
  const auto &config = Fwg::Cfg::Values();
  if (config.width % 64 || config.height % 64) {
    throw(std::exception("Invalid format, both width and height of the image "
                         "must be multiples of 64."));
  } else if (config.scale && (config.scaleX % 64 || config.scaleY % 64)) {
    throw(std::exception("Invalid target dimensions for scaling mode, both "
                         "scaleX and scaleY of the image "
                         "must be multiples of 64."));
  }
  if (!createPaths())
    return;
  try {
    initNameData("resources//names", this->pathcfg.gamePath);
    // start with the generic stuff in the Scenario hoi4Gen
    hoi4Gen->mapProvinces();
    hoi4Gen->mapRegions();
    hoi4Gen->mapContinents();
    hoi4Gen->mapTerrain();
    hoi4Gen->generateCountries<Hoi4::Hoi4Country>();
    // build hoi4 countries out of basic countries
    hoi4Gen->mapCountries();
    hoi4Gen->evaluateNeighbours();
    hoi4Gen->generateWorldCivilizations();

    // non-country stuff
    hoi4Gen->generateStrategicRegions();
    hoi4Gen->generateWeather();
    // now generate hoi4 specific stuff
    hoi4Gen->generateCountrySpecifics();
    hoi4Gen->generateStateSpecifics();
    hoi4Gen->generateStateResources();
    // should work with countries = 0
    hoi4Gen->evaluateCountries();
    hoi4Gen->generateLogistics();
    NationalFocus::buildMaps();
    hoi4Gen->generateFocusTrees();
    hoi4Gen->generateCountryUnits();
    hoi4Gen->generateImportance();
    hoi4Gen->distributeVictoryPoints();
  } catch (std::exception e) {
    std::string error = "Error while generating the Hoi4 Module.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  // now start writing game files
  try {
    writeImages();
    writeTextFiles();
  } catch (std::exception e) {
    std::string error = "Error while dumping and writing files.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  // now if everything worked, print info about world and pause for user to
  // see
  hoi4Gen->printStatistics();
}

} // namespace Scenario::Hoi4