#include "hoi4/Hoi4Airforce.h"

namespace Scenario::Hoi4 {
void adjustTechsForPlaneModules(
    std::map<TechEra, std::vector<Technology>> &availableModuleTech) {
  auto &airTechs = availableModuleTech.at(TechEra::Interwar);
  std::set<std::string> airframeSet;

  // Collect all existing airframes
  for (const auto &tech : airTechs) {
    airframeSet.insert(tech.name);
  }

  // Replace airframes if necessary
  for (auto &tech : airTechs) {
    if (tech.name == "iw_medium_airframe" &&
        airframeSet.find("iw_small_airframe") == airframeSet.end()) {
      tech.name = "iw_small_airframe";
      airframeSet.insert("iw_small_airframe");
      airframeSet.erase("iw_medium_airframe");
    }
    if (tech.name == "iw_large_airframe" &&
        airframeSet.find("iw_medium_airframe") == airframeSet.end()) {
      tech.name = "iw_medium_airframe";
      airframeSet.insert("iw_medium_airframe");
      airframeSet.erase("iw_large_airframe");
    }
  }
  airframeSet.clear();
  // Collect all existing airframes
  for (const auto &tech : airTechs) {
    airframeSet.insert(tech.name);
  }
  // make sure we have at least one fitting purpose for iw_small_airframe (which
  // is aa_lmg) and iw_medium_airframe (which is early_bombs)
  if (airframeSet.contains("iw_small_airframe") &&
      !airframeSet.contains("aa_lmg")) {
    airTechs.push_back(Technology{"aa_lmg", "", TechEra::Interwar});
  }
  if (airframeSet.contains("iw_medium_airframe") &&
      !airframeSet.contains("early_bombs")) {
    airTechs.push_back(Technology{"early_bombs", "", TechEra::Interwar});
  }
}

void selectMainWeapon(PlaneVariant &planeVariant) {
  if (planeVariant.subType == PlaneRole::Fighter ||
      planeVariant.subType == PlaneRole::CarrierFighter) {
    planeVariant.bbaModules["fixed_main_weapon_slot"] = "light_mg_2x";
  } else if (planeVariant.subType == PlaneRole::Cas ||
             planeVariant.subType == PlaneRole::CarrierCas) {
    planeVariant.bbaModules["fixed_main_weapon_slot"] = "bomb_locks";
  } else if (planeVariant.subType == PlaneRole::NavalBomber ||
             planeVariant.subType == PlaneRole::CarrierNavalBomber) {
    planeVariant.bbaModules["fixed_main_weapon_slot"] = "torpedo_mounting";
  } else if (planeVariant.subType == PlaneRole::TacticalBomber) {
    planeVariant.bbaModules["fixed_main_weapon_slot"] = "medium_bomb_bay";
  } else if (planeVariant.subType == PlaneRole::StrategicBomber) {
    planeVariant.bbaModules["fixed_main_weapon_slot"] = "large_bomb_bay";
  }
}
void selectEngine(PlaneVariant &planeVariant) {
  if (planeVariant.type == PlaneType::SmallFrame) {
    planeVariant.bbaModules["engine_type_slot"] = "engine_1_1x";
  } else if (planeVariant.type == PlaneType::MediumFrame) {
    planeVariant.bbaModules["engine_type_slot"] = "engine_1_2x";
  } else if (planeVariant.type == PlaneType::LargeFrame) {
    planeVariant.bbaModules["engine_type_slot"] = "engine_1_4x";
  }
}

void selectSpecialEquipment(PlaneVariant &planeVariant) {
  if (planeVariant.type == PlaneType::SmallFrame) {
    planeVariant.bbaModules["special_type_slot_1"] = "empty";
  } else if (planeVariant.type == PlaneType::MediumFrame) {
    planeVariant.bbaModules["special_type_slot_1"] = "lmg_defense_turret";
  } else if (planeVariant.type == PlaneType::LargeFrame) {
    planeVariant.bbaModules["special_type_slot_1"] = "lmg_defense_turret";
  }
}

void addPlaneModules(
    PlaneVariant &planeVariant,
    const std::map<TechEra, std::vector<Technology>> &availableModuleTech) {
  selectEngine(planeVariant);
  selectMainWeapon(planeVariant);
  selectSpecialEquipment(planeVariant);
}

} // namespace Scenario::Hoi4