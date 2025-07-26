#pragma once
#include "areas/ArdaProvince.h"
#include "hoi4/Hoi4Armor.h"
#include "hoi4/Hoi4Tech.h"
#include "hoi4/Hoi4Airforce.h"
#include <array>
#include <string>
#include <vector>

namespace Rpx::Hoi4 {
enum class CombatRegimentType {
  Irregulars,
  Infantry,
  Cavalry,
  Motorized,
  LightArmor,
  AntiTank,
  AntiAir,
  Artillery,
  MotorizedAntiTank,
  MotorizedAntiAir,
  MotorizedArtillery,
  Paratroopers,
  Marines,
  Mountaineers
};

enum class SupportRegimentType {
  Engineer,
  Recon,
  FieldHospital,
  Logistics,
  Maintenance,
  AntiTank,
  AntiAir,
  Artillery
};

enum class DivisionType {
  Irregulars,
  Militia,
  Infantry,
  SupportedInfantry,
  HeavyArtilleryInfantry,
  Cavalry,
  Motorized,
  SupportedMotorized,
  HeavyArtilleryMotorized,
  Armor
};

static const std::map<CombatRegimentType, double> regimentCost = {
    {CombatRegimentType::Irregulars, 0.5},
    {CombatRegimentType::Infantry, 1.0},
    {CombatRegimentType::Cavalry, 1.0},
    {CombatRegimentType::Motorized, 2.0},
    {CombatRegimentType::LightArmor, 3.0},
    {CombatRegimentType::AntiTank, 1.5},
    {CombatRegimentType::AntiAir, 1.5},
    {CombatRegimentType::Artillery, 1.5},
    {CombatRegimentType::MotorizedAntiTank, 2.5},
    {CombatRegimentType::MotorizedAntiAir, 2.5},
    {CombatRegimentType::MotorizedArtillery, 2.5},
    {CombatRegimentType::Paratroopers, 1.0},
    {CombatRegimentType::Marines, 1.5},
    {CombatRegimentType::Mountaineers, 1.0}};
static const std::map<SupportRegimentType, double> supportRegimentCost = {
    {SupportRegimentType::Engineer, 1.0},
    {SupportRegimentType::Recon, 1.0},
    {SupportRegimentType::FieldHospital, 1.0},
    {SupportRegimentType::Logistics, 1.0},
    {SupportRegimentType::Maintenance, 1.0},
    {SupportRegimentType::AntiTank, 1.0},
    {SupportRegimentType::AntiAir, 1.0},
    {SupportRegimentType::Artillery, 1.0}};

struct DivisionTemplate {
  DivisionType type;
  std::string name;
  std::vector<std::vector<CombatRegimentType>> regiments;
  std::vector<SupportRegimentType> supportRegiments;
  double cost = 4.0;
  double armyShare = 0.0;
};
struct Division {
  DivisionTemplate divisionTemplate;
  std::string name;
  std::shared_ptr<Arda::ArdaProvince> location;
  double startingExperienceFactor;
  double startingEquipmentFactor;
};

std::vector<DivisionTemplate> createDivisionTemplates(
    const std::set<DivisionType> &desiredTypes,
    const std::vector<CombatRegimentType> &availableRegimentTypes,
    const std::vector<SupportRegimentType> &availableSupportRegimentTypes);

} // namespace Rpx::Hoi4