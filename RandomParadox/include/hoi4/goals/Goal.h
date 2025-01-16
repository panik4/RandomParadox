#pragma once
#include <string>
#include <vector>
#include "hoi4/Hoi4Country.h"
namespace Scenario::Hoi4 {
enum class GoalType { Political, Military, Economic, ForeignPolicy, Research };

struct Prerequisite {
  std::string name;
  std::string parameter;
};
struct PrerequisiteGrouping {
  std::vector<Prerequisite> prerequisites;

};
struct Selector {
  std::string name;
  std::string parameter;
};
struct SelectorGrouping {
  std::vector<Selector> selectors;
};

struct Effect {
  std::map<std::string, std::vector<std::string>> effects;
};

class Goal {
public:
  std::string name;
  GoalType type;
  std::vector<PrerequisiteGrouping> prerequisites;
  std::vector<SelectorGrouping> selectors;
  std::vector<Effect> effects;
};


class CountryGoal : public Goal {
public:
  std::shared_ptr<Hoi4Country> target;
};



} // namespace Scenario::Hoi4