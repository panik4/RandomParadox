#include "hoi4/Hoi4Generator.h"
using namespace Fwg;
using namespace Fwg::Gfx;
namespace Scenario::Hoi4 {
Generator::Generator() {}

Generator::Generator(const std::string &configSubFolder)
    : Scenario::Generator(configSubFolder) {}

Generator::~Generator() {}

void Generator::mapRegions() {
  Fwg::Utils::Logging::logLine("Mapping Regions");
  gameRegions.clear();
  hoi4States.clear();

  for (auto &region : this->areas.regions) {
    std::sort(region.provinces.begin(), region.provinces.end(),
              [](const Fwg::Province *a, const Fwg::Province *b) {
                return (*a < *b);
              });
    auto gameRegion = std::make_shared<Region>(region);
    // generate random name for region
    gameRegion->name = "";

    for (auto &province : gameRegion->provinces) {
      gameRegion->gameProvinces.push_back(gameProvinces[province->ID]);
    }
    // save game region to generic module container and to hoi4 specific
    // container
    gameRegions.push_back(gameRegion);
    hoi4States.push_back(gameRegion);
  }
  // sort by gameprovince ID
  std::sort(gameRegions.begin(), gameRegions.end(),
            [](auto l, auto r) { return *l < *r; });
  // check if we have the same amount of gameProvinces as FastWorldGen provinces
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
  generateUrbanisation();
  return typeMap;
}

/*Get the generator in the correct state first, e.g. load all the maps in.
 * They can be modified later still. However, generation of early steps
 * would then require generation and therefore overwriting of previously cut
 * parts */
void Generator::cutFromFiles(const std::string &gamePath) {
  // first, cut the heightmap
  auto heightMapPath = gamePath + "//heightMap.bmp";
}
void Generator::mapCountries() {
  hoi4Countries.clear();
  for (auto &country : countries) {
    // construct a hoi4country with country from ScenarioGenerator.
    // We want a copy here
    Hoi4Country hC(*country.second, this->hoi4States);
    hoi4Countries.insert({hC.tag, hC});
  }
  std::sort(hoi4States.begin(), hoi4States.end(),
            [](auto l, auto r) { return *l < *r; });
}

void Generator::generateStateResources() {
  Fwg::Utils::Logging::logLine("HOI4: Digging for resources");

  for (const auto &resource : resources) {
    totalResources[resource.first] = 0;
  }
  for (auto &hoi4Region : hoi4States) {
    for (const auto &resource : resources) {
      auto chance = resource.second[2];
      if (RandNum::getRandom(100) < chance * 100.0) {
        // calc total of this resource
        auto totalOfResource = resource.second[1] * resource.second[0];
        // more per selected state if the chance is lower
        double averagePerState =
            (totalOfResource / (double)areas.landRegions) * (1.0 / chance);
        // range 1 to (2 times average - 1)
        double value =
            1.0 +
            (RandNum::getRandom((int)ceil((2.0 * averagePerState)) - 1.0));
        // increase by industry factor
        value *= worldIndustryFactor;
        hoi4Region->resources[resource.first] = (int)value;
        totalResources[resource.first] += (int)value;
      }
    }
  }
}

void Generator::generateStateSpecifics() {
  Fwg::Utils::Logging::logLine("HOI4: Planning the economy");
  auto &config = Cfg::Values();
  // calculate the target industry amount
  auto targetWorldIndustry = 1248.0 * worldIndustryFactor;
  auto targetWorldPop = 3'000'000'000.0 * worldPopulationFactor;
  // we need a reference to determine how industrious a state is
  double averageEconomicActivity = 1.0 / areas.landRegions;

  worldPop = 0;
  militaryIndustry = 0;
  civilianIndustry = 0;
  navalIndustry = 0;
  // cleanup work
  for (auto &hoi4State : hoi4States) {
    hoi4State->dockyards = 0;
    hoi4State->civilianFactories = 0;
    hoi4State->armsFactories = 0;
  }

  // go through all states and figure out the importance of the largest port
  // in the state
  auto maxImportance = 0.0;
  for (auto &hoi4State : hoi4States) {
    // create naval bases for all port locations
    for (auto &location : hoi4State->locations) {
      if (location->type == Fwg::Civilization::LocationType::Port ||
          location->secondaryType == Fwg::Civilization::LocationType::Port) {
        maxImportance = std::max<double>(maxImportance, location->importance);
      }
    }
  }
  Fwg::Utils::Logging::logLine(config.landPercentage);
  for (auto &hoi4State : hoi4States) {
    // skip sea and lake states
    if (hoi4State->sea || hoi4State->lake)
      continue;

    // state level is calculated from population and development
    hoi4State->stateCategory =
        std::clamp((int)(2.0 + 3.0 * hoi4State->worldEconomicActivityShare /
                                   averageEconomicActivity),
                   0, 9);
    hoi4State->infrastructure =
        std::clamp((int)(1.0 + hoi4State->worldEconomicActivityShare /
                                   averageEconomicActivity),
                   1, 5);
    // one province state? Must be an island state
    if (hoi4State->gameProvinces.size() == 1) {
      // if only one province, should be an island. Make it an island state,
      // if it isn't more developed
      hoi4State->stateCategory = std::max<int>(1, hoi4State->stateCategory);
    }

    hoi4State->totalPopulation =
        static_cast<int>(targetWorldPop * hoi4State->worldPopulationShare);
    worldPop += (long long)hoi4State->totalPopulation;

    // create naval bases for all port locations
    for (auto &location : hoi4State->locations) {
      if (location->type == Fwg::Civilization::LocationType::Port ||
          location->secondaryType == Fwg::Civilization::LocationType::Port) {
        hoi4State->navalBases[location->provinceID] =
            std::max<double>(location->importance / maxImportance, 1.0);
        Fwg::Utils::Logging::logLineLevel(
            5, "Naval base in ", hoi4State->name, " at ", location->provinceID,
            " with importance ", location->importance);
      }
    }
    double dockChance = 0.25;
    double civChance = 0.5;
    // distribute it to military, civilian and naval factories
    if (!hoi4State->coastalProvinces) {
      dockChance = 0.0;
      civChance = 0.6;
    }

    // calculate total industry in this state
    if (targetWorldIndustry != 0) {
      auto stateIndustry = std::min<double>(
          hoi4State->worldEconomicActivityShare * targetWorldIndustry, 12.0);
      // if we're below one, randomize if this state gets a actory or not
      if (stateIndustry < 1.0) {
        stateIndustry =
            RandNum::getRandom(0.0, 1.0) < stateIndustry ? 1.0 : 0.0;
      }

      while (--stateIndustry >= 0) {
        auto choice = RandNum::getRandom(0.0, 1.0);
        if (choice < dockChance) {
          hoi4State->dockyards++;
        } else if (Fwg::Utils::inRange(dockChance, dockChance + civChance,
                                       choice)) {
          hoi4State->civilianFactories++;

        } else {
          hoi4State->armsFactories++;
        }
      }
    }
    militaryIndustry += (int)hoi4State->armsFactories;
    civilianIndustry += (int)hoi4State->civilianFactories;
    navalIndustry += (int)hoi4State->dockyards;
    // get potential building positions
    hoi4State->calculateBuildingPositions(this->heightMap, typeMap);
  }
  dumpRegions(hoi4States);
}

void Generator::generateCountrySpecifics() {
  Fwg::Utils::Logging::logLine("HOI4: Choosing uniforms and electing Tyrants");

  // graphical culture pairs:
  // { graphical_culture = type }
  // { graphical_culture_2d = type_2d }
  // {western_european_gfx, western_european_2d}
  // {eastern_european_gfx, eastern_european_2d}
  // {middle_eastern_gfx, middle_eastern_2d}
  // {african_gfx, african_2d}
  // {southamerican_gfx, southamerican_2d}
  // {commonwealth_gfx, commonwealth_2d}
  // {asian_gfx, asian_2d}
  const std::vector<std::string> gfxCultures{
      "western_european", "eastern_european", "middle_eastern", "african",
      "southamerican",    "commonwealth",     "asian"};
  const std::vector<std::string> ideologies{"fascism", "democratic",
                                            "communism", "neutrality"};
  mapCountries();
  for (auto &country : hoi4Countries) {
    // select a random country ideology
    country.second.gfxCulture = Fwg::Utils::selectRandom(gfxCultures);
    std::vector<double> popularities{};
    double totalPop = 0;
    for (int i = 0; i < 4; i++) {
      popularities.push_back(RandNum::getRandom(1, 100));
      totalPop += popularities[i];
    }
    auto sumPop = 0;
    for (int i = 0; i < 4; i++) {
      popularities[i] = popularities[i] / totalPop * 100;
      sumPop += (int)popularities[i];
      int offset = 0;
      // to ensure a total of 100 as the sum for all ideologies
      if (i == 3 && sumPop < 100) {
        offset = 100 - sumPop;
      }
      country.second.parties[i] = (int)popularities[i] + offset;
    }
    // assign a ruling party
    country.second.rulingParty =
        ideologies[RandNum::getRandom(0, (int)ideologies.size())];
    // allow or forbid elections
    if (country.second.rulingParty == "democratic")
      country.second.allowElections = 1;
    else if (country.second.rulingParty == "neutrality")
      country.second.allowElections = RandNum::getRandom(0, 1);
    else
      country.second.allowElections = 0;
    // now get the full name of the country
    country.second.fullName = NameGeneration::modifyWithIdeology(
        country.second.rulingParty, country.second.name,
        country.second.adjective, nData);
  }
}

void Generator::generateWeather() {
  for (auto &strat : strategicRegions) {
    for (auto &reg : strat.gameRegions) {
      for (auto i = 0; i < 12; i++) {
        double averageTemperature = 0.0;
        double averageDeviation = 0.0;
        double averagePrecipitation = 0.0;
        for (auto &prov : reg->gameProvinces) {
          averageDeviation += prov->baseProvince->weatherMonths[i][0];
          averageTemperature += prov->baseProvince->weatherMonths[i][1];
          averagePrecipitation += prov->baseProvince->weatherMonths[i][2];
        }
        double divisor = (int)reg->gameProvinces.size();
        averageDeviation /= divisor;
        averageTemperature /= divisor;
        averagePrecipitation /= divisor;
        // now save monthly data, 0, 1, 2
        strat.weatherMonths.push_back(
            {averageDeviation, averageTemperature, averagePrecipitation});
        // temperature low, 3
        strat.weatherMonths[i].push_back(Cfg::Values().minimumDegCelcius +
                                         averageTemperature *
                                             Cfg::Values().temperatureRange);
        // tempHigh, 4
        strat.weatherMonths[i].push_back(
            Cfg::Values().minimumDegCelcius +
            averageTemperature * Cfg::Values().temperatureRange +
            averageDeviation * Cfg::Values().deviationFactor);
        // light_rain chance: cold and humid -> high, 5
        strat.weatherMonths[i].push_back(
            this->weatherChances.at("baseLightRainChance") *
            (1.0 - averageTemperature) * averagePrecipitation);
        // heavy rain chance: warm and humid -> high, 6
        strat.weatherMonths[i].push_back(
            this->weatherChances.at("baseHeavyRainChance") *
            averageTemperature * averagePrecipitation);
        // mud chance, 7
        strat.weatherMonths[i].push_back(
            this->weatherChances.at("baseMudChance") *
            (2.0 * strat.weatherMonths[i][6] + strat.weatherMonths[i][5]));
        // blizzard chance, 8
        strat.weatherMonths[i].push_back(
            std::clamp(this->weatherChances.at("baseBlizzardChance") -
                           averageTemperature,
                       0.0, 0.2) *
            averagePrecipitation);
        // sandstorm chance, 9
        auto snadChance =
            std::clamp((averageTemperature - 0.8) *
                           this->weatherChances.at("baseSandstormChance"),
                       0.0, 0.1) *
            std::clamp(0.2 - averagePrecipitation, 0.0, 0.2);
        strat.weatherMonths[i].push_back(snadChance);
        // snow chance, 10
        strat.weatherMonths[i].push_back(
            std::clamp(this->weatherChances.at("baseSnowChance") -
                           averageTemperature,
                       0.0, 0.2) *
            averagePrecipitation);
        // no phenomenon chance, 11
        strat.weatherMonths[i].push_back(
            1.0 - strat.weatherMonths[i][5] - strat.weatherMonths[i][6] -
            strat.weatherMonths[i][8] - strat.weatherMonths[i][9] -
            strat.weatherMonths[i][10]);
      }
    }
  }
}

void Generator::generateLogistics() {
  Fwg::Utils::Logging::logLine("HOI4: Building rail networks");
  supplyNodeConnections.clear();
  auto width = Cfg::Values().width;
  // create a copy of the country map for
  // visualisation of the logistics
  auto logistics = this->countryMap;
  for (auto &country : hoi4Countries) {
    // GameProvince ID, distance
    std::map<double, int> supplyHubs;
    // add capital
    auto capitalPosition =
        gameRegions[country.second.capitalRegionID]->position;
    auto &capitalProvince = Fwg::Utils::selectRandom(
        gameRegions[country.second.capitalRegionID]->gameProvinces);
    std::vector<double> distances;
    // region ID, provinceID
    std::map<int, std::shared_ptr<GameProvince>> supplyHubProvinces;
    std::map<int, bool> navalBases;
    std::set<int> gProvIDs;
    for (auto &region : country.second.hoi4Regions) {
      if ((region->stateCategory > 4 &&
           region->ID != country.second.capitalRegionID)
          // if we're nearing the end of our region std::vector, and don't
          // have more than 25% of our regions as supply bases generate
          // supply bases for the last two regions
          || (country.second.hoi4Regions.size() > 2 &&
              (region->ID == (*(country.second.hoi4Regions.end() - 2))->ID) &&
              supplyHubProvinces.size() <
                  (country.second.hoi4Regions.size() / 4))) {
        if (region->sea || region->lake)
          continue;
        // select a random gameprovince of the state
        int lakeCounter = 0;
        auto hubProvince{Fwg::Utils::selectRandom(region->gameProvinces)};
        while (hubProvince->baseProvince->isLake && lakeCounter++ < 1000) {
          hubProvince = Fwg::Utils::selectRandom(region->gameProvinces);
        }
        // just skip a lake
        if (lakeCounter >= 1000) {
          Fwg::Utils::Logging::logLine("Error: Skipping a region for logistics "
                                       "as it only contains lakes");
          continue;
        }
        // search for a port location in this region. If we have one, overwrtie
        // hubProvince with the location province
        for (auto &location : region->locations) {
          if (location->type == Fwg::Civilization::LocationType::Port ||
              location->secondaryType ==
                  Fwg::Civilization::LocationType::Port) {
            hubProvince = gameProvinces[location->provinceID];
            break;
          }
        }

        // save the province under the provinces ID
        supplyHubProvinces[hubProvince->ID] = hubProvince;
        navalBases[hubProvince->ID] = hubProvince->baseProvince->coastal;
        // get the distance between this supply hub and the capital
        auto distance = Fwg::getPositionDistance(
            capitalPosition, hubProvince->baseProvince->position, width);
        // save the distance under the province ID
        supplyHubs[distance] = hubProvince->ID;
        // save the distance
        distances.push_back(distance); // save distances to ensure ordering
      }
      for (auto &gProv : region->gameProvinces) {
        gProvIDs.insert(gProv->ID);
      }
    }
    std::sort(distances.begin(), distances.end());
    for (const auto distance : distances) {
      std::vector<int> passthroughProvinceIDs;
      int attempts = 0;
      auto sourceNodeID = capitalProvince->ID;
      supplyNodeConnections.push_back({sourceNodeID});
      do {
        attempts++;
        // the region we want to connect to the source
        auto destNodeID = supplyHubs[distance];
        if (sourceNodeID == capitalProvince->ID) {
          // we are at the start of the search
          // distance to capital
          auto tempDistance = distance;
          for (auto distance2 : distances) {
            // only check hubs that were already assigned
            if (distance2 < distance) {
              // distance is the distance between us and the capital
              // now find distance2, the distance between us and the other
              // already assigned supply hubs
              auto dist3 = Fwg::getPositionDistance(
                  gameProvinces[supplyHubs[distance2]]->baseProvince->position,
                  gameProvinces[supplyHubs[distance]]->baseProvince->position,
                  width);
              if (dist3 < tempDistance) {
                sourceNodeID = gameProvinces[supplyHubs[distance2]]->ID;
                tempDistance = dist3;
              }
            }
            supplyNodeConnections.back()[0] = sourceNodeID;
          }
        } else {
          // NOT at the start of the search, therefore sourceNodeID must be
          // the last element of passThroughStates
          sourceNodeID = passthroughProvinceIDs.back();
        }
        // break if this is another landmass. We can't reach it anyway
        if (gameProvinces[sourceNodeID]->baseProvince->landMassID !=
            gameProvinces[destNodeID]->baseProvince->landMassID)
          break;
        ;
        // the origins position
        auto sourceNodePosition =
            gameProvinces[sourceNodeID]->baseProvince->position;
        // save the distance in a temp variable
        double tempMinDistance = width;
        auto closestID = INT_MAX;
        // now check every sourceNode neighbour for distance to
        // destinationNode
        for (auto &neighbourGProvince :
             gameProvinces[sourceNodeID]->neighbours) {
          // check if this belongs to us and is an eligible province
          if (gProvIDs.find(neighbourGProvince.ID) == gProvIDs.end() ||
              neighbourGProvince.baseProvince->isLake ||
              neighbourGProvince.baseProvince->sea)
            continue;
          bool cont = false;
          for (auto passThroughID : passthroughProvinceIDs) {
            if (passThroughID == neighbourGProvince.ID)
              cont = true;
          }
          if (cont)
            continue;
          // the distance to the sources neighbours
          auto nodeDistance = Fwg::getPositionDistance(
              gameProvinces[destNodeID]->baseProvince->position,
              neighbourGProvince.baseProvince->position, width);
          if (nodeDistance < tempMinDistance) {
            tempMinDistance = nodeDistance;
            closestID = neighbourGProvince.ID;
          }
        }
        if (closestID != INT_MAX) {
          // we found the next best state to go through in this direction
          passthroughProvinceIDs.push_back(closestID);
          // now save source
          sourceNodeID = passthroughProvinceIDs.back();
        }
        // if we can't end this rail line, wrap up. Rails shouldn't be
        // longer than 200 provinces anyway
        else if (attempts == 200) {
          // clean it up: if we can't reach our target, the railway must be
          // cleared
          supplyNodeConnections.back().clear();
          passthroughProvinceIDs.clear();
          break;
        } else
          break;
      }
      // are we done? If no, find the next state, but the source is now the
      // currently chosen neighbour
      while (passthroughProvinceIDs.back() != supplyHubs[distance] &&
             attempts < 200);
      // we are done, as we have reached the destination node
      for (auto &passState : passthroughProvinceIDs) {
        supplyNodeConnections.back().push_back(passState);
      }
    }

    for (auto &pix : capitalProvince->baseProvince->pixels) {
      logistics.setColourAtIndex(pix, {255, 255, 0});
    }
    for (auto &supplyHubProvince : supplyHubProvinces) {
      for (auto &pix : supplyHubProvince.second->baseProvince->pixels) {
        logistics.setColourAtIndex(pix, {0, 255, 0});
      }
    }
  }
  for (auto &connection : supplyNodeConnections) {
    for (int i = 0; i < connection.size(); i++) {
      // check if we accidentally added a sea province or lake province to the
      // connection
      if (gameProvinces[connection[i]]->baseProvince->sea ||
          gameProvinces[connection[i]]->baseProvince->isLake) {
        Fwg::Utils::Logging::logLine("Error: Skipping an invalid connection "
                                     "due to sea or lake province "
                                     "in it in logistics");
        connection.erase(connection.begin() + i);
        i--;
        continue;
      }
      for (auto pix : gameProvinces[connection[i]]->baseProvince->pixels) {
        // don't overwrite capitals and supply nodes
        if (logistics[pix] == Colour{255, 255, 0} ||
            logistics[pix] == Colour{0, 255, 0})
          continue;
        logistics.setColourAtIndex(pix, {255, 255, 255});
      }
    }
  }
  Bmp::save(logistics, "Maps//logistics.bmp");
}

void Generator::evaluateCountries() {
  Fwg::Utils::Logging::logLine("HOI4: Evaluating Country Strength");
  countryImportanceScores.clear();
  double maxScore = 0.0;
  for (auto &c : hoi4Countries) {
    c.second.capitalRegionID = 0;
    auto totalIndustry = 0.0;
    auto totalPop = 0.0;
    for (auto &ownedRegion : c.second.hoi4Regions) {
      auto regionIndustry = ownedRegion->civilianFactories +
                            ownedRegion->dockyards + ownedRegion->armsFactories;
      // always make the most important location the capital
      c.second.selectCapital();

      totalIndustry += regionIndustry;
      totalPop += (int)ownedRegion->totalPopulation;
    }
    countryImportanceScores[(int)(totalIndustry + totalPop / 1'000'000.0)]
        .push_back(c.first);
    c.second.importanceScore = totalIndustry + totalPop / 1'000'000.0;
    if (c.second.importanceScore > maxScore) {
      maxScore = c.second.importanceScore;
    }
    // global
    totalWorldIndustry += (int)totalIndustry;
  }

  int totalDeployedCountries = numCountries - countryImportanceScores.size()
                                   ? (int)countryImportanceScores[0].size()
                                   : 0;
  int numMajorPowers = totalDeployedCountries / 10;
  int numRegionalPowers = totalDeployedCountries / 3;
  int numWeakStates =
      totalDeployedCountries - numMajorPowers - numRegionalPowers;
  for (const auto &scores : countryImportanceScores) {
    for (const auto &entry : scores.second) {
      if (hoi4Countries.at(entry).importanceScore > 0.0) {
        hoi4Countries.at(entry).relativeScore = (double)scores.first / maxScore;
        if (numWeakStates > weakPowers.size()) {
          weakPowers.insert(entry);
          hoi4Countries.at(entry).rank = "weak";
        } else if (numRegionalPowers > regionalPowers.size()) {
          regionalPowers.insert(entry);
          hoi4Countries.at(entry).rank = "regional";
        } else {
          majorPowers.insert(entry);
          hoi4Countries.at(entry).rank = "major";
        }
      }
    }
  }
}

void Generator::generateCountryUnits() {
  Fwg::Utils::Logging::logLine("HOI4: Generating Country Unit Files");
  // read in different compositions
  auto unitTemplateFile = Parsing::readFile(
      Fwg::Cfg::Values().resourcePath + "hoi4//history//divisionTemplates.txt");
  // now tokenize by : character to get single
  auto unitTemplates = Fwg::Parsing::getTokens(unitTemplateFile, ':');
  for (auto &c : hoi4Countries) {
    // determine army doctrine
    // defensive vs offensive
    // infantry/milita, infantry+support, mechanized+armored, artillery
    // bully factor? Getting bullied? Infantry+artillery in defensive
    // doctrine bully? Mechanized+armored major nation? more mechanized
    // share
    auto majorFactor = c.second.relativeScore;
    auto bullyFactor = 0.05 * c.second.bully / 5.0;
    if (c.second.rank == "major") {
      bullyFactor += 0.5;
    } else if (c.second.rank == "regional") {
    }
    // army focus:
    // simply give templates if we qualify for them
    if (majorFactor > 0.5 && bullyFactor > 0.25) {
      // choose one of the mechanised doctrines
      if (RandNum::getRandom(2))
        c.second.doctrines.push_back(Hoi4Country::doctrineType::blitz);
      else
        c.second.doctrines.push_back(Hoi4Country::doctrineType::armored);
    }
    if (bullyFactor < 0.25) {
      // will likely get bullied, add defensive doctrines
      c.second.doctrines.push_back(Hoi4Country::doctrineType::defensive);
    }
    // give all stronger powers infantry with support divisions
    if (majorFactor >= 0.2) {
      // any relatively large power has support divisions
      c.second.doctrines.push_back(Hoi4Country::doctrineType::infantry);
      c.second.doctrines.push_back(Hoi4Country::doctrineType::artillery);
      // any relatively large power has support divisions
      c.second.doctrines.push_back(Hoi4Country::doctrineType::support);
    }
    // give all weaker powers infantry without support
    if (majorFactor < 0.2) {
      c.second.doctrines.push_back(Hoi4Country::doctrineType::milita);
      c.second.doctrines.push_back(Hoi4Country::doctrineType::mass);
    }

    // now evaluate each template and add it if all requirements are
    // fulfilled
    for (int i = 0; i < unitTemplates.size(); i++) {
      auto requirements = Parsing::Scenario::getBracketBlockContent(
          unitTemplates[i], "requirements");
      auto requirementTokens = Fwg::Parsing::getTokens(requirements, ';');
      if (unitFulfillsRequirements(requirementTokens, c.second)) {
        // get the ID and save it for used divison templates
        c.second.units.push_back(i);
      }
    }
    // now compose the army from the templates
    std::map<int, int> unitCount;
    c.second.unitCount.resize(100);
    auto totalUnits = c.second.importanceScore / 5;
    while (totalUnits-- > 0) {
      // now randomly add units
      auto unit = Fwg::Utils::selectRandom(c.second.units);
      c.second.unitCount[unit]++;
    }
  }
}

void Generator::generateFocusTrees() {
  Hoi4::FocusGen::evaluateCountryGoals(this->hoi4Countries, this->gameRegions);
}

void Generator::printStatistics() {
  Fwg::Utils::Logging::logLine(
      "Total Industry: ", militaryIndustry + civilianIndustry + navalIndustry);
  Fwg::Utils::Logging::logLine("Military Industry: ", militaryIndustry);
  Fwg::Utils::Logging::logLine("Civilian Industry: ", civilianIndustry);
  Fwg::Utils::Logging::logLine("Naval Industry: ", navalIndustry);
  for (auto &res : totalResources) {
    Fwg::Utils::Logging::logLine(res.first, " ", res.second);
  }

  Fwg::Utils::Logging::logLine("World Population: ", worldPop);

  for (auto &scores : countryImportanceScores) {
    for (auto &entry : scores.second) {
      Fwg::Utils::Logging::logLine("Strength: ", scores.first, " ",
                                   hoi4Countries.at(entry).fullName, " ",
                                   hoi4Countries.at(entry).rulingParty, "");
    }
  }
}

void Generator::loadStates() {}

void Generator::distributeVictoryPoints() {
  double baseVPs = 10000;
  double assignedVPs = 0;
  for (auto country : hoi4Countries) {
    auto language = country.second.getPrimaryCulture()->language;
    for (auto &region : country.second.hoi4Regions) {
      if (region->sea || region->lake)
        continue;
      region->totalVictoryPoints =
          std::max<int>(region->relativeImportance * baseVPs, 1);
      std::map<int, double> provinceImportance;
      // also a map of province to std::vector locations
      std::map<int, std::vector<std::shared_ptr<Fwg::Civilization::Location>>>
          provinceLocations;

      double totalImportance = 0;
      for (auto &location : region->locations) {
        // ignore waterports
        if (location->type == Fwg::Civilization::LocationType::WaterPort)
          continue;
        provinceImportance[location->provinceID] += location->importance;
        provinceLocations[location->provinceID].push_back(location);
        totalImportance += location->importance;
      }
      // now distribute the victory points according to province importance
      for (auto &province : provinceImportance) {
        auto vps = (int)(province.second / totalImportance *
                         region->totalVictoryPoints);
        VictoryPoint vp{vps};
        // find the most significant location in this province, with a custom
        // comparator using the location importance
        auto mostImportantLocation =
            std::max_element(provinceLocations[province.first].begin(),
                             provinceLocations[province.first].end(),
                             [](const auto &l, const auto &r) {
                               return l->importance < r->importance;
                             });
        vp.position = (*mostImportantLocation)->position;
        vp.name = Fwg::Utils::selectRandom(language->cityNames);
        if ((int)vps > 0) {
          region->victoryPointsMap[province.first] = vp;
          assignedVPs += region->victoryPointsMap[province.first].amount;
        }
      }
    }
  }
}

void Generator::generateUrbanisation() {
  for (auto &region : hoi4States) {
    for (auto &location : region->locations) {
      if (location->type == Fwg::Civilization::LocationType::City ||
          location->type == Fwg::Civilization::LocationType::Port) {
        for (auto &pix : location->pixels) {
          this->civLayer.urbanisation[pix] = 255;
        }
      }
    }
  }
}

bool Generator::unitFulfillsRequirements(
    std::vector<std::string> unitRequirements, Hoi4Country &country) {
  // now check if the country fulfills the target requirements
  for (auto &requirement : unitRequirements) {
    // need to check rank, first get the desired value
    auto value = Parsing::Scenario::getBracketBlockContent(requirement, "rank");
    if (value != "") {
      if (value.find("any") == std::string::npos)
        continue; // fine, may target any ideology
      if (value.find(country.rank) == std::string::npos)
        return false; // targets rank is not right
    }
  }
  for (auto &requirement : unitRequirements) {
    // need to check rank, first get the desired value
    auto value =
        Parsing::Scenario::getBracketBlockContent(requirement, "doctrine");
    if (value != "") {
      if (value.find("any") != std::string::npos)
        continue; // fine, may target any ideology
      // now split by +
      auto requiredDoctrines = Parsing::getTokens(value, '+');
      // for every required doctrine string
      for (const auto &requiredDoctrine : requiredDoctrines) {
        // check if country has that doctrine
        bool found = false;
        for (const auto doctrine : country.doctrines) {
          // map doctrine ID to a string and compare
          if (requiredDoctrine.find(doctrineMap.at((int)doctrine))) {
            found = true;
          }
        }
        // return false if we didn't find this doctrine
        if (!found)
          return false;
      }
    }
  }
  return true;
}
bool Generator::loadRivers(Fwg::Cfg &config, Fwg::Gfx::Bitmap &riverInput) {

  // replace a few colours by the colours understood by FWG
  std::map<Fwg::Gfx::Colour, Fwg::Gfx::Colour> colourMapping{
      {{0, 255, 0}, config.colours.at("riverStart")},
      {{255, 0, 0}, config.colours.at("riverEnd")},
      {{255, 252, 0}, config.colours.at("riverStartTributary")},
      {{0, 225, 255}, config.colours.at("river")},
      {{0, 200, 255}, config.colours.at("river")},
      {{0, 150, 255}, config.colours.at("river")},
      {{0, 100, 255}, config.colours.at("river")},
      {{0, 0, 255}, config.colours.at("river")},
      {{0, 0, 225}, config.colours.at("river")},
      {{0, 0, 200}, config.colours.at("river")},
      {{0, 0, 150}, config.colours.at("river")},
      {{0, 0, 100}, config.colours.at("river")},
      {{0, 85, 0}, config.colours.at("riverStart")},
      {{0, 125, 0}, config.colours.at("riverStart")},
      {{0, 158, 0}, config.colours.at("riverStart")},
      {{24, 206, 0}, config.colours.at("riverStart")}};
  // now replace the colours
  for (auto &pix : riverInput.imageData) {
    if (colourMapping.find(pix) != colourMapping.end()) {
      pix = colourMapping.at(pix);
    }
  }

  // Call the base class method from FastWorldGenerator, to load the now mapped
  // river input
  return FastWorldGenerator::loadRivers(config, riverInput);
}
} // namespace Scenario::Hoi4