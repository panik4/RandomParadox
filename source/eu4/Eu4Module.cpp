#include "eu4/Eu4Module.h"
using namespace Fwg;

namespace Scenario::Eu4 {
Module::Module(const boost::property_tree::ptree &gamesConf,
               const std::string &configSubFolder,
               const std::string &username) {
  generator = std::make_shared<Scenario::Eu4::Generator>((configSubFolder));
  eu4Gen = std::reinterpret_pointer_cast<Scenario::Eu4::Generator,
                                         Scenario::Generator>(generator);
  // read eu4 configs and potentially overwrite settings for fwg
  readEu4Config(configSubFolder, username, gamesConf);

  // set the executable subpath
  this->executableSubPath = "eu4.exe";
  gameType = GameType::Eu4;
}

Module::~Module() {}

bool Module::createPaths() { // prepare folder structure
  try {
    using namespace std::filesystem;
    // generic cleanup and path creation
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
    create_directory(pathcfg.gameModPath + "//history//diplomacy//");
    create_directory(pathcfg.gameModPath + "//history//provinces//");
    create_directory(pathcfg.gameModPath + "//history//wars//");
    create_directory(pathcfg.gameModPath + "//common//colonial_regions//");
    create_directory(pathcfg.gameModPath + "//common//trade_companies//");
    create_directory(pathcfg.gameModPath + "//common//trade_nodes//");
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
  Fwg::Utils::Logging::logLine("Reading Eu4 Config");
  this->configurePaths(username, "Europa Universalis IV", rpdConf);

  //// now try to locate game files
  // if (!findGame(pathcfg.gamePath, "Europa Universalis IV")) {
  //   //throw(std::exception("Could not locate the game. Exiting"));
  // }
  //// now try to locate game files
  // if (!validateModFolders("Europa Universalis IV")) {
  //   //throw(std::exception("Could not locate the mod folders. Exiting"));
  // }
  auto &config = Cfg::Values();
  namespace pt = boost::property_tree;
  pt::ptree eu4Conf;
  try {
    // Read the basic settings
    std::ifstream f(configSubFolder + "//Europa Universalis IVModule.json");
    std::stringstream buffer;
    if (!f.good())
      Fwg::Utils::Logging::logLine("Config could not be loaded");
    buffer << f.rdbuf();
    Fwg::Parsing::replaceInStringStream(buffer, "//", "//");

    pt::read_json(buffer, eu4Conf);
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
  eu4Gen->numCountries = eu4Conf.get<int>("scenario.numCountries");
  config.seaLevel = 95;
  config.seaProvFactor *= 0.7;
  config.landProvFactor *= 0.7;
  config.loadMapsPath = eu4Conf.get<std::string>("fastworldgen.loadMapsPath");
  cut = config.cut;
  // check if config settings are fine
  config.sanityCheck();
}

void Module::initFormatConverter() {
  formatConverter = Gfx::Eu4::FormatConverter(pathcfg.gamePath, "Eu4");
}

void Module::generate() {
  if (!createPaths())
    return;

  initNameData(Fwg::Cfg::Values().resourcePath + "//names",
               this->pathcfg.gamePath);
  try {
    // start with the generic stuff in the Scenario Generator
    eu4Gen->mapProvinces();
    eu4Gen->mapRegions();

    eu4Gen->mapTerrain();
    eu4Gen->mapContinents();
    Civilization::generateWorldCivilizations(
        eu4Gen->gameRegions, eu4Gen->gameProvinces, eu4Gen->civData,
        eu4Gen->scenContinents);
    eu4Gen->generateCountries<Scenario::Country>();
    eu4Gen->evaluateCountryNeighbours();
    eu4Gen->visualiseCountries(generator->countryMap);
    eu4Gen->generateRegions(eu4Gen->gameRegions);
  } catch (std::exception e) {
    std::string error = "Error while generating the Eu4 Module.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  try {
    // generate map files. Format must be converted and colours mapped to eu4
    // compatible colours
    formatConverter.dump8BitTerrain(
        eu4Gen->terrainData, eu4Gen->climateData, eu4Gen->civLayer,
        pathcfg.gameModPath + "//map//terrain.bmp", "terrain", cut);
    formatConverter.dump8BitRivers(eu4Gen->terrainData, eu4Gen->climateData,
                                   pathcfg.gameModPath + "//map//rivers",
                                   "rivers", cut);
    formatConverter.dump8BitTrees(eu4Gen->terrainData, eu4Gen->climateData,
                                  pathcfg.gameModPath + "//map//trees.bmp",
                                  "trees", false);
    formatConverter.dump8BitHeightmap(eu4Gen->terrainData.detailedHeightMap,
                                      pathcfg.gameModPath + "//map//heightmap",
                                      "heightmap");
    std::vector<Fwg::Gfx::Bitmap> seasonalColourmaps;
    eu4Gen->genSeasons(Cfg::Values(), seasonalColourmaps);
    formatConverter.dumpTerrainColourmap(seasonalColourmaps[0],
                                         eu4Gen->civLayer, pathcfg.gameModPath,
                                         "//map//terrain//colormap_spring.dds",
                                         DXGI_FORMAT_B8G8R8A8_UNORM, 2, cut);
    formatConverter.dumpTerrainColourmap(seasonalColourmaps[1],
                                         eu4Gen->civLayer, pathcfg.gameModPath,
                                         "//map//terrain//colormap_summer.dds",
                                         DXGI_FORMAT_B8G8R8A8_UNORM, 2, cut);
    formatConverter.dumpTerrainColourmap(seasonalColourmaps[2],
                                         eu4Gen->civLayer, pathcfg.gameModPath,
                                         "//map//terrain//colormap_autumn.dds",
                                         DXGI_FORMAT_B8G8R8A8_UNORM, 2, cut);
    formatConverter.dumpTerrainColourmap(seasonalColourmaps[3],
                                         eu4Gen->civLayer, pathcfg.gameModPath,
                                         "//map//terrain//colormap_winter.dds",
                                         DXGI_FORMAT_B8G8R8A8_UNORM, 2, cut);
    formatConverter.dumpDDSFiles(
        eu4Gen->terrainData.detailedHeightMap,
        pathcfg.gameModPath + "//map//terrain//colormap_water", cut, 2);
    formatConverter.dumpWorldNormal(
        Fwg::Gfx::Bitmap(Cfg::Values().width, Cfg::Values().height, 24,
                         eu4Gen->terrainData.sobelData),
        pathcfg.gameModPath + "//map//world_normal.bmp", cut);

    using namespace Fwg::Gfx;
    // just copy over provinces.bmp, already in a compatible format
    Bmp::save(eu4Gen->provinceMap,
              pathcfg.gameModPath + "//map//provinces.bmp");
    {
      using namespace Parsing;
      // now do text
      writeAdj(pathcfg.gameModPath + "//map//adjacencies.csv",
               eu4Gen->gameProvinces);
      writeAmbientObjects(pathcfg.gameModPath + "//map//ambient_object.txt",
                          eu4Gen->gameProvinces);
      writeAreas(pathcfg.gameModPath + "//map//area.txt", eu4Gen->gameRegions,
                 pathcfg.gamePath);
      writeColonialRegions(
          pathcfg.gameModPath +
              "//common//colonial_regions//00_colonial_regions.txt",
          pathcfg.gamePath, eu4Gen->gameProvinces);
      writeClimate(pathcfg.gameModPath + "//map//climate.txt",
                   eu4Gen->gameProvinces);
      writeContinent(pathcfg.gameModPath + "//map//continent.txt",
                     eu4Gen->gameProvinces);
      writeDefaultMap(pathcfg.gameModPath + "//map//default.map",
                      eu4Gen->gameProvinces);
      writeDefinition(pathcfg.gameModPath + "//map//definition.csv",
                      eu4Gen->gameProvinces);
      writePositions(pathcfg.gameModPath + "//map//positions.txt",
                     eu4Gen->gameProvinces);
      writeRegions(pathcfg.gameModPath + "//map//region.txt", pathcfg.gamePath,
                   eu4Gen->getEu4Regions());
      writeSuperregion(pathcfg.gameModPath + "//map//superregion.txt",
                       pathcfg.gamePath, eu4Gen->gameRegions);
      writeTerrain(pathcfg.gameModPath + "//map//terrain.txt",
                   eu4Gen->gameProvinces);
      writeTradeCompanies(
          pathcfg.gameModPath +
              "//common//trade_companies//00_trade_companies.txt",
          pathcfg.gamePath, eu4Gen->gameProvinces);
      writeTradewinds(pathcfg.gameModPath + "//map//trade_winds.txt",
                      eu4Gen->gameProvinces);

      copyDescriptorFile(
          Fwg::Cfg::Values().resourcePath + "//eu4//descriptor.mod",
          pathcfg.gameModPath, pathcfg.gameModsDirectory, pathcfg.modName);

      writeProvinces(pathcfg.gameModPath + "//history//provinces//",
                     eu4Gen->gameProvinces, eu4Gen->gameRegions);
      writeLoc(pathcfg.gameModPath + "//localisation//", pathcfg.gamePath,
               eu4Gen->gameRegions, eu4Gen->gameProvinces,
               eu4Gen->getEu4Regions());
      Fwg::Utils::Logging::logLine("Done with the eu4 export");
    }
  } catch (std::exception e) {
    std::string error = "Error while dumping and writing files.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
}

} // namespace Scenario::Eu4