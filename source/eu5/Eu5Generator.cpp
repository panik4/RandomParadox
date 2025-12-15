#include "eu5/Eu5Generator.h"
namespace Rpx::Eu5 {
using namespace Fwg;
using namespace Fwg::Gfx;

Generator::Generator(const std::string &configSubFolder,
                     const boost::property_tree::ptree &rpdConf)
    : Rpx::ModGenerator(configSubFolder, GameType::Eu5, "binaries//eu5.exe",
                        rpdConf) {
  configureModGen(configSubFolder, Fwg::Cfg::Values().username, rpdConf);
  factories.regionFactory = []() {
    return std::make_shared<Arda::ArdaRegion>();
  };
  // this->terrainTypeToString.at(Fwg::Areas::Province::TerrainType::marsh) =
  // "wetlands";
  // this->terrainTypeToString.at(Fwg::Areas::Province::TerrainType::savannah) =
  //     "savanna";
  // this->terrainTypeToString.at(Fwg::Areas::Province::TerrainType::tundra) =
  // "snow";
  // this->terrainTypeToString.at(Fwg::Areas::Province::TerrainType::arctic) =
  // "snow";
}

// prepare folder structure
bool Generator::createPaths() {
  try {
    using namespace std::filesystem;
    std::string prefix = "//in_game//";
    std::vector<std::string> paths = {
        "",
        "//gfx//terrain2//",
        "//gfx//terrain2//decals//",
        "//gfx//terrain2//decals//mountain_01//instances",
        "//gfx//terrain2//decals//mountain_02//instances",
        "//gfx//terrain2//decals//mountain_03//instances",
        "//gfx//terrain2//decals//mountain_04//instances",
        "//gfx//terrain2//decals//mountain_jagged_01//instances",
        "//gfx//terrain2//decals//volcano_01//instances",
        "//gfx//terrain2//terrain_textures/masks",
        "//gfx//map//water",
        "//map_data",
        "//map_data//named_locations//",
        "//content_source//map_objects//masks",

    };
    for (int i = 1; i <= 16; i++) {
      paths.push_back("//gfx//terrain2//decals//heightmap_" +
                      std::to_string(i) + "_16");
    }
    std::vector<std::string> pathsToRemove = {"//common//", "//localization//"
                                              };

    for (const auto &path : pathsToRemove) {
      Fwg::Utils::Logging::logLine("Removing path: " + pathcfg.gameModPath +
                                   prefix + path);
      remove_all(pathcfg.gameModPath + prefix + path);
    }
    Fwg::Utils::Logging::logLine("Creating path: " + pathcfg.gameModPath +
                                 "//.metadata");
    create_directories(pathcfg.gameModPath + "//.metadata");
    for (const auto &path : paths) {
      Fwg::Utils::Logging::logLine("Creating path: " + pathcfg.gameModPath +
                                   prefix + path);
      create_directories(pathcfg.gameModPath + prefix + path);
    }
    // specific debugging path
    if (Cfg::Values().debugLevel > 0)
      create_directory(Cfg::Values().mapsPath + "Eu5");

    return true;
  } catch (std::exception e) {
    std::string error = "Configured paths seem to be messed up, check EuIV "
                        "Module.json\n";
    error += "You can try fixing it yourself. Error is:\n ";
    error += e.what();
    Fwg::Utils::Logging::logLine(error);
    throw(std::exception(error.c_str()));
    return false;
  }
}

void Generator::configureModGen(const std::string &configSubFolder,
                                const std::string &username,
                                const boost::property_tree::ptree &rpdConf) {
  Fwg::Utils::Logging::logLine("Reading Eu 5 Config");
  Rpx::Utils::configurePaths(username, "Europa Universalis V", rpdConf,
                             this->pathcfg);
  auto &config = Cfg::Values();
  namespace pt = boost::property_tree;
  pt::ptree eu5Conf;
  try {
    // Read the basic settings
    std::ifstream f(configSubFolder + "//Victoria3Module.json");
    std::stringstream buffer;
    if (!f.good())
      Fwg::Utils::Logging::logLine("Config could not be loaded");
    buffer << f.rdbuf();
    Fwg::Parsing::replaceInStringStream(buffer, "//", "//");

    pt::read_json(buffer, eu5Conf);
  } catch (std::exception e) {
    Fwg::Utils::Logging::logLine(
        "Incorrect config \"Europa Universalis IVModule.json\"");
    Fwg::Utils::Logging::logLine("You can try fixing it yourself. Error is: ",
                                 e.what());
    Fwg::Utils::Logging::logLine(
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or search for \"json validator\"");
  }
  //  passed to generic ScenarioGenerator
  ardaConfig.numCountries = eu5Conf.get<int>("scenario.numCountries");
  ardaConfig.generationAge = Arda::Utils::GenerationAge::Victorian;
  ardaConfig.targetWorldPopulation = 1'000'000'000;
  ardaConfig.targetWorldGdp = 1'000'000'000'000;

  config.seaLevel = 20;
  config.riverFactor = 0.0;
  config.seaProvFactor *= 0.10;
  config.landProvFactor *= 1.0;
  config.minProvPerSeaRegion = 1;
  config.maxImageArea = 16384 * 8192;
  config.autoSeaRegionParams = false;
  config.forceResolutionBase = false;
  config.autoSplitProvinces = false;
  config.resolutionBase = 1;
  // allow massive images for Vic3
  config.targetMaxImageSize = 160'000'000;
  config.loadMapsPath = eu5Conf.get<std::string>("fastworldgen.loadMapsPath");
  ardaConfig.locationConfig.miningPerRegion = 1;
  ardaConfig.locationConfig.forestryPerRegion = 1;
  ardaConfig.locationConfig.citiesPerRegion = 1;
  ardaConfig.locationConfig.portsPerRegion = 1;
  ardaConfig.locationConfig.agriculturePerRegion = 1;
  // check if config settings are fine
  config.sanityCheck();
}

/* a visualisation of the final terrain types. This is not vic3 specific yet, a
 * TODO*/
Fwg::Gfx::Bitmap Generator::mapTerrain() {
  const auto &climateMap = this->climateMap;
  Bitmap typeMap(climateMap.width(), climateMap.height(), 24);
  for (auto &prov : areaData.provinces) {
    prov->calculateTerrainType(terrainData);
    prov->calculateClimateType(climateData);
  }

  return typeMap;
}

void Generator::totalArableLand(const std::vector<float> &arableLand) {
  // const auto baseWorldArableSlots = 50000.0;
  // auto totalArable = 0.0f;
  // for (auto &val : arableLand) {
  //   totalArable += val;
  // }
  // for (auto &reg : modData.vic3Regions) {
  //   auto resShare = 0.0;
  //   for (const auto &prov : reg->provinces) {
  //     for (const auto pix : prov->getNonOwningPixelView()) {
  //       resShare += arableLand[pix];
  //     }
  //   }
  //   // basically fictive value from given input of how often this resource
  //   // appears
  //   auto stateArable = baseWorldArableSlots * (resShare / totalArable);
  //   reg->arableLand = stateArable;
  // }
}

void Generator::distributeResources() {

  //// distribute arable land to states
  // totalArableLand(climateData.arableLand);

  //// config for all resource types
  // for (auto &resConfig : vic3Config.resConfigs) {
  //   std::vector<float> resPrev;
  //   if (resConfig.random) {
  //     resPrev = Fwg::Resources::randomResourceLayer(
  //         resConfig.name, resConfig.noiseConfig.fractalFrequency,
  //         resConfig.noiseConfig.tanFactor, resConfig.noiseConfig.cutOff,
  //         resConfig.noiseConfig.mountainBonus);
  //   } else if (resConfig.considerSea) {
  //     resPrev = Fwg::Resources::coastDependentLayer(
  //         resConfig.name, resConfig.oceanFactor, resConfig.lakeFactor,
  //         areaData.provinces);
  //   } else {
  //     resPrev = Fwg::Resources::climateDependentLayer(
  //         resConfig.name, resConfig.noiseConfig.fractalFrequency,
  //         resConfig.noiseConfig.tanFactor, resConfig.noiseConfig.cutOff,
  //         resConfig.noiseConfig.mountainBonus, resConfig.considerClimate,
  //         resConfig.climateEffects, resConfig.considerTrees,
  //         resConfig.treeEffects, climateData);
  //   }
  //   if (resPrev.size())
  //     totalResourceVal(resPrev, resConfig.resourcePrevalence, resConfig);
  // }
}

void Generator::mapRegions() { ArdaGen::mapRegions(); }
void Generator::mapCountries() {}
// set tech levels, give vic3GameData.techs, count pops, cultures and religions,
// set diplomatic relations (e.g. puppets, markets, protectorates)
void Generator::generateCountrySpecifics() {
  // auto &cfg = Fwg::Cfg::Values();
  //// count pops
  // for (auto &cEntry : modData.vic3Countries) {
  //   auto &c = cEntry.second;
  //   auto totalPop = 0;
  //   auto averageDevelopment = 0.0;
  //   for (auto &state : c->ownedVic3Regions) {
  //     // to count total pop
  //     totalPop += state->totalPopulation;
  //   }
  //   c->pop = totalPop;
  //   for (auto &state : c->ownedVic3Regions) {
  //     // development should be weighed by the pop in the state
  //     averageDevelopment += state->averageDevelopment *
  //                           ((double)state->totalPopulation /
  //                           (double)totalPop);
  //   }
  //   c->averageDevelopment = averageDevelopment;
  //   c->evaluateTechLevel(vic3GameData.techLevels);
  //   if (cfg.debugLevel > 5) {
  //     Fwg::Utils::Logging::logLine(c->tag, " has a population of ", c->pop);
  //   }
  // }
}
bool Generator::importData(const std::string &path) { return true; }
void Generator::diplomaticRelations() {}
void Generator::createMarkets() {}

void Generator::calculateNeeds() {}
void Generator::distributeBuildings() {}

void Generator::createLocators() {}

void Generator::calculateNavalExits() {
  // for (auto &region : modData.vic3Regions) {
  //   if (!region->isSea()) {
  //     //  check if we have a port locator, and take its naval exit ID
  //     for (auto &location : region->significantLocations) {
  //       if (location->type == Fwg::Civilization::LocationType::Port) {
  //         region->navalExit = location->portExitProvinceID;
  //         break;
  //       }
  //     }
  //   }
  // }
}

void Generator::initImageExporter() {
  // imageExporter = Gfx::Eu5::ImageExporter(pathcfg.gamePath, "Vic3");
}

void Generator::generate() {
  if (!createPaths())
    return;

  try {
    // start with the generic stuff in the Scenario Generator
    mapProvinces();
    mapRegions();

    mapContinents();
    mapTerrain();
    // genCivilisationData();
    // auto countryFactory = []() -> std::shared_ptr<Arda::Country> {
    //   return std::make_shared<Arda::Country>();
    // };
    //// generate country data
    // generateCountries(countryFactory);

    //// non-country stuff
    // auto factory = []() -> std::shared_ptr<StrategicRegion> {
    //   return std::make_shared<StrategicRegion>();
    // };
    // generateStrategicRegions(factory);
    //// Vic3 specifics:
    // distributeResources();
    // if (!importData(this->pathcfg.gamePath + "//game//")) {
    //   Fwg::Utils::Logging::logLine("ERROR: Could not import data from game "
    //                                "folder. The generation has FAILED");
    //   return;
    // }

    //// handle basic development, tech level, policies,
    // generateCountrySpecifics();
    // diplomaticRelations();
    // createMarkets();
    // calculateNeeds();
    // distributeBuildings();

  } catch (std::exception e) {
    std::string error = "Error while generating the Vic3 Module.\n";
    error += "Error is: \n";
    error += e.what();
    Fwg::Utils::Logging::logLine(error);
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
    Fwg::Utils::Logging::logLine(error);
    throw(std::exception(error.c_str()));
  }
  printStatistics();
}

void Generator::writeTextFiles() {
  using namespace Parsing::Writing;

  writeMetadata(pathcfg.gameModPath + "//.metadata//metadata.json");
  overwrites(pathcfg.gameModPath + "//in_game//");
  mainMenuOverrides(pathcfg.gameModPath + "//main_menu");
}

void Generator::writeImages() {
  //// TODO: improve handling of altitude data to not rely on image
  imageExporter.Eu5ColourMaps(terrainData, climateData, ardaData.civLayer,
                              pathcfg.gameModPath + "//in_game//gfx//map//",
                              this->exportWidth / 2, this->exportHeight / 2);
   imageExporter.writeLocations(provinceMap, ardaProvinces, ardaRegions,
                                ardaContinents, ardaData.civLayer,
                                pathcfg.gameModPath + "//in_game//map_data//",
                                this->exportWidth, this->exportHeight);
  // imageExporter.dumpHeightmap(
  //     terrainData.detailedHeightMap,
  //     pathcfg.gameModPath + "//in_game//gfx//terrain2//", "",
  //     this->exportWidth * 4, this->exportHeight * 4);
  imageExporter.dumpDecalMasks(
      terrainData, climateData,
      pathcfg.gameModPath + "//in_game//gfx//terrain2//decals//", "",
      this->exportWidth * 2, this->exportHeight * 2);
  //imageExporter.dumpTerrainMasks(
  //    terrainData, climateData,
  //    pathcfg.gameModPath +
  //        "//in_game//gfx//terrain2//terrain_textures//masks//",
  //    "", this->exportWidth, this->exportHeight);

  //imageExporter.mapObjectMasks(
  //    terrainData, climateData, ardaData.civLayer,
  //    pathcfg.gameModPath + "//in_game//content_source//map_objects//masks//",
  //    this->exportWidth, this->exportHeight);
}

void Generator::writeSplnet() {
  // createLocators();
  // Parsing::Writing::locators(pathcfg.gameModPath +
  //                                "//gfx//map//map_object_data//",
  //                            modData.vic3Regions);
  // genNavmesh();
  // calculateNavalExits();

  // Splnet splnet;
  // splnet.constructSplnet(ardaRegions);
  // splnet.writeFile(pathcfg.gameModPath +
  //                  "//gfx//map//spline_network//spline_network.splnet");
}

} // namespace Rpx::Eu5