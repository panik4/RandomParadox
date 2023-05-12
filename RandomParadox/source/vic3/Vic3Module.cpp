#include "vic3/Vic3Module.h"
using namespace Fwg;

namespace Scenario::Vic3 {
Module::Module(const boost::property_tree::ptree &gamesConf,
               const std::string &configSubFolder,
               const std::string &username) {
  vic3Gen = Vic3::Generator(configSubFolder);
  // read eu4 configs and potentially overwrite settings for fwg
  readVic3Config(configSubFolder, username, gamesConf);
  // now run the world generation
  vic3Gen.generateWorld();
  vic3Gen.nData = NameGeneration::prepare("resources\\names", gamePath + "");
}

Module::~Module() {}

bool Module::createPaths() { // prepare folder structure
  try {
    using namespace std::filesystem;
    create_directory(gameModPath);
    // generic cleanup and path creation
    // remove_all(gameModPath + "\\map_data\\");
    remove_all(gameModPath + "\\common\\");
    remove_all(gameModPath + "\\localization\\");
    create_directory(gameModPath);
    create_directory(gameModPath + "\\.metadata\\");
    create_directory(gameModPath + "\\map_data\\");
    create_directory(gameModPath + "\\map_data\\state_regions\\");
    create_directory(gameModPath + "\\common\\");
    create_directory(gameModPath + "\\common\\strategic_regions");
    create_directory(gameModPath + "\\common\\cultures");
    create_directory(gameModPath + "\\common\\religions");
    create_directory(gameModPath + "\\common\\country_definitions");
    create_directory(gameModPath + "\\common\\history");
    create_directory(gameModPath + "\\common\\history\\states");
    create_directory(gameModPath + "\\common\\history\\pops");
    create_directory(gameModPath + "\\common\\history\\countries");
    create_directory(gameModPath + "\\common\\country_creation");
    create_directory(gameModPath + "\\gfx\\");
    create_directory(gameModPath + "\\gfx\\map");
    create_directory(gameModPath + "\\gfx\\map\\terrain");
    create_directory(gameModPath + "\\gfx\\map\\textures");
    create_directory(gameModPath + "\\gfx\\map\\water");
    create_directory(gameModPath + "\\gfx\\map\\spline_network");
    create_directory(gameModPath + "\\localization\\");

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
  Utils::Logging::logLine("Reading Vic 3 Config");
  this->configurePaths(username, "Victoria 3", rpdConf);

  // now try to locate game files
  if (!findGame(gamePath, "Victoria 3")) {
    throw(std::exception("Could not locate the game. Exiting"));
  }
  // now try to locate game files
  if (!findModFolders()) {
    throw(std::exception("Could not locate the mod folders. Exiting"));
  }
  auto &config = Cfg::Values();
  namespace pt = boost::property_tree;
  pt::ptree vic3Conf;
  try {
    // Read the basic settings
    std::ifstream f(configSubFolder + "//Victoria3Module.json");
    std::stringstream buffer;
    if (!f.good())
      Utils::Logging::logLine("Config could not be loaded");
    buffer << f.rdbuf();

    pt::read_json(buffer, vic3Conf);
  } catch (std::exception e) {
    Utils::Logging::logLine(
        "Incorrect config \"Europa Universalis IVModule.json\"");
    Utils::Logging::logLine("You can try fixing it yourself. Error is: ",
                            e.what());
    Utils::Logging::logLine(
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or search for \"json validator\"");
    system("pause");
  }
  //  passed to generic ScenarioGenerator
  numCountries = vic3Conf.get<int>("scenario.numCountries");
  config.seaLevel = 18;
  config.numRivers = 0;
  config.seaProvFactor *= 0.02;
  config.landProvFactor *= 1.0;
  config.loadMapsPath = vic3Conf.get<std::string>("fastworldgen.loadMapsPath");
  config.heightmapIn = config.loadMapsPath +
                       vic3Conf.get<std::string>("fastworldgen.heightMapName");
  cut = config.cut;
  // check if config settings are fine
  config.sanityCheck();
}

void Module::genVic3() {
  if (!createPaths())
    return;

  try {
    // start with the generic stuff in the Scenario Generator
    vic3Gen.mapProvinces();
    vic3Gen.mapRegions();
    vic3Gen.mapTerrain();
    vic3Gen.mapContinents();
    vic3Gen.generateCountries(numCountries, gamePath);
    vic3Gen.evaluateNeighbours();
    vic3Gen.generateWorldCivilizations();
    vic3Gen.dumpDebugCountrymap(Cfg::Values().mapsPath + "countries.bmp");
    vic3Gen.generateRegions(vic3Gen.gameRegions);
    // Vic3 specifics:
    vic3Gen.generateStrategicRegions();

  } catch (std::exception e) {
    std::string error = "Error while generating the Vic3 Module.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  try {
    using namespace Parsing::Writing;
    compatRegions(gamePath + "\\game\\map_data\\state_regions\\",
                  gameModPath + "\\map_data\\state_regions\\",
                  vic3Gen.gameRegions);
    compatStratRegions(gamePath + "\\game\\common\\strategic_regions\\",
                       gameModPath + "\\common\\strategic_regions\\");
    compatReleasable(gamePath + "\\game\\common\\country_creation\\",
                     gameModPath + "\\common\\country_creation\\");
    adj(gameModPath + "\\map_data\\adjacencies.csv");
    defaultMap(gameModPath + "\\map_data\\default.map", vic3Gen.gameProvinces);
    provinceTerrains(gameModPath + "\\map_data\\province_terrains.txt",
                     vic3Gen.gameProvinces);
    heightmap(gameModPath + "\\map_data\\heightmap.heightmap",
              vic3Gen.heightMap);
    stateFiles(gameModPath + "\\map_data\\state_regions\\00_regions.txt",
               vic3Gen.gameRegions);
    writeMetadata(gameModPath + "\\.metadata\\metadata.json");
    strategicRegions(
        gameModPath +
            "\\common\\strategic_regions\\randVic_strategic_regions.txt",
        vic3Gen.strategicRegions, vic3Gen.gameRegions);
    cultureCommon(gameModPath +
                      "\\common\\cultures\\01_additional_cultures.txt",
                  vic3Gen.cultures);
    religionCommon(gameModPath + "\\common\\religions\\religions.txt",
                   vic3Gen.religions);
    countryCommon(gameModPath + "\\common\\country_definitions\\02_custom.txt",
                  vic3Gen.countries, vic3Gen.gameRegions);
    stateHistory(gameModPath + "\\common\\history\\states\\00_states.txt",
                 vic3Gen.gameRegions);
    popsHistory(gameModPath + "\\common\\history\\pops\\00_world.txt",
                vic3Gen.gameRegions);
    countryHistory(gameModPath + "\\common\\history\\countries",
                   vic3Gen.countries);
    splineNetwork(gameModPath + "\\gfx\\map\\spline_network\\");

    //  generate map files. Format must be converted and colours mapped to vic3
    //  compatible colours
    Gfx::FormatConverter formatConverter(gamePath, "Vic3");
    if (true) {

      formatConverter.Vic3ColourMaps(vic3Gen.climateMap, vic3Gen.treeMap,
                                     vic3Gen.heightMap, vic3Gen.humidityMap,
                                     gameModPath + "\\gfx\\map\\");
      formatConverter.dump8BitRivers(
          vic3Gen.riverMap, gameModPath + "\\map_data\\rivers", "rivers", cut);

      // overwrite the heightmap with the input heightmap, to conserve detail,
      // which would otherwise be lost if the scaled down version had be used
      if (vic3Gen.originalHeightMap.size() > 0) {
        vic3Gen.heightMap = vic3Gen.originalHeightMap;
      }

      // also dump uncompressed packed heightmap
      formatConverter.dump8BitHeightmap(vic3Gen.heightMap,
                                        gameModPath + "\\map_data\\heightmap",
                                        "heightmap");
      formatConverter.dumpPackedHeightmap(
          vic3Gen.heightMap, gameModPath + "\\map_data\\packed_heightmap",
          "heightmap");
      formatConverter.detailIndexMap(vic3Gen.climateMap,
                                     gameModPath + "\\gfx\\map\\");
      using namespace Fwg::Gfx;
      // just copy over provinces.bmp as a .png, already in a compatible format
      auto scaledMap = Bmp::scale(vic3Gen.provinceMap, 8192, 3616, false);
      Png::save(scaledMap, gameModPath + "\\map_data\\provinces.png");

      // copy indirection_heightmap.png from resources
      const auto copyOptions = std::filesystem::copy_options::update_existing;
      std::filesystem::copy(
          "resources\\vic3\\map_data\\indirection_heightmap.png",
          gameModPath + "\\map_data\\indirection_heightmap.png", copyOptions);
    }

  } catch (std::exception e) {
    std::string error = "Error while dumping and writing files.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
}

} // namespace Scenario::Vic3