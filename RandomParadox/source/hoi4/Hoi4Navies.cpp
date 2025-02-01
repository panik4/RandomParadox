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
             shipClass.type == ShipClassType::HeavyCruiser)
  return "empty";
}

std::string getBestSuitedAntiAir(
    const std::map<TechEra, std::vector<Module>> &availableModules, TechEra era,
    const std::map<TechEra, std::vector<ArmyTech>> &availableArmyTech) {
  if (era == TechEra::Buildup) {
    if (availableArmyTech.find(TechEra::Buildup) != availableArmyTech.end()) {
      for (auto module : availableArmyTech.at(TechEra::Buildup)) {
        if (module.name == "basic_anti_air") {
          return "ship_anti_air_1";
        }
      }
    }
  }
  if (era == TechEra::Interwar || era == TechEra::Buildup) {
    if (availableArmyTech.find(TechEra::Interwar) != availableArmyTech.end()) {
      for (auto module : availableArmyTech.at(TechEra::Interwar)) {
        if (module.name == "improved_anti_air") {
          return "ship_anti_air_2";
        }
      }
    }
  }
  return "empty";
}

std::string findBestSuitedFireControlSystem(
    const std::map<TechEra, std::vector<Module>> &availableModules,
    TechEra era) {
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
          return "ship_fire_control_system_2";
        }
      }
    }
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
    engineType = "battleship_";
  } else if (shipClass.type == ShipClassType::Carrier) {
    engineType = "carrier_";
  } else if (shipClass.type == ShipClassType::Submarine) {
    engineType = "submarine_";
  }
  if (era == TechEra::Interwar) {
    return engineType + "1";
  }
  if (era == TechEra::Buildup) {
    return engineType + "2";
  }

  return "empty";
}

std::string findBestSuitedTorpedoLauncher(
    const std::map<TechEra, std::vector<Module>> &availableModules,
    TechEra era) {
  if (era == TechEra::Buildup) {
    if (availableModules.find(TechEra::Buildup) != availableModules.end()) {
      for (auto module : availableModules.at(TechEra::Buildup)) {
        if (module.name == "improved_ship_torpedo_launcher") {
          return "ship_torpedo_launcher_2";
        }
      }
    }
  }
  if (era == TechEra::Interwar || era == TechEra::Buildup) {
    if (availableModules.find(TechEra::Interwar) != availableModules.end()) {
      for (auto module : availableModules.at(TechEra::Interwar)) {
        if (module.name == "basic_torpedo") {
          return "ship_torpedo_launcher_1";
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
          return "ship_depth_charges_2";
        }
      }
    }
  }
  if (era == TechEra::Interwar || era == TechEra::Buildup) {
    if (availableModules.find(TechEra::Interwar) != availableModules.end()) {
      for (auto module : availableModules.at(TechEra::Interwar)) {
        if (module.name == "basic_depth_charges") {
          return "ship_depth_charges_1";
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
            return "empty";
          }
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
    bool torpedosAllowed, bool depthChargesAllowed) {

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
    // availableModulesForSlot.insert("ship_secondary_battery_1");
  }
  if (minesAllowed) {
    // availableModulesForSlot.insert("ship_mines_1");
  }
  if (torpedosAllowed) {
    availableModulesForSlot.insert(
        findBestSuitedTorpedoLauncher(availableModules, era));
  }
  if (depthChargesAllowed) {
    availableModulesForSlot.insert(
        findBestSuitedDepthCharges(availableModules, era));
  }
  // select one random string
  return Fwg::Utils::selectRandom(availableModulesForSlot);
}

void addShipClassModules(
    ShipClass shipClass,
    const std::map<TechEra, std::vector<Module>> &availableModuleTech,
    const std::map<TechEra, std::vector<ArmyTech>> &availableArmyTech) {

  /* destroyers:
* light battery + aa + sonar/fire control + sonar/radar + torpedo + engine
* Slots:
* 		fixed_ship_battery_slot =
                    fixed_ship_anti_air_slot =
                    fixed_ship_fire_control_system_slot =
                    fixed_ship_radar_slot =
                    fixed_ship_engine_slot =
                    fixed_ship_torpedo_slot =
* + 2x custom(AA, Mines, light battery, torpedo launcher, depth charges)
*		mid_1_custom_slot =
        rear_1_custom_slot =
        */

  // all ships have an engine
  // fixed_ship_engine_slot
  shipClass.mtgModules["fixed_ship_engine_slot"] =
      findBestSuitedEngine(shipClass.era, shipClass);
  // all ships except for submarine have these slots
  if (shipClass.type != ShipClassType::Submarine) {
    // fixed_ship_anti_air_slot
    shipClass.mtgModules["fixed_ship_anti_air_slot"] = getBestSuitedAntiAir(
        availableModuleTech, shipClass.era, availableArmyTech);
    // fixed_ship_fire_control_system_slot
    shipClass.mtgModules["fixed_ship_fire_control_system_slot"] =
        findBestSuitedFireControlSystem(availableModuleTech, shipClass.era);
    // fixed_ship_radar_slot
    shipClass.mtgModules["fixed_ship_radar_slot"] = "empty";
  }
  // all ships except for submarine and carrier have a battery slot
  if (shipClass.type != ShipClassType::Submarine &&
      shipClass.type != ShipClassType::Carrier) {
    // fixed_ship_battery_slot
    shipClass.mtgModules["fixed_ship_battery_slot"] =
        getBestSuitedGun(availableModuleTech, shipClass.era, shipClass);
  }

  // all submarines and destroyers have these slots
  if (shipClass.type == ShipClassType::Destroyer ||
      shipClass.type == ShipClassType::Submarine) {
    // fixed_ship_torpedo_slot
    shipClass.mtgModules["fixed_ship_torpedo_slot"] =
        findBestSuitedTorpedoLauncher(availableModuleTech, shipClass.era);
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
        false, false, false, true, true, true);
    // rear_1_custom_slot
    shipClass.mtgModules["rear_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, true,
        false, false, false, true, true, true);
  }

  /*
* cruisers: distinguished mainly by battery size: light battery for light
* cruisers, medium battery for heavy cruisers:
* battery + aa + sonar/fire control + sonar/radar + engine + cruiser armor
* Slots:
* 		fixed_ship_battery_slot =
                    fixed_ship_fire_control_system_slot =
                    fixed_ship_radar_slot =
                    fixed_ship_engine_slot =
                    fixed_ship_armor_slot =
                    fixed_ship_secondaries_slot =
* 3x custom(AA, Mines, light battery, torpedo launcher, depth charges)
* 		mid_1_custom_slot =
                    mid_2_custom_slot =
                    rear_1_custom_slot =
*
*
*
*/
  else if (shipClass.type == ShipClassType::LightCruiser) {
    // mid_1_custom_slot
    shipClass.mtgModules["mid_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, true, true, true, true, true, true, true);
    // mid_2_custom_slot
    shipClass.mtgModules["mid_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass,
        true, true, true, true, true, true, true);
    // rear_1_custom_slot
    shipClass.mtgModules["rear_1_custom_slot"] = selectRandomCustomSlot(
        availableModuleTech, shipClass.era, availableArmyTech, shipClass, true, true, true, false, true, false, true);
  }

  // print all slots of the ship
  for (auto &slot : shipClass.mtgModules) {
    std::cout << slot.first << ": " << slot.second << std::endl;
  }
}

} // namespace Scenario::Hoi4