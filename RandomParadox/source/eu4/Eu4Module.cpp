#include "eu4/Eu4Module.h"
using namespace Fwg;

namespace Scenario::Eu4 {
Module::Module(const boost::property_tree::ptree &gamesConf,
               const std::string &configSubFolder,
               const std::string &username) {
  FastWorldGenerator fwg(configSubFolder);
  // read eu4 configs and potentially overwrite settings for fwg
  readEu4Config(configSubFolder, username, gamesConf);
  // now run the world generation
  fwg.generateWorld();
  eu4Gen = {fwg};
  eu4Gen.nData = NameGeneration::prepare("resources\\names", gamePath);
}

Module::~Module() {}

bool Module::createPaths() { // prepare folder structure
  try {
    using namespace std::filesystem;
    // generic cleanup and path creation
    GenericModule::createPaths(gameModPath);
    create_directory(gameModPath + "\\history\\diplomacy\\");
    create_directory(gameModPath + "\\history\\provinces\\");
    create_directory(gameModPath + "\\history\\wars\\");
    create_directory(gameModPath + "\\common\\colonial_regions\\");
    create_directory(gameModPath + "\\common\\trade_companies\\");
    create_directory(gameModPath + "\\common\\trade_nodes\\");
    return true;
  } catch (std::exception e) {
    std::string error = "Configured paths seem to be messed up, check Europa "
                        "Universalis IVModule.json\n";
    error += "You can try fixing it yourself. Error is:\n ";
    error += e.what();
    throw(std::exception(error.c_str()));
    return false;
  }
}

void Module::readEu4Config(const std::string &configSubFolder,
                           const std::string &username,
                           const boost::property_tree::ptree &rpdConf) {
  Utils::Logging::logLine("Reading Eu4 Config");
  this->configurePaths(username, "Europa Universalis IV", rpdConf);

  // now try to locate game files
  if (!findGame(gamePath, "Europa Universalis IV")) {
    throw(std::exception("Could not locate the game. Exiting"));
  }
  // now try to locate game files
  if (!findModFolders()) {
    throw(std::exception("Could not locate the mod folders. Exiting"));
  }
  auto &config = Env::Instance();
  namespace pt = boost::property_tree;
  pt::ptree eu4Conf;
  try {
    // Read the basic settings
    std::ifstream f(configSubFolder + "//Europa Universalis IVModule.json");
    std::stringstream buffer;
    if (!f.good())
      Utils::Logging::logLine("Config could not be loaded");
    buffer << f.rdbuf();

    pt::read_json(buffer, eu4Conf);
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
  numCountries = eu4Conf.get<int>("scenario.numCountries");
  config.seaLevel = 95;
  config.seaProvFactor *= 0.5;
  config.landProvFactor *= 0.4;
  config.loadMapsPath = eu4Conf.get<std::string>("fastworldgen.loadMapsPath");
  config.heightmapIn = config.loadMapsPath +
                       eu4Conf.get<std::string>("fastworldgen.heightMapName");
  cut = config.cut;
  // check if config settings are fine
  config.sanityCheck();
}

void Module::genEu4() {
  if (!createPaths())
    return;

  try {
    // start with the generic stuff in the Scenario Generator
    eu4Gen.mapRegions();
    eu4Gen.mapContinents();
    eu4Gen.generateCountries(numCountries, gamePath);
    eu4Gen.evaluateNeighbours();
    eu4Gen.generateWorld();
    eu4Gen.dumpDebugCountrymap(Env::Instance().mapsPath + "countries.bmp");
    eu4Gen.generateRegions(eu4Gen.gameRegions);
  } catch (std::exception e) {
    std::string error = "Error while generating the Eu4 Module.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  try {
    // generate map files. Format must be converted and colours mapped to eu4
    // compatible colours
    Gfx::FormatConverter formatConverter(gamePath, "Eu4");
    formatConverter.dump8BitTerrain(eu4Gen.fwg.climateMap,
                                    gameModPath + "\\map\\terrain.bmp",
                                    "terrain", cut);
    formatConverter.dump8BitRivers(
        eu4Gen.fwg.riverMap, gameModPath + "\\map\\rivers.bmp", "rivers", cut);
    formatConverter.dump8BitTrees(eu4Gen.fwg.climateMap, eu4Gen.fwg.treeMap,
                                  gameModPath + "\\map\\trees.bmp", "trees",
                                  false);
    formatConverter.dump8BitHeightmap(eu4Gen.fwg.heightMap,
                                      gameModPath + "\\map\\heightmap.bmp",
                                      "heightmap");
    formatConverter.dumpTerrainColourmap(
        eu4Gen.fwg.springMap, eu4Gen.fwg.cityMap, gameModPath,
        "\\map\\terrain\\colormap_spring.dds", DXGI_FORMAT_B8G8R8A8_UNORM, cut);
    formatConverter.dumpTerrainColourmap(
        eu4Gen.fwg.summerMap, eu4Gen.fwg.cityMap, gameModPath,
        "\\map\\terrain\\colormap_summer.dds", DXGI_FORMAT_B8G8R8A8_UNORM, cut);
    formatConverter.dumpTerrainColourmap(
        eu4Gen.fwg.autumnMap, eu4Gen.fwg.cityMap, gameModPath,
        "\\map\\terrain\\colormap_autumn.dds", DXGI_FORMAT_B8G8R8A8_UNORM, cut);
    formatConverter.dumpTerrainColourmap(
        eu4Gen.fwg.winterMap, eu4Gen.fwg.cityMap, gameModPath,
        "\\map\\terrain\\colormap_winter.dds", DXGI_FORMAT_B8G8R8A8_UNORM, cut);
    formatConverter.dumpDDSFiles(eu4Gen.fwg.riverMap, eu4Gen.fwg.heightMap,
                                 gameModPath + "\\map\\terrain\\colormap_water",
                                 cut, 2);
    formatConverter.dumpWorldNormal(
        eu4Gen.fwg.sobelMap, gameModPath + "\\map\\world_normal.bmp", cut);

    using namespace Fwg::Gfx;
    // just copy over provinces.bmp, already in a compatible format
    Bmp::save(eu4Gen.fwg.provinceMap, gameModPath + "\\map\\provinces.bmp");
    {
      using namespace Parsing;
      // now do text
      writeAdj(gameModPath + "\\map\\adjacencies.csv", eu4Gen.gameProvinces);
      writeAmbientObjects(gameModPath + "\\map\\ambient_object.txt",
                          eu4Gen.gameProvinces);
      writeAreas(gameModPath + "\\map\\area.txt", eu4Gen.gameRegions, gamePath);
      writeColonialRegions(
          gameModPath + "\\common\\colonial_regions\\00_colonial_regions.txt",
          gamePath, eu4Gen.gameProvinces);
      writeClimate(gameModPath + "\\map\\climate.txt", eu4Gen.gameProvinces);
      writeContinent(gameModPath + "\\map\\continent.txt",
                     eu4Gen.gameProvinces);
      writeDefaultMap(gameModPath + "\\map\\default.map", eu4Gen.gameProvinces);
      writeDefinition(gameModPath + "\\map\\definition.csv",
                      eu4Gen.gameProvinces);
      writePositions(gameModPath + "\\map\\positions.txt",
                     eu4Gen.gameProvinces);
      writeRegions(gameModPath + "\\map\\region.txt", gamePath,
                   eu4Gen.getEu4Regions());
      writeSuperregion(gameModPath + "\\map\\superregion.txt", gamePath,
                       eu4Gen.gameRegions);
      writeTerrain(gameModPath + "\\map\\terrain.txt", eu4Gen.gameProvinces);
      writeTradeCompanies(
          gameModPath + "\\common\\trade_companies\\00_trade_companies.txt",
          gamePath, eu4Gen.gameProvinces);
      writeTradewinds(gameModPath + "\\map\\trade_winds.txt",
                      eu4Gen.gameProvinces);

      copyDescriptorFile("resources\\eu4\\descriptor.mod", gameModPath,
                         gameModsDirectory, modName);

      writeProvinces(gameModPath + "\\history\\provinces\\",
                     eu4Gen.gameProvinces, eu4Gen.gameRegions);
      writeLoc(gameModPath + "\\localisation\\", gamePath, eu4Gen.gameRegions,
               eu4Gen.gameProvinces, eu4Gen.getEu4Regions());
    }

  } catch (std::exception e) {
    std::string error = "Error while dumping and writing files.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
}

} // namespace Scenario::Eu4