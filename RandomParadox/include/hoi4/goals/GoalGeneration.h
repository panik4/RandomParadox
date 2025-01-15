#pragma once
#include "hoi4/goals/Goal.h"
#include "hoi4/goals/Selectors.h"

namespace Scenario::Hoi4 {

class GoalGeneration {
public:
  // a list of ALL the potential goals, together with their prerequisites and
  // selectors
  std::vector<Goal> potentialGoals;
  // a sorted representation of the potential goals, sorted by type
  std::map<std::string, std::vector<Goal>> goalsByType;

  std::map<std::string, std::vector<Goal>> goalsByCountry;

  std::map<std::string, GoalType> goalTypeMap = {
      {"cat:political", GoalType::Political},
      {"cat:military", GoalType::Military},
      {"cat:economy", GoalType::Economic},
      {"cat:foreign_policy", GoalType::ForeignPolicy},
      {"cat:research", GoalType::Research}};

  // std::vector<std::string> parsedPrerequisites;
  // std::vector<std::string> parsedSelectors;
  // std::vector<std::string> parsedGoals;

  void parseGoals(const std::string &path);
  //void evaluateGoals(std::map<std::string, Hoi4Country> &hoi4Countries);
};

} // namespace Scenario::Hoi4