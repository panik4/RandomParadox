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
  vic3Gen->nData =
      NameGeneration::prepare("resources//names", pathcfg.gamePath + "");
}

Module::~Module() {}

bool Module::createPaths() { // prepare folder structure
  try {
    using namespace std::filesystem;
    create_directory(pathcfg.gameModPath);
    // generic cleanup and path creation
    // remove_all(pathcfg.gameModPath + "//map_data//");
    remove_all(pathcfg.gameModPath + "//common//");
    remove_all(pathcfg.gameModPath + "//localization//");
    create_directory(pathcfg.gameModPath);
    create_directory(pathcfg.gameModPath + "//.metadata//");
    create_directory(pathcfg.gameModPath + "//map_data//");
    create_directory(pathcfg.gameModPath + "//map_data//state_regions//");
    create_directory(pathcfg.gameModPath + "//common//");
    create_directory(pathcfg.gameModPath + "//common//strategic_regions");
    create_directory(pathcfg.gameModPath + "//common//cultures");
    create_directory(pathcfg.gameModPath + "//common//religions");
    create_directory(pathcfg.gameModPath + "//common//country_definitions");
    create_directory(pathcfg.gameModPath + "//common//history");
    create_directory(pathcfg.gameModPath + "//common//history//states");
    create_directory(pathcfg.gameModPath + "//common//history//pops");
    create_directory(pathcfg.gameModPath + "//common//history//countries");
    create_directory(pathcfg.gameModPath + "//common//country_creation");
    create_directory(pathcfg.gameModPath + "//common//journal_entries");
    create_directory(pathcfg.gameModPath + "//common//decisions");
    create_directory(pathcfg.gameModPath + "//events");
    create_directory(pathcfg.gameModPath + "//events//agitators_events");
    create_directory(pathcfg.gameModPath + "//gfx//");
    create_directory(pathcfg.gameModPath + "//gfx//map");
    create_directory(pathcfg.gameModPath + "//gfx//map//terrain");
    create_directory(pathcfg.gameModPath + "//gfx//map//textures");
    create_directory(pathcfg.gameModPath + "//gfx//map//water");
    create_directory(pathcfg.gameModPath + "//gfx//map//spline_network");
    create_directory(pathcfg.gameModPath + "//localization//");

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

  //// now try to locate game files
  // if (!findGame(pathcfg.gamePath, "Victoria 3")) {
  //   //throw(std::exception("Could not locate the game. Exiting"));
  // }
  //// now try to locate game files
  // if (!validateModFolders("Victoria 3")) {
  //   //throw(std::exception("Could not locate the mod folders. Exiting"));
  // }
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
  config.seaProvFactor *= 0.02;
  config.landProvFactor *= 1.0;
  config.minProvPerSeaRegion = 1;
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

  try {
    // start with the generic stuff in the Scenario Generator
    vic3Gen->mapProvinces();
    vic3Gen->mapRegions();
    vic3Gen->mapTerrain();
    vic3Gen->mapContinents();
    vic3Gen->generateCountries();
    vic3Gen->evaluateNeighbours();
    vic3Gen->generateWorldCivilizations();
    vic3Gen->dumpDebugCountrymap(Cfg::Values().mapsPath + "countries.png");
    vic3Gen->generateRegions(vic3Gen->gameRegions);
    // Vic3 specifics:
    vic3Gen->generateStrategicRegions();

  } catch (std::exception e) {
    std::string error = "Error while generating the Vic3 Module.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  try {
    using namespace Parsing::Writing;
    auto foundRegions =
        compatRegions(pathcfg.gamePath + "//game//map_data//state_regions//",
                      pathcfg.gameModPath + "//map_data//state_regions//",
                      vic3Gen->gameRegions);
    compatStratRegions(pathcfg.gamePath + "//game//common//strategic_regions//",
                       pathcfg.gameModPath + "//common//strategic_regions//",
                       vic3Gen->gameRegions, foundRegions);
    compatReleasable(pathcfg.gamePath + "//game//common//country_creation//",
                     pathcfg.gameModPath + "//common//country_creation//");
    adj(pathcfg.gameModPath + "//map_data//adjacencies.csv");
    defaultMap(pathcfg.gameModPath + "//map_data//default.map",
               vic3Gen->gameProvinces);
    provinceTerrains(pathcfg.gameModPath + "//map_data//province_terrains.txt",
                     vic3Gen->gameProvinces);
    heightmap(pathcfg.gameModPath + "//map_data//heightmap.heightmap",
              vic3Gen->heightMap);
    stateFiles(pathcfg.gameModPath +
                   "//map_data//state_regions//00_regions.txt",
               vic3Gen->gameRegions);
    writeMetadata(pathcfg.gameModPath + "//.metadata//metadata.json");
    strategicRegions(
        pathcfg.gameModPath +
            "//common//strategic_regions//randVic_strategic_regions.txt",
        vic3Gen->strategicRegions, vic3Gen->gameRegions);
    cultureCommon(pathcfg.gameModPath +
                      "//common//cultures//01_additional_cultures.txt",
                  vic3Gen->cultures);
    religionCommon(pathcfg.gameModPath + "//common//religions//religions.txt",
                   vic3Gen->religions);
    countryCommon(pathcfg.gameModPath +
                      "//common//country_definitions//02_custom.txt",
                  vic3Gen->countries, vic3Gen->gameRegions);
    stateHistory(pathcfg.gameModPath +
                     "//common//history//states//00_states.txt",
                 vic3Gen->gameRegions);
    popsHistory(pathcfg.gameModPath + "//common//history//pops//00_world.txt",
                vic3Gen->gameRegions);
    countryHistory(pathcfg.gameModPath + "//common//history//countries",
                   vic3Gen->countries);
    splineNetwork(pathcfg.gameModPath + "//gfx//map//spline_network//");
    compatCanals(pathcfg.gameModPath +
                 "//common//decisions//canal_decisions.txt");
    compatCanals(pathcfg.gameModPath + "//events//canal_events.txt");
    compatCanals(pathcfg.gameModPath +
                 "//events//agitators_events//paris_commune_events.txt");
    compatCanals(pathcfg.gameModPath +
                 "//events//agitators_events//paris_commune_events.txt");
    compatCanals(pathcfg.gameModPath +
                 "//common//journal_entries//00_canals.txt");
    compatCanals(pathcfg.gameModPath +
                 "//common//journal_entries//02_paris_commune.txt");

    //  generate map files. Format must be converted and colours mapped to vic3
    //  compatible colours
    Gfx::FormatConverter formatConverter(pathcfg.gamePath, "Vic3");
    if (true) {

      formatConverter.Vic3ColourMaps(vic3Gen->climateMap, vic3Gen->treeMap,
                                     vic3Gen->heightMap, vic3Gen->humidityMap,
                                     pathcfg.gameModPath + "//gfx//map//");
      formatConverter.dump8BitRivers(vic3Gen->riverMap,
                                     pathcfg.gameModPath + "//map_data//rivers",
                                     "rivers", cut);

      // overwrite the heightmap with the input heightmap, to conserve detail,
      // which would otherwise be lost if the scaled down version had be used
      if (vic3Gen->originalHeightMap.size() > 0) {
        vic3Gen->heightMap = vic3Gen->originalHeightMap;
      }

      // also dump uncompressed packed heightmap
      //formatConverter.dump8BitHeightmap(
      //    vic3Gen->heightMap, pathcfg.gameModPath + "//map_data//heightmap",
      //    "heightmap");
      formatConverter.dumpPackedHeightmap(
          vic3Gen->heightMap,
          pathcfg.gameModPath + "//map_data//packed_heightmap", "heightmap");
      formatConverter.detailIndexMap(vic3Gen->climateMap,
                                     pathcfg.gameModPath + "//gfx//map//");
      using namespace Fwg::Gfx;
      // just copy over provinces.bmp as a .png, already in a compatible format
      auto scaledMap = Bmp::scale(vic3Gen->provinceMap, 8192, 3616, false);
      Png::save(scaledMap, pathcfg.gameModPath + "//map_data//provinces.png");

      // copy indirection_heightmap.png from resources
      const auto copyOptions = std::filesystem::copy_options::update_existing;
      std::filesystem::copy(
          "resources//vic3//map_data//indirection_heightmap.png",
          pathcfg.gameModPath + "//map_data//indirection_heightmap.png",
          copyOptions);
    }
  } catch (std::exception e) {
    std::string error = "Error while dumping and writing files.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
}

} // namespace Scenario::Vic3