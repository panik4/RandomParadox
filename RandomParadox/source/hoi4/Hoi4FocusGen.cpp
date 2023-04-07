#include "hoi4/Hoi4FocusGen.h"

namespace Scenario::Hoi4::FocusGen {
NationalFocus FocusGen::buildFocus(const std::vector<std::string> chainStep,
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
  for (const auto &anddd : ands)
    nF.andFoci.push_back(anddd);
  // and "or" foci
  auto ors = ParserUtils::getNumbers(
      ParserUtils::getBracketBlockContent(chainStep[7], "or"), ',',
      std::set<int>());
  for (const auto &orrr : ors)
    nF.orFoci.push_back(orrr);
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

void FocusGen::buildFocusTree(Hoi4Country &source) {
  // std::array<std::array<int, 100>, 100> occupiedPositions;
  // start left. Chains go down, new chains go right
  int curX = 1;
  int curY = 1;
  int maxX = 1;
  if (source.tag == "DIA")
    Fwg::Utils::Logging::logLine("AA");
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

bool FocusGen::stepFulfillsRequirements(
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
bool FocusGen::targetFulfillsRequirements(
    const std::string &targetRequirements, const Hoi4Country &source,
    const Hoi4Country &target,
    const std::vector<std::set<std::string>> &levelTargets,
    const std::vector<std::shared_ptr<Scenario::Region>>& gameRegions,
    const int level) {
  const auto &cfg = Fwg::Cfg::Values();
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
      auto maxDistance = sqrt(cfg.width * cfg.height) * 0.2;
      if (Fwg::getPositionDistance(
              gameRegions[source.capitalRegionID]->position,
              gameRegions[target.capitalRegionID]->position,
              cfg.width) > maxDistance)
        return false;
    }
    if (value == "far") {
      auto minDistance = sqrt(cfg.width * cfg.height) * 0.2;
      if (Fwg::getPositionDistance(
              gameRegions[source.capitalRegionID]->position,
              gameRegions[target.capitalRegionID]->position,
              cfg.width) < minDistance)
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

void FocusGen::evaluateCountryGoals(
    std::map<std::string, Hoi4Country>& hoi4Countries,
    const std::vector<std::shared_ptr<Scenario::Region>>& gameRegions) {
  Fwg::Utils::Logging::logLine("HOI4: Generating Country Goals");
  std::vector<int> defDate{1, 1, 1936};
  std::vector<std::vector<std::vector<std::string>>> chains;

  chains.push_back(ParserUtils::getLinesByID(
      "resources\\hoi4\\ai\\national_focus\\chains\\major_chains.txt"));
  chains.push_back(ParserUtils::getLinesByID(
      "resources\\hoi4\\ai\\national_focus\\chains\\regional_chains.txt"));
  chains.push_back(ParserUtils::getLinesByID(
      "resources\\hoi4\\ai\\national_focus\\chains\\army_chains.txt"));
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
          Fwg::Utils::Logging::logLineLevel(9, chainFocus);
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
                          destCountry.second, levelTargets, gameRegions,
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
          Fwg::Utils::Logging::logLineLevel(5, "Building focus");
          std::map<int, NationalFocus> fulfilledSteps;
          int stepIndex = -1;
          std::vector<NationalFocus> chainFoci;

          for (auto &targets : stepTargets) {
            stepIndex++;
            if (!targets.size())
              continue;
            // select random target
            const auto &target = Fwg::Utils::selectRandom(targets);
            auto focus{buildFocus(ParserUtils::getTokens(chain[stepIndex], ';'),
                                  hoi4Countries.at(sourceCountry.first),
                                  target)};
            focus.stepID = stepIndex;
            focus.chainID = chainID;
            Fwg::Utils::Logging::logLineLevel(1, focus);
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

} // namespace Scenario::Hoi4