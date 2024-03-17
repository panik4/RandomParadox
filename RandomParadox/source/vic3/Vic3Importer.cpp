#include "vic3/Vic3Importer.h"

namespace Scenario::Vic3::Importing {
namespace PU = Fwg::Parsing;
namespace PUS = Fwg::Parsing::Scenario;

std::map<std::string, Technology> readTechs(const std::string &inFolder) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: Map: Reading techs from ",
                               inFolder);
  std::map<std::string, Technology> techs;
  for (auto const &dir_entry : std::filesystem::directory_iterator{inFolder}) {
    std::string pathString = dir_entry.path().string();
    if (pathString.find(".txt") == std::string::npos)
      continue;

    Fwg::Utils::Logging::logLine("Working with: ", pathString);
    std::string filename =
        pathString.substr(pathString.find_last_of("//") + 1,
                          pathString.back() - pathString.find_last_of("//"));
    Fwg::Utils::Logging::logLine("Determined filename: ", filename);
    std::string content = "";
    auto lines = Fwg::Parsing::getLines(pathString);
    auto blocks = Fwg::Parsing::Scenario::getOuterBlocks(lines);

    for (auto &block : blocks) {
      Technology tech;
      tech.name = block.name;
      PUS::removeSpecials(tech.name);
      tech.era = PU::getValue(block.content, "era");
      PUS::removeCharacter(tech.era, ' ');
      auto unlockTechs =
          PUS::getBracketBlockContent(block.content, "unlocking_technologies");
      auto unlockTechTokens = PU::getTokens(unlockTechs, '\n');
      for (auto &technology : unlockTechTokens) {
        PUS::removeSpecials(technology);
        if (technology.size())
          tech.unlockingTechnologies.push_back(technology);
      }

      techs.insert({tech.name, tech});
    }
  }

  return techs;
}
std::map<std::string, TechnologyLevel>
readTechLevels(const std::string &inPath,
               const std::map<std::string, Technology> &techs) {
  std::map<std::string, TechnologyLevel> techlevels;
  auto lines = Fwg::Parsing::getLines(inPath);
  auto blocks = Fwg::Parsing::Scenario::getOuterBlocks(lines);

  for (auto &block : blocks) {
    TechnologyLevel techLevel;
    // something like wealth_9
    techLevel.name = block.name;
    PUS::removeSpecials(techLevel.name);

    auto blockLines = PU::splitLines(block.content);
    for (auto &line : blockLines) {
      if (line.find("add_era_researched") != std::string::npos) {
        // add a fully researched era
        auto &era = Fwg::Parsing::getValue(line, "add_era_researched");
        PUS::removeCharacter(era, ' ');
        techLevel.era_researched = era;
        for (auto &tech : techs) {
          if (tech.second.era == era) {
            techLevel.technologies.push_back(tech.second);
          }
        }
      } else if (line.find("add_technology_researched") != std::string::npos) {
        // add particular technologies
        auto &tech = Fwg::Parsing::getValue(line, "add_technology_researched");
        PUS::removeCharacter(tech, ' ');
        if (techs.find(tech) != techs.end()) {
          techLevel.technologies.push_back(techs.at(tech));
        } else {
          Fwg::Utils::Logging::logLine("Warning: Can't find tech ", tech,
                                       " for techlevel");
        }
      }
    }
    techlevels.emplace(techLevel.name, techLevel);
  }
  return techlevels;
}
std::map<std::string, ProductionmethodGroup> readProdMethodGroups(
    const std::string &inFolder,
    const std::map<std::string, Productionmethod> &productionmethods) {
  std::map<std::string, ProductionmethodGroup> prodMethodGroups;
  Fwg::Utils::Logging::logLine(
      "Vic3 Parser: Map: Reading production method groups from ", inFolder);
  for (auto const &dir_entry : std::filesystem::directory_iterator{inFolder}) {
    std::string pathString = dir_entry.path().string();
    if (pathString.find(".txt") == std::string::npos)
      continue;

    std::string filename =
        pathString.substr(pathString.find_last_of("//") + 1,
                          pathString.back() - pathString.find_last_of("//"));
    std::string content = "";
    auto lines = Fwg::Parsing::getLines(pathString);
    auto blocks = Fwg::Parsing::Scenario::getOuterBlocks(lines);

    for (auto &block : blocks) {
      ProductionmethodGroup prodmethGroup;
      PUS::removeSpecials(block.name);
      prodmethGroup.name = block.name;
      // the contained "production_methods" block
      auto innerblocks = Fwg::Parsing::Scenario::getOuterBlocks(
          Fwg::Parsing::splitLines(block.content));
      for (auto &innerblock : innerblocks) {
        std::cout << innerblock.content << std::endl;
        auto lines = PU::splitLines(innerblock.content);
        for (auto &line : lines) {
          PUS::removeSpecials(line);
          PUS::removeCharacter(line, ' ');
          try {

            prodmethGroup.productionMethods.emplace(line,
                                                    productionmethods.at(line));
          } catch (std::exception e) {
            Fwg::Utils::Logging::logLineLevel(
                9, "Warning: Can't access with line ", line);
          }
        }
      }
      prodMethodGroups.emplace(prodmethGroup.name, prodmethGroup);
    }
  }

  return prodMethodGroups;
}
std::map<std::string, Productionmethod>
readProdMethods(const std::string &inFolder,
                const std::map<std::string, Good> &goods,
                const std::map<std::string, Technology> &techs) {
  const std::string endString = "_add";
  const std::string inGood = "goods_input_";
  const std::string outGood = "goods_output_";
  std::map<std::string, Productionmethod> prodMethods;
  Fwg::Utils::Logging::logLine(
      "Vic3 Parser: Map: Reading production methods from ", inFolder);
  for (auto const &dir_entry : std::filesystem::directory_iterator{inFolder}) {
    std::string pathString = dir_entry.path().string();
    if (pathString.find(".txt") == std::string::npos)
      continue;

    std::string filename =
        pathString.substr(pathString.find_last_of("//") + 1,
                          pathString.back() - pathString.find_last_of("//"));
    std::string content = "";
    auto lines = Fwg::Parsing::getLines(pathString);
    auto blocks = Fwg::Parsing::Scenario::getOuterBlocks(lines);

    for (auto &block : blocks) {
      Productionmethod prodmeth;
      PUS::removeSpecials(block.name);
      prodmeth.name = block.name;
      auto prodmetLines = Fwg::Parsing::splitLines(block.content);

      auto unlockTechs =
          PUS::getBracketBlockContent(block.content, "unlocking_technologies");
      auto unlockTechTokens = PU::getTokens(unlockTechs, '\n');
      for (auto &tech : unlockTechTokens) {
        PUS::removeSpecials(tech);
        PUS::removeCharacter(tech, ' ');
        if (tech.size() && techs.find(tech) != techs.end()) {
          prodmeth.unlockingTechnologies.emplace(tech, techs.at(tech));
        }
      }

      for (auto &line : prodmetLines) {
        if (line.find(inGood) != std::string::npos) {
          auto goodName = PUS::getEntrenched(line, inGood, endString);
          // get first number in line
          auto amount = PUS::getNumber(line);
          if (amount) {
            prodmeth.inputs.push_back({goods.at(goodName), amount});
          }
        } else if (line.find(outGood) != std::string::npos) {
          auto goodName = PUS::getEntrenched(line, outGood, endString);
          auto amount = PUS::getNumber(line);
          if (amount) {
            prodmeth.outputs.push_back({goods.at(goodName), amount});
          }
        }
      }
      prodMethods.emplace(prodmeth.name, prodmeth);
    }
  }

  return prodMethods;
}
std::vector<BuildingType> readBuildings(
    const std::string &inFolder,
    std::map<std::string, ProductionmethodGroup> productionmethodGroups,
    const std::map<std::string, Technology> &techs) {
  std::vector<BuildingType> bts;
  Fwg::Utils::Logging::logLine("Vic3 Parser: Map: Reading buildings from ",
                               inFolder);
  for (auto const &dir_entry : std::filesystem::directory_iterator{inFolder}) {
    std::string pathString = dir_entry.path().string();
    if (pathString.find(".txt") == std::string::npos)
      continue;

    Fwg::Utils::Logging::logLine("Working with: ", pathString);
    std::string filename =
        pathString.substr(pathString.find_last_of("//") + 1,
                          pathString.back() - pathString.find_last_of("//"));
    Fwg::Utils::Logging::logLine("Determined filename: ", filename);
    std::string content = "";
    auto lines = Fwg::Parsing::getLines(pathString);
    auto blocks = Fwg::Parsing::Scenario::getOuterBlocks(lines);

    for (auto &block : blocks) {
      BuildingType bt;
      bt.name = block.name;
      PUS::removeSpecials(bt.name);
      bt.group = PU::getValue(block.content, "building_group");
      PUS::removeCharacter(bt.group, ' ');

      auto unlockTechs =
          PUS::getBracketBlockContent(block.content, "unlocking_technologies");
      auto unlockTechTokens = PU::getTokens(unlockTechs, '\n');
      for (auto &tech : unlockTechTokens) {
        PUS::removeSpecials(tech);
        PUS::removeCharacter(tech, ' ');
        if (tech.size() && techs.find(tech) != techs.end())
          bt.unlockTechs.push_back(techs.at(tech));
      }
      auto prodMethods = PUS::getBracketBlockContent(
          block.content, "production_method_groups");
      auto tempProductionMethodGroups = PU::getTokens(prodMethods, '\n');
      for (auto &prod : tempProductionMethodGroups) {
        PUS::removeSpecials(prod);
        if (prod.size()) {
          try {
            auto prodGroup = productionmethodGroups.at(prod);
            bt.productionMethodGroups.push_back(prodGroup);
            for (const auto &prodMethod : prodGroup.productionMethods) {
              bt.productionMethods.emplace(prodMethod);
            }
          } catch (std::exception e) {
            // silent
          }
        }
      }
      bool input, output = false;
      // designate classification of the building
      for (auto &productionMethod : bt.productionMethods) {
        if (productionMethod.second.inputs.size()) {
          input = true;
        }
        if (productionMethod.second.outputs.size()) {
          output = true;
        }
      }
      if (output && input) {
        bt.category = BuildingCategory::SECONDARY;
      } else if (output) {
        bt.category = BuildingCategory::PRIMARY;
      } else {
        bt.category = BuildingCategory::TERTIARY;
      }
      bts.push_back(bt);
    }
  }
  return bts;
}
std::map<std::string, Buypackage>
readBuypackages(const std::string &inFolder,
                const std::map<std::string, PopNeed> &popNeeds) {
  std::map<std::string, Buypackage> buypackages;
  auto lines = Fwg::Parsing::getLines(inFolder);
  auto blocks = Fwg::Parsing::Scenario::getOuterBlocks(lines);

  for (auto &block : blocks) {
    Buypackage bt;
    // something like wealth_9
    bt.name = block.name;
    PUS::removeSpecials(bt.name);
    // the contained "goods" block
    auto innerblocks = Fwg::Parsing::Scenario::getOuterBlocks(
        Fwg::Parsing::splitLines(block.content));
    for (auto &innerblock : innerblocks) {
      auto &lines = PU::splitLines(innerblock.content);
      for (auto &line : lines) {
        auto popneedname = PUS::getEntrenched(line, "\t", "=");
        PUS::removeSpecials(popneedname);
        PUS::removeCharacter(popneedname, ' ');
        auto amount = PUS::getNumber(line);
        if (amount) {
          bt.popNeeds.push_back({popNeeds.at(popneedname), amount});
        }
      }
    }
    buypackages.emplace(bt.name, bt);
  }
  return buypackages;
}
std::map<std::string, PopNeed>
readPopNeeds(const std::string &inFolder,
             const std::map<std::string, Good> &goods) {
  std::map<std::string, PopNeed> popneeds;
  auto lines = Fwg::Parsing::getLines(inFolder);
  auto blocks = Fwg::Parsing::Scenario::getOuterBlocks(lines);

  for (auto &block : blocks) {
    PopNeed bt;
    bt.name = block.name;
    PUS::removeSpecials(bt.name);
    auto innerblocks = Fwg::Parsing::Scenario::getOuterBlocks(
        Fwg::Parsing::splitLines(block.content));
    for (auto &innerblock : innerblocks) {
      auto goodName = Fwg::Parsing::getValue(innerblock.content, "goods");
      Fwg::Parsing::Scenario::removeSpecials(goodName);
      Fwg::Parsing::Scenario::removeCharacter(goodName, ' ');
      try {
        bt.goods.push_back(goods.at(goodName));
      } catch (std::exception e) {
        Fwg::Utils::Logging::logLine("Unable to access good with name: ",
                                     goodName);
      }
    }
    popneeds.emplace(bt.name, bt);
  }
  return popneeds;
}
std::map<std::string, Good> readGoods(const std::string &inFolder) {
  std::map<std::string, Good> goods;
  auto lines = Fwg::Parsing::getLines(inFolder);
  auto blocks = Fwg::Parsing::Scenario::getOuterBlocks(lines);

  for (auto &block : blocks) {
    Good good;
    PUS::removeSpecials(block.name);
    good.name = block.name;
    auto lines = PU::splitLines(block.content);
    for (auto &line : lines) {
      if (line.find("cost") != std::string::npos &&
          line.find("_cost") == std::string::npos) {
        good.cost = PUS::getNumber(line);
      }
    }

    goods.emplace(good.name, good);
  }

  return goods;
}
std::set<std::string> readTags(const std::string &inFolder) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: Map: Reading tags from ",
                               inFolder);
  std::set<std::string> tags;
  for (auto const &dir_entry : std::filesystem::directory_iterator{inFolder}) {
    std::string pathString = dir_entry.path().string();
    if (pathString.find(".txt") == std::string::npos)
      continue;
    std::string tag = pathString.substr(0, 3);
    std::transform(tag.begin(), tag.end(), tag.begin(), ::toupper);
    tags.insert(tag);
  }
  return tags;
}
} // namespace Scenario::Vic3::Importing
