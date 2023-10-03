#include "hoi4/Hoi4Generator.h"
using namespace Fwg;
using namespace Fwg::Gfx;
namespace Scenario::Hoi4 {
Generator::Generator() {}

Generator::Generator(const std::string &configSubFolder)
    : Scenario::Generator(configSubFolder) {
  nData = NameGeneration::prepare("resources\\names");
}

Generator::~Generator() {}

void Generator::initializeStates() {
  hoi4States.clear();
  for (auto &region : this->gameRegions) {
    hoi4States.push_back(std::make_shared<Region>(*region));
  }
}

void Generator::initializeCountries() {
  hoi4Countries.clear();
  for (auto &country : countries) {
    // construct a hoi4country with country from ScenarioGenerator.
    // We want a copy here
    Hoi4Country hC(country.second, this->hoi4States);
    hoi4Countries.insert({hC.tag, hC});
  }
  std::sort(hoi4States.begin(), hoi4States.end(),
            [](auto l, auto r) { return *l < *r; });
}

void Generator::generateStateResources() {
  Fwg::Utils::Logging::logLine("HOI4: Digging for resources");
  for (auto &hoi4Region : hoi4States) {
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

void Generator::generateStateSpecifics() {
  Fwg::Utils::Logging::logLine("HOI4: Planning the economy");
  auto &config = Cfg::Values();
  // calculate the target industry amount
  auto targetWorldIndustry = 1248 * sizeFactor * industryFactor;
  Fwg::Utils::Logging::logLine(config.landPercentage);
  for (auto &hoi4Region : hoi4States) {
    if (hoi4Region->sea)
      continue;
    // count the number of land states for resource generation
    landStates++;
    double totalStateArea = 0;
    double totalDevFactor = 0;
    double totalPopFactor = 0;
    for (const auto &gameProv : hoi4Region->gameProvinces) {
      totalDevFactor +=
          gameProv->devFactor / (double)hoi4Region->gameProvinces.size();
      totalPopFactor +=
          gameProv->popFactor / (double)hoi4Region->gameProvinces.size();
      totalStateArea += gameProv->baseProvince->pixels.size();
    }
    // state level is calculated from population and development
    hoi4Region->stateCategory =
        std::clamp((int)(totalPopFactor * 5.0 + totalDevFactor * 6.0), 0, 9);
    // one province region? Must be an island state
    if (hoi4Region->gameProvinces.size() == 1) {
      hoi4Region->stateCategory = 1;
    }
    hoi4Region->development = totalDevFactor;
    // only init this when it hasn't been initialized via text input before
    if (hoi4Region->population < 0) {
      hoi4Region->population = totalStateArea * 1250.0 * totalPopFactor *
                               worldPopulationFactor * (1.0 / sizeFactor);
    }
    worldPop += (long long)hoi4Region->population;

    // count the total coastal provinces of this region
    auto totalCoastal = 0;
    for (auto &gameProv : hoi4Region->gameProvinces) {
      if (gameProv->baseProvince->coastal) {
        totalCoastal++;
        // only create a naval base, if a coastal supply hub was determined in
        // this province
        if (gameProv->attributeDoubles["naval_bases"] == 1)
          gameProv->attributeDoubles["naval_bases"] = RandNum::getRandom(1, 5);
      } else {
        gameProv->attributeDoubles["naval_bases"] = 0;
      }
    }
    // calculate total industry in this state
    if (targetWorldIndustry != 0) {
      auto stateIndustry =
          round(0.5 + totalPopFactor * (targetWorldIndustry /
                                        (double)(this->gameRegions.size())));
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
        } else if (Fwg::Utils::inRange(dockChance, dockChance + civChance,
                                       choice)) {
          hoi4Region->civilianFactories++;

        } else {
          hoi4Region->armsFactories++;
        }
      }
    }
    militaryIndustry += (int)hoi4Region->armsFactories;
    civilianIndustry += (int)hoi4Region->civilianFactories;
    navalIndustry += (int)hoi4Region->dockyards;
    // get potential building positions
    hoi4Region->calculateBuildingPositions(this->heightMap, typeMap);
  }
  dumpRegions(hoi4States);
}

void Generator::generateCountrySpecifics() {
  Fwg::Utils::Logging::logLine("HOI4: Choosing uniforms and electing Tyrants");
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
  const std::vector<std::string> gfxCultures{
      "western_european", "eastern_european", "middle_eastern", "african",
      "southamerican",    "commonwealth",     "asian"};
  const std::vector<std::string> ideologies{"fascism", "democratic",
                                            "communism", "neutrality"};
  initializeCountries();
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
    for (auto &reg : strat.gameRegionIDs) {
      for (auto i = 0; i < 12; i++) {
        double averageTemperature = 0.0;
        double averageDeviation = 0.0;
        double averagePrecipitation = 0.0;
        for (auto &prov : gameRegions[reg]->gameProvinces) {
          averageDeviation += prov->baseProvince->weatherMonths[i][0];
          averageTemperature += prov->baseProvince->weatherMonths[i][1];
          averagePrecipitation += prov->baseProvince->weatherMonths[i][2];
        }
        double divisor = (int)gameRegions[reg]->gameProvinces.size();
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
      if ((region->stateCategory > 6 &&
           region->ID != country.second.capitalRegionID)
          // if we're nearing the end of our region std::vector, and don't have
          // more than 25% of our regions as supply bases generate supply bases
          // for the last two regions
          || (country.second.hoi4Regions.size() > 2 &&
              (region->ID == (*(country.second.hoi4Regions.end() - 2))->ID) &&
              supplyHubProvinces.size() <
                  (country.second.hoi4Regions.size() / 4))) {
        // select a random gameprovince of the state

        auto y{Fwg::Utils::selectRandom(region->gameProvinces)};
        for (auto &prov : region->gameProvinces) {
          if (prov->baseProvince->coastal) {
            // if this is a coastal region, the supply hub is a naval base as
            // well
            y = prov;
            prov->attributeDoubles["naval_bases"] = 1;
            break;
          }
        }
        // save the province under the provinces ID
        supplyHubProvinces[y->ID] = y;
        navalBases[y->ID] = y->baseProvince->coastal;
        // get the distance between this supply hub and the capital
        auto distance = Fwg::getPositionDistance(
            capitalPosition, y->baseProvince->position, width);
        // save the distance under the province ID
        supplyHubs[distance] = y->ID;
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
          // NOT at the start of the search, therefore sourceNodeID must be the
          // last element of passThroughStates
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
        // now check every sourceNode neighbour for distance to destinationNode
        for (auto &neighbourGProvince :
             gameProvinces[sourceNodeID]->neighbours) {
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
      for (auto &pix : supplyHubProvince.second->baseProvince->pixels) {
        logistics.setColourAtIndex(pix, {0, 255, 0});
      }
    }
  }
  for (auto &connection : supplyNodeConnections) {
    for (int i = 0; i < connection.size(); i++) {
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
  strengthScores.clear();
  double maxScore = 0.0;
  for (auto &c : hoi4Countries) {
    c.second.capitalRegionID = 0;
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

  int totalDeployedCountries =
      numCountries - strengthScores.size() ? (int)strengthScores[0].size() : 0;
  int numMajorPowers = totalDeployedCountries / 10;
  int numRegionalPowers = totalDeployedCountries / 3;
  int numWeakStates =
      totalDeployedCountries - numMajorPowers - numRegionalPowers;
  for (const auto &scores : strengthScores) {
    for (const auto &entry : scores.second) {
      if (hoi4Countries.at(entry).strengthScore > 0.0) {
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
  auto unitTemplateFile =
      Parsing::readFile("resources\\hoi4\\history\\divisionTemplates.txt");
  // now tokenize by : character to get single
  auto unitTemplates = Fwg::Parsing::getTokens(unitTemplateFile, ':');
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
    auto totalUnits = c.second.strengthScore / 5;
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

  for (auto &scores : strengthScores) {
    for (auto &entry : scores.second) {
      Fwg::Utils::Logging::logLine("Strength: ", scores.first, " ",
                                   hoi4Countries.at(entry).fullName, " ",
                                   hoi4Countries.at(entry).rulingParty, "");
    }
  }
}

void Generator::loadStates() {}

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
} // namespace Scenario::Hoi4