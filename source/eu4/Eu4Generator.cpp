#include "eu4/Eu4Generator.h"
namespace Rpx::Eu4 {
using namespace Fwg;
using namespace Fwg::Gfx;

Generator::Generator(const std::string &configSubFolder,
                     const boost::property_tree::ptree &rpdConf)
    : Rpx::ModGenerator(configSubFolder, GameType::Eu4, "eu4.exe", rpdConf) {
  configureModGen(configSubFolder, Fwg::Cfg::Values().username, rpdConf);
}

bool Generator::createPaths() { // prepare folder structure
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

void Generator::configureModGen(const std::string &configSubFolder,
                                const std::string &username,
                                const boost::property_tree::ptree &rpdConf) {
  Fwg::Utils::Logging::logLine("Reading Eu4 Config");
  Rpx::Utils::configurePaths(username, "Europa Universalis IV", rpdConf,
                             this->pathcfg);
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
  ardaConfig.numCountries = eu4Conf.get<int>("scenario.numCountries");
  ardaConfig.generationAge = Arda::Utils::GenerationAge::Renaissance;
  ardaConfig.targetWorldPopulation = 500'000'000;
  ardaConfig.targetWorldGdp = 50'000'000'000;
  config.maxImageArea = 5632 * 2304;
  config.resolutionBase = 256;
  config.seaLevel = 95;
  config.seaProvFactor *= 0.7;
  config.landProvFactor *= 0.7;
  config.loadMapsPath = eu4Conf.get<std::string>("fastworldgen.loadMapsPath");
  // check if config settings are fine
  config.sanityCheck();
}

void Generator::generateRegions(
    std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions) {
  Fwg::Utils::Logging::logLine("Eu4: Dividing world into strategic regions");
  std::set<int> assignedIdeas;
  for (auto &region : regions) {
    if (assignedIdeas.find(region->ID) == assignedIdeas.end()) {
      eu4Region euR;
      // std::set<int>stratRegion;
      euR.areaIDs.insert(region->ID);
      assignedIdeas.insert(region->ID);
      for (auto &neighbour : region->neighbours) {
        // should be equal in sea/land
        if (neighbour > regions.size())
          continue;
        if (regions[neighbour]->isSea() == region->isSea() &&
            assignedIdeas.find(neighbour) == assignedIdeas.end()) {
          euR.areaIDs.insert(neighbour);
          assignedIdeas.insert(neighbour);
        }
      }
      eu4regions.push_back(euR);
    }
  }
  Bitmap eu4RegionBmp(Cfg::Values().width, Cfg::Values().height, 24);
  for (auto &stratRegion : eu4regions) {
    Colour c{
        static_cast<unsigned char>(RandNum::getRandom<unsigned char>() % 255),
        static_cast<unsigned char>(RandNum::getRandom<unsigned char>() % 255),
        static_cast<unsigned char>(RandNum::getRandom<unsigned char>() % 255)};
    for (auto &area : stratRegion.areaIDs) {
      c.setBlue(regions[area]->isSea() ? 255 : 0);
      for (auto &prov : regions[area]->ardaProvinces) {
        for (auto &pix : prov->pixels) {
          eu4RegionBmp.setColourAtIndex(pix, c);
        }
      }
    }
  }
  Bmp::bufferBitmap("eu4regions", eu4RegionBmp);
  Bmp::save(eu4RegionBmp, Fwg::Cfg::Values().mapsPath + "//eu4Regions.bmp");
}

Fwg::Gfx::Bitmap Generator::mapTerrain() {
  return Fwg::Gfx::Bitmap(Cfg::Values().width, Cfg::Values().height, 24);
}

// initialize states
void Generator::mapCountries() {}

void Generator::initImageExporter() {
  formatConverter = Gfx::Eu4::ImageExporter(pathcfg.gamePath, "Eu4");
}
void Generator::writeTextFiles() {}
void Generator::writeImages() {}
void Generator::generate() {
  if (!createPaths())
    return;

  try {
    // start with the generic stuff in the Scenario Generator
    mapProvinces();
    mapRegions();

    mapTerrain();
    mapContinents();
    genCivilisationData();

    auto countryFactory = []() -> std::shared_ptr<Arda::Country> {
      return std::make_shared<Arda::Country>();
    };
    // generate country data
    generateCountries(countryFactory);

    generateRegions(ardaRegions);
  } catch (std::exception e) {
    std::string error = "Error while generating the Eu4 Module.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
  try {
    // generate map files. Format must be converted and colours mapped to eu4
    // compatible colours
    formatConverter.dump8BitTerrain(terrainData, climateData, ardaData.civLayer,
                                    pathcfg.gameModPath + "//map//terrain.bmp",
                                    "terrain", false);
    formatConverter.dump8BitRivers(terrainData, climateData,
                                   pathcfg.gameModPath + "//map//rivers",
                                   "rivers", false);
    formatConverter.dump8BitTrees(terrainData, climateData,
                                  pathcfg.gameModPath + "//map//trees.bmp",
                                  "trees", false);
    formatConverter.dump8BitHeightmap(terrainData.detailedHeightMap,
                                      pathcfg.gameModPath + "//map//heightmap",
                                      "heightmap");
    std::vector<Fwg::Gfx::Bitmap> seasonalColourmaps;
    genSeasons(Cfg::Values(), seasonalColourmaps);
    formatConverter.dumpTerrainColourmap(seasonalColourmaps[0], ardaData.civLayer,
                                         pathcfg.gameModPath,
                                         "//map//terrain//colormap_spring.dds",
                                         DXGI_FORMAT_B8G8R8A8_UNORM, 2, false);
    formatConverter.dumpTerrainColourmap(seasonalColourmaps[1],
                                         ardaData.civLayer,
                                         pathcfg.gameModPath,
                                         "//map//terrain//colormap_summer.dds",
                                         DXGI_FORMAT_B8G8R8A8_UNORM, 2, false);
    formatConverter.dumpTerrainColourmap(seasonalColourmaps[2],
                                         ardaData.civLayer,
                                         pathcfg.gameModPath,
                                         "//map//terrain//colormap_autumn.dds",
                                         DXGI_FORMAT_B8G8R8A8_UNORM, 2, false);
    formatConverter.dumpTerrainColourmap(seasonalColourmaps[3],
                                         ardaData.civLayer,
                                         pathcfg.gameModPath,
                                         "//map//terrain//colormap_winter.dds",
                                         DXGI_FORMAT_B8G8R8A8_UNORM, 2, false);
    formatConverter.dumpDDSFiles(
        terrainData.detailedHeightMap,
        pathcfg.gameModPath + "//map//terrain//colormap_water", false, 2);
    formatConverter.dumpWorldNormal(
        Fwg::Gfx::Bitmap(Cfg::Values().width, Cfg::Values().height, 24,
                         terrainData.sobelData),
        pathcfg.gameModPath + "//map//world_normal.bmp", false);

    using namespace Fwg::Gfx;
    // just copy over provinces.bmp, already in a compatible format
    Bmp::save(provinceMap, pathcfg.gameModPath + "//map//provinces.bmp");
    {
      using namespace Parsing;
      // now do text
      writeAdj(pathcfg.gameModPath + "//map//adjacencies.csv", ardaProvinces);
      writeAmbientObjects(pathcfg.gameModPath + "//map//ambient_object.txt",
                          ardaProvinces);
      writeAreas(pathcfg.gameModPath + "//map//area.txt", ardaRegions,
                 pathcfg.gamePath);
      writeColonialRegions(
          pathcfg.gameModPath +
              "//common//colonial_regions//00_colonial_regions.txt",
          pathcfg.gamePath, ardaProvinces);
      writeClimate(pathcfg.gameModPath + "//map//climate.txt", ardaProvinces);
      writeContinent(pathcfg.gameModPath + "//map//continent.txt",
                     ardaProvinces);
      writeDefaultMap(pathcfg.gameModPath + "//map//default.map",
                      ardaProvinces);
      writeDefinition(pathcfg.gameModPath + "//map//definition.csv",
                      ardaProvinces);
      writePositions(pathcfg.gameModPath + "//map//positions.txt",
                     ardaProvinces);
      writeRegions(pathcfg.gameModPath + "//map//region.txt", pathcfg.gamePath,
                   getEu4Regions());
      writeSuperregion(pathcfg.gameModPath + "//map//superregion.txt",
                       pathcfg.gamePath, ardaRegions);
      writeTerrain(pathcfg.gameModPath + "//map//terrain.txt", ardaProvinces);
      writeTradeCompanies(
          pathcfg.gameModPath +
              "//common//trade_companies//00_trade_companies.txt",
          pathcfg.gamePath, ardaProvinces);
      writeTradewinds(pathcfg.gameModPath + "//map//trade_winds.txt",
                      ardaProvinces);

      copyDescriptorFile(
          Fwg::Cfg::Values().resourcePath + "//eu4//descriptor.mod",
          pathcfg.gameModPath, pathcfg.gameModsDirectory, pathcfg.modName);

      writeProvinces(pathcfg.gameModPath + "//history//provinces//",
                     ardaProvinces, ardaRegions);
      writeLoc(pathcfg.gameModPath + "//localisation//", pathcfg.gamePath,
               ardaRegions, ardaProvinces, getEu4Regions());
      Fwg::Utils::Logging::logLine("Done with the eu4 export");
    }
  } catch (std::exception e) {
    std::string error = "Error while dumping and writing files.\n";
    error += "Error is: \n";
    error += e.what();
    throw(std::exception(error.c_str()));
  }
}

} // namespace Rpx::Eu4