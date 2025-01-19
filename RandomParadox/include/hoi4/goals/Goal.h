#pragma once
#include "hoi4/Hoi4Country.h"
#include <string>
#include <vector>
namespace Scenario::Hoi4 {
enum class GoalType { Political, Military, Economic, ForeignPolicy, Research };
enum class GoalScope { Country, Region };

struct Prerequisite {
  std::string name;
  std::string parameter;
};
// contains a vector of prerequisites, which are in an AND relationship
struct PrerequisiteGrouping {
  std::vector<Prerequisite> prerequisites;
};
struct Selector {
  std::string name;
  std::string parameter;
};
// contains a vector of selectors, which are in an AND relationship
struct SelectorGrouping {
  std::vector<Selector> selectors;
};

// contains a name and parameters. Parameters are in string format as they can
// be anything
struct Effect {
  std::string name;
  std::vector<std::string> parameters;
};
// contains a vector of effects, which are in an AND relationship
struct EffectGrouping {
  std::vector<Effect> effects;
};

class Goal {
public:
  std::string name;
  GoalType type;
  GoalScope scope;
  std::vector<PrerequisiteGrouping> prerequisites;
  std::vector<SelectorGrouping> selectors;
  std::vector<EffectGrouping> effects;
  std::shared_ptr<Region> regionTarget;
  std::shared_ptr<Hoi4Country> countryTarget;
};

} // namespace Scenario::Hoi4