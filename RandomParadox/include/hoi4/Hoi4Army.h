#pragma once
#include "generic/GameProvince.h"
#include "hoi4/Hoi4Armor.h"
#include "hoi4/Hoi4Tech.h"
#include <array>
#include <string>
#include <vector>

namespace Scenario::Hoi4 {
enum class RegimentType {
  Irregulars,
  Infantry,
  Mountaineers,
  Cavalry,
  Motorized,
  LightArmor,
  Artillery,
  Paratroopers,
  Marines
};

enum class SupportRegimentType {
  Engineer,
  Recon,
  Artillery,
  MotorizedAntiTank,
  MotorizedAntiAir,
  MotorizedArtillery
};

enum class DivisionType {
  Irregulars,
  Militia,
  Infantry,
  SupportedInfantry,
  HeavyArtilleryInfantry,
  Cavalry,
  Motorized,
  Armor
};

struct DivisionTemplate {
  DivisionType type;
  std::string name;
  std::vector<std::vector<RegimentType>> regiments;
  std::vector<SupportRegimentType> supportRegiments;
  double cost = 4.0;
};
struct Division {
  DivisionTemplate divisionTemplate;
  std::string name;
  GameProvince location;
  double startingExperienceFactor;
  double startingEquipmentFactor;
};

std::vector<DivisionTemplate> createDivisionTemplates(
    const std::vector<DivisionType> &desiredTypes,
    const std::vector<RegimentType> &availableRegimentTypes,
    const std::vector<SupportRegimentType> &availableSupportRegimentTypes);

} // namespace Scenario::Hoi4