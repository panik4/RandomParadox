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
    // now gather limits
    for (size_t i = 2; i < goalParts.size(); i++) {
      if (goalParts[i].find("limit:") != std::string::npos) {
        auto limitParts = Fwg::Parsing::getTokens(goalParts[i], ':');
        goal.limit = std::stoi(limitParts[1]);
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
      [&](std::shared_ptr<Scenario::Hoi4::Goal> &goalPtr, std::shared_ptr<Hoi4Country> country) {
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
        goalPtr->limit = goal.limit;
        Scenario::Hoi4::Effects::constructEffects(goalPtr->effects, country);
      };

  if (targetRegion) {
    auto regionGoal = std::make_shared<Scenario::Hoi4::Goal>();
    regionGoal->scope = GoalScope::Region;
    regionGoal->regionTarget = targetRegion;
    setCommonAttributes(regionGoal, nullptr);
    goalsByCountry[sourceCountry].push_back(regionGoal);
  }

  if (targetCountry) {
    auto countryGoal = std::make_shared<Scenario::Hoi4::Goal>();
    countryGoal->scope = GoalScope::Country;
    countryGoal->countryTarget = targetCountry;
    setCommonAttributes(countryGoal, targetCountry);
    goalsByCountry[sourceCountry].push_back(countryGoal);
  }
}
bool Scenario::Hoi4::GoalGeneration::checkPrerequisites(
    const Goal &categoryGoal, const std::shared_ptr<Hoi4Country> &country,
    std::vector<Goal> &categoryGoals) {
  bool valid = true;
  for (const auto &prerequisiteGroup : categoryGoal.prerequisites) {
    for (const auto &prerequisite : prerequisiteGroup.prerequisites) {
      if (prerequisite.name == "is_major" &&
          country->rank != Rank::GreatPower) {
        valid = false;
        break;
      } else if (prerequisite.name == "is_secondary" &&
                 country->rank != Rank::SecondaryPower) {
        valid = false;
        break;
      } else if (prerequisite.name == "is_regional" &&
                 country->rank != Rank::RegionalPower) {
        valid = false;
        break;
      } else if (prerequisite.name == "is_local" &&
                 country->rank != Rank::LocalPower) {
        valid = false;
        break;
      } else if (prerequisite.name == "is_minor" &&
                 country->rank != Rank::MinorPower) {
        valid = false;
        break;
      }
    }
  }
  if (!valid) {
    // remove the goal from the list if the name is the same
    categoryGoals.erase(std::remove_if(categoryGoals.begin(),
                                       categoryGoals.end(),
                                       [&](const auto &goal) {
                                         return goal.name == categoryGoal.name;
                                       }),
                        categoryGoals.end());
  }
  return valid;
}

// Method to check if the goal limit is reached
bool Scenario::Hoi4::GoalGeneration::isGoalLimitReached(
    const Goal &categoryGoal, const std::shared_ptr<Hoi4Country> &country,
    std::vector<Goal> &categoryGoals) {
  if (categoryGoal.limit != 999) {
    int count = 0;
    for (const auto &goal : goalsByCountry[country]) {
      if (goal->name == categoryGoal.name) {
        count++;
      }
    }
    if (count >= categoryGoal.limit) {
      // remove the goal from the list if the name is the same
      categoryGoals.erase(
          std::remove_if(
              categoryGoals.begin(), categoryGoals.end(),
              [&](const auto &goal) { return goal.name == categoryGoal.name; }),
          categoryGoals.end());
      return true;
    }
  }
  return false;
}

void Scenario::Hoi4::GoalGeneration::evaluateGoals(
    std::vector<std::shared_ptr<Hoi4Country>> &hoi4Countries) {
  Fwg::Utils::Logging::logLineLevel(5, "HOI4: Evaluating goals");
  int counter = 0;
  std::vector<std::string> simpleCategories = {"cat:economic", "cat:political",
                                               "cat:army",     "cat:navy",
                                               "cat:air",      "cat:research"};
  for (auto &country : hoi4Countries) {
    for (auto &category : simpleCategories) {
      auto categoryGoals = goalsByType[category];
      for (auto i = 0; i < 20; i++) {
        if (categoryGoals.empty()) {
          break;
        }
        // select a random goal from a copy of the goal
        auto categoryGoal = Fwg::Utils::selectRandom(categoryGoals);

        // check if we fulfill the prerequisites
        bool valid = checkPrerequisites(categoryGoal, country, categoryGoals);

        if (!valid) {
          i--;
          continue;
        }

        // if the goal has a limit, check if it is already reached
        if (isGoalLimitReached(categoryGoal, country, categoryGoals)) {
          i--;
          continue;
        }
        // run the selector
        std::shared_ptr<Scenario::Hoi4::Region> targetRegion;
        std::shared_ptr<Scenario::Hoi4::Hoi4Country> targetCountry;

        for (auto &selectorGroup : categoryGoal.selectors) {
          for (auto &selector : selectorGroup.selectors) {
            if (selector.name == "get_random_state") {
              targetRegion =
                  Scenario::Hoi4::Selectors::getRandomRegion(country);
            } else if (selector.name == "get_random_coastal_state") {
              targetRegion =
                  Scenario::Hoi4::Selectors::getRandomRegion(country);
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
        constructGoal(country, targetRegion, targetCountry, categoryGoal,
                      counter);
      }
    }

    std::vector<std::string> foreignPolicyCategories = {
        "cat:foreign_policy_offensive", "cat:foreign_policy"};
    for (auto &category : foreignPolicyCategories) {
      auto categoryGoals = goalsByType[category];
      for (auto i = 0; i < 20; i++) {
        for (auto &categoryGoal : categoryGoals) {
          // check if we fulfill the prerequisites
          bool valid = checkPrerequisites(categoryGoal, country, categoryGoals);

          if (!valid) {
            i--;
            continue;
          }

          // if the goal has a limit, check if it is already reached
          if (isGoalLimitReached(categoryGoal, country, categoryGoals)) {
            i--;
            continue;
          }

          // now different approach: we take the selector, and depending on what
          // it is, we gather ALL possible targets
          std::vector<std::shared_ptr<Scenario::Hoi4::Region>> targetRegions;
          std::vector<std::shared_ptr<Scenario::Hoi4::Hoi4Country>>
              targetCountries;
          for (auto &selectorGroup : categoryGoal.selectors) {
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
              } else if (selector.name == "self") {
                targetCountries.push_back(country);
              }
            }
          }
          // now we have all possible targets, we can now create a goal for each
          // of them
          for (auto &targetCountry : targetCountries) {
            constructGoal(country, nullptr, targetCountry, categoryGoal,
                          counter);
          }
          // remove the goal from the list if the name is the same, we do this
          // as each foreign policy goal is only evaluated once and applied to
          // all potential targets
          categoryGoals.erase(
              std::remove_if(categoryGoals.begin(), categoryGoals.end(),
                             [&](const auto &goal) {
                               return goal.name == categoryGoal.name;
                             }),
              categoryGoals.end());
        }
      }
    }
  }
}

struct Position {
  int x;
  int y;
  bool operator<(const Position &other) const {
    return (y == other.y) ? (x < other.x) : (y < other.y);
  }
};

void Scenario::Hoi4::GoalGeneration::structureGoals(
    std::vector<std::shared_ptr<Hoi4Country>> &hoi4Countries) {

  std::random_device rd;
  std::mt19937 gen(rd());

  for (auto &countryGoals : goalsByCountry) {
    int totalXOffset = 5;

    // Assign random priorities
    for (auto &goal : countryGoals.second) {
      if (goal->priority == 999) {
        goal->priority = std::uniform_int_distribution<>(100, 999)(gen);
      }
    }

    // Group goals by type
    std::map<GoalType, std::vector<std::shared_ptr<Goal>>> goalsByType;
    for (auto &goal : countryGoals.second) {
      goalsByType[goal->type].push_back(goal);
    }

    // Handle each category
    for (auto &[type, goals] : goalsByType) {
      // Sort goals by priority
      std::sort(goals.begin(), goals.end(), [](const auto &a, const auto &b) {
        return a->priority < b->priority;
      });

      std::set<std::shared_ptr<Goal>> usedGoals;
      std::vector<std::shared_ptr<Goal>> frontier;
      std::set<Position> usedPositions;

      // Place root
      auto root = goals.front();
      root->xPosition = 0;
      root->yPosition = 0;
      // root->rootGoal = root;
      usedPositions.insert({root->xPosition, root->yPosition});

      usedGoals.insert(root);
      frontier.push_back(root);

      // Build tree
      for (size_t i = 1; i < goals.size(); ++i) {
        auto currentGoal = goals[i];

        // Select random parent from frontier
        std::uniform_int_distribution<> dist(
            0, static_cast<int>(frontier.size()) - 1);
        int parentIndex = dist(gen);
        auto parent = frontier[parentIndex];

        // Positioning
        int targetX = parent->xPosition;
        int targetY = parent->yPosition + 1;

        // Collision avoidance
        while (usedPositions.count({targetX, targetY})) {
          if (targetX < parent->xPosition) {
            targetX -= 2;
          } else {
            targetX += 2;
          }
        }

        // Set position
        currentGoal->xPosition = targetX;
        currentGoal->yPosition = targetY;
        usedPositions.insert({targetX, targetY});

        // Connect goals
        parent->successorsGoals.push_back(currentGoal);
        currentGoal->prerequisitesGoals.push_back(parent);
        currentGoal->rootGoal = root;

        usedGoals.insert(currentGoal);

        // Second Parent Possibility
        if (std::uniform_int_distribution<>(1, 3)(gen) == 1) { // 33% chance
          auto secondParent =
              findSecondParent(frontier, parent, usedGoals, targetX);
          if (secondParent) {
            secondParent->successorsGoals.push_back(currentGoal);
            currentGoal->prerequisitesGoals.push_back(secondParent);
          }
        }
        if (currentGoal->successorsGoals.size() < 2) {
          frontier.push_back(currentGoal);
        }
        // if we have a second parent, make sure our y is also larger than
        // theirs
        if (currentGoal->prerequisitesGoals.size() == 2) {
          currentGoal->yPosition =
              std::max<int>(currentGoal->prerequisitesGoals[0]->yPosition,
                            currentGoal->prerequisitesGoals[1]->yPosition) +
              1;
        }

        // Remove parent if full
        if (parent->successorsGoals.size() >= 2) {
          frontier.erase(frontier.begin() + parentIndex);
        }
      }
      // determine width of tree
      int minX = 0;
      int maxX = 0;
      for (auto &goal : goals) {
        minX = std::min<int>(minX, goal->xPosition);
        maxX = std::max<int>(maxX, goal->xPosition);
      }
      // center root
      root->xPosition = (maxX - minX) / 2 + totalXOffset;

      totalXOffset += (maxX - minX) + 15; // Add padding
                                          // checks:
      // 1. if a goal has 2 prerequisites, it should be below both
      // 2. if a goal has 2 successors, it should be above both
      // 3. If a goal has prerequisites, it must not point to itself
      // 4. If a goal has successors, it must not point to itself

      for (auto &goal : goals) {
        if (goal->prerequisitesGoals.size() == 2) {
          // check if we are below both
          if (goal->prerequisitesGoals[0]->yPosition > goal->yPosition &&
              goal->prerequisitesGoals[1]->yPosition > goal->yPosition) {
            // we are good
          } else {
            //std::cout << "ERROR: Goal " << goal->name
            //          << " has 2 prerequisites, but is not below both"
            //          << std::endl;
          }
        }
        if (goal->successorsGoals.size() == 2) {
          // check if we are above both
          if (goal->successorsGoals[0]->yPosition < goal->yPosition &&
              goal->successorsGoals[1]->yPosition < goal->yPosition) {
            // we are good
          } else {
            //std::cout << "ERROR: Goal " << goal->name
            //          << " has 2 successors, but is not above both"
            //          << std::endl;
          }
        }
        if (goal->prerequisitesGoals.size() > 0) {
          if (goal->prerequisitesGoals[0] == goal) {
            std::cout << "ERROR: Goal " << goal->name
                      << " has a prerequisite pointing to itself" << std::endl;
          }
        }
        if (goal->successorsGoals.size() > 0) {
          if (goal->successorsGoals[0] == goal) {
            std::cout << "ERROR: Goal " << goal->name
                      << " has a successor pointing to itself" << std::endl;
          }
        }
      }
    }

    // Sort all country goals by priority
    std::sort(
        countryGoals.second.begin(), countryGoals.second.end(),
        [](const auto &a, const auto &b) { return a->priority < b->priority; });
  }
}

std::shared_ptr<Scenario::Hoi4::Goal>
Scenario::Hoi4::GoalGeneration::findSecondParent(
    const std::vector<std::shared_ptr<Goal>> &frontier,
    const std::shared_ptr<Goal> &primaryParent,
    const std::set<std::shared_ptr<Goal>> &usedGoals, int currentX) {

  std::vector<std::shared_ptr<Goal>> candidates;
  for (const auto &goal : frontier) {
    if (goal != primaryParent && goal->successorsGoals.size() < 2 &&
        goal->prerequisitesGoals.size() < 2 &&
        std::abs(goal->xPosition - currentX) <= 5) { // Close in xPosition
      candidates.push_back(goal);
    }
  }

  if (!candidates.empty()) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(
        0, static_cast<int>(candidates.size()) - 1);
    return candidates[dist(gen)];
  }
  return nullptr;
}
