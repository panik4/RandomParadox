#include "vic3/Vic3Generator.h"
namespace Rpx::Vic3 {
using namespace Fwg;
using namespace Fwg::Gfx;

Generator::Generator(const std::string &configSubFolder,
                     const boost::property_tree::ptree &rpdConf)
    : Rpx::ModGenerator(configSubFolder, GameType::Vic3,
                        "binaries//victoria3.exe", rpdConf) {
  configureModGen(configSubFolder, Fwg::Cfg::Values().username, rpdConf);
  // this->terrainTypeToString.at(Fwg::Areas::Province::TerrainType::marsh) =
  // "wetlands";
  // this->terrainTypeToString.at(Fwg::Areas::Province::TerrainType::savannah) =
  //     "savanna";
  // this->terrainTypeToString.at(Fwg::Areas::Province::TerrainType::tundra) =
  // "snow";
  // this->terrainTypeToString.at(Fwg::Areas::Province::TerrainType::arctic) =
  // "snow";
  factories.regionFactory = []() {
    return std::make_shared<Rpx::Vic3::Region>();
  };

  ardaFactories.countryFactory = []() -> std::shared_ptr<Rpx::Vic3::Country> {
    return std::make_shared<Rpx::Vic3::Country>();
  };
}

// prepare folder structure
bool Generator::createPaths() {
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
    Fwg::Utils::Logging::logLine(error);
    throw(std::exception(error.c_str()));
    return false;
  }
}

void Generator::configureModGen(const std::string &configSubFolder,
                                const std::string &username,
                                const boost::property_tree::ptree &rpdConf) {
  Fwg::Utils::Logging::logLine("Reading Vic 3 Config");
  Rpx::Utils::configurePaths(username, "Victoria 3", rpdConf, this->pathcfg);
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
  }
  //  passed to generic ScenarioGenerator
  ardaConfig.numCountries = vic3Conf.get<int>("scenario.numCountries");
  ardaConfig.generationAge = Arda::Utils::GenerationAge::Victorian;
  ardaConfig.targetWorldPopulation = 1'000'000'000;
  ardaConfig.targetWorldGdp = 1'000'000'000'000;

  config.seaLevel = 18;
  config.riverFactor = 0.0;
  config.seaProvFactor *= 0.10;
  config.landProvFactor *= 1.0;
  config.minProvPerSeaRegion = 1;
  config.autoSeaRegionParams = false;
  config.forceResolutionBase = false;
  config.autoSplitProvinces = false;
  config.maxImageArea = 8192 * 3616;
  config.resolutionBase = 1;
  // allow massive images for Vic3
  config.targetMaxImageSize = 160'000'000;
  config.loadMapsPath = vic3Conf.get<std::string>("fastworldgen.loadMapsPath");
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
Fwg::Gfx::Image Generator::mapTerrain() {
  const auto &climateMap = this->climateMap;
  Image typeMap(climateMap.width(), climateMap.height(), 24);
  auto &colours = Fwg::Cfg::Values().colours;
  auto &climateColours = Fwg::Cfg::Values().climateColours;
  auto &elevationColours = Fwg::Cfg::Values().elevationColours;
  typeMap.fill(colours.at("sea"));
  Fwg::Utils::Logging::logLine("Mapping Terrain");
  const auto &landFormIds = terrainData.landFormIds;
  const auto &climates = climateData.climateChances;
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
        std::map<Fwg::Climate::Detail::ClimateClassId, int> climateScores;
        std::map<Fwg::Terrain::LandformId, int> terrainTypeScores;
        // get the dominant climate of the province
        for (auto &pix : baseProv->pixels) {
          climateScores[climates.getChance(0, pix).typeIndex]++;
          terrainTypeScores[landFormIds[pix]]++;
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
        if (dominantTerrain == Fwg::Terrain::LandformId::MOUNTAINS ||
            dominantTerrain == Fwg::Terrain::LandformId::PEAKS ||
            dominantTerrain == Fwg::Terrain::LandformId::STEEPPEAKS) {
          gameProv->terrainType = "mountain";
          for (auto &pix : baseProv->pixels) {
            typeMap.setColourAtIndex(pix, elevationColours.at("mountains"));
          }
        } else if (dominantTerrain == Fwg::Terrain::LandformId::HILLS) {
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
          using CTI = Fwg::Climate::Detail::ClimateClassId;
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
  const auto baseWorldArableSlots = 50000.0;
  auto totalArable = 0.0f;
  for (auto &val : arableLand) {
    totalArable += val;
  }
  for (auto &reg : modData.vic3Regions) {
    auto resShare = 0.0;
    for (const auto &prov : reg->provinces) {
      for (const auto pix : prov->getNonOwningPixelView()) {
        resShare += arableLand[pix];
      }
    }
    // basically fictive value from given input of how often this resource
    // appears
    auto stateArable = baseWorldArableSlots * (resShare / totalArable);
    reg->arableLand = stateArable;
  }
}

void Generator::distributeResources() {

  // distribute arable land to states
  totalArableLand(climateData.arableLand);

  // config for all resource types
  for (auto &resConfig : vic3Config.resConfigs) {
    std::vector<float> resPrev;
    if (resConfig.random) {
      resPrev = Fwg::Resources::randomResourceLayer(
          resConfig.name, resConfig.noiseConfig.fractalFrequency,
          resConfig.noiseConfig.tanFactor, resConfig.noiseConfig.cutOff,
          resConfig.noiseConfig.mountainBonus);
    } else if (resConfig.considerSea) {
      resPrev = Fwg::Resources::coastDependentLayer(
          resConfig.name, resConfig.oceanFactor, resConfig.lakeFactor,
          areaData.provinces);
    } else {
      resPrev = Fwg::Resources::climateDependentLayer(
          resConfig.name, resConfig.noiseConfig.fractalFrequency,
          resConfig.noiseConfig.tanFactor, resConfig.noiseConfig.cutOff,
          resConfig.noiseConfig.mountainBonus, resConfig.considerClimate,
          resConfig.climateEffects, resConfig.considerTrees,
          resConfig.treeEffects, climateData);
    }
    if (resPrev.size())
      totalResourceVal(resPrev, resConfig.resourcePrevalence, resConfig);
  }
}

void Generator::mapRegions() {
  Fwg::Utils::Logging::logLine("Mapping Regions");
  ardaRegions.clear();
  modData.vic3Regions.clear();

  for (auto &region : this->areaData.regions) {
    std::sort(region->provinces.begin(), region->provinces.end(),
              [](const std::shared_ptr<Fwg::Areas::Province> a,
                 const std::shared_ptr<Fwg::Areas::Province> b) {
                return (*a < *b);
              });
    auto ardaRegion = std::dynamic_pointer_cast<Rpx::Vic3::Region>(region);

    // generate random name for region
    ardaRegion->name = "";
    ardaRegion->ardaProvinces.clear();

    for (auto &province : ardaRegion->provinces) {
      ardaRegion->ardaProvinces.push_back(ardaProvinces[province->ID]);
    }
    // save game region to generic module container and to hoi4 specific
    // container
    ardaRegions.push_back(ardaRegion);
    modData.vic3Regions.push_back(ardaRegion);
  }
  // sort by Arda::ArdaProvince ID
  // std::sort(ardaRegions.begin(), ardaRegions.end(),
  //          [](auto l, auto r) { return *l < *r; });
  // check if we have the same amount of ardaProvinces as FastWorldGen
  // provinces
  if (ardaProvinces.size() != this->areaData.provinces.size())
    throw(std::exception("Fatal: Lost provinces, terminating"));
  if (ardaRegions.size() != this->areaData.regions.size())
    throw(std::exception("Fatal: Lost regions, terminating"));
  for (const auto &ardaRegion : ardaRegions) {
    if (ardaRegion->ID > ardaRegions.size()) {
      throw(std::exception("Fatal: Invalid region IDs, terminating"));
    }
  }
  applyRegionInput();
}
void Generator::mapCountries() {
  for (auto &country : countries) {
    std::shared_ptr<Country> vic3Country =
        std::dynamic_pointer_cast<Vic3::Country, Arda::Country>(country.second);
    if (vic3Country != nullptr) {
      for (auto &region : vic3Country->ownedRegions) {
        // vic3Country->ownedVic3Regions.push_back(
        //     std::reinterpret_pointer_cast<Vic3::Region, Arda::ArdaRegion>(
        //         region));

        vic3Country->ownedVic3Regions.push_back(
            std::dynamic_pointer_cast<Region>(region));
      }
    } else {

      Fwg::Utils::Logging::logLine("Warning: Country ", country.first,
                                   " not found in Vic3 countries");
    }
    modData.vic3Countries.emplace(country.first, vic3Country);
  }
}
// set tech levels, give vic3GameData.techs, count pops, cultures and religions,
// set diplomatic relations (e.g. puppets, markets, protectorates)
void Generator::generateCountrySpecifics() {
  auto &cfg = Fwg::Cfg::Values();
  // count pops
  for (auto &cEntry : modData.vic3Countries) {
    auto &c = cEntry.second;
    auto totalPop = 0;
    auto averageDevelopment = 0.0;
    for (auto &state : c->ownedVic3Regions) {
      // to count total pop
      totalPop += state->totalPopulation;
    }
    c->pop = totalPop;
    for (auto &state : c->ownedVic3Regions) {
      // development should be weighed by the pop in the state
      averageDevelopment += state->averageDevelopment *
                            ((double)state->totalPopulation / (double)totalPop);
    }
    c->averageDevelopment = averageDevelopment;
    c->evaluateTechLevel(vic3GameData.techLevels);
    if (cfg.debugLevel > 5) {
      Fwg::Utils::Logging::logLine(c->tag, " has a population of ", c->pop);
    }
  }
}
bool Generator::importData(const std::string &path) {
  try {
    vic3GameData.techs =
        Vic3::Importing::readTechs(path + "common//technology//technologies//");
    vic3GameData.techLevels = Vic3::Importing::readTechLevels(
        path + "common//scripted_effects//00_starting_inventions.txt",
        vic3GameData.techs);
    vic3GameData.goods = Vic3::Importing::readGoods(
        path + "common//vic3GameData.goods//00_goods.txt");
    vic3GameData.productionmethods = Vic3::Importing::readProdMethods(
        path + "common//production_methods//", vic3GameData.goods,
        vic3GameData.techs);
    vic3GameData.productionmethodGroups = Vic3::Importing::readProdMethodGroups(
        path + "common//production_method_groups//",
        vic3GameData.productionmethods);
    vic3GameData.buildingsTypes = Vic3::Importing::readBuildings(
        path + "common//buildings//", vic3GameData.productionmethodGroups,
        vic3GameData.techs);
    vic3GameData.popNeeds = Vic3::Importing::readPopNeeds(
        path + "common//pop_needs//00_pop_needs.txt", vic3GameData.goods);
    vic3GameData.buypackages = Vic3::Importing::readBuypackages(
        path + "//common//buy_packages//00_buy_packages.txt",
        vic3GameData.popNeeds);
    nData.disallowedTokens =
        Vic3::Importing::readTags(path + "common//history//countries//");
  } catch (std::exception e) {
    Fwg::Utils::Logging::logLine("Error: ", e.what());
    return false;
  }
  Fwg::Utils::Logging::logLine("Reading vic3GameData.goods");
  // now map vic3GameData.goods to building types
  for (auto &good : vic3GameData.goods) {
    std::vector<Productionmethod> prodMethods;
    for (auto &productionmethod : vic3GameData.productionmethods) {
      // ignore subsistence buildings
      if (productionmethod.first.find("subsistence") != std::string::npos) {
        continue;
      }
      for (auto &outputGood : productionmethod.second.outputs) {
        // this building has a production method that outputs this good
        if (good.first == outputGood.first.name && outputGood.second > 0) {
          if (vic3GameData.goodToProdMethodsOutput.count(good.first)) {
            vic3GameData.goodToProdMethodsOutput[good.first].push_back(
                productionmethod.second);
          } else {
            Fwg::Utils::Logging::logLine("Cannot find output good: ",
                                         good.first);
          }
        }
      }
      for (auto &inputGood : productionmethod.second.inputs) {
        // this building has a production method that inputs this good
        if (good.first == inputGood.first.name && inputGood.second > 0) {
          if (vic3GameData.goodToProdMethodsInput.count(good.first)) {
            vic3GameData.goodToProdMethodsInput[good.first].push_back(
                productionmethod.second);
          } else {
            Fwg::Utils::Logging::logLine("Cannot find input good: ",
                                         good.first);
          }
        }
      }
    }
  }
  Fwg::Utils::Logging::logLine("Reading building types");
  for (const auto &buildingType : vic3GameData.buildingsTypes) {
    for (const auto &buildingProdGroups : buildingType.productionMethodGroups) {
      for (const auto &buildingProductionmethod :
           buildingProdGroups.productionMethods) {
        for (const auto &prodMethod : vic3GameData.productionmethods) {
          if (prodMethod.first == buildingProductionmethod.first) {
            vic3GameData.productionMethodToBuildingTypes[prodMethod.first]
                .push_back(buildingType);
          }
        }
      }
    }
  }
  return true;
}
void Generator::diplomaticRelations() {}
void Generator::createMarkets() {}

void Generator::calculateNeeds() {

  for (auto &country : modData.vic3Countries) {
    std::map<std::string, double> summedPopNeeds;
    auto wealth = country.second->averageDevelopment;
    // guesstimate size of wealth groups from development
    auto wealthDispersion = shiftedGaussian(wealth);
    double pop = country.second->pop;
    for (int i = 0; i < wealthDispersion.size(); i++) {
      // get the buypackage, it tells us which popneeds we have for this
      // wealth
      auto buyPackage =
          vic3GameData.buypackages.at("wealth_" + std::to_string(i + 1));
      // this is the pop of that wealth, needing the above buypackage contents
      auto affectedPopFactor = wealthDispersion[i] * pop / 10000.0;
      for (auto &popNeed : buyPackage.popNeeds) {
        // if we don't have it yet, add it
        if (summedPopNeeds.find(popNeed.first.name) == summedPopNeeds.end()) {
          summedPopNeeds.emplace(popNeed.first.name,
                                 (double)popNeed.second * affectedPopFactor);
        } else {
          summedPopNeeds.at(popNeed.first.name) +=
              (double)popNeed.second * affectedPopFactor;
        }
      }
    }

    Fwg::Utils::Logging::logLineLevel(
        5, country.first,
        " requires vic3GameData.goods with development: ", wealth);
    for (auto &rq : summedPopNeeds) {
      Fwg::Utils::Logging::logLineLevel(5, rq.first, " needed ", rq.second);
    }
    country.second->summedPopNeeds = summedPopNeeds;
  }
}
void Generator::distributeBuildings() {
  for (auto &countryEntry : modData.vic3Countries) {
    auto &country = countryEntry.second;
    for (auto &need : country->summedPopNeeds) {
      auto &popNeed = vic3GameData.popNeeds.at(need.first);
      auto amount = need.second;
      // this is the vic3GameData.goods used in the popneed to fuilfill demand
      // we want to produce optimally all of them a bit, if possible
      // if not, try to produce some of them with a larger share
      auto goods = popNeed.goods;
      std::vector<Good> produceableGoods;

      // now find the building that produces this, and plop it into a state
      for (auto &produceableGood : goods) {
        double actualDemand = amount / (double)produceableGoods.size() /
                              (double)produceableGood.cost;
        Fwg::Utils::Logging::logLine(produceableGood.name, " has demand of ",
                                     actualDemand);
        // filter out tiny demands for now: TODO
        if (actualDemand > 5.0 &&
            vic3GameData.goodToProdMethodsOutput.count(produceableGood.name)) {
          // find production methods that produce this good
          const auto &potentialProdMethods =
              vic3GameData.goodToProdMethodsOutput.at(produceableGood.name);

          // now find all buildings for all the production methods, so we get
          // a complete list of prod methods we support
          std::map<std::string, BuildingType> buildingTypes;
          for (auto &entry : potentialProdMethods) {
            // check against vic3GameData.techs, this is the decisive factor in
            // evaluating if a building can actually be built already to
            // fulfill the demand
            if (!country->canUseProductionMethod(entry)) {
              continue;
            }

            try {
              if (vic3GameData.productionMethodToBuildingTypes.find(
                      entry.name) !=
                  vic3GameData.productionMethodToBuildingTypes.end()) {
                auto &entry2 =
                    vic3GameData.productionMethodToBuildingTypes.at(entry.name);
                for (auto &entry3 : entry2) {
                  if (entry3.name.find("subsistence") == std::string::npos)
                    buildingTypes.emplace(entry3.name, entry3);
                }
              }
            } catch (std::exception e) {
              Fwg::Utils::Logging::logLine(
                  "Warning: Couldn't match production method to building for "
                  "production method: ",
                  entry.name);
            }
          }
          // now figure out how much to plop, and where
          actualDemand /= std::max<int>(buildingTypes.size(), 1);
          for (auto &type : buildingTypes) {
            Fwg::Utils::Logging::logLine("Have ", type.second.name,
                                         " as option for ",
                                         produceableGood.name);
            // now get the potential production methods of this single
            // building
            std::vector<Productionmethod> buildingProdMethods;
            for (auto &prodMethod : potentialProdMethods) {
              if (type.second.productionMethods.find(prodMethod.name) !=
                  type.second.productionMethods.end()) {
                // filter by tech
                if (country->canUseProductionMethod(prodMethod)) {
                  buildingProdMethods.push_back(prodMethod);
                }
              }
            }
            // only proceed if the country actually has the production method
            // available
            if (buildingProdMethods.size()) {
              // then select the production method, by ordering them by output
              // of the good and checking if it is in the potentialProdMethods
              auto &prodMethod = Fwg::Utils::selectRandom(buildingProdMethods);
              auto outputAmount = 0;
              for (auto &prodMethodGood : prodMethod.outputs) {
                if (produceableGood.name == prodMethodGood.first.name) {
                  outputAmount = prodMethodGood.second;
                }
              }
              // then figure out how many buildings we need for the overall
              // demand
              int levelRequired =
                  std::max<int>(actualDemand / (double)outputAmount, 1);
              Fwg::Utils::Logging::logLine("Requiring ", levelRequired,
                                           " to create ", actualDemand, " ",
                                           produceableGood.name);
              // get the amount of states that support this building
              std::vector<std::pair<std::shared_ptr<Region>, int>>
                  potentialRegions;
              for (auto &region : country->ownedVic3Regions) {
                int retVal = region->supportsBuilding(type.second);
                if (retVal) {
                  potentialRegions.push_back({region, retVal});
                }
              }

              // then figure out how to distribute the buildings to states
              if (!potentialRegions.size()) {
                Fwg::Utils::Logging::logLineLevel(
                    9, "No potential state found to produce ",
                    produceableGood.name);
              } else {
                for (auto i = 0; i < levelRequired; i++) {
                  auto &region =
                      potentialRegions[i % potentialRegions.size()].first;
                  if (region->buildings.find(type.first) !=
                      region->buildings.end()) {
                    region->buildings.at(type.first).level++;
                  } else {
                    region->buildings.emplace(
                        type.first, Building{type.second, 1, prodMethod});
                  }
                }
              }
            }
          }
        }
      }
    }
    // now distribute ports
    for (auto &region : country->ownedVic3Regions) {
      if (Cfg::Values().debugLevel > 9) {
        if (region->coastal) {
          // try to find a port location
          auto portLocator =
              region->getLocation(Fwg::Civilization::LocationType::Port);
          if (portLocator != nullptr) {
            // find the building in vic3GameData.buildingsTypes with the name
            // building_port. This is a vector we search in
            auto portBuilding = std::find_if(
                vic3GameData.buildingsTypes.begin(),
                vic3GameData.buildingsTypes.end(), [](BuildingType &building) {
                  return building.name == "building_port";
                });
            region->buildings.emplace(
                "building_port",
                Building{*portBuilding, 1,
                         portBuilding->productionMethods.at("pm_basic_port")});
          }
        }
      }
    }
  }
}

void Generator::createLocators() {
  auto &config = Fwg::Cfg::Values();
  std::vector<bool> binaryLandMap(config.width * config.height);

  for (auto i = 0; i < terrainData.detailedHeightMap.size(); i++) {
    binaryLandMap[i] =
        terrainData.detailedHeightMap[i] <= config.seaLevel ? false : true;
  }
  auto searchVector = Fwg::Utils::getCircularOffsets(config.width, 10);

  for (auto &region : modData.vic3Regions) {
    region->significantLocations.clear();
    if (region->isLand()) {
      // create a locator for each building in the region
      region->findCityLocator();
      region->findFarmLocator();
      region->findMineLocator();
      region->findPortLocator();
      region->findWaterPortLocator();
      region->findWoodLocator();
    } else if (region->isSea()) {
      region->findWaterLocator();
    }
    //// now overwrite the locator position for the farm - we want to find the
    //// point most distant to the coast, if this is a coastal region
    // if (region->coastal) {
    //   // get the farm locator
    //   auto farmLocator =
    //       region->getLocation(Fwg::Civilization::LocationType::Farm);
    //   if (farmLocator != nullptr) {
    //     // gather all region pixels
    //     std::vector<int> regionPixels;
    //     for (auto &prov : region->provinces) {
    //       for (const auto pix : prov->getNonOwningPixelView()) {
    //         regionPixels.push_back(pix);
    //       }
    //     }

    //    auto ogPosition = farmLocator->position.weightedCenter;
    //    auto circDistance = Fwg::HeightGeneration::circularDistance(
    //        binaryLandMap, config.width,
    //        farmLocator->position.weightedCenter, config.seaLevel,
    //        searchVector, 10, true);
    //    if (circDistance < 5) {
    //      // now try to find the most distant point in this whole region
    //      auto maxDistance = 0;
    //      auto maxIndex = 0;
    //      for (auto &pix : regionPixels) {
    //        auto distance = Fwg::HeightGeneration::circularDistance(
    //            binaryLandMap, config.width, pix, config.seaLevel,
    //            searchVector, 10, true);
    //        if (distance > maxDistance) {
    //          maxDistance = distance;
    //          maxIndex = pix;
    //        }
    //        debugImage.setColourAtIndex(
    //            pix, Fwg::Gfx::Colour(static_cast<int>(25.0 * distance), 0,
    //            0));
    //      }
    //      farmLocator->position.weightedCenter = maxIndex;
    //      farmLocator->position.widthCenter = maxIndex % config.width;
    //      farmLocator->position.heightCenter = maxIndex / config.width;
    //      if (maxDistance < 5) {
    //        std::cout << "Warning: Farm locator still too close to coast"
    //                  << std::endl;
    //        debugImage.setColourAtIndex(ogPosition,
    //                                    Fwg::Gfx::Colour(128, 0, 255));
    //        debugImage.setColourAtIndex(maxIndex,
    //                                    Fwg::Gfx::Colour(0, 255, 255));
    //        // we must delete it
    //        region->significantLocations.erase(
    //            std::remove(region->significantLocations.begin(),
    //                        region->significantLocations.end(),
    //                        farmLocator),
    //            region->significantLocations.end());
    //        // also clear it from region locations
    //        region->locations.erase(std::remove(region->locations.begin(),
    //                                            region->locations.end(),
    //                                            farmLocator),
    //                                region->locations.end());
    //      }
    //    }
    //  }
    //}
  }
  // Png::save(debugImage, "Maps/debugImage.png", false);
}

void Generator::calculateNavalExits() {
  for (auto &region : modData.vic3Regions) {
    if (!region->isSea()) {
      //  check if we have a port locator, and take its naval exit ID
      for (auto &location : region->significantLocations) {
        if (location->type == Fwg::Civilization::LocationType::Port) {
          region->navalExit = location->portExitProvinceID;
          break;
        }
      }
    }
  }
}

void Generator::initImageExporter() {
  imageExporter = Gfx::Vic3::ImageExporter(pathcfg.gamePath, "Vic3");
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
    genCivilisationData();
    auto countryFactory = []() -> std::shared_ptr<Vic3::Country> {
      return std::make_shared<Vic3::Country>();
    };
    // generate country data
    generateCountries(countryFactory);

    // non-country stuff
    auto factory = []() -> std::shared_ptr<StrategicRegion> {
      return std::make_shared<StrategicRegion>();
    };
    generateStrategicRegions(factory);
    // Vic3 specifics:
    distributeResources();
    if (!importData(this->pathcfg.gamePath + "//game//")) {
      Fwg::Utils::Logging::logLine("ERROR: Could not import data from game "
                                   "folder. The generation has FAILED");
      return;
    }

    // handle basic development, tech level, policies,
    generateCountrySpecifics();
    diplomaticRelations();
    createMarkets();
    calculateNeeds();
    distributeBuildings();

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
  auto foundRegions = compatRegions(
      pathcfg.gamePath + "//game//map_data//state_regions//",
      pathcfg.gameModPath + "//map_data//state_regions//", modData.vic3Regions);
  compatStratRegions(pathcfg.gamePath + "//game//common//strategic_regions//",
                     pathcfg.gameModPath + "//common//strategic_regions//",
                     modData.vic3Regions, foundRegions);
  // compatReleasable(pathcfg.gamePath + "//game//common//country_creation//",
  //                  pathcfg.gameModPath + "//common//country_creation//");
  adj(pathcfg.gameModPath + "//map_data//adjacencies.csv");
  defaultMap(pathcfg.gameModPath + "//map_data//default.map", ardaProvinces);
  defines(pathcfg.gameModPath + "//common//defines//01_defines.txt");
  provinceTerrains(pathcfg.gameModPath + "//map_data//province_terrains.txt",
                   ardaProvinces);
  stateFiles(pathcfg.gameModPath + "//map_data//state_regions//00_regions.txt",
             modData.vic3Regions);
  Parsing::History::writeBuildings(
      pathcfg.gameModPath + "//common//history//buildings//00_buildings.txt",
      modData.vic3Regions);
  writeMetadata(pathcfg.gameModPath + "//.metadata//metadata.json");
  strategicRegions(
      pathcfg.gameModPath +
          "//common//strategic_regions//randVic_strategic_regions.txt",
      superRegions, modData.vic3Regions);
  cultureCommon(pathcfg.gameModPath + "//common//cultures//00_cultures.txt",
                civData.cultures);
  religionCommon(pathcfg.gameModPath + "//common//religions//religions.txt",
                 civData.religions);
  staticModifiers(pathcfg.gameModPath + "//common//static_modifiers//",
                  civData.cultures, civData.religions);
  countryCommon(pathcfg.gameModPath +
                    "//common//country_definitions//00_countries.txt",
                modData.vic3Countries, modData.vic3Regions);

  compatFile(pathcfg.gameModPath +
             "//common//country_creation//00_releasable_countries.txt");
  compatFile(pathcfg.gameModPath +
             "//common//cultures//00_additional_cultures.txt");
  compatFile(pathcfg.gameModPath +
             "//common//country_definitions//01_africa.txt");
  compatFile(pathcfg.gameModPath +
             "//common//country_definitions//01_pacific_and_australasia.txt");
  compatFile(pathcfg.gameModPath +
             "//common//country_formation//00_formable_countries.txt");
  compatFile(pathcfg.gameModPath +
             "//common//country_formation//00_major_formables.txt");
  stateHistory(pathcfg.gameModPath + "//common//history//states//00_states.txt",
               modData.vic3Regions);
  popsHistory(pathcfg.gameModPath + "//common//history//pops//00_world.txt",
              modData.vic3Regions);
  countryHistory(pathcfg.gameModPath + "//common//history//countries",
                 modData.vic3Countries);
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

void Generator::writeImages() {
  // TODO: improve handling of altitude data to not rely on image
  auto heightMap = Fwg::Gfx::displayHeightMap(terrainData.detailedHeightMap);

  imageExporter.Vic3ColourMaps(worldMap, heightMap, climateData,
                               ardaData.civLayer,
                               pathcfg.gameModPath + "//gfx//map//");
  // imageExporter.dump8BitRivers(riverMap,
  //                                pathcfg.gameModPath +
  //                                "//map_data//rivers", "rivers", false);

  imageExporter.detailMaps(terrainData, climateData, ardaData.civLayer,
                           pathcfg.gameModPath + "//gfx//map//");
  imageExporter.dynamicMasks(pathcfg.gameModPath + "//gfx//map//masks//",
                             climateData, ardaData.civLayer);
  imageExporter.contentSource(pathcfg.gameModPath +
                                  "//content_source//map_objects//masks//",
                              climateData, ardaData.civLayer);
  // save this and reset the heightmap later. The map will be scaled and the
  // scaled one then used for the packed heightmap generation. It is important
  // we reset this after
  auto temporaryHeightmap = heightMap;
  // also dump uncompressed packed heightmap
  imageExporter.dump8BitHeightmap(terrainData.detailedHeightMap,
                                  pathcfg.gameModPath + "//map_data//heightmap",
                                  "heightmap");
  auto packedHeightmap = imageExporter.dumpPackedHeightmap(
      heightMap, pathcfg.gameModPath + "//map_data//packed_heightmap",
      "heightmap");
  imageExporter.dumpIndirectionMap(
      heightMap, pathcfg.gameModPath + "//map_data//indirection_heightmap.png");
  Parsing::Writing::heightmap(pathcfg.gameModPath +
                                  "//map_data//heightmap.heightmap",
                              heightMap, packedHeightmap);
  heightMap = temporaryHeightmap;
  temporaryHeightmap.clear();
  visualiseCountries(countryMap);
  Fwg::Gfx::Png::save(countryMap, Cfg::Values().mapsPath + "countries.png");
  using namespace Fwg::Gfx;
  // just copy over provinces.bmp as a .png, already in a compatible format
  // auto scaledMap = Util::scale(provinceMap, 8192, 3616, false);
  Png::save(provinceMap, pathcfg.gameModPath + "//map_data//provinces.png");
}

void Generator::writeSplnet() {
  createLocators();
  Parsing::Writing::locators(pathcfg.gameModPath +
                                 "//gfx//map//map_object_data//",
                             modData.vic3Regions);
  genNavmesh();
  calculateNavalExits();

  Splnet splnet;
  splnet.constructSplnet(ardaRegions);
  splnet.writeFile(pathcfg.gameModPath +
                   "//gfx//map//spline_network//spline_network.splnet");
}

} // namespace Rpx::Vic3