#include "eu5/Eu5Generator.h"
namespace Rpx::Eu5 {
using namespace Fwg;
using namespace Fwg::Gfx;

Generator::Generator(const std::string &configSubFolder,
                     const boost::property_tree::ptree &rpdConf)
    : Rpx::ModGenerator(configSubFolder, GameType::Eu5, "binaries//eu5.exe",
                        rpdConf) {
  configureModGen(configSubFolder, Fwg::Cfg::Values().username, rpdConf);
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
                                      "//common//country_formation",
                                      "//common//history",
                                      "//common//history//buildings",
                                      "//common//history//countries",
                                      "//common//history//pops",
                                      "//common//history//states",
                                      "//common//country_creation",
                                      "//common//journal_entries",
                                      "//common//scripted_triggers",
                                      "//common//static_modifiers",
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
      Fwg::Utils::Logging::logLine("Removing path: " + prefix +
                                   pathcfg.gameModPath + path);
      remove_all(pathcfg.gameModPath + prefix + path);
    }
    for (const auto &path : paths) {
      Fwg::Utils::Logging::logLine("Creating path: " + pathcfg.gameModPath +
                                   prefix + path);
      create_directory(pathcfg.gameModPath + prefix + path);
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

  config.seaLevel = 18;
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
  auto &colours = Fwg::Cfg::Values().colours;
  auto &climateColours = Fwg::Cfg::Values().climateColours;
  auto &elevationColours = Fwg::Cfg::Values().elevationColours;
  typeMap.fill(colours.at("sea"));
  Fwg::Utils::Logging::logLine("Mapping Terrain");
  const auto &landForms = terrainData.landForms;
  const auto &climates = climateData.climates;
  const auto &forests = climateData.dominantForest;
  for (auto &ardaRegion : ardaRegions) {
    for (auto &gameProv : ardaRegion->ardaProvinces) {
      gameProv->terrainType = "plains";
      const auto &baseProv = gameProv;
      if (baseProv->isLake()) {
        gameProv->terrainType = "lake";
        for (auto &pix : baseProv->pixels) {
          typeMap.setColourAtIndex(pix, colours.at("lake"));
        }
      } else if (baseProv->isSea()) {
        gameProv->terrainType = "sea";
        for (auto &pix : baseProv->pixels) {
          typeMap.setColourAtIndex(pix, climateColours.at("ocean"));
        }
      } else {
        int forestPixels = 0;
        std::map<Fwg::Climate::Detail::ClimateTypeIndex, int> climateScores;
        std::map<Fwg::Terrain::ElevationTypeIndex, int> terrainTypeScores;
        // get the dominant climate of the province
        for (auto &pix : baseProv->pixels) {
          climateScores[climates[pix].getChances(0).second]++;
          terrainTypeScores[landForms[pix].landForm]++;
          if (forests[pix]) {
            forestPixels++;
          }
        }
        auto dominantClimate =
            std::max_element(climateScores.begin(), climateScores.end(),
                             [](const auto &l, const auto &r) {
                               return l.second < r.second;
                             })
                ->first;
        auto dominantTerrain =
            std::max_element(terrainTypeScores.begin(), terrainTypeScores.end(),
                             [](const auto &l, const auto &r) {
                               return l.second < r.second;
                             })
                ->first;
        // now first check the terrains, if e.g. mountains or peaks are too
        // dominant, this is a mountainous province
        if (dominantTerrain == Fwg::Terrain::ElevationTypeIndex::MOUNTAINS ||
            dominantTerrain == Fwg::Terrain::ElevationTypeIndex::PEAKS ||
            dominantTerrain == Fwg::Terrain::ElevationTypeIndex::STEEPPEAKS) {
          gameProv->terrainType = "mountain";
          for (auto &pix : baseProv->pixels) {
            typeMap.setColourAtIndex(pix, elevationColours.at("mountains"));
          }
        } else if (dominantTerrain == Fwg::Terrain::ElevationTypeIndex::HILLS) {
          gameProv->terrainType = "hills";
          for (auto &pix : baseProv->pixels) {
            typeMap.setColourAtIndex(pix, elevationColours.at("hills"));
          }
        } else if ((double)forestPixels / baseProv->pixels.size() > 0.5) {
          gameProv->terrainType = "forest";
          for (auto &pix : baseProv->pixels) {
            typeMap.setColourAtIndex(pix, Fwg::Gfx::Colour(16, 40, 8));
          }
        } else {
          using CTI = Fwg::Climate::Detail::ClimateTypeIndex;
          // now, if this is a more flat land, check the climate type
          if (dominantClimate == CTI::TROPICSMONSOON ||
              dominantClimate == CTI::TROPICSRAINFOREST) {
            gameProv->terrainType = "jungle";
            for (auto &pix : baseProv->pixels) {
              typeMap.setColourAtIndex(pix,
                                       climateColours.at("tropicsrainforest"));
            }
          } else if (dominantClimate == CTI::COLDDESERT ||
                     dominantClimate == CTI::DESERT) {
            gameProv->terrainType = "desert";
            for (auto &pix : baseProv->pixels) {
              typeMap.setColourAtIndex(pix, climateColours.at("desert"));
            }
          } else {
            gameProv->terrainType = "plains";
            for (auto &pix : baseProv->pixels) {
              typeMap.setColourAtIndex(pix, elevationColours.at("plains"));
            }
          }
        }
      }
    }
  }
  Png::save(typeMap, Fwg::Cfg::Values().mapsPath + "/typeMap.png");
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

void Generator::mapRegions() {
  // Fwg::Utils::Logging::logLine("Mapping Regions");
  // ardaRegions.clear();
  // modData.vic3Regions.clear();

  // for (auto &region : this->areaData.regions) {
  //   std::sort(region->provinces.begin(), region->provinces.end(),
  //             [](const std::shared_ptr<Fwg::Areas::Province> a,
  //                const std::shared_ptr<Fwg::Areas::Province> b) {
  //               return (*a < *b);
  //             });
  //   auto ardaRegion = std::dynamic_pointer_cast<Rpx::Vic3::Region>(region);

  //  // generate random name for region
  //  ardaRegion->name = "";
  //  ardaRegion->ardaProvinces.clear();

  //  for (auto &province : ardaRegion->provinces) {
  //    ardaRegion->ardaProvinces.push_back(ardaProvinces[province->ID]);
  //  }
  //  // save game region to generic module container and to hoi4 specific
  //  // container
  //  ardaRegions.push_back(ardaRegion);
  //  modData.vic3Regions.push_back(ardaRegion);
  //}
  //// sort by Arda::ArdaProvince ID
  //// std::sort(ardaRegions.begin(), ardaRegions.end(),
  ////          [](auto l, auto r) { return *l < *r; });
  //// check if we have the same amount of ardaProvinces as FastWorldGen
  //// provinces
  // if (ardaProvinces.size() != this->areaData.provinces.size())
  //   throw(std::exception("Fatal: Lost provinces, terminating"));
  // if (ardaRegions.size() != this->areaData.regions.size())
  //   throw(std::exception("Fatal: Lost regions, terminating"));
  // for (const auto &ardaRegion : ardaRegions) {
  //   if (ardaRegion->ID > ardaRegions.size()) {
  //     throw(std::exception("Fatal: Invalid region IDs, terminating"));
  //   }
  // }
  // applyRegionInput();
}
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

    mapTerrain();
    mapContinents();
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
  // auto foundRegions = compatRegions(
  //     pathcfg.gamePath + "//game//map_data//state_regions//",
  //     pathcfg.gameModPath + "//map_data//state_regions//",
  //     modData.vic3Regions);
  // compatStratRegions(pathcfg.gamePath +
  // "//game//common//strategic_regions//",
  //                    pathcfg.gameModPath + "//common//strategic_regions//",
  //                    modData.vic3Regions, foundRegions);
  //// compatReleasable(pathcfg.gamePath + "//game//common//country_creation//",
  ////                  pathcfg.gameModPath + "//common//country_creation//");
  // adj(pathcfg.gameModPath + "//map_data//adjacencies.csv");
  // defaultMap(pathcfg.gameModPath + "//map_data//default.map", ardaProvinces);
  // defines(pathcfg.gameModPath + "//common//defines//01_defines.txt");
  // provinceTerrains(pathcfg.gameModPath + "//map_data//province_terrains.txt",
  //                  ardaProvinces);
  // stateFiles(pathcfg.gameModPath +
  // "//map_data//state_regions//00_regions.txt",
  //            modData.vic3Regions);
  // Parsing::History::writeBuildings(
  //     pathcfg.gameModPath + "//common//history//buildings//00_buildings.txt",
  //     modData.vic3Regions);
  writeMetadata(pathcfg.gameModPath + "//.metadata//metadata.json");
  // strategicRegions(
  //     pathcfg.gameModPath +
  //         "//common//strategic_regions//randVic_strategic_regions.txt",
  //     superRegions, modData.vic3Regions);
  // cultureCommon(pathcfg.gameModPath + "//common//cultures//00_cultures.txt",
  //               civData.cultures);
  // religionCommon(pathcfg.gameModPath + "//common//religions//religions.txt",
  //                civData.religions);
  // staticModifiers(pathcfg.gameModPath + "//common//static_modifiers//",
  //                 civData.cultures, civData.religions);
  // countryCommon(pathcfg.gameModPath +
  //                   "//common//country_definitions//00_countries.txt",
  //               modData.vic3Countries, modData.vic3Regions);

  // compatFile(pathcfg.gameModPath +
  //            "//common//country_creation//00_releasable_countries.txt");
  // compatFile(pathcfg.gameModPath +
  //            "//common//cultures//00_additional_cultures.txt");
  // compatFile(pathcfg.gameModPath +
  //            "//common//country_definitions//01_africa.txt");
  // compatFile(pathcfg.gameModPath +
  //            "//common//country_definitions//01_pacific_and_australasia.txt");
  // compatFile(pathcfg.gameModPath +
  //            "//common//country_formation//00_formable_countries.txt");
  // compatFile(pathcfg.gameModPath +
  //            "//common//country_formation//00_major_formables.txt");
  // stateHistory(pathcfg.gameModPath +
  // "//common//history//states//00_states.txt",
  //              modData.vic3Regions);
  // popsHistory(pathcfg.gameModPath + "//common//history//pops//00_world.txt",
  //             modData.vic3Regions);
  // countryHistory(pathcfg.gameModPath + "//common//history//countries",
  //                modData.vic3Countries);
  // compatFile(pathcfg.gameModPath +
  // "//common//decisions//canal_decisions.txt"); compatFile(pathcfg.gameModPath
  // + "//events//canal_events.txt"); compatFile(pathcfg.gameModPath +
  //            "//events//agitators_events//paris_commune_events.txt");
  // compatFile(pathcfg.gameModPath +
  //            "//events//agitators_events//paris_commune_events.txt");
  // compatFile(pathcfg.gameModPath +
  // "//common//journal_entries//00_canals.txt"); compatFile(pathcfg.gameModPath
  // +
  //            "//common//journal_entries//02_paris_commune.txt");
  // compatTriggers(pathcfg.gamePath + "//game//common//scripted_triggers//",
  //                pathcfg.gameModPath + "//common//scripted_triggers//");
}

void Generator::writeImages() {
  //// TODO: improve handling of altitude data to not rely on image
  auto heightMap = Fwg::Gfx::displayHeightMap(terrainData.detailedHeightMap);
  imageExporter.dumpHeightmap(
      heightMap, pathcfg.gameModPath + "//in_game//map_data//heightmap.png", "",
      this->exportWidth, this->exportHeight);

  // imageExporter.Vic3ColourMaps(worldMap, heightMap, climateData,
  //                              ardaData.civLayer,
  //                              pathcfg.gameModPath + "//gfx//map//");
  //// imageExporter.dump8BitRivers(riverMap,
  ////                                pathcfg.gameModPath +
  ////                                "//map_data//rivers", "rivers", false);

  // imageExporter.detailMaps(terrainData, climateData, ardaData.civLayer,
  //                          pathcfg.gameModPath + "//gfx//map//");
  // imageExporter.dynamicMasks(pathcfg.gameModPath + "//gfx//map//masks//",
  //                            climateData, ardaData.civLayer);
  // imageExporter.contentSource(pathcfg.gameModPath +
  //                                 "//content_source//map_objects//masks//",
  //                             climateData, ardaData.civLayer);
  //// save this and reset the heightmap later. The map will be scaled and the
  //// scaled one then used for the packed heightmap generation. It is important
  //// we reset this after
  // auto temporaryHeightmap = heightMap;
  //// also dump uncompressed packed heightmap
  // imageExporter.dump8BitHeightmap(terrainData.detailedHeightMap,
  //                                 pathcfg.gameModPath +
  //                                 "//map_data//heightmap", "heightmap");
  // auto packedHeightmap = imageExporter.dumpPackedHeightmap(
  //     heightMap, pathcfg.gameModPath + "//map_data//packed_heightmap",
  //     "heightmap");
  // imageExporter.dumpIndirectionMap(
  //     heightMap, pathcfg.gameModPath +
  //     "//map_data//indirection_heightmap.png");
  // Parsing::Writing::heightmap(pathcfg.gameModPath +
  //                                 "//map_data//heightmap.heightmap",
  //                             heightMap, packedHeightmap);
  // heightMap = temporaryHeightmap;
  // temporaryHeightmap.clear();
  // visualiseCountries(countryMap);
  // Fwg::Gfx::Png::save(countryMap, Cfg::Values().mapsPath + "countries.png");
  // using namespace Fwg::Gfx;
  //// just copy over provinces.bmp as a .png, already in a compatible format
  //// auto scaledMap = Bmp::scale(provinceMap, 8192, 3616, false);
  // Png::save(provinceMap, pathcfg.gameModPath + "//map_data//provinces.png");
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