#include "Hoi4ScenarioGenerator.h"

Hoi4ScenarioGenerator::Hoi4ScenarioGenerator()
    : random{Data::getInstance().random2} {}

Hoi4ScenarioGenerator::~Hoi4ScenarioGenerator() {}

void Hoi4ScenarioGenerator::generateStateResources(ScenarioGenerator &scenGen) {
  Logger::logLine("HOI4: Digging for resources");
  for (auto &c : scenGen.countryMap) {
    for (auto &gameRegion : c.second.ownedRegions) {
      for (auto &resource : resources) {
        auto chance = resource.second[2];
        if (random() % 100 < chance * 100.0) {
          // calc total of this resource
          auto totalOfResource = resource.second[1] * resource.second[0];
          // more per selected state if the chance is lower
          double averagePerState =
              (totalOfResource / (double)landStates) * (1.0 / chance);
          // range 1 to (2 times average - 1)
          double value =
              1.0 + (random() % (int)ceil((2.0 * averagePerState)) - 1.0);
          // increase by industry factor
          value *= industryFactor;
          gameRegion.attributeDoubles[resource.first] = value;
          // track amount of deployed resources
          if (resource.first == "aluminium")
            totalAluminium += (int)value;
          else if (resource.first == "chromium")
            totalChromium += (int)value;
          else if (resource.first == "rubber")
            totalRubber += (int)value;
          else if (resource.first == "oil")
            totalOil += (int)value;
          else if (resource.first == "steel")
            totalSteel += (int)value;
          else if (resource.first == "tungsten")
            totalTungsten += (int)value;
        }
      }
    }
  }
}

void Hoi4ScenarioGenerator::generateStateSpecifics(ScenarioGenerator &scenGen) {
  Logger::logLine("HOI4: Planning the economy");
  // calculate the world land area
  double worldArea = (double)(Data::getInstance().bitmapSize / 3) *
                     Data::getInstance().landMassPercentage;
  // calculate the target industry amount
  auto targetWorldIndustry =
      (double)Data::getInstance().landMassPercentage * 3648.0 *
      (sqrt(Data::getInstance().bitmapSize) / sqrt((double)(5632 * 2048)));
  for (auto &c : scenGen.countryMap) {
    for (auto &gameRegion : c.second.ownedRegions) {
      // count the number of land states for resource generation
      landStates++;
      int totalPop = 0;
      double totalStateArea = 0;
      double totalDevFactor = 0;
      double totalPopFactor = 0;
      for (const auto &gameProv : gameRegion.gameProvinces) {
        totalDevFactor +=
            gameProv.devFactor / (double)gameRegion.gameProvinces.size();
        totalPopFactor +=
            gameProv.popFactor / (double)gameRegion.gameProvinces.size();
        totalStateArea += gameProv.baseProvince->pixels.size();
      }
      // state level is calculated from population and development
      gameRegion.attributeDoubles["stateCategory"] = std::clamp(
          (int)(totalPopFactor * 5.0 + totalDevFactor * 6.0), 0, 9);
      // one province region? Must be an island state
      if (gameRegion.gameProvinces.size() == 1) {
        gameRegion.attributeDoubles["stateCategory"] = 1;
      }
      gameRegion.attributeDoubles["development"] = totalDevFactor;
      gameRegion.attributeDoubles["population"] =
          totalStateArea * 1250.0 * totalPopFactor * worldPopulationFactor;
      worldPop += (long long)gameRegion.attributeDoubles["population"];
      // count the total coastal provinces of this region
      auto totalCoastal = 0;
      for (auto &gameProv : gameRegion.gameProvinces) {
        if (gameProv.baseProvince->coastal) {
          totalCoastal++;
          // only create a naval base, if a coastal supply hub was determined in
          // this province
          if (gameProv.attributeDoubles["naval_bases"] == 1)
            gameProv.attributeDoubles["naval_bases"] =
                Data::getInstance().getRandomNumber(1, 5);
        } else {
          gameProv.attributeDoubles["naval_bases"] = 0;
        }
      }
      // calculate total industry in this state
      auto stateIndustry =
          (totalStateArea / worldArea) * totalPopFactor * targetWorldIndustry;
      // distribute it to military, civilian and naval factories
      if (totalCoastal > 0) {
        gameRegion.attributeDoubles["dockyards"] =
            std::clamp((int)round(stateIndustry * (0.25)), 0, 4);
        gameRegion.attributeDoubles["civilianFactories"] =
            std::clamp((int)round(stateIndustry * (0.5)), 0, 8);
        gameRegion.attributeDoubles["armsFactories"] =
            std::clamp((int)round(stateIndustry * (0.25)), 0, 4);
        militaryIndustry += (int)gameRegion.attributeDoubles["armsFactories"];
        civilianIndustry +=
            (int)gameRegion.attributeDoubles["civilianFactories"];
        navalIndustry += (int)gameRegion.attributeDoubles["dockyards"];
      } else {
        gameRegion.attributeDoubles["civilianFactories"] =
            std::clamp((int)round(stateIndustry * (0.6)), 0, 8);
        gameRegion.attributeDoubles["armsFactories"] =
            std::clamp((int)round(stateIndustry * (0.4)), 0, 4);
        gameRegion.attributeDoubles["dockyards"] = 0;
        militaryIndustry += (int)gameRegion.attributeDoubles["armsFactories"];
        civilianIndustry +=
            (int)gameRegion.attributeDoubles["civilianFactories"];
      }
    }
  }
}

void Hoi4ScenarioGenerator::generateCountrySpecifics(
    ScenarioGenerator &scenGen, std::map<std::string, Country> &countries) {
  Logger::logLine("HOI4: Choosing uniforms and electing Tyrants");
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
  std::vector<std::string> gfxCultures{
      "western_european", "eastern_european", "middle_eastern", "african",
      "southamerican",    "commonwealth",     "asian"};
  std::vector<std::string> ideologies{"fascism", "democratic", "communism",
                                      "neutrality"};
  for (auto &c : countries) {
    // select a random country ideology
    c.second.attributeStrings["gfxCulture"] =
        *UtilLib::select_random(gfxCultures);
    std::vector<double> popularities{};
    double totalPop = 0;
    for (int i = 0; i < 4; i++) {
      popularities.push_back(Data::getInstance().getRandomNumber(1, 100));
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
      c.second.attributeDoubles[ideologies[i]] = (int)popularities[i] + offset;
    }
    // assign a ruling party
    c.second.attributeStrings["rulingParty"] =
        ideologies[Data::getInstance().getRandomNumber(0,
                                                       (int)ideologies.size())];
    // allow or forbid elections
    if (c.second.attributeStrings["rulingParty"] == "democratic")
      c.second.attributeDoubles["allowElections"] = 1;
    else if (c.second.attributeStrings["rulingParty"] == "neutrality")
      c.second.attributeDoubles["allowElections"] =
          Data::getInstance().getRandomNumber(0, 1);
    else
      c.second.attributeDoubles["allowElections"] = 0;
    // now get the full name of the country
    c.second.attributeStrings["fullName"] = NameGenerator::modifyWithIdeology(
        c.second.attributeStrings["rulingParty"], c.second.name,
        c.second.adjective);
  }
}

void Hoi4ScenarioGenerator::generateStrategicRegions(
    ScenarioGenerator &scenGen) {
  Logger::logLine("HOI4: Dividing world into strategic regions");
  for (auto &region : scenGen.gameRegions) {
    if (region.attributeDoubles["stratID"] == 0.0) {
      strategicRegion sR;
      // std::set<int>stratRegion;
      sR.gameRegionIDs.insert(region.ID);
      region.attributeDoubles["stratID"] = 1.0;
      for (auto &neighbour : region.neighbours) {
        // should be equal in sea/land
        if (scenGen.gameRegions[neighbour].sea == region.sea &&
            scenGen.gameRegions[neighbour].attributeDoubles["stratID"] == 0.0) {
          sR.gameRegionIDs.insert(neighbour);
          scenGen.gameRegions[neighbour].attributeDoubles["stratID"] = 1.0;
        }
      }
      sR.name = NameGenerator::generateName();
      strategicRegions.push_back(sR);
    }
  }
  Bitmap stratRegionBMP(Data::getInstance().width, Data::getInstance().height,
                        24);
  for (auto &strat : strategicRegions) {
    Colour c{random() % 255, random() % 255, random() % 255};
    for (auto &reg : strat.gameRegionIDs) {
      c.setBlue(scenGen.gameRegions[reg].sea ? 255 : 0);
      for (auto &prov : scenGen.gameRegions[reg].gameProvinces) {
        for (auto &pix : prov.baseProvince->pixels) {
          stratRegionBMP.setColourAtIndex(pix, c);
        }
      }
    }
  }
  Bitmap::bufferBitmap("strat", stratRegionBMP);
  Bitmap::SaveBMPToFile(stratRegionBMP, "Maps\\stratRegions.bmp");
}

void Hoi4ScenarioGenerator::generateWeather(ScenarioGenerator &scenGen) {
  for (auto &strat : strategicRegions) {
    for (auto &reg : strat.gameRegionIDs) {
      for (auto i = 0; i < 12; i++) {
        double averageTemperature = 0.0;
        double averageDeviation = 0.0;
        double averagePrecipitation = 0.0;
        for (auto &prov : scenGen.gameRegions[reg].gameProvinces) {
          averageDeviation += prov.baseProvince->weatherMonths[i][0];
          averageTemperature += prov.baseProvince->weatherMonths[i][1];
          averagePrecipitation += prov.baseProvince->weatherMonths[i][2];
        }
        double divisor = (int)scenGen.gameRegions[reg].gameProvinces.size();
        averageDeviation /= divisor;
        averageTemperature /= divisor;
        averagePrecipitation /= divisor;
        // now save monthly data, 0, 1, 2
        strat.weatherMonths.push_back(
            {averageDeviation, averageTemperature, averagePrecipitation});
        // temperature low, 3
        strat.weatherMonths[i].push_back(
            Data::getInstance().baseTemperature +
            averageTemperature * Data::getInstance().temperatureRange);
        // tempHigh, 4
        strat.weatherMonths[i].push_back(
            Data::getInstance().baseTemperature +
            averageTemperature * Data::getInstance().temperatureRange +
            averageDeviation * Data::getInstance().deviationFactor);
        // light_rain chance: cold and humid -> high, 5
        strat.weatherMonths[i].push_back((1.0 - averageTemperature) *
                                         averagePrecipitation);
        // heavy rain chance: warm and humid -> high, 6
        strat.weatherMonths[i].push_back(averageTemperature *
                                         averagePrecipitation);
        // mud chance, 7
        strat.weatherMonths[i].push_back(
            0.5 * (2 * strat.weatherMonths[i][6] + strat.weatherMonths[i][5]));
        // blizzard chance, 8
        strat.weatherMonths[i].push_back(
            std::clamp(0.2 - averageTemperature, 0.0, 0.2) *
            averagePrecipitation);
        // sandstorm chance, 9
        strat.weatherMonths[i].push_back(
            std::clamp(averageTemperature - 0.8, 0.0, 0.2) *
            std::clamp(0.2 - averagePrecipitation, 0.0, 0.2));
        // snow chance, 10
        strat.weatherMonths[i].push_back(
            std::clamp(0.4 - averageTemperature, 0.0, 0.2) *
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

void Hoi4ScenarioGenerator::generateLogistics(ScenarioGenerator &scenGen) {
  Logger::logLine("HOI4: Building rail networks");
  auto width = Data::getInstance().width;
  Bitmap logistics = Bitmap::findBitmapByKey("countries");
  for (auto &c : scenGen.countryMap) {
    // GameProvince ID, distance
    std::map<double, int> supplyHubs;
    // add capital
    auto capitalPosition =
        scenGen.gameRegions[c.second.capitalRegionID].position;
    auto capitalProvince = UtilLib::select_random(
        scenGen.gameRegions[c.second.capitalRegionID].gameProvinces);
    std::vector<double> distances;
    // region ID, provinceID
    std::map<int, GameProvince> supplyHubProvinces;
    std::map<int, bool> navalBases;
    std::set<int> gProvIDs;
    bool connectedNavalBase = false;
    for (auto &region : c.second.ownedRegions) {
      if (region.attributeDoubles["stateCategory"] > 6 &&
              region.ID != c.second.capitalRegionID
          // if we're nearing the end of our region std::vector, and don't have
          // more than 25% of our regions as supply bases generate supply bases
          // for the last two regions
          || (c.second.ownedRegions.size() > 2 &&
              (region.ID == (c.second.ownedRegions.end() - 2)->ID) &&
              supplyHubProvinces.size() < (c.second.ownedRegions.size() / 4))) {
        // select a random gameprovince of the state

        auto y{*UtilLib::select_random(region.gameProvinces)};
        for (auto &prov : region.gameProvinces) {
          if (prov.baseProvince->coastal) {
            // if this is a coastal region, the supply hub is a naval base as
            // well
            y = prov;
            prov.attributeDoubles["naval_bases"] = 1;
            break;
          }
        }
        // save the province under the provinces ID
        supplyHubProvinces[y.ID] = y;
        navalBases[y.ID] = y.baseProvince->coastal;
        // get the distance between this supply hub and the capital
        auto distance = UtilLib::getDistance(capitalPosition,
                                             y.baseProvince->position, width);
        // save the distance under the province ID
        supplyHubs[distance] = y.ID;
        // save the distance
        distances.push_back(distance); // save distances to ensure ordering
      }
      for (auto &gProv : region.gameProvinces) {
        gProvIDs.insert(gProv.ID);
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
              auto dist3 = UtilLib::getDistance(
                  scenGen.gameProvinces[supplyHubs[distance2]]
                      .baseProvince->position,
                  scenGen.gameProvinces[supplyHubs[distance]]
                      .baseProvince->position,
                  width);
              if (dist3 < tempDistance) {
                sourceNodeID = scenGen.gameProvinces[supplyHubs[distance2]].ID;
                tempDistance = dist3;
              }
            }
            supplyNodeConnections.back()[0] = sourceNodeID;
          }
        } else {
          // NOT at the start of the search, therefore sourceNodeID must be the
          // last element of passThroughStates
          sourceNodeID = passthroughProvinceIDs.back();
        }
        // break if this is another landmass. We can't reach it anyway
        if (scenGen.gameProvinces[sourceNodeID].baseProvince->landMassID !=
            scenGen.gameProvinces[destNodeID].baseProvince->landMassID)
          break;
        ;
        // the origins position
        auto sourceNodePosition =
            scenGen.gameProvinces[sourceNodeID].baseProvince->position;
        // save the distance in a temp variable
        double tempMinDistance = width;
        auto closestID = INT_MAX;
        // now check every sourceNode neighbour for distance to destinationNode
        for (auto &neighbourGProvince :
             scenGen.gameProvinces[sourceNodeID].neighbours) {
          // check if this belongs to us
          if (gProvIDs.find(neighbourGProvince.ID) == gProvIDs.end())
            continue;
          bool cont = false;
          for (auto passThroughID : passthroughProvinceIDs) {
            if (passThroughID == neighbourGProvince.ID)
              cont = true;
          }
          if (cont)
            continue;
          // the distance to the sources neighbours
          auto nodeDistance = UtilLib::getDistance(
              scenGen.gameProvinces[destNodeID].baseProvince->position,
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
          if (passthroughProvinceIDs.back() == supplyHubs[distance]) {
            if (navalBases.at(supplyHubs[distance])) {
              connectedNavalBase = true;
            }
          }
        }
        // if we can't end this rail line, wrap up. Rails shouldn't be longer
        // than 200 provinces anyway
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
      for (auto &pix : supplyHubProvince.second.baseProvince->pixels) {
        logistics.setColourAtIndex(pix, {0, 255, 0});
      }
    }
  }
  for (auto &connection : supplyNodeConnections) {
    for (int i = 0; i < connection.size(); i++) {
      for (auto pix :
           scenGen.gameProvinces[connection[i]].baseProvince->pixels) {
        // don't overwrite capitals and supply nodes
        if (logistics.getColourAtIndex(pix) == Colour{255, 255, 0} ||
            logistics.getColourAtIndex(pix) == Colour{0, 255, 0})
          continue;
        logistics.setColourAtIndex(pix, {255, 255, 255});
      }
    }
  }
  Bitmap::SaveBMPToFile(logistics, "Maps//logistics.bmp");
}

void Hoi4ScenarioGenerator::evaluateCountries(ScenarioGenerator &scenGen) {
  Logger::logLine("HOI4: Evaluating Country Strength");
  double maxScore = 0.0;
  for (auto &c : scenGen.countryMap) {
    auto totalIndustry = 0.0;
    auto totalPop = 0.0;
    auto maxIndustryID = 0;
    auto maxIndustryLevel = 0;
    for (auto &ownedRegion : c.second.ownedRegions) {
      auto regionIndustry = ownedRegion.attributeDoubles["civilianFactories"] +
                            ownedRegion.attributeDoubles["dockyards"] +
                            ownedRegion.attributeDoubles["armsFactories"];
      // always make the most industrious region the capital
      if (regionIndustry > maxIndustryLevel)
        c.second.capitalRegionID = ownedRegion.ID;
      totalIndustry += regionIndustry;
      totalPop += (int)ownedRegion.attributeDoubles["population"];
    }
    strengthScores[(int)(totalIndustry + totalPop / 1'000'000.0)].push_back(
        c.first);
    c.second.attributeDoubles["strengthScore"] =
        totalIndustry + totalPop / 1'000'000.0;
    if (c.second.attributeDoubles["strengthScore"] > maxScore) {
      maxScore = c.second.attributeDoubles["strengthScore"];
    }
    // global
    totalWorldIndustry += (int)totalIndustry;
  }
  int totalDeployedCountries =
      scenGen.numCountries - (int)strengthScores[0].size();
  int numMajorPowers = totalDeployedCountries / 10;
  int numRegionalPowers = totalDeployedCountries / 3;
  int numWeakStates =
      totalDeployedCountries - numMajorPowers - numRegionalPowers;
  for (const auto &scores : strengthScores) {
    for (const auto &entry : scores.second) {
      if (scenGen.countryMap[entry].attributeDoubles["strengthScore"] > 0.0) {
        scenGen.countryMap[entry].attributeDoubles["relativeScore"] =
            (double)scores.first / maxScore;
        if (numWeakStates > weakPowers.size()) {
          weakPowers.insert(entry);
          scenGen.countryMap[entry].attributeStrings["rank"] = "weak";
        } else if (numRegionalPowers > regionalPowers.size()) {
          regionalPowers.insert(entry);
          scenGen.countryMap[entry].attributeStrings["rank"] = "regional";
        } else {
          majorPowers.insert(entry);
          scenGen.countryMap[entry].attributeStrings["rank"] = "major";
        }
      }
    }
  }
}

void Hoi4ScenarioGenerator::generateCountryUnits(ScenarioGenerator &scenGen) {
  Logger::logLine("HOI4: Generating Country Unit Files");
  // read in different compositions
  auto unitTemplateFile =
      ParserUtils::readFile("resources\\hoi4\\history\\divisionTemplates.txt");
  // now tokenize by : character to get single
  auto unitTemplates = ParserUtils::getTokens(unitTemplateFile, ':');
  for (auto &c : scenGen.countryMap) {
    // determine army doctrine
    // defensive vs offensive
    // infantry/milita, infantry+support, mechanized+armored, artillery
    // bully factor? Getting bullied? Infantry+artillery in defensive doctrine
    // bully? Mechanized+armored
    // major nation? more mechanized share
    auto majorFactor = c.second.attributeDoubles["relativeScore"];
    auto bullyFactor = 0.05 * c.second.attributeDoubles["bully"] / 5.0;
    auto marinesFactor = 0.0;
    auto mountaineersFactor = 0.0;
    if (c.second.attributeStrings["rank"] == "major") {
      bullyFactor += 0.5;
    } else if (c.second.attributeStrings["rank"] == "regional") {
    }
    // army focus:
    // simply give templates if we qualify for them
    if (majorFactor > 0.5 && bullyFactor > 0.25) {
      // choose one of the mechanised doctrines
      if (Data::getInstance().random2() % 2)
        c.second.attributeVectors["doctrines"].push_back(doctrineType::blitz);
      else
        c.second.attributeVectors["doctrines"].push_back(doctrineType::armored);
    }
    if (bullyFactor < 0.25) {
      // will likely get bullied, add defensive doctrines
      c.second.attributeVectors["doctrines"].push_back(doctrineType::defensive);
    }
    // give all stronger powers infantry with support divisions
    if (majorFactor >= 0.2) {
      // any relatively large power has support divisions
      c.second.attributeVectors["doctrines"].push_back(doctrineType::infantry);
      c.second.attributeVectors["doctrines"].push_back(doctrineType::artillery);
      // any relatively large power has support divisions
      c.second.attributeVectors["doctrines"].push_back(doctrineType::support);
    }
    // give all weaker powers infantry without support
    if (majorFactor < 0.2) {
      c.second.attributeVectors["doctrines"].push_back(doctrineType::milita);
      c.second.attributeVectors["doctrines"].push_back(doctrineType::mass);
    }

    // now evaluate each template and add it if all requirements are fulfilled
    for (int i = 0; i < unitTemplates.size(); i++) {
      auto requirements =
          ParserUtils::getBracketBlockContent(unitTemplates[i], "requirements");
      auto requirementTokens = ParserUtils::getTokens(requirements, ';');
      if (unitFulfillsRequirements(requirementTokens, c.second)) {
        // get the ID and save it for used divison templates
        c.second.attributeVectors["units"].push_back(i);
      }
    }
    // now compose the army from the templates
    std::map<int, int> unitCount;
    c.second.attributeVectors["unitCount"].resize(100);
    auto totalUnits = c.second.attributeDoubles["strengthScore"] / 5;
    while (totalUnits-- > 0) {
      // now randomly add units
      auto unit = *UtilLib::select_random(c.second.attributeVectors["units"]);
      c.second.attributeVectors["unitCount"][unit]++;
    }
  }
}

NationalFocus
Hoi4ScenarioGenerator::buildFocus(const std::vector<std::string> chainStep,
                                  const Country &source,
                                  const Country &target) {
  // map the string of the chainstep to the type
  auto type = NationalFocus::typeMapping[chainStep[5]];
  auto dateTokens = ParserUtils::getNumbers(chainStep[8], '-', std::set<int>{});
  NationalFocus nF(type, false, source.tag, target.tag, dateTokens);

  auto predecessors = ParserUtils::getNumbers(
      ParserUtils::getBracketBlockContent(chainStep[2], "predecessor"), ',',
      std::set<int>());
  for (const auto &predecessor : predecessors)
    nF.precedingFoci.push_back(predecessor);

  // now get the "or"...
  auto exclusives = ParserUtils::getNumbers(
      ParserUtils::getBracketBlockContent(chainStep[7], "exclusive"), ',',
      std::set<int>());
  for (const auto &exclusive : exclusives)
    nF.alternativeFoci.push_back(exclusive);
  // and "and" foci
  auto ands = ParserUtils::getNumbers(
      ParserUtils::getBracketBlockContent(chainStep[7], "and"), ',',
      std::set<int>());
  for (const auto &and : ands)
    nF.andFoci.push_back(and);

  // add completion reward keys
  auto available = ParserUtils::getTokens(
      ParserUtils::getBracketBlockContent(chainStep[9], "available"), '+');
  for (const auto &availKey : available) {
    nF.available.push_back(availKey);
  }
  // add completion reward keys
  auto bypasses = ParserUtils::getTokens(
      ParserUtils::getBracketBlockContent(chainStep[10], "bypass"), '+');
  for (const auto &bypassKey : bypasses) {
    nF.bypasses.push_back(bypassKey);
  }
  // add completion reward keys
  auto rewards = ParserUtils::getTokens(
      ParserUtils::getBracketBlockContent(chainStep[11], "completion_reward"),
      '+');
  for (const auto &rewardKey : rewards) {
    nF.completionRewards.push_back(rewardKey);
  }
  return nF;
}
void Hoi4ScenarioGenerator::buildFocusTree(Country &source) {
  // std::array<std::array<int, 100>, 100> occupiedPositions;
  // start left. Chains go down, new chains go right
  int curX = 1;
  int curY = 1;
  int maxX = 1;
  for (auto &focusChain : source.foci) {
    curY = 1;
    std::array<std::set<int>, 100> levels;
    int index = 0;
    int width = 0;
    for (auto &focus : focusChain) {
      // if this focus is already on this level, just continue
      if (levels[index].find(focus.stepID) != levels[index].end())
        continue;
      levels[index].insert(focus.stepID);
      for (auto stepID : focus.alternativeFoci) {
        levels[index].insert(stepID);
      }
      for (auto stepID : focus.andFoci) {
        levels[index].insert(stepID);
      }
      // now check for every newly added focus, if that also has and or or foci
      for (auto chainStepID : levels[index]) {
        if (chainStepID < focusChain.size()) {
          for (auto stepID : focusChain[chainStepID].alternativeFoci) {
            levels[index].insert(stepID);
          }
          for (auto stepID : focusChain[chainStepID].andFoci) {
            levels[index].insert(stepID);
          }
        }
      }
      // save the maximum width for later
      if (levels[index].size() > width)
        width = (int)levels[index].size();
      // now increment level, we have all alternatives or AND
      index++;
    }
    // now that we have every focus assigned to a level, we can start setting
    // positions for this chain
    auto baseX = curX;
    for (const auto &level : levels) {
      curX = baseX;
      for (const auto &entry : level) {
        if (entry < focusChain.size())
          focusChain.at(entry).position = {curX += 2, curY};
      }
      if (curX > maxX) {
        maxX = curX;
      }
      curY++;
    }
    // use this for next chain to ensure spacing
    curX = maxX;
  }
}
bool Hoi4ScenarioGenerator::stepFulfillsRequirements(
    std::vector<std::string> stepRequirements,
    const std::vector<std::set<Country>> stepTargets) {
  // first check if the step can be fulfilled at all
  for (auto &stepRequirement : stepRequirements) {
    // need to check if required steps were made, first get the desired value
    const auto value =
        ParserUtils::getBracketBlockContent(stepRequirement, "requires");
    const auto requiredPredecessors = ParserUtils::getTokens(value, ',');
    bool hasRequiredPredecessor = requiredPredecessors.size() ? false : true;
    for (const auto &predecessor : requiredPredecessors)
      if (predecessor != "") {
        if (stepTargets[stoi(value)].size())
          hasRequiredPredecessor = true;
      }
    if (!hasRequiredPredecessor)
      return false; // missing predecessor
  }
  return true;
}
/* checks all requirements for a national focus. Returns false if any
 * requirement isn't fulfilled, else returns true*/
bool Hoi4ScenarioGenerator::targetFulfillsRequirements(
    std::vector<std::string> targetRequirements, Country &source,
    Country &target, const std::vector<std::set<std::string>> levelTargets,
    const int level) {
  // now check if the country fulfills the target requirements
  for (auto &targetRequirement : targetRequirements) {
    // need to check rank, first get the desired value
    auto value = ParserUtils::getBracketBlockContent(targetRequirement, "rank");
    if (value != "") {
      if (target.attributeStrings["rank"] != value)
        return false; // targets rank is not right
    }
    value = ParserUtils::getBracketBlockContent(targetRequirement, "ideology");
    if (value != "") {
      if (value == "any")
        continue; // fine, may target any ideology
      if (value == "same")
        if (target.attributeStrings["rulingParty"] !=
            source.attributeStrings["rulingParty"])
          return false;
      if (value == "not")
        if (target.attributeStrings["rulingParty"] ==
            source.attributeStrings["rulingParty"])
          return false;
    }
    value = ParserUtils::getBracketBlockContent(targetRequirement, "location");
    if (value != "") {
      if (value == "any")
        continue; // fine, may target any ideology
      if (value == "neighbour") {
        if (source.neighbours.find(target.tag) == source.neighbours.end())
          return false;
      }
      // to do: near, distant, any
    }
    value = ParserUtils::getBracketBlockContent(targetRequirement, "target");
    if (value != "") {
      if (value == "notlevel") {
        // don't consider this country if already used on same level
        if (levelTargets[level].find(target.tag) != levelTargets[level].end())
          return false;
      }
      if (value == "level") {
        // don't consider this country if NOT used on same level
        if (levelTargets[level].size() &&
            levelTargets[level].find(target.tag) == levelTargets[level].end())
          return false;
      }
      if (value == "notchain") {
        for (int i = 0; i < levelTargets.size(); i++) {
          // don't consider this country if already used in same chain
          if (levelTargets[i].find(target.tag) != levelTargets[level].end())
            return false;
        }
      }
      if (value == "chain") {
        bool foundUse = false;
        for (int i = 0; i < levelTargets.size(); i++) {
          // don't consider this country if NOT used in same chain
          if (levelTargets[i].find(target.tag) == levelTargets[level].end())
            foundUse = true;
        }
        if (!foundUse)
          return false;
      }
    }
  }
  return true;
}

void Hoi4ScenarioGenerator::evaluateCountryGoals(ScenarioGenerator &scenGen) {
  Logger::logLine("HOI4: Generating Country Goals");
  std::vector<int> defDate{1, 1, 1936};
  std::vector<std::vector<std::vector<std::string>>> chains;

  chains.push_back(ParserUtils::getLinesByID(
      "resources\\hoi4\\ai\\national_focus\\chains\\major_chains.txt"));
  chains.push_back(ParserUtils::getLinesByID(
      "resources\\hoi4\\ai\\national_focus\\chains\\regional_chains.txt"));
  chains.push_back(ParserUtils::getLinesByID(
      "resources\\hoi4\\ai\\national_focus\\chains\\army_chains.txt"));
  auto typeCounter = 0;
  for (auto &sourceCountry : scenGen.countryMap) {
    const auto &sourceS =
        scenGen.countryMap[sourceCountry.first].attributeStrings;
    auto &sourceD = scenGen.countryMap[sourceCountry.first].attributeDoubles;
    sourceCountry.second.attributeDoubles["bully"] = 0;
    sourceCountry.second.attributeDoubles["defensive"] = 0;
    for (const auto &chainType : chains) {
      for (const auto &chain : chainType) {
        // evaluate whole chain (chain defined by ID)
        if (!chain.size())
          continue;
        // we need to save options for every chain step
        std::vector<std::set<Country>> stepTargets;
        std::vector<std::set<std::string>> levelTargets(chain.size());
        int chainID = 0;
        for (const auto &chainFocus : chain) {
          Logger::logLineLevel(9, chainFocus);
          // evaluate every single focus of that chain
          const auto chainTokens = ParserUtils::getTokens(chainFocus, ';');
          const int chainStep = stoi(chainTokens[1]);
          chainID = stoi(chainTokens[0]);
          const int level = stoi(chainTokens[12]);
          if (sourceS.at("rulingParty") == chainTokens[4] ||
              chainTokens[4] == "any") {
            stepTargets.resize(stepTargets.size() + 1);
            auto stepRequirements = ParserUtils::getTokens(chainTokens[2], '+');
            if (stepFulfillsRequirements(stepRequirements, stepTargets)) {
              // source triggers this focus
              // split requirements
              auto targetRequirements =
                  ParserUtils::getTokens(chainTokens[6], '+');
              // if there are no target requirements, only the country itself is
              // a target
              if (!targetRequirements.size())
                stepTargets[chainStep].insert(sourceCountry.second);
              else {
                for (auto &destCountry : scenGen.countryMap) {
                  // now check every country if it fulfills the target
                  // requirements
                  if (targetFulfillsRequirements(
                          targetRequirements,
                          scenGen.countryMap[sourceCountry.first],
                          destCountry.second, levelTargets, level)) {
                    stepTargets[chainStep].insert(destCountry.second);
                    // save that we targeted this country on this level already.
                    // Next steps on same level should not consider this tag
                    // anymore
                    levelTargets[level].insert(destCountry.first);
                  }
                }
              }
            }
          }
        }
        // now build the chain from the options
        // for every step of the chain, choose a target
        if (stepTargets.size()) {
          Logger::logLineLevel(5, "Building focus");
          std::map<int, NationalFocus> fulfilledSteps;
          int stepIndex = -1;
          std::vector<NationalFocus> chainFoci;

          for (auto &targets : stepTargets) {
            stepIndex++;
            if (!targets.size())
              continue;
            // select random target
            const auto &target = *UtilLib::select_random(targets);
            // however
            // if (targets.find(scenGen.countryMap.at(chainFoci.back().destTag))
            // != targets.end()) 	target =
            //scenGen.countryMap.at(chainFoci.back().destTag);
            auto focus{buildFocus(ParserUtils::getTokens(chain[stepIndex], ';'),
                                  scenGen.countryMap.at(sourceCountry.first),
                                  target)};
            focus.stepID = stepIndex;
            focus.chainID = chainID;
            Logger::logLineLevel(1, focus);
            if (focus.fType == focus.attack) {
              // country aims to bully
              sourceCountry.second.attributeDoubles["bully"]++;
            }
            chainFoci.push_back(focus);
          }
          sourceCountry.second.foci.push_back(chainFoci);
        }
      }
    }
    // now build a tree out of the focus chains
    buildFocusTree(sourceCountry.second);
  }
}

void Hoi4ScenarioGenerator::printStatistics(ScenarioGenerator &scenGen) {
  Logger::logLine("Total Industry: ", totalWorldIndustry, "");
  Logger::logLine("Military Industry: ", militaryIndustry, "");
  Logger::logLine("Civilian Industry: ", civilianIndustry, "");
  Logger::logLine("Naval Industry: ", navalIndustry, "");
  Logger::logLine("Total Aluminium: ", totalAluminium, "");
  Logger::logLine("Total Chromium: ", totalChromium, "");
  Logger::logLine("Total Rubber: ", totalRubber, "");
  Logger::logLine("Total Oil: ", totalOil, "");
  Logger::logLine("Total Steel: ", totalSteel, "");
  Logger::logLine("Total Tungsten: ", totalTungsten, "");
  Logger::logLine("World Population: ", worldPop, "");

  for (auto &scores : strengthScores) {
    for (auto &entry : scores.second) {
      Logger::logLine(
          "Strength: ", scores.first, " ",
          scenGen.countryMap.at(entry).attributeStrings.at("fullName"), " ",
          scenGen.countryMap.at(entry).attributeStrings.at("rulingParty"), "");
    }
  }
}

bool Hoi4ScenarioGenerator::unitFulfillsRequirements(
    std::vector<std::string> unitRequirements, Country &country) {
  // now check if the country fulfills the target requirements
  for (auto &requirement : unitRequirements) {
    // need to check rank, first get the desired value
    auto value = ParserUtils::getBracketBlockContent(requirement, "rank");
    if (value != "") {
      if (value.find("any") == std::string::npos)
        continue; // fine, may target any ideology
      if (value.find(country.attributeStrings["rank"]) == std::string::npos)
        return false; // targets rank is not right
    }
  }
  for (auto &requirement : unitRequirements) {
    // need to check rank, first get the desired value
    auto value = ParserUtils::getBracketBlockContent(requirement, "doctrine");
    if (value != "") {
      if (value.find("any") != std::string::npos)
        continue; // fine, may target any ideology
      // now split by +
      auto requiredDoctrines = ParserUtils::getTokens(value, '+');
      // for every required doctrine string
      for (const auto &requiredDoctrine : requiredDoctrines) {
        // check if country has that doctrine
        bool found = false;
        for (const auto doctrine : country.attributeVectors["doctrines"]) {
          // map doctrine ID to a string and compare
          if (requiredDoctrine.find(doctrineMap.at(doctrine))) {
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
