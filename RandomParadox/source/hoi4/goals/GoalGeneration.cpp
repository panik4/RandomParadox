#include "hoi4/goals/GoalGeneration.h"

void Scenario::Hoi4::GoalGeneration::parseGoals(const std::string &path) {
  auto goalLines = Fwg::Parsing::getLines(path);
  for (auto &goalLine : goalLines) {
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

    potentialGoals.push_back(goal);
    goalsByType[type].push_back(goal);
  }
}