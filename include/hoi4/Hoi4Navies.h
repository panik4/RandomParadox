#pragma once
#include "hoi4/Hoi4Region.h"
#include "hoi4/Hoi4Tech.h"
#include <array>
#include <string>
#include <vector>
#include <map>


namespace Rpx::Hoi4 {

enum class ShipClassType {
  Destroyer,
  LightCruiser,
  HeavyCruiser,
  BattleCruiser,
  BattleShip,
  Carrier,
  Submarine
};
  // map from the ShipClassType to the required NavalHullType
inline std::map<ShipClassType, NavalHullType> shipClassToHullType = {
    {ShipClassType::Destroyer, NavalHullType::Light},
    {ShipClassType::LightCruiser, NavalHullType::Cruiser},
    {ShipClassType::HeavyCruiser, NavalHullType::Cruiser},
    {ShipClassType::BattleCruiser, NavalHullType::Heavy},
    {ShipClassType::BattleShip, NavalHullType::Heavy},
    {ShipClassType::Carrier, NavalHullType::Carrier},
    {ShipClassType::Submarine, NavalHullType::Submarine}};

// navy:
inline std::map<ShipClassType, int> tonnages = {
    {ShipClassType::Destroyer, 2000},     {ShipClassType::LightCruiser, 5000},
    {ShipClassType::HeavyCruiser, 10000}, {ShipClassType::BattleCruiser, 30000},
    {ShipClassType::BattleShip, 30000},   {ShipClassType::Carrier, 20000},
    {ShipClassType::Submarine, 1500}};
// vector of all ShipClassTypes
inline std::vector<ShipClassType> shipClassTypes = {
    ShipClassType::Destroyer,    ShipClassType::LightCruiser,
    ShipClassType::HeavyCruiser, ShipClassType::BattleCruiser,
    ShipClassType::BattleShip,   ShipClassType::Carrier,
    ShipClassType::Submarine};
// vector of all ShipClassEras
inline std::vector<TechEra> shipEras = {TechEra::Interwar, TechEra::Buildup};

inline std::map<ShipClassType, std::string> ShipClassTypeDefinitions = {
    {ShipClassType::Destroyer, "destroyer"},
    {ShipClassType::LightCruiser, "light_cruiser"},
    {ShipClassType::HeavyCruiser, "heavy_cruiser"},
    {ShipClassType::BattleCruiser, "battle_cruiser"},
    {ShipClassType::BattleShip, "battleship"},
    {ShipClassType::Carrier, "carrier"},
    {ShipClassType::Submarine, "submarine"}};
// for mtg
inline std::map<ShipClassType, std::string> shipHullDefinitions = {
    {ShipClassType::Destroyer, "ship_hull_light"},
    {ShipClassType::LightCruiser, "ship_hull_cruiser"},
    {ShipClassType::HeavyCruiser, "ship_hull_cruiser"},
    {ShipClassType::BattleCruiser, "ship_hull_heavy"},
    {ShipClassType::BattleShip, "ship_hull_heavy"},
    {ShipClassType::Carrier, "ship_hull_carrier"},
    {ShipClassType::Submarine, "ship_hull_submarine"}};


struct ShipClass {
  ShipClassType type;
  TechEra era;
  std::string name;
  int tonnage;
  std::string vanillaShipType;
  std::string mtgHullname;
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
  std::shared_ptr<Arda::ArdaProvince> startingPort;
  std::vector<std::shared_ptr<Ship>> ships;
};

void addShipClassModules(
    ShipClass& shipClass,
    const std::map<TechEra, std::vector<Technology>> &availableModuleTech,
    const std::map<TechEra, std::vector<Technology>> &availableArmyTech);

} // namespace Rpx::Hoi4