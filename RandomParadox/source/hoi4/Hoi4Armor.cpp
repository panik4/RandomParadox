#include "hoi4/Hoi4Armor.h"

namespace Scenario::Hoi4 {

std::string getBestTurretType(
    const std::map<TechEra, std::vector<Technology>> &availableModules,
    TechEra era, const TankVariant &tankVariant) {
  std::vector<std::string> lightTurretTypes = {
      "tank_light_one_man_tank_turret", "tank_light_two_man_tank_turret",
      "tank_light_three_man_tank_turret",
      "tank_light_fixed_superstructure_turret"};
  if (tankVariant.type == ArmorType::LightArmor) {
    if (tankVariant.subType == ArmorSubType::TankDestroyer) {
      return "tank_light_fixed_superstructure_turret";
    }
    return Fwg::Utils::selectRandom(lightTurretTypes);
  }
  std::vector<std::string> mediumTurretTypes = {
      "tank_medium_one_man_tank_turret", "tank_medium_two_man_tank_turret",
      "tank_medium_three_man_tank_turret",
      "tank_medium_fixed_superstructure_turret"};
  if (tankVariant.type == ArmorType::MediumArmor) {
    if (tankVariant.subType == ArmorSubType::TankDestroyer) {
      return "tank_medium_fixed_superstructure_turret";
    }
    return Fwg::Utils::selectRandom(mediumTurretTypes);
  }
  std::vector<std::string> heavyTurretTypes = {
      "tank_heavy_two_man_tank_turret", "tank_heavy_three_man_tank_turret",
      "tank_heavy_fixed_superstructure_turret"};
  if (tankVariant.type == ArmorType::HeavyArmor) {
    if (tankVariant.subType == ArmorSubType::TankDestroyer) {
      return "tank_heavy_fixed_superstructure_turret";
    }
    return Fwg::Utils::selectRandom(heavyTurretTypes);
  }

  return "empty";
}
std::string getBestSuitedGun(
    const std::map<TechEra, std::vector<Technology>> &availableModules,
    TechEra era, const TankVariant &tankVariant) {
  std::vector<std::string> lightGuns = {"tank_heavy_machine_gun"};
  if (hasTechnology(availableModules, "gw_artillery")) {
    // noones is using the heavy mg if we have the tech for a cannon
    lightGuns.clear();
    lightGuns.push_back("tank_small_cannon");
  }

  if (hasTechnology(availableModules, "interwar_artillery")) {
    lightGuns.push_back("tank_small_cannon_2");
    lightGuns.push_back("tank_close_support_gun");
  }
  if (tankVariant.type == ArmorType::LightArmor) {
    if (tankVariant.subType == ArmorSubType::Tank) {
      return Fwg::Utils::selectRandom(lightGuns);
    } else if (tankVariant.subType == ArmorSubType::TankDestroyer) {
      return "tank_high_velocity_cannon";
    } else if (tankVariant.subType == ArmorSubType::AntiAir) {
      return "tank_anti_air_cannon";
    } else if (tankVariant.subType == ArmorSubType::Artillery) {
      return "tank_close_support_gun";
    }
  } else if (tankVariant.type == ArmorType::MediumArmor) {
    if (tankVariant.subType == ArmorSubType::Tank) {
      return "tank_medium_cannon";
    } else if (tankVariant.subType == ArmorSubType::TankDestroyer) {
      // a medium tank destroyer should use the medium fixed superstructure
      // turret and therefore be able to use the heavy cannon
      return "tank_heavy_cannon";
    } else if (tankVariant.subType == ArmorSubType::AntiAir) {
      return "tank_anti_air_cannon";
    } else if (tankVariant.subType == ArmorSubType::Artillery) {
      return "tank_close_support_gun";
    }
  } else if (tankVariant.type == ArmorType::HeavyArmor) {
    if (tankVariant.subType == ArmorSubType::Tank) {
      return "tank_heavy_cannon";
    } else if (tankVariant.subType == ArmorSubType::TankDestroyer) {
      return "tank_heavy_cannon";
    } else if (tankVariant.subType == ArmorSubType::AntiAir) {
      return "tank_anti_air_cannon";
    } else if (tankVariant.subType == ArmorSubType::Artillery) {
      return "tank_close_support_gun";
    }
  }

  return "tank_heavy_machine_gun";
}

std::string getBestEngineType(
    const std::map<TechEra, std::vector<Technology>> &availableModules,
    TechEra era, const TankVariant &tankVariant) {

  const std::vector<std::string> engineTypes = {"tank_gasoline_engine",
                                                "tank_diesel_engine",
                                                "tank_petrol_electric_engine"};
  // return a random armor type
  return Fwg::Utils::selectRandom(engineTypes);
}

std::string getBestSuitedArmorType(
    const std::map<TechEra, std::vector<Technology>> &availableModules,
    TechEra era, const TankVariant &tankVariant) {
  std::vector<std::string> armorTypes = {"tank_riveted_armor",
                                         "tank_cast_armor"};
  // search for the technology "armor_tech_1" in the available modules
  if (availableModules.find(era) != availableModules.end()) {
    for (auto module : availableModules.at(era)) {
      if (module.name == "armor_tech_1") {
        armorTypes.push_back("tank_welded_armor");
      }
    }
  }
  // return a random armor type
  return Fwg::Utils::selectRandom(armorTypes);
}

std::string getBestSuitedSuspensionType(
    const std::map<TechEra, std::vector<Technology>> &availableModules,
    TechEra era, const TankVariant &tankVariant) {

  const std::vector<std::string> suspensionTypes = {
      "tank_christie_suspension", "tank_bogie_suspension",
      "tank_interleaved_suspension", "tank_torsion_bar_suspension"};
  // return a random suspension type
  return Fwg::Utils::selectRandom(suspensionTypes);
}

std::string getBestSuitedAdditionalModules(
    const std::map<TechEra, std::vector<Technology>> &availableModules,
    TechEra era, const TankVariant &tankVariant, bool wantAdditionalTurret) {

  if (wantAdditionalTurret) {
    std::vector<std::string> moduleTypes = {"secondary_turret_hmg",
                                            "secondary_turret_small_cannon"};
    return Fwg::Utils::selectRandom(moduleTypes);

  } else {
    std::vector<std::string> moduleTypes = {
        "extra_ammo_storage", "wet_ammo_storage", "smoke_launchers",
        "expanded_fuel_tank"};
    return Fwg::Utils::selectRandom(moduleTypes);
  }

  return "empty";
}

void addArmorModules(
    TankVariant &tankVariant,
    const std::map<TechEra, std::vector<Technology>> &availableModuleTech) {

  // all tanks have:
  // 			suspension_type_slot = tank_christie_suspension
  // armor_type_slot = tank_riveted_armor
  // engine_type_slot = tank_gasoline_engine
  tankVariant.bbaModules["suspension_type_slot"] = getBestSuitedSuspensionType(
      availableModuleTech, tankVariant.era, tankVariant);
  tankVariant.bbaModules["armor_type_slot"] =
      getBestSuitedArmorType(availableModuleTech, tankVariant.era, tankVariant);
  tankVariant.bbaModules["engine_type_slot"] =
      getBestEngineType(availableModuleTech, tankVariant.era, tankVariant);
  // turret type:
  tankVariant.bbaModules["turret_type_slot"] =
      getBestTurretType(availableModuleTech, tankVariant.era, tankVariant);
  // gun type:
  tankVariant.bbaModules["main_armament_slot"] =
      getBestSuitedGun(availableModuleTech, tankVariant.era, tankVariant);
  // additional modules:
  tankVariant.bbaModules["special_type_slot_1"] =
      getBestSuitedAdditionalModules(availableModuleTech, tankVariant.era,
                                     tankVariant,
                                     RandNum::getRandom(0, 4) % 3 == 0);
  if (RandNum::getRandom(0, 4)) {
    tankVariant.bbaModules["special_type_slot_2"] =
        getBestSuitedAdditionalModules(availableModuleTech, tankVariant.era,
                                       tankVariant, false);
  }
  if (RandNum::getRandom(0, 3)) {
    tankVariant.bbaModules["special_type_slot_3"] =
        getBestSuitedAdditionalModules(availableModuleTech, tankVariant.era,
                                       tankVariant, false);
  }
}

} // namespace Scenario::Hoi4