#pragma once
#include "hoi4/Hoi4Region.h"
#include <array>
#include <string>
#include <vector>

namespace Scenario::Hoi4 {

enum class TechEra { Interwar, Buildup, Early };

enum class NavalHullType { Light, Cruiser, Heavy, Carrier, Submarine };

enum class ModuleType {
  Armor,
  Gun,
  Shell,
  Torpedo,
  Sonar,
  SmokeGenerator,
  DepthCharge,
  Transport,
  DamageControl,
  FireControl
};

struct Module {
  std::string name;
  std::string predecessor;
  TechEra era;
};

struct ArmyTech {
  std::string name;
  std::string predecessor;
  TechEra era;
};

static std::map<TechEra, std::vector<Module>> moduleTech = {
    {TechEra::Interwar,
     {{"basic_cruiser_armor_scheme", "", TechEra::Interwar},
      {"basic_battery", "", TechEra::Interwar},
      {"basic_torpedo", "", TechEra::Interwar},
      {"basic_naval_mines", "", TechEra::Interwar},
      {"submarine_mine_laying", "", TechEra::Interwar},
      {"mtg_transport", "", TechEra::Interwar},
      {"smoke_generator", "", TechEra::Interwar},
      {"basic_depth_charges", "", TechEra::Interwar},
      {"sonar", "", TechEra::Interwar}

     }},
    {TechEra::Buildup,
     {{"basic_heavy_armor_scheme", "basic_cruiser_armor_scheme",
       TechEra::Buildup},
      {"basic_light_battery", "basic_battery", TechEra::Buildup},
      {"basic_medium_battery", "basic_battery", TechEra::Buildup},
      {"basic_heavy_battery", "basic_battery", TechEra::Buildup},
      {"magnetic_detonator", "basic_torpedo", TechEra::Buildup},
      {"improved_ship_torpedo_launcher", "basic_torpedo", TechEra::Buildup},
      {"damage_control_1", "", TechEra::Buildup},
      {"fire_control_methods_1", "", TechEra::Buildup},
      {"improved_depth_charges", "basic_depth_charges", TechEra::Buildup},
      {"improved_sonar", "sonar", TechEra::Buildup}

     }},
    {TechEra::Early,
     {{"basic_light_shell", "basic_light_battery", TechEra::Buildup},
      {"basic_medium_shell", "basic_medium_battery", TechEra::Buildup},
      {"basic_heavy_shell", "basic_heavy_battery", TechEra::Buildup}

     }}};

enum class ShipClassType {
  Destroyer,
  LightCruiser,
  HeavyCruiser,
  BattleCruiser,
  BattleShip,
  Carrier,
  Submarine
};

struct ShipClass {
  ShipClassType type;
  TechEra era;
  std::string name;
  int tonnage;
  std::string hullname;
  std::map<std::string, std::string> mtgModules;
  // this is the level between two major upgrades, and determines for mtgModules
  // if we take some of the modules from the next era, and for vanilla how many
  // upgrades have happened. Range between 0.0 and 1.0
  double upgradeLevel = 0.0;
};

struct Ship {
  ShipClass shipClass;
  std::string name;
};

struct Fleet {
  std::string name;
  std::shared_ptr<GameProvince> startingPort;
  std::vector<std::shared_ptr<Ship>> ships;
};

void addShipClassModules(
    ShipClass& shipClass,
    const std::map<TechEra, std::vector<Module>> &availableModuleTech,
    const std::map<TechEra, std::vector<ArmyTech>> &availableArmyTech);

} // namespace Scenario::Hoi4