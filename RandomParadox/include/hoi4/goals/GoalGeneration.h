#pragma once
#include "hoi4/goals/Effects.h"
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

  std::map<std::shared_ptr<Hoi4Country>, std::vector<std::shared_ptr<Goal>>>
      goalsByCountry;

  std::map<std::string, GoalType> goalTypeMap = {
      {"cat:political", GoalType::Political},
      {"cat:military", GoalType::Military},
      {"cat:economic", GoalType::Economic},
      {"cat:foreign_policy", GoalType::ForeignPolicy},
      {"cat:foreign_policy_offensive", GoalType::ForeignPolicyOffensive},
      {"cat:research", GoalType::Research}};

  void parseGoals(const std::string &path);

  void gatherAggressionGoals(
      const std::vector<std::shared_ptr<Hoi4Country>> &hoi4Countries);

  void constructGoal(std::shared_ptr<Hoi4Country> sourceCountry,
                     std::shared_ptr<Scenario::Hoi4::Region> targetRegion,
                     std::shared_ptr<Scenario::Hoi4::Hoi4Country> targetCountry,
                     const Goal &goal);

  void evaluateGoals(std::vector<std::shared_ptr<Hoi4Country>> &hoi4Countries);
};

} // namespace Scenario::Hoi4