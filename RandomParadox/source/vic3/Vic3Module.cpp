#include "vic3/Vic3Module.h"
using namespace Fwg;

namespace Scenario::Vic3 {
Module::Module(const boost::property_tree::ptree &gamesConf,
               const std::string &configSubFolder,
               const std::string &username) {
  generator = std::make_shared<Scenario::Vic3::Generator>((configSubFolder));
  vic3Gen = std::reinterpret_pointer_cast<Scenario::Vic3::Generator,
                                          Scenario::Generator>(generator);
  // read eu4 configs and potentially overwrite settings for fwg
  readVic3Config(configSubFolder, username, gamesConf);
  // set the executable subpath
  this->executableSubPath = "binaries//victoria3.exe";
}

Module::~Module() {}

bool Module::createPaths() { // prepare folder structure
  try {
    using namespace std::filesystem;

    std::vector<std::string> paths = {"",
                                      "//.metadata//",
                                      "//map_data//",
                                      "//map_data//state_regions//",
                                      "//common//",
                                      "//common//defines",
                                      "//common//strategic_regions",
                                      "//common//cultures",
                                      "//common//religions",
                                      "//common//country_definitions",
                                      "//common//history",
                                      "//common//history//buildings",
                                      "//common//history//countries",
                                      "//common//history//pops",
                                      "//common//history//states",
                                      "//common//country_creation",
                                      "//common//journal_entries",
                                      "//common//scripted_triggers",
                                      "//common//decisions",
                                      "//events",
                                      "//events//agitators_events",
                                      "//gfx//",
                                      "//gfx//map",
                                      "//gfx//map//masks",
                                      "//gfx//map//map_object_data",
                                      "//gfx//map//terrain",
                                      "//gfx//map//textures",
                                      "//gfx//map//water",
                                      "//gfx//map//spline_network",
                                      "//content_source//",
                                      "//content_source//map_objects",
                                      "//content_source//map_objects//masks",
                                      "//localization//"};
    std::vector<std::string> pathsToRemove = {"//common//", "//localization//",
                                              "//map_data//"};

    for (const auto &path : pathsToRemove) {
      Fwg::Utils::Logging::logLine("Removing path: " + pathcfg.gameModPath +
                                   path);
      remove_all(pathcfg.gameModPath + path);
    }
    for (const auto &path : paths) {
      Fwg::Utils::Logging::logLine("Creating path: " + pathcfg.gameModPath +
                                   path);
      create_directory(pathcfg.gameModPath + path);
    }
    // specific debugging path
    if (Cfg::Values().debugLevel > 0)
      create_directory(Cfg::Values().mapsPath + "Vic3");

    return true;
  } catch (std::exception e) {
    std::string error = "Configured paths seem to be messed up, check Victoria "
                        "IIIModule.json\n";
    error += "You can try fixing it yourself. Error is:\n ";
    error += e.what();
    throw(std::exception(error.c_str()));
    return false;
  }
}

void Module::readVic3Config(const std::string &configSubFolder,
                            const std::string &username,
                            const boost::property_tree::ptree &rpdConf) {
  Fwg::Utils::Logging::logLine("Reading Vic 3 Config");
  this->configurePaths(username, "Victoria 3", rpdConf);
  auto &config = Cfg::Values();
  namespace pt = boost::property_tree;
  pt::ptree vic3Conf;
  try {
    // Read the basic settings
    std::ifstream f(configSubFolder + "//Victoria3Module.json");
    std::stringstream buffer;
    if (!f.good())
      Fwg::Utils::Logging::logLine("Config could not be loaded");
    buffer << f.rdbuf();
    Fwg::Parsing::replaceInStringStream(buffer, "//", "//");

    pt::read_json(buffer, vic3Conf);
  } catch (std::exception e) {
    Fwg::Utils::Logging::logLine(
        "Incorrect config \"Europa Universalis IVModule.json\"");
    Fwg::Utils::Logging::logLine("You can try fixing it yourself. Error is: ",
                                 e.what());
    Fwg::Utils::Logging::logLine(
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or search for \"json validator\"");
    system("pause");
  }
  //  passed to generic ScenarioGenerator
  vic3Gen->numCountries = vic3Conf.get<int>("scenario.numCountries");
  config.seaLevel = 18;
  config.riverFactor = 0.0;
  config.seaProvFactor *= 0.10;
  config.landProvFactor *= 1.0;
  config.minProvPerSeaRegion = 1;
  config.autoSeaRegionParams = false;
  config.forceResolutionBase = false;
  config.autoSplitProvinces = false;
  config.resolutionBase = 1;
  // allow massive images for Vic3
  config.targetMaxImageSize = 160'000'000;
  config.loadMapsPath = vic3Conf.get<std::string>("fastworldgen.loadMapsPath");
  config.heightmapIn = config.loadMapsPath +
                       vic3Conf.get<std::string>("fastworldgen.heightMapName");
  cut = config.cut;
  // check if config settings are fine
  config.sanityCheck();
}

void Module::generate() {
  if (!createPaths())
    return;

  initNameData("resources//names", this->pathcfg.gamePath);
  try {
    // start with the generic stuff in the Scenario Generator
    vic3Gen->mapProvinces();
    vic3Gen->mapRegions();

    vic3Gen->mapTerrain();
    vic3Gen->mapContinents();
    vic3Gen->generateCountries<Vic3::Country>();
    vic3Gen->evaluateNeighbours();
    vic3Gen->generateWorldCivilizations();
    vic3Gen->visualiseCountries(generator->countryMap);
    vic3Gen->generateStrategicRegions();
    // Vic3 specifics:
    vic3Gen->distributePops();
    vic3Gen->distributeResources();
    vic3Gen->mapCountries();
    if (!vic3Gen->importData(this->pathcfg.gamePath + "//game//")) {
      Fwg::Utils::Logging::logLine("ERROR: Could not import data from game folder. The generation has FAILED");
      return;
    }

    // handle basic development, tech level, policies,
    vic3Gen->generateCountrySpecifics();
    vic3Gen->diplomaticRelations();
    vic3Gen->createMarkets();
    vic3Gen->calculateNeeds();
    vic3Gen->distributeBuildings();

  } catch (std::exception e) {
    std::string error = "Error while generating the Vic3 Module.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  try {

    writeSplnet();
    // now write the files
    writeTextFiles();
    //  generate map files. Format must be converted and colours mapped to vic3
    //  compatible colours
    writeImages();

  } catch (std::exception e) {
    std::string error = "Error while dumping and writing files.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  vic3Gen->printStatistics();
}

void Module::writeTextFiles() {
  using namespace Parsing::Writing;
  auto foundRegions =
      compatRegions(pathcfg.gamePath + "//game//map_data//state_regions//",
                    pathcfg.gameModPath + "//map_data//state_regions//",
                    vic3Gen->vic3Regions);
  compatStratRegions(pathcfg.gamePath + "//game//common//strategic_regions//",
                     pathcfg.gameModPath + "//common//strategic_regions//",
                     vic3Gen->vic3Regions, foundRegions);
  compatReleasable(pathcfg.gamePath + "//game//common//country_creation//",
                   pathcfg.gameModPath + "//common//country_creation//");
  adj(pathcfg.gameModPath + "//map_data//adjacencies.csv");
  defaultMap(pathcfg.gameModPath + "//map_data//default.map",
             vic3Gen->gameProvinces);
  defines(pathcfg.gameModPath + "//common//defines//01_defines.txt");
  provinceTerrains(pathcfg.gameModPath + "//map_data//province_terrains.txt",
                   vic3Gen->gameProvinces);
  stateFiles(pathcfg.gameModPath + "//map_data//state_regions//00_regions.txt",
             vic3Gen->vic3Regions);
  Parsing::History::writeBuildings(
      pathcfg.gameModPath + "//common//history//buildings//00_buildings.txt",
      vic3Gen->vic3Regions);
  writeMetadata(pathcfg.gameModPath + "//.metadata//metadata.json");
  strategicRegions(
      pathcfg.gameModPath +
          "//common//strategic_regions//randVic_strategic_regions.txt",
      vic3Gen->strategicRegions, vic3Gen->vic3Regions);
  cultureCommon(pathcfg.gameModPath +
                    "//common//cultures//01_additional_cultures.txt",
                vic3Gen->cultures);
  religionCommon(pathcfg.gameModPath + "//common//religions//religions.txt",
                 vic3Gen->religions);
  countryCommon(pathcfg.gameModPath +
                    "//common//country_definitions//02_custom.txt",
                vic3Gen->vic3Countries, vic3Gen->vic3Regions);
  stateHistory(pathcfg.gameModPath + "//common//history//states//00_states.txt",
               vic3Gen->vic3Regions);
  popsHistory(pathcfg.gameModPath + "//common//history//pops//00_world.txt",
              vic3Gen->vic3Regions);
  countryHistory(pathcfg.gameModPath + "//common//history//countries",
                 vic3Gen->vic3Countries);
  compatFile(pathcfg.gameModPath + "//common//decisions//canal_decisions.txt");
  compatFile(pathcfg.gameModPath + "//events//canal_events.txt");
  compatFile(pathcfg.gameModPath +
             "//events//agitators_events//paris_commune_events.txt");
  compatFile(pathcfg.gameModPath +
             "//events//agitators_events//paris_commune_events.txt");
  compatFile(pathcfg.gameModPath + "//common//journal_entries//00_canals.txt");
  compatFile(pathcfg.gameModPath +
             "//common//journal_entries//02_paris_commune.txt");
  compatTriggers(pathcfg.gamePath + "//game//common//scripted_triggers//",
                 pathcfg.gameModPath + "//common//scripted_triggers//");
}

void Module::writeImages() {
  Gfx::Vic3::FormatConverter formatConverter(pathcfg.gamePath, "Vic3");

  formatConverter.Vic3ColourMaps(
      Fwg::Gfx::MapMerging::mergeTerrain(
          vic3Gen->heightMap, vic3Gen->climateMap, vic3Gen->sobelMap),
      vic3Gen->worldMap, vic3Gen->heightMap, vic3Gen->humidityMap,
      vic3Gen->civLayer, pathcfg.gameModPath + "//gfx//map//");
  // formatConverter.dump8BitRivers(vic3Gen->riverMap,
  //                                pathcfg.gameModPath +
  //                                "//map_data//rivers", "rivers", cut);

  // overwrite the heightmap with the input heightmap, to conserve detail,
  // which would otherwise be lost if the scaled down version had be used
  if (vic3Gen->originalHeightMap.size() > 0) {
    vic3Gen->heightMap = vic3Gen->originalHeightMap;
  }
  formatConverter.detailMaps(vic3Gen->climateData, vic3Gen->civLayer,
                             pathcfg.gameModPath + "//gfx//map//");
  formatConverter.dynamicMasks(pathcfg.gameModPath + "//gfx//map//masks//",
                               vic3Gen->climateData, vic3Gen->civLayer);
  formatConverter.contentSource(pathcfg.gameModPath +
                                    "//content_source//map_objects//masks//",
                                vic3Gen->climateData, vic3Gen->civLayer);
  // save this and reset the heightmap later. The map will be scaled and the
  // scaled one then used for the packed heightmap generation. It is important
  // we reset this after
  auto temporaryHeightmap = vic3Gen->heightMap;
  // also dump uncompressed packed heightmap
  formatConverter.dump8BitHeightmap(
      vic3Gen->heightMap, pathcfg.gameModPath + "//map_data//heightmap",
      "heightmap");
  auto packedHeightmap = formatConverter.dumpPackedHeightmap(
      vic3Gen->heightMap, pathcfg.gameModPath + "//map_data//packed_heightmap",
      "heightmap");
  formatConverter.dumpIndirectionMap(
      vic3Gen->heightMap,
      pathcfg.gameModPath + "//map_data//indirection_heightmap.png");
  Parsing::Writing::heightmap(pathcfg.gameModPath +
                                  "//map_data//heightmap.heightmap",
                              vic3Gen->heightMap, packedHeightmap);
  vic3Gen->heightMap = temporaryHeightmap;
  temporaryHeightmap.clear();
  vic3Gen->visualiseCountries(generator->countryMap);
  Fwg::Gfx::Png::save(vic3Gen->countryMap,
                      Cfg::Values().mapsPath + "countries.png");
  using namespace Fwg::Gfx;
  // just copy over provinces.bmp as a .png, already in a compatible format
  // auto scaledMap = Bmp::scale(vic3Gen->provinceMap, 8192, 3616, false);
  Png::save(vic3Gen->provinceMap,
            pathcfg.gameModPath + "//map_data//provinces.png");
}

void Module::writeSplnet() {
  vic3Gen->createLocators();
  Parsing::Writing::locators(pathcfg.gameModPath +
                                 "//gfx//map//map_object_data//",
                             vic3Gen->vic3Regions);
  vic3Gen->calculateNavalExits();

  Splnet splnet;
  splnet.constructSplnet(vic3Gen->gameRegions);
  splnet.writeFile(pathcfg.gameModPath +
                   "//gfx//map//spline_network//spline_network.splnet");
}

} // namespace Scenario::Vic3