#include "hoi4/Hoi4Generator.h"
using namespace Fwg;
using namespace Fwg::Gfx;
namespace Rpx::Hoi4 {
Generator::Generator() {}

Generator::Generator(const std::string &configSubFolder)
    : Rpx::ModGenerator(configSubFolder) {}

Generator::~Generator() {}

void Generator::mapRegions() {
  Fwg::Utils::Logging::logLine("Mapping Regions");
  ardaRegions.clear();
  hoi4States.clear();
  worldPop = 0;
  militaryIndustry = 0;
  civilianIndustry = 0;
  navalIndustry = 0;
  totalWorldIndustry = 0;
  statesInitialised = false;
  for (auto &region : this->areaData.regions) {
    std::sort(region.provinces.begin(), region.provinces.end(),
              [](const std::shared_ptr<Fwg::Areas::Province> a,
                 const std::shared_ptr<Fwg::Areas::Province> b) {
                return (*a < *b);
              });
    auto ardaRegion = std::make_shared<Region>(region);
    // generate random name for region
    ardaRegion->name = "";
    ardaRegion->identifier = "STATE_" + std::to_string(region.ID + 1);

    for (auto &province : ardaRegion->provinces) {
      ardaRegion->ardaProvinces.push_back(ardaProvinces[province->ID]);
    }
    // save game region to generic module container and to hoi4 specific
    // container
    ardaRegions.push_back(ardaRegion);
    hoi4States.push_back(ardaRegion);
  }
  // sort by Arda::ArdaProvince ID
  std::sort(ardaRegions.begin(), ardaRegions.end(),
            [](auto l, auto r) { return *l < *r; });
  // check if we have the same amount of ardaProvinces as FastWorldGen provinces
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

Fwg::Gfx::Bitmap Generator::mapTerrain() {
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
      const auto &baseProv = gameProv->baseProvince;
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
  Png::save(typeMap, Fwg::Cfg::Values().mapsPath + "Maps/typeMap.png");
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
  std::vector<std::shared_ptr<Arda::Country>> countryVector;
  for (auto &country : countries) {
    countryVector.push_back(country.second);
  }
  countries.clear();
  for (auto &country : countryVector) {
    countries[country->tag] = country;
  }

  for (auto &country : countries) {
    // construct a hoi4country with country from ScenarioGenerator.
    // We want a copy here
    // Hoi4Country hC(*country.second, this->hoi4States);
    // push back cast to hoi4Country
    // hoi4Countries.push_back(
    //    std::make_shared<Hoi4Country>(country.second, hoi4States));
    // Attempt to cast the shared pointer to Hoi4Country
    auto hoi4Country = std::dynamic_pointer_cast<Hoi4Country>(country.second);
    if (hoi4Country) {

      // Successfully casted, add to hoi4Countries
      hoi4Countries.push_back(hoi4Country);
      // now for all ownedRegions, find the equivalent in Hoi4Regions
      for (auto &region : country.second->ownedRegions) {
        for (auto &hoi4Region : hoi4States) {
          if (region->ID == hoi4Region->ID) {
            hoi4Country->hoi4Regions.push_back(hoi4Region);
          }
        }
      }
    } else {
      // Handle the case where the cast fails, if necessary
      // For example, log an error or throw an exception
      Fwg::Utils::Logging::logLine("Failed to cast Country to Hoi4Country");
    }
  }
  // now also map the neighbours by replacing the pointer to the country with
  // the pointer to the hoi4Country
  for (auto &country : hoi4Countries) {
    std::vector<std::shared_ptr<Hoi4Country>> neighboursTemp;
    for (auto &neighbour : country->neighbours) {
      if (neighbour) {
        for (auto &hoi4Country : hoi4Countries) {
          if (neighbour->ID == hoi4Country->ID) {
            neighboursTemp.push_back(hoi4Country);
          }
        }
      }
    }

    country->neighbours.clear();
    for (auto &neighbour : neighboursTemp) {
      country->neighbours.insert(neighbour);
    }
  }
  std::sort(hoi4States.begin(), hoi4States.end(),
            [](auto l, auto r) { return *l < *r; });
}

void Generator::generateStateResources() {
  Fwg::Utils::Logging::logLine("HOI4: Digging for resources");

  // config for all resource types
  for (auto &resConfig : resConfigs) {
    std::vector<float> resPrev;
    if (resConfig.random) {
      resPrev = Fwg::Civilization::Resources::randomResourceLayer(
          resConfig.name, resConfig.noiseConfig.fractalFrequency,
          resConfig.noiseConfig.tanFactor, resConfig.noiseConfig.cutOff,
          resConfig.noiseConfig.mountainBonus);
    } else if (resConfig.considerSea) {
      resPrev = Fwg::Civilization::Resources::coastDependentLayer(
          resConfig.name, resConfig.oceanFactor, resConfig.lakeFactor,
          areaData.provinces);
    } else {
      resPrev = Fwg::Civilization::Resources::climateDependentLayer(
          resConfig.name, resConfig.noiseConfig.fractalFrequency,
          resConfig.noiseConfig.tanFactor, resConfig.noiseConfig.cutOff,
          resConfig.noiseConfig.mountainBonus, resConfig.considerClimate,
          resConfig.climateEffects, resConfig.considerTrees,
          resConfig.treeEffects, climateData);
    }
    if (resPrev.size())
      totalResourceVal(resPrev,
                       resConfig.resourcePrevalence *
                           resources.at(resConfig.name).at(0),
                       resConfig);
  }
}

void Generator::generateStateSpecifics() {
  Fwg::Utils::Logging::logLine("HOI4: Planning the economy");
  auto &config = Cfg::Values();
  // calculate the target industry amount
  auto targetWorldIndustry = 1248.0 * worldIndustryFactor;
  auto targetWorldPop = 3'000'000'000.0 * worldPopulationFactor;
  // we need a reference to determine how industrious a state is
  double averageEconomicActivity = 1.0 / areaData.landRegions;

  worldPop = 0;
  militaryIndustry = 0;
  civilianIndustry = 0;
  navalIndustry = 0;
  totalWorldIndustry = 0;
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
    if (!hoi4State->isLand())
      continue;

    // state level is calculated from population and development
    hoi4State->stateCategory =
        std::clamp((int)(2.0 + 3.0 * hoi4State->worldEconomicActivityShare /
                                   averageEconomicActivity),
                   0, 9);
    hoi4State->infrastructure =
        std::clamp((int)(1.0 +
                         (hoi4State->worldEconomicActivityShare /
                          averageEconomicActivity) *
                             0.5 +
                         3.0 * hoi4State->developmentFactor),
                   1, 5);
    // one province state? Must be an island state
    if (hoi4State->ardaProvinces.size() == 1) {
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
            8, "Naval base in ", hoi4State->name, " at ", location->provinceID,
            " with importance ", location->importance);
      }
    }
    double dockChance = 0.25;
    double civChance = 0.5;
    // distribute it to military, civilian and naval factories
    if (!hoi4State->coastal) {
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
    hoi4State->calculateBuildingPositions(this->terrainData.detailedHeightMap,
                                          typeMap);
  }
  totalWorldIndustry = militaryIndustry + civilianIndustry + navalIndustry;
  statesInitialised = true;
  Arda::Parsing::dumpRegions(hoi4States);
}

void Generator::generateCountrySpecifics() {
  Fwg::Utils::Logging::logLine("HOI4: Choosing uniforms and electing Tyrants");

  const std::vector<std::string> caucasianGfxCultures{
      "western_european", "eastern_european", "commonwealth"};

  const std::vector<std::string> ideologies{"fascism", "democratic",
                                            "communism", "neutrality"};
  for (auto &country : hoi4Countries) {
    if (!country->ownedRegions.size())
      continue;
    // clear some info from all owned regions
    for (auto &region : country->hoi4Regions) {
      region->airBase = nullptr;
    }
    // refresh the provinces
    country->evaluateProvinces();
    switch (country->getPrimaryCulture()->visualType) {
    case Arda::VisualType::ASIAN:
      country->gfxCulture = "asian";
      break;
    case Arda::VisualType::AFRICAN:
      country->gfxCulture = "african";
      break;
    case Arda::VisualType::ARABIC:
      country->gfxCulture = "middle_eastern";
      break;
    case Arda::VisualType::CAUCASIAN:
      country->gfxCulture = Fwg::Utils::selectRandom(caucasianGfxCultures);
      break;
    case Arda::VisualType::SOUTH_AMERICAN:
      country->gfxCulture = "southamerican";
      break;
    }
    // select a random country ideology
    double totalPopularity = 0;
    std::vector<int> popularities(4);

    // Generate random popularities and calculate the total
    for (auto &popularity : popularities) {
      popularity = RandNum::getRandom(1, 100);
      totalPopularity += popularity;
    }

    // Normalize popularities to ensure they sum up to 100
    int sumPop = 0;
    for (int i = 0; i < 4; ++i) {
      popularities[i] = (popularities[i] / totalPopularity) * 100;
      sumPop += popularities[i];
      int offset = 0;
      // Ensure the total sum is exactly 100
      if (i == 3 && sumPop < 100) {
        offset = 100 - sumPop;
      }
      country->parties[i] = popularities[i] + offset;
    }

    // Assign a ideology from strongest popularity
    country->ideology = ideologies[std::max_element(country->parties.begin(),
                                                    country->parties.end()) -
                                   country->parties.begin()];
    // in randomly 1 of 5 cases, take the second strongest ideology
    if (RandNum::getRandom(0, 5) == 0) {
      country->ideology =
          ideologies[std::max_element(country->parties.begin(),
                                      country->parties.end() - 1) -
                     country->parties.begin()];
    }

    // allow or forbid elections
    if (country->ideology == "democratic")
      country->allowElections = 1;
    else if (country->ideology == "neutrality")
      country->allowElections = RandNum::getRandom(0, 1);
    else
      country->allowElections = 0;

    // randomly gather the date of the last election, up to a maximum of 48
    // months back, except for non-democratic countries
    std::string electionYear = std::to_string(
        RandNum::getRandom(country->allowElections ? 1932 : 1880, 1935));
    std::string electionMonth = std::to_string(RandNum::getRandom(1, 12));
    std::string electionDay = std::to_string(RandNum::getRandom(1, 28));
    country->lastElection =
        electionYear + "." + electionMonth + "." + electionDay;

    // random stability between 0 and 100
    country->stability = RandNum::getRandom(0, 100);
    // random war support between 0 and 100, higher for fascist and communist
    // countries
    if (country->ideology == "fascism" || country->ideology == "communism") {
      country->warSupport = RandNum::getRandom(40, 80);
    } else {
      country->warSupport = RandNum::getRandom(0, 60);
    }

    // amount of research slots between 3 and 6, depending on average
    // development of the country and strength rank
    auto rankModifier = 0.0;
    if (country->rank == Arda::Rank::RegionalPower) {
      rankModifier = 0.5;
    } else if (country->rank == Arda::Rank::GreatPower) {
      rankModifier = 1.0;
    } else if (country->rank == Arda::Rank::SecondaryPower) {
      rankModifier = 0.75;
    } else if (country->rank == Arda::Rank::LocalPower) {
      rankModifier = 0.25;
    }

    // rounded to the nearest integer
    country->researchSlots =
        std::round(3.0 + 2.0 * country->averageDevelopment + rankModifier);

    // now get the full name of the country
    country->fullName = NameGeneration::modifyWithIdeology(
        country->ideology, country->name, country->adjective, nData);

    // military focus: first gather info about position of the country, taking
    // coastline into account
    auto coastalRegions = 0.0;
    for (auto &region : country->hoi4Regions) {
      if (region->coastal) {
        coastalRegions++;
      }
    }
    // naval focus goes from 0-50%. If we have a lot of coastal regions, we
    // focus on naval
    country->navalFocus = std::clamp(
        (coastalRegions / country->hoi4Regions.size() * 100.0), 0.0, 50.0);
    // gather all naval bases from all regions
    std::vector<int> navalBases;
    for (auto &region : country->hoi4Regions) {
      for (auto &navBase : region->navalBases) {
        navalBases.push_back(navBase.first);
      }
    }

    // check if this country has a navalfocus larger 0, but no port
    if (country->navalFocus > 0 && navalBases.size() == 0) {
      // if we have a naval focus, but no port, we need to reduce the naval
      // focus
      country->landFocus += country->navalFocus;
      country->navalFocus = 0;
    }

    // TODO: Increase if our position is very remote?
    // now let's get the air focus, which primarily depends on randomness,
    // should be between 5 and 35%
    country->airFocus = RandNum::getRandom(5.0, 35.0);
    // land focus is the rest
    country->landFocus = 100.0 - country->navalFocus - country->airFocus;
  }
  generateTechLevels();
  generateArmorVariants();
  generateCountryUnits();
  generateCountryNavies();
  generateAirVariants();
  generateCharacters();
}

void Generator::generateWeather() {
  for (auto &strat : strategicRegions) {
    for (auto &reg : strat.ardaRegions) {
      for (auto i = 0; i < 12; i++) {
        double averageTemperature = 0.0;
        double averageDeviation = 0.0;
        double averagePrecipitation = 0.0;
        for (auto &prov : reg->ardaProvinces) {
          averageDeviation += prov->baseProvince->weatherMonths[i][0];
          averageTemperature += prov->baseProvince->weatherMonths[i][1];
          averagePrecipitation += prov->baseProvince->weatherMonths[i][2];
        }
        double divisor = (int)reg->ardaProvinces.size();
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
    // Arda::ArdaProvince ID, distance
    std::map<double, int> supplyHubs;
    // add capital
    auto capitalPosition = ardaRegions[country->capitalRegionID]->position;
    auto &capitalProvince = Fwg::Utils::selectRandom(
        ardaRegions[country->capitalRegionID]->ardaProvinces);
    std::vector<double> distances;
    // region ID, provinceID
    std::map<int, std::shared_ptr<Arda::ArdaProvince>> supplyHubProvinces;
    std::map<int, bool> navalBases;
    std::set<int> gProvIDs;
    for (auto &region : country->hoi4Regions) {
      if ((region->stateCategory > 4 && region->ID != country->capitalRegionID)
          // if we're nearing the end of our region std::vector, and don't
          // have more than 25% of our regions as supply bases generate
          // supply bases for the last two regions
          || (country->hoi4Regions.size() > 2 &&
              (region->ID == (*(country->hoi4Regions.end() - 2))->ID) &&
              supplyHubProvinces.size() < (country->hoi4Regions.size() / 4))) {
        if (!region->isLand())
          continue;
        // select a random Arda::ArdaProvince of the state
        int lakeCounter = 0;
        auto hubProvince{Fwg::Utils::selectRandom(region->ardaProvinces)};
        while (hubProvince->baseProvince->isLake() && lakeCounter++ < 1000) {
          hubProvince = Fwg::Utils::selectRandom(region->ardaProvinces);
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
            hubProvince = ardaProvinces[location->provinceID];
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
      for (auto &gProv : region->ardaProvinces) {
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
                  ardaProvinces[supplyHubs[distance2]]->baseProvince->position,
                  ardaProvinces[supplyHubs[distance]]->baseProvince->position,
                  width);
              if (dist3 < tempDistance) {
                sourceNodeID = ardaProvinces[supplyHubs[distance2]]->ID;
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
        if (ardaProvinces[sourceNodeID]->baseProvince->landMassID !=
            ardaProvinces[destNodeID]->baseProvince->landMassID)
          break;
        ;
        // the origins position
        auto sourceNodePosition =
            ardaProvinces[sourceNodeID]->baseProvince->position;
        // save the distance in a temp variable
        double tempMinDistance = width;
        auto closestID = INT_MAX;
        // now check every sourceNode neighbour for distance to
        // destinationNode
        for (auto &neighbourGProvince :
             ardaProvinces[sourceNodeID]->neighbours) {
          // check if this belongs to us and is an eligible province
          if (gProvIDs.find(neighbourGProvince.ID) == gProvIDs.end() ||
              neighbourGProvince.baseProvince->isLake() ||
              neighbourGProvince.baseProvince->isSea())
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
              ardaProvinces[destNodeID]->baseProvince->position,
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
      if (ardaProvinces[connection[i]]->baseProvince->isSea() ||
          ardaProvinces[connection[i]]->baseProvince->isLake()) {
        Fwg::Utils::Logging::logLine("Error: Skipping an invalid connection "
                                     "due to sea or lake province "
                                     "in it in logistics");
        connection.erase(connection.begin() + i);
        i--;
        continue;
      }
      for (auto pix : ardaProvinces[connection[i]]->baseProvince->pixels) {
        // don't overwrite capitals and supply nodes
        if (logistics[pix] == Colour{255, 255, 0} ||
            logistics[pix] == Colour{0, 255, 0})
          continue;
        logistics.setColourAtIndex(pix, {255, 255, 255});
      }
    }
  }
  Bmp::save(logistics, Fwg::Cfg::Values().mapsPath + "//logistics.bmp");
}

void createTech(const std::vector<std::string> &fileLines,
                std::map<TechEra, std::vector<Technology>> &techMap) {
  for (const auto &line : fileLines) {
    if (line.size()) {
      auto parts = Fwg::Parsing::getTokens(line, ';');
      if (parts.size() == 3) {
        Technology tech;
        if (parts[2] == "interwar") {
          tech.era = TechEra::Interwar;
        } else if (parts[2] == "buildup") {
          tech.era = TechEra::Buildup;
        } else if (parts[2] == "early") {
          tech.era = TechEra::Early;
        }
        tech.name = parts[0];
        tech.predecessor = parts[1];
        techMap[tech.era].push_back(tech);
      }
    }
  }
}

void assignTechsRandomly(
    const std::map<TechEra, std::vector<Technology>> &techsToAssign,
    std::map<TechEra, std::vector<Technology>> &countryCategoryTechs,
    double techLevel, double modifier) {
  // now randomly assign the module techs. Go through each era of the techs
  // and gather all the technology names that we have in a set.
  if (techsToAssign.find(TechEra::Interwar) != techsToAssign.end()) {
    for (auto &moduleTech : techsToAssign.at(TechEra::Interwar)) {
      // check if we already have that tech
      bool alreadyHas = false;
      for (auto &module : countryCategoryTechs.at(TechEra::Interwar)) {
        if (module.name == moduleTech.name) {
          alreadyHas = true;
          break;
        }
      }
      if (alreadyHas) {
        continue;
      }
      if (moduleTech.predecessor.size()) {
        // check if any of the previous era tech modules have the name of the
        // predecessor
        for (auto &module : countryCategoryTechs.at(TechEra::Interwar)) {
          if (module.name == moduleTech.predecessor) {
            countryCategoryTechs.at(TechEra::Interwar).push_back(moduleTech);
            break;
          }
        }
      }

      auto randomVal = RandNum::getRandom(0.0, 1.0) * techLevel;
      if (randomVal > 0.5) {
        countryCategoryTechs.at(TechEra::Interwar).push_back(moduleTech);
      }
    }
  }
  if (techsToAssign.find(TechEra::Buildup) != techsToAssign.end()) {
    for (auto &moduleTech : techsToAssign.at(TechEra::Buildup)) {
      auto randomVal = RandNum::getRandom(0.0, 1.0) * techLevel;
      if (randomVal > 1.0) {
        // check if any of the previous era tech modules have the name of the
        // predecessor
        for (auto &module : countryCategoryTechs.at(TechEra::Interwar)) {
          if (module.name == moduleTech.predecessor) {
            countryCategoryTechs.at(TechEra::Buildup).push_back(moduleTech);
            break;
          }
        }
      }
    }
  }
  if (techsToAssign.find(TechEra::Early) != techsToAssign.end()) {
    for (auto &moduleTech : techsToAssign.at(TechEra::Early)) {
      auto randomVal = RandNum::getRandom(0.0, 1.0) * techLevel;
      if (randomVal > 1.5) {
        // check if any of the previous era tech modules have the name of the
        // predecessor
        for (auto &module : countryCategoryTechs.at(TechEra::Buildup)) {
          if (module.name == moduleTech.predecessor) {
            countryCategoryTechs.at(TechEra::Early).push_back(moduleTech);
            break;
          }
        }
      }
    }
  }
}

void Generator::generateTechLevels() {
  // vector for all hull types
  const std::vector<NavalHullType> navalHullTypes{
      NavalHullType::Light, NavalHullType::Heavy, NavalHullType::Cruiser,
      NavalHullType::Carrier, NavalHullType::Submarine};

  // read in the techs from the files
  auto industryElectronicTechsFile = Fwg::Parsing::getLines(
      Fwg::Cfg::Values().resourcePath +
      "//hoi4//common//technologies//industryElectronicTechs.txt");
  std::map<TechEra, std::vector<Technology>> industryElectronicTechs;
  createTech(industryElectronicTechsFile, industryElectronicTechs);

  auto infantryTechsFile =
      Fwg::Parsing::getLines(Fwg::Cfg::Values().resourcePath +
                        "//hoi4//common//technologies//infantryTechs.txt");
  std::map<TechEra, std::vector<Technology>> infantryTechs;
  createTech(infantryTechsFile, infantryTechs);

  auto armorTechsFile =
      Fwg::Parsing::getLines(Fwg::Cfg::Values().resourcePath +
                        "//hoi4//common//technologies//armorTechs.txt");
  std::map<TechEra, std::vector<Technology>> armorTechs;
  createTech(armorTechsFile, armorTechs);

  auto airTechsFile =
      Fwg::Parsing::getLines(Fwg::Cfg::Values().resourcePath +
                        "//hoi4//common//technologies//airTechs.txt");
  std::map<TechEra, std::vector<Technology>> airTechs;
  createTech(airTechsFile, airTechs);

  auto navyTechsFile =
      Fwg::Parsing::getLines(Fwg::Cfg::Values().resourcePath +
                        "//hoi4//common//technologies//navyTechs.txt");
  std::map<TechEra, std::vector<Technology>> navyTechs;
  createTech(navyTechsFile, navyTechs);

  for (auto &country : hoi4Countries) {
    // clear all techs
    country->industryElectronicTechs = {
        {TechEra::Interwar, {}}, {TechEra::Buildup, {}}, {TechEra::Early, {}}};
    country->infantryTechs = {
        {TechEra::Interwar, {}}, {TechEra::Buildup, {}}, {TechEra::Early, {}}};
    country->armorTechs = {
        {TechEra::Interwar, {}}, {TechEra::Buildup, {}}, {TechEra::Early, {}}};
    country->airTechs = {
        {TechEra::Interwar, {}}, {TechEra::Buildup, {}}, {TechEra::Early, {}}};
    country->navyTechs = {
        {TechEra::Interwar, {}}, {TechEra::Buildup, {}}, {TechEra::Early, {}}};

    // a few techs are guranteed, such as infantry_weapons
    country->infantryTechs.at(TechEra::Interwar)
        .push_back({"infantry_weapons", "", TechEra::Interwar});
    // gurantee we have sonar and basic_battery
    country->navyTechs.at(TechEra::Interwar)
        .push_back({"sonar", "", TechEra::Interwar});
    country->navyTechs.at(TechEra::Interwar)
        .push_back({"basic_battery", "", TechEra::Interwar});
    auto development = country->averageDevelopment;
    auto navyTechLevel = development * country->navalFocus / 10.0;
    auto infantryTechLevel = development * country->landFocus / 10.0;
    auto armorTechLevel = development * country->landFocus / 10.0;
    auto airTechLevel = development * country->airFocus / 10.0;
    auto industryTechLevel = development * 5.0;

    assignTechsRandomly(airTechs, country->airTechs, airTechLevel, 1.0);
    // ensure we have meaningful techs for planes, should we have any
    adjustTechsForPlaneModules(country->airTechs);

    assignTechsRandomly(industryElectronicTechs,
                        country->industryElectronicTechs, industryTechLevel,
                        1.0);
    assignTechsRandomly(infantryTechs, country->infantryTechs,
                        infantryTechLevel, 1.0);
    assignTechsRandomly(armorTechs, country->armorTechs, armorTechLevel, 1.0);
    assignTechsRandomly(navyTechs, country->navyTechs, navyTechLevel, 1.0);
  }

  for (auto &country : hoi4Countries) {
    // lets start with the navy. The higher our development and the more focues
    // we are on navy, the more advanced our navy#
    auto development = country->averageDevelopment;
    auto navyTechLevel = development * country->navalFocus / 10.0;
    // generate a tech level for each hull type, either Interwar or BuildUp. The
    // higher the navy tech level, the more likely we are to get BuildUp
    // technology. Tech levels usually range between 0 and 5.
    for (auto &hull : navalHullTypes) {
      auto randomVal = RandNum::getRandom(0.0, 1.0) * navyTechLevel;
      if (randomVal > 0.8) {
        country->hullTech[hull].push_back(TechEra::Interwar);
        country->hullTech[hull].push_back(TechEra::Buildup);
      } else if (randomVal > 0.2) {
        country->hullTech[hull].push_back(TechEra::Interwar);
      }
    }
    // guarantee we have at least a destroyer tech
    if (country->hullTech[NavalHullType::Light].size() == 0) {
      country->hullTech[NavalHullType::Light].push_back(TechEra::Interwar);
    }
  }
}

void Generator::evaluateCountries() {
  Fwg::Utils::Logging::logLine("HOI4: Evaluating Country Strength");
  countryImportanceScores.clear();
  double maxScore = 0.0;
  for (auto &country : hoi4Countries) {
    country->evaluatePopulations(civData.worldPopulationFactorSum);
    country->evaluateDevelopment();
    country->evaluateEconomicActivity(worldEconomicActivity);
    country->evaluateProperties();
    country->capitalRegionID = 0;
    country->civilianIndustry = 0;
    country->dockyards = 0;
    country->armsFactories = 0;
    auto totalIndustry = 0.0;
    auto totalPop = 0.0;
    for (auto &ownedRegion : country->hoi4Regions) {
      country->civilianIndustry += ownedRegion->civilianFactories;
      country->dockyards += ownedRegion->dockyards;
      country->armsFactories += ownedRegion->armsFactories;

      totalIndustry += ownedRegion->civilianFactories + ownedRegion->dockyards +
                       ownedRegion->armsFactories;
      totalPop += (int)ownedRegion->totalPopulation;
    }
    // always make the most important location the capital
    country->selectCapital();
    countryImportanceScores[(int)(totalIndustry + totalPop / 1'000'000.0)]
        .push_back(country);
    country->importanceScore = totalIndustry + totalPop / 1'000'000.0;
    if (country->importanceScore > maxScore) {
      maxScore = country->importanceScore;
    }
    // global
    totalWorldIndustry += (int)totalIndustry;
  }

  int totalDeployedCountries = numCountries - countryImportanceScores.size()
                                   ? (int)countryImportanceScores[0].size()
                                   : 0;

  // sort countries by rank

  int numMajorPowers = std::min<int>(numCountries / 10, 8);
  int numSecondaryPowers = std::min<int>(numCountries / 10, 8);
  int numRegionalPowers = numCountries / 6;
  int numLocalPowers = numCountries / 6;
  int numMinorPowers =
      totalDeployedCountries - numMajorPowers - numRegionalPowers;

  // init countriesByRank
  countriesByRank = {{Arda::Rank::GreatPower, {}},
                     {Arda::Rank::SecondaryPower, {}},
                     {Arda::Rank::RegionalPower, {}},
                     {Arda::Rank::LocalPower, {}},
                     {Arda::Rank::MinorPower, {}}};

  for (auto it = countryImportanceScores.rbegin();
       it != countryImportanceScores.rend(); ++it) {
    for (const auto &entry : it->second) {
      if (entry->importanceScore > 0.0) {
        entry->relativeScore = (double)it->first / maxScore;
        if (numMajorPowers > countriesByRank.at(Arda::Rank::GreatPower).size()) {
          countriesByRank[Arda::Rank::GreatPower].push_back(entry);
          entry->rank = Arda::Rank::GreatPower;
        } else if (numSecondaryPowers >
                   countriesByRank.at(Arda::Rank::SecondaryPower).size()) {
          countriesByRank[Arda::Rank::SecondaryPower].push_back(entry);
          entry->rank = Arda::Rank::SecondaryPower;
        } else if (numRegionalPowers >
                   countriesByRank.at(Arda::Rank::RegionalPower).size()) {
          countriesByRank[Arda::Rank::RegionalPower].push_back(entry);
          entry->rank = Arda::Rank::RegionalPower;
        } else if (numLocalPowers >
                   countriesByRank.at(Arda::Rank::LocalPower).size()) {
          countriesByRank[Arda::Rank::LocalPower].push_back(entry);
          entry->rank = Arda::Rank::LocalPower;
        } else {
          countriesByRank[Arda::Rank::MinorPower].push_back(entry);
          entry->rank = Arda::Rank::MinorPower;
        }
      }
    }
  }
}

void Generator::generateArmorVariants() {
  struct TankType {
    ArmorType type;
    ArmorRole subType;
  };
  Fwg::Utils::Logging::logLine("HOI4: Generating Armor Variants");
  for (auto &country : hoi4Countries) {
    // first check if we have any armor techs
    if (hasTechnology(country->armorTechs, "gwtank_chassis")) {
      auto combinedTech = country->armorTechs;
      // add all landTechs for the different weapon types
      for (auto &techEra : country->infantryTechs) {
        for (auto &tech : techEra.second) {
          combinedTech.at(techEra.first).push_back(tech);
        }
      }
      std::map<std::string, TankType> chassisToGenerate;
      chassisToGenerate["light_tank_chassis_0"] = {ArmorType::LightArmor,
                                                   ArmorRole::Tank};
      chassisToGenerate["medium_tank_chassis_0"] = {ArmorType::MediumArmor,
                                                    ArmorRole::Tank};
      if (hasTechnology(country->armorTechs, "interwar_antitank")) {
        chassisToGenerate["light_tank_chassis_0"] = {ArmorType::LightArmor,
                                                     ArmorRole::TankDestroyer};
        chassisToGenerate["medium_tank_chassis_0"] = {ArmorType::MediumArmor,
                                                      ArmorRole::TankDestroyer};
      }
      if (hasTechnology(country->armorTechs, "interwar_artillery")) {
        chassisToGenerate["light_tank_chassis_0"] = {ArmorType::LightArmor,
                                                     ArmorRole::Artillery};
        chassisToGenerate["medium_tank_chassis_0"] = {ArmorType::MediumArmor,
                                                      ArmorRole::Artillery};
      }
      chassisToGenerate["heavy_tank_chassis_0"] = {ArmorType::HeavyArmor,
                                                   ArmorRole::Tank};
      if (hasTechnology(country->armorTechs, "basic_light_tank_chassis")) {
        chassisToGenerate["light_tank_chassis_1"] = {ArmorType::LightArmor,
                                                     ArmorRole::Tank};
        if (hasTechnology(country->armorTechs, "interwar_antitank")) {
          chassisToGenerate["light_tank_chassis_1"] = {
              ArmorType::LightArmor, ArmorRole::TankDestroyer};
        }
        if (hasTechnology(country->armorTechs, "interwar_artillery")) {
          chassisToGenerate["light_tank_chassis_1"] = {ArmorType::LightArmor,
                                                       ArmorRole::Artillery};
        }
      }
      if (hasTechnology(country->armorTechs, "improved_light_tank_chassis")) {
        chassisToGenerate["light_tank_chassis_2"] = {ArmorType::LightArmor,
                                                     ArmorRole::Tank};

        if (hasTechnology(country->armorTechs, "interwar_antitank")) {
          chassisToGenerate["light_tank_chassis_2"] = {
              ArmorType::LightArmor, ArmorRole::TankDestroyer};
        }
        if (hasTechnology(country->armorTechs, "interwar_artillery")) {
          chassisToGenerate["light_tank_chassis_2"] = {ArmorType::LightArmor,
                                                       ArmorRole::Artillery};
        }
      }

      if (hasTechnology(country->armorTechs, "basic_heavy_tank_chassis")) {
        chassisToGenerate["heavy_tank_chassis_1"] = {ArmorType::HeavyArmor,
                                                     ArmorRole::Tank};
        if (hasTechnology(country->armorTechs, "interwar_antitank")) {
          chassisToGenerate["heavy_tank_chassis_1"] = {
              ArmorType::HeavyArmor, ArmorRole::TankDestroyer};
        }
      }

      for (auto &chassis : chassisToGenerate) {
        // we can create a tank variant
        TankVariant tankVariant;
        tankVariant.type = chassis.second.type;
        tankVariant.subType = chassis.second.subType;
        tankVariant.bbaArmorName = chassis.first;
        tankVariant.era = TechEra::Interwar;
        tankVariant.name = country->getPrimaryCulture()
                               ->language->generateGenericCapitalizedWord() +
                           " Mk " + std::to_string(RandNum::getRandom(0, 3));

        addArmorModules(tankVariant, combinedTech);
        country->tankVariants.push_back(tankVariant);
      }
    }
  }
}

void Generator::generateAirVariants() {
  struct AirType {
    PlaneType type;
    PlaneRole subType;
    std::string frame;
    TechEra era;
  };
  Fwg::Utils::Logging::logLine("HOI4: Generating Air Variants");

  for (auto &country : hoi4Countries) {
    country->planeVariants.clear();
    bool hasCarrier = false;
    for (auto &ship : country->ships) {
      if (ship->shipClass.type == ShipClassType::Carrier) {
        hasCarrier = true;
        break;
      }
    }
    std::map<std::string, AirType> frameToGenerate;
    // first check if we have techs for small airframes
    if (hasTechnology(country->airTechs, "iw_small_airframe")) {
      frameToGenerate["iw_fighter"] = {PlaneType::SmallFrame,
                                       PlaneRole::Fighter, "iw_small_airframe",
                                       TechEra::Interwar};
      if (hasCarrier) {
        frameToGenerate["iw_carrier_fighter"] = {
            PlaneType::SmallFrame, PlaneRole::CarrierFighter,
            "iw_small_airframe", TechEra::Interwar};
      }
      if (hasTechnology(country->airTechs, "air_torpedoe_1")) {
        frameToGenerate["iw_nav_bomb"] = {
            PlaneType::SmallFrame, PlaneRole::NavalBomber, "iw_small_airframe",
            TechEra::Interwar};
        if (hasCarrier) {
          frameToGenerate["iw_carrier_nav_bomb"] = {
              PlaneType::SmallFrame, PlaneRole::CarrierNavalBomber,
              "iw_small_airframe", TechEra::Interwar};
        }
      }
      // check if we have everything for gw CAS
      if (hasTechnology(country->airTechs, "early_bombs")) {
        frameToGenerate["gw_cas"] = {PlaneType::SmallFrame, PlaneRole::Cas,
                                     "gw_small_airframe", TechEra::Interwar};
        if (hasCarrier) {
          frameToGenerate["iw_carrier_cas"] = {
              PlaneType::SmallFrame, PlaneRole::CarrierCas, "iw_small_airframe",
              TechEra::Interwar};
        }
      }
      // do the same for basic small airframes
      if (hasTechnology(country->airTechs, "basic_small_airframe")) {
        frameToGenerate["basic_fighter"] = {
            PlaneType::SmallFrame, PlaneRole::Fighter, "basic_small_airframe",
            TechEra::Buildup};
        if (hasCarrier) {
          frameToGenerate["basic_carrier_fighters"] = {
              PlaneType::SmallFrame, PlaneRole::CarrierFighter,
              "basic_small_airframe", TechEra::Buildup};
        }
        if (hasTechnology(country->airTechs, "air_torpedoe_1")) {
          frameToGenerate["basic_nav_bomb"] = {
              PlaneType::SmallFrame, PlaneRole::NavalBomber,
              "basic_small_airframe", TechEra::Buildup};
          if (hasCarrier) {
            frameToGenerate["basic_carrier_nav_bomb"] = {
                PlaneType::SmallFrame, PlaneRole::CarrierNavalBomber,
                "basic_small_airframe", TechEra::Buildup};
          }
        }
        // check if we have everything for basic CAS
        if (hasTechnology(country->airTechs, "early_bombs")) {
          frameToGenerate["basic_cas"] = {PlaneType::SmallFrame, PlaneRole::Cas,
                                          "basic_small_airframe",
                                          TechEra::Buildup};
          if (hasCarrier) {
            frameToGenerate["basic_carrier_cas"] = {
                PlaneType::SmallFrame, PlaneRole::CarrierCas,
                "basic_small_airframe", TechEra::Buildup};
          }
        }
      }
    }
    // tact bombers, strat bombers
    if (hasTechnology(country->airTechs, "early_bombs")) {
      if (hasTechnology(country->airTechs, "iw_medium_airframe")) {
        frameToGenerate["iw_tac_bomb"] = {
            PlaneType::MediumFrame, PlaneRole::TacticalBomber,
            "iw_medium_airframe", TechEra::Interwar};
        if (hasTechnology(country->airTechs, "basic_medium_airframe")) {
          frameToGenerate["basic_tac_bomb"] = {
              PlaneType::MediumFrame, PlaneRole::TacticalBomber,
              "basic_medium_airframe", TechEra::Buildup};
        }
      }
      if (hasTechnology(country->airTechs, "iw_large_airframe")) {
        frameToGenerate["iw_strat_bomb"] = {
            PlaneType::LargeFrame, PlaneRole::StrategicBomber,
            "iw_large_airframe", TechEra::Interwar};
        if (hasTechnology(country->airTechs, "basic_large_airframe")) {
          frameToGenerate["basic_strat_bomb"] = {
              PlaneType::LargeFrame, PlaneRole::StrategicBomber,
              "basic_large_airframe", TechEra::Buildup};
        }
      }
    }
    for (auto &frame : frameToGenerate) {
      // we can create a plane variant
      PlaneVariant airVariant;
      airVariant.type = frame.second.type;
      airVariant.subType = frame.second.subType;
      if (airVariant.type == PlaneType::SmallFrame) {
        airVariant.bbaFrameName = "small_plane_airframe_0";
        airVariant.vanillaFrameName = "fighter_equipment_0";
        if (airVariant.subType == PlaneRole::CarrierCas) {
          airVariant.bbaFrameName = "cv_small_plane_cas_airframe_0";
          airVariant.vanillaFrameName = "cv_CAS_equipment_0";
        } else if (airVariant.subType == PlaneRole::CarrierFighter) {
          airVariant.bbaFrameName = "cv_small_plane_airframe_0";
          airVariant.vanillaFrameName = "cv_fighter_equipment_0";
        } else if (airVariant.subType == PlaneRole::CarrierNavalBomber) {
          airVariant.bbaFrameName = "cv_small_plane_naval_bomber_airframe_0";
          airVariant.vanillaFrameName = "cv_nav_bomber_equipment_0";
        } else if (airVariant.subType == PlaneRole::Cas) {
          airVariant.bbaFrameName = "small_plane_cas_airframe_0";
          airVariant.vanillaFrameName = "CAS_equipment_0";
        } else if (airVariant.subType == PlaneRole::Fighter) {
          airVariant.bbaFrameName = "small_plane_airframe_0";
          airVariant.vanillaFrameName = "fighter_equipment_0";
        } else if (airVariant.subType == PlaneRole::NavalBomber) {
          airVariant.bbaFrameName = "small_plane_naval_bomber_airframe_0";
          airVariant.vanillaFrameName = "nav_bomber_equipment_0";
        }

      } else if (airVariant.type == PlaneType::MediumFrame) {
        airVariant.bbaFrameName = "medium_plane_airframe_0";
        airVariant.vanillaFrameName = "tac_bomber_equipment_0";
      } else if (airVariant.type == PlaneType::LargeFrame) {
        airVariant.bbaFrameName = "large_plane_airframe_0";
        airVariant.vanillaFrameName = "strat_bomber_equipment_0";
      }
      // if we have a basic variant, we replace the 0 with 1
      if (frame.second.era == TechEra::Buildup) {
        airVariant.bbaFrameName[airVariant.bbaFrameName.size() - 1] = '1';
      }

      airVariant.name = country->getPrimaryCulture()
                            ->language->generateGenericCapitalizedWord() +
                        " Mk " + std::to_string(RandNum::getRandom(0, 3));

      addPlaneModules(airVariant, country->airTechs);
      country->planeVariants.push_back(airVariant);
    }

    int airforceStrength = country->airFocus * country->armsFactories;
    int airBaseAmount = 1 + airforceStrength / 10;
    if (country->planeVariants.size() && airforceStrength > 0) {
      // lets distribute airbases throughout the country
      for (int i = 0; i < airBaseAmount; i++) {
        country->addAirBase(1);
      }
      // first gather the amount of planes per variant
      while (airforceStrength > 0) {
        auto &variant = Fwg::Utils::selectRandom(country->planeVariants);
        variant.amount++;
        airforceStrength -= variant.cost;
      }

      // now we generate the air wings
      for (auto i = 0; i < country->planeVariants.size(); i++) {
        for (auto j = 0; j < country->planeVariants[i].amount; j += 50) {
          AirWing wing;
          wing.variant = country->planeVariants[i];
          wing.name = std::to_string(i) + ". " + country->planeVariants[i].name;
          wing.amount = std::min<int>(country->planeVariants[i].amount, 50);
          auto &randomAirbase = Fwg::Utils::selectRandom(country->airBases);
          randomAirbase.second->wings.push_back(wing);
          country->airWings.push_back(wing);
        }
      }
    }
  }
}
void Generator::generateCountryUnits() {
  Fwg::Utils::Logging::logLine("HOI4: Generating Country Unit Files");

  const std::vector<DivisionType> divisionTypes = {
      DivisionType::Irregulars,
      DivisionType::Infantry,
      DivisionType::SupportedInfantry,
      DivisionType::HeavyArtilleryInfantry,
      DivisionType::Cavalry,
      DivisionType::Motorized,
      DivisionType::Armor};

  for (auto &country : hoi4Countries) {
    // first determine total army strength based on arms industry
    // TODO add a factor to settings
    country->totalArmyStrength = country->armsFactories * 30;

    // basic idea: we create unit templates first. We start with irregulars,
    // then infantry only, then infantry with support, then infantry with
    // artillery, then infantry with armor, then motorised infantry, then
    // motorised infantry with support, then motorised infantry with armor.
    // for each of those, we depend on certain techs.
    // each of these will vary a bit per country, depending on their techs and
    // some randomness in regiments per column (we vary between 2-4 regiments
    // of the same type per column)
    std::vector<CombatRegimentType> allowedRegimentTypes;
    std::vector<SupportRegimentType> allowedSupportRegimentTypes;
    std::set<DivisionType> desiredDivisionTemplates;
    // we also vary the amount of columns per division, between 2 and 4
    if (country->hasTech("infantry_weapons")) {
      allowedRegimentTypes.push_back(CombatRegimentType::Infantry);
      allowedRegimentTypes.push_back(CombatRegimentType::Irregulars);
      desiredDivisionTemplates.insert(DivisionType::Militia);
      desiredDivisionTemplates.insert(DivisionType::Infantry);
      desiredDivisionTemplates.insert(DivisionType::Cavalry);
    }
    if (country->hasTech("tech_recon")) {
      allowedSupportRegimentTypes.push_back(SupportRegimentType::Recon);
      desiredDivisionTemplates.insert(DivisionType::SupportedInfantry);
    }
    if (country->hasTech("tech_maintenance_company")) {
      allowedSupportRegimentTypes.push_back(SupportRegimentType::Maintenance);
      desiredDivisionTemplates.insert(DivisionType::SupportedInfantry);
    }
    if (country->hasTech("tech_engineers")) {
      allowedSupportRegimentTypes.push_back(SupportRegimentType::Engineer);
      desiredDivisionTemplates.insert(DivisionType::SupportedInfantry);
    }
    if (country->hasTech("gw_artillery")) {
      allowedRegimentTypes.push_back(CombatRegimentType::Artillery);
      allowedSupportRegimentTypes.push_back(SupportRegimentType::Artillery);
      desiredDivisionTemplates.insert(DivisionType::HeavyArtilleryInfantry);
      if (country->hasTech("tech_trucks")) {
        allowedRegimentTypes.push_back(CombatRegimentType::MotorizedArtillery);
      }
    }
    if (country->hasTech("interwar_antiair")) {
      allowedRegimentTypes.push_back(CombatRegimentType::AntiAir);
      allowedSupportRegimentTypes.push_back(SupportRegimentType::AntiAir);
      if (country->hasTech("tech_trucks")) {
        allowedRegimentTypes.push_back(CombatRegimentType::MotorizedAntiAir);
      }
    }
    if (country->hasTech("interwar_antitank")) {
      allowedRegimentTypes.push_back(CombatRegimentType::AntiTank);
      allowedSupportRegimentTypes.push_back(SupportRegimentType::AntiTank);
      if (country->hasTech("tech_trucks")) {
        allowedRegimentTypes.push_back(CombatRegimentType::MotorizedAntiTank);
      }
    }
    if (country->hasTech("motorised_infantry")) {
      allowedRegimentTypes.push_back(CombatRegimentType::Motorized);
      desiredDivisionTemplates.insert(DivisionType::Motorized);
      // if we have CombatRegimentType::MotorizedAntiTank or AntiAir or
      // Artillery we want a supportedMotorized
      if (country->hasTech("tech_recon") ||
          country->hasTech("tech_engineers") ||
          country->hasTech("gw_artillery")) {
        desiredDivisionTemplates.insert(DivisionType::SupportedMotorized);
      }
      // with artillery available, lets get a motorized artillery division
      if (country->hasTech("gw_artillery")) {
        desiredDivisionTemplates.insert(DivisionType::HeavyArtilleryMotorized);
      }
    }
    if (country->hasTech("basic_light_tank_chassis")) {
      allowedRegimentTypes.push_back(CombatRegimentType::LightArmor);
    }

    // now we generate the division templates
    country->divisionTemplates =
        createDivisionTemplates(desiredDivisionTemplates, allowedRegimentTypes,
                                allowedSupportRegimentTypes);

    // now find names for the divisions
    for (auto &division : country->divisionTemplates) {
    }

    // at the end, we evaluate which of these templates is used with which
    // share, as a developed country for example will NOT use irregular
    // infantry in its army, but a minor power might. the more developed we
    // are, the more likely we are to use the more expensive divisions
    auto &development = country->averageDevelopment;
    for (auto &division : country->divisionTemplates) {
      if (division.type == DivisionType::Militia) {
        division.armyShare = 0.35 - development;
      } else if (division.type == DivisionType::Cavalry) {
        division.armyShare = 0.2 - (development - 0.3);
      } else if (division.type == DivisionType::Infantry) {
        division.armyShare = 0.2 - (development - 0.3);
      } else if (division.type == DivisionType::SupportedInfantry) {
        division.armyShare = 0.2 - (development - 0.4);
      } else if (division.type == DivisionType::HeavyArtilleryInfantry) {
        division.armyShare = 0.2 - (development - 0.4);
      } else if (division.type == DivisionType::Motorized) {
        division.armyShare = 0.1 - (development - 0.5);
      } else if (division.type == DivisionType::SupportedMotorized) {
        division.armyShare = 0.1 - (development - 0.6);
      } else if (division.type == DivisionType::HeavyArtilleryMotorized) {
        division.armyShare = 0.1 - (development - 0.6);
      } else if (division.type == DivisionType::Armor) {
        division.armyShare = 0.1 - (development - 0.7);
      }
    }
    // now normalise the shares so we get a sum of 1
    double sum = 0.0;
    for (auto &division : country->divisionTemplates) {
      sum += division.armyShare;
    }
    for (auto &division : country->divisionTemplates) {
      division.armyShare /= sum;
    }
    // now we can generate the divisions. Each typeshare is multiplied with
    // the totalArmyStrength, and then we generate the divisions until their
    // cost reaches the typeshare
    if (country->ownedRegions.size()) {
      for (auto &divisionTemplate : country->divisionTemplates) {
        auto divisionMaxCost =
            divisionTemplate.armyShare * country->totalArmyStrength;
        int count = 1;
        while ((divisionMaxCost -= divisionTemplate.cost) > 0) {
          Division division;
          division.divisionTemplate = divisionTemplate;
          division.name = std::to_string(count++);
          if (count % 10 == 1)
            division.name += "st";
          else if (count % 10 == 2)
            division.name += "nd";
          else if (count % 10 == 3)
            division.name += "rd";
          else
            division.name += "th";
          division.location = Fwg::Utils::selectRandom(country->ownedProvinces);
          division.name += " '" + division.location->name + "' " +
                           division.divisionTemplate.name;
          division.startingEquipmentFactor =
              std::min<double>(0.7 + country->averageDevelopment * 0.3 +
                                   RandNum::getRandom(0.0, 0.2),
                               1.0);
          division.startingExperienceFactor = RandNum::getRandom(0.0, 1.0);
          country->divisions.push_back(division);
        }
      }
    }
  }
}

void Generator::generateCountryNavies() {
  // map from the ShipClassType to the required NavalHullType
  std::map<ShipClassType, NavalHullType> shipClassToHullType = {
      {ShipClassType::Destroyer, NavalHullType::Light},
      {ShipClassType::LightCruiser, NavalHullType::Cruiser},
      {ShipClassType::HeavyCruiser, NavalHullType::Cruiser},
      {ShipClassType::BattleCruiser, NavalHullType::Heavy},
      {ShipClassType::BattleShip, NavalHullType::Heavy},
      {ShipClassType::Carrier, NavalHullType::Carrier},
      {ShipClassType::Submarine, NavalHullType::Submarine}};

  // navy:
  std::map<ShipClassType, int> tonnages = {
      {ShipClassType::Destroyer, 2000},
      {ShipClassType::LightCruiser, 5000},
      {ShipClassType::HeavyCruiser, 10000},
      {ShipClassType::BattleCruiser, 30000},
      {ShipClassType::BattleShip, 30000},
      {ShipClassType::Carrier, 20000},
      {ShipClassType::Submarine, 1500}};
  // vector of all ShipClassTypes
  std::vector<ShipClassType> shipClassTypes = {
      ShipClassType::Destroyer,    ShipClassType::LightCruiser,
      ShipClassType::HeavyCruiser, ShipClassType::BattleCruiser,
      ShipClassType::BattleShip,   ShipClassType::Carrier,
      ShipClassType::Submarine};
  // vector of all ShipClassEras
  std::vector<TechEra> shipEras = {TechEra::Interwar, TechEra::Buildup};

  std::map<ShipClassType, std::string> ShipClassTypeDefinitions = {
      {ShipClassType::Destroyer, "destroyer"},
      {ShipClassType::LightCruiser, "light_cruiser"},
      {ShipClassType::HeavyCruiser, "heavy_cruiser"},
      {ShipClassType::BattleCruiser, "battle_cruiser"},
      {ShipClassType::BattleShip, "battleship"},
      {ShipClassType::Carrier, "carrier"},
      {ShipClassType::Submarine, "submarine"}};
  // for mtg
  std::map<ShipClassType, std::string> shipHullDefinitions = {
      {ShipClassType::Destroyer, "ship_hull_light"},
      {ShipClassType::LightCruiser, "ship_hull_cruiser"},
      {ShipClassType::HeavyCruiser, "ship_hull_cruiser"},
      {ShipClassType::BattleCruiser, "ship_hull_heavy"},
      {ShipClassType::BattleShip, "ship_hull_heavy"},
      {ShipClassType::Carrier, "ship_hull_carrier"},
      {ShipClassType::Submarine, "ship_hull_submarine"}};

  for (auto &country : hoi4Countries) {
    if (!country->ownedRegions.size())
      continue;
    // first generate the different ship classes, in each ShipclassType, we
    // have three: Interwar, Buildup
    for (const auto &shipclassType : shipClassTypes) {
      country->shipClasses.insert({shipclassType, {}});
      auto availableHullTypeEras =
          country->hullTech[shipClassToHullType[shipclassType]];

      for (const auto &shipera : shipEras) {
        // check if we have the required tech level for this ship class
        if (std::find(availableHullTypeEras.begin(),
                      availableHullTypeEras.end(),
                      shipera) == availableHullTypeEras.end()) {
          continue;
        }

        ShipClass shipClass;
        shipClass.type = shipclassType;
        shipClass.era = shipera;
        shipClass.name =
            Fwg::Utils::selectRandom(
                country->getPrimaryCulture()->language->shipNames) +
            " Class";
        shipClass.vanillaShipType =
            ShipClassTypeDefinitions[shipclassType] +
            (shipClass.era == TechEra::Interwar ? "_1" : "_2");

        shipClass.mtgHullname =
            shipHullDefinitions[shipclassType] +
            (shipClass.era == TechEra::Interwar ? "_1" : "_2");
        shipClass.tonnage = tonnages[shipclassType];

        addShipClassModules(shipClass, country->navyTechs,
                            country->infantryTechs);
        country->shipClasses.at(shipClass.type).push_back(shipClass);
      }
    }
    // we only set the designs if we're landlocked
    if (country->landlocked) {
      continue;
    }

    // determine the total tonnage by taking the naval focus times the
    // countries naval industry
    auto totalTonnage = country->navalFocus * country->dockyards * 100.0;

    // calculate amount of convoys based on tonnage
    country->convoyAmount = totalTonnage / 500;

    // now determine the composition of the navy, first the share of carriers,
    // battleships and screens
    auto carrierShare = 0.0;
    auto battleshipShare = 0.0;
    auto screenShare = 0.0;
    // carriers are only built by major powers
    if (country->rank == Arda::Rank::GreatPower) {
      carrierShare = 0.1;
      battleshipShare = 0.2;
      screenShare = 0.7;
    } else if (country->rank == Arda::Rank::SecondaryPower) {
      carrierShare = 0.075;
      battleshipShare = 0.15;
      screenShare = 0.775;
    } else if (country->rank == Arda::Rank::RegionalPower) {
      carrierShare = 0.00;
      battleshipShare = 0.3;
      screenShare = 0.7;
    } else if (country->rank == Arda::Rank::LocalPower) {
      carrierShare = 0.00;
      battleshipShare = 0.2;
      screenShare = 0.8;
    } else {
      carrierShare = 0.0;
      battleshipShare = 0.1;
      screenShare = 0.9;
    }

    // let's evaluate if the carrier tonnage is enough to spawn one carrier
    int carrierTargetTonnage = totalTonnage * carrierShare;
    const std::vector<ShipClass> &carrierClasses =
        country->shipClasses.at(ShipClassType::Carrier);
    if (carrierClasses.size()) {
      auto randomCarrierShipClass = Fwg::Utils::selectRandom(carrierClasses);
      // as long as we have enough tonnage for a carrier, spawn one
      while (carrierTargetTonnage > randomCarrierShipClass.tonnage) {
        // create a carrier ship
        Ship carrier;
        carrier.shipClass = randomCarrierShipClass;
        // push shared pointer to new ship
        country->ships.push_back(std::make_shared<Ship>(carrier));
        carrierTargetTonnage -= randomCarrierShipClass.tonnage;
      }
    }
    int heavyShipTargetTonnage =
        carrierTargetTonnage + totalTonnage * battleshipShare;
    // we randomly select Ship Classes Battleship and Heavy Cruiser
    const std::vector<ShipClass> &battleshipClasses =
        country->shipClasses.at(ShipClassType::BattleShip);
    const std::vector<ShipClass> &battleCruiserClasses =
        country->shipClasses.at(ShipClassType::BattleCruiser);
    const std::vector<ShipClass> &heavyCruiserClasses =
        country->shipClasses.at(ShipClassType::HeavyCruiser);
    if (battleshipClasses.size() || battleCruiserClasses.size() ||
        heavyCruiserClasses.size()) {
      // as long as we have enough tonnage for a heavy ship, spawn one
      while (heavyShipTargetTonnage > 0) {
        // create a heavy ship
        Ship heavyShip;
        if (RandNum::getRandom(0, 2)) {
          if (!heavyCruiserClasses.size())
            continue;
          heavyShip.shipClass = Fwg::Utils::selectRandom(heavyCruiserClasses);
        } else if (RandNum::getRandom(0, 2)) {
          if (!battleCruiserClasses.size())
            continue;
          heavyShip.shipClass = Fwg::Utils::selectRandom(battleCruiserClasses);
        } else {
          if (!battleshipClasses.size())
            continue;
          heavyShip.shipClass = Fwg::Utils::selectRandom(battleshipClasses);
        }
        // push shared pointer to new ship
        country->ships.push_back(std::make_shared<Ship>(heavyShip));
        heavyShipTargetTonnage -= heavyShip.shipClass.tonnage;
      }
    }

    // now we have to distribute the remaining tonnage to screens
    int screenTargetTonnage =
        heavyShipTargetTonnage + totalTonnage * screenShare;
    const std::vector<ShipClass> &destroyerClasses =
        country->shipClasses.at(ShipClassType::Destroyer);
    const std::vector<ShipClass> &lightCruiserClasses =
        country->shipClasses.at(ShipClassType::LightCruiser);
    if (destroyerClasses.size() || lightCruiserClasses.size()) {
      // as long as we have enough tonnage for a screen, spawn one
      while (screenTargetTonnage > 0) {
        // create a screen ship
        Ship screenShip;
        if (RandNum::getRandom(0, 2)) {
          if (!destroyerClasses.size())
            continue;
          screenShip.shipClass = Fwg::Utils::selectRandom(destroyerClasses);
        } else {
          if (!lightCruiserClasses.size())
            continue;
          screenShip.shipClass = Fwg::Utils::selectRandom(lightCruiserClasses);
        }
        // push shared pointer to new ship
        country->ships.push_back(std::make_shared<Ship>(screenShip));
        screenTargetTonnage -= screenShip.shipClass.tonnage;
      }
    }
  }
  // put all ships in one fleet
  for (auto &country : hoi4Countries) {
    // we only set the designs if we're landlocked
    if (country->landlocked) {
      continue;
    }
    std::map<std::string, int> utilisedShipNames;
    Fleet fleet;
    fleet.name = country->name + " Fleet";
    for (auto &ship : country->ships) {
      ship->name = Fwg::Utils::selectRandom(
          country->getPrimaryCulture()->language->shipNames);
      if (utilisedShipNames.find(ship->name) != utilisedShipNames.end()) {
        utilisedShipNames[ship->name]++;
        ship->name += " " + std::to_string(utilisedShipNames[ship->name]);
      } else {
        utilisedShipNames[ship->name] = 1;
      }
      fleet.ships.push_back(ship);
    }
    // find some random port location
    for (auto &region : country->hoi4Regions) {
      for (auto &navalbase : region->navalBases) {
        if (navalbase.second > 0) {
          fleet.startingPort = ardaProvinces.at(navalbase.first);
          break;
        }
      }
    }
    // check if no port was found
    if (fleet.startingPort == nullptr) {
      // just take the capital
      fleet.startingPort = ardaProvinces.at(country->capitalRegionID);
    }

    country->fleets.push_back(fleet);
  }
}

void Generator::generateFocusTrees() {
  Hoi4::FocusGen::evaluateCountryGoals(this->hoi4Countries, this->ardaRegions);
}

Arda::ScenarioPosition
createPosition(Fwg::Position &position, Arda::PositionType type, int typeIndex,
               const std::vector<Fwg::Terrain::LandForm> &landForms) {
  Arda::ScenarioPosition scenarioPos;
  scenarioPos.position = position;
  scenarioPos.position.altitude =
      landForms[scenarioPos.position.weightedCenter].altitude;
  scenarioPos.type = type;
  scenarioPos.typeIndex = typeIndex;
  return scenarioPos;
}
std::vector<Fwg::Areas::NeighbourProvince>
getNeighbourRelations(const std::shared_ptr<Fwg::Areas::Province> &prov,
                      const Fwg::Cfg &cfg, const float &factor) {
  // create the neighbour relations for each of the neighbours
  std::vector<Fwg::Areas::NeighbourProvince> neighbourRelations;
  for (auto &neighbour : prov->neighbours) {
    Fwg::Areas::NeighbourProvince neighbourProv;
    neighbourProv.neighbour = neighbour;
    neighbourProv.cost = 1.0f;
    double angle = 0.0;
    auto positionBetweenProvinces = prov->getPositionToNeighbourProvince(
        neighbour, cfg.width, angle, factor);
    neighbourProv.positionToNeighbour = positionBetweenProvinces;

    neighbourRelations.push_back(neighbourProv);
  }
  return neighbourRelations;
}

void Generator::generatePositions() {
  const auto &landForms = this->terrainData.landForms;
  const auto &cfg = Fwg::Cfg::Values();
  for (auto &gameProv : ardaProvinces) {
    Fwg::Position position;
    if (gameProv->victoryPoint) {
      position = gameProv->victoryPoint->position;
    } else {
      position = gameProv->baseProvince->position;
    }
    gameProv->positions.push_back(
        createPosition(position, Arda::PositionType::VictoryPoint, 38, landForms));

    // now we get neighbour relations for a province, but in a very short
    // distance to the centre.
    std::vector<Fwg::Areas::NeighbourProvince> neighbourRelations;
    const auto &prov = gameProv->baseProvince;
    neighbourRelations = getNeighbourRelations(prov, cfg, 0.2f);

    //  We use those for standstill, standstill RG, defending, attacking
    auto allowedSize = neighbourRelations.size() - 1;
    gameProv->positions.push_back(createPosition(
        neighbourRelations[std::min<int>(0, allowedSize)].positionToNeighbour,
        Arda::PositionType::Standstill, 0, landForms));
    gameProv->positions.push_back(createPosition(
        neighbourRelations[std::min<int>(1, allowedSize)].positionToNeighbour,
        Arda::PositionType::StandstillRG, 21, landForms));
    gameProv->positions.push_back(createPosition(
        neighbourRelations[std::min<int>(2, allowedSize)].positionToNeighbour,
        Arda::PositionType::Defending, 10, landForms));
    gameProv->positions.push_back(createPosition(
        neighbourRelations[std::min<int>(3, allowedSize)].positionToNeighbour,
        Arda::PositionType::Attacking, 9, landForms));

    // for sea we need: standstill, standstill RG, defending, attacking, per
    // neighbour: moving, disembarck (11 + x), moving RG, disembarck RG (30 + x)
    // for land we need: standstill, standstill RG, defending, attacking, per
    // neighbour: moving, moving RG for coastal land we need additionally: ship
    // in port (19), ship in port moving (20)
    // commonality for all: standstill (0), standstill RG (21), defending (10),
    // attacking (9), per neighbour: moving (1 + x), moving RG (22 + x), victory
    // point (38)
    auto sea = gameProv->baseProvince->isSea();

    // evaluate if we need ship in port positions
    if (gameProv->baseProvince->isLand() && gameProv->baseProvince->coastal) {
      // now check if we have a port location
      auto &locations = gameProv->baseProvince->locations;
      // get the port if we have one
      auto portLocation =
          std::find_if(locations.begin(), locations.end(), [](const auto &loc) {
            return loc->type == Fwg::Civilization::LocationType::Port;
          });
      if (portLocation == locations.end()) {
        // no port location, so we take random coastal pixels from the
        // baseProvince
        position = Fwg::Position(
            Fwg::Utils::selectRandom(gameProv->baseProvince->coastalPixels),
            cfg.width);
      } else {
        position = (*portLocation)->position;
      }

      gameProv->positions.push_back(
          createPosition(position, Arda::PositionType::ShipInPort, 19, landForms));
      gameProv->positions.push_back(createPosition(
          position, Arda::PositionType::ShipInPortMoving, 20, landForms));
    }
    neighbourRelations = getNeighbourRelations(prov, cfg, 0.33f);
    // really close to the destination coast
    auto embarkNeighbourRelations = getNeighbourRelations(prov, cfg, 0.9f);
    auto embarkRgNeighbourRelations = getNeighbourRelations(prov, cfg, 0.8f);

    // all need moving, and moving RG. Moving we take from the
    // gameProv->baseProvince->neighbourRelations, while moving RG we take
    // from neighbourRelations, as they are closer to the center of the
    // province
    for (auto counter = 0;
         auto &neighbour : gameProv->baseProvince->neighbourRelations) {
      if (counter > 7) {
        // hoi4 only supports 8 moving positions, so we break here
        break;
      }

      gameProv->positions.push_back(
          createPosition(neighbour->positionToNeighbour,
                         Arda::PositionType::UnitMoving, 1 + counter, landForms));
      gameProv->positions.push_back(
          createPosition(neighbourRelations[counter].positionToNeighbour,
                         Arda::PositionType::UnitMovingRG, 22 + counter, landForms));
      if (sea) {
        // now we add the embark positions, which are the same as the
        // neighbour relations, but with a different type
        gameProv->positions.push_back(createPosition(
            embarkNeighbourRelations[counter].positionToNeighbour,
            Arda::PositionType::UnitDisembarking, 11 + counter, landForms));

        gameProv->positions.push_back(createPosition(
            embarkRgNeighbourRelations[counter].positionToNeighbour,
            Arda::PositionType::UnitDisembarkingRG, 30 + counter, landForms));
        counter++;
      }
    }
    // now sort by typeIndex
    std::sort(gameProv->positions.begin(), gameProv->positions.end(),
        [](const Arda::ScenarioPosition &a, const Arda::ScenarioPosition &b) {
                return a.typeIndex < b.typeIndex;
              });
  }
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
      // auto &hoi4Country = hoi4Countries[entry->tag];
      //  search the corresponding hoi4Country in hoi4COuntries by tag.
      // reinterpret this country as a shared pointer to Hoi4Country
      auto hoi4Country = std::dynamic_pointer_cast<Hoi4Country>(entry);
      Fwg::Utils::Logging::logLine("Strength: ", scores.first, " ",
                                   hoi4Country->fullName, " ",
                                   hoi4Country->ideology, "");
    }
  }
}

void Generator::loadStates() {}

void Generator::distributeVictoryPoints() {
  double baseVPs = 10000;
  double assignedVPs = 0;
  for (auto country : hoi4Countries) {

    if (!country->ownedRegions.size())
      continue;
    auto language = country->getPrimaryCulture()->language;
    for (auto &region : country->hoi4Regions) {
      if (!region->isLand())
        continue;
      region->victoryPointsMap.clear();
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
        Arda::VictoryPoint vp{vps};
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
          region->victoryPointsMap[province.first] =
              std::make_shared<Arda::VictoryPoint>(vp);
          assignedVPs += region->victoryPointsMap[province.first]->amount;
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

void Generator::generateCharacters() {
  std::map<Arda::Ideology, std::vector<std::string>> leaderTraits = {
      {Arda::Ideology::None,
       {"cabinet_crisis", "exiled", "headstrong", "humble",
        "inexperienced_monarch", "socialite_connections",
        "staunch_constitutionalist", "gentle_scholar", "the_statist",
        "the_academic"}},
      {Arda::Ideology::Neutral,
       {"cabinet_crisis", "exiled", "headstrong", "humble",
        "inexperienced_monarch", "socialite_connections",
        "staunch_constitutionalist", "celebrity_junta_leader"}},
      {Arda::Ideology::Fascist,
       {"autocratic_imperialist", "collaborator_king", "generallissimo",
        "inexperienced_imperialist", "spirit_of_genghis", "warmonger",
        "the_young_magnate", "polemarch", "archon_basileus", "autokrator",
        "basileus", "infirm", "celebrity_junta_leader"}},
      {Arda::Ideology::Communist,
       {"political_dancer", "indomitable_perseverance",
        "mastermind_code_cracker", "polemarch", "infirm",
        "reluctant_stalinist"}},
      {Arda::Ideology::Democratic,
       {"british_bulldog", "chamberlain_appeaser", "conservative_grandee",
        "famous_aviator", "first_lady", "rearmer", "staunch_constitutionalist",
        "the_banker", "the_young_magnate", "infirm",
        "liberal_democratic_paragon"}}};

  std::vector<std::string> armyChiefTraits = {
      "army_chief_defensive_",      "army_chief_offensive_",
      "army_chief_drill_",          "army_chief_reform_",
      "army_chief_organizational_", "army_chief_planning_",
      "army_chief_morale_",         "army_chief_maneuver_",
      "army_chief_entrenchment_"};

  std::vector<std::string> airChiefTraits = {
      "air_chief_reform_",         "air_chief_safety_",
      "air_chief_old_guard_",      "air_chief_night_operations_",
      "air_chief_ground_support_", "air_chief_all_weather_"};

  std::vector<std::string> navyChiefTraits = {
      "navy_chief_naval_aviation_",   "navy_chief_decisive_battle_",
      "navy_chief_commerce_raiding_", "navy_chief_old_guard_",
      "navy_chief_reform_",           "navy_chief_maneuver_"};

  std::vector<std::string> highCommandTraits = {"navy_anti_submarine_",
                                                "navy_naval_air_defense_",
                                                "navy_fleet_logistics_",
                                                "navy_amphibious_assault_",
                                                "navy_submarine_",
                                                "navy_capital_ship_",
                                                "navy_screen_",
                                                "navy_carrier_",
                                                "air_air_combat_training_",
                                                "air_naval_strike_",
                                                "air_bomber_interception_",
                                                "air_air_superiority_",
                                                "air_close_air_support_",
                                                "air_strategic_bombing_",
                                                "air_tactical_bombing_",
                                                "air_airborne_",
                                                "air_pilot_training_",
                                                "army_entrenchment_",
                                                "army_armored_",
                                                "army_artillery_",
                                                "army_infantry_",
                                                "army_commando_",
                                                "army_cavalry_",
                                                "army_CombinedArms_",
                                                "army_regrouping_",
                                                "army_concealment_",
                                                "army_logistics_",
                                                "army_radio_intelligence_"};
  std::vector<std::string> theoristTraits = {
      "military_theorist", "naval_theorist", "air_warfare_theorist"};

  for (auto &country : hoi4Countries) {
    if (!country->ownedRegions.size())
      continue;
    // per country, we want to avoid duplicate names
    std::set<std::string> usedNames;
    // we want of every ideology: Neutral, Fascist, Communist, Democratic
    std::vector<Arda::Ideology> ideologies = {Arda::Ideology::Neutral, Arda::Ideology::Fascist,
                                        Arda::Ideology::Communist,
                                        Arda::Ideology::Democratic};

    auto createCharacter = [&](Arda::Type type, Arda::Ideology ideology,
                               const std::vector<std::string> &traits,
                               int count, bool addLevel = false) {
      for (int i = 0; i < count; i++) {
        Arda::Character character;
        character.gender = Arda::Gender::Male;
        do {
          character.name = Fwg::Utils::selectRandom(
              country->getPrimaryCulture()->language->maleNames);
          character.surname = Fwg::Utils::selectRandom(
              country->getPrimaryCulture()->language->surnames);
        } while (usedNames.find(character.name + " " + character.surname) !=
                 usedNames.end());

        usedNames.insert(character.name + " " + character.surname);
        character.ideology = ideology;
        character.type = type;
        if (traits.size()) {
          if (addLevel) {
            int level = RandNum::getRandom(1, 3);
            character.traits.push_back(Fwg::Utils::selectRandom(traits) +
                                       std::to_string(level));
          } else {
            character.traits.push_back(Fwg::Utils::selectRandom(traits));
          }
        }
        country->characters.push_back(character);
      }
    };

    for (const auto &ideology : ideologies) {
      // 1 country leader
      createCharacter(Arda::Type::Leader, ideology, leaderTraits[ideology], 1);

      // 6 Politicians
      createCharacter(Arda::Type::Politician, ideology, leaderTraits[ideology], 6);

      // 4 Command Generals
      createCharacter(Arda::Type::ArmyChief, ideology, armyChiefTraits, 4, true);

      // 2 Command Admirals
      createCharacter(Arda::Type::NavyChief, ideology, navyChiefTraits, 2, true);

      // 2 Airforce Chiefs
      createCharacter(Arda::Type::AirForceChief, ideology, airChiefTraits, 2, true);

      // 6 High Command
      createCharacter(Arda::Type::HighCommand, ideology, highCommandTraits, 6, true);

      // 2 Generals
      createCharacter(Arda::Type::ArmyGeneral, ideology, {}, 0);

      // 2 Admirals
      createCharacter(Arda::Type::FleetAdmiral, ideology, {}, 0);
    }

    // 3 theorists, 1 per trait
    for (int i = 0; i < 3; i++) {
      Arda::Character theorist;
      theorist.gender = Arda::Gender::Male;
      do {
        theorist.name = Fwg::Utils::selectRandom(
            country->getPrimaryCulture()->language->maleNames);
        theorist.surname = Fwg::Utils::selectRandom(
            country->getPrimaryCulture()->language->surnames);
      } while (usedNames.find(theorist.name + " " + theorist.surname) !=
               usedNames.end());

      usedNames.insert(theorist.name + " " + theorist.surname);
      theorist.ideology = Arda::Ideology::Neutral;
      theorist.type = Arda::Type::Theorist;
      theorist.traits.push_back(theoristTraits.at(i));
      country->characters.push_back(theorist);
    }
  }
}

bool Generator::loadRivers(Fwg::Cfg &config,
                           const Fwg::Gfx::Bitmap &riverInput) {
  auto riverCopy = riverInput;
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
  for (auto &pix : riverCopy.imageData) {
    if (colourMapping.find(pix) != colourMapping.end()) {
      pix = colourMapping.at(pix);
    }
  }

  // Call the base class method from FastWorldGenerator, to load the now
  // mapped river input
  return FastWorldGenerator::loadRivers(config, riverCopy);
}
} // namespace Rpx::Hoi4