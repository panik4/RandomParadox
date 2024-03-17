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
  std::map<Fwg::ClimateGeneration::Detail::TreeType, double> treeEffects;
  bool considerSea = false;
  double oceanFactor = 0.0;
  double lakeFactor = 0.0;
};

struct Resource {
  std::string name;
  bool capped;
  double amount;
};

using CTI = Fwg::ClimateGeneration::Detail::ClimateTypeIndex;
static std::vector<ResConfig> resConfigs{
    {"bg_coal_mining", true, 5.0, true, defaultNoise},
    {"bg_gold_mining", true, 0.2, true, rareNoise},
    {"bg_gold_fields", true, 0.2, true, rareNoise},
    {"bg_iron_mining", true, 5.0, true, defaultNoise},
    {"bg_lead_mining", true, 2.0, true, semiRareNoise},
    {"bg_oil_extraction", true, 1.0, true, rareLargePatch},
    {"bg_sulfur_mining", true, 2.0, true, semiRareNoise},
    {"bg_logging",
     true,
     10.0,
     false,
     agriNoise,
     true,
     {{CTI::TROPICSMONSOON, 1.0},
      {CTI::TROPICSRAINFOREST, 0.8},
      {CTI::TROPICSSAVANNA, 0.8},
      {CTI::COLDSEMIARID, 0.1},
      {CTI::HOTSEMIARID, 0.1},
      {CTI::CONTINENTALCOLD, 1.0},
      {CTI::CONTINENTALHOT, 1.0},
      {CTI::CONTINENTALWARM, 0.8},
      {CTI::TEMPERATECOLD, 1.0},
      {CTI::TEMPERATEWARM, 0.8},
      {CTI::TEMPERATEHOT, 0.6}}},
    {"bg_livestock_ranches",
     false,
     10.0,
     false,
     agriNoise,
     true,
     {{CTI::POLARTUNDRA, 0.2},
      {CTI::TROPICSMONSOON, 0.7},
      {CTI::TROPICSRAINFOREST, 0.6},
      {CTI::TROPICSSAVANNA, 0.8},
      {CTI::COLDSEMIARID, 0.5},
      {CTI::HOTSEMIARID, 0.4},
      {CTI::CONTINENTALCOLD, 0.9},
      {CTI::CONTINENTALHOT, 0.9},
      {CTI::CONTINENTALWARM, 0.9},
      {CTI::TEMPERATECOLD, 1.0},
      {CTI::TEMPERATEWARM, 1.0},
      {CTI::TEMPERATEHOT, 1.0}}},
    {"bg_rubber",
     true,
     5.0,
     false,
     agriNoise,
     true,
     {{CTI::TROPICSMONSOON, 1.0},
      {CTI::TROPICSRAINFOREST, 0.8},
      {CTI::TROPICSSAVANNA, 0.5}}},
    {"bg_banana_plantations",
     false,
     10.0,
     false,
     agriNoise,
     true,
     {{CTI::TROPICSMONSOON, 1.0},
      {CTI::TROPICSRAINFOREST, 1.0},
      {CTI::TROPICSSAVANNA, 0.9}}},
    {"bg_coffee_plantations",
     false,
     5.0,
     false,
     agriNoise,
     true,
     {{CTI::TROPICSMONSOON, 1.0},
      {CTI::TROPICSRAINFOREST, 1.0},
      {CTI::TROPICSSAVANNA, 1.0},
      {CTI::HOTSEMIARID, 1.0}}},
    {"bg_cotton_plantations",
     false,
     10.0,
     false,
     agriNoise,
     true,
     {{CTI::TEMPERATEHOT, 0.8},
      {CTI::HOTSEMIARID, 1.0},
      {CTI::TROPICSMONSOON, 0.8},
      {CTI::TROPICSRAINFOREST, 0.4},
      {CTI::TROPICSSAVANNA, 0.4}}},
    {"bg_dye_plantations",
     false,
     5.0,
     false,
     agriNoise,
     true,
     {{CTI::HOTSEMIARID, 1.0},
      {CTI::TROPICSMONSOON, 0.8},
      {CTI::TROPICSRAINFOREST, 0.4},
      {CTI::TROPICSSAVANNA, 0.4}}},
    {"bg_opium_plantations",
     false,
     4.0,
     false,
     agriNoise,
     true,
     {{CTI::HOTSEMIARID, 0.8}}},
    {"bg_silk_plantations",
     false,
     5.0,
     false,
     agriNoise,
     true,
     {{CTI::TEMPERATEHOT, 1.0}}},
    {"bg_sugar_plantations",
     false,
     1.0,
     false,
     agriNoise,
     true,
     {{CTI::TEMPERATEHOT, 0.8},
      {CTI::HOTSEMIARID, 1.0},
      {CTI::TROPICSMONSOON, 0.8},
      {CTI::TROPICSRAINFOREST, 0.4},
      {CTI::TROPICSSAVANNA, 0.4}}},
    {"bg_tea_plantations",
     false,
     1.0,
     false,
     agriNoise,
     true,
     {{CTI::HOTSEMIARID, 1.0},
      {CTI::TROPICSMONSOON, 1.0},
      {CTI::TROPICSRAINFOREST, 0.4},
      {CTI::TROPICSSAVANNA, 0.6}}},
    {"bg_tobacco_plantations",
     false,
     1.0,
     false,
     agriNoise,
     true,
     {{CTI::HOTSEMIARID, 1.0},
      {CTI::TROPICSMONSOON, 0.8},
      {CTI::TROPICSRAINFOREST, 0.4},
      {CTI::TROPICSSAVANNA, 0.4}}},
    {"bg_vineyard_plantations",
     false,
     1.0,
     false,
     agriNoise,
     true,
     {{CTI::TEMPERATEHOT, 1.0}, {CTI::TEMPERATEWARM, 0.8}}},
    {"bg_wheat_farms",
     false,
     1.0,
     false,
     agriNoise,
     true,
     {{CTI::CONTINENTALHOT, 1.0},
      {CTI::CONTINENTALWARM, 0.8},
      {CTI::TEMPERATEHOT, 1.0},
      {CTI::TEMPERATEWARM, 0.8},
      {CTI::HOTSEMIARID, 0.8}}},
    {"bg_maize_farms",
     false,
     1.0,
     false,
     agriNoise,
     true,
     {{CTI::TROPICSMONSOON, 1.0},
      {CTI::TROPICSRAINFOREST, 0.8},
      {CTI::TROPICSSAVANNA, 1.0},
      {CTI::CONTINENTALHOT, 1.0},
      {CTI::CONTINENTALWARM, 0.8},
      {CTI::TEMPERATEHOT, 1.0},
      {CTI::TEMPERATEWARM, 0.8},
      {CTI::HOTSEMIARID, 0.8}}},
    {"bg_millet_farms",
     false,
     1.0,
     false,
     agriNoise,
     true,
     {{CTI::TROPICSMONSOON, 1.0},
      {CTI::TROPICSRAINFOREST, 0.8},
      {CTI::TROPICSSAVANNA, 1.0},
      {CTI::HOTSEMIARID, 0.8}}},
    {"bg_rice_farms",
     false,
     1.0,
     false,
     agriNoise,
     true,
     {{CTI::TROPICSMONSOON, 1.0},
      {CTI::TROPICSRAINFOREST, 0.8},
      {CTI::TEMPERATEWARM, 0.8},
      {CTI::HOTSEMIARID, 0.8}}},
    {"bg_rye_farms",
     false,
     1.0,
     false,
     agriNoise,
     true,
     {{CTI::CONTINENTALCOLD, 1.0},
      {CTI::CONTINENTALWARM, 0.8},
      {CTI::TEMPERATECOLD, 1.0},
      {CTI::COLDSEMIARID, 0.8}}},
    {"bg_fishing",
     true,
     1.0,
     false,
     defaultNoise,
     false,
     {},
     false,
     {},
     true,
     1.0,
     1.0},
    {"bg_whaling",
     true,
     1.0,
     false,
     defaultNoise,
     false,
     {},
     false,
     {},
     true,
     1.0,
     0.0}};

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

} // namespace Scenario::Vic3