#include "hoi4/Hoi4FocusGen.h"
#include "RandNum.h"
#include <filesystem>
#include <fstream>
#include <regex>

namespace Rpx::Hoi4::FocusGen {

std::string
addAvailableBlocks(std::shared_ptr<Hoi4Country> country,
                   std::shared_ptr<Goal> goal,
                   const std::map<std::string, std::string> &availableMap) {
  std::string availableBlock = "";

  for (auto &availableGroup : goal->availabilities) {
    if (goal->availabilities.size() > 1) {
      availableBlock = "OR = {\n";
    }
    for (auto avail : availableGroup.availabilities) {
      if (availableMap.find(avail.name) != availableMap.end()) {
        auto blockText = availableMap.at(avail.name);

        // replace the country tag
        Fwg::Parsing::replaceOccurences(blockText, "templateTag", country->tag);
        // replace the target country tag
        if (goal->countryTarget) {
          Fwg::Parsing::replaceOccurences(blockText, "templateTargetTag",
                                          goal->countryTarget->tag);
        }
        // replace the target region ID
        if (goal->regionTarget) {
          Fwg::Parsing::replaceOccurences(
              blockText, "templateStateID",
              std::to_string(goal->regionTarget->ID + 1));
        }
        // replace the ideology
        Fwg::Parsing::replaceOccurences(
            blockText, "templateIdeology",
            Arda::Utils::ideologyToString.at(country->ideology));

        availableBlock.append(blockText);
      }
    }

    if (goal->availabilities.size() > 1) {
      availableBlock += "\n\t\t}\n";
    }
  }

  return availableBlock;
}

std::string
addBypassBlocks(std::shared_ptr<Hoi4Country> country,
                std::shared_ptr<Goal> goal,
                const std::map<std::string, std::string> &bypassMap) {
  std::string bypassBlock = "";
  if (goal->bypasses.size() > 1) {
    bypassBlock = "OR = {\n";
  }
  for (auto &bypassGroup : goal->bypasses) {
    for (auto bypass : bypassGroup.bypasses) {
      if (bypassMap.find(bypass.name) != bypassMap.end()) {
        auto blockText = bypassMap.at(bypass.name);

        // replace the country tag
        Fwg::Parsing::replaceOccurences(blockText, "templateTag", country->tag);
        // replace the target country tag
        if (goal->countryTarget) {
          Fwg::Parsing::replaceOccurences(blockText, "templateTargetTag",
                                          goal->countryTarget->tag);
        }
        // replace the target region ID
        if (goal->regionTarget) {
          Fwg::Parsing::replaceOccurences(
              blockText, "templateStateID",
              std::to_string(goal->regionTarget->ID + 1));
        }
        // replace the ideology
        Fwg::Parsing::replaceOccurences(
            blockText, "templateIdeology",
            Arda::Utils::ideologyToString.at(country->ideology));

        bypassBlock.append(blockText);
      }
    }
  }
  if (goal->bypasses.size() > 1) {
    bypassBlock += "\n\t\t}\n";
  }
  return bypassBlock;
}

std::string
addAiModifierBlocks(std::shared_ptr<Hoi4Country> country,
                    std::shared_ptr<Goal> goal,
                    const std::map<std::string, std::string> &aiModifierMap) {
  std::string aiModifierBlock = "";
  for (auto &aiModifier : goal->aiModifiers) {
    if (aiModifierMap.find(aiModifier.name) != aiModifierMap.end()) {
      auto blockText = aiModifierMap.at(aiModifier.name);

      // replace the country tag
      Fwg::Parsing::replaceOccurences(blockText, "templateTag", country->tag);
      // replace the target country tag
      if (goal->countryTarget) {
        Fwg::Parsing::replaceOccurences(blockText, "templateTargetTag",
                                        goal->countryTarget->tag);
      }
      // replace the target region ID
      if (goal->regionTarget) {
        Fwg::Parsing::replaceOccurences(
            blockText, "templateStateID",
            std::to_string(goal->regionTarget->ID + 1));
      }
      // replace the ideology
      Fwg::Parsing::replaceOccurences(
          blockText, "templateIdeology",
          Arda::Utils::ideologyToString.at(country->ideology));

      aiModifierBlock.append(blockText);
    }
  }
  return aiModifierBlock;
}

void evaluateCountryGoals(
    std::vector<std::shared_ptr<Hoi4Country>> &hoi4Countries,
    const std::vector<std::shared_ptr<Arda::ArdaRegion>> &ardaRegions) {
  Fwg::Utils::Logging::logLine("HOI4: Generating Country Goals");
  // get the base focus tree file
  const auto focusTreeBaseFile = Fwg::Parsing::readFile(
      Fwg::Cfg::Values().resourcePath + "hoi4/goals/focusTreeBase.txt");
  // get the base focus file
  const auto focusBaseFile = Fwg::Parsing::readFile(
      Fwg::Cfg::Values().resourcePath + "hoi4/goals/focusBase.txt");
  // get the effects file
  const auto effectDetailsFile = Fwg::Parsing::readFile(
      Fwg::Cfg::Values().resourcePath + "hoi4/goals/effectDetails.txt");
  // get the ideas file
  const auto ideaTemplateFile = Fwg::Parsing::readFile(
      Fwg::Cfg::Values().resourcePath + "hoi4/goals/ideaTemplates.txt");
  // get the avail file
  const auto availableBlocksFile = Fwg::Parsing::readFile(
      Fwg::Cfg::Values().resourcePath + "hoi4/goals/availableBlocks.txt");
  // get the bypass file
  const auto bypassBlocksFile = Fwg::Parsing::readFile(
      Fwg::Cfg::Values().resourcePath + "hoi4/goals/bypassBlocks.txt");
  // get the ai modifier file
  const auto aiModifierFile = Fwg::Parsing::readFile(
      Fwg::Cfg::Values().resourcePath + "hoi4/goals/aiModifiers.txt");

  Fwg::Utils::Logging::logLineLevel(5, "HOI4: Parsing availableMap");
  std::map<std::string, std::string> availableMap;
  auto availBlocks = Fwg::Parsing::getTokens(availableBlocksFile, ';');
  for (auto &avail : availBlocks) {
    if (avail.size() < 10)
      continue;
    auto parts = Fwg::Parsing::getTokens(avail, ',');
    // replace any special characters in key
    Fwg::Parsing::replaceOccurences(parts[0], "\n", "");
    availableMap[parts[0]] = parts[1];
  }

  Fwg::Utils::Logging::logLineLevel(5, "HOI4: Parsing bypassMap");
  std::map<std::string, std::string> bypassMap;
  auto bypassBlocks = Fwg::Parsing::getTokens(bypassBlocksFile, ';');
  for (auto &bypass : bypassBlocks) {
    if (bypass.size() < 10)
      continue;
    auto parts = Fwg::Parsing::getTokens(bypass, ',');
    // replace any special characters in key
    Fwg::Parsing::replaceOccurences(parts[0], "\n", "");
    bypassMap[parts[0]] = parts[1];
  }

  Fwg::Utils::Logging::logLineLevel(5, "HOI4: Parsing aiModifierMap");
  std::map<std::string, std::string> aiModifierMap;
  auto aiModifierBlocks = Fwg::Parsing::getTokens(aiModifierFile, ';');
  for (auto &aiModifier : aiModifierBlocks) {
    if (aiModifier.size() < 10)
      continue;
    auto parts = Fwg::Parsing::getTokens(aiModifier, ',');
    // replace any special characters in key
    Fwg::Parsing::replaceOccurences(parts[0], "\n", "");
    aiModifierMap[parts[0]] = parts[1];
  }

  Fwg::Utils::Logging::logLineLevel(5, "HOI4: Tokenizing effects");
  // tokenize effects file by ;
  auto effects = Fwg::Parsing::getTokens(effectDetailsFile, ';');
  // contains the key and the value is the effect which must be written into the
  // focus
  std::map<std::string, std::string> effectMap;
  for (auto &effect : effects) {
    if (effect.size() < 10)
      continue;
    auto parts = Fwg::Parsing::getTokens(effect, ',');
    // replace any special characters in key
    Fwg::Parsing::replaceOccurences(parts[0], "\n", "");
    effectMap[parts[0]] = parts[1];
  }
  Fwg::Utils::Logging::logLineLevel(5, "HOI4: Tokenizing ideas");
  std::map<std::string, std::string> ideaMap;
  auto ideaTemplates = Fwg::Parsing::getTokens(ideaTemplateFile, ';');
  for (auto &idea : ideaTemplates) {
    if (idea.size() < 10)
      continue;
    auto parts = Fwg::Parsing::getTokens(idea, ',');
    // replace any special characters in key
    Fwg::Parsing::replaceOccurences(parts[0], "\n", "");
    ideaMap[parts[0]] = parts[1];
  }

  GoalGeneration goalGen;
  goalGen.parseGoals(Fwg::Cfg::Values().resourcePath + "hoi4/goals/goals.txt");
  goalGen.evaluateGoals(hoi4Countries);
  goalGen.structureGoals(hoi4Countries);
  for (auto &countryGoals : goalGen.goalsByCountry) {
    Fwg::Utils::Logging::logLineLevel(
        9, "Evaluating country goals for country: ", countryGoals.first);
    auto &country = countryGoals.first;
    int idCounter = 0;
    std::string ideaBase = "ideas = {\n\tcountry = {\n";
    std::string focusTreeBase = focusTreeBaseFile;
    Fwg::Parsing::replaceOccurence(focusTreeBase, "templateFocusID",
                                   countryGoals.first->tag + "_focus");
    Fwg::Parsing::replaceOccurence(focusTreeBase, "templateCountryTag",
                                   country->tag);
    std::string focusList = "";
    for (auto &goal : countryGoals.second) {
      auto focusBase = focusBaseFile;
      // determine the name of the focus
      Fwg::Parsing::replaceOccurences(focusBase, "templateFocusId",
                                      goal->uniqueName);
      std::string prereqBlock = "";
      for (auto &prereq : goal->prerequisitesGoals) {
        prereqBlock.append("prerequisite = { focus = " + prereq->uniqueName +
                           " }\n\t\t");
      }
      Fwg::Parsing::replaceOccurences(focusBase, "templatePrerequisite",
                                      prereqBlock);
      Fwg::Parsing::replaceOccurences(focusBase, "templateXpos",
                                      std::to_string(goal->xPosition));
      Fwg::Parsing::replaceOccurences(focusBase, "templateYpos",
                                      std::to_string(goal->yPosition));
      if (goal->rootGoal != nullptr) {
        Fwg::Parsing::replaceOccurences(focusBase, "templateRootGoal",
                                        "relative_position_id = " +
                                            goal->rootGoal->uniqueName);
      } else {
        Fwg::Parsing::replaceOccurences(focusBase, "templateRootGoal", "");
      }

      for (auto &effectGroup : goal->effects) {
        std::string effectGroupText = "";
        for (auto &effect : effectGroup.effects) {
          if (effectMap.find(effect.name) != effectMap.end()) {
            auto focusEffectText = effectMap.at(effect.name);
            for (auto i = 0; i < effect.parameters.size(); i++) {
              Fwg::Parsing::replaceOccurences(
                  focusEffectText, "templateEffect" + std::to_string(i),
                  effect.parameters[i]);
            }
            if (goal->scope == GoalScope::Region) {
              // we need to replace the region name
              Fwg::Parsing::replaceOccurences(
                  focusEffectText, "templateStateID",
                  std::to_string(goal->regionTarget->ID + 1));
              effectGroupText.append(focusEffectText);
            } else {
              // in case of ideas, we have an indirection: the idea must first
              // be constructed for the country with the parameters of the
              // effect
              if (effect.name.contains("idea=")) {
                // remove the idea= part
                effect.name = effect.name.substr(5);
                // get the idea template
                auto ideaTemplate = ideaMap.at(effect.name);
                std::string ideaName = effect.name + "_" + country->tag + "_" +
                                       std::to_string(idCounter++);
                Fwg::Parsing::replaceOccurences(ideaTemplate,
                                                "templateIdeaName", ideaName);

                for (auto i = 0; i < effect.parameters.size(); i++) {
                  Fwg::Parsing::replaceOccurences(
                      ideaTemplate, "templateEffect" + std::to_string(i),
                      effect.parameters[i]);
                }
                ideaBase.append(ideaTemplate);
                Fwg::Parsing::replaceOccurence(focusEffectText, effect.name,
                                               ideaName);
                effectGroupText.append(focusEffectText);

              } else {
                // we need to replace the country tag
                Fwg::Parsing::replaceOccurences(focusEffectText, "templateTag",
                                                country->tag);
                // if the text contains templateTarget, we need to replace it by
                // the goal target
                if (focusEffectText.contains("templateTarget")) {
                  Fwg::Parsing::replaceOccurences(focusEffectText,
                                                  "templateTarget",
                                                  goal->countryTarget->tag);
                }
                for (auto i = 0; i < effect.parameters.size(); i++) {
                  Fwg::Parsing::replaceOccurences(
                      focusEffectText, "templateEffect" + std::to_string(i),
                      effect.parameters[i]);
                }
                effectGroupText.append(focusEffectText);
              }
            }
          }
        }
        // replace the effectGroupText in the focusBase
        Fwg::Parsing::replaceOccurences(focusBase, "templateEffectGroup",
                                        effectGroupText);
        auto availableBlock = addAvailableBlocks(country, goal, availableMap);
        Fwg::Parsing::replaceOccurences(focusBase, "templateAvailable",
                                        availableBlock);
        auto bypassBlock = addBypassBlocks(country, goal, bypassMap);
        Fwg::Parsing::replaceOccurences(focusBase, "templateBypass",
                                        bypassBlock);
        auto aiModifierBlock =
            addAiModifierBlocks(country, goal, aiModifierMap);
        Fwg::Parsing::replaceOccurences(focusBase, "templateAiModifiers",
                                        aiModifierBlock);
      }
      focusList.append(focusBase);
    }
    Fwg::Parsing::replaceOccurence(focusTreeBase, "templateFocusList",
                                   focusList);
    ideaBase.append("\n}\n}\n");
    country->focusTree = focusTreeBase;
    country->ideas = ideaBase;
  }
}

void generateFocusFiles(
    std::vector<std::shared_ptr<Hoi4Country>> &hoi4Countries) {
  auto &cfg = Fwg::Cfg::Values();
  std::string focusDir = cfg.resourcePath + "/hoi4/common/national_focus/";

  // Read template
  std::string templatePath = focusDir + "rpx_focus_template.txt";
  std::ifstream tFile(templatePath);
  if (!tFile.is_open()) {
    Fwg::Utils::Logging::logLine("Focus template not found: ", templatePath);
    return;
  }
  std::string templateContent((std::istreambuf_iterator<char>(tFile)),
                              std::istreambuf_iterator<char>());
  tFile.close();

  // Scan and group focus files by category
  std::map<std::string, std::vector<std::string>> categoryFiles;
  for (auto &entry : std::filesystem::directory_iterator(focusDir)) {
    std::string name = entry.path().filename().string();
    // Match rpx_{category}_{N}.txt
    std::smatch m;
    if (std::regex_match(name, m, std::regex("rpx_(\\w+)_\\d+\\.txt"))) {
      categoryFiles[m[1].str()].push_back(entry.path().string());
    }
  }

  if (categoryFiles.empty()) {
    Fwg::Utils::Logging::logLine("No focus tree files found in ", focusDir);
    return;
  }

  for (auto &country : hoi4Countries) {
    // Randomly select one file per category
    std::vector<std::string> selectedFiles;
    for (auto &[cat, files] : categoryFiles) {
      int idx = RandNum::getRandom(static_cast<int>(files.size()));
      selectedFiles.push_back(files[idx]);
      std::cout << idx << ";";
    }
    std::cout << std::endl;

    // Process selected files: extract focuses, offset x positions
    std::string combinedFocuses;
    int cumulativeX = 0;
    int prevRootX = 0;
    int prevMaxX = 0;
    const int BUFFER = 7;
    bool first = true;

    for (auto &filePath : selectedFiles) {
      std::ifstream file(filePath);
      if (!file.is_open())
        continue;
      std::string content((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
      file.close();

      int treeMinX = 0, treeMaxX = 0;
      std::regex xRegex("x\\s*=\\s*(-?\\d+)");
      auto itBegin =
          std::sregex_iterator(content.begin(), content.end(), xRegex);
      auto itEnd = std::sregex_iterator();
      for (auto it = itBegin; it != itEnd; ++it) {
        int x = std::stoi((*it)[1].str());
        treeMaxX = std::max(treeMaxX, x);
        treeMinX = std::min(treeMinX, x);
      }

      if (first) {
        cumulativeX = 0;
        first = false;
      } else {
        cumulativeX = prevRootX + prevMaxX - treeMinX + BUFFER;
      }

      // Extract focus = { ... } blocks by brace matching
      std::vector<std::string> focusBlocks;
      size_t pos = 0;
      while (true) {
        pos = content.find("focus = {", pos);
        if (pos == std::string::npos)
          break;
        size_t start = pos;
        pos += 9; // skip "focus = {" (9 chars)
        int depth = 1;
        while (depth > 0 && pos < content.size()) {
          if (content[pos] == '{')
            ++depth;
          else if (content[pos] == '}')
            --depth;
          ++pos;
        }
        focusBlocks.push_back(content.substr(start, pos - start));
      }

      Fwg::Utils::Logging::logLine(
          "  Focus tree file: ", filePath, " blocks=", focusBlocks.size(),
          " minX=", treeMinX, " maxX=", treeMaxX, " cumX=", cumulativeX);
      // Only offset x for root focuses (no prerequisite).
      // Children use relative_position_id so their x stays as-is.
      for (auto &block : focusBlocks) {
        if (block.find("prerequisite") == std::string::npos) {
          // This is a ROOT focus — offset its x by cumulativeX
          std::string result;
          size_t lastPos = 0;
          auto rBegin =
              std::sregex_iterator(block.begin(), block.end(), xRegex);
          auto rEnd = std::sregex_iterator();
          for (auto it = rBegin; it != rEnd; ++it) {
            result += block.substr(lastPos, it->position() - lastPos);
            int x = std::stoi((*it)[1].str());
            Fwg::Utils::Logging::logLine("    root block: old x=", x,
                                         " + cumX=", cumulativeX,
                                         " = new x=", x + cumulativeX);
            result += "x = " + std::to_string(x + cumulativeX);
            lastPos = it->position() + it->length();
          }
          result += block.substr(lastPos);
          combinedFocuses += result + "\n";
        } else {
          // CHILD focus — keep original position
          combinedFocuses += block + "\n";
        }
      }

      prevRootX = cumulativeX;
      prevMaxX = treeMaxX;
    }

    // Build final focus tree per country
    std::regex refRegex(
        "((?:id\\s*=\\s*|focus\\s*=\\s*|relative_position_id\\s*=\\s*))"
        "rpx_([a-zA-Z0-9_]+)");
    // Tag all rpx_ references with country tag to ensure uniqueness
    std::string taggedFocuses = std::regex_replace(combinedFocuses, refRegex,
                                                   "$1rpx_$2_" + country->tag);
    std::string result = templateContent;
    Fwg::Parsing::replaceOccurences(result, "templateTag", country->tag);
    Fwg::Parsing::replaceOccurence(result, "templateFocusses", taggedFocuses);
    country->focusTree = result;
  }
}

} // namespace Rpx::Hoi4::FocusGen