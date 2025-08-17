#include "hoi4/Hoi4FocusGen.h"

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
        Fwg::Parsing::replaceOccurences(blockText, "templateIdeology",
                                        country->ideology);

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
        Fwg::Parsing::replaceOccurences(blockText, "templateIdeology",
                                        country->ideology);

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
      Fwg::Parsing::replaceOccurences(blockText, "templateIdeology",
                                      country->ideology);

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

} // namespace Rpx::Hoi4::FocusGen