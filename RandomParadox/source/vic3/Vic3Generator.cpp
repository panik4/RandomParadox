#include "vic3/Vic3Generator.h"
namespace Scenario::Vic3 {
using namespace Fwg;
using namespace Fwg::Gfx;

Generator::Generator() : Scenario::Generator() {}

Generator::Generator(const std::string &configSubFolder)
    : Scenario::Generator(configSubFolder) {
  // this->terrainTypeToString.at(Fwg::Province::TerrainType::marsh) =
  // "wetlands";
  // this->terrainTypeToString.at(Fwg::Province::TerrainType::savannah) =
  //     "savanna";
  // this->terrainTypeToString.at(Fwg::Province::TerrainType::tundra) = "snow";
  // this->terrainTypeToString.at(Fwg::Province::TerrainType::arctic) = "snow";
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
  const auto &landForms = climateData.landForms;
  const auto &climates = climateData.climates;
  const auto &forests = climateData.dominantForest;
  for (auto &gameRegion : gameRegions) {
    for (auto &gameProv : gameRegion->gameProvinces) {
      gameProv->terrainType = "plains";
      const auto &baseProv = gameProv->baseProvince;
      if (baseProv->isLake) {
        gameProv->terrainType = "lake";
        for (auto &pix : baseProv->pixels) {
          typeMap.setColourAtIndex(pix, colours.at("lake"));
        }
      } else if (baseProv->sea) {
        gameProv->terrainType = "sea";
        for (auto &pix : baseProv->pixels) {
          typeMap.setColourAtIndex(pix, climateColours.at("ocean"));
        }
      } else {
        int forestPixels = 0;
        std::map<ClimateGeneration::Detail::ClimateTypeIndex, int>
            climateScores;
        std::map<Fwg::ElevationTypeIndex, int> terrainTypeScores;
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
        if (dominantTerrain == Fwg::ElevationTypeIndex::MOUNTAINS ||
            dominantTerrain == Fwg::ElevationTypeIndex::PEAKS ||
            dominantTerrain == Fwg::ElevationTypeIndex::STEEPPEAKS) {
          gameProv->terrainType = "mountain";
          for (auto &pix : baseProv->pixels) {
            typeMap.setColourAtIndex(pix, elevationColours.at("mountains"));
          }
        } else if (dominantTerrain == Fwg::ElevationTypeIndex::HILLS) {
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
          using CTI = ClimateGeneration::Detail::ClimateTypeIndex;
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
  Png::save(typeMap, "Maps/typeMap.png");
  return typeMap;
}

void Generator::distributePops() {
  auto targetWorldPop = 1'000'000'000.0 * worldPopulationFactor;
  for (auto &region : vic3Regions) {
    if (region->sea)
      continue;
    // only init this when it hasn't been initialized via text input before
    if (region->totalPopulation < 0) {
      region->totalPopulation =
          static_cast<int>(targetWorldPop * region->worldPopulationShare);
    }
    worldPop += (long long)region->totalPopulation;
  }
}
void Generator::totalArableLand(const std::vector<float> &arableLand) {
  const auto baseWorldArableSlots = 50000.0;
  auto totalArable = 0.0f;
  for (auto &val : arableLand) {
    totalArable += val;
  }
  for (auto &reg : vic3Regions) {
    auto resShare = 0.0;
    for (const auto &prov : reg->provinces) {
      for (const auto &pix : prov->pixels) {
        resShare += arableLand[pix];
      }
    }
    // basically fictive value from given input of how often this resource
    // appears
    auto stateArable = baseWorldArableSlots * (resShare / totalArable);
    reg->arableLand = stateArable;
  }
}
void Generator::totalResourceVal(const std::vector<double> &resPrev,
                                 double resourceModifier,
                                 const ResConfig &resourceConfig) {
  const auto baseResourceAmount = 2500.0 * resourceModifier;
  auto totalRes = 0.0;
  for (auto &val : resPrev) {
    totalRes += val;
  }
  for (auto &reg : vic3Regions) {
    auto resShare = 0.0;
    for (const auto &prov : reg->provinces) {
      for (const auto &pix : prov->pixels) {
        resShare += resPrev[pix];
      }
    }
    // basically fictive value from given input of how often this resource
    // appears
    auto stateRes = baseResourceAmount * (resShare / totalRes);
    reg->resources.insert(
        {resourceConfig.name,
         {resourceConfig.name, resourceConfig.capped, stateRes}});
  }
}

void Generator::distributeResources() {
  const auto &cfg = Fwg::Cfg::Values();

  // distribute arable land to states
  totalArableLand(climateData.arableLand);

  // config for all resource types

  for (auto &resConfig : resConfigs) {
    std::vector<double> resPrev;
    if (resConfig.random) {
      resPrev = Fwg::Civilization::Resources::randomResourceLayer(
          resConfig.name, resConfig.noiseConfig.fractalFrequency,
          resConfig.noiseConfig.tanFactor, resConfig.noiseConfig.cutOff,
          resConfig.noiseConfig.mountainBonus);
    } else if (resConfig.considerSea) {
      resPrev = Fwg::Civilization::Resources::coastDependentLayer(
          resConfig.name, resConfig.oceanFactor, resConfig.lakeFactor,
          areas.provinces);
    } else {
      resPrev = Fwg::Civilization::Resources::climateDependentLayer(
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
  gameRegions.clear();
  vic3Regions.clear();

  for (auto &region : this->areas.regions) {
    std::sort(region.provinces.begin(), region.provinces.end(),
              [](const Fwg::Province *a, const Fwg::Province *b) {
                return (*a < *b);
              });
    auto gameRegion = std::make_shared<Region>(region);
    // generate random name for region
    gameRegion->name = NameGeneration::generateName(nData);

    for (auto &province : gameRegion->provinces) {
      gameRegion->gameProvinces.push_back(gameProvinces[province->ID]);
    }
    // save game region to generic module container and to hoi4 specific
    // container
    gameRegions.push_back(gameRegion);
    vic3Regions.push_back(gameRegion);
  }
  // sort by gameprovince ID
  std::sort(gameRegions.begin(), gameRegions.end(),
            [](auto l, auto r) { return *l < *r; });
  // check if we have the same amount of gameProvinces as FastWorldGen
  // provinces
  if (gameProvinces.size() != this->areas.provinces.size())
    throw(std::exception("Fatal: Lost provinces, terminating"));
  if (gameRegions.size() != this->areas.regions.size())
    throw(std::exception("Fatal: Lost regions, terminating"));
  for (const auto &gameRegion : gameRegions) {
    if (gameRegion->ID > gameRegions.size()) {
      throw(std::exception("Fatal: Invalid region IDs, terminating"));
    }
  }
  applyRegionInput();
}
// initialize states
void Generator::initializeStates() {}
void Generator::mapCountries() {
  for (auto &country : countries) {
    std::shared_ptr<Country> vic3Country =
        std::dynamic_pointer_cast<Vic3::Country, Scenario::Country>(
            country.second);
    if (vic3Country != nullptr) {
      for (auto &region : vic3Country->ownedRegions) {
        // vic3Country->ownedVic3Regions.push_back(
        //     std::reinterpret_pointer_cast<Vic3::Region, Scenario::Region>(
        //         region));

        vic3Country->ownedVic3Regions.push_back(
            std::dynamic_pointer_cast<Region>(region));
      }
    } else {

      Fwg::Utils::Logging::logLine("Warning: Country ", country.first,
                                   " not found in Vic3 countries");
    }
    vic3Countries.emplace(country.first, vic3Country);
  }
}
// set tech levels, give techs, count pops, cultures and religions, set
// diplomatic relations (e.g. puppets, markets, protectorates)
void Generator::generateCountrySpecifics() {
  auto &cfg = Fwg::Cfg::Values();
  // count pops
  for (auto &cEntry : vic3Countries) {
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
      averageDevelopment += state->developmentFactor *
                            ((double)state->totalPopulation / (double)totalPop);
    }
    // c->developmentFactor = averageDevelopment;
    c->evaluateTechLevel(techLevels);
    if (cfg.debugLevel > 5) {
      Fwg::Utils::Logging::logLine(c->tag, " has a population of ", c->pop);
    }
  }
}
bool Generator::importData(const std::string &path) {
  try {
    techs =
        Vic3::Importing::readTechs(path + "common//technology//technologies//");
    techLevels = Vic3::Importing::readTechLevels(
        path + "common//scripted_effects//00_starting_inventions.txt", techs);
    goods = Vic3::Importing::readGoods(path + "common//goods//00_goods.txt");
    productionmethods = Vic3::Importing::readProdMethods(
        path + "common//production_methods//", goods, techs);
    productionmethodGroups = Vic3::Importing::readProdMethodGroups(
        path + "common//production_method_groups//", productionmethods);
    buildingsTypes = Vic3::Importing::readBuildings(
        path + "common//buildings//", productionmethodGroups, techs);
    popNeeds = Vic3::Importing::readPopNeeds(
        path + "common//pop_needs//00_pop_needs.txt", goods);
    buypackages = Vic3::Importing::readBuypackages(
        path + "//common//buy_packages//00_buy_packages.txt", popNeeds);
    nData.disallowedTokens =
        Vic3::Importing::readTags(path + "common//history//countries//");
  } catch (std::exception e) {
    Fwg::Utils::Logging::logLine("Error: ", e.what());
    return false;
  }
  // now map goods to building types
  for (auto &good : goods) {
    std::vector<Productionmethod> prodMethods;
    for (auto &productionmethod : productionmethods) {
      // ignore subsistence buildings
      if (productionmethod.first.find("subsistence") != std::string::npos) {
        continue;
      }
      for (auto &outputGood : productionmethod.second.outputs) {
        // this building has a production method that outputs this good
        if (good.first == outputGood.first.name && outputGood.second > 0) {
          goodToProdMethodsOutput[good.first].push_back(
              productionmethod.second);
        }
      }
      for (auto &inputGood : productionmethod.second.inputs) {
        // this building has a production method that inputs this good
        if (good.first == inputGood.first.name && inputGood.second > 0) {
          goodToProdMethodsInput[good.first].push_back(productionmethod.second);
        }
      }
    }
  }
  for (const auto &buildingType : buildingsTypes) {
    for (const auto &buildingProdGroups : buildingType.productionMethodGroups) {
      for (const auto &buildingProductionmethod :
           buildingProdGroups.productionMethods) {
        for (const auto &prodMethod : productionmethods) {
          if (prodMethod.first == buildingProductionmethod.first) {
            productionMethodToBuildingTypes[prodMethod.first].push_back(
                buildingType);
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

  for (auto &country : vic3Countries) {
    std::map<std::string, double> summedPopNeeds;
    auto wealth = country.second->developmentFactor;
    // guesstimate size of wealth groups from development
    auto wealthDispersion = shiftedGaussian(wealth);
    double pop = country.second->pop;
    for (int i = 0; i < wealthDispersion.size(); i++) {
      // get the buypackage, it tells us which popneeds we have for this wealth
      auto buyPackage = buypackages.at("wealth_" + std::to_string(i + 1));
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
        5, country.first, " requires goods with development: ", wealth);
    for (auto &rq : summedPopNeeds) {
      Fwg::Utils::Logging::logLineLevel(5, rq.first, " needed ", rq.second);
    }
    country.second->summedPopNeeds = summedPopNeeds;
  }
}
void Generator::distributeBuildings() {
  for (auto &countryEntry : vic3Countries) {
    auto &country = countryEntry.second;
    for (auto &need : country->summedPopNeeds) {
      auto &popNeed = popNeeds.at(need.first);
      auto amount = need.second;
      // this is the goods used in the popneed to fuilfill demand
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
        if (actualDemand > 5.0) {
          // find production methods that produce this good
          const auto &potentialProdMethods =
              goodToProdMethodsOutput.at(produceableGood.name);

          // now find all buildings for all the production methods, so we get a
          // complete list of prod methods we support
          std::map<std::string, BuildingType> buildingTypes;
          for (auto &entry : potentialProdMethods) {
            // check against techs, this is the decisive factor in
            // evaluating if a building can actually be built already to
            // fulfill the demand
            if (!country->canUseProductionMethod(entry)) {
              continue;
            }

            try {
              if (productionMethodToBuildingTypes.find(entry.name) !=
                  productionMethodToBuildingTypes.end()) {
                auto &entry2 = productionMethodToBuildingTypes.at(entry.name);
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
            // now get the potential production methods of this single building
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
                int retVal = 0;
                if (retVal = region->supportsBuilding(type.second)) {
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
  }
}
void Generator::createLocators() {
  auto &cfg = Fwg::Cfg::Values();

  for (auto &region : vic3Regions) {
    if (region->sea || region->lake)
      continue;
    region->significantLocations.clear();
    // create a locator for each building in the region
    region->findCityLocator();
    region->findFarmLocator();
    region->findMineLocator();
    region->findPortLocator();
    region->findWoodLocator();
  }
}
} // namespace Scenario::Vic3