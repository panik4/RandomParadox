#pragma once
#include "areas/ArdaProvince.h"
#include "hoi4/Hoi4Armor.h"
#include "hoi4/Hoi4Tech.h"
#include "hoi4/Hoi4Airforce.h"
#include "utils/Archive.h"
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

inline std::map<Rpx::Hoi4::DivisionType, std::string> divisionTypeMap{
    {Rpx::Hoi4::DivisionType::Irregulars, "Irregulars Division"},
    {Rpx::Hoi4::DivisionType::Militia, "Militia Division"},
    {Rpx::Hoi4::DivisionType::Infantry, "Infantry Division"},
    {Rpx::Hoi4::DivisionType::SupportedInfantry, "Supported Infantry Division"},
    {Rpx::Hoi4::DivisionType::HeavyArtilleryInfantry, "Artillery Division"},
    {Rpx::Hoi4::DivisionType::Cavalry, "Cavalry Division"},
    {Rpx::Hoi4::DivisionType::Motorized, "Motorized Division"},
    {Rpx::Hoi4::DivisionType::SupportedMotorized,
     "Supported Motorized Division"},
    {Rpx::Hoi4::DivisionType::HeavyArtilleryMotorized,
     "Motorized Artillery Division"},
    {Rpx::Hoi4::DivisionType::Armor, "Armored Division"}};



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

  void serialise(Fwg::Utils::Serialisation::Archive &ar) {
    ar.serialiseEnum(type);
    ar &name &cost &armyShare;
    // Manual enum vector serialisation
    if (ar.isWriting()) {
      auto writeEnumVec = [&](auto &v) {
        uint64_t sz = v.size();
        ar &sz;
        for (auto &e : v) {
          std::underlying_type_t<std::decay_t<decltype(e)>> raw =
              static_cast<std::underlying_type_t<std::decay_t<decltype(e)>>>(e);
          ar &raw;
        }
      };
      auto writeEnumVecVec = [&](auto &v) {
        uint64_t osz = v.size();
        ar &osz;
        for (auto &inner : v)
          writeEnumVec(inner);
      };
      writeEnumVecVec(regiments);
      writeEnumVec(supportRegiments);
    } else {
      auto readEnumVec = [&](auto &v) {
        uint64_t sz;
        ar &sz;
        v.resize(static_cast<size_t>(sz));
        for (auto &e : v) {
          std::underlying_type_t<typename std::decay_t<decltype(v)>::value_type>
              raw;
          ar &raw;
          e = static_cast<typename std::decay_t<decltype(v)>::value_type>(raw);
        }
      };
      auto readEnumVecVec = [&](auto &v) {
        uint64_t osz;
        ar &osz;
        v.resize(static_cast<size_t>(osz));
        for (auto &inner : v)
          readEnumVec(inner);
      };
      readEnumVecVec(regiments);
      readEnumVec(supportRegiments);
    }
  }
  void deserialise(Fwg::Utils::Serialisation::Archive &ar) { serialise(ar); }
};
struct Division {
  DivisionTemplate divisionTemplate;
  std::string name;
  std::shared_ptr<Arda::ArdaProvince> location;
  double startingExperienceFactor;
  double startingEquipmentFactor;

  void serialise(Fwg::Utils::Serialisation::Archive &ar) {
    divisionTemplate.serialise(ar);
    ar &name;
    ar.polymorphicPtr(location);
    ar &startingExperienceFactor &startingEquipmentFactor;
  }
  void deserialise(Fwg::Utils::Serialisation::Archive &ar) { serialise(ar); }
};

std::vector<DivisionTemplate> createDivisionTemplates(
    const std::set<DivisionType> &desiredTypes,
    const std::vector<CombatRegimentType> &availableRegimentTypes,
    const std::vector<SupportRegimentType> &availableSupportRegimentTypes);

} // namespace Rpx::Hoi4