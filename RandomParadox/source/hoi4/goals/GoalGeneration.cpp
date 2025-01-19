#include "hoi4/goals/GoalGeneration.h"

void Scenario::Hoi4::GoalGeneration::parseGoals(const std::string &path) {
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
        // check if this contains a (, if so get the content from there on until
        // excluding the )
        if (effectPart.find("(") != std::string::npos) {
          auto start = effectPart.find("(");
          auto end = effectPart.find(")");
          e.name = effectPart.substr(0, start).substr(4);
          e.parameters.push_back(effectPart.substr(start + 1, end - start - 1));
        } else {
          e.name = effectPart;
        }
        eg.effects.push_back(e);
      }
      goal.effects.push_back(eg);
    }

    potentialGoals.push_back(goal);
    goalsByType[type].push_back(goal);
  }
}

void Scenario::Hoi4::GoalGeneration::evaluateGoals(
    std::vector<std::shared_ptr<Hoi4Country>> &hoi4Countries) {
  for (auto &country : hoi4Countries) {
    auto economicGoals = goalsByType.at("cat:economic");
    for (auto i = 0; i < 20; i++) {
      // select a random goal from a copy of the goal
      auto economicGoal = Fwg::Utils::selectRandom(economicGoals);
      // for (auto &goal : economicGoals) {
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
            targetRegion = Scenario::Hoi4::Selectors::getRandomRegion(*country);
          }
          if (selector.name == "self") {
            targetCountry = country;
          }
        }
      }
      // now check if we found anything
      if (targetRegion) {
        // we can now create a RegionGoal from the economicGoal
        auto regionGoal = std::make_shared<Scenario::Hoi4::Goal>();
        regionGoal->name = economicGoal.name;
        regionGoal->type = economicGoal.type;
        regionGoal->scope = GoalScope::Region;
        regionGoal->prerequisites = economicGoal.prerequisites;
        regionGoal->selectors = economicGoal.selectors;
        regionGoal->effects = economicGoal.effects;
        regionGoal->regionTarget = targetRegion;
        // we now modify the goals effects, by selecting from the effects a
        // random parameter
        Scenario::Hoi4::Effects::constructEffects(regionGoal->effects);
        goalsByCountry[country].push_back(regionGoal);
      }
      if (targetCountry) {
        // we can now create a CountryGoal from the economicGoal
        auto countryGoal = std::make_shared<Scenario::Hoi4::Goal>();
        countryGoal->name = economicGoal.name;
        countryGoal->type = economicGoal.type;
        countryGoal->scope = GoalScope::Country;
        countryGoal->prerequisites = economicGoal.prerequisites;
        countryGoal->selectors = economicGoal.selectors;
        countryGoal->effects = economicGoal.effects;
        countryGoal->countryTarget = targetCountry;
        // we now modify the goals effects, by selecting from the effects a
        // random parameter
        Scenario::Hoi4::Effects::constructEffects(countryGoal->effects);
        goalsByCountry[country].push_back(countryGoal);
      }
    }
  }
}
