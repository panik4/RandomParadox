#include "hoi4/Hoi4FocusGen.h"

namespace Rpx::Hoi4::FocusGen {
NationalFocus FocusGen::buildFocus(const std::vector<std::string> chainStep,
                                   const Hoi4Country &source,
                                   const Hoi4Country &target) {
  // map the string of the chainstep to the type
  auto type = NationalFocus::typeMapping[chainStep[5]];
  auto dateTokens =
      Arda::Parsing::getNumbers(chainStep[8], '-', std::set<int>{});
  NationalFocus nF(type, false, source.tag, target.tag, dateTokens);

  auto predecessors = Arda::Parsing::getNumbers(
      Rpx::Parsing::getBracketBlockContent(chainStep[2],
                                                     "predecessor"),
      ',', std::set<int>());
  for (const auto &predecessor : predecessors)
    nF.precedingFoci.push_back(predecessor);

  // now get the "xor"...
  auto exclusives = Arda::Parsing::getNumbers(
      Rpx::Parsing::getBracketBlockContent(chainStep[7], "exclusive"),
      ',', std::set<int>());
  for (const auto &exclusive : exclusives)
    nF.xorFoci.push_back(exclusive);
  // and "and" foci
  auto ands = Arda::Parsing::getNumbers(
      Rpx::Parsing::getBracketBlockContent(chainStep[7], "and"), ',',
      std::set<int>());
  for (const auto &anddd : ands)
    nF.andFoci.push_back(anddd);
  // and "or" foci
  auto ors = Arda::Parsing::getNumbers(
      Rpx::Parsing::getBracketBlockContent(chainStep[7], "or"), ',',
      std::set<int>());
  for (const auto &orrr : ors)
    nF.orFoci.push_back(orrr);
  // add completion reward keys
  auto available = Fwg::Parsing::getTokens(
      Rpx::Parsing::getBracketBlockContent(chainStep[9], "available"),
      '+');
  for (const auto &availKey : available) {
    nF.available.push_back(availKey);
  }
  // add completion reward keys
  auto bypasses = Fwg::Parsing::getTokens(
      Rpx::Parsing::getBracketBlockContent(chainStep[10], "bypass"),
      '+');
  for (const auto &bypassKey : bypasses) {
    nF.bypasses.push_back(bypassKey);
  }
  // add completion reward keys
  auto rewards =
      Fwg::Parsing::getTokens(Rpx::Parsing::getBracketBlockContent(
                                  chainStep[11], "completion_reward"),
                              '+');
  for (const auto &rewardKey : rewards) {
    nF.completionRewards.push_back(rewardKey);
  }
  // save the level for later evaluation
  auto level = std::stoi(chainStep[12]);
  nF.level = level;
  return nF;
}

void mergeFocusBranches(Hoi4Country &source) {

  // The National Foci we can attach this chain to
  std::vector<int> attacheableTo;
  for (auto &currentBranch : source.focusBranches) {
    bool requirementsFulfilled = true;
    // try to find a preceding branch
    for (auto &reqBranch : currentBranch.requiredPreceding) {
      bool found = false;

      for (int i = 0; i < source.focusBranches.size(); i++) {
        if (source.focusBranches[i].ID == reqBranch) {
          found = true;
          attacheableTo.push_back(i);
        }
      }
      if (!found)
        requirementsFulfilled = false;
    }
    // we couldn't find all of the required branches, invalidate the branch
    // completely!
    if (!requirementsFulfilled) {
      currentBranch.ID = -1;
      currentBranch.foci.clear();
      continue;
    }
    // gather additional optional branches to attach to, this can also happen if
    // NO required branch is used
    for (auto &optBranch : currentBranch.optionalPreceding) {
      for (auto i = 0; i < source.focusBranches.size(); i++) {
        if (source.focusBranches[i].ID == optBranch) {
          attacheableTo.push_back(i);
        }
      }
    }

    // now, where do we attach?
    // nothing to attach to, just go back, this branch will be placed
    // automatically on the lowest level
    if (!attacheableTo.size())
      continue;

    // randomly attach to one of the branches
    auto &branchToAttachTo =
        source.focusBranches[Fwg::Utils::selectRandom(attacheableTo)];
    // take the last focus level of the branch, so we can use these as
    // predecessors, however, we need to focus on the ORIGINALLY last foci, as
    // we might've already attached another branch
    std::vector<NationalFocus> lastFocusLevel;
    int maxLevel = branchToAttachTo[branchToAttachTo.attachPoint].level;
    for (int f = branchToAttachTo.size() - 1; f >= 0; f--) {
      if (branchToAttachTo.foci[f].level == maxLevel) {
        lastFocusLevel.push_back(branchToAttachTo[f]);
      }
    }
    auto origBranchSize = branchToAttachTo.size();
    for (auto &focus : currentBranch.foci) {
      // modify the focus and copy it to other branch, that we attach to
      // update the ID in the chain, so the order is correct
      focus.stepID += origBranchSize;
      focus.level += maxLevel + 1;
      // now we need to change the predecessor, it must be attached to the
      // attachpoint focus, therefore we take the ID of the attach point as a
      // predecessor.
      if (!focus.precedingFoci.size()) {
        // we have no predecessor as this is the first focus of the branch
        // therefore, we set the predecessor to the last of the previous branch
        // now find additional ones, as the previous focus might be in and "and"
        // or
        // "or" relation to other foci. In case of and, we need to set BOTH as
        // predecessors, with none skippable, in the case of OR we need to set
        // both as skippable as well
        for (auto &prevFocus : lastFocusLevel) {
          focus.precedingFoci.push_back(prevFocus.stepID);
          // focus.precedingFoci.push_back(prevFocus.stepID);
        }
      } else {
        for (auto &previousPredecessor : focus.precedingFoci) {
          // +1 due to starting stepID count from 0
          previousPredecessor += branchToAttachTo.attachPoint + 1;
        }
      }
      branchToAttachTo.foci.push_back(focus);
    }

    currentBranch.foci.clear();
    currentBranch.ID = -1;
  }
}

void buildFocusTree(Hoi4Country &source) {
  mergeFocusBranches(source);
  // std::array<std::array<int, 100>, 100> occupiedPositions;
  // start left. Chains go down, new chains go right
  int curX = 1;
  int curY = 1;
  int maxX = 1;
  // try sorting it, because otherwise the ordering of the tree is weird.
  // Either reorder by level, or change stepIDs to match the levels. But then,
  // we also have to reorder the references to stepIDs
  // TODO: sorting algo
  // std::sort(source.focusBranches.begin(), source.focusBranches.end(),
  //          source.focusBranches.begin(), )

  for (auto &focusBranch : source.focusBranches) {
    curY = 1;
    std::set<int> fociIDs;
    std::array<std::set<int>, 100> levels;
    int index = 0;
    int width = 0;

    for (auto &focus : focusBranch.foci) {
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
        if (chainStepID < focusBranch.size()) {
          for (auto stepID : focusBranch[chainStepID].xorFoci) {
            levels[index].insert(stepID);
            fociIDs.insert(stepID);
          }
          for (auto stepID : focusBranch[chainStepID].andFoci) {
            levels[index].insert(stepID);
            fociIDs.insert(stepID);
          }
          for (auto stepID : focusBranch[chainStepID].orFoci) {
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
        focusBranch.foci.at(index++).position = {curX += 2, curY};
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