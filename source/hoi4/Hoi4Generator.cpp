#include "hoi4/Hoi4Generator.h"
#include "areas/SuperRegion.h"
#include "generic/StrategicRegion.h"
#include <limits>
#include <numeric>
using namespace Fwg;
using namespace Fwg::Gfx;
namespace Rpx::Hoi4 {
namespace {} // namespace

Generator::Generator(const std::string &configSubFolder,
                     const boost::property_tree::ptree &rpdConf)
    : Rpx::ModGenerator(configSubFolder, GameType::Hoi4, "hoi4.exe", rpdConf) {
  configureModGen(configSubFolder, Fwg::Cfg::Values().username, rpdConf);
  factories.regionFactory = []() {
    return std::make_shared<Rpx::Hoi4::Region>();
  };

  ardaFactories.countryFactory =
      []() -> std::shared_ptr<Rpx::Hoi4::Hoi4Country> {
    return std::make_shared<Rpx::Hoi4::Hoi4Country>();
  };
}

Generator::~Generator() {}

bool Generator::createPaths() {
  // prepare folder structure
  try {
    // generic cleanup and path creation
    using namespace std::filesystem;
    // GenericModule::createPaths(pathcfg.gameModPath);
    create_directory(pathcfg.gameModPath);
    // map
    remove_all(pathcfg.gameModPath + "/map/");
    remove_all(pathcfg.gameModPath + "/gfx");
    remove_all(pathcfg.gameModPath + "/events/");
    remove_all(pathcfg.gameModPath + "/history");
    remove_all(pathcfg.gameModPath + "/common/");
    remove_all(pathcfg.gameModPath + "/portraits/");
    remove_all(pathcfg.gameModPath + "/localisation/");
    create_directory(pathcfg.gameModPath + "/map/");
    create_directory(pathcfg.gameModPath + "/map/terrain/");
    // gfx
    create_directory(pathcfg.gameModPath + "/gfx/");
    create_directory(pathcfg.gameModPath + "/gfx/flags/");
    // history
    create_directory(pathcfg.gameModPath + "/history/");
    // localisation
    create_directory(pathcfg.gameModPath + "/localisation/");
    // portraits
    create_directory(pathcfg.gameModPath + "/portraits/");
    // common
    create_directory(pathcfg.gameModPath + "/common/");
    // map
    create_directory(pathcfg.gameModPath + "/map/strategicregions/");
    // gfx
    create_directory(pathcfg.gameModPath + "/gfx/flags/small/");
    create_directory(pathcfg.gameModPath + "/gfx/flags/medium/");
    // history
    create_directory(pathcfg.gameModPath + "/history/units/");
    create_directory(pathcfg.gameModPath + "/history/states/");
    create_directory(pathcfg.gameModPath + "/history/countries/");
    // localisation
    create_directory(pathcfg.gameModPath + "/localisation/braz_por/");
    create_directory(pathcfg.gameModPath + "/localisation/english/");
    create_directory(pathcfg.gameModPath + "/localisation/french/");
    create_directory(pathcfg.gameModPath + "/localisation/german/");
    create_directory(pathcfg.gameModPath + "/localisation/japanese/");
    create_directory(pathcfg.gameModPath + "/localisation/korean/");
    create_directory(pathcfg.gameModPath + "/localisation/polish/");
    create_directory(pathcfg.gameModPath + "/localisation/russian/");
    create_directory(pathcfg.gameModPath + "/localisation/simp_chinese/");
    create_directory(pathcfg.gameModPath + "/localisation/spanish/");
    // common
    // create_directory(pathcfg.gameModPath + "/common/national_focus/");
    create_directory(pathcfg.gameModPath + "/common/countries/");
    create_directory(pathcfg.gameModPath + "/common/characters/");
    create_directory(pathcfg.gameModPath + "/common/decisions/");
    create_directory(pathcfg.gameModPath + "/common/ideas/");
    create_directory(pathcfg.gameModPath + "/common/bookmarks/");
    create_directory(pathcfg.gameModPath + "/common/national_focus/");
    create_directory(pathcfg.gameModPath + "/common/country_tags/");
    create_directory(pathcfg.gameModPath + "/common/names/");
    create_directory(pathcfg.gameModPath + "/common/scripted_triggers/");
    create_directory(pathcfg.gameModPath + "/tutorial/");
    return true;
  } catch (std::exception &e) {
    std::string error =
        "Configured paths seem to be messed up, check Hoi4Module.json\n";
    error += "You can try fixing it yourself. Error is:\n ";
    error += e.what();
    Fwg::Utils::Logging::logLine(error);
    throw(std::runtime_error(error.c_str()));
    return false;
  }
}

void Generator::configureModGen(const std::string &configSubFolder,
                                const std::string &username,
                                const boost::property_tree::ptree &rpdConf) {
  Fwg::Utils::Logging::logLine("Reading Hoi4 Config");
  Rpx::Utils::configurePaths(username, "Hearts of Iron IV", rpdConf,
                             this->pathcfg);

  auto &config = Cfg::Values();
  namespace pt = boost::property_tree;
  pt::ptree hoi4Conf;
  try {
    // Read the basic settings
    std::ifstream f(configSubFolder + "/Hearts of Iron IVModule.json");
    std::stringstream buffer;
    if (!f.good())
      Fwg::Utils::Logging::logLine("Config could not be loaded");
    buffer << f.rdbuf();
    Fwg::Parsing::replaceInStringStream(buffer, "//", "/");

    pt::read_json(buffer, hoi4Conf);
  } catch (std::exception &e) {
    Fwg::Utils::Logging::logLine("Incorrect config \"RandomParadox.json\"");
    Fwg::Utils::Logging::logLine("You can try fixing it yourself. Error is: ",
                                 e.what());
    Fwg::Utils::Logging::logLine(
        "Otherwise try running it through a json validator");
    system("pause");
  }
  // default values taken from base game

  ;
  this->modConfig.resources = {
      {"aluminium",
       {hoi4Conf.get<double>("hoi4.aluminiumFactor"), 1169.0, 0.3}},
      {"coal", {hoi4Conf.get<double>("hoi4.chromiumFactor"), 1250.0, 0.2}},
      {"chromium", {hoi4Conf.get<double>("hoi4.chromiumFactor"), 1250.0, 0.2}},
      {"oil", {hoi4Conf.get<double>("hoi4.oilFactor"), 1220.0, 0.1}},
      {"rubber", {hoi4Conf.get<double>("hoi4.rubberFactor"), 1029.0, 0.1}},
      {"steel", {hoi4Conf.get<double>("hoi4.steelFactor"), 2562.0, 0.5}},
      {"tungsten", {hoi4Conf.get<double>("hoi4.tungstenFactor"), 1188.0, 0.2}}};
  this->modConfig.weatherChances = {
      {"baseLightRainChance", hoi4Conf.get<double>("hoi4.baseLightRainChance")},
      {"baseHeavyRainChance", hoi4Conf.get<double>("hoi4.baseHeavyRainChance")},
      {"baseMudChance", hoi4Conf.get<double>("hoi4.baseMudChance")},
      {"baseBlizzardChance", hoi4Conf.get<double>("hoi4.baseBlizzardChance")},
      {"baseSandstormChance", hoi4Conf.get<double>("hoi4.baseSandstormChance")},
      {"baseSnowChance", hoi4Conf.get<double>("hoi4.baseSnowChance")}};
  this->ardaConfig.worldPopulationFactor =
      hoi4Conf.get<double>("scenario.worldPopulationFactor");
  this->ardaConfig.worldIndustryFactor =
      hoi4Conf.get<double>("scenario.industryFactor");
  this->ardaConfig.resourceFactor = hoi4Conf.get<double>("hoi4.resourceFactor");

  // settings for scenGen

  //  passed to generic Scenariohoi4Gen
  this->ardaConfig.numCountries = hoi4Conf.get<int>("scenario.numCountries");
  // force defaults for the game, if not set otherwise
  if (config.targetLandRegionAmount == 0 && config.autoLandRegionParams)
    config.targetLandRegionAmount = 640;
  // force defaults for the game, if not set otherwise
  if (config.targetSeaRegionAmount == 0 && config.autoSeaRegionParams)
    config.targetSeaRegionAmount = 160;
  config.forceResolutionBase = true;
  config.resolutionBase = 256;
  config.maxImageArea = 10240 * 1280;
  config.autoSplitProvinces = false;
  ardaConfig.locationConfig.miningPerRegion = 0;
  ardaConfig.locationConfig.forestryPerRegion = 0;
  ardaConfig.locationConfig.citiesPerRegion = 2;
  ardaConfig.locationConfig.portsPerRegion = 1;
  ardaConfig.locationConfig.agriculturePerRegion = 3;
  ardaConfig.locationConfig.agricultureFactor = 0.9;
  ardaConfig.locationConfig.urbanFactor = 1.0;
  this->ardaConfig.generationAge = Arda::Utils::GenerationAge::WorldWar;
  ardaConfig.calculateTargetWorldPopulation();
  ardaConfig.calculateTargetWorldGdp();
  // check if config settings are fine
  config.sanityCheck();
}

void Generator::mapRegions() {
  Fwg::Utils::Logging::logLine("Mapping Regions");
  ardaRegions.clear();
  modData.hoi4States.clear();
  stats.militaryIndustry = 0;
  stats.civilianIndustry = 0;
  stats.navalIndustry = 0;
  stats.totalWorldIndustry = 0;
  modData.statesInitialised = false;
  for (auto &region : this->areaData.regions) {
    std::sort(region->provinces.begin(), region->provinces.end(),
              [](const std::shared_ptr<Fwg::Areas::Province> a,
                 const std::shared_ptr<Fwg::Areas::Province> b) {
                return (*a < *b);
              });
    auto ardaRegion = std::dynamic_pointer_cast<Rpx::Hoi4::Region>(region);
    assert(dynamic_cast<Rpx::Hoi4::Region *>(ardaRegion.get()) != nullptr);
    if (!ardaRegion) {
      Fwg::Utils::Logging::logLine("SEVERE: Bad cast for region ID=",
                                   region->ID);
      continue;
    }
    if (ardaRegion->neighbours.size() == 0)
      continue;
    // generate random name for region
    ardaRegion->name = "";
    ardaRegion->identifier = "STATE_" + std::to_string(region->ID + 1);
    ardaRegion->ardaProvinces.clear();
    for (auto &province : ardaRegion->provinces) {
      if (province->ID >= 0 && province->ID < ardaProvinces.size() &&
          ardaProvinces[province->ID])
        ardaRegion->ardaProvinces.push_back(ardaProvinces[province->ID]);
      else {
        Fwg::Utils::Logging::logLine("Invalid province ID ", province->ID,
                                     " in region ID ", ardaRegion->ID);
      }
    }
    // save game region to generic module container and to hoi4 specific
    // container
    ardaRegions.push_back(ardaRegion);
    modData.hoi4States.push_back(ardaRegion);
  }

  for (size_t i = 0; i < ardaRegions.size(); ++i) {
    if (!ardaRegions[i]) {
      Fwg::Utils::Logging::logLine("SEVERE: ardaRegions[", i, "] is null!");
      continue;
    }
  }

  // sort by Arda::ArdaProvince ID
  // std::sort(ardaRegions.begin(), ardaRegions.end(),
  //          [](auto l, auto r) { return *l < *r; });
  // check if we have the same amount of ardaProvinces as FastWorldGen provinces
  if (ardaProvinces.size() != this->areaData.provinces.size())
    throw(std::runtime_error("Fatal: Lost provinces, terminating"));
  if (ardaRegions.size() != this->areaData.regions.size())
    throw(std::runtime_error("Fatal: Lost regions, terminating"));
  for (const auto &ardaRegion : ardaRegions) {
    if (ardaRegion->ID > ardaRegions.size()) {
      throw(std::runtime_error("Fatal: Invalid region IDs, terminating"));
    }
  }
  applyRegionInput();
}

Fwg::Gfx::Image Generator::mapTerrain() {
  Image typeMap = ArdaGen::mapTerrain();
  const auto &config = Fwg::Cfg::Values();
  auto &colours = config.colours;
  auto &climateColours = config.climateConfig.climateColours;
  auto &elevationColours = config.terrainConfig.elevationColours;
  auto &topographyOverlayColours = config.topographyOverlayColours;
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
        int marshPixels = this->ardaData.civLayer.countOfTypeInRange(
            baseProv->pixels, Arda::Civilization::TopographyType::MARSH);
        int cityPixels = this->ardaData.civLayer.countOfTypeInRange(
            baseProv->pixels, Arda::Civilization::TopographyType::CITY);

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
        if (cityPixels > baseProv->pixels.size() / 4) {
          gameProv->terrainType = "urban";
          for (auto &pix : baseProv->pixels) {
            typeMap.setColourAtIndex(pix, topographyOverlayColours.at("urban"));
          }
        } else if (dominantTerrain == Fwg::Terrain::LandformId::MOUNTAINS ||
                   dominantTerrain == Fwg::Terrain::LandformId::PEAKS) {
          gameProv->terrainType = "mountain";
          for (auto &pix : baseProv->pixels) {
            typeMap.setColourAtIndex(pix, elevationColours.at("mountains"));
          }
        } else if (dominantTerrain == Fwg::Terrain::LandformId::HILLS) {
          gameProv->terrainType = "hills";
          for (auto &pix : baseProv->pixels) {
            typeMap.setColourAtIndex(pix, elevationColours.at("hills"));
          }
        } else if (marshPixels > baseProv->pixels.size() / 2) {
          gameProv->terrainType = "marsh";
          for (auto &pix : baseProv->pixels) {
            typeMap.setColourAtIndex(pix, topographyOverlayColours.at("marsh"));
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
  if (config.debugLevel > 5) {
    Png::save(typeMap, Fwg::Cfg::Values().mapsPath + "debug/typeMap.png");
  }
  return typeMap;
}

void Generator::mapCountries() {
  modData.hoi4Countries.clear();
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
    // Hoi4Country hC(*country.second, this->modData.hoi4States);
    // push back cast to hoi4Country
    // modData.hoi4Countries.push_back(
    //    std::make_shared<Hoi4Country>(country.second, modData.hoi4States));
    // Attempt to cast the shared pointer to Hoi4Country
    auto hoi4Country = std::dynamic_pointer_cast<Hoi4Country>(country.second);
    if (hoi4Country) {
      hoi4Country->hoi4Regions.clear();
      // Successfully casted, add to modData.hoi4Countries
      modData.hoi4Countries.push_back(hoi4Country);
      // now for all ownedRegions, find the equivalent in Hoi4Regions
      for (auto &region : country.second->ownedRegions) {
        for (auto &hoi4Region : modData.hoi4States) {
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
  for (auto &country : modData.hoi4Countries) {
    std::vector<std::shared_ptr<Hoi4Country>> neighboursTemp;
    for (auto &neighbour : country->neighbourCountries) {
      if (neighbour) {
        for (auto &hoi4Country : modData.hoi4Countries) {
          if (neighbour->ID == hoi4Country->ID) {
            neighboursTemp.push_back(hoi4Country);
          }
        }
      }
    }

    country->neighbourCountries.clear();
    for (auto &neighbour : neighboursTemp) {
      country->neighbourCountries.insert(neighbour);
    }
  }
  // std::sort(modData.hoi4States.begin(), modData.hoi4States.end(),
  //           [](auto l, auto r) { return *l < *r; });
}

void Generator::generateStateSpecifics() {
  Rpx::Hoi4::generateStateSpecifics(modConfig, modData, stats, ardaConfig,
                                    areaData, terrainData, ardaRegions,
                                    typeMap);
}

void Generator::generateStateResources() {
  Rpx::Hoi4::generateStateResources(modConfig, modData, stats, ardaConfig,
                                    areaData, climateData, shared_from_this());
}

void Generator::generateSoftCountryDetails() {
  Fwg::Utils::Logging::logLine("HOI4: Electing Tyrants");
  const std::vector<Arda::Utils::Ideology> ideologies{
      Arda::Utils::Ideology::FASCISM, Arda::Utils::Ideology::DEMOCRATIC,
      Arda::Utils::Ideology::COMMUNISM, Arda::Utils::Ideology::NEUTRALITY};
  for (auto &country : modData.hoi4Countries) {
    if (!country->ownedRegions.size())
      continue;
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

    if (country->ideology == Arda::Utils::Ideology::NONE) {
      Fwg::Utils::Logging::logLine("Unassigned country ideology");
    }

    // allow or forbid elections
    if (country->ideology == Arda::Utils::Ideology::DEMOCRATIC)
      country->allowElections = 1;
    else if (country->ideology == Arda::Utils::Ideology::NEUTRALITY)
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
    if (country->ideology == Arda::Utils::Ideology::FASCISM ||
        country->ideology == Arda::Utils::Ideology::COMMUNISM) {
      country->warSupport = RandNum::getRandom(40, 80);
    } else {
      country->warSupport = RandNum::getRandom(20, 60);
    }

    assignStartingLaws(*country);
  }
  // now that politics are settled, AND hard data has been generated, we can
  // balance the great powers and factions
  balanceGreatPowers();
  generateAndBalanceFactions();
}

void Generator::generateHardCountrySpecifics() {
  Fwg::Utils::Logging::logLine("HOI4: Choosing uniforms");

  Fwg::Utils::Randomisation::resetRandomisation();
  const std::vector<std::string> caucasianGfxCultures{
      "western_european", "eastern_european", "commonwealth"};

  for (auto &country : modData.hoi4Countries) {
    if (!country->ownedRegions.size())
      continue;
    // clear some info from all owned regions
    for (auto &region : country->hoi4Regions) {
      region->airBase = nullptr;
    }
    // refresh the provinces
    country->evaluateProvinces();
    auto primaryCulture = country->getPrimaryCulture();
    if (primaryCulture == nullptr) {
      country->gfxCulture = "asian";
    } else {
      switch (primaryCulture->visualType) {
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
        country->gfxCulture =
            Fwg::Utils::Random::selectRandom(caucasianGfxCultures);
        break;
      case Arda::VisualType::SOUTH_AMERICAN:
        country->gfxCulture = "southamerican";
        break;
      }
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
        std::round(3.0 + 2.0 * country->technologyLevel + rankModifier);

    // now get the full name of the country
    country->fullName = NameGeneration::modifyWithIdeology(
        country->ideology, country->name, country->adjective, nData);

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
  }
  generateTechLevels(modData);
  generateArmorVariants(modConfig, modData, stats);
  generateCountryUnits(modConfig, modData, stats);
  generateCountryNavies(modConfig, modData, stats, ardaProvinces);
  generateAirVariants(modConfig, modData, stats);
  generateCharacters(modData);
  distributeVictoryPoints(modData, stats, ardaProvinces);
}

void Generator::deriveCountrySpecificsFromSimulation() {
  Fwg::Utils::Logging::logLine("HOI4: Choosing uniforms and electing Tyrants");
  generateHardCountrySpecifics();
  // Soft follows hard, as soft includes balancing and politics.
  generateSoftCountryDetails();

  for (auto &country : modData.hoi4Countries) {
    if (country->ownedRegions.empty())
      continue;

    const auto polity =
        ardaData.simulationExport.polities.find(country->polityID);
    if (polity == ardaData.simulationExport.polities.end())
      continue;

    const auto &simulationPolity = polity->second.polity;
    setSimulationParties(*country, polity->second);
    country->ideology = ideologyFromGovernment(simulationPolity.governmentForm);
    // allow or forbid elections
    if (country->ideology == Arda::Utils::Ideology::DEMOCRATIC)
      country->allowElections = 1;
    else if (country->ideology == Arda::Utils::Ideology::NEUTRALITY)
      country->allowElections = RandNum::getRandom(0, 1);
    else
      country->allowElections = 0;
    country->stability = static_cast<int>(
        std::lround(std::clamp(simulationPolity.stability, 0.0, 1.0) * 100.0));
    country->warSupport = static_cast<int>(std::lround(
        std::clamp(3.0 * simulationPolity.militaryInfluence, 0.0, 1.0) *
        100.0));
    assignStartingLaws(*country);
    country->fullName = NameGeneration::modifyWithIdeology(
        country->ideology, country->name, country->adjective, nData);
  }
}

void Generator::balanceGreatPowers() {
  Fwg::Utils::Logging::logLine("HOI4: Balancing factions");

  // let's start with faction leaders. Only great powers can have starting
  // factions
  auto greatPowers = ardaData.countriesByRank.at(Arda::Rank::GreatPower);

  std::vector<std::shared_ptr<Rpx::Hoi4::Hoi4Country>> hoi4GreatPowers;
  for (auto &greatPower : greatPowers) {
    if (auto gpHoi4 =
            std::dynamic_pointer_cast<Rpx::Hoi4::Hoi4Country>(greatPower)) {
      hoi4GreatPowers.push_back(gpHoi4);
    }
  }

  // ideology -> great power countries
  modData.greatPowerIdeologyMap = {{Arda::Utils::Ideology::FASCISM, {}},
                                   {Arda::Utils::Ideology::DEMOCRATIC, {}},
                                   {Arda::Utils::Ideology::COMMUNISM, {}},
                                   {Arda::Utils::Ideology::NEUTRALITY, {}}};

  for (auto &greatPower : hoi4GreatPowers) {
    modData.greatPowerIdeologyMap.at(greatPower->ideology)
        .push_back(greatPower);
  }

  // determine which ideologies are missing among great powers
  std::vector<Arda::Utils::Ideology> missingIdeologies;
  for (const auto &[ideology, countries] : modData.greatPowerIdeologyMap) {
    if (countries.empty()) {
      missingIdeologies.push_back(ideology);
    }
  }

  // if one or more ideologies are missing, we need to flip countries
  for (auto missingIdeology : missingIdeologies) {
    // find the ideology with the most great powers
    Arda::Utils::Ideology sourceIdeology = Arda::Utils::Ideology::NEUTRALITY;
    size_t maxSize = 0;

    for (const auto &[ideology, countries] : modData.greatPowerIdeologyMap) {
      if (countries.size() > maxSize) {
        maxSize = countries.size();
        sourceIdeology = ideology;
      }
    }

    auto &sourceCountries = modData.greatPowerIdeologyMap.at(sourceIdeology);
    if (sourceCountries.empty()) {
      // should not happen, but fail safely
      continue;
    }

    // select a random country to flip
    auto chosenCountry = Fwg::Utils::Random::selectRandom(sourceCountries);

    // remove from source ideology vector
    sourceCountries.erase(std::remove(sourceCountries.begin(),
                                      sourceCountries.end(), chosenCountry),
                          sourceCountries.end());

    // flip ideology
    chosenCountry->ideology = missingIdeology;

    // add to missing ideology vector
    modData.greatPowerIdeologyMap.at(missingIdeology).push_back(chosenCountry);
  }
}

void Generator::generateAndBalanceFactions() {
  Fwg::Utils::Logging::logLine("HOI4: Balancing factions");

  // now create one faction per ideology (leader selection only)
  for (const auto &[ideology, countries] : modData.greatPowerIdeologyMap) {
    if (countries.empty()) {
      continue;
    }

    auto factionLeader = Fwg::Utils::Random::selectRandom(countries);
    Faction faction;
    faction.name = "Unassigned";
    faction.ideology = factionLeader->ideology;
    faction.factionLeader = factionLeader->tag;
    faction.memberTags.push_back(faction.factionLeader);
    faction.faction_template = "faction_template_generic";
    switch (faction.ideology) {
    case Arda::Utils::Ideology::FASCISM: {
      faction.faction_template =
          Fwg::Utils::Random::selectRandom(std::vector<std::string>{
              "faction_template_generic_dominance",
              "faction_template_regional_anti_communist",
              "faction_template_regional_anti_democratic"});
      break;
    }
    case Arda::Utils::Ideology::DEMOCRATIC: {
      faction.faction_template = Fwg::Utils::Random::selectRandom(
          std::vector<std::string>{"faction_template_defensive_democratic",
                                   "faction_template_industrial_focus"});
      break;
    }
    case Arda::Utils::Ideology::NEUTRALITY: {
      faction.faction_template =
          Fwg::Utils::Random::selectRandom(std::vector<std::string>{
              "faction_template_generic_dominance",
              "faction_template_anti_communist",
              "faction_template_anti_fascist",
              "faction_template_regional_anti_communist",
              "faction_template_regional_anti_democratic"});
      break;
    }
    case Arda::Utils::Ideology::COMMUNISM: {
      faction.faction_template = Fwg::Utils::Random::selectRandom(
          std::vector<std::string>{"faction_template_generic_dominance",
                                   "faction_template_anti_fascist",
                                   "faction_template_world_revolution"});

      break;
    }
    }
    auto ptrFaction = std::make_shared<Faction>(faction);
    factionLeader->faction = ptrFaction;
    modData.factions.push_back(ptrFaction);
  }

  // also track overall amount of ideologies (all countries, not just great
  // powers)
  std::map<Arda::Utils::Ideology,
           std::vector<std::shared_ptr<Rpx::Hoi4::Hoi4Country>>>
      genericIdeologyMap = {{Arda::Utils::Ideology::FASCISM, {}},
                            {Arda::Utils::Ideology::DEMOCRATIC, {}},
                            {Arda::Utils::Ideology::COMMUNISM, {}},
                            {Arda::Utils::Ideology::NEUTRALITY, {}}};

  for (const auto &[rank, countries] : ardaData.countriesByRank) {
    for (const auto &country : countries) {
      if (auto hoi4Country =
              std::dynamic_pointer_cast<Rpx::Hoi4::Hoi4Country>(country)) {
        genericIdeologyMap.at(hoi4Country->ideology).push_back(hoi4Country);
      }
    }
  }
}

void Generator::generateWeather() {
  for (auto &superRegion : superRegions) {
    auto stratRegion = std::dynamic_pointer_cast<StrategicRegion>(superRegion);
    // for every month, gather the averages from all regions and their provinces
    for (auto i = 0; i < 12; i++) {
      double averageTemperature = 0.0;
      double averageDeviation = 0.0;
      double averagePrecipitation = 0.0;
      int provinceAmount = 0;
      for (auto &reg : stratRegion->ardaRegions) {
        for (auto &prov : reg->ardaProvinces) {
          averageDeviation += prov->weatherMonths[i][0];
          averageTemperature += prov->weatherMonths[i][1];
          averagePrecipitation += prov->weatherMonths[i][2];
        }
        provinceAmount += (int)reg->ardaProvinces.size();
      }
      double divisor = provinceAmount;
      averageDeviation /= divisor;
      averageTemperature /= divisor;
      averagePrecipitation /= divisor;
      // now save monthly data, 0, 1, 2
      stratRegion->weatherMonths.push_back(
          {averageDeviation, averageTemperature, averagePrecipitation});
      // referenceTemperature low, 3
      stratRegion->weatherMonths[i].push_back(
          Cfg::Values().minimumDegCelcius +
          averageTemperature * Cfg::Values().temperatureRange);
      // tempHigh, 4
      stratRegion->weatherMonths[i].push_back(
          Cfg::Values().minimumDegCelcius +
          averageTemperature * Cfg::Values().temperatureRange +
          averageDeviation * Cfg::Values().deviationFactor);
      // light_rain chance: cold and humid -> high, 5
      stratRegion->weatherMonths[i].push_back(
          this->modConfig.weatherChances.at("baseLightRainChance") *
          (1.0 - averageTemperature) * averagePrecipitation);
      // heavy rain chance: warm and humid -> high, 6
      stratRegion->weatherMonths[i].push_back(
          this->modConfig.weatherChances.at("baseHeavyRainChance") *
          averageTemperature * averagePrecipitation);
      // mud chance, 7
      stratRegion->weatherMonths[i].push_back(
          this->modConfig.weatherChances.at("baseMudChance") *
          (2.0 * stratRegion->weatherMonths[i][6] +
           stratRegion->weatherMonths[i][5]));
      // blizzard chance, 8
      stratRegion->weatherMonths[i].push_back(
          std::clamp(this->modConfig.weatherChances.at("baseBlizzardChance") -
                         averageTemperature,
                     0.0, 0.2) *
          averagePrecipitation);
      // sandstorm chance, 9
      auto sandChance = std::clamp((averageTemperature - 0.8) *
                                       this->modConfig.weatherChances.at(
                                           "baseSandstormChance"),
                                   0.0, 0.1) *
                        std::clamp(0.2 - averagePrecipitation, 0.0, 0.2);
      stratRegion->weatherMonths[i].push_back(sandChance);
      // snow chance, 10
      stratRegion->weatherMonths[i].push_back(
          std::clamp(this->modConfig.weatherChances.at("baseSnowChance") -
                         averageTemperature,
                     0.0, 0.2) *
          averagePrecipitation);
      // no phenomenon chance, 11
      stratRegion->weatherMonths[i].push_back(
          1.0 - stratRegion->weatherMonths[i][5] -
          stratRegion->weatherMonths[i][6] - stratRegion->weatherMonths[i][8] -
          stratRegion->weatherMonths[i][9] - stratRegion->weatherMonths[i][10]);
    }
  }
}

void Generator::finaliseData() {
  generatePositions(terrainData, ardaProvinces);
  generateLogistics(shared_from_this(), modData, areaData, ardaProvinces,
                    provinceMap, countryMap);
  evaluateCountryStrength(modData, stats, ardaData, ardaConfig);
  generateRandomDecisions(modData, ardaRegions);
  generateFocusTrees(modData);
  generateWeather();
}

void Generator::printStatistics() {
  gatherStatistics();

  for (auto &scores : ardaData.countryImportanceScores) {
    for (auto &entry : scores.second) {
      // auto &hoi4Country = modData.hoi4Countries[entry->tag];
      //  search the corresponding hoi4Country in hoi4COuntries by tag.
      // reinterpret this country as a shared pointer to Hoi4Country
      auto hoi4Country = std::dynamic_pointer_cast<Hoi4Country>(entry);
      Fwg::Utils::Logging::logLine(
          "Strength: ", scores.first, " ", hoi4Country->fullName, " ",
          Arda::Utils::ideologyToString.at(hoi4Country->ideology), "");
    }
  }
  Fwg::Utils::Logging::logLine("Total Industry: ", stats.militaryIndustry +
                                                       stats.civilianIndustry +
                                                       stats.navalIndustry);
  Fwg::Utils::Logging::logLine("Military Industry: ", stats.militaryIndustry);
  Fwg::Utils::Logging::logLine("Civilian Industry: ", stats.civilianIndustry);
  Fwg::Utils::Logging::logLine("Naval Industry: ", stats.navalIndustry);
  for (auto &res : ardaStats.totalResources) {
    Fwg::Utils::Logging::logLine(res.first, " ", res.second);
  }

  Fwg::Utils::Logging::logLine("World Gdp: ", ardaStats.totalWorldGdp);
  Fwg::Utils::Logging::logLine("World Population: ",
                               ardaStats.totalWorldPopulation);
  Fwg::Utils::Logging::logLine(
      "Over all countries distributed, the following amount of divisions was "
      "deployed: ");

  for (const auto &divisionTypeAmount : this->stats.divisionsByType) {
    Fwg::Utils::Logging::logLine(
        divisionTypeAmount.second, " ",
        Hoi4::divisionTypeMap.at(divisionTypeAmount.first), "s");
  }

  Fwg::Utils::Logging::logLine(
      "Over all countries distributed, the following amount of ships was "
      "deployed: ");
  for (const auto shipTypeAmount : this->stats.shipsByClass) {
    Fwg::Utils::Logging::logLine(
        shipTypeAmount.second, " ",
        Hoi4::shipClassTypeMap.at(shipTypeAmount.first), "s");
  }
}

void Generator::loadStates() {}

bool Generator::loadRivers(Fwg::Cfg &config,
                           const Fwg::Gfx::Image &riverInput) {
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

  // print all colours in the colourMapping
  for (const auto &pair : colourMapping) {
    Fwg::Utils::Logging::logLine("Colour Mapping: ", pair.first.toString(),
                                 " -> ", pair.second.toString());
  }

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

bool Generator::loadRiversFromBlueMask(
    Fwg::Cfg &config, const Fwg::Gfx::Image &riverInput) {
  auto riverCopy = riverInput;
  const std::set<Fwg::Gfx::Colour> blueShades{
      {0, 225, 255}, {0, 200, 255}, {0, 150, 255}, {0, 100, 255},
      {0, 0, 255},   {0, 0, 225},   {0, 0, 200},   {0, 0, 150},
      {0, 0, 100}};
  for (auto &pixel : riverCopy.imageData) {
    if (blueShades.contains(pixel))
      pixel = {0, 0, 255};
  }
  return FastWorldGenerator::loadRiversFromBlueMask(config, riverCopy);
}

void Generator::initImageExporter() {
  imageExporter = Gfx::Hoi4::ImageExporter(pathcfg.gamePath, "Hoi4");
}

void Generator::writeTextFiles(bool scenarioDetails) {
  using namespace Parsing::Writing;
  Fwg::Utils::Logging::logLine(
      "Writing Hoi4 mod text files to path: ",
      Fwg::Utils::userFilter(pathcfg.gameModPath, Cfg::Values().username));
  copyDescriptorFile(Fwg::Cfg::Values().resourcePath + "/hoi4/descriptor.mod",
                     pathcfg.gameModPath, pathcfg.gameModsDirectory,
                     pathcfg.modName);
  tutorials(pathcfg.gameModPath + "tutorial/tutorial.txt");
  Map::adj(pathcfg.gameModPath + "map/adjacencies.csv");
  Map::adjacencyRules(pathcfg.gameModPath + "map/adjacency_rules.txt");
  Map::ambientObjects(pathcfg.gameModPath + "map/ambient_object.txt");
  Map::supply(pathcfg.gameModPath + "map/", modData.supplyNodeConnections);
  Map::buildings(pathcfg.gameModPath + "map/buildings.txt", modData.hoi4States);
  Map::continents(pathcfg.gameModPath + "map/continent.txt", ardaContinents,
                  pathcfg.gamePath,
                  pathcfg.gameModPath +
                      "localisation/language/province_names_l_language.yml");
  Map::definition(pathcfg.gameModPath + "map/definition.csv", ardaProvinces);
  Map::strategicRegions(pathcfg.gameModPath + "map/strategicregions",
                        areaData.regions, superRegions);
  Map::unitStacks(pathcfg.gameModPath + "map/unitstacks.txt", ardaProvinces,
                  modData.hoi4States, terrainData.detailedHeightMap);
  Map::weatherPositions(pathcfg.gameModPath + "map/weatherpositions.txt",
                        areaData.regions, superRegions);

  Countries::commonCountryTags(pathcfg.gameModPath +
                                   "common/country_tags/02_countries.txt",
                               modData.hoi4Countries);
  Countries::commonCountries(pathcfg.gameModPath + "common/countries/",
                             pathcfg.gamePath + "common/countries/colors.txt",
                             modData.hoi4Countries);

  Countries::flags(pathcfg.gameModPath + "gfx/flags/", modData.hoi4Countries);
  Countries::historyCountries(pathcfg.gameModPath + "history/countries/",
                              modData.hoi4Countries, pathcfg.gamePath,
                              areaData.regions, Rpx::Hoi4::shipClassTypeMap);
  if (scenarioDetails) {
    Common::commonDecisions(pathcfg.gameModPath + "/common/decisions/",
                            modData.decisionData);

    Countries::commonCharacters(pathcfg.gameModPath + "common/characters/",
                                modData.hoi4Countries);

    Countries::commonNames(pathcfg.gameModPath + "common/names/00_names.txt",
                           modData.hoi4Countries);
    Countries::historyUnits(pathcfg.gameModPath + "history/units/",
                            modData.hoi4Countries);
    Countries::ideas(pathcfg.gameModPath + "common/ideas/",
                     modData.hoi4Countries);
    Countries::foci(pathcfg.gameModPath + "common/national_focus/",
                    modData.hoi4Countries, nData);
    // Countries::foci(pathcfg.gameModPath + "/common/national_focus//",
    //                 modData.hoi4Countries, nData);
  }

  Countries::portraits(pathcfg.gameModPath + "portraits/",
                       modData.hoi4Countries);
  Countries::states(pathcfg.gameModPath + "history/states/",
                    modData.hoi4States);
  Compatibility::compatibilityFactionMechanics(
      pathcfg.gameModPath + "common/factions/", "");
  Compatibility::compatibilityNationalFocus(
      pathcfg.gameModPath + "common/national_focus/", pathcfg.gamePath);

  aiStrategy(pathcfg.gameModPath + "common/", ardaContinents);
  // copy in generic events
  events(pathcfg.gameModPath);
  onActions(pathcfg.gameModPath);
  commonBookmarks(pathcfg.gameModPath + "common/bookmarks/",
                  modData.hoi4Countries, ardaData.countryImportanceScores);
  dynamicModifiers(Fwg::Cfg::Values().resourcePath +
                       "/hoi4/common/dynamic_modifiers/",
                   pathcfg.gameModPath + "common/dynamic_modifiers/");
  scriptedEffects(Fwg::Cfg::Values().resourcePath +
                      "/hoi4/common/scripted_effects/",
                  pathcfg.gameModPath + "common/scripted_effects/");
  scriptedTriggers(Fwg::Cfg::Values().resourcePath +
                       "/hoi4/common/scripted_triggers/",
                   pathcfg.gameModPath + "common/scripted_triggers/");
  // commonFiltering(pathcfg.gamePath, pathcfg.gameModPath);
}
void Generator::writeLocalisation() {

  using namespace Parsing::Writing::Localisation;
  decisionNames(pathcfg.gameModPath + "/localisation/language/",
                modData.decisionData.decisionNames);
  stateNames(pathcfg.gameModPath + "/localisation/language/",
             modData.hoi4Countries);
  countryNames(pathcfg.gameModPath + "/localisation/language/",
               modData.hoi4Countries, nData);
  strategicRegionNames(pathcfg.gameModPath + "/localisation/language/",
                       superRegions);
  victoryPointNames(pathcfg.gameModPath + "/localisation/language/",
                    modData.hoi4States);
  predefinedLocalisation(pathcfg.gameModPath + "/localisation/");
  focusTreeLocalisation(pathcfg.gameModPath + "/localisation/",
                        modData.hoi4Countries);
}
void Generator::writeImages() {
  Fwg::Utils::Logging::logLine(
      "Writing Hoi4 mod image files to path: ",
      Fwg::Utils::userFilter(pathcfg.gameModPath, Cfg::Values().username));

  imageExporter.dump8BitTerrain(terrainData, climateData, ardaData.civLayer,
                                pathcfg.gameModPath + "/map/terrain.bmp",
                                "terrain", false);
  imageExporter.dump8BitCities(
      climateMap, pathcfg.gameModPath + "/map/cities.bmp", "cities", false);
  imageExporter.dump8BitRivers(terrainData, climateData,
                               pathcfg.gameModPath + "/map/rivers", "rivers",
                               false);
  imageExporter.dump8BitTrees(terrainData, climateData,
                              pathcfg.gameModPath + "/map/trees.bmp", "trees",
                              false);
  imageExporter.dump8BitHeightmap(terrainData.detailedHeightMap,
                                  pathcfg.gameModPath + "/map/heightmap",
                                  "heightmap");
  imageExporter.dumpTerrainColourmap(
      worldMap, ardaData.civLayer, pathcfg.gameModPath,
      "/map/terrain/colormap_rgb_cityemissivemask_a.dds",
      gli::format::FORMAT_BGR8_UNORM_PACK32, 2, false);
  imageExporter.dumpDDSFiles(
      terrainData.detailedHeightMap,
      pathcfg.gameModPath + "/map/terrain/colormap_water_", false, 8);
  imageExporter.dumpWorldNormal(
      Fwg::Gfx::Image(Cfg::Values().width, Cfg::Values().height, 24,
                      terrainData.sobelData),
      pathcfg.gameModPath + "/map/world_normal.bmp", false);

  // just copy over provinces.bmp, already in a compatible format
  Fwg::Gfx::Bmp::save(provinceMap,
                      (pathcfg.gameModPath + ("/map/provinces.bmp")).c_str());
}

void Generator::generate() {
  const auto &config = Fwg::Cfg::Values();
  if (config.width % 64 || config.height % 64) {
    throw(
        std::runtime_error("Invalid format, both width and height of the image "
                           "must be multiples of 64."));
  } else if (config.scale && (config.scaleX % 64 || config.scaleY % 64)) {
    throw(std::runtime_error("Invalid target dimensions for scaling mode, both "
                             "scaleX and scaleY of the image "
                             "must be multiples of 64."));
  }
  if (!createPaths())
    return;
  try {
    // start with the generic stuff in the Scenario hoi4Gen
    mapProvinces();
    mapRegions();
    mapContinents();
    mapTerrain();
    // generate generic world data
    genCivilisationData();

    // non-country stuff
    auto stratFactory = []() -> std::shared_ptr<StrategicRegion> {
      return std::make_shared<StrategicRegion>();
    };
    if (!generateStrategicRegions(stratFactory)) {
      Fwg::Utils::Logging::logLine(
          "Error generating strategic regions, aborting");
      return;
    }
    // generate state information
    generateStateSpecifics();
    generateStateResources();
    auto countryFactory = []() -> std::shared_ptr<Hoi4Country> {
      return std::make_shared<Hoi4Country>();
    };
    // generate country data
    generateCountries(countryFactory);
    // politics, etc
    // generateHardCountrySpecifics();
    deriveCountrySpecificsFromSimulation();

    finaliseData();

  } catch (std::exception &e) {
    std::string error = "Error while generating the Hoi4 Module.\n";
    error += "Error is: \n";
    error += e.what();
    Fwg::Utils::Logging::logLine(error);
  }
  // now start writing game files
  try {
    writeImages();
    writeTextFiles(true);
    writeLocalisation();
  } catch (std::exception &e) {
    std::string error = "Error while dumping and writing files.\n";
    error += "Error is: \n";
    error += e.what();
    Fwg::Utils::Logging::logLine(error);
  }
  // now if everything worked, print info about world and pause for user to
  // see
  printStatistics();
}
void Generator::readHoi(std::string &path) {
  path.append("/");
  auto &config = Fwg::Cfg::Values();
  bool bufferedCut = config.cut;
  config.cut = false;
  auto heightmap = Fwg::IO::Reader::readGenericImage(path + "map/heightmap.bmp",
                                                     config, false);
  loadHeight(config, heightmap);
  genSobelMap(config);
  genLand();
  loadClimate(config, Fwg::IO::Reader::readGenericImage(
                          path + "map/terrain.bmp", config));
  provinceMap =
      Fwg::IO::Reader::readGenericImage(path + "map/provinces.bmp", config);
  //// read in game or mod files
  climateData.habitabilities.resize(provinceMap.size());
  Hoi4::Parsing::Reading::readProvinces(terrainData, climateData, path,
                                        "provinces.bmp", areaData);
  wrapupProvinces(config);
  // get the provinces into ardaProvinces
  mapProvinces();
  // load existing states: we first get all the state files and parse their
  // provinces for land regions (including lakes) then we need to get the
  // strategic region files, and for every strategic region that is a sea
  // state, we create a sea region? Hoi4::Parsing::Reading::readStates(path,
  // hoi4Gen);

  // ensure continents are created via the details in definition.csv.
  // Which means we also need to load the existing continents file to match
  // those with each other, so another export does not overwrite the
  // continents
  std::map<int, Areas::Continent> continents;
  for (auto &prov : areaData.provinces) {
    if (prov->continent->ID != -1) {
      if (continents.find(prov->continent->ID) == continents.end()) {
        Areas::Continent continent(prov->continent->ID);
        continents.insert({prov->continent->ID, continent});
      } else {
        continents.at(prov->continent->ID).provinces.push_back(prov);
      }
    }
  }
  // areaData.continents.clear();
  // for (auto &c : continents) {
  //   areaData.continents.push_back(c.second);
  // }

  // get the provinces into ardaProvinces
  // mapProvinces();
  // get the states from files to initialize ardaRegions
  // Hoi4::Parsing::Reading::readStates(gamePath, *hoi4Gen);
  // try {
  //  mapRegions();
  //} catch (std::exception& e) {
  //  Fwg::Utils::Logging::logLine("Error while mapping regions, ", e.what());
  //};
  //// read the colour codes from the game/mod files
  // countryColourMap =
  //     Hoi4::Parsing::Reading::readColourMapping(pathcfg.gamePath);
  //// now initialize hoi4 states from the ardaRegions
  // mapTerrain();
  // for (auto &c : countries) {
  //   auto fCol = countryColourMap.valueSearch(c.first);
  //   if (fCol != Fwg::Gfx::Colour{0, 0, 0}) {
  //     c.second->colour = fCol;
  //   } else {
  //     do {
  //       // generate random colour as long as we have a duplicate
  //       c.second->colour = Fwg::Gfx::Colour(RandNum::getRandom(1, 254),
  //                                           RandNum::getRandom(1, 254),
  //                                           RandNum::getRandom(1, 254));
  //     } while (countryColourMap.find(c.second->colour));
  //     countryColourMap.setValue(c.second->colour, c.first);
  //   }
  // }
  // mapCountries();
  //// read in further state details from map files
  // Hoi4::Parsing::Reading::readAirports(pathcfg.gamePath,
  // modData.hoi4States);
  // Hoi4::Parsing::Reading::readRocketSites(pathcfg.gamePath,
  //                                         modData.hoi4States);
  // Hoi4::Parsing::Reading::readBuildings(pathcfg.gamePath,
  // modData.hoi4States);
  // Hoi4::Parsing::Reading::readSupplyNodes(pathcfg.gamePath,
  //                                         modData.hoi4States);
  // Hoi4::Parsing::Reading::readWeatherPositions(pathcfg.gamePath,
  //                                              modData.hoi4States);
  config.cut = bufferedCut;
}

void Generator::save(const std::string &path) {
  // take every superregion, cast it to a strategic region
  for (auto &superRegion : superRegions) {
    auto stratRegion = std::make_shared<StrategicRegion>();
    stratRegion->forceStrategicRegionLink();
  }
  std::ofstream file(path, std::ios::binary);
  boost::archive::binary_oarchive ar(file);
  ar << areaData << terrainData << climateData;
  ar << climateMap << worldMap << segmentMap << provinceMap << regionMap;
  ar << locationMap << navmeshMap << errorMap;
  ar << preModifyHeightMap << preModifyHumidityMap;
  ar << ardaContinents << ardaRegions << ardaProvinces << superRegions;
  ar << countries << civData << nData;
  ar << typeMap << countryMap << superRegionMap;
  ar << gameType << exportWidth << exportHeight;
  ar << pathcfg;
  ar << modData.hoi4States << modData.hoi4Countries;
  ar << modData.supplyNodeConnections;
  ar << modData.statesInitialised;
  ar << modData.factions;
  ar << modData.decisionData;
}

void Generator::load(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  boost::archive::binary_iarchive ar(file);
  resetData();
  ar >> areaData >> terrainData >> climateData;
  ar >> climateMap >> worldMap >> segmentMap >> provinceMap >> regionMap;
  ar >> locationMap >> navmeshMap >> errorMap;
  ar >> preModifyHeightMap >> preModifyHumidityMap;
  ar >> ardaContinents >> ardaRegions >> ardaProvinces >> superRegions;
  ar >> countries >> civData >> nData;
  ar >> typeMap >> countryMap >> superRegionMap;
  ar >> gameType >> exportWidth >> exportHeight;
  ar >> pathcfg;
  ar >> modData.hoi4States >> modData.hoi4Countries;
  ar >> modData.supplyNodeConnections;
  ar >> modData.statesInitialised;
  ar >> modData.factions;
  ar >> modData.decisionData;

  mapProvinces();
  Fwg::Areas::Provinces::Detail::createProvinceMap(areaData.provinces,
                                                   areaData.provinceColourMap);
  Fwg::Areas::Provinces::Detail::PostProcessing::evaluateProvinceNeighbours(
      provinceMap, areaData.provinces, areaData.provinceColourMap);
  Fwg::Areas::Regions::evaluateRegionNeighbours(areaData.regions);
  mapRegions();
  this->modData.statesInitialised = true;
  mapContinents();
}

} // namespace Rpx::Hoi4