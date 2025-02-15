#include "hoi4/goals/GoalGeneration.h"

void Scenario::Hoi4::GoalGeneration::parseGoals(const std::string &path) {
  Fwg::Utils::Logging::logLineLevel(5, "HOI4: Parsing goals");
  auto goalLines = Fwg::Parsing::getLines(path);
  for (auto &goalLine : goalLines) {
    if (goalLine.empty()) {
      continue;
    }
    Goal goal;
    auto goalParts = Fwg::Parsing::getTokens(goalLine, ';');
    goal.name = goalParts[0];
    auto type = goalParts[1];
    goal.type = goalTypeMap[type];
    // now get all prerequisites, by finding every token containing prq:
    std::vector<std::string> prerequisites;
    for (size_t i = 2; i < goalParts.size(); i++) {
      if (goalParts[i].find("prq:") != std::string::npos) {
        prerequisites.push_back(goalParts[i]);
      }
    }
    std::vector<std::string> selectors;
    for (size_t i = 2; i < goalParts.size(); i++) {
      if (goalParts[i].find("sel:") != std::string::npos) {
        selectors.push_back(goalParts[i]);
      }
    }
    std::vector<std::string> effects;
    for (size_t i = 2; i < goalParts.size(); i++) {
      if (goalParts[i].find("eff:") != std::string::npos) {
        effects.push_back(goalParts[i]);
      }
    }
    // find priority value, if exists
    for (size_t i = 2; i < goalParts.size(); i++) {
      if (goalParts[i].find("prio:") != std::string::npos) {
        auto priorityParts = Fwg::Parsing::getTokens(goalParts[i], ':');
        goal.priority = std::stoi(priorityParts[1]);
      }
    }

    // gather bypass blocks
    std::vector<std::string> bypasses;
    for (size_t i = 2; i < goalParts.size(); i++) {
      if (goalParts[i].find("bypass:") != std::string::npos) {
        bypasses.push_back(goalParts[i]);
      }
    }
    // now go through all bypasses, and split them into more tokens by &
    for (auto &bypass : bypasses) {
      BypassGrouping bg;
      auto bypassParts = Fwg::Parsing::getTokens(bypass, '&');
      for (auto &bypassPart : bypassParts) {
        Bypass b;
        // take the first part as the name, but remove the prq: part
        b.name = bypassPart.substr(7);
        bg.bypasses.push_back(b);
      }
      goal.bypasses.push_back(bg);
    }

    // gather availability blocks
    std::vector<std::string> availabilities;
    for (size_t i = 2; i < goalParts.size(); i++) {
      if (goalParts[i].find("avail:") != std::string::npos) {
        availabilities.push_back(goalParts[i]);
      }
    }
    // now go through all availabilities, and split them into more tokens by &
    for (auto &availability : availabilities) {
      AvailabilityGrouping ag;
      auto availabilityParts = Fwg::Parsing::getTokens(availability, '&');
      for (auto &availabilityPart : availabilityParts) {
        Availability a;
        // take the first part as the name, but remove the prq: part
        a.name = availabilityPart.substr(6);
        ag.availabilities.push_back(a);
      }
      goal.availabilities.push_back(ag);
    }

    // now go through all prerequisites, and split them into more tokens by &
    for (auto &prerequisite : prerequisites) {
      PrerequisiteGrouping pg;
      auto prerequisiteParts = Fwg::Parsing::getTokens(prerequisite, '&');
      for (auto &prerequisitePart : prerequisiteParts) {
        Prerequisite p;
        auto pParts = Fwg::Parsing::getTokens(prerequisitePart, '/');
        // take the first part as the name, but remove the prq: part
        p.name = pParts[0].substr(4);
        // only if we can find parameters
        if (pParts.size() > 1) {
          p.parameter = pParts[1];
        }
        pg.prerequisites.push_back(p);
      }
      goal.prerequisites.push_back(pg);
    }
    // now go through all selectors, and split them into more tokens by &
    for (auto &selector : selectors) {
      SelectorGrouping sg;
      auto selectorParts = Fwg::Parsing::getTokens(selector, '&');
      for (auto &selectorPart : selectorParts) {
        Selector s;
        auto sParts = Fwg::Parsing::getTokens(selectorPart, '/');
        // take the first part as the name, but remove the sel: part
        s.name = sParts[0].substr(4);
        // only if we can find parameters
        if (sParts.size() > 1) {
          s.parameter = sParts[1];
        }
        sg.selectors.push_back(s);
      }
      goal.selectors.push_back(sg);
    }
    // now go through all effects, and split them into more tokens by &
    for (auto &effect : effects) {
      EffectGrouping eg;
      auto effectParts = Fwg::Parsing::getTokens(effect, '&');
      for (auto &effectPart : effectParts) {
        Effect e;
        // check if this contains a (, if so get the content from there on
        // until excluding the )
        if (effectPart.find("(") != std::string::npos) {
          auto start = effectPart.find("(");
          auto end = effectPart.find(")");
          e.name = effectPart.substr(0, start).substr(4);
          e.parameters.push_back(effectPart.substr(start + 1, end - start - 1));
        } else {
          e.name = effectPart.substr(4);
        }
        eg.effects.push_back(e);
      }
      goal.effects.push_back(eg);
    }
    // now gather AiModifiers
    for (size_t i = 2; i < goalParts.size(); i++) {
      if (goalParts[i].find("ai:") != std::string::npos) {
        auto aiModifierParts = Fwg::Parsing::getTokens(goalParts[i], ':');
        AiModifier aiModifier;
        aiModifier.name = aiModifierParts[1];
        goal.aiModifiers.push_back(aiModifier);
      }
    }

    potentialGoals.push_back(goal);
    goalsByType[type].push_back(goal);
  }
}

void Scenario::Hoi4::GoalGeneration::gatherAggressionGoals(
    const std::vector<std::shared_ptr<Hoi4Country>> &hoi4Countries) {}

void Scenario::Hoi4::GoalGeneration::constructGoal(
    std::shared_ptr<Hoi4Country> sourceCountry,
    std::shared_ptr<Scenario::Hoi4::Region> targetRegion,
    std::shared_ptr<Scenario::Hoi4::Hoi4Country> targetCountry,
    const Goal &goal, int &idCounter) {

  auto setCommonAttributes =
      [&](std::shared_ptr<Scenario::Hoi4::Goal> &goalPtr) {
        goalPtr->name = goal.name;

        goalPtr->uniqueName = sourceCountry->tag + "_" + goalPtr->name + "_" +
                              std::to_string(idCounter++);
        goalPtr->type = goal.type;
        goalPtr->prerequisites = goal.prerequisites;
        goalPtr->selectors = goal.selectors;
        goalPtr->effects = goal.effects;
        goalPtr->bypasses = goal.bypasses;
        goalPtr->availabilities = goal.availabilities;
        goalPtr->aiModifiers = goal.aiModifiers;
        Scenario::Hoi4::Effects::constructEffects(goalPtr->effects);
      };

  if (targetRegion) {
    auto regionGoal = std::make_shared<Scenario::Hoi4::Goal>();
    regionGoal->scope = GoalScope::Region;
    regionGoal->regionTarget = targetRegion;
    setCommonAttributes(regionGoal);
    goalsByCountry[sourceCountry].push_back(regionGoal);
  }

  if (targetCountry) {
    auto countryGoal = std::make_shared<Scenario::Hoi4::Goal>();
    countryGoal->scope = GoalScope::Country;
    countryGoal->countryTarget = targetCountry;
    setCommonAttributes(countryGoal);
    goalsByCountry[sourceCountry].push_back(countryGoal);
  }
}

void Scenario::Hoi4::GoalGeneration::evaluateGoals(
    std::vector<std::shared_ptr<Hoi4Country>> &hoi4Countries) {
  Fwg::Utils::Logging::logLineLevel(5, "HOI4: Evaluating goals");
  int counter = 0;
  for (auto &country : hoi4Countries) {
    auto economicGoals = goalsByType["cat:economic"];
    for (auto i = 0; i < 20; i++) {
      if (economicGoals.empty()) {
        break;
      }
      // select a random goal from a copy of the goal
      auto economicGoal = Fwg::Utils::selectRandom(economicGoals);
      // check for each of the prerequisite groups if it is valid
      bool valid = true;
      for (auto &prerequisiteGroup : economicGoal.prerequisites) {
        for (auto &prerequisite : prerequisiteGroup.prerequisites) {
        }
      }
      // run the selector
      std::shared_ptr<Scenario::Hoi4::Region> targetRegion;
      std::shared_ptr<Scenario::Hoi4::Hoi4Country> targetCountry;

      for (auto &selectorGroup : economicGoal.selectors) {
        for (auto &selector : selectorGroup.selectors) {
          if (selector.name == "get_random_state") {
            targetRegion = Scenario::Hoi4::Selectors::getRandomRegion(country);
          } else if (selector.name == "get_random_coastal_state") {
            targetRegion = Scenario::Hoi4::Selectors::getRandomRegion(country);
          }

          if (selector.name == "self") {
            targetCountry = country;
          }
          if (selector.name == "get_opposing_ideology_neighbour") {
            // targetCountry =
            //     Scenario::Hoi4::Selectors::getOpposingIdeologyNeighbour(hoi4Countries);
          }
        }
      }
      constructGoal(country, targetRegion, targetCountry, economicGoal,
                    counter);
    }
    auto foreignPolicyGoals = goalsByType["cat:foreign_policy_offensive"];
    // group all foreignPolicyGoals by priority
    std::map<int, std::vector<Goal>> foreignPolicyGoalsByPriority;
    for (auto &foreignPolicyGoal : foreignPolicyGoals) {
      foreignPolicyGoalsByPriority[foreignPolicyGoal.priority].push_back(
          foreignPolicyGoal);
    }
    // now start iterating from 0
    for (auto i = 0; i < 3; i++) {
      if (foreignPolicyGoalsByPriority.find(i) ==
          foreignPolicyGoalsByPriority.end()) {
        continue;
      }
      for (auto &foreignPolicyGoal : foreignPolicyGoalsByPriority[i]) {
        // check for each of the prerequisite groups if it is valid
        bool valid = true;
        for (auto &prerequisiteGroup : foreignPolicyGoal.prerequisites) {
          for (auto &prerequisite : prerequisiteGroup.prerequisites) {
          }
        }
        // now different approach: we take the selector, and depending on what
        // it is, we gather ALL possible targets
        std::vector<std::shared_ptr<Scenario::Hoi4::Region>> targetRegions;
        std::vector<std::shared_ptr<Scenario::Hoi4::Hoi4Country>>
            targetCountries;
        for (auto &selectorGroup : foreignPolicyGoal.selectors) {
          for (auto &selector : selectorGroup.selectors) {
            if (selector.name == "get_opposing_ideology_neighbour") {
              targetCountries =
                  Scenario::Hoi4::Selectors::getOpposingIdeologyNeighbours(
                      country, hoi4Countries);
            } else if (selector.name ==
                       "get_opposing_ideology_neighbours_neighbour") {
              targetCountries = Scenario::Hoi4::Selectors::
                  getOpposingIdeologyNeighboursNeighbours(country,
                                                          hoi4Countries);
            }
          }
        }
        // now we have all possible targets, we can now create a goal for each
        // of them
        for (auto &targetCountry : targetCountries) {
          constructGoal(country, nullptr, targetCountry, foreignPolicyGoal,
                        counter);
        }
      }
    }
  }
}

void Scenario::Hoi4::GoalGeneration::structureGoals(
    std::vector<std::shared_ptr<Hoi4Country>> &hoi4Countries) {

  std::random_device rd;
  std::mt19937 gen(rd());

  // X offset for categories
  int totalXOffset = 0;

  for (auto &countryGoals : goalsByCountry) {
    // Assign random priorities if needed
    for (auto &goal : countryGoals.second) {
      if (goal->priority == 999) {
        goal->priority = std::uniform_int_distribution<>(100, 999)(gen);
      }
    }

    // Sort goals by category
    std::map<GoalType, std::vector<std::shared_ptr<Goal>>> goalsByType;
    for (auto &goal : countryGoals.second) {
      goalsByType[goal->type].push_back(goal);
    }

    // Structure goals for each category
    for (auto &[type, goals] : goalsByType) {

      // Sort goals by priority (lowest first)
      std::sort(goals.begin(), goals.end(), [](const auto &a, const auto &b) {
        return a->priority < b->priority;
      });

      std::set<std::shared_ptr<Goal>> usedGoals;

      // Find roots (highest priority)
      std::vector<std::shared_ptr<Goal>> roots;
      for (auto &goal : goals) {
        if (goal->priority == goals.front()->priority) {
          roots.push_back(goal);
          usedGoals.insert(goal);
        }
      }

      // Position roots and build their trees
      for (auto &root : roots) {
        root->xPosition = totalXOffset + 5; // Start category with padding
        root->yPosition = 0;

        int nextXOffset = 0;
        buildGoalTree(root, goals, usedGoals, gen, nextXOffset, 1);

        // Add spacing for the next category
        totalXOffset += nextXOffset + 10;
      }
    }
  }
}

void Scenario::Hoi4::GoalGeneration::buildGoalTree(
    std::shared_ptr<Goal> currentGoal,
    const std::vector<std::shared_ptr<Goal>> &goals,
    std::set<std::shared_ptr<Goal>> &usedGoals, std::mt19937 &gen, int &xOffset,
    int depth) {

  // Find possible successors
  std::vector<std::shared_ptr<Goal>> possibleSuccessors;
  for (const auto &goal : goals) {
    if (usedGoals.find(goal) == usedGoals.end() &&
        goal->priority > currentGoal->priority &&
        goal->prerequisitesGoals.size() < 2) { // Max 2 predecessors
      possibleSuccessors.push_back(goal);
    }
  }

  if (possibleSuccessors.empty()) {
    return;
  }

  // Randomize order
  std::shuffle(possibleSuccessors.begin(), possibleSuccessors.end(), gen);
  int numSuccessors = std::min<int>(static_cast<int>(possibleSuccessors.size()), 2);

  // Track sibling horizontal positions
  int localXOffset = 0;
  int spacing = 5; // Horizontal spacing between siblings

  for (int i = 0; i < numSuccessors; ++i) {
    auto successor = possibleSuccessors[i];

    // Relative positioning from root
    successor->xPosition = currentGoal->xPosition + localXOffset;
    successor->yPosition = currentGoal->yPosition + 1;

    currentGoal->successorsGoals.push_back(successor);
    successor->prerequisitesGoals.push_back(currentGoal);
    usedGoals.insert(successor);

    // Recurse
    buildGoalTree(successor, goals, usedGoals, gen, localXOffset, depth + 1);

    // Move horizontally for next sibling
    localXOffset += spacing;
  }

  // Random merge with another branch
  if (possibleSuccessors.size() > numSuccessors) {
    auto mergeCandidate = possibleSuccessors[numSuccessors];
    if (std::uniform_int_distribution<>(1, 4)(gen) == 1 &&
        mergeCandidate->prerequisitesGoals.size() < 2) {
      mergeCandidate->prerequisitesGoals.push_back(currentGoal);
      currentGoal->successorsGoals.push_back(mergeCandidate);
      mergeCandidate->xPosition = currentGoal->xPosition;
      mergeCandidate->yPosition = currentGoal->yPosition + 1;
      usedGoals.insert(mergeCandidate);
    }
  }

  // Update total offset for siblings
  xOffset += localXOffset;
}