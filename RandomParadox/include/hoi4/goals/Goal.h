#pragma once
#include "hoi4/Hoi4Country.h"
#include <string>
#include <vector>
namespace Scenario::Hoi4 {
enum class GoalType {
  Undefined,
  Political,
  Military,
  Economic,
  ForeignPolicy,
  ForeignPolicyOffensive,
  Research
};
enum class GoalScope { Undefined, Country, Region };

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

// determines the availability of a goal
struct Availability {
  std::string name;
  std::string parameter;
};

struct AvailabilityGrouping {
  std::vector<Availability> availabilities;
};

// to be able to skip a goal if a certain condition is met
struct Bypass {
  std::string name;
  std::string parameter;
};

struct BypassGrouping {
  std::vector<Bypass> bypasses;
};

struct AiModifier {
  std::string name;
};

class Goal {
public:
  std::string name = "";
  std::string uniqueName = "";
  // to determine ordering of goals
  int priority = 999;
  GoalType type = GoalType::Undefined;
  GoalScope scope = GoalScope::Undefined;
  std::vector<PrerequisiteGrouping> prerequisites;
  std::vector<SelectorGrouping> selectors;
  std::vector<EffectGrouping> effects;
  std::vector<AvailabilityGrouping> availabilities;
  std::vector<AiModifier> aiModifiers;
  std::vector<BypassGrouping> bypasses;
  std::shared_ptr<Region> regionTarget;
  std::shared_ptr<Hoi4Country> countryTarget;
  std::shared_ptr<Goal> rootGoal;
  std::vector<std::shared_ptr<Goal>> prerequisitesGoals;
  std::vector<std::shared_ptr<Goal>> successorsGoals;
  int xPosition = 0;
  int yPosition = 0;
};

enum class GroupingType { Undefined, Alternatives, Successors };
struct AlternativeGoalGrouping {

  // std::vector<Goal> goals;
};

} // namespace Scenario::Hoi4