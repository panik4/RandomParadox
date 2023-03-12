#include "vic3/Vic3Module.h"
using namespace Fwg;

namespace Scenario::Vic3 {
Module::Module(const boost::property_tree::ptree &gamesConf,
               const std::string &configSubFolder,
               const std::string &username) {
  FastWorldGenerator fwg(configSubFolder);
  // read eu4 configs and potentially overwrite settings for fwg
  readVic3Config(configSubFolder, username, gamesConf);
  // now run the world generation
  fwg.generateWorld();
  vic3Gen = {fwg};
  vic3Gen.nData = NameGeneration::prepare("resources\\names", gamePath + "");
}

Module::~Module() {}

bool Module::createPaths() { // prepare folder structure
  try {
    using namespace std::filesystem;
    create_directory(gameModPath);
    // generic cleanup and path creation
    std::cout << gameModPath << std::endl;
    remove_all(gameModPath + "\\map_data\\");
    remove_all(gameModPath + "\\common\\");
    remove_all(gameModPath + "\\localization\\");
    create_directory(gameModPath);
    create_directory(gameModPath + "\\.metadata\\");
    create_directory(gameModPath + "\\map_data\\");
    create_directory(gameModPath + "\\map_data\\state_regions\\");
    create_directory(gameModPath + "\\common\\");
    create_directory(gameModPath + "\\gfx\\");
    create_directory(gameModPath + "\\gfx\\map");
    create_directory(gameModPath + "\\gfx\\map\\terrain");
    create_directory(gameModPath + "\\gfx\\map\\textures");
    create_directory(gameModPath + "\\gfx\\map\\water");
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
  config.seaLevel = 14;
  config.numRivers = 0;
  config.seaProvFactor *= 0.3;
  config.landProvFactor *= 0.7;
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
    vic3Gen.mapContinents();
    vic3Gen.generateCountries(numCountries, gamePath);
    vic3Gen.evaluateNeighbours();
    vic3Gen.generateWorld();
    vic3Gen.dumpDebugCountrymap(Cfg::Values().mapsPath + "countries.bmp");
    vic3Gen.generateRegions(vic3Gen.gameRegions);
  } catch (std::exception e) {
    std::string error = "Error while generating the Vic3 Module.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  // try {
  //  generate map files. Format must be converted and colours mapped to vic3
  //  compatible colours
  Gfx::FormatConverter formatConverter(gamePath, "Vic3");
  formatConverter.Vic3ColourMaps(vic3Gen.fwg.climateMap, vic3Gen.fwg.treeMap,
                                 vic3Gen.fwg.heightMap,
                                 gameModPath + "\\gfx\\map\\");
  formatConverter.dump8BitRivers(
      vic3Gen.fwg.riverMap, gameModPath + "\\map_data\\rivers", "rivers", cut);

  formatConverter.dumpPackedHeightmap(
      vic3Gen.fwg.heightMap, gameModPath + "\\map_data\\packed_heightmap",
      "heightmap");
  // also dump uncompressed packed heightmap
  formatConverter.dump8BitHeightmap(vic3Gen.fwg.heightMap,
                                    gameModPath + "\\map_data\\heightmap",
                                    "heightmap");
  formatConverter.detailIndexMap(vic3Gen.fwg.climateMap,
                                 gameModPath + "\\gfx\\map\\");
  using namespace Fwg::Gfx;
  // just copy over provinces.bmp as a .png, already in a compatible format
  auto scaledMap = Bmp::scale(vic3Gen.fwg.provinceMap, 8192, 3616, false);
  Png::save(scaledMap, gameModPath + "\\map_data\\provinces.png");

  using namespace Parsing::Writing;
  adj(gameModPath + "\\map_data\\adjacencies.csv");
  defaultMap(gameModPath + "\\map_data\\default.map", vic3Gen.gameProvinces);
  provinceTerrains(gameModPath + "\\map_data\\province_terrains.txt",
                   vic3Gen.gameProvinces);
  heightmap(gameModPath + "\\map_data\\heightmap.heightmap",
            vic3Gen.fwg.heightMap);
  stateFiles(gameModPath + "\\map_data\\state_regions\\00_regions.txt",
             vic3Gen.gameRegions);
  writeMetadata(gameModPath + "\\.metadata\\metadata.json");

  /* } catch (std::exception e) {
     std::string error = "Error while dumping and writing files.\n";
     error += "Error is: \n";
     error += e.what();
     throw(std::exception(error.c_str()));
   }*/
}

} // namespace Scenario::Vic3