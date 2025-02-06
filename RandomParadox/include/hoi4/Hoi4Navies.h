#pragma once
#include "hoi4/Hoi4Region.h"
#include "hoi4/Hoi4Tech.h"
#include <array>
#include <string>
#include <vector>

namespace Scenario::Hoi4 {

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
  std::shared_ptr<GameProvince> startingPort;
  std::vector<std::shared_ptr<Ship>> ships;
};

void addShipClassModules(
    ShipClass& shipClass,
    const std::map<TechEra, std::vector<Module>> &availableModuleTech,
    const std::map<TechEra, std::vector<Module>> &availableArmyTech);

} // namespace Scenario::Hoi4