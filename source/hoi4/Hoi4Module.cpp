#include "hoi4/Hoi4Module.h"
using namespace Fwg;
namespace Rpx::Hoi4 {
Hoi4Module::Hoi4Module(const boost::property_tree::ptree &gamesConf,
                       const std::string &configSubFolder,
                       const std::string &username, const bool editMode) {
  generator = std::make_shared<Rpx::Hoi4::Generator>((configSubFolder));
  hoi4Gen = std::reinterpret_pointer_cast<Rpx::Hoi4::Generator, Arda::ArdaGen>(
      generator);
  const auto &config = Fwg::Cfg::Values();
  // set the game subpath
  this->gameSubPath = "hoi4.exe";
  gameType = GameType::Hoi4;

  // read hoi configs and potentially overwrite settings for fwg
  readHoiConfig(configSubFolder, username, gamesConf);
  initNameData(Fwg::Cfg::Values().resourcePath + "names", pathcfg.gamePath);
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
    remove_all(pathcfg.gameModPath + "//events//");
    remove_all(pathcfg.gameModPath + "//history");
    remove_all(pathcfg.gameModPath + "//common//");
    remove_all(pathcfg.gameModPath + "//portraits//");
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
    // portraits
    create_directory(pathcfg.gameModPath + "//portraits//");
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
    create_directory(pathcfg.gameModPath + "//common//characters//");
    create_directory(pathcfg.gameModPath + "//common//ideas//");
    create_directory(pathcfg.gameModPath + "//common//bookmarks//");
    create_directory(pathcfg.gameModPath + "//common//country_tags//");
    create_directory(pathcfg.gameModPath + "//common//names//");
    create_directory(pathcfg.gameModPath + "//common//scripted_triggers//");
    //
    create_directory(pathcfg.gameModPath + "//tutorial//");
    return true;
  } catch (std::exception e) {
    std::string error =
        "Configured paths seem to be messed up, check Hoi4Module.json\n";
    error += "You can try fixing it yourself. Error is:\n ";
    error += e.what();
    Fwg::Utils::Logging::logLine(error);
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
  config.resolutionBase = 256;
  config.autoSplitProvinces = false;
  config.miningPerRegion = 0;
  config.forestryPerRegion = 0;
  config.citiesPerRegion = 5;
  config.portsPerRegion = 1;
  config.agriculturePerRegion = 3;
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

void Hoi4Module::initFormatConverter() {
  formatConverter = Gfx::Hoi4::FormatConverter(pathcfg.gamePath, "Hoi4");
}

void Hoi4Module::writeTextFiles() {
  using namespace Parsing::Writing;
  Fwg::Utils::Logging::logLine(
      "Writing Hoi4 mod text files to path: ",
      Fwg::Utils::userFilter(pathcfg.gameModPath, Cfg::Values().username));
  Map::adj(pathcfg.gameModPath + "//map//adjacencies.csv");
  Map::adjacencyRules(pathcfg.gameModPath + "//map//adjacency_rules.txt");
  Map::ambientObjects(pathcfg.gameModPath + "//map//ambient_object.txt");
  Map::supply(pathcfg.gameModPath + "//map//", hoi4Gen->supplyNodeConnections);
  Map::buildings(pathcfg.gameModPath + "//map//buildings.txt",
                 hoi4Gen->hoi4States);
  Map::continents(pathcfg.gameModPath + "//map//continent.txt",
                  hoi4Gen->scenContinents, pathcfg.gamePath,
                  pathcfg.gameModPath +
                      "//localisation//english//province_names_l_english.yml");
  Map::definition(pathcfg.gameModPath + "//map//definition.csv",
                  hoi4Gen->ardaProvinces);
  Map::strategicRegions(pathcfg.gameModPath + "//map//strategicregions",
                        hoi4Gen->areaData.regions, hoi4Gen->superRegions);
  Map::unitStacks(pathcfg.gameModPath + "//map//unitstacks.txt",
                  hoi4Gen->ardaProvinces, hoi4Gen->hoi4States,
                  hoi4Gen->terrainData.detailedHeightMap);
  Map::weatherPositions(pathcfg.gameModPath + "//map//weatherpositions.txt",
                        hoi4Gen->areaData.regions, hoi4Gen->superRegions);

  Countries::commonCountryTags(pathcfg.gameModPath +
                                   "//common//country_tags//02_countries.txt",
                               hoi4Gen->hoi4Countries);
  Countries::commonCountries(pathcfg.gameModPath + "//common//countries//",
                             pathcfg.gamePath +
                                 "//common//countries//colors.txt",
                             hoi4Gen->hoi4Countries);
  Countries::commonCharacters(pathcfg.gameModPath + "//common//characters//",
                              hoi4Gen->hoi4Countries);
  Countries::commonNames(pathcfg.gameModPath + "//common//names//00_names.txt",
                         hoi4Gen->hoi4Countries);
  Countries::foci(pathcfg.gameModPath + "//common//national_focus//",
                  hoi4Gen->hoi4Countries, hoi4Gen->nData);
  Countries::flags(pathcfg.gameModPath + "//gfx//flags//",
                   hoi4Gen->hoi4Countries);
  Countries::historyCountries(pathcfg.gameModPath + "//history//countries//",
                              hoi4Gen->hoi4Countries, pathcfg.gamePath,
                              hoi4Gen->areaData.regions);
  Countries::historyUnits(pathcfg.gameModPath + "//history//units//",
                          hoi4Gen->hoi4Countries);
  Countries::ideas(pathcfg.gameModPath + "//common//ideas//",
                   hoi4Gen->hoi4Countries);
  Countries::portraits(pathcfg.gameModPath + "//portraits//",
                       hoi4Gen->hoi4Countries);
  Countries::states(pathcfg.gameModPath + "//history//states",
                    hoi4Gen->hoi4States);

  aiStrategy(pathcfg.gameModPath + "//common//", hoi4Gen->scenContinents);
  events(pathcfg.gameModPath + "//");
  commonBookmarks(pathcfg.gameModPath + "//common//bookmarks//",
                  hoi4Gen->hoi4Countries, hoi4Gen->countryImportanceScores);
  tutorials(pathcfg.gameModPath + "//tutorial//tutorial.txt");
  copyDescriptorFile(Fwg::Cfg::Values().resourcePath + "hoi4//descriptor.mod",
                     pathcfg.gameModPath, pathcfg.gameModsDirectory,
                     pathcfg.modName);

  /*scriptedTriggers(pathcfg.gamePath + "//common//scripted_triggers//",
                   pathcfg.gameModPath + "//common//scripted_triggers//");*/
  // commonFiltering(pathcfg.gamePath, pathcfg.gameModPath);
}
void Hoi4Module::writeLocalisation() {

  using namespace Parsing::Writing::Localisation;
  stateNames(pathcfg.gameModPath + "//localisation//english//",
             hoi4Gen->hoi4Countries);
  countryNames(pathcfg.gameModPath + "//localisation//english//",
               hoi4Gen->hoi4Countries, hoi4Gen->nData);
  strategicRegionNames(pathcfg.gameModPath + "//localisation//english//",
                       hoi4Gen->superRegions);
  victoryPointNames(pathcfg.gameModPath + "//localisation//english//",
                    hoi4Gen->hoi4States);
}
void Hoi4Module::writeImages() {
  Fwg::Utils::Logging::logLine(
      "Writing Hoi4 mod image files to path: ",
      Fwg::Utils::userFilter(pathcfg.gameModPath, Cfg::Values().username));

  formatConverter.dump8BitTerrain(
      hoi4Gen->terrainData, hoi4Gen->climateData, hoi4Gen->civLayer,
      pathcfg.gameModPath + "//map//terrain.bmp", "terrain", cut);
  formatConverter.dump8BitCities(hoi4Gen->climateMap,
                                 pathcfg.gameModPath + "//map//cities.bmp",
                                 "cities", cut);
  formatConverter.dump8BitRivers(hoi4Gen->terrainData, hoi4Gen->climateData,
                                 pathcfg.gameModPath + "//map//rivers",
                                 "rivers", cut);
  formatConverter.dump8BitTrees(hoi4Gen->terrainData, hoi4Gen->climateData,
                                pathcfg.gameModPath + "//map//trees.bmp",
                                "trees", false);
  formatConverter.dump8BitHeightmap(hoi4Gen->terrainData.detailedHeightMap,
                                    pathcfg.gameModPath + "//map//heightmap",
                                    "heightmap");
  formatConverter.dumpTerrainColourmap(
      hoi4Gen->worldMap, hoi4Gen->civLayer, pathcfg.gameModPath,
      "//map//terrain//colormap_rgb_cityemissivemask_a.dds",
      DXGI_FORMAT_B8G8R8A8_UNORM, 2, cut);
  formatConverter.dumpDDSFiles(
      hoi4Gen->terrainData.detailedHeightMap,
      pathcfg.gameModPath + "//map//terrain//colormap_water_", cut, 8);
  formatConverter.dumpWorldNormal(
      Fwg::Gfx::Bitmap(Cfg::Values().width, Cfg::Values().height, 24,
                       hoi4Gen->terrainData.sobelData),
      pathcfg.gameModPath + "//map//world_normal.bmp", cut);

  // just copy over provinces.bmp, already in a compatible format
  Fwg::Gfx::Bmp::save(hoi4Gen->provinceMap,
                      (pathcfg.gameModPath + ("//map//provinces.bmp")).c_str());
}

void Hoi4Module::readHoi(std::string &path) {
  path.append("//");
  initNameData(Fwg::Cfg::Values().resourcePath + "names", path);
  auto &config = Fwg::Cfg::Values();
  bool bufferedCut = config.cut;
  config.cut = false;
  auto heightmap = Fwg::IO::Reader::readGenericImage(
      path + "map//heightmap.bmp", config, false);
  hoi4Gen->loadHeight(config, heightmap);
  hoi4Gen->genSobelMap(config);
  hoi4Gen->genLand();
  hoi4Gen->loadClimate(config, path + "map//terrain.bmp");
  hoi4Gen->provinceMap =
      Fwg::IO::Reader::readGenericImage(path + "map//provinces.bmp", config);
  //// read in game or mod files
  hoi4Gen->climateData.habitabilities.resize(hoi4Gen->provinceMap.size());
  Hoi4::Parsing::Reading::readProvinces(hoi4Gen->terrainData,
                                        hoi4Gen->climateData, path,
                                        "provinces.bmp", hoi4Gen->areaData);
  hoi4Gen->wrapupProvinces(config);
  // get the provinces into ardaProvinces
  hoi4Gen->mapProvinces();
  // load existing states: we first get all the state files and parse their
  // provinces for land regions (including lakes) then we need to get the
  // strategic region files, and for every strategic region that is a sea state,
  // we create a sea region?
  Hoi4::Parsing::Reading::readStates(path, hoi4Gen);

  // ensure continents are created via the details in definition.csv.
  // Which means we also need to load the existing continents file to match
  // those with each other, so another export does not overwrite the continents
  std::map<int, Areas::Continent> continents;
  for (auto &prov : hoi4Gen->areaData.provinces) {
    if (prov->continentID != -1) {
      if (continents.find(prov->continentID) == continents.end()) {
        Areas::Continent continent("", prov->continentID);
        continents.insert({prov->continentID, continent});
      } else {
        continents.at(prov->continentID).provinces.push_back(prov);
      }
    }
  }
  hoi4Gen->areaData.continents.clear();
  for (auto &c : continents) {
    hoi4Gen->areaData.continents.push_back(c.second);
  }

  // get the provinces into ardaProvinces
  // hoi4Gen->mapProvinces();
  // get the states from files to initialize ardaRegions
  // Hoi4::Parsing::Reading::readStates(gamePath, *hoi4Gen);
  // try {
  //  hoi4Gen->mapRegions();
  //} catch (std::exception e) {
  //  Fwg::Utils::Logging::logLine("Error while mapping regions, ", e.what());
  //};
  //// read the colour codes from the game/mod files
  // hoi4Gen->countryColourMap =
  //     Hoi4::Parsing::Reading::readColourMapping(pathcfg.gamePath);
  //// now initialize hoi4 states from the ardaRegions
  // hoi4Gen->mapTerrain();
  // for (auto &c : hoi4Gen->countries) {
  //   auto fCol = hoi4Gen->countryColourMap.valueSearch(c.first);
  //   if (fCol != Fwg::Gfx::Colour{0, 0, 0}) {
  //     c.second->colour = fCol;
  //   } else {
  //     do {
  //       // generate random colour as long as we have a duplicate
  //       c.second->colour = Fwg::Gfx::Colour(RandNum::getRandom(1, 254),
  //                                           RandNum::getRandom(1, 254),
  //                                           RandNum::getRandom(1, 254));
  //     } while (hoi4Gen->countryColourMap.find(c.second->colour));
  //     hoi4Gen->countryColourMap.setValue(c.second->colour, c.first);
  //   }
  // }
  // hoi4Gen->mapCountries();
  //// read in further state details from map files
  // Hoi4::Parsing::Reading::readAirports(pathcfg.gamePath,
  // hoi4Gen->hoi4States);
  // Hoi4::Parsing::Reading::readRocketSites(pathcfg.gamePath,
  //                                         hoi4Gen->hoi4States);
  // Hoi4::Parsing::Reading::readBuildings(pathcfg.gamePath,
  // hoi4Gen->hoi4States);
  // Hoi4::Parsing::Reading::readSupplyNodes(pathcfg.gamePath,
  //                                         hoi4Gen->hoi4States);
  // Hoi4::Parsing::Reading::readWeatherPositions(pathcfg.gamePath,
  //                                              hoi4Gen->hoi4States);
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
    initNameData(Fwg::Cfg::Values().resourcePath + "names",
                 this->pathcfg.gamePath);
    // start with the generic stuff in the Scenario hoi4Gen
    hoi4Gen->mapProvinces();
    hoi4Gen->mapRegions();
    hoi4Gen->mapContinents();
    hoi4Gen->mapTerrain();
    // generate generic world data
    Arda::Civilization::generateWorldCivilizations(
        hoi4Gen->ardaRegions, hoi4Gen->ardaProvinces, hoi4Gen->civData,
        hoi4Gen->scenContinents, hoi4Gen->superRegions);

    // non-country stuff
    auto stratFactory = []() -> std::shared_ptr<StrategicRegion> {
      return std::make_shared<StrategicRegion>();
    };
    hoi4Gen->generateStrategicRegions(stratFactory);
    hoi4Gen->generateWeather();
    // generate state information
    hoi4Gen->generateStateSpecifics();
    hoi4Gen->generateStateResources();
    auto countryFactory = []() -> std::shared_ptr<Hoi4Country> {
      return std::make_shared<Hoi4Country>();
    };
    // generate country data
    hoi4Gen->generateCountries(countryFactory);

    hoi4Gen->generateLogistics();
    // politics, etc
    hoi4Gen->generateCountrySpecifics();

    NationalFocus::buildMaps();
    hoi4Gen->generateFocusTrees();
    hoi4Gen->distributeVictoryPoints();
    hoi4Gen->generatePositions();

  } catch (std::exception e) {
    std::string error = "Error while generating the Hoi4 Module.\n";
    error += "Error is: \n";
    error += e.what();
    Fwg::Utils::Logging::logLine(error);
  }
  // now start writing game files
  try {
    writeImages();
    writeTextFiles();
    writeLocalisation();
  } catch (std::exception e) {
    std::string error = "Error while dumping and writing files.\n";
    error += "Error is: \n";
    error += e.what();
    Fwg::Utils::Logging::logLine(error);
  }
  // now if everything worked, print info about world and pause for user to
  // see
  hoi4Gen->printStatistics();
}

} // namespace Rpx::Hoi4