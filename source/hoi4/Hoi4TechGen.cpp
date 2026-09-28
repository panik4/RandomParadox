#include "hoi4/Hoi4TechGen.h"

namespace Rpx::Hoi4 {
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

  // Lambda to process a single tech era
  auto processTechEra = [&](TechEra currentEra, TechEra prerequisiteEra,
                            double threshold) {
    if (techsToAssign.find(currentEra) == techsToAssign.end()) {
      return;
    }

    for (auto &moduleTech : techsToAssign.at(currentEra)) {
      // check if we already have that tech
      bool alreadyHas = false;
      for (auto &module : countryCategoryTechs.at(currentEra)) {
        if (module.name == moduleTech.name) {
          alreadyHas = true;
          break;
        }
      }
      if (alreadyHas) {
        continue;
      }

      // check if this tech has a prerequisite
      if (moduleTech.predecessor.size()) {
        bool hasPrerequisite = false;
        // check if we have the prerequisite tech from the appropriate era
        for (auto &module : countryCategoryTechs.at(prerequisiteEra)) {
          if (module.name == moduleTech.predecessor) {
            hasPrerequisite = true;
            break;
          }
        }
        // if we don't have the prerequisite, skip this tech
        if (!hasPrerequisite) {
          continue;
        }
      }

      // randomly decide if we take this tech
      auto randomVal = RandNum::getRandom(0.0, 1.0) * techLevel;
      if (randomVal > threshold) {
        countryCategoryTechs.at(currentEra).push_back(moduleTech);
      }
    }
  };

  // Process each era with its parameters
  processTechEra(TechEra::Interwar, TechEra::Interwar, 0.25);
  processTechEra(TechEra::Buildup, TechEra::Interwar, 0.75);
  processTechEra(TechEra::Early, TechEra::Buildup, 1.25);
}

void generateTechLevels(Hoi4Data &modData) {
  // vector for all hull types
  const std::vector<NavalHullType> navalHullTypes{
      NavalHullType::Light, NavalHullType::Heavy, NavalHullType::Cruiser,
      NavalHullType::Carrier, NavalHullType::Submarine};

  // read in the techs from the files
  auto industryElectronicTechsFile = Fwg::Parsing::getLines(
      Fwg::Cfg::Values().resourcePath +
      "/hoi4/common/technologies/industryElectronicTechs.txt");
  std::map<TechEra, std::vector<Technology>> industryElectronicTechs;
  createTech(industryElectronicTechsFile, industryElectronicTechs);

  auto infantryTechsFile =
      Fwg::Parsing::getLines(Fwg::Cfg::Values().resourcePath +
                             "/hoi4/common/technologies/infantryTechs.txt");
  std::map<TechEra, std::vector<Technology>> infantryTechs;
  createTech(infantryTechsFile, infantryTechs);

  auto armorTechsFile =
      Fwg::Parsing::getLines(Fwg::Cfg::Values().resourcePath +
                             "/hoi4/common/technologies/armorTechs.txt");
  std::map<TechEra, std::vector<Technology>> armorTechs;
  createTech(armorTechsFile, armorTechs);

  auto airTechsFile =
      Fwg::Parsing::getLines(Fwg::Cfg::Values().resourcePath +
                             "/hoi4/common/technologies/airTechs.txt");
  std::map<TechEra, std::vector<Technology>> airTechs;
  createTech(airTechsFile, airTechs);

  auto navyTechsFile =
      Fwg::Parsing::getLines(Fwg::Cfg::Values().resourcePath +
                             "/hoi4/common/technologies/navyTechs.txt");
  std::map<TechEra, std::vector<Technology>> navyTechs;
  createTech(navyTechsFile, navyTechs);

  for (auto &country : modData.hoi4Countries) {
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

    // a few techs are guaranteed, such as infantry_weapons
    country->infantryTechs.at(TechEra::Interwar)
        .push_back({"infantry_weapons", "", TechEra::Interwar});
    // gurantee we have sonar and basic_battery
    country->navyTechs.at(TechEra::Interwar)
        .push_back({"sonar", "", TechEra::Interwar});
    country->navyTechs.at(TechEra::Interwar)
        .push_back({"basic_battery", "", TechEra::Interwar});
    auto development = country->technologyLevel;
    auto navyTechLevel = development * country->navalFocus / 10.0;
    auto infantryTechLevel = development * country->landFocus / 10.0;
    auto armorTechLevel = development * country->landFocus / 10.0;
    auto airTechLevel = development * country->airFocus / 10.0;
    auto industryTechLevel = development * 5.0;

    if (country->rank == Arda::Rank::GreatPower ||
        country->rank == Arda::Rank::SecondaryPower) {
      // print levels
      Fwg::Utils::Logging::logLineLevel(
          8, "Country ", country->name, " has tech levels: navy ",
          navyTechLevel, ", infantry ", infantryTechLevel, ", armor ",
          armorTechLevel, ", air ", airTechLevel, ", industry ",
          industryTechLevel);
    }

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

  for (auto &country : modData.hoi4Countries) {
    // lets start with the navy. The higher our development and the more focues
    // we are on navy, the more advanced our navy#
    auto development = country->technologyLevel;
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

} // namespace Rpx::Hoi4