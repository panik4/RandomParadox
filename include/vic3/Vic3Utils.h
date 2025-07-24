#pragma once
#include "FastWorldGenerator.h"
#include <generic/ScenarioUtils.h>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
namespace Scenario::Vic3 {

struct DiplomaticRelation {};
struct Technology {
  std::string name;
  std::string era;
  std::vector<std::string> unlockingTechnologies;
};

struct TechnologyLevel {
  std::string name;
  std::string era_researched;
  std::vector<Technology> technologies;
};

struct Good {
  std::string name;
  int cost;
};

struct PopNeed {
  std::string name;
  std::vector<Good> goods;
};
struct Buypackage {
  std::string name;
  std::vector<std::pair<PopNeed, int>> popNeeds;
};
struct Productionmethod {
  std::string name;
  std::vector<std::pair<Good, int>> inputs;
  std::vector<std::pair<Good, int>> outputs;
  std::map<std::string, Technology> unlockingTechnologies;
};

struct ProductionmethodGroup {
  std::string name;
  std::map<std::string, Productionmethod> productionMethods;
};

enum class BuildingCategory : int { PRIMARY, SECONDARY, TERTIARY };

struct BuildingType {
  std::string name;
  std::string group;
  std::vector<Technology> unlockTechs;
  std::vector<ProductionmethodGroup> productionMethodGroups;
  // for a simpler mapping
  std::map<std::string, Productionmethod> productionMethods;
  BuildingCategory category = BuildingCategory::PRIMARY;
};

struct Building {
  BuildingType type;
  int level;
  Productionmethod prodMethod;
};

std::vector<double> shiftedGaussian(double input);

} // namespace Scenario::Vic3