#pragma once
#include "hoi4/Hoi4Region.h"
#include "hoi4/Hoi4Tech.h"
#include <array>
#include <string>
#include <vector>

namespace Rpx::Hoi4 {

enum class ArmorType { LightArmor, MediumArmor, HeavyArmor };
enum class ArmorRole { Tank, TankDestroyer, AntiAir, Artillery };

struct TankVariant {
  ArmorType type;
  ArmorRole subType;
  TechEra era;
  std::string name;
  std::string vanillaArmorName;
  std::string bbaArmorName;
  std::map<std::string, std::string> nsbModules;
  // this is the level between two major upgrades, and determines for nsbModules
  // if we take some of the modules from the next era, and for vanilla how many
  // upgrades have happened. Range between 0.0 and 1.0
  double upgradeLevel = 0.0;
};

void addArmorModules(
    TankVariant &tankVariant,
    const std::map<TechEra, std::vector<Technology>> &availableModuleTech);

} // namespace Rpx::Hoi4