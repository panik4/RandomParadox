#pragma once
#include "hoi4/Hoi4Region.h"
#include "hoi4/Hoi4Tech.h"
#include <array>
#include <string>
#include <vector>

namespace Scenario::Hoi4 {

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
  // this is the level between two major upgrades, and determines for bbaModules
  // if we take some of the modules from the next era, and for vanilla how many
  // upgrades have happened. Range between 0.0 and 1.0
  double upgradeLevel = 0.0;
};

void addPlaneModules(
    PlaneVariant &tankVariant,
    const std::map<TechEra, std::vector<Technology>> &availableModuleTech);

} // namespace Scenario::Hoi4