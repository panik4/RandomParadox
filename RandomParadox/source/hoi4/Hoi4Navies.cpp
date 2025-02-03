#include "hoi4/Hoi4Navies.h"

namespace Scenario::Hoi4 {

Module getBestSuitedModule(
    const std::map<TechEra, std::vector<Module>> &availableModules,
    TechEra era) {
  return Module();
}

std::string
getBestSuitedGun(const std::map<TechEra, std::vector<Module>> &availableModules,
                 TechEra era, const ShipClass &shipClass) {

  if (shipClass.type == ShipClassType::Destroyer) {
    if (shipClass.era == TechEra::Buildup) {
      if (availableModules.find(TechEra::Buildup) != availableModules.end()) {
        for (auto module : availableModules.at(TechEra::Buildup)) {
          if (module.name == "basic_light_battery") {
            return "ship_light_battery_2";
          }
        }
      }
    }

    if (shipClass.era == TechEra::Interwar ||
        shipClass.era == TechEra::Buildup) {
      if (availableModules.find(TechEra::Interwar) != availableModules.end()) {
        for (auto module : availableModules.at(TechEra::Interwar)) {
          if (module.name == "basic_battery") {
            return "ship_light_battery_1";
          }
        }
      }
    }
  } else if (shipClass.type == ShipClassType::LightCruiser ||
             shipClass.type == ShipClassType::HeavyCruiser) {
    // check if we have the basic_medium_battery module
    if (shipClass.era == TechEra::Buildup) {
      if (availableModules.find(TechEra::Buildup) != availableModules.end()) {
        for (auto module : availableModules.at(TechEra::Buildup)) {
          if (module.name == "basic_medium_battery") {
            return shipClass.type == ShipClassType::LightCruiser
                       ? "ship_light_medium_battery_2"
                       : "ship_medium_battery_2";
          }
        }
      }
    }
    // check if we have the basic_battery module
    if (shipClass.era == TechEra::Interwar ||
        shipClass.era == TechEra::Buildup) {
      if (availableModules.find(TechEra::Interwar) != availableModules.end()) {
        for (auto module : availableModules.at(TechEra::Interwar)) {
          if (module.name == "basic_battery") {
            return "ship_medium_battery_1";
          }
        }
      }
    }
    return "";
  } else if (shipClass.type == ShipClassType::BattleCruiser ||
             shipClass.type == ShipClassType::BattleShip) {
    // check if we have the basic_heavy_battery module
    if (shipClass.era == TechEra::Buildup) {
      if (availableModules.find(TechEra::Buildup) != availableModules.end()) {
        for (auto module : availableModules.at(TechEra::Buildup)) {
          if (module.name == "basic_heavy_battery") {
            return "ship_heavy_battery_2";
          }
        }
      }
    }
    // check if we have the basic_battery module
    if (shipClass.era == TechEra::Interwar ||
        shipClass.era == TechEra::Buildup) {
      if (availableModules.find(TechEra::Interwar) != availableModules.end()) {
        for (auto module : availableModules.at(TechEra::Interwar)) {
          if (module.name == "basic_battery") {
            return "ship_heavy_battery_1";
          }
        }
      }
    }
  }
  return "empty";
}

std::string getBestSuitedAntiAir(
    const std::map<TechEra, std::vector<Module>> &availableModules, TechEra era,
    const std::map<TechEra, std::vector<ArmyTech>> &availableArmyTech) {
  if (era == TechEra::Buildup) {
    if (availableArmyTech.find(TechEra::Buildup) != availableArmyTech.end()) {
      for (auto module : availableArmyTech.at(TechEra::Buildup)) {
        if (module.name == "improved_anti_air") {
          return "ship_anti_air_2";
        }
      }
    }
  }
  if (era == TechEra::Interwar || era == TechEra::Buildup) {
    if (availableArmyTech.find(TechEra::Interwar) != availableArmyTech.end()) {
      for (auto module : availableArmyTech.at(TechEra::Interwar)) {
        if (module.name == "basic_anti_air") {
          return "ship_anti_air_1";
        }
      }
    }
  }
  return "empty";
}

std::string findBestSuitedFireControlSystem(
    const std::map<TechEra, std::vector<Module>> &availableModules, TechEra era,
    const ShipClass &shipClass) {
  if (era == TechEra::Buildup) {
    if (availableModules.find(TechEra::Buildup) != availableModules.end()) {
      for (auto module : availableModules.at(TechEra::Buildup)) {
        if (module.name == "fire_control_methods_1") {
          return "ship_fire_control_system_1";
        }
      }
    }
  }
  if (era == TechEra::Interwar || era == TechEra::Buildup) {
    if (availableModules.find(TechEra::Interwar) != availableModules.end()) {
      for (auto module : availableModules.at(TechEra::Interwar)) {
        if (module.name == "basic_fire_control_methods") {
          return "ship_fire_control_system_0";
        }
      }
    }
  }
  if (shipClass.type == ShipClassType::LightCruiser ||
      shipClass.type == ShipClassType::HeavyCruiser) {
    // always return the ship_sonar_1 if no fire control system is available, as
    // heavier ships need them
    return "ship_sonar_1";
  }
  return "empty";
}

std::string findBestSuitedEngine(TechEra era, const ShipClass &shipClass) {

  std::string engineType = "";
  if (shipClass.type == ShipClassType::Destroyer) {
    engineType = "light_";
  } else if (shipClass.type == ShipClassType::LightCruiser ||
             shipClass.type == ShipClassType::HeavyCruiser) {
    engineType = "cruiser_";
  } else if (shipClass.type == ShipClassType::BattleShip ||
             shipClass.type == ShipClassType::BattleCruiser) {
    engineType = "heavy_";
  } else if (shipClass.type == ShipClassType::Carrier) {
    engineType = "carrier_";
  } else if (shipClass.type == ShipClassType::Submarine) {
    engineType = "sub_";
  }
  engineType += "ship_engine_";
  if (era == TechEra::Interwar) {
    return engineType + "1";
  }
  if (era == TechEra::Buildup) {
    return engineType + "2";
  }

  return "empty";
}

std::string findBestSuitedTorpedoLauncher(
    const std::map<TechEra, std::vector<Module>> &availableModules, TechEra era,
    const ShipClass &shipClass) {
  // we always have the torpedo equivalent of the submarine hull tech already
  if (shipClass.type == ShipClassType::Submarine) {
    if (era == TechEra::Buildup) {
      return "ship_torpedo_sub_2";
    }
    if (era == TechEra::Interwar) {
      return "ship_torpedo_sub_1";
    }
  } else {
    if (era == TechEra::Buildup) {
      if (availableModules.find(TechEra::Buildup) != availableModules.end()) {
        for (auto module : availableModules.at(TechEra::Buildup)) {
          if (module.name == "improved_ship_torpedo_launcher") {
            return "ship_torpedo_2";
          }
        }
      }
    }
    if (era == TechEra::Interwar || era == TechEra::Buildup) {
      if (availableModules.find(TechEra::Interwar) != availableModules.end()) {
        for (auto module : availableModules.at(TechEra::Interwar)) {
          if (module.name == "basic_torpedo") {
            return "ship_torpedo_1";
          }
        }
      }
    }
  }

  return "empty";
}

std::string findBestSuitedDepthCharges(
    const std::map<TechEra, std::vector<Module>> &availableModules,
    TechEra era) {
  if (era == TechEra::Buildup) {
    if (availableModules.find(TechEra::Buildup) != availableModules.end()) {
      for (auto module : availableModules.at(TechEra::Buildup)) {
        if (module.name == "improved_depth_charges") {
          return "ship_depth_charge_2";
        }
      }
    }
  }
  if (era == TechEra::Interwar || era == TechEra::Buildup) {
    if (availableModules.find(TechEra::Interwar) != availableModules.end()) {
      for (auto module : availableModules.at(TechEra::Interwar)) {
        if (module.name == "basic_depth_charges") {
          return "ship_depth_charge_1";
        }
      }
    }
  }
  return "empty";
}

std::string findBestSuitedArmor(
    const std::map<TechEra, std::vector<Module>> &availableModules, TechEra era,
    const ShipClass &shipClass) {
  if (era == TechEra::Buildup) {
    if (availableModules.find(TechEra::Buildup) != availableModules.end()) {
      for (auto module : availableModules.at(TechEra::Buildup)) {
        if (module.name == "basic_heavy_armor_scheme") {
          // switch the armor types for cruiser, battlecruiser, battleship and
          // carrier
          switch (shipClass.type) {
          case ShipClassType::LightCruiser:
            return "ship_armor_cruiser_2";
          case ShipClassType::HeavyCruiser:
            return "ship_armor_cruiser_2";
          case ShipClassType::BattleCruiser:
            return "ship_armor_bc_2";
          case ShipClassType::BattleShip:
            return "ship_armor_bb_2";
          case ShipClassType::Carrier:
            return "ship_armor_cruiser_2";
          default:
            return "empty";
          }
        }
      }
    }
  }
  if (era == TechEra::Interwar || era == TechEra::Buildup) {
    if (availableModules.find(TechEra::Interwar) != availableModules.end()) {
      for (auto module : availableModules.at(TechEra::Interwar)) {
        if (module.name == "basic_cruiser_armor_scheme") {
          // switch the armor types for cruiser, battlecruiser, battleship and
          // carrier
          switch (shipClass.type) {
          case ShipClassType::LightCruiser:
            return "ship_armor_cruiser_1";
          case ShipClassType::HeavyCruiser:
            return "ship_armor_cruiser_1";
          case ShipClassType::BattleCruiser:
            return "ship_armor_bc_1";
          case ShipClassType::BattleShip:
            return "ship_armor_bb_1";
          case ShipClassType::Carrier:
            return "ship_armor_cruiser_1";
          default:
            return "empty";
          }
        }
      }
    }
  }
  return "empty";
}

std::string findBestSuitedSecondaryGun(
    const std::map<TechEra, std::vector<Module>> &availableModules,
    TechEra era) {
  if (era == TechEra::Buildup) {
    if (availableModules.find(TechEra::Buildup) != availableModules.end()) {
      for (auto module : availableModules.at(TechEra::Buildup)) {
        if (module.name == "basic_medium_battery") {
          return "ship_secondaries_2";
        }
      }
    }
    if (era == TechEra::Interwar || era == TechEra::Buildup) {
      if (availableModules.find(TechEra::Interwar) != availableModules.end()) {
        for (auto module : availableModules.at(TechEra::Interwar)) {
          if (module.name == "basic_battery") {
            return "ship_secondaries_1";
          }
        }
      }
    }
  }
  return "empty";
}

std::string findBestSuitedMineLayer(
    const std::map<TechEra, std::vector<Module>> &availableModules, TechEra era,
    const ShipClass &shipClass) {

  if (shipClass.type == ShipClassType::Submarine) {
    if (availableModules.find(TechEra::Buildup) != availableModules.end()) {
      for (auto module : availableModules.at(TechEra::Buildup)) {
        if (module.name == "submarine_mine_laying") {
          return "ship_mine_layer_sub";
        }
      }
    }
    return "empty";
  }

  if (era == TechEra::Interwar || era == TechEra::Buildup) {
    if (availableModules.find(TechEra::Interwar) != availableModules.end()) {
      for (auto module : availableModules.at(TechEra::Interwar)) {
        if (module.name == "basic_naval_mines") {
          return "ship_mine_layer_1";
        }
      }
    }
  }
  return "empty";
}

std::string selectRandomCustomSlot(
    const std::map<TechEra, std::vector<Module>> &availableModules, TechEra era,
    const std::map<TechEra, std::vector<ArmyTech>> &availableArmyTech,
    ShipClass &shipClass, bool aaAllowed, bool gunAllowed,
    bool rapidFireAllowed, bool secondaryBatteryAllowed, bool minesAllowed,
    bool torpedosAllowed, bool depthChargesAllowed, bool deckArmorAllowed,
    bool hangarSpaceAllowed) {

  // first gather all the available modules for the slot
  std::set<std::string> availableModulesForSlot;
  if (gunAllowed) {
    availableModulesForSlot.insert(
        shipClass.mtgModules["fixed_ship_battery_slot"]);
  }
  if (aaAllowed) {
    availableModulesForSlot.insert(getBestSuitedAntiAir(
        availableModules, shipClass.era, availableArmyTech));
  }
  if (rapidFireAllowed) {
    // availableModulesForSlot.insert("ship_rapid_fire_1");
  }
  if (secondaryBatteryAllowed) {
    availableModulesForSlot.insert(
        findBestSuitedSecondaryGun(availableModules, era));
  }
  if (minesAllowed) {
    availableModulesForSlot.insert(
        findBestSuitedMineLayer(availableModules, era, shipClass));
  }
  if (torpedosAllowed) {
    availableModulesForSlot.insert(
        findBestSuitedTorpedoLauncher(availableModules, era, shipClass));
  }
  if (depthChargesAllowed) {
    availableModulesForSlot.insert(
        findBestSuitedDepthCharges(availableModules, era));
  }
  if (deckArmorAllowed) {
    availableModulesForSlot.insert("ship_armor_carrier_deck");
  }
  if (hangarSpaceAllowed) {
    availableModulesForSlot.insert("ship_deck_space");
  }
  // select one random string
  return Fwg::Utils::selectRandom(availableModulesForSlot);
}

void addShipClassModules(
    ShipClass &shipClass,
    const std::map<TechEra, std::vector<Module>> &availableModuleTech,
    const std::map<TechEra, std::vector<ArmyTech>> &availableArmyTech) {

  // all ships have an engine
  // fixed_ship_engine_slot
  shipClass.mtgModules["fixed_ship_engine_slot"] =
      findBestSuitedEngine(shipClass.era, shipClass);
  // all ships except for submarine have these slots
  if (shipClass.type != ShipClassType::Submarine) {
    // fixed_ship_anti_air_slot
    shipClass.mtgModules["fixed_ship_anti_air_slot"] = getBestSuitedAntiAir(
        availableModuleTech, shipClass.era, availableArmyTech);
    // fixed_ship_radar_slot
    shipClass.mtgModules["fixed_ship_radar_slot"] = "empty";
  }
  // all ships except for submarine and carrier have a battery slot
  if (shipClass.type != ShipClassType::Submarine &&
      shipClass.type != ShipClassType::Carrier) {
    // fixed_ship_battery_slot
    shipClass.mtgModules["fixed_ship_battery_slot"] =
        getBestSuitedGun(availableModuleTech, shipClass.era, shipClass);
    // fixed_ship_fire_control_system_slot
    shipClass.mtgModules["fixed_ship_fire_control_system_slot"] =
        findBestSuitedFireControlSystem(availableModuleTech, shipClass.era,
                                        shipClass);
  }

  // all submarines and destroyers have these slots
  if (shipClass.type == ShipClassType::Destroyer ||
      shipClass.type == ShipClassType::Submarine) {
    // fixed_ship_torpedo_slot
    shipClass.mtgModules["fixed_ship_torpedo_slot"] =
        findBestSuitedTorpedoLauncher(availableModuleTech, shipClass.era,
                                      shipClass);
  }

  // ships except for submarines and destroyers have armor
  if (shipClass.type != ShipClassType::Destroyer &&
      shipClass.type != ShipClassType::Submarine) {
    // fixed_ship_armor_slot
    shipClass.mtgModules["fixed_ship_armor_slot"] =
        findBestSuitedArmor(availableModuleTech, shipClass.era, shipClass);
  }

  if (shipClass.type == ShipClassType::Destroyer) {
    // start filling the slots as described above, taking available modules,
    // preferrably of the correct era, if not, of the previous era if no
    // modules are available, the slot is empty

    // mid_1_custom_slot
    shipClass.mtgModules["mid_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, true,
        false, false, false, true, true, true, false, false);
    // rear_1_custom_slot
    shipClass.mtgModules["rear_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, true,
        false, false, false, true, true, true, false, false);
  }

  else if (shipClass.type == ShipClassType::LightCruiser ||
           shipClass.type == ShipClassType::HeavyCruiser) {
    // mid_1_custom_slot
    shipClass.mtgModules["mid_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, true,
        true, true, true, true, true, false, false, false);
    // mid_2_custom_slot
    shipClass.mtgModules["mid_2_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, true,
        true, true, true, true, true, false, false, false);
    // rear_1_custom_slot
    shipClass.mtgModules["rear_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, true,
        true, true, false, true, false, true, false, false);
  } else if (shipClass.type == ShipClassType::BattleCruiser ||
             shipClass.type == ShipClassType::BattleShip) {
    // front_1_custom_slot
    shipClass.mtgModules["front_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, true,
        true, false, false, false, false, false, false, false);
    // mid_1_custom_slot
    shipClass.mtgModules["mid_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, true,
        false, false, true, false, false, false, false, false);
    // mid_2_custom_slot
    shipClass.mtgModules["mid_2_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, true,
        false, false, true, false, false, false, false, false);
    // rear_1_custom_slot
    shipClass.mtgModules["rear_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, true,
        true, false, true, false, false, false, false, false);
  }
  // carriers: distinguished by hangar space
  else if (shipClass.type == ShipClassType::Carrier) {
    // front_1_custom_slot
    shipClass.mtgModules["front_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, true,
        false, false, false, false, false, false, true, true);
    // mid_1_custom_slot
    shipClass.mtgModules["mid_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, false,
        false, false, true, false, false, false, true, true);
    // fixed_ship_deck_slot_1 = ship_deck_space
    shipClass.mtgModules["fixed_ship_deck_slot_1"] = "ship_deck_space";
    // fixed_ship_deck_slot_2 = ship_deck_space
    shipClass.mtgModules["fixed_ship_deck_slot_2"] = "ship_deck_space";
    // fixed_ship_secondaries_slot =
    shipClass.mtgModules["fixed_ship_secondaries_slot"] = "ship_secondaries_1";
  } else if (shipClass.type == ShipClassType::Submarine) {
    // rear_1_custom_slot
    shipClass.mtgModules["rear_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, false,
        false, false, false, true, true, false, false, false);
  }
}

} // namespace Scenario::Hoi4