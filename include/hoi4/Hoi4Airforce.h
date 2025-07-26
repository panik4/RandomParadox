#pragma once
#include "hoi4/Hoi4Tech.h"
#include <array>
#include <set>
#include <unordered_set>
#include <string>
#include <vector>
namespace Rpx::Hoi4 {

enum class PlaneType { SmallFrame, MediumFrame, LargeFrame };
enum class PlaneRole {
  Fighter,
  Cas,
  NavalBomber,
  TacticalBomber,
  StrategicBomber,
  CarrierCas,
  CarrierFighter,
  CarrierNavalBomber
};

struct PlaneVariant {
  PlaneType type;
  PlaneRole subType;
  TechEra era;
  std::string name;
  std::string vanillaFrameName;
  std::string bbaFrameName;
  std::map<std::string, std::string> bbaModules;
  double cost = 1.0;
  int amount = 0;
};

struct AirWing {
  std::string name;
  PlaneRole role;
  PlaneVariant variant;
  int amount;
};

struct AirBase {
  int level;
  std::vector<AirWing> wings;
  int provinceID = 0;
};

void adjustTechsForPlaneModules(
    std::map<TechEra, std::vector<Technology>> &availableModuleTech);

void addPlaneModules(
    PlaneVariant &tankVariant,
    const std::map<TechEra, std::vector<Technology>> &availableModuleTech);

} // namespace Rpx::Hoi4