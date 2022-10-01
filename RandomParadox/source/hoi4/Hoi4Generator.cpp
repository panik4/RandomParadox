#include "hoi4/Hoi4Generator.h"
using namespace Fwg;
using namespace Fwg::Gfx;
namespace Scenario::Hoi4 {
Generator::Generator() {}
Generator::Generator(FastWorldGenerator &fwg) : Scenario::Generator(fwg) {
  nData = NameGeneration::prepare("resources\\names");
}

Generator::~Generator() {}

void Generator::generateStateResources() {
  Utils::Logging::logLine("HOI4: Digging for resources");
  for (auto &c : hoi4Countries) {
    for (auto &hoi4Region : c.second.hoi4Regions) {
      for (const auto &resource : resources) {
        auto chance = resource.second[2];
        if (RandNum::getRandom(100) < chance * 100.0) {
          // calc total of this resource
          auto totalOfResource = resource.second[1] * resource.second[0];
          // more per selected state if the chance is lower
          double averagePerState =
              (totalOfResource / (double)landStates) * (1.0 / chance);
          // range 1 to (2 times average - 1)
          double value =
              1.0 +
              (RandNum::getRandom((int)ceil((2.0 * averagePerState)) - 1.0));
          // increase by industry factor
          value *= industryFactor;
          value *= sizeFactor;
          hoi4Region->resources[resource.first] = (int)value;
          totalResources[resource.first] += (int)value;
        }
      }
    }
  }
}

void Generator::generateStateSpecifics(const int regionAmount) {
  Utils::Logging::logLine("HOI4: Planning the economy");
  auto &config = Cfg::Values();
  // calculate the target industry amount
  auto targetWorldIndustry = 1248 * sizeFactor * industryFactor;
  Utils::Logging::logLine(config.landPercentage);
  for (auto &c : hoi4Countries) {
    for (auto &hoi4Region : c.second.hoi4Regions) {
      // count the number of land states for resource generation
      landStates++;
      double totalStateArea = 0;
      double totalDevFactor = 0;
      double totalPopFactor = 0;
      for (const auto &gameProv : hoi4Region->gameProvinces) {
        totalDevFactor +=
            gameProv.devFactor / (double)hoi4Region->gameProvinces.size();
        totalPopFactor +=
            gameProv.popFactor / (double)hoi4Region->gameProvinces.size();
        totalStateArea += gameProv.baseProvince->pixels.size();
      }
      // state level is calculated from population and development
      hoi4Region->stateCategory =
          std::clamp((int)(totalPopFactor * 5.0 + totalDevFactor * 6.0), 0, 9);
      // one province region? Must be an island state
      if (hoi4Region->gameProvinces.size() == 1) {
        hoi4Region->stateCategory = 1;
      }
      hoi4Region->development = totalDevFactor;
      hoi4Region->population = totalStateArea * 1250.0 * totalPopFactor *
                              worldPopulationFactor * (1.0 / sizeFactor);
      worldPop += (long long)hoi4Region->population;
      // count the total coastal provinces of this region
      auto totalCoastal = 0;
      for (auto &gameProv : hoi4Region->gameProvinces) {
        if (gameProv.baseProvince->coastal) {
          totalCoastal++;
          // only create a naval base, if a coastal supply hub was determined in
          // this province
          if (gameProv.attributeDoubles["naval_bases"] == 1)
            gameProv.attributeDoubles["naval_bases"] = RandNum::getRandom(1, 5);
        } else {
          gameProv.attributeDoubles["naval_bases"] = 0;
        }
      }
      // calculate total industry in this state
      auto stateIndustry = round(
          totalPopFactor * (targetWorldIndustry / (double)(regionAmount)));
      double dockChance = 0.25;
      double civChance = 0.5;
      // distribute it to military, civilian and naval factories
      if (!totalCoastal) {
        dockChance = 0.0;
        civChance = 0.6;
      }
      while (--stateIndustry >= 0) {
        auto choice = RandNum::getRandom(0.0, 1.0);
        if (choice < dockChance) {
          hoi4Region->dockyards++;
        } else if (Utils::inRange(dockChance, dockChance + civChance, choice)) {
          hoi4Region->civilianFactories++;

        } else {
          hoi4Region->armsFactories++;
        }
      }
      militaryIndustry += (int)hoi4Region->armsFactories;
      civilianIndustry += (int)hoi4Region->civilianFactories;
      navalIndustry += (int)hoi4Region->dockyards;
    }
  }
}

void Generator::generateCountrySpecifics() {
  Utils::Logging::logLine("HOI4: Choosing uniforms and electing Tyrants");
  sizeFactor = sqrt((double)(Cfg::Values().width * Cfg::Values().height) /
                    (double)(5632 * 2048));
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
  for (auto &country : countries) {
    // construct a hoi4country with country from ScenarioGenerator.
    // We want a copy here
    Hoi4Country hC(country.second, gameRegions);
    // save the pointers to states not only in countries
    for (const auto &hoi4State : hC.hoi4Regions) {
      hoi4States.push_back(hoi4State);
    }

    // select a random country ideology
    hC.gfxCulture = Utils::selectRandom(gfxCultures);
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
      hC.parties[i] = (int)popularities[i] + offset;
    }
    // assign a ruling party
    hC.rulingParty = ideologies[RandNum::getRandom(0, (int)ideologies.size())];
    // allow or forbid elections
    if (hC.rulingParty == "democratic")
      hC.allowElections = 1;
    else if (hC.rulingParty == "neutrality")
      hC.allowElections = RandNum::getRandom(0, 1);
    else
      hC.allowElections = 0;
    // now get the full name of the country
    hC.fullName = NameGeneration::modifyWithIdeology(
        hC.rulingParty, country.second.name, country.second.adjective, nData);
    hoi4Countries.insert({hC.tag, hC});
  }
}

void Generator::generateStrategicRegions() {
  Utils::Logging::logLine("HOI4: Dividing world into strategic regions");
  std::set<int> assignedIdeas;
  for (auto &region : gameRegions) {
    if (assignedIdeas.find(region.ID) == assignedIdeas.end()) {
      strategicRegion sR;
      // std::set<int>stratRegion;
      sR.gameRegionIDs.insert(region.ID);
      assignedIdeas.insert(region.ID);
      for (auto &neighbour : region.neighbours) {
        // should be equal in sea/land
        if (neighbour > gameRegions.size())
          continue;
        if (gameRegions[neighbour].sea == region.sea &&
            assignedIdeas.find(neighbour) == assignedIdeas.end()) {
          sR.gameRegionIDs.insert(neighbour);
          assignedIdeas.insert(neighbour);
        }
      }
      sR.name = NameGeneration::generateName(nData);
      strategicRegions.push_back(sR);
    }
  }
  Bitmap stratRegionBMP(Cfg::Values().width, Cfg::Values().height, 24);
  for (auto &strat : strategicRegions) {
    Colour c{static_cast<unsigned char>(RandNum::getRandom(255)),
             static_cast<unsigned char>(RandNum::getRandom(255)),
             static_cast<unsigned char>(RandNum::getRandom(255))};
    for (auto &reg : strat.gameRegionIDs) {
      c.setBlue(gameRegions[reg].sea ? 255 : 0);
      for (auto &prov : gameRegions[reg].gameProvinces) {
        for (auto &pix : prov.baseProvince->pixels) {
          stratRegionBMP.setColourAtIndex(pix, c);
        }
      }
    }
  }
  Bmp::bufferBitmap("strat", stratRegionBMP);
  Bmp::save(stratRegionBMP, "Maps\\stratRegions.bmp");
}

void Generator::generateWeather() {
  for (auto &strat : strategicRegions) {
    for (auto &reg : strat.gameRegionIDs) {
      for (auto i = 0; i < 12; i++) {
        double averageTemperature = 0.0;
        double averageDeviation = 0.0;
        double averagePrecipitation = 0.0;
        for (auto &prov : gameRegions[reg].gameProvinces) {
          averageDeviation += prov.baseProvince->weatherMonths[i][0];
          averageTemperature += prov.baseProvince->weatherMonths[i][1];
          averagePrecipitation += prov.baseProvince->weatherMonths[i][2];
        }
        double divisor = (int)gameRegions[reg].gameProvinces.size();
        averageDeviation /= divisor;
        averageTemperature /= divisor;
        averagePrecipitation /= divisor;
        // now save monthly data, 0, 1, 2
        strat.weatherMonths.push_back(
            {averageDeviation, averageTemperature, averagePrecipitation});
        // temperature low, 3
        strat.weatherMonths[i].push_back(Cfg::Values().baseTemperature +
                                         averageTemperature *
                                             Cfg::Values().temperatureRange);
        // tempHigh, 4
        strat.weatherMonths[i].push_back(
            Cfg::Values().baseTemperature +
            averageTemperature * Cfg::Values().temperatureRange +
            averageDeviation * Cfg::Values().deviationFactor);
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

void Generator::generateLogistics(Bitmap logistics) {
  Utils::Logging::logLine("HOI4: Building rail networks");
  auto width = Cfg::Values().width;
  for (auto &country : hoi4Countries) {
    // GameProvince ID, distance
    std::map<double, int> supplyHubs;
    // add capital
    auto capitalPosition = gameRegions[country.second.capitalRegionID].position;
    auto &capitalProvince = Utils::selectRandom(
        gameRegions[country.second.capitalRegionID].gameProvinces);
    std::vector<double> distances;
    // region ID, provinceID
    std::map<int, GameProvince> supplyHubProvinces;
    std::map<int, bool> navalBases;
    std::set<int> gProvIDs;
    for (auto &region : country.second.hoi4Regions) {
      if ((region->stateCategory > 6 &&
           region->ID != country.second.capitalRegionID)
          // if we're nearing the end of our region std::vector, and don't have
          // more than 25% of our regions as supply bases generate supply bases
          // for the last two regions
          || (country.second.hoi4Regions.size() > 2 &&
           (region->ID == (* (country.second.hoi4Regions.end() - 2))->ID) &&
              supplyHubProvinces.size() <
                  (country.second.hoi4Regions.size() / 4))) {
        // select a random gameprovince of the state

        auto y{Utils::selectRandom(region->gameProvinces)};
        for (auto &prov : region->gameProvinces) {
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
        auto distance = Utils::getDistance(capitalPosition,
                                           y.baseProvince->position, width);
        // save the distance under the province ID
        supplyHubs[distance] = y.ID;
        // save the distance
        distances.push_back(distance); // save distances to ensure ordering
      }
      for (auto &gProv : region->gameProvinces) {
        gProvIDs.insert(gProv.ID);
      }
    }
    std::sort(distances.begin(), distances.end());
    for (const auto distance : distances) {
      std::vector<int> passthroughProvinceIDs;
      int attempts = 0;
      auto sourceNodeID = capitalProvince.ID;
      supplyNodeConnections.push_back({sourceNodeID});
      do {
        attempts++;
        // the region we want to connect to the source
        auto destNodeID = supplyHubs[distance];
        if (sourceNodeID == capitalProvince.ID) {
          // we are at the start of the search
          // distance to capital
          auto tempDistance = distance;
          for (auto distance2 : distances) {
            // only check hubs that were already assigned
            if (distance2 < distance) {
              // distance is the distance between us and the capital
              // now find distance2, the distance between us and the other
              // already assigned supply hubs
              auto dist3 = Utils::getDistance(
                  gameProvinces[supplyHubs[distance2]].baseProvince->position,
                  gameProvinces[supplyHubs[distance]].baseProvince->position,
                  width);
              if (dist3 < tempDistance) {
                sourceNodeID = gameProvinces[supplyHubs[distance2]].ID;
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
        if (gameProvinces[sourceNodeID].baseProvince->landMassID !=
            gameProvinces[destNodeID].baseProvince->landMassID)
          break;
        ;
        // the origins position
        auto sourceNodePosition =
            gameProvinces[sourceNodeID].baseProvince->position;
        // save the distance in a temp variable
        double tempMinDistance = width;
        auto closestID = INT_MAX;
        // now check every sourceNode neighbour for distance to destinationNode
        for (auto &neighbourGProvince :
             gameProvinces[sourceNodeID].neighbours) {
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
          auto nodeDistance = Utils::getDistance(
              gameProvinces[destNodeID].baseProvince->position,
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
    for (auto &pix : capitalProvince.baseProvince->pixels) {
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
      for (auto pix : gameProvinces[connection[i]].baseProvince->pixels) {
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
  Utils::Logging::logLine("HOI4: Evaluating Country Strength");
  double maxScore = 0.0;
  for (auto &c : hoi4Countries) {
    auto totalIndustry = 0.0;
    auto totalPop = 0.0;
    auto maxIndustryLevel = 0;
    for (auto &ownedRegion : c.second.hoi4Regions) {
      auto regionIndustry = ownedRegion->civilianFactories +
                            ownedRegion->dockyards + ownedRegion->armsFactories;
      // always make the most industrious region the capital
      if (regionIndustry > maxIndustryLevel)
        c.second.capitalRegionID = ownedRegion->ID;
      totalIndustry += regionIndustry;
      totalPop += (int)ownedRegion->population;
    }
    strengthScores[(int)(totalIndustry + totalPop / 1'000'000.0)].push_back(
        c.first);
    c.second.strengthScore = totalIndustry + totalPop / 1'000'000.0;
    if (c.second.strengthScore > maxScore) {
      maxScore = c.second.strengthScore;
    }
    // global
    totalWorldIndustry += (int)totalIndustry;
  }
  int totalDeployedCountries = numCountries - (int)strengthScores[0].size();
  int numMajorPowers = totalDeployedCountries / 10;
  int numRegionalPowers = totalDeployedCountries / 3;
  int numWeakStates =
      totalDeployedCountries - numMajorPowers - numRegionalPowers;
  for (const auto &scores : strengthScores) {
    for (const auto &entry : scores.second) {
      if (hoi4Countries[entry].strengthScore > 0.0) {
        hoi4Countries[entry].relativeScore = (double)scores.first / maxScore;
        if (numWeakStates > weakPowers.size()) {
          weakPowers.insert(entry);
          hoi4Countries[entry].rank = "weak";
        } else if (numRegionalPowers > regionalPowers.size()) {
          regionalPowers.insert(entry);
          hoi4Countries[entry].rank = "regional";
        } else {
          majorPowers.insert(entry);
          hoi4Countries[entry].rank = "major";
        }
      }
    }
  }
}

void Generator::generateCountryUnits() {
  Utils::Logging::logLine("HOI4: Generating Country Unit Files");
  // read in different compositions
  auto unitTemplateFile =
      ParserUtils::readFile("resources\\hoi4\\history\\divisionTemplates.txt");
  // now tokenize by : character to get single
  auto unitTemplates = ParserUtils::getTokens(unitTemplateFile, ':');
  for (auto &c : hoi4Countries) {
    // determine army doctrine
    // defensive vs offensive
    // infantry/milita, infantry+support, mechanized+armored, artillery
    // bully factor? Getting bullied? Infantry+artillery in defensive doctrine
    // bully? Mechanized+armored
    // major nation? more mechanized share
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

    // now evaluate each template and add it if all requirements are fulfilled
    for (int i = 0; i < unitTemplates.size(); i++) {
      auto requirements =
          ParserUtils::getBracketBlockContent(unitTemplates[i], "requirements");
      auto requirementTokens = ParserUtils::getTokens(requirements, ';');
      if (unitFulfillsRequirements(requirementTokens, c.second)) {
        // get the ID and save it for used divison templates
        c.second.units.push_back(i);
      }
    }
    // now compose the army from the templates
    std::map<int, int> unitCount;
    c.second.unitCount.resize(100);
    auto totalUnits = c.second.strengthScore / 5;
    while (totalUnits-- > 0) {
      // now randomly add units
      auto unit = Utils::selectRandom(c.second.units);
      c.second.unitCount[unit]++;
    }
  }
}

NationalFocus Generator::buildFocus(const std::vector<std::string> chainStep,
                                    const Hoi4Country &source,
                                    const Hoi4Country &target) {
  // map the string of the chainstep to the type
  auto type = NationalFocus::typeMapping[chainStep[5]];
  auto dateTokens = ParserUtils::getNumbers(chainStep[8], '-', std::set<int>{});
  NationalFocus nF(type, false, source.tag, target.tag, dateTokens);

  auto predecessors = ParserUtils::getNumbers(
      ParserUtils::getBracketBlockContent(chainStep[2], "predecessor"), ',',
      std::set<int>());
  for (const auto &predecessor : predecessors)
    nF.precedingFoci.push_back(predecessor);

  // now get the "xor"...
  auto exclusives = ParserUtils::getNumbers(
      ParserUtils::getBracketBlockContent(chainStep[7], "exclusive"), ',',
      std::set<int>());
  for (const auto &exclusive : exclusives)
    nF.xorFoci.push_back(exclusive);
  // and "and" foci
  auto ands = ParserUtils::getNumbers(
      ParserUtils::getBracketBlockContent(chainStep[7], "and"), ',',
      std::set<int>());
  for (const auto &and : ands)
    nF.andFoci.push_back(and);
  // and "or" foci
  auto ors = ParserUtils::getNumbers(
      ParserUtils::getBracketBlockContent(chainStep[7], "or"), ',',
      std::set<int>());
  for (const auto & or : ors)
    nF.orFoci.push_back(or);
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

void Generator::buildFocusTree(Hoi4Country &source) {
  // std::array<std::array<int, 100>, 100> occupiedPositions;
  // start left. Chains go down, new chains go right
  int curX = 1;
  int curY = 1;
  int maxX = 1;
  if (source.tag == "DIA")
    Utils::Logging::logLine("AA");
  for (auto &focusChain : source.foci) {
    curY = 1;
    std::set<int> fociIDs;
    std::array<std::set<int>, 100> levels;
    int index = 0;
    int width = 0;
    for (auto &focus : focusChain) {
      // if this focus is already on this level, just continue
      if (levels[index].find(focus.stepID) != levels[index].end())
        continue;
      // if this focus is already assgined on SOME level, just continue
      if (fociIDs.find(focus.stepID) != fociIDs.end())
        continue;
      levels[index].insert(focus.stepID);
      fociIDs.insert(focus.stepID);
      for (auto stepID : focus.xorFoci) {
        levels[index].insert(stepID);
        fociIDs.insert(stepID);
      }
      for (auto stepID : focus.andFoci) {
        levels[index].insert(stepID);
        fociIDs.insert(stepID);
      }
      // now check for every newly added focus, if that also has and or or foci
      for (auto chainStepID : levels[index]) {
        if (chainStepID < focusChain.size()) {
          for (auto stepID : focusChain[chainStepID].xorFoci) {
            levels[index].insert(stepID);
            fociIDs.insert(stepID);
          }
          for (auto stepID : focusChain[chainStepID].andFoci) {
            levels[index].insert(stepID);
            fociIDs.insert(stepID);
          }
          for (auto stepID : focusChain[chainStepID].orFoci) {
            levels[index].insert(stepID);
            fociIDs.insert(stepID);
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
    index = 0;
    for (const auto &level : levels) {
      curX = baseX;
      for (const auto &entry : level) {
        focusChain.at(index++).position = {curX += 2, curY};
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

bool Generator::stepFulfillsRequirements(
    const std::string stepRequirements,
    const std::vector<std::set<Hoi4Country>> &stepTargets) {

  const auto predecessors =
      ParserUtils::getBracketBlockContent(stepRequirements, "predecessor");
  const auto v = ParserUtils::getNumbers(
      ParserUtils::getBracketBlockContent(stepRequirements, "skippable"), ',',
      std::set<int>());
  std::set<int> skipNumbers(std::make_move_iterator(v.begin()),
                            std::make_move_iterator(v.end()));
  const auto requiredPredecessors = ParserUtils::getTokens(predecessors, ',');
  bool hasRequiredPredecessor = requiredPredecessors.size() ? false : true;
  for (const auto &predecessor : requiredPredecessors)
    if (predecessor != "") {
      if (stepTargets[stoi(predecessors)].size())
        hasRequiredPredecessor = true;
      else if (skipNumbers.find(stoi(predecessors)) != skipNumbers.end())
        hasRequiredPredecessor = true;
    }
  return hasRequiredPredecessor;
}

/* checks all requirements for a national focus. Returns false if any
 * requirement isn't fulfilled, else returns true*/
bool Generator::targetFulfillsRequirements(
    const std::string &targetRequirements, const Hoi4Country &source,
    const Hoi4Country &target, const std::vector<Scenario::Region> &gameRegions,
    const std::vector<std::set<std::string>> &levelTargets, const int level) {
  // now check if the country fulfills the target requirements
  // need to check rank, first get the desired value
  auto value = ParserUtils::getBracketBlockContent(targetRequirements, "rank");
  if (value != "" && value != "any") {
    if (target.rank != value)
      return false; // targets rank is not right
  }
  value = ParserUtils::getBracketBlockContent(targetRequirements, "ideology");
  if (value != "" && value != "any") {
    if (value == "same") {
      if (target.rulingParty != source.rulingParty) {
        return false;
      }
    } else if (value == "not") {
      if (target.rulingParty == source.rulingParty)
        return false;
    } else {
      // for any other value, must be specific ideology
      if (target.rulingParty != source.rulingParty)
        return false;
    }
  }
  value = ParserUtils::getBracketBlockContent(targetRequirements, "location");
  if (value != "" && value != "any") {
    if (value == "neighbour") {
      if (source.neighbours.find(target.tag) == source.neighbours.end())
        return false;
    }
    if (value == "near") {
      auto maxDistance = sqrt(Cfg::Values().width * Cfg::Values().height) * 0.2;
      if (Utils::getDistance(gameRegions[source.capitalRegionID].position,
                             gameRegions[target.capitalRegionID].position,
                             Cfg::Values().width) > maxDistance)
        return false;
    }
    if (value == "far") {
      auto minDistance = sqrt(Cfg::Values().width * Cfg::Values().height) * 0.2;
      if (Utils::getDistance(gameRegions[source.capitalRegionID].position,
                             gameRegions[target.capitalRegionID].position,
                             Cfg::Values().width) < minDistance)
        return false;
    }
  }
  value = ParserUtils::getBracketBlockContent(targetRequirements, "target");
  if (value != "" && value != "any") {
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
        if (levelTargets[i].find(target.tag) != levelTargets[i].end())
          return false;
      }
    }
    if (value == "chain") {
      bool foundUse = false;
      for (int i = 0; i < levelTargets.size(); i++) {
        // don't consider this country if NOT used in same chain
        if (levelTargets[i].find(target.tag) == levelTargets[i].end())
          foundUse = true;
      }
      if (!foundUse)
        return false;
    }
  }
  return true;
}

void Generator::evaluateCountryGoals() {
  Utils::Logging::logLine("HOI4: Generating Country Goals");
  std::vector<int> defDate{1, 1, 1936};
  std::vector<std::vector<std::vector<std::string>>> chains;

  chains.push_back(ParserUtils::getLinesByID(
      "resources\\hoi4\\ai\\national_focus\\chains\\major_chains.txt"));
  chains.push_back(ParserUtils::getLinesByID(
      "resources\\hoi4\\ai\\national_focus\\chains\\regional_chains.txt"));
  chains.push_back(ParserUtils::getLinesByID(
      "resources\\hoi4\\ai\\national_focus\\chains\\army_chains.txt"));
  auto typeCounter = 0;
  for (auto &sourceCountry : hoi4Countries) {
    const auto &source = hoi4Countries[sourceCountry.first];
    sourceCountry.second.bully = 0;
    // sourceCountry.second.defensive = 0;
    for (const auto &chainType : chains) {
      for (const auto &chain : chainType) {
        // evaluate whole chain (chain defined by ID)
        if (!chain.size())
          continue;
        // we need to save options for every chain step
        std::vector<std::set<Hoi4Country>> stepTargets;
        stepTargets.resize(100);
        std::vector<std::set<std::string>> levelTargets(chain.size());
        int chainID = 0;
        for (const auto &chainFocus : chain) {
          Utils::Logging::logLineLevel(9, chainFocus);
          // evaluate every single focus of that chain
          const auto chainTokens = ParserUtils::getTokens(chainFocus, ';');
          const int chainStep = stoi(chainTokens[1]);
          chainID = stoi(chainTokens[0]);
          const int level = stoi(chainTokens[12]);
          if ((chainTokens[3].find(source.rank) != std::string::npos ||
               chainTokens[3] == "any") &&
              (chainTokens[4].find(source.rulingParty) != std::string::npos ||
               chainTokens[4] == "any")) {
            if (stepFulfillsRequirements(chainTokens[2], stepTargets)) {
              const auto &targetRequirements = chainTokens[6];
              // if there are no target requirements, only the country itself is
              // a target
              if (!targetRequirements.size())
                stepTargets[chainStep].insert(sourceCountry.second);
              else {
                for (auto &destCountry : hoi4Countries) {
                  // now check every country if it fulfills the target
                  // requirements
                  if (targetFulfillsRequirements(
                          targetRequirements,
                          hoi4Countries[sourceCountry.first],
                          destCountry.second, gameRegions, levelTargets,
                          level)) {
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
          Utils::Logging::logLineLevel(5, "Building focus");
          std::map<int, NationalFocus> fulfilledSteps;
          int stepIndex = -1;
          std::vector<NationalFocus> chainFoci;

          for (auto &targets : stepTargets) {
            stepIndex++;
            if (!targets.size())
              continue;
            // select random target
            const auto &target = Utils::selectRandom(targets);
            auto focus{buildFocus(ParserUtils::getTokens(chain[stepIndex], ';'),
                                  hoi4Countries.at(sourceCountry.first),
                                  target)};
            focus.stepID = stepIndex;
            focus.chainID = chainID;
            Utils::Logging::logLineLevel(1, focus);
            if (focus.fType == NationalFocus::FocusType::attack) {
              // country aims to bully
              sourceCountry.second.bully++;
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

void Generator::printStatistics() {
  Utils::Logging::logLine("Total Industry: ", totalWorldIndustry);
  Utils::Logging::logLine("Military Industry: ", militaryIndustry);
  Utils::Logging::logLine("Civilian Industry: ", civilianIndustry);
  Utils::Logging::logLine("Naval Industry: ", navalIndustry);
  for (auto &res : totalResources) {
    Utils::Logging::logLine(res.first, " ", res.second);
  }

  Utils::Logging::logLine("World Population: ", worldPop);

  for (auto &scores : strengthScores) {
    for (auto &entry : scores.second) {
      Utils::Logging::logLine("Strength: ", scores.first, " ",
                              hoi4Countries.at(entry).fullName, " ",
                              hoi4Countries.at(entry).rulingParty, "");
    }
  }
}

bool Generator::unitFulfillsRequirements(
    std::vector<std::string> unitRequirements, Hoi4Country &country) {
  // now check if the country fulfills the target requirements
  for (auto &requirement : unitRequirements) {
    // need to check rank, first get the desired value
    auto value = ParserUtils::getBracketBlockContent(requirement, "rank");
    if (value != "") {
      if (value.find("any") == std::string::npos)
        continue; // fine, may target any ideology
      if (value.find(country.rank) == std::string::npos)
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
} // namespace Scenario::Hoi4