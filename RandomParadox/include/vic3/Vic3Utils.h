#pragma once
#include "FastWorldGenerator.h"
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
namespace Scenario::Vic3 {

struct NoiseConfig {
  double fractalFrequency;
  double tanFactor;
  double cutOff;
  double mountainBonus;
};
static NoiseConfig defaultNoise{0.02, 0.0, 0.8, 2.0};
static NoiseConfig semiRareNoise{0.015, 0.0, 0.85, 2.0};
static NoiseConfig rareLargePatch{0.005, 0.0, 0.7, 0.0};
static NoiseConfig rareNoise{0.01, 0.0, 0.9, 2.0};
static NoiseConfig agriNoise{0.24, 0.0, 0.0, 0.0};

struct ResConfig {
  std::string name;
  bool capped;
  double resourcePrevalence;
  bool random = false;
  NoiseConfig noiseConfig;
  bool considerClimate = false;
  std::map<Fwg::ClimateGeneration::Detail::ClimateTypeIndex, double>
      climateEffects;
  bool considerTrees = false;
  std::map<Fwg::ClimateGeneration::Detail::TreeTypeIndex, double> treeEffects;
  bool considerSea = false;
  double oceanFactor = 0.0;
  double lakeFactor = 0.0;
};

struct Resource {
  std::string name;
  bool capped;
  double amount;
};


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