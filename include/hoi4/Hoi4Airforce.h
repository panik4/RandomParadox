#pragma once
#include "hoi4/Hoi4Tech.h"
#include "utils/Archive.h"
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

  void serialise(Fwg::Utils::Serialisation::Archive &ar) {
    ar.serialiseEnum(type);
    ar.serialiseEnum(subType);
    ar.serialiseEnum(era);
    ar &name &vanillaFrameName &bbaFrameName;
    ar &bbaModules &cost &amount;
  }
  void deserialise(Fwg::Utils::Serialisation::Archive &ar) {
    serialise(ar);
  }
};

struct AirWing {
  std::string name;
  PlaneRole role;
  PlaneVariant variant;
  int amount;

  void serialise(Fwg::Utils::Serialisation::Archive &ar) {
    ar &name;
    ar.serialiseEnum(role);
    variant.serialise(ar);
    ar &amount;
  }
  void deserialise(Fwg::Utils::Serialisation::Archive &ar) {
    serialise(ar);
  }
};

struct AirBase {
  int level;
  std::vector<AirWing> wings;
  int regionID = 0;
  int provinceID = 0;

  void serialise(Fwg::Utils::Serialisation::Archive &ar) {
    ar &level &regionID &provinceID;
    ar &wings;
  }
  void deserialise(Fwg::Utils::Serialisation::Archive &ar) {
    serialise(ar);
  }
};

void adjustTechsForPlaneModules(
    std::map<TechEra, std::vector<Technology>> &availableModuleTech);

void addPlaneModules(
    PlaneVariant &tankVariant,
    const std::map<TechEra, std::vector<Technology>> &availableModuleTech);

} // namespace Rpx::Hoi4