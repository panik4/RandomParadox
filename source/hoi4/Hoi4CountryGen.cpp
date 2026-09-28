#include "hoi4/Hoi4CountryGen.h"

namespace Rpx::Hoi4 {

Arda::Utils::Ideology
ideologyFromGovernment(Arda::Simulation::GovernmentForm government) {
  using Arda::Simulation::GovernmentForm;
  switch (government) {
  case GovernmentForm::CommunistState:
    return Arda::Utils::Ideology::COMMUNISM;
  case GovernmentForm::FascistState:
    return Arda::Utils::Ideology::FASCISM;
  case GovernmentForm::AristocraticRepublic:
  case GovernmentForm::OligarchicRepublic:
  case GovernmentForm::ConstitutionalRepublic:
  case GovernmentForm::ParliamentaryRepublic:
  case GovernmentForm::DirectDemocracy:
    return Arda::Utils::Ideology::DEMOCRATIC;
  default:
    return Arda::Utils::Ideology::NEUTRALITY;
  }
}

void setSimulationParties(
    Rpx::Hoi4::Hoi4Country &country,
    const Arda::Simulation::SimulationPolityExport &polityExport) {
  const auto &polity = polityExport.polity;
  const auto central = std::clamp(polity.politicalPower[0], 0.0, 1.0);
  const auto nobility = std::clamp(polity.politicalPower[1], 0.0, 1.0);
  const auto religious = std::clamp(polity.politicalPower[2], 0.0, 1.0);
  const auto people = std::clamp(polity.politicalPower[3], 0.0, 1.0);
  const auto military = std::clamp(polity.militaryInfluence, 0.0, 1.0);
  const auto partyControl = std::clamp(polity.partyControl, 0.0, 1.0);

  std::array<double, 4> scores{central + military, people,
                               people * (0.5 + partyControl),
                               nobility + religious + central * 0.5};
  const auto ideology = ideologyFromGovernment(polity.governmentForm);
  const auto dominantParty = ideology == Arda::Utils::Ideology::FASCISM      ? 0
                             : ideology == Arda::Utils::Ideology::DEMOCRATIC ? 1
                             : ideology == Arda::Utils::Ideology::COMMUNISM ? 2
                                                                            : 3;
  scores[dominantParty] += 1.0;

  const auto total = std::accumulate(scores.begin(), scores.end(), 0.0);
  if (total <= 0.0) {
    country.parties = {0, 0, 0, 100};
    return;
  }
  int assigned = 0;
  for (std::size_t index = 0; index < scores.size(); ++index) {
    country.parties[index] = static_cast<int>(scores[index] / total * 100.0);
    assigned += country.parties[index];
  }
  country.parties[dominantParty] += 100 - assigned;
}

void assignStartingLaws(Hoi4Country &country) {
  using namespace Arda::Utils;
  std::vector<std::string> conscriptionOptions, economyOptions, tradeOptions;
  switch (country.ideology) {
  case Ideology::FASCISM:
    conscriptionOptions = {"limited_conscription", "extensive_conscription"};
    economyOptions = {"low_economic_mobilisation",
                      "partial_economic_mobilisation"};
    tradeOptions = {"limited_exports", "autarkic_economy"};
    break;
  case Ideology::COMMUNISM:
    conscriptionOptions = {"disarmed_nation", "volunteer_only",
                           "limited_conscription", "extensive_conscription"};
    economyOptions = {"civilian_economy", "low_economic_mobilisation",
                      "partial_economic_mobilisation"};
    tradeOptions = {"autarkic_economy", "closed_economy"};
    break;
  case Ideology::DEMOCRATIC:
    conscriptionOptions = {"disarmed_nation", "volunteer_only",
                           "limited_conscription"};
    economyOptions = {"civilian_economy", "low_economic_mobilisation"};
    tradeOptions = {"free_trade", "export_focus", "limited_exports"};
    break;
  default:
    conscriptionOptions = {"volunteer_only", "limited_conscription"};
    economyOptions = {"civilian_economy", "low_economic_mobilisation"};
    tradeOptions = {"export_focus", "limited_exports", "autarkic_economy"};
    break;
  }
  country.conscriptionLaw =
      Fwg::Utils::Random::selectRandom(conscriptionOptions);
  country.economyLaw = Fwg::Utils::Random::selectRandom(economyOptions);
  country.tradeLaw = Fwg::Utils::Random::selectRandom(tradeOptions);
  if (country.rank == Arda::Rank::GreatPower) {
    if (country.ideology != Ideology::COMMUNISM &&
        country.ideology != Ideology::FASCISM && RandNum::getRandom(0, 1))
      country.conscriptionLaw = "limited_conscription";
    if (RandNum::getRandom(0, 1))
      country.economyLaw = "partial_economic_mobilisation";
  }
}

void distributeVictoryPoints(
    Hoi4Data &modData, Hoi4Stats &stats,
    std::vector<std::shared_ptr<Arda::ArdaProvince>> &ardaProvinces) {
  Fwg::Utils::Logging::logLine("Distributing victory points");
  double baseVPs = 10000;
  double assignedVPs = 0;
  for (auto country : modData.hoi4Countries) {

    if (!country->ownedRegions.size())
      continue;
    auto primaryCulture = country->getPrimaryCulture();
    for (auto &region : country->hoi4Regions) {
      if (!region->isLand() ||
          region->topographyTypes.count(
              Arda::Civilization::TopographyType::WASTELAND))
        continue;
      region->victoryPointsMap.clear();
      for (auto province : region->ardaProvinces) {
        province->victoryPoint = nullptr;
      }
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
        if (primaryCulture != nullptr) {
          vp.name = Fwg::Utils::Random::selectRandom(
              primaryCulture->language->cityNames);
        } else {
          vp.name = "Unnamed";
        }
        if (vps > 0) {
          region->victoryPointsMap[province.first] =
              std::make_shared<Arda::VictoryPoint>(vp);
          // assign the victory point to the province as well
          ardaProvinces.at(province.first)->victoryPoint =
              region->victoryPointsMap[province.first];
          assignedVPs += region->victoryPointsMap[province.first]->amount;
        }
      }
    }
  }
}

void generateCharacters(Hoi4Data &modData) {
  std::map<Arda::Utils::Ideology, std::vector<std::string>> leaderTraits = {
      {Arda::Utils::Ideology::NONE,
       {"cabinet_crisis", "headstrong", "humble", "inexperienced_monarch",
        "socialite_connections", "staunch_constitutionalist", "gentle_scholar",
        "the_statist", "the_academic"}},
      {Arda::Utils::Ideology::NEUTRALITY,
       {"cabinet_crisis",
        "headstrong",
        "humble",
        "inexperienced_monarch",
        "socialite_connections",
        "staunch_constitutionalist",
        "celebrity_junta_leader",
        "he_who_bears_the_throne",
        "conservative_grandee",
        "famous_aviator",
        "first_lady",
        "rearmer",
        "staunch_aristocrat",
        "autocratic_archbishop",
        "royal_dictator",
        "right_industrialist",
        "national_determinist",
        "noble_beurocrat",
        "veteran_anti_bolshevik",
        "agricultural_capitalist",
        "agricultural_nationalist",
        "democratic_crusader"}},
      {Arda::Utils::Ideology::FASCISM,
       {"autocratic_imperialist", "collaborator_king", "generallissimo",
        "inexperienced_imperialist", "spirit_of_genghis", "warmonger",
        "the_young_magnate", "polemarch", "archon_basileus", "autokrator",
        "basileus", "celebrity_junta_leader", "falangist_militarist",
        "subservient_ultranationalist", "vapsid_economist", "militant_minister",
        "dictator"}},
      {Arda::Utils::Ideology::COMMUNISM,
       {"political_dancer", "indomitable_perseverance",
        "mastermind_code_cracker", "polemarch", "reluctant_stalinist",
        "socialist_autocrat", "leftist_independent", "devoted_marxist",
        "anti_bolshevik_leftist", "leftist_intellectual", "leftist_legionary",
        "patriotic_socialist", "marxist_fundamentalist", "socialist_justice",
        "revolutionary_poet"}},
      {Arda::Utils::Ideology::DEMOCRATIC,
       {"conservative_grandee", "famous_aviator", "first_lady", "rearmer",
        "staunch_constitutionalist", "the_banker", "the_young_magnate",
        "liberal_democratic_paragon", "leftist_independent",
        "leftist_legionary", "veteran_minister"}}};

  std::map<Arda::Utils::Ideology, std::vector<std::string>> advisorTraits = {
      {Arda::Utils::Ideology::NONE,
       {"headstrong", "humble", "socialite_connections",
        "staunch_constitutionalist", "gentle_scholar", "the_statist",
        "the_academic"}},
      {Arda::Utils::Ideology::NEUTRALITY,
       {"headstrong", "humble", "socialite_connections",
        "staunch_constitutionalist", "gentle_scholar", "the_statist",
        "the_academic", "celebrity_junta_leader", "right_industrialist",
        "national_determinist", "noble_beurocrat", "veteran_anti_bolshevik",
        "agricultural_capitalist", "agricultural_nationalist"}},
      {Arda::Utils::Ideology::FASCISM,
       {"autocratic_imperialist", "collaborator_king", "generallissimo",
        "inexperienced_imperialist", "spirit_of_genghis", "warmonger",
        "the_young_magnate", "polemarch", "archon_basileus", "autokrator",
        "basileus", "celebrity_junta_leader", "subservient_ultranationalist",
        "vapsid_economist", "militant_minister"}},
      {Arda::Utils::Ideology::COMMUNISM,
       {"political_dancer", "indomitable_perseverance",
        "mastermind_code_cracker", "polemarch", "reluctant_stalinist",
        "leftist_independent", "devoted_marxist", "anti_bolshevik_leftist",
        "leftist_intellectual", "patriotic_socialist", "marxist_fundamentalist",
        "socialist_justice", "revolutionary_poet"}},
      {Arda::Utils::Ideology::DEMOCRATIC,
       {"conservative_grandee", "first_lady", "rearmer",
        "staunch_constitutionalist", "the_banker", "the_young_magnate",
        "gentle_scholar", "the_statist", "the_academic",
        "liberal_democratic_paragon", "leftist_legionary", "veteran_minister",
        "democratic_crusader"}}};

  std::vector<std::string> armyChiefTraits = {
      "army_chief_defensive_",      "army_chief_offensive_",
      "army_chief_drill_",          "army_chief_reform_",
      "army_chief_organizational_", "army_chief_planning_",
      "army_chief_morale_",         "army_chief_maneuver_",
      "army_chief_entrenchment_"};

  std::vector<std::string> airChiefTraits = {
      "air_chief_reform_",         "air_chief_safety_",
      "air_chief_old_guard",       "air_chief_night_operations_",
      "air_chief_ground_support_", "air_chief_all_weather_"};

  std::vector<std::string> navyChiefTraits = {
      "navy_chief_naval_aviation_",   "navy_chief_decisive_battle_",
      "navy_chief_commerce_raiding_", "navy_chief_old_guard",
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

  std::map<std::string, std::vector<std::string>> politicalAdvisorPortraits = {
      {"african",
       {"GFX_Portrait_Africa_Generic_1_small",
        "GFX_Portrait_South_Africa_Political_Leader_Generic_2_small",
        "GFX_Portrait_South_Africa_Political_Leader_Generic_small"}},
      {"asian",
       {"GFX_Portrait_Asia_Generic_1_small",
        "GFX_Portrait_Asia_Generic_2_small",
        "GFX_Portrait_Asia_Generic_3_small"}},
      {"western_european",
       {"GFX_Portrait_Europe_Generic_1_small",
        "GFX_Portrait_Europe_Generic_2_small",
        "GFX_Portrait_Europe_Generic_3_small"}},
      {"commonwealth",
       {"GFX_Portrait_Europe_Generic_1_small",
        "GFX_Portrait_Europe_Generic_2_small",
        "GFX_Portrait_Europe_Generic_3_small"}},
      {"eastern_european",
       {"GFX_Portrait_Europe_Generic_1_small",
        "GFX_Portrait_Europe_Generic_2_small",
        "GFX_Portrait_Europe_Generic_3_small"}},
      {"middle_eastern",
       {"GFX_Portrait_Arabia_Generic_1_small",
        "GFX_Portrait_Arabia_Generic_2_small",
        "GFX_Portrait_Arabia_Generic_3_small"}},
      {"southamerican",
       {"GFX_Portrait_South_America_Generic_1_small",
        "GFX_Portrait_South_America_Generic_2_small",
        "GFX_Portrait_South_America_Generic_3_small"}},

  };

  Fwg::Utils::Logging::logLine("Hoi4: Generating characters");
  for (auto &country : modData.hoi4Countries) {
    if (!country->ownedRegions.size())
      continue;
    country->characters.clear();
    // per country, we want to avoid duplicate names
    std::set<std::string> usedNames;
    // we want of every ideology: Neutral, Fascist, Communist, Democratic
    std::vector<Arda::Utils::Ideology> ideologies = {
        Arda::Utils::Ideology::NEUTRALITY, Arda::Utils::Ideology::FASCISM,
        Arda::Utils::Ideology::COMMUNISM, Arda::Utils::Ideology::DEMOCRATIC};

    auto createCharacter = [&](Arda::Type type, Arda::Utils::Ideology ideology,
                               const std::vector<std::string> &traits,
                               int count, bool addLevel = false) {
      for (int i = 0; i < count; i++) {
        Arda::Character character;
        character.gender = Arda::Gender::Male;
        do {
          auto primaryCulture = country->getPrimaryCulture();
          if (!primaryCulture) {
            Fwg::Utils::Logging::logLine(
                "Warning: Country " + country->name +
                " has no primary culture, cannot generate character names");
            character.name = "John";
            character.surname =
                "Doe " + std::to_string(country->characters.size());
          } else {
            character.name = Fwg::Utils::Random::selectRandom(
                primaryCulture->language->maleNames);
            character.surname = Fwg::Utils::Random::selectRandom(
                primaryCulture->language->surnames);
          }

        } while (usedNames.find(character.name + " " + character.surname) !=
                 usedNames.end());

        usedNames.insert(character.name + " " + character.surname);
        character.ideology = ideology;
        character.type = type;
        if (character.type == Arda::Type::Politician) {
          // for politicians, we want to assign a portrait according to the
          // country's primary culture
          auto gfxCulture = country->gfxCulture;
          auto portraits = politicalAdvisorPortraits.at(gfxCulture);
          character.portraitPath = Fwg::Utils::Random::selectRandom(portraits);
        }
        if (traits.size()) {
          auto trait = Fwg::Utils::Random::selectRandom(traits);
          if (addLevel && !trait.contains("old_guard")) {
            int level = RandNum::getRandom(1, 3);
            character.traits.push_back(trait + std::to_string(level));
          } else {
            character.traits.push_back(trait);
          }
        }
        country->characters.push_back(character);
      }
    };

    for (const auto &ideology : ideologies) {
      // 1 country leader
      createCharacter(Arda::Type::Leader, ideology, leaderTraits[ideology], 1);

      // 6 Politicians
      createCharacter(Arda::Type::Politician, ideology, advisorTraits[ideology],
                      6);

      // 4 Command Generals
      createCharacter(Arda::Type::ArmyChief, ideology, armyChiefTraits, 4,
                      true);

      // 2 Command Admirals
      createCharacter(Arda::Type::NavyChief, ideology, navyChiefTraits, 2,
                      true);

      // 2 Airforce Chiefs
      createCharacter(Arda::Type::AirForceChief, ideology, airChiefTraits, 2,
                      true);

      // 6 High Command
      createCharacter(Arda::Type::HighCommand, ideology, highCommandTraits, 6,
                      true);

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
        auto primaryCulture = country->getPrimaryCulture();
        if (!primaryCulture) {
          Fwg::Utils::Logging::logLine(
              "Warning: Country " + country->name +
              " has no primary culture, cannot generate theorist names");
          theorist.name = "John";
          theorist.surname =
              "Doe " + std::to_string(country->characters.size());
        } else {
          theorist.name = Fwg::Utils::Random::selectRandom(
              primaryCulture->language->maleNames);
          theorist.surname = Fwg::Utils::Random::selectRandom(
              primaryCulture->language->surnames);
        }
      } while (usedNames.find(theorist.name + " " + theorist.surname) !=
               usedNames.end());

      usedNames.insert(theorist.name + " " + theorist.surname);
      theorist.ideology = Arda::Utils::Ideology::NEUTRALITY;
      theorist.type = Arda::Type::Theorist;
      theorist.traits.push_back(theoristTraits.at(i));
      country->characters.push_back(theorist);
    }
  }
}

void generateFocusTrees(Hoi4Data &modData) {
  Hoi4::FocusGen::generateFocusFiles(modData.hoi4Countries);
}

void generateRandomDecisions(
    Hoi4Data &modData,
    std::vector<std::shared_ptr<Arda::ArdaRegion>> &ardaRegions) {
  Hoi4::DecisionGen::generateDecisions(modData.decisionData, ardaRegions);
}

void evaluateCountryStrength(Hoi4Data &modData, Hoi4Stats &stats,
                             Arda::ArdaData &ardaData,
                             Arda::ArdaConfig &ardaConfig) {
  Fwg::Utils::Logging::logLine("HOI4: Evaluating Country Strength");
  ardaData.countryImportanceScores.clear();
  double maxScore = 0.0;
  for (auto &country : modData.hoi4Countries) {
    country->evaluateTechnologyLevel();
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
    ardaData
        .countryImportanceScores[(int)(totalIndustry + totalPop / 1'000'000.0)]
        .push_back(country);
    country->importanceScore = totalIndustry + totalPop / 1'000'000.0;
    if (country->importanceScore > maxScore) {
      maxScore = country->importanceScore;
    }
    // global
    stats.totalWorldIndustry += (int)totalIndustry;
  }

  int totalDeployedCountries =
      ardaConfig.numCountries - ardaData.countryImportanceScores.size()
          ? (int)ardaData.countryImportanceScores[0].size()
          : 0;

  // sort countries by rank

  int numMajorPowers = std::min<int>(ardaConfig.numCountries / 10, 8);
  int numSecondaryPowers = std::min<int>(ardaConfig.numCountries / 10, 8);
  int numRegionalPowers = ardaConfig.numCountries / 6;
  int numLocalPowers = ardaConfig.numCountries / 6;

  // init countriesByRank
  ardaData.countriesByRank = {{Arda::Rank::GreatPower, {}},
                              {Arda::Rank::SecondaryPower, {}},
                              {Arda::Rank::RegionalPower, {}},
                              {Arda::Rank::LocalPower, {}},
                              {Arda::Rank::MinorPower, {}}};

  for (auto it = ardaData.countryImportanceScores.rbegin();
       it != ardaData.countryImportanceScores.rend(); ++it) {
    for (const auto &entry : it->second) {
      if (entry->importanceScore > 0.0) {
        entry->relativeScore = (double)it->first / maxScore;
        if (numMajorPowers >
            ardaData.countriesByRank.at(Arda::Rank::GreatPower).size()) {
          ardaData.countriesByRank[Arda::Rank::GreatPower].push_back(entry);
          entry->rank = Arda::Rank::GreatPower;
        } else if (numSecondaryPowers >
                   ardaData.countriesByRank.at(Arda::Rank::SecondaryPower)
                       .size()) {
          ardaData.countriesByRank[Arda::Rank::SecondaryPower].push_back(entry);
          entry->rank = Arda::Rank::SecondaryPower;
        } else if (numRegionalPowers >
                   ardaData.countriesByRank.at(Arda::Rank::RegionalPower)
                       .size()) {
          ardaData.countriesByRank[Arda::Rank::RegionalPower].push_back(entry);
          entry->rank = Arda::Rank::RegionalPower;
        } else if (numLocalPowers >
                   ardaData.countriesByRank.at(Arda::Rank::LocalPower).size()) {
          ardaData.countriesByRank[Arda::Rank::LocalPower].push_back(entry);
          entry->rank = Arda::Rank::LocalPower;
        } else {
          ardaData.countriesByRank[Arda::Rank::MinorPower].push_back(entry);
          entry->rank = Arda::Rank::MinorPower;
        }
      }
    }
  }
}
} // namespace Rpx::Hoi4