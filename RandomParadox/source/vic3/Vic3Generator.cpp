#include "vic3/Vic3Generator.h"
namespace Scenario::Vic3 {
using namespace Fwg;
using namespace Fwg::Gfx;

Generator::Generator() : Scenario::Generator() {}

Generator::Generator(const std::string &configSubFolder)
    : Scenario::Generator(configSubFolder) {
  this->terrainTypeToString.at(Fwg::Province::TerrainType::marsh) = "wetlands";
  this->terrainTypeToString.at(Fwg::Province::TerrainType::savannah) =
      "savanna";
  this->terrainTypeToString.at(Fwg::Province::TerrainType::tundra) = "snow";
  this->terrainTypeToString.at(Fwg::Province::TerrainType::arctic) = "snow";
}

void Generator::distributePops() {
  auto overallpopFactor =
      1000000.0 * worldPopulationFactor * (1.0 / Fwg::Cfg::Values().sizeFactor);
  for (auto &region : vic3Regions) {
    if (region->sea)
      continue;

    //// only init this when it hasn't been initialized via text input before
    if (region->totalPopulation < 0) {
      region->totalPopulation =
          static_cast<int>(region->populationFactor * overallpopFactor);
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

    std::shared_ptr<Country> vic3 =
        std::reinterpret_pointer_cast<Vic3::Country, Scenario::Country>(
            country.second);
    for (auto &region : vic3->ownedRegions) {
      vic3->ownedVic3Regions.push_back(
          std::reinterpret_pointer_cast<Vic3::Region, Scenario::Region>(
              region));
    }
    vic3Countries.emplace(country.first, vic3);
  }
}
// set tech levels, give techs, count pops, cultures and religions, set
// diplomatic relations (e.g. puppets, markets, protectorates)
void Generator::initializeCountries() {
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
      averageDevelopment += state->development *
                            ((double)state->totalPopulation / (double)totalPop);
    }
    if (cfg.debugLevel > 5) {
      Fwg::Utils::Logging::logLine(c->tag, " has a population of ", c->pop);
    }
  }
}
void Generator::importData(const std::string &path) {
  goods = Vic3::Importing::readGoods(path + "common//goods//00_goods.txt");
  productionmethods = Vic3::Importing::readProdMethods(
      path + "common//production_methods//", goods);
  productionmethodGroups = Vic3::Importing::readProdMethodGroups(
      path + "common//production_method_groups//", productionmethods);
  buildingsTypes = Vic3::Importing::readBuildings(path + "common//buildings//",
                                                  productionmethodGroups);
  popNeeds = Vic3::Importing::readPopNeeds(
      path + "common//pop_needs//00_pop_needs.txt", goods);
  buypackages = Vic3::Importing::readBuypackages(
      path + "//common//buy_packages//00_buy_packages.txt", popNeeds);

  // now map goods to building types
  for (auto &good : goods) {
    std::vector<Productionmethod> prodMethods;
    for (auto &productionmethod : productionmethods) {
      for (auto &outputGood : productionmethod.second.outputs) {
        // this building has a production method that outputs this good
        if (good.first == outputGood.first.name) {
          goodToProdMethodsOutput[good.first].push_back(
              productionmethod.second);
        }
      }
      for (auto &inputGood : productionmethod.second.inputs) {
        // this building has a production method that outputs this good
        if (good.first == inputGood.first.name) {
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
}
void Generator::diplomaticRelations() {}
void Generator::createMarkets() {}

std::vector<double> shiftedGaussian(double input) {
  std::vector<double> values(15, 0.0);

  // Calculate the mean and standard deviation based on the input
  double mean = 1.0 + input * 14.0;  // Scale mean between 1 and 15
  double stdDev = 0.1 + input * 5.0; // Scale std deviation between 0.1 and 2.1

  // Create a random number generator
  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<double> distribution(mean, stdDev);

  // Generate 15 values representing the distribution at each integer value
  // between 1 and 15
  for (int i = 0; i < 15; ++i) {
    double x = i + 1; // Value between 1 and 15
    values[i] = std::exp(-0.5 * std::pow((x - mean) / stdDev, 2)) /
                (stdDev * std::sqrt(2.0 * 3.14));
  }

  // Normalize the values such that their sum equals 1.0
  double sum = 0.0;
  for (double value : values) {
    sum += value;
  }
  for (double &value : values) {
    value /= sum;
  }

  return values;
}

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
      for (auto &good : goods) {
        // to check if we can actually produce this
        try {
          const auto &potentialProdMethods =
              goodToProdMethodsOutput.at(good.name);
          // TODO: insert check against techs, this is the decisive factor in
          // evaluating if a building can actually be built already to fulfill
          // the demand
          produceableGoods.push_back(good);
        } catch (std::exception e) {
          Fwg::Utils::Logging::logLineLevel(
              9, "Couldn't find production method for ", good.name);
        }
      }
      // now find the building that produces this, and plop it into a state
      for (auto &produceableGood : produceableGoods) {
        double actualDemand =
            amount / (double)produceableGoods.size() / produceableGood.cost;
        // filter out tiny demands for now: TODO
        if (actualDemand > 5.0) {
          // find production methods that produce this good
          const auto &potentialProdMethods =
              goodToProdMethodsOutput.at(produceableGood.name);
          for (auto i = 0; i < potentialProdMethods.size(); i++) {
            // TODO: insert check against techs, this is the decisive factor in
            // evaluating if a building can actually be built already to fulfill
            // the demand
          }

          // now find all buildings for all the production methods, so we get a
          // complete list of prod methods we support
          std::map<std::string, BuildingType> buildingTypes;
          for (auto &entry : potentialProdMethods) {
            try {
              if (productionMethodToBuildingTypes.find(entry.name) !=
                  productionMethodToBuildingTypes.end()) {
                auto &entry2 = productionMethodToBuildingTypes.at(entry.name);
                for (auto &entry3 : entry2) {
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
          for (auto &type : buildingTypes) {
            if (type.second.name.find("subsistence") != std::string::npos) {
              continue;
            }
            Fwg::Utils::Logging::logLine("Have ", type.second.name,
                                         " as option for ",
                                         produceableGood.name);

            // now check if we have the tech, if not, we can't build the
            // building

            // then select the production method, by ordering them by output of
            // the good and checking if it is in the potentialProdMethods

            // then figure out how many buildings we need for the overall demand

            // get the amount of states that support this building

            // then figure out how to distribute the buildings to states








            // const auto& buildingOutputs =
            //     type.productionMethodGroups[0].productionMethods[0].outputs;
            // auto a = type.productionMethodGroups[0];
            //// find the lowest outputting production method
            // std::map<Good, int> selectedOutput;
            // for (auto &prodMeth : a.productionMethods) {
            //   // go through the outputs
            //   for (auto &outputs : prodMeth.second.outputs) {
            //     // if we already have selected a production method, make sure
            //     // the output is lower
            //     if (!selectedProdMeth.outputs.size()) {
            //       selectedProdMeth = prodMeth.second;
            //     } else {
            //       if (outputs.second > selectedProdMeth.outputs)
            //     }
            //   }
            // }

            // now find the productionmethodgroup that actually outputs the good
            // required

            // auto buildingOutputs = b.outputs;
            // for (const auto &output : buildingOutputs) {
            //   // the good we are actually trying to produce
            //   if (output.first.name == produceableGood.name) {
            //     auto outputAmount = output.second;
            //     int levelRequired = actualDemand / (double)outputAmount;
            //     if (levelRequired > 0) {
            //       // find state to put the buildings
            //       // TODO: disperse buildings between states
            //       auto &state =
            //           Fwg::Utils::selectRandom(country->ownedVic3Regions);
            //       state->buildings.push_back(Building(
            //           type, levelRequired, type.productionMethodGroups[0]));
            //       country->buildings.push_back(Building(
            //           type, levelRequired, type.productionMethodGroups[0]));
            //     }
            //   }
            // }
          }
        }
      }
    }
  }
}
} // namespace Scenario::Vic3