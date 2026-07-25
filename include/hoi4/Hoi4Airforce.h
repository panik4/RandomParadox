#pragma once
#include "hoi4/Hoi4Tech.h"
#include "utils/SerialisationFwd.h"
#include <algorithm>
#include <array>
#include <set>
#include <string>
#include <unordered_set>
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

  template<class Archive>
  void serialize(Archive &ar, const unsigned int /*version*/) {
    ar &type &subType &era &name &vanillaFrameName &bbaFrameName;
    ar &bbaModules &cost &amount;
  }
};

struct AirWing {
  std::string name;
  PlaneRole role;
  PlaneVariant variant;
  int amount;

  template<class Archive>
  void serialize(Archive &ar, const unsigned int /*version*/) {
    ar &name &role &variant &amount;
  }
};

struct AirBase {
  int level;
  std::vector<AirWing> wings;
  int regionID = 0;
  int provinceID = 0;

  template<class Archive>
  void serialize(Archive &ar, const unsigned int /*version*/) {
    ar &level &regionID &provinceID &wings;
  }
};

void adjustTechsForPlaneModules(
    std::map<TechEra, std::vector<Technology>> &availableModuleTech);

void addPlaneModules(
    PlaneVariant &tankVariant,
    const std::map<TechEra, std::vector<Technology>> &availableModuleTech);

} // namespace Rpx::Hoi4